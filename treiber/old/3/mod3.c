#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static char hello_world[] = "hello world\n";

static dev_t hello_dev_number;

static struct cdev *driver_object;
static struct class *hello_class;
static struct device *hello_dev;

static int driver_open(struct inode *geraete_datei, struct file *instanz){
	dev_info(hello_dev, "driver_open called\n");
	return 0;
}

static int driver_close(struct inode *geraete_datei, struct file *instanz){
	dev_info(hello_dev, "driver_close called\n");
	return 0;	
}

static ssize_t driver_read(struct file *instanz, char __user *user, size_t count, loff_t *offset){
	unsigned long not_copied, to_copy;
	//return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = driver_read,
	.open = driver_open,
	.release = driver_close
};

static int __init mod_init(void){
	if(alloc_chrdev_region(&hello_dev_number, 0, 1, "Hello") < 0)
		return -EIO; 
	driver_object = cdev_alloc();
	if(driver_object == NULL)
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if(cdev_add(driver_object, hello_dev_number, 1))
		goto free_cdev;
	hello_class = class_create(THIS_MODULE, "Hello");
	if(IS_ERR(hello_class))
		pr_err("hello: no udev supoort\n");
		goto free_cdev;
	hello_dev = device_create(hello_class, NULL, hello_dev_number, NULL, "%s", "hello");
	if(IS_ERR(hello_dev)){
		pr_err("hello: device_create failed\n");
		goto free_class;
	}
	return 0;

free_class:
	class_destroy(hello_class);
free_cdev:
	kobject_put(&driver_object->kobj);
free_device_number:
	unregister_chrdev_region(hello_dev_number, 1);
	return -EIO;
}

static void __exit mod_exit(void){
	device_destroy(hello_class, hello_dev_number);
	class_destroy(hello_class);
	cdev_del(driver_object);
	unregister_chrdev_region(hello_dev_number, 1);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("FOO MCBAR");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("abcde");
MODULE_SUPPORTED_DEVICE("virtual dev");

