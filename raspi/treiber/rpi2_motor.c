#include <linux/module.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

//static const int ML1 = 23;
//static const int ML2 = 24;
static const int ML1 = 24;
static const int ML2 = 23;
static const int MR1 = 25;
static const int MR2 = 8;

static dev_t gpio_dev_number;
static struct cdev *driver_object;
static struct class *gpio_class;
static struct device *motorl_dev, *motorr_dev;


struct instance_data {
	int gpio_1, gpio_2;
};

static int driver_open(struct inode *geraetedatei, struct file *instanz){
	int err1, err2;
	struct instance_data *id = kmalloc(sizeof(struct instance_data), GFP_USER);
	if (id==NULL) {
		printk("kmalloc failed\n");
		return -EIO;
	}
	instanz->private_data = (struct instance_data *) id;
	if (iminor(geraetedatei)==0) { // motor-left
		id->gpio_1 = ML1;
		id->gpio_2 = ML2;
	} else { // motor-right
		id->gpio_1 = MR1;
		id->gpio_2 = MR2;
	}
	printk("current gpio1: %d, gpio2: %d\n", id->gpio_1, id->gpio_2);

	// Initialisieren der GPIOs
	err1 = gpio_request(id->gpio_1, "rpi-­gpio-1");
	err2 = gpio_request(id->gpio_2, "rpi-­gpio-2");
	if (err1 || err2) {
		printk("gpio_request error: gpio1: %d, gpio2: %d\n", err1, err2);
		return -EIO;
	}

	// setze als Ausgabe-GPIO
	err1 = gpio_direction_output(id->gpio_1, 0);
	err2 = gpio_direction_output(id->gpio_2, 0);
	if (err1 || err2) {
		printk("gpio_output error: gpio1: %d, gpio2: %d\n", err1, err2);
		gpio_free(id->gpio_1);
		gpio_free(id->gpio_2);
		return -EIO;
	}

	printk("gpio %d and %d successafully configured\n", id->gpio_1, id->gpio_2);
	return 0;
}

static int driver_close(struct inode *geraete_datei, struct file *instanz){
	struct instance_data *id=(struct instance_data *)instanz->private_data;
	dev_info(motorl_dev, "driver_close called\n");
	if  (id!=NULL) {
		gpio_free(id->gpio_1);
		gpio_free(id->gpio_2);
		kfree(id);
	}
	return 0;
}

static ssize_t driver_write(struct file *instanz, const char __user *user, size_t count, loff_t *offset){	
	int value = 0;
	unsigned long to_copy = min(count, sizeof(value));	
	unsigned long act_copied = to_copy - copy_from_user(&value, user, to_copy);
	struct instance_data *id=(struct instance_data *)instanz->private_data;
	//dev_info(motorl_dev, "driver_write called: value %d, tryed cpy %lu, cpyed %lu\n", value, to_copy, act_copied);

	gpio_set_value(id->gpio_1, value > 0 ? 1 : 0);
	gpio_set_value(id->gpio_2, value < 0 ? 1 : 0);

	return act_copied;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.write= driver_write,
	.open= driver_open, 
	.release= driver_close,
};

static int __init mod_init(void){
	if(alloc_chrdev_region(&gpio_dev_number,0,2,"motor")<0)
		return -EIO;
	driver_object = cdev_alloc(); /* Anmeldeobjekt reservieren */
	if(driver_object==NULL)
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if(cdev_add(driver_object,gpio_dev_number,2))
		goto free_cdev;
	/* Eintrag im Sysfs, damit Udev den Geraetedateieintrag erzeugt. */
	gpio_class = class_create(THIS_MODULE, "motor");
	if(IS_ERR(gpio_class)) {
		pr_err("gpio: no udev support\n");
		goto free_cdev;
	}
	motorl_dev = device_create(gpio_class, NULL, gpio_dev_number, NULL, "%s", "motor-left");
	motorr_dev = device_create(gpio_class, NULL, gpio_dev_number+1, NULL, "%s", "motor-right");
	dev_info(motorl_dev, "mod_init");
	return 0;
free_cdev:
	kobject_put(&driver_object->kobj);
free_device_number:
	unregister_chrdev_region(gpio_dev_number, 2);
	return -EIO;
}

static void __exit mod_exit(void){
	dev_info(motorl_dev, "mod_exit");
	/* Loeschen des Sysfs-Eintrags und damit der Geraetedatei */
	device_destroy(gpio_class, gpio_dev_number+1);
	device_destroy(gpio_class, gpio_dev_number);
	class_destroy(gpio_class);
	/* Abmelden des Treibers */
	cdev_del(driver_object);
	unregister_chrdev_region(gpio_dev_number, 2);
	return;
}

module_init(mod_init);
module_exit(mod_exit);

/* Metainformation */
MODULE_LICENSE("GPL");
