#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/pci_regs.h>
#include "driver.h"
#include "e7cdev.h"

MODULE_LICENSE("GPL");



/**
 * @brief
 * Once the driver knows about a PCI device and takes ownership, the driver
 generally needs to perform the following initialization:
 *       - Enable the device
 *       - Request MMIO/IOP resources
 *       [DMA]Set the DMA mask size (for both coherent and streaming DMA)
 *       - Allocate and initialize shared control data (pci_allocate_coherent())
 *       - Access device configuration space (if needed)
 *       [IRQ]Register IRQ handler (request_irq())
 *       [nop]Initialize non-PCI (i.e. LAN/SCSI/etc parts of the chip)
 *       [DMA]Enable DMA/processing engines
 * @return int
 */
static int __init eduv7_init(void) {
        int rc = -1;
        
        rc = pci_register_driver(&eduv7_driver);
        if(rc){
                printk(KERN_WARNING "Eduv7 driver could not register with device");
                return rc;
        }
        eduv7cdev_init();
        printk(KERN_DEBUG "[Eduv7] Init succesfull");

        return 0;
}

static void __exit eduv7_exit(void) {
        pci_unregister_driver(&eduv7_driver);
        eduv7cdev_destroy();
        printk(KERN_DEBUG "[Eduv7] Exit succesfull");
}

module_init(eduv7_init);
module_exit(eduv7_exit);
