#include <linux/module.h>
#include <linux/init.h>
#include <linux/tty.h>          
#include <linux/kd.h>           
#include <linux/vt.h>
#include <linux/console_struct.h>       
#include <linux/vt_kern.h>
#include <linux/timer.h>
#include <linux/fs.h>           
#include <linux/kobject.h>      
#include <linux/sysfs.h>        

#define BLINK_DELAY   HZ/5
#define ALL_LEDS_ON   7
#define RESTORE_LEDS  0

MODULE_AUTHOR("Snuffy");
MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs.");
MODULE_LICENSE("GPL");

static struct timer_list my_timer;
static struct tty_driver *my_driver;
static int ledStatus = RESTORE_LEDS;
static int test = ALL_LEDS_ON;

static struct kobject *kbleds_kobj;

static void my_timer_func(struct timer_list *ptr)
{
    if (ledStatus )
        ledStatus  = RESTORE_LEDS;
    else {
        if(test & 1) ledStatus  |= 1;
        if(test & 2) ledStatus  |= 4;
        if(test & 4) ledStatus  |= 2;
    }

    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, ledStatus);
    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);
}

static ssize_t kbleds_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", test);
}
static ssize_t kbleds_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int temp;
    sscanf(buf, "%du", &temp);
    test = temp;
    return count;
}

static struct kobj_attribute kbleds_attribute = __ATTR(test, 0660, kbleds_show, kbleds_store);

static int __init kbleds_init(void)
{
    int i;
    int retval;

    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
    for (i = 0; i < MAX_NR_CONSOLES; i++) {
        if (!vc_cons[i].d)
            break;
        printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
               MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
               (unsigned long)vc_cons[i].d->port.tty);
    }
    printk(KERN_INFO "kbleds: finished scanning consoles\n");
    my_driver = vc_cons[fg_console].d->port.tty->driver;
    printk(KERN_INFO "kbleds: tty driver magic %s\n", my_driver->name);

    timer_setup(&my_timer, my_timer_func, 0);
    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);

    kbleds_kobj = kobject_create_and_add("kbleds", kernel_kobj);
    if (!kbleds_kobj)
        return -ENOMEM;

    retval = sysfs_create_file(kbleds_kobj, &kbleds_attribute.attr);
    if (retval)
        kobject_put(kbleds_kobj);

    return retval;
}

static void __exit kbleds_cleanup(void)
{
    printk(KERN_INFO "kbleds: unloading...\n");
    del_timer(&my_timer);
    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);


    kobject_put(kbleds_kobj);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);