#include "driver.h"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/uaccess.h>

#define EDU_BUFF_START 0x40000
/* PCI bus error detected on this device */
pci_ers_result_t eduv7_err_error_detected(struct pci_dev *dev,
                                          pci_channel_state_t error) {
        return PCI_ERS_RESULT_NONE;
}

/* MMIO has been re-enabled, but not DMA */
pci_ers_result_t eduv7_err_mmio_enabled(struct pci_dev *dev) {
        return PCI_ERS_RESULT_NONE;
}

/* PCI slot has been reset */
pci_ers_result_t eduv7_err_slot_reset(struct pci_dev *dev) {
        return PCI_ERS_RESULT_NONE;
}

/* PCI function reset prepare */
void eduv7_err_reset_prepare(struct pci_dev *dev) {}

/* PCI function reset completed */
void eduv7_err_reset_done(struct pci_dev *dev) {}

/* Device driver may resume normal operations */
void eduv7_err_resume(struct pci_dev *dev) {}

void eduv7_remove(struct pci_dev *dev) {
        struct eduv7_priv *drv_priv = pci_get_drvdata(dev);

        if (drv_priv) {
                if (drv_priv->mem) {
                        iounmap(drv_priv->mem);
                }

                pci_free_irq_vectors(dev);

                kfree(drv_priv);
        }

        pci_release_region(dev, pci_select_bars(dev, IORESOURCE_MEM));
}

int eduv7_suspend(struct pci_dev *dev, pm_message_t state) { return 0; }
int eduv7_resume(struct pci_dev *dev) { return 0; }
void eduv7_shutdown(struct pci_dev *dev) {}

int eduv7_driver_write(struct pci_dev *dev, int data) {
        struct eduv7_priv *eduv7_priv_data =
            (struct eduv7_priv *)pci_get_drvdata(dev);
        if (!eduv7_priv_data) return -1;
        iowrite32(data, eduv7_priv_data->mem + 4);
        return 0;
}

int eduv7_driver_read(struct pci_dev *dev) {
        struct eduv7_priv *eduv7_priv_data =
            (struct eduv7_priv *)pci_get_drvdata(dev);
        if (!eduv7_priv_data) return -1;

        return ioread32(eduv7_priv_data->mem + 4);
}

int eduv7_driver_dma_write(struct pci_dev *dev, u8 *data, int dat_len) {
        struct eduv7_priv *eduv7_priv_data =
            (struct eduv7_priv *)pci_get_drvdata(dev);
        int index = 0;
        if (!eduv7_priv_data) return -1;

        // addr     -> DMA source address
        iowrite32(data, eduv7_priv_data->mem + 0x80);

        // 0x40000  -> DMA destination address
        iowrite32(EDU_BUFF_START, eduv7_priv_data->mem + 0x88);

        // count      -> DMA transfer count
        iowrite32(dat_len, eduv7_priv_data->mem + 0x90);
#ifdef _USE_IRQS

#else
        // 1        -> DMA command register
        iowrite32(0x01, eduv7_priv_data->mem + 0x98);
        while (ioread32(eduv7_priv_data->mem + 0x98) & 1)
                ;
#endif
        return 0;
}

int eduv7_driver_dma_read(struct pci_dev *dev, u8 *addr, int dat_len) {
        struct eduv7_priv *eduv7_priv_data =
            (struct eduv7_priv *)pci_get_drvdata(dev);
        int index = 0;
        if (!eduv7_priv_data) return -EIO;
        if (dat_len > 4095) return -EIO;

        // 0x40000  -> DMA source address
        iowrite32(EDU_BUFF_START, eduv7_priv_data->mem + 0x80);

        // addr+count -> DMA destination address
        iowrite32(addr, eduv7_priv_data->mem + 0x88);

        // count      -> DMA transfer count
        iowrite32(dat_len, eduv7_priv_data->mem + 0x90);

#ifdef _USE_IRQS

#else
        // 3        -> DMA command register
        iowrite32(0x03, eduv7_priv_data->mem + 0x98);
        while (ioread32(eduv7_priv_data->mem + 0x98) & 1)
                ;
#endif
        return 0;
}

void ping_data_using_edubuf(struct pci_dev *dev) {
        if (!dev) {
                printk(KERN_CRIT "[Eduv7] Error: could not get pci device");
                return;
        }
        struct eduv7_priv *eduv7_priv_data =
            (struct eduv7_priv *)pci_get_drvdata(dev);
        if (!eduv7_priv_data) {
                printk(KERN_CRIT
                       "[Eduv7] Error: could not get private mem of driver");
                return;
        }
        int data_to_write = 0x5ca1ab1e;  // scalable?
        iowrite32(data_to_write, eduv7_priv_data->mem + 4);
        printk(KERN_DEBUG "[Eduv7] Sent data, and received 0x%08x",
               ~ioread32(eduv7_priv_data->mem + 4));  //*data_from_read);
}

int eduv7_probe(struct pci_dev *dev, const struct pci_device_id *id) {
        u16 vendor, device;
        int bar, err;
        struct eduv7_priv *eduv7_priv_mem;
        // struct device* idev = &(dev-> dev);
        unsigned long mmio_range_start, mmio_range_len;
        pci_read_config_word(dev, PCI_VENDOR_ID, &vendor);
        pci_read_config_word(dev, PCI_DEVICE_ID, &device);
        printk(KERN_INFO "[Eduv7] Device vid: 0x%X pid: 0x%X\n", vendor,
               device);

#ifdef _USE_DMA
        pci_set_master(dev);
        dma_supported(&(dev->dev), DMA_BIT_MASK(28));
        err = pci_set_dma_mask(dev, DMA_BIT_MASK(28));
        if (err) {
                printk(KERN_ALERT
                       "[Eduv7] Error: could not set DMA mask. Limited "
                       "functionality!");
        }

        // dma_addr_t* dma_h;
        // dma_databuf = (u8*) kzalloc(sizeof(u8)* 4096,GFP_KERNEL );
        // dma_h = pci_map_single(dev, dma_databuf,4096,DMA_TO_DEVICE);
        // if(dma_h){
        //         printk(KERN_DEBUG "[Eduv7] DMA mem allocated");
        // }else{
        //         printk(KERN_ALERT "[Eduv7] Error: could not allocate DMA
        //         mmem.");

        // }
#endif
        /* Request IO BAR */
        bar = pci_select_bars(dev, IORESOURCE_MEM);

        /* Enable device memory */
        err = pci_enable_device_mem(dev);

        if (err) {
                return err;
        }

        /* Request memory region for the BAR, and to be owned by the driver */
        err = pci_request_region(dev, bar, "eduv7");
        if (err) {
                // pci_disable_device(dev);
                printk(KERN_CRIT "[Eduv7] Error: could not get memory region");
                return err;
        }

        /* Get start and stop memory offsets */
        mmio_range_start = pci_resource_start(dev, 0);
        mmio_range_len = pci_resource_len(dev, 0);

        eduv7_priv_mem = kzalloc(sizeof(struct eduv7_priv), GFP_KERNEL);

        if (!eduv7_priv_mem) {
                printk(KERN_CRIT
                       "[Eduv7] Error: could not get memory for driver's data");
                return -ENOMEM;
        }

        /* Remap BAR to the local pointer */
        eduv7_priv_mem->mem = ioremap(mmio_range_start, mmio_range_len);

        if (!eduv7_priv_mem->mem) {
                printk(
                    KERN_CRIT
                    "[Eduv7] Error: could not get remap BAR to driver's data");
                return -EIO;
        }

        /* Set driver private data */
        pci_set_drvdata(dev, eduv7_priv_mem);

        ping_data_using_edubuf(dev);
        printk(KERN_DEBUG
               "[Eduv7] succesfully probed eduv7, mmio starts at 0x%08x with "
               "range %d",
               mmio_range_start, mmio_range_len);

        return 0;
}
