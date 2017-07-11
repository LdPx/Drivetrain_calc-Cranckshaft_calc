
#include <linux/module.h>
#include <linux/init.h>

static int __init mod_init(void){
	printk("mod_init called\n");
	return 0;
}

static void __exit mod_exit(void){
	printk("mod_exit called\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("FOO MCBAR");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("abcde");
MODULE_SUPPORTED_DEVICE("rtfm");
