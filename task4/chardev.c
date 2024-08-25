#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Snuffy");
MODULE_DESCRIPTION("Copy/Paste char device with read/write support");

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

static int major;
static char msg[BUF_LEN];
static atomic_t already_open = ATOMIC_INIT(0);
static struct class *cls;

static struct file_operations chardev_fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

static int __init chardev_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops);

    if (major < 0) {
        pr_alert("Registering char device failed with %d\n", major);
        return major;
    }
    pr_info("I was assigned major number %d.\n", major);

    cls = class_create(DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    pr_info("Device created on /dev/%s\n", DEVICE_NAME);
    return SUCCESS;
}

static void __exit chardev_exit(void) {
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);

    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Device unregistered and cleaned up\n");
}

static int device_open(struct inode *inode, struct file *file) {
    if (atomic_cmpxchg(&already_open, 0, 1)) {
        return -EBUSY;
    }

    try_module_get(THIS_MODULE);
    pr_info("Device opened\n");
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
    atomic_set(&already_open, 0);
    module_put(THIS_MODULE);
    pr_info("Device closed\n");
    return SUCCESS;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
    int bytes_read = 0;
    const char *msg_ptr = msg;

    if (!*(msg_ptr + *offset)) {
        *offset = 0;
        return 0;
    }

    msg_ptr += *offset;

    while (length && *msg_ptr) {
        if (put_user(*(msg_ptr++), buffer++)) {
            return -EFAULT;
        }
        length--;
        bytes_read++;
    }

    *offset += bytes_read;
    pr_info("Device read, %d bytes\n", bytes_read);
    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off) {
    int bytes_to_copy = len < BUF_LEN ? len : BUF_LEN - 1;

    if (copy_from_user(msg, buff, bytes_to_copy)) {
        return -EFAULT;
    }
    msg[bytes_to_copy] = '\0';

    pr_info("Received from user: %s\n", msg);
    return bytes_to_copy;
}

module_init(chardev_init);
module_exit(chardev_exit);