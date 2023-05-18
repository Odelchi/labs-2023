#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/kthread.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Odelchi");
MODULE_DESCRIPTION("A counter reader device");
MODULE_VERSION("0.0.3");

#define DEVICE_NAME "odelchi_counter"

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations counter_fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

enum {
    CDEV_NOT_USED = 0,
    CDEV_EXCLUSIVE_OPEN = 1,
};

static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);

static int Major;

static struct class *cls;

static int counter = 0;
static int is_read = 0;

struct task_struct *ts;

static int counter_thread(void *data) {
	while (!kthread_should_stop()) {
		counter++;
		is_read = 0;
		msleep(5000);
	}
	return 0;
}

static int __init counter_init(void) {
	printk(KERN_INFO "Counter: Initializing the counter reader device.\n");
	Major = register_chrdev(0, DEVICE_NAME, &counter_fops);
	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}
	printk(KERN_INFO "I was assigned major number %d.", Major);

	cls = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);

	ts = kthread_run(counter_thread, NULL, "counter_thread");
	return 0;
}

static void __exit counter_finished(void) {
	kthread_stop(ts);
	device_destroy(cls, MKDEV(Major, 0));
	class_destroy(cls);
	unregister_chrdev(Major, "odelchi_counter");
	printk(KERN_INFO "Counter: i am finished.\n");
}

static int device_open(struct inode *inode, struct file *file) {
	if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) {
		return -EBUSY;
	}

	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *file) {
	atomic_set(&already_open, CDEV_NOT_USED);
	module_put(THIS_MODULE);
	is_read = 0;
	return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
	// Output the counter_reader value to the user.
	int bytes_read = 0;
	char buf[32];
	int len = sprintf(buf, "%d\n", counter);
	if (is_read) {
		return 0;
	}
	if (len > length) {
		return -EINVAL;
	}
	if (copy_to_user(buffer, buf, len)) {
		return -EINVAL;
	}
	bytes_read = len;
	printk("Counter: Read %d bytes, counter_reader = %d", bytes_read, counter);
	is_read = 1;
	return bytes_read;
}

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset) {
	// Input the counter_reader value from the user.
	char buf[32];
	int len = length;
	if (len > sizeof(buf)) {
		len = sizeof(buf);
	}
	if (copy_from_user(buf, buffer, len)) {
		return -EINVAL;
	}
	buf[len] = 0;
	if (sscanf(buf, "%d", &counter) != 1) {
		return -EINVAL;
	}
	return len;
}

module_init(counter_init);
module_exit(counter_finished);
