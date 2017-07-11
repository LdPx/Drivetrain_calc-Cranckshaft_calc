#include <linux/module.h>

int init_module(){
	printk("init_module called\n");
	return 0;
}

void cleanup_module(){
	printk("cleanup_module called\n");
}
