#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/device.h>


#define DEVICE_NAME "ioctl_dev"
#define CLASS_NAME "ioctl_class"

// IOCTL commands
#define WR_VALUE _IOW('a', 'a', int32_t*)
#define RD_VALUE _IOR('a', 'b', int32_t*)

static int major_num;
static int value = 0;
static struct class* ioctl_class = NULL;
static struct device* ioctl_device = NULL;

// Function prototypes
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);
static long device_ioctl(struct file*, unsigned int, unsigned long);

static struct file_operations fops = {
    .open = device_open,
    .read = device_read,
    .write = device_write,
    .release = device_release,
    .unlocked_ioctl = device_ioctl
};

// IOCTL operation
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
        case WR_VALUE:
            if (copy_from_user(&value, (int32_t*)arg, sizeof(value)))
                return -EFAULT;
            printk(KERN_INFO "ioctl_driver: Wrote value %d\n", value);
            break;
        case RD_VALUE:
            if (copy_to_user((int32_t*)arg, &value, sizeof(value)))
                return -EFAULT;
            printk(KERN_INFO "ioctl_driver: Read value %d\n", value);
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

// Init module
static int __init ioctl_driver_init(void)
{
    // Register device class
    ioctl_class = class_create(CLASS_NAME);
    if (IS_ERR(ioctl_class)) {
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(ioctl_class);
    }

    // Register character device and get major number
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        class_destroy(ioctl_class);
        return major_num;
    }

    // Register the device driver
    ioctl_device = device_create(ioctl_class, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ioctl_device)) {
        class_destroy(ioctl_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(ioctl_device);
    }

    printk(KERN_INFO "ioctl_driver: device class created correctly\n");
    return 0;
}

// Exit module
static void __exit ioctl_driver_exit(void)
{
    device_destroy(ioctl_class, MKDEV(major_num, 0));
    class_unregister(ioctl_class);
    class_destroy(ioctl_class);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "ioctl_driver: Goodbye from the LKM!\n");
}

// Device functions
static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "ioctl_driver: Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "ioctl_driver: Device closed\n");
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "ioctl_driver: Read operation\n");
    return 0;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "ioctl_driver: Write operation\n");
    return len;
}

module_init(ioctl_driver_init);
module_exit(ioctl_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple IOCTL driver"); 