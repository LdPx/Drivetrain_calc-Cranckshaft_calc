#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static const char *driver_name = "hello";
static char hello_world[] = "Hello World\n";
static dev_t hello_dev_number;
static struct cdev *driver_object;
static struct class *hello_class;
static struct device *hello_dev;

static atomic_t driver_lock = ATOMIC_INIT(-1);


static int driver_open(struct inode *geraete_datei, struct file *instanz){
	dev_info(hello_dev, "driver_open called\n");
	if(atomic_inc_and_test(&driver_lock)){
		dev_info(hello_dev, "driver access permitted => device locked\n");
		return 0;
	}
	else {
		dev_info(hello_dev, "driver access denied\n");
		atomic_dec(&driver_lock);
		return -EBUSY;
	}
}

static int driver_close(struct inode *geraete_datei, struct file *instanz){
	dev_info(hello_dev, "driver_close called => device unlocked\n");
	atomic_dec(&driver_lock);
	return 0;
}

static ssize_t driver_read(struct file *instanz, char __user *user, size_t count, loff_t *offset){
	unsigned long to_copy = min(strlen(hello_world)+1, count);
	unsigned long not_copied = copy_to_user(user, hello_world, to_copy);
	return to_copy - not_copied;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.read= driver_read,
	.open= driver_open,
	.release= driver_close,
};

static int __init mod_init(void){
	if (alloc_chrdev_region(&hello_dev_number,0,1,"Hello")<0)
		return -EIO;
	driver_object = cdev_alloc(); /* Anmeldeobjekt reservieren */ 
	if (driver_object==NULL)
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if (cdev_add(driver_object,hello_dev_number,1))
		goto free_cdev;
	/* Eintrag im Sysfs, damit Udev den Geraetedateieintrag erzeugt. */
	hello_class = class_create(THIS_MODULE, "Hello");
	if (IS_ERR(hello_class)) {
		pr_err("hello: no udev support\n");
		goto free_cdev;
	}
	hello_dev = device_create(hello_class, NULL, hello_dev_number, NULL, "%s",  driver_name);
	if (IS_ERR(hello_dev)) {
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
	/* Loeschen des Sysfs-Eintrags und damit der Geraetedatei */
	device_destroy(hello_class, hello_dev_number);
	class_destroy(hello_class);
	/* Abmelden des Treibers */
	cdev_del(driver_object);
	unregister_chrdev_region(hello_dev_number, 1);
}

module_init(mod_init);
module_exit(mod_exit);
/* Metainformation */
MODULE_AUTHOR("colonel kernel");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A virtual device, which returns hello.");





