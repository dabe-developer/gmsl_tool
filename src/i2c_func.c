/**
 * @file   i2c_func.c
 * @author DAB-Embedded
 * @date   21 Nov 2024
 * @brief  I2C layer - function body.
 *
 */

#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

int i2c_fd = -1;
struct i2c_rdwr_ioctl_data i2c_data;

/**
 * i2c
 *
 * */
int i2c_init(int i2c_bus_num)
{
    int fd = 0, ret = 0;
    char i2c_dev_link[32] = "\0";
    sprintf(i2c_dev_link, "/dev/i2c-%d", i2c_bus_num);
    if ((fd = open(i2c_dev_link, O_RDWR)) < 0)
    {
        printf("open %s failed:%s\n", i2c_dev_link, strerror(errno));
        return -1;
    }
    if ((ret = ioctl(fd, I2C_TIMEOUT, 1000)) < 0) //
    {
        printf("set i2c timeout failed:%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    if ((ret = ioctl(fd, I2C_RETRIES, 5)) < 0) //
    {
        printf("set i2c retries failed:%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    if ((ret = ioctl(fd, I2C_TENBIT, 0)) < 0) // 7bit address
    {
        printf("set i2c Address mode failed:%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    i2c_fd = fd;
    return 0;
}
/**
 *
 * */
int i2c_exit(void)
{
    if (i2c_fd > 0)
        close(i2c_fd);
    return 0;
}

/**
 * i2c read register
 * */
int i2c_read_reg8a8(unsigned char dev_addr,unsigned char reg_addr,unsigned char *buf)
{
    int ret = 0;
    unsigned char buf_in[2]={reg_addr & 0xFF};
    struct i2c_msg message[2];

    message[0].addr = dev_addr;//Slave address
    message[0].flags = 0;   //Write
    message[0].buf = buf_in;//Register 2 bytes
    message[0].len = 1;

    message[1].addr = dev_addr;//Slave address
    message[1].flags = I2C_M_RD; //Read
    message[1].buf = buf;
    message[1].len = 1; // 1 byte of data

    i2c_data.msgs = message;
    i2c_data.nmsgs = 2;

    ret = ioctl(i2c_fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0)
    {
        //printf("%s:%d I2C: read error:%s\n", __func__, __LINE__, strerror(errno));
        return ret;
    }
    return 0;
}

/**
 * i2c read register
 * */
int i2c_read_reg8a16(unsigned char dev_addr,unsigned short reg_addr,unsigned char *buf)
{
    int ret = 0;
    unsigned char buf_in[2]={(reg_addr >> 8) & 0xFF, reg_addr & 0xFF};
    struct i2c_msg message[2];

    message[0].addr = dev_addr;//Slave address
    message[0].flags = 0;   //Write
    message[0].buf = buf_in;//Register 2 bytes
    message[0].len = 2;

    message[1].addr = dev_addr;//Slave address
    message[1].flags = I2C_M_RD; //Read
    message[1].buf = buf;
    message[1].len = 1; // 1 byte of data

    i2c_data.msgs = message;
    i2c_data.nmsgs = 2;

    ret = ioctl(i2c_fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0)
    {
        //printf("%s:%d I2C: read error:%s\n", __func__, __LINE__, strerror(errno));
        return ret;
    }
    return 0;
}

/**
 * i2c Write reg
 * */
int i2c_write_reg8a16(unsigned char dev_addr,unsigned short reg_addr,unsigned char value_byte)
{
    int ret = 0;
    unsigned char buf[3]={0};
    struct i2c_msg message;

    buf[0] = (reg_addr >> 8) & 0xFF;//High reg
    buf[1] = reg_addr & 0xFF;//Low reg
    buf[2] = value_byte;//Reg value

    message.addr = dev_addr;//Slave address
    message.buf = buf;
    message.flags = 0;//Write
    message.len = 3;

    i2c_data.msgs = &message;
    i2c_data.nmsgs = 1;

    ret = ioctl(i2c_fd, I2C_RDWR,&i2c_data);
    if (ret < 0)
    {
        //printf("%s:%d write data error:%s\n", __func__, __LINE__, strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * i2c Write reg
 * */
int i2c_write_reg8a8(unsigned char dev_addr,unsigned char reg_addr,unsigned char value_byte)
{
    int ret = 0;
    unsigned char buf[3]={0};
    struct i2c_msg message;

    buf[0] = reg_addr;//Low reg
    buf[1] = value_byte;//Reg value

    message.addr = dev_addr;//Slave address
    message.buf = buf;
    message.flags = 0;//Write
    message.len = 2;

    i2c_data.msgs = &message;
    i2c_data.nmsgs = 1;

    ret = ioctl(i2c_fd, I2C_RDWR,&i2c_data);
    if (ret < 0)
    {
        //printf("%s:%d write data error:%s\n", __func__, __LINE__, strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * i2c Write reg
 * */
int i2c_write_buffer(unsigned char dev_addr,unsigned char *pbuf,size_t length)
{
    int ret = 0;
    struct i2c_msg message;

    message.addr = dev_addr;//Slave address
    message.buf = pbuf;
    message.flags = 0;//Write
    message.len = length;

    i2c_data.msgs = &message;
    i2c_data.nmsgs = 1;

    ret = ioctl(i2c_fd, I2C_RDWR,&i2c_data);
    if (ret < 0)
    {
        //printf("%s:%d write data error:%s\n", __func__, __LINE__, strerror(errno));
        return -1;
    }
    return 0;
}
