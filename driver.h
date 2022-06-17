#ifndef _EDUV7_DRIVER_H_
#define EDUV7_DRIVER_H_

#include <linux/pci.h>


// #define _USE_DMA


/* PCI bus error detected on this device */
pci_ers_result_t eduv7_err_error_detected(struct pci_dev *dev,
                                          pci_channel_state_t error);
/* MMIO has been re-enabled, but not DMA */
pci_ers_result_t eduv7_err_mmio_enabled(struct pci_dev *dev);
/* PCI slot has been reset */
pci_ers_result_t eduv7_err_slot_reset(struct pci_dev *dev);

/* PCI function reset prepare */
void eduv7_err_reset_prepare(struct pci_dev *dev);

/* PCI function reset completed */
void eduv7_err_reset_done(struct pci_dev *dev);

/* Device driver may resume normal operations */
void eduv7_err_resume(struct pci_dev *dev);

static struct pci_error_handlers eduv7_pci_error_handlers = {
    .error_detected = eduv7_err_error_detected,
    .mmio_enabled = eduv7_err_mmio_enabled,
    .slot_reset = eduv7_err_slot_reset,

    .reset_prepare = eduv7_err_reset_prepare,
    .reset_done = eduv7_err_reset_done,
    .resume = eduv7_err_resume};

static struct pci_device_id eduv7_driver_id_table[] = {
    {PCI_DEVICE(0x1234, 0x11e8)},
    {
        0,
    }};

int eduv7_driver_dma_read(struct pci_dev *dev, u8 *addr, int dat_len);
int eduv7_driver_dma_write(struct pci_dev *dev, u8 *data, int dat_len);
int eduv7_driver_write(struct pci_dev *dev, int data);
int eduv7_driver_read(struct pci_dev *dev);

void ping_data_using_edubuf(struct pci_dev *);
void eduv7_remove(struct pci_dev *dev);
int eduv7_suspend(struct pci_dev *dev, pm_message_t state);
int eduv7_resume(struct pci_dev *dev);
void eduv7_shutdown(struct pci_dev *dev);
int eduv7_probe(struct pci_dev *dev, const struct pci_device_id *id);
static struct pci_driver eduv7_driver = {
    .name = "eduv7",
    .id_table = eduv7_driver_id_table,
    .probe = eduv7_probe,
    .remove = eduv7_remove,
    .suspend = eduv7_suspend,
    .resume = eduv7_resume,
    .shutdown = eduv7_shutdown,
    .sriov_configure = NULL, /* On PF */
    .err_handler = &eduv7_pci_error_handlers,
    .groups = NULL,
    // .driver = 0,
    // .dynids = 0
};
struct eduv7_priv {
    u8 __iomem *mem;
};
static u8* dma_databuf;


#endif