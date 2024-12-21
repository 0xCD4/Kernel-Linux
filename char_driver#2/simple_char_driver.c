#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mydev"
#define MAX_SIZE 100

static int major_num;
static char message[MAX_SIZE];
static int message_len = 0;

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset)
{
    size_t bytes_to_copy = min(length, (size_t)MAX_SIZE);
    if (copy_from_user(message, buffer, bytes_to_copy))
        return -EFAULT;
    message_len = bytes_to_copy;
    *offset += bytes_to_copy;
    return bytes_to_copy;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
    // Check offset condition
    if (*offset >= message_len)
        return 0;

    // Calculate bytes to copy
    int bytes_to_copy = min(length, (size_t)(message_len - *offset));
    
    // --- Start of core read operations ---
    // 1. Use copy_to_user to transfer data
    if (copy_to_user(buffer, message + *offset, bytes_to_copy))
        return -EFAULT;

    // 2. Update offset position
    *offset += bytes_to_copy;

    // 3. Return number of bytes copied
    return bytes_to_copy;
    // --- End of core read operations ---
}

static struct file_operations fops = {
    .write = device_write,
    .read = device_read,
};

static int __init char_driver_init(void)
{
    // Register device and call register_chrdev()
    major_num = register_chrdev(0, DEVICE_NAME, &fops);

    // Check for error
    if (major_num < 0) {
        // Print error
        printk(KERN_ALERT "Failed to register device\n");
        return major_num;
    }

    // Print major number on success
    printk(KERN_INFO "MyDev module loaded with major number %d\n", major_num);
    return 0;
}

static void __exit char_driver_exit(void)
{
    // Unregister device in char_driver_exit
    
    // Call unregister_chrdev()
    unregister_chrdev(major_num, DEVICE_NAME);
    
    // Print module unload message
    printk(KERN_INFO "MyDev module unloaded\n");
}

module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("shadowintel");
MODULE_DESCRIPTION("A simple character driver"); 