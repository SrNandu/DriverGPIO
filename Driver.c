#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#define DRIVER_NAME "GPIODriver"
#define DRIVER_CLASS "GPIODriverClass"
#define PIN_COUNT 28

static dev_t deviceFirstNumber;      // Global variable for the first device number
static struct cdev c_dev[PIN_COUNT]; // Global variable for the character devices structure
static struct class *class;          // Global variable for the device class

static int open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "GPIODriver open()\n");
    return 0;
}
static int close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "GPIODriver close()\n");
    return 0;
}

static ssize_t read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    ssize_t bytes;
    size_t gpio;
    char value;

    // Get gpio pin number
    gpio = iminor(f->f_path.dentry->d_inode);

    printk(KERN_INFO "GPIODriver: GPIO%zu read()\n", gpio);

    for (bytes = 0; bytes < len; ++bytes)
    {
        value = '0' + gpio_get_value(gpio);
        if (put_user(value, buf + bytes))
        {
            break;
        }
    }

    return bytes;
}

static ssize_t write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "GPIODriver: write()\n");

    return 0;
}

static struct file_operations pugs_fops =
    {
        .owner = THIS_MODULE,
        .open = open,
        .release = close,
        .read = read,
        .write = write};

static int __init initDriver(void)
{
    int ret = 0;

    printk(KERN_INFO "GPIODriver: Registered succesfully..!!\n");

    // Create device number
    if (alloc_chrdev_region(&deviceFirstNumber, 0, PIN_COUNT, DRIVER_NAME) < 0)
    {
        printk(KERN_WARNING "GPIODriver: Device number could not be allocated\n");
        return -1;
    }

    // Create device class
    if (IS_ERR(class = class_create(THIS_MODULE, DRIVER_CLASS)))
    {
        printk(KERN_WARNING "GPIODriver: Device class can not be created\n");
        unregister_chrdev_region(deviceFirstNumber, PIN_COUNT);
        return PTR_ERR(class);
    }

    // Create for each pin
    for (size_t i = 0; i < PIN_COUNT; i++)
    {
        if (gpio_request_one(i, GPIOF_IN, NULL) < 0)
        {
            printk(KERN_WARNING "GPIODriver: Can not allocate GPIO %zu\n", i);

            // Free previous GPIO
            for (size_t j = i - 1; i < 0; i--)
            {
                gpio_free(j);
            }

            return -1;
        }

        // Register device
        cdev_init(&c_dev[i], &pugs_fops);

        if ((ret = cdev_add(&c_dev[i], (deviceFirstNumber + i), 1)))
        {
            printk(KERN_WARNING "GPIODriver: Error %d adding cdev\n", ret);

            // Destroy previous devices
            for (size_t j = i - 1; i < 0; i--)
            {
                device_destroy(class, MKDEV(MAJOR(deviceFirstNumber),
                                            MINOR(deviceFirstNumber) + j));
            }
            class_destroy(class);
            unregister_chrdev_region(deviceFirstNumber, PIN_COUNT);
            return ret;
        }

        // Create device
        if (device_create(class,
                          NULL,
                          MKDEV(MAJOR(deviceFirstNumber), MINOR(deviceFirstNumber) + i),
                          NULL,
                          "GPIO%zu",
                          i) == NULL)
        {
            printk(KERN_WARNING "GPIODriver: Error %d creating device\n", ret);

            // Destroy previous devices
            for (size_t j = i - 1; i < 0; i--)
            {
                device_destroy(class, MKDEV(MAJOR(deviceFirstNumber),
                                            MINOR(deviceFirstNumber) + j));
            }
            class_destroy(class);
            unregister_chrdev_region(deviceFirstNumber, PIN_COUNT);
            return -1;
        }
    }

    return 0;
}

static void __exit exitDriver(void)
{
    for (size_t i = 0; i < PIN_COUNT; i++)
    {
        gpio_direction_output(i, 0);
        cdev_del(&c_dev[i]);
        device_destroy(class, MKDEV(MAJOR(deviceFirstNumber), MINOR(deviceFirstNumber) + i));
        gpio_free(i);
    }

    class_destroy(class);
    unregister_chrdev_region(deviceFirstNumber, PIN_COUNT);

    printk(KERN_INFO "GPIODriver: Unregistered..!!\n");
}

module_init(initDriver);
module_exit(exitDriver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paulo Carteras");
MODULE_DESCRIPTION("GPIO Driver");