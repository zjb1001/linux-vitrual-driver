#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "vI2C"
#define I2C_BUFFER_SIZE 1024

static int major;
static struct cdev vI2C_cdev;
static struct class *vI2C_class = NULL;
static dev_t dev;

struct i2c_buffer {
    char buffer[I2C_BUFFER_SIZE];
    int pos;
} buffer;

static struct i2c_buffer *i2c_dev = &buffer;

static int vI2C_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "vI2C: Device opened\n");
    return 0;
}

static ssize_t vI2C_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    size_t bytes_to_read = min(count, (size_t)(I2C_BUFFER_SIZE - *ppos));
    
    if (bytes_to_read == 0) return 0; // 没有更多数据可读
    
    if (copy_to_user(buf, &i2c_dev->buffer[*ppos], bytes_to_read)) {
        return -EFAULT;
    }
    
    *ppos += bytes_to_read; // 更新文件位置
    return bytes_to_read;
}

static ssize_t vI2C_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    size_t bytes_to_write = min(count, (size_t)(I2C_BUFFER_SIZE - *ppos));
    
    if (bytes_to_write == 0) return -ENOSPC; // 没有足够空间写入, here loop back to the beginning is another way
    
    if (copy_from_user(&i2c_dev->buffer[*ppos], buf, bytes_to_write)) {
        return -EFAULT;
    }
    
    *ppos += bytes_to_write; // 更新文件位置
    return bytes_to_write;
}

static int vI2C_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "vI2C: Device closed\n");
    return 0;
}

static struct file_operations vI2C_fops = {
    .owner = THIS_MODULE,
    .open = vI2C_open,
    .read = vI2C_read,
    .write = vI2C_write,
    .release = vI2C_release,
};

static int __init vI2C_init(void) {
    int ret;

    // 动态分配主设备号
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "vI2C failed to allocate a major number\n");
        return ret;
    }
    major = MAJOR(dev);
    printk(KERN_INFO "vI2C: registered correctly with major number %d\n", major);

    // 初始化并添加cdev到系统中
    cdev_init(&vI2C_cdev, &vI2C_fops);
    vI2C_cdev.owner = THIS_MODULE;
    ret = cdev_add(&vI2C_cdev, dev, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT "vI2C failed to add cdev\n");
        return ret;
    }

    // 创建设备类
    vI2C_class = class_create(DEVICE_NAME);
    if (IS_ERR(vI2C_class)) {
        cdev_del(&vI2C_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT "Failed to create class\n");
        return PTR_ERR(vI2C_class);
    }

    // 创建设备节点
    if (device_create(vI2C_class, NULL, dev, NULL, DEVICE_NAME) == NULL) {
        class_destroy(vI2C_class);
        cdev_del(&vI2C_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT "Failed to create device\n");
        return -1;
    }

    printk(KERN_INFO "vI2C: device class created successfully\n");
    return 0;
}

static void __exit vI2C_exit(void) {
    device_destroy(vI2C_class, dev);
    class_destroy(vI2C_class);
    cdev_del(&vI2C_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "vI2C: Goodbye from the LKM!\n");
}

module_init(vI2C_init);
module_exit(vI2C_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhang Page");
MODULE_DESCRIPTION("A simple Linux char driver for a virtual I2C device");
MODULE_VERSION("0.1");