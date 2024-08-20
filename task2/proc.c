#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define BUFFER_SIZE 1024
#define PROC_NAME "oleg"

MODULE_LICENSE("NXWE");
MODULE_AUTHOR("Snuffy");
MODULE_DESCRIPTION("A copy paste /proc module");

static char *msg;
static int len = 0;
static int temp = 0;

ssize_t read_proc(struct file *file, char __user *buf, size_t count, loff_t *offset) {
    if (*offset > 0) {
        return 0;
    }

    if (count > temp) {
        count = temp;
    }

    if (copy_to_user(buf, msg, count)) {
        return -EFAULT;
    }

    temp -= count;
    if (temp == 0) {
        temp = len;
    }

    *offset += count;

    return count;
}

ssize_t write_proc(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    if (count > BUFFER_SIZE - 1) {
        count = BUFFER_SIZE - 1;
    }

    if (copy_from_user(msg, buf, count)) {
        return -EFAULT;
    }

    msg[count] = '\0';

    len = count;
    temp = len;

    return count;
}

static const struct proc_ops proc_fops = {
    .proc_read = read_proc,
    .proc_write = write_proc,
};

static int __init proc_init(void) {
    msg = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!msg) {
        return -ENOMEM;
    }

    if (!proc_create(PROC_NAME, 0666, NULL, &proc_fops)) {
        kfree(msg);
        return -ENOMEM;
    }

    printk(KERN_INFO "%s loaded :)\n", PROC_NAME);

    return 0;
}

static void __exit proc_cleanup(void) {
    remove_proc_entry(PROC_NAME, NULL);

    kfree(msg);
    printk(KERN_INFO "%s unloaded >:(\n", PROC_NAME);
}

module_init(proc_init);
module_exit(proc_cleanup);