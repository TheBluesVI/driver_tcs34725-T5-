#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>

#define DRIVER_NAME "tcs34725_driver"
#define DEVICE_NAME "tcs34725"
#define CLASS_NAME  "tcs34725_class"

#define CMD_BIT         0x80
#define ENABLE_REG      0x00
#define ATIME_REG       0x01
#define CONTROL_REG     0x0F
#define CDATAL_REG      0x14

#define ENABLE_PON      0x01
#define ENABLE_AEN      0x02

#define TCS_IOCTL_MAGIC 't'
#define TCS_IOCTL_READ_CLEAR  _IOR(TCS_IOCTL_MAGIC, 1, __u16)
#define TCS_IOCTL_READ_RED    _IOR(TCS_IOCTL_MAGIC, 2, __u16)
#define TCS_IOCTL_READ_GREEN  _IOR(TCS_IOCTL_MAGIC, 3, __u16)
#define TCS_IOCTL_READ_BLUE   _IOR(TCS_IOCTL_MAGIC, 4, __u16)

static struct i2c_client *tcs34725_client;
static int major_number;
static struct class *tcs_class;
static struct device *tcs_device;

// IOCTL file operations
static int tcs_open(struct inode *inode, struct file *file) {
    return 0;
}

static int tcs_release(struct inode *inode, struct file *file) {
    return 0;
}

static int tcs_read_channel(int index)
{
    u8 buf[8] = {0};
    u16 values[4];

    if (!tcs34725_client)
        return -ENODEV;

    if (i2c_smbus_read_i2c_block_data(tcs34725_client, CMD_BIT | CDATAL_REG, sizeof(buf), buf) < 0)
        return -EIO;

    values[0] = (buf[1] << 8) | buf[0]; // Clear
    values[1] = (buf[3] << 8) | buf[2]; // Red
    values[2] = (buf[5] << 8) | buf[4]; // Green
    values[3] = (buf[7] << 8) | buf[6]; // Blue

    return (index >= 0 && index < 4) ? values[index] : -EINVAL;
}

static long tcs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    __u16 value;

    switch (cmd) {
        case TCS_IOCTL_READ_CLEAR:
            value = tcs_read_channel(0);
            break;
        case TCS_IOCTL_READ_RED:
            value = tcs_read_channel(1);
            break;
        case TCS_IOCTL_READ_GREEN:
            value = tcs_read_channel(2);
            break;
        case TCS_IOCTL_READ_BLUE:
            value = tcs_read_channel(3);
            break;
        default:
            return -EINVAL;
    }

    if (copy_to_user((__u16 __user *)arg, &value, sizeof(value)))
        return -EFAULT;

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = tcs_open,
    .release = tcs_release,
    .unlocked_ioctl = tcs_ioctl,
};

static int tcs34725_probe(struct i2c_client *client)
{
    int ret;

    tcs34725_client = client;

    // Sensor init
    ret = i2c_smbus_write_byte_data(client, CMD_BIT | ENABLE_REG, ENABLE_PON);
    if (ret < 0) return ret;
    msleep(3);
    ret = i2c_smbus_write_byte_data(client, CMD_BIT | ENABLE_REG, ENABLE_PON | ENABLE_AEN);
    if (ret < 0) return ret;
    ret = i2c_smbus_write_byte_data(client, CMD_BIT | ATIME_REG, 0x00);
    if (ret < 0) return ret;
    ret = i2c_smbus_write_byte_data(client, CMD_BIT | CONTROL_REG, 0x00);
    if (ret < 0) return ret;
    msleep(700);

    // Register char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) return major_number;

    tcs_class = class_create(CLASS_NAME);
    if (IS_ERR(tcs_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(tcs_class);
    }

    tcs_device = device_create(tcs_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(tcs_device)) {
        class_destroy(tcs_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(tcs_device);
    }

    dev_info(&client->dev, "TCS34725 driver installed with ioctl\n");
    return 0;
}

static void tcs34725_remove(struct i2c_client *client)
{
    device_destroy(tcs_class, MKDEV(major_number, 0));
    class_destroy(tcs_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    dev_info(&client->dev, "TCS34725 driver removed\n");
}

static const struct of_device_id tcs34725_of_match[] = {
    { .compatible = "taos,tcs34725" },
    { }
};
MODULE_DEVICE_TABLE(of, tcs34725_of_match);

static struct i2c_driver tcs34725_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = tcs34725_of_match,
    },
    .probe = tcs34725_probe,
    .remove = tcs34725_remove,
};

static int __init tcs34725_init(void)
{
    pr_info("Initializing TCS34725 ioctl driver\n");
    return i2c_add_driver(&tcs34725_driver);
}

static void __exit tcs34725_exit(void)
{
    pr_info("Exiting TCS34725 ioctl driver\n");
    i2c_del_driver(&tcs34725_driver);
}

module_init(tcs34725_init);
module_exit(tcs34725_exit);

MODULE_AUTHOR("You");
MODULE_DESCRIPTION("TCS34725 I2C Driver with ioctl");
MODULE_LICENSE("GPL");
