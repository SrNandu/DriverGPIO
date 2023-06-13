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
#define DEVICE_COUNT 2

static dev_t deviceNumber;  // Global variable for the first device number
static struct cdev c_dev;   // Global variable for the character device structure
static struct class *class; // Global variable for the device class

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

// ssize_t resulta ser una palabra con signo.
// Por lo tanto, puede ocurrir que devuelva un número negativo. Esto sería un error.
// Pero un valor de retorno no negativo tiene un significado adicional.
// Para my_read sería el número de bytes leídos

// Cuando hago un $ cat /dev/SdeC_drv3, se convoca a my_read.!!
// my_read lee "len" bytes, los guarda en "buf" y devuelve la cantidad leida, que puede
// ser menor, pero nunca mayor que len.

// En SdeC_drv3, devuelve cero. Dado que es un archivo, esto significa no hay mas datos ó EOF.
// Lo que tendría que ocurrir es que el device escriba sobre buf para que el usuario pueda
// obtener una lectura no nula.

static ssize_t read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "SdeC_drv4: read()\n");

    return 0;
}

// my_write escribe "len" bytes en "buf" y devuelve la cantidad escrita, que debe ser igual "len".
// Cuando hago un $ echo "bla bla bla..." > /dev/SdeC_drv3, se convoca a my_write.!!

static ssize_t write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "SdeC_drv4: write()\n");

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
    struct device *dev_ret;

    printk(KERN_INFO "GPIODriver: Registered succesfully..!!\n");

    // Create 2 device number
    if (alloc_chrdev_region(&deviceNumber, 0, DEVICE_COUNT, DRIVER_NAME) < 0)
    {
        printk("Device number could not be allocated\n");
        return -1;
    }

    // Create device class
    if (IS_ERR(class = class_create(THIS_MODULE, DRIVER_CLASS)))
    {
        printk("Device class can not be created\n");
        unregister_chrdev_region(deviceNumber, DEVICE_COUNT);
        return PTR_ERR(class);
    }

    // Create device file
    if (IS_ERR(dev_ret = device_create(class, NULL, deviceNumber, NULL, DRIVER_NAME)))
    {
        printk("Registering of device to kernel failed\n");
        class_destroy(class);
        unregister_chrdev_region(deviceNumber, DEVICE_COUNT);
        return PTR_ERR(dev_ret);
    }

    // Init device file
    cdev_init(&c_dev, &pugs_fops);

    // Register device to kernel
    if (cdev_add(&c_dev, deviceNumber, DEVICE_COUNT) < 0)
    {
        device_destroy(class, deviceNumber);
        class_destroy(class);
        unregister_chrdev_region(deviceNumber, DEVICE_COUNT);
        return -1;
    }

    /* GPIO 1 init */
    if (gpio_request(0, "rpi-gpio-0"))
    {
        printk("Can not allocate GPIO 0\n");

        cdev_del(&c_dev);
        device_destroy(class, deviceNumber);
        class_destroy(class);
        unregister_chrdev_region(deviceNumber, DEVICE_COUNT);
        return -1;
    }

    /* Set GPIO 0 input */
    if (gpio_direction_input(0))
    {
        printk("Can not set GPIO 0 to input!\n");

        gpio_free(0);
        cdev_del(&c_dev);
        device_destroy(class, deviceNumber);
        class_destroy(class);
        unregister_chrdev_region(deviceNumber, DEVICE_COUNT);
        return -1;
    }

    /* GPIO 1 init */
    if (gpio_request(1, "rpi-gpio-1"))
    {
        printk("Can not allocate GPIO 1\n");

        gpio_free(0);
        cdev_del(&c_dev);
        device_destroy(class, deviceNumber);
        class_destroy(class);
        unregister_chrdev_region(deviceNumber, DEVICE_COUNT);
        return -1;
    }

    /* Set GPIO 1 input */
    if (gpio_direction_input(1))
    {
        printk("Can not set GPIO 1 to input!\n");

        gpio_free(0);
        gpio_free(1);
        cdev_del(&c_dev);
        device_destroy(class, deviceNumber);
        class_destroy(class);
        unregister_chrdev_region(deviceNumber, DEVICE_COUNT);
        return -1;
    }

    return 0;
}

static void __exit exitDriver(void)
{
    gpio_free(0);
    gpio_free(1);
    cdev_del(&c_dev);
    device_destroy(class, deviceNumber);
    class_destroy(class);
    unregister_chrdev_region(deviceNumber, DEVICE_COUNT);
    printk(KERN_INFO "GPIODriver: dice Adios mundo cruel..!!\n");
}

module_init(initDriver);
module_exit(exitDriver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paulo Carteras");
MODULE_DESCRIPTION("GPIO Driver");