#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#define BUFSIZE 1024

#define READ_POSSIBLE (atomic_read(&buffer_filled_size) > 0)

static char buffer[BUFSIZE];
static atomic_t buffer_filled_size = ATOMIC_INIT(0);
static wait_queue_head_t wait_queue_for_read;

static dev_t modcontrol_dev_number;
static struct cdev *driver_object;
static struct class *modcontrol_class;
static struct device *modcontrol_dev;


static int driver_open(struct inode *geraete_datei, struct file *instanz){
	dev_info(modcontrol_dev, "driver_open called\n");
	return 0;
}

static int driver_close(struct inode *geraete_datei, struct file *instanz){
	dev_info(modcontrol_dev, "driver_close called\n");
	return 0;
}

static ssize_t driver_read(struct file *instanz, char __user *user, size_t count, loff_t *offset){
	dev_info(modcontrol_dev, "driver_read called\n");
	if(wait_event_interruptible(wait_queue_for_read, READ_POSSIBLE)){
		return -ERESTARTSYS;
	}
	else {	
		unsigned long local_buffer_filled_size = atomic_read(&buffer_filled_size);
		unsigned long to_copy = min(local_buffer_filled_size, count);
		unsigned long act_copied = to_copy - copy_to_user(user, &buffer[local_buffer_filled_size-to_copy], to_copy);
		atomic_sub(act_copied, &buffer_filled_size);
		dev_info(modcontrol_dev, "driver_read: user bufsize %zu, tryed cpy %lu, cpyd %lu\n", count, to_copy, act_copied);
		return act_copied;
	}
}

static ssize_t driver_write(struct file *instance, const char __user *user, size_t count, loff_t *offset){
	unsigned long local_buffer_filled_size = atomic_read(&buffer_filled_size);
	unsigned long to_copy = min(BUFSIZE - local_buffer_filled_size, count);
	unsigned long act_copied = to_copy - copy_from_user(&buffer[local_buffer_filled_size], user, to_copy);
	atomic_add(act_copied, &buffer_filled_size);
	dev_info(modcontrol_dev, "driver_write: user bufsize %zu, tryed cpy %lu, cpyd %lu\n", count, to_copy, act_copied);
	wake_up_interruptible(&wait_queue_for_read);
	return act_copied;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.read= driver_read,
	.write= driver_write,
	.open= driver_open,
	.release= driver_close,
};

static int __init mod_init(void){
	if (alloc_chrdev_region(&modcontrol_dev_number,0,1,"modcontrol")<0)
		return -EIO;
	driver_object = cdev_alloc(); /* Anmeldeobjekt reservieren */ 
	if (driver_object==NULL)
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if (cdev_add(driver_object,modcontrol_dev_number,1))
		goto free_cdev;
	/* Eintrag im Sysfs, damit Udev den Geraetedateieintrag erzeugt. */
	modcontrol_class = class_create(THIS_MODULE, "modcontrol");
	if (IS_ERR(modcontrol_class)) {
		pr_err("modcontrol: no udev support\n");
		goto free_cdev;
	}
	modcontrol_dev = device_create(modcontrol_class, NULL, modcontrol_dev_number, NULL, "%s", "modcontrol");
	if (IS_ERR(modcontrol_dev)) {
		pr_err("modcontrol: device_create failed\n");
		goto free_class;
	}
	// initialisiere queue
	init_waitqueue_head(&wait_queue_for_read);
	return 0;

free_class:
	class_destroy(modcontrol_class);
free_cdev:
	kobject_put(&driver_object->kobj);
free_device_number:
	unregister_chrdev_region(modcontrol_dev_number, 1);
	return -EIO;
}

static void __exit mod_exit(void){
	/* Loeschen des Sysfs-Eintrags und damit der Geraetedatei */
	device_destroy(modcontrol_class, modcontrol_dev_number);
	class_destroy(modcontrol_class);
	/* Abmelden des Treibers */
	cdev_del(driver_object);
	unregister_chrdev_region(modcontrol_dev_number, 1);
}

module_init(mod_init);
module_exit(mod_exit);
/* Metainformation */
MODULE_AUTHOR("colonel kernel");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A virtual device, which returns modcontrol.");





