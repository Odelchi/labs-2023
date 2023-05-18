// Символьный PCI драйвер, который считыват MAC-адрес с сетевой карты:
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Odelchi");
MODULE_DESCRIPTION("PCI driver");
MODULE_VERSION("0.0.1");

#define VENDOR_ID 0x10ec
#define DEVICE_ID 0x8168

#define DRIVER_NAME "network_card_pci_driver_mac"

#define CTL_GET_MAC _IOR('mac', 1, char *)

static struct pci_device_id network_card_pci_driver_mac_id_table[] = {
        { PCI_DEVICE(VENDOR_ID, DEVICE_ID) },
        { 0 }
};

MODULE_DEVICE_TABLE(pci, network_card_pci_driver_mac_id_table);

static int network_card_pci_driver_mac_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void network_card_pci_driver_mac_remove(struct pci_dev *dev);

static struct pci_driver network_card_pci_driver_mac = {
        .name = DRIVER_NAME,
        .id_table = network_card_pci_driver_mac_id_table,
        .probe = network_card_pci_driver_mac_probe,
        .remove = network_card_pci_driver_mac_remove
};

static int network_card_open(struct inode *inode, struct file *file);
static int network_card_release(struct inode *inode, struct file *file);
static ssize_t network_card_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations network_card_fops = {
        .owner = THIS_MODULE,
        .open = network_card_open,
        .release = network_card_release,
        .unlocked_ioctl = network_card_ioctl
};

static int Major;
static struct class *network_card_class;

unsigned char mac_addr[6];

static atomic_t already_open = ATOMIC_INIT(0);

static int __init network_card_pci_driver_mac_init(void) {
    int register_result = pci_register_driver(&network_card_pci_driver_mac);
    if (register_result < 0) {
        printk(KERN_ERR
        "pci_register_driver didnt work\n");
        return register_result;
    }

    Major = register_chrdev(0, DRIVER_NAME, &network_card_fops);
    if (Major < 0) {
        printk(KERN_ERR
        "register_chrdev didnt work\n");
        return Major;
    }

    network_card_class = class_create(THIS_MODULE, DRIVER_NAME);
    if (!network_card_class) {
        printk(KERN_ERR
        "class_create didnt work\n");
        return -1;
    }

    device_create(network_card_class, NULL, MKDEV(Major, 0), NULL, DRIVER_NAME);

    return 0;
}

static void __exit network_card_pci_driver_mac_exit(void) {
    class_destroy(network_card_class);
    unregister_chrdev(Major, DRIVER_NAME);
    pci_unregister_driver(&network_card_pci_driver_mac);

    printk(KERN_INFO
    "Finished.\n");

    return;
}

static int network_card_pci_driver_mac_probe(struct pci_dev *dev, const struct pci_device_id *id) {
    u8 __iomem *mmio_base;

    int bar = 2;

    if (pci_resource_flags(dev, bar) & IORESOURCE_MEM) {
        mmio_base = pci_iomap(dev, bar, pci_resource_len(dev, bar));
        if (!mmio_base) {
            printk(KERN_ERR
            "pci_iomap didnt work\n");
            return -1;
        }
    } else {
        printk(KERN_ERR
        "Not a memory-mapped resource\n");
        return -1;
    }

    int i;
    for (i = 0; i < 6; i++) {
        mac_addr[i] = ioread8(mmio_base + i);
        printk(KERN_INFO
        "MAC address byte  %d: %02x\n", i, mmio_base[i]);
    }

    iounmap(mmio_base);

    return 0;
}

static void network_card_pci_driver_mac_remove(struct pci_dev *dev) {
    dev_t devno = MKDEV(Major, 0);
    device_destroy(network_card_class, devno)
}

static int network_card_open(struct inode *inode, struct file *file) {
    if (atomic_cmpxchg(&already_open, 0, 1)) {
        printk(KERN_ERR
        "already_open is true\n");
        return -EBUSY;
    }

    try_module_get(THIS_MODULE);
    return 0;
}

static int network_card_release(struct inode *inode, struct file *file) {
    atomic_set(&already_open, 0);
    module_put(THIS_MODULE);

    return 0;
}


static ssize_t network_card_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case CTL_GET_MAC:
            if (copy_to_user((unsigned long __user *) arg, mac_addr, 6) != 0) {
                printk(KERN_ERR
                "copy_to_user didnt work\n");
                return -EFAULT;
            }
            break;
        default:
            printk(KERN_ERR
            "unknown ioctl\n");
            return -EINVAL;
    }

    return 0;
}

module_init(network_card_pci_driver_mac_init);
module_exit(network_card_pci_driver_mac_exit);
