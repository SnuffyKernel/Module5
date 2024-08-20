#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/init.h>

MODULE_LICENSE("NXWE");
MODULE_AUTHOR("Snuffy");
MODULE_DESCRIPTION("A copy paste Hello World module");

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello Snuffy!\n");
    return 0; 
}

static void __exit hello_cleanup(void)
{
    printk(KERN_INFO "Nope.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);