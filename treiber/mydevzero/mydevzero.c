#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#define BUFSIZE 1024

static char zero[BUFSIZE] = {0};

static dev_t mydevzero_dev_number;
static struct cdev *driver_object;
static struct class *mydevzero_class;
static struct device *mydevzero_dev;


static int driver_open(struct inode *geraete_datei, struct file *instanz){
	dev_info(mydevzero_dev, "driver_open called\n");
	return 0;
}

static int driver_close(struct inode *geraete_datei, struct file *instanz){
	dev_info(mydevzero_dev, "driver_close called\n");
	return 0;
}

static ssize_t driver_read(struct file *instanz, char *user, size_t count, loff_t *offset){
	unsigned long not_copied = 0;
	size_t buf_offset; 
	dev_info(mydevzero_dev, "driver_read with count %zu\n", count);
	for(buf_offset = 0; buf_offset < count; buf_offset += BUFSIZE){
		size_t to_copy = min((size_t)BUFSIZE, count - buf_offset);
		not_copied += copy_to_user(user + buf_offset, zero, to_copy);
		dev_info(mydevzero_dev, "driver_read copied %zu bytes (offset %zu)\n", to_copy, buf_offset);
	}
	dev_info(mydevzero_dev, "driver_read not copied %zu bytes\n", not_copied);
	return count - not_copied;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.read= driver_read,
	.open= driver_open,
	.release= driver_close,
};

static int __init mod_init(void){
	if (alloc_chrdev_region(&mydevzero_dev_number,0,1,"mydevzero")<0)
		return -EIO;
	driver_object = cdev_alloc(); /* Anmeldeobjekt reservieren */ 
	if (driver_object==NULL)
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if (cdev_add(driver_object,mydevzero_dev_number,1))
		goto free_cdev;
	/* Eintrag im Sysfs, damit Udev den Geraetedateieintrag erzeugt. */
	mydevzero_class = class_create(THIS_MODULE, "mydevzero");
	if (IS_ERR(mydevzero_class)) {
		pr_err("mydevzero: no udev support\n");
		goto free_cdev;
	}
	mydevzero_dev = device_create(mydevzero_class, NULL, mydevzero_dev_number, NULL, "%s",  "mydevzero");
	if (IS_ERR(mydevzero_dev)) {
		pr_err("mydevzero: device_create failed\n");
		goto free_class;
	}
	return 0;

free_class:
	class_destroy(mydevzero_class);
free_cdev:
	kobject_put(&driver_object->kobj);
free_device_number:
	unregister_chrdev_region(mydevzero_dev_number, 1);
	return -EIO;
}

static void __exit mod_exit(void){
	/* Loeschen des Sysfs-Eintrags und damit der Geraetedatei */
	device_destroy(mydevzero_class, mydevzero_dev_number);
	class_destroy(mydevzero_class);
	/* Abmelden des Treibers */
	cdev_del(driver_object);
	unregister_chrdev_region(mydevzero_dev_number, 1);
}

module_init(mod_init);
module_exit(mod_exit);
/* Metainformation */
MODULE_AUTHOR("colonel kernel");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A zero-device-clone");





