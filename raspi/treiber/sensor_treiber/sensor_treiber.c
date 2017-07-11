#include <linux/module.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#define USECS_PER_SEC 1000000
#define NSECS_PER_USEC 1000
#define C_M_PER_SEC 343

//static const int GPIO_TRIG = 20;
//static const int GPIO_ECHO = 21;
static const int GPIO_TRIG = 17;
static const int GPIO_ECHO = 27;


static dev_t gpio_dev_number;
static struct cdev *driver_object;
static struct class *gpio_class;
static struct device *sensor_dev;

static atomic_t driver_lock = ATOMIC_INIT(-1);

struct instance_data {
	int gpio_trig, gpio_echo;
};

static int driver_open(struct inode *geraetedatei, struct file *instanz){

	if(atomic_inc_and_test(&driver_lock)){
		dev_info(sensor_dev, "driver access permitted => device locked\n");
		int err1, err2;
		struct instance_data *id = kmalloc(sizeof(struct instance_data), GFP_USER);
		if (id == NULL) {
			printk("kmalloc failed\n");
			return -EIO;
		}
		instanz->private_data = (struct instance_data *) id;
		id->gpio_echo = GPIO_ECHO;
		id->gpio_trig = GPIO_TRIG;
		printk("current gpios: echo %d, trig: %d\n", id->gpio_echo, id->gpio_trig);

		// Initialisieren der GPIOs
		err1 = gpio_request(id->gpio_echo, "rpi-­gpio-echo");
		err2 = gpio_request(id->gpio_trig, "rpi-­gpio-trig");
		if (err1 || err2) {
			printk("gpio_request error: gpio_echo %d, gpio2_trig %d\n", err1, err2);
			return -EIO;
		}

		// setze als Ausgabe-GPIO
		err1 = gpio_direction_output(id->gpio_trig, 0);
		err2 = gpio_direction_input(id->gpio_echo);
		if (err1 || err2) {
			printk("gpio_request error: gpio_echo %d, gpio2_trig %d\n", err1, err2);
			gpio_free(id->gpio_trig);
			gpio_free(id->gpio_echo);
			return -EIO;
		}

		printk("gpio echo %d, trig %d successfully configured\n", id->gpio_echo, id->gpio_trig);
		return 0;
	}
	else {
		dev_info(sensor_dev, "driver access denied\n");
		atomic_dec(&driver_lock);
		return -EBUSY;
	}
}

static int driver_close(struct inode *geraete_datei, struct file *instanz){
	dev_info(sensor_dev, "driver_close called => device unlocked\n");
	atomic_dec(&driver_lock);
	struct instance_data *id=(struct instance_data *)instanz->private_data;
	if  (id!=NULL) {
		gpio_free(id->gpio_echo);
		gpio_free(id->gpio_trig);
		kfree(id);
	}
	return 0;
}

static ssize_t driver_read(struct file *instanz, char __user *user, size_t count, loff_t *offset){	
	const ktime_t WAIT_ECHO_TIMEOUT = ktime_set(0, 10000000);
	struct instance_data *id = (struct instance_data *)instanz->private_data;	
	//struct timespec trig_end, echo_start, echo_end, diff;

	ktime_t trig_end, echo_start, echo_end;
	struct timespec diff_ts;

	//dev_info(sensor_dev, "sensor: driver_read: starting sensor trigger\n");	
	//unsigned long start_sensor_delay = 10;
	//udelay(start_sensor_delay);
	gpio_set_value(id->gpio_trig, 1);
	//unsigned long min_start_sensor_delay = 10, max_start_sensor_delay = 20;
	//usleep_range(min_start_sensor_delay, max_start_sensor_delay);
	udelay(11);	
	gpio_set_value(id->gpio_trig, 0);

	//dev_info(sensor_dev, "sensor: driver_read: waiting for sensor echo\n");	
	trig_end = ktime_get();
	while(gpio_get_value(id->gpio_echo) == 0){
		ktime_t diff = ktime_sub(ktime_get(), trig_end);
		if(ktime_compare(diff, WAIT_ECHO_TIMEOUT) > 0){
			return -EIO;
		}
	}
	echo_start = ktime_get();
	//ktime_get_ts(&echo_start);
	while(gpio_get_value(id->gpio_echo) > 0);
	echo_end = ktime_get();	
	//dev_info(sensor_dev, "sensor: driver_read: finished sensor measure\n");

	ktime_t diff = ktime_sub(echo_end, echo_start);
	diff_ts = ktime_to_timespec(diff);

	//unsigned long echo_start_us = echo_start.tv_nsec/NSECS_PER_USEC + echo_start.tv_sec*USECS_PER_SEC;
	//unsigned long echo_end_us = echo_end.tv_nsec/NSECS_PER_USEC + echo_end.tv_sec*USECS_PER_SEC;
	unsigned long echo_delay_us = diff_ts.tv_nsec/NSECS_PER_USEC + diff_ts.tv_sec*USECS_PER_SEC;
	unsigned long distance_um = C_M_PER_SEC * echo_delay_us / 2;
	//dev_info(sensor_dev, "sensor: driver_read: delay[us] %lu, distance[um] %lu\n", echo_delay_us, distance_um);

	unsigned long to_copy = min(count, sizeof(distance_um));	
	unsigned long act_copied = to_copy - copy_to_user(user, &distance_um, to_copy);
	//dev_info(sensor_dev, "sensor: driver_read: value %lu, tryed cpy %lu, cpyed %lu\n", distance_um, to_copy, act_copied);
	return act_copied;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.read= driver_read,
	.open= driver_open, 
	.release= driver_close,
};

static int __init mod_init(void){
	//if(alloc_chrdev_region(&gpio_dev_number,0,2,"sensor")<0)
	if(alloc_chrdev_region(&gpio_dev_number,0,1,"sensor")<0)
		return -EIO;
	driver_object = cdev_alloc(); /* Anmeldeobjekt reservieren */
	if(driver_object==NULL)
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	//if(cdev_add(driver_object,gpio_dev_number,2))
	if(cdev_add(driver_object,gpio_dev_number,1))
		goto free_cdev;
	/* Eintrag im Sysfs, damit Udev den Geraetedateieintrag erzeugt. */
	gpio_class = class_create(THIS_MODULE, "sensor");
	if(IS_ERR(gpio_class)) {
		pr_err("gpio: no udev support\n");
		goto free_cdev;
	}
	sensor_dev = device_create(gpio_class, NULL, gpio_dev_number, NULL, "%s", "sensor");
	//motorr_dev = device_create(gpio_class, NULL, gpio_dev_number+1, NULL, "%s", "motor-right");
	dev_info(sensor_dev, "mod_init");
	return 0;
free_cdev:
	kobject_put(&driver_object->kobj);
free_device_number:
	//unregister_chrdev_region(gpio_dev_number, 2);
	unregister_chrdev_region(gpio_dev_number, 1);
	return -EIO;
}

static void __exit mod_exit(void){
	dev_info(sensor_dev, "mod_exit");
	/* Loeschen des Sysfs-Eintrags und damit der Geraetedatei */
	//device_destroy(gpio_class, gpio_dev_number+1);
	device_destroy(gpio_class, gpio_dev_number);
	class_destroy(gpio_class);
	/* Abmelden des Treibers */
	cdev_del(driver_object);
	//unregister_chrdev_region(gpio_dev_number, 2);
	unregister_chrdev_region(gpio_dev_number, 1);
	return;
}

module_init(mod_init);
module_exit(mod_exit);

/* Metainformation */
MODULE_LICENSE("GPL");
