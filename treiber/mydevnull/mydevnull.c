#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static dev_t mydevnull_dev_number;
static struct cdev *driver_object;
static struct class *mydevnull_class;
static struct device *mydevnull_dev;


static int driver_open(struct inode *geraete_datei, struct file *instanz){
	dev_info(mydevnull_dev, "driver_open called\n");
	return 0;
}

static int driver_close(struct inode *geraete_datei, struct file *instanz){
	dev_info(mydevnull_dev, "driver_close called\n");
	return 0;
}

static ssize_t driver_write(struct file *instanz, const char *user, size_t count, loff_t *offs){
	dev_info(mydevnull_dev, "driver_write called with input %s\n", user);
	return count;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.write= driver_write,
	.open= driver_open,
	.release= driver_close,
};

static int __init mod_init(void){
	if (alloc_chrdev_region(&mydevnull_dev_number,0,1,"Mydevnull")<0)
		return -EIO;
	driver_object = cdev_alloc(); /* Anmeldeobjekt reservieren */ 
	if (driver_object==NULL)
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if (cdev_add(driver_object,mydevnull_dev_number,1))
		goto free_cdev;
	/* Eintrag im Sysfs, damit Udev den Geraetedateieintrag erzeugt. */
	mydevnull_class = class_create(THIS_MODULE, "Mydevnull");
	if (IS_ERR(mydevnull_class)) {
		pr_err("mydevnull: no udev support\n");
		goto free_cdev;
	}
	mydevnull_dev = device_create(mydevnull_class, NULL, mydevnull_dev_number, NULL, "%s",  "mydevnull");
	if (IS_ERR(mydevnull_dev)) {
		pr_err("mydevnull: device_create failed\n");
		goto free_class;
	}
	return 0;

free_class:
	class_destroy(mydevnull_class);
free_cdev:
	kobject_put(&driver_object->kobj);
free_device_number:
	unregister_chrdev_region(mydevnull_dev_number, 1);
	return -EIO;
}

static void __exit mod_exit(void){
	/* Loeschen des Sysfs-Eintrags und damit der Geraetedatei */
	device_destroy(mydevnull_class, mydevnull_dev_number);
	class_destroy(mydevnull_class);
	/* Abmelden des Treibers */
	cdev_del(driver_object);
	unregister_chrdev_region(mydevnull_dev_number, 1);
}

module_init(mod_init);
module_exit(mod_exit);
/* Metainformation */
MODULE_AUTHOR("colonel kernel");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A null-device-clone");





