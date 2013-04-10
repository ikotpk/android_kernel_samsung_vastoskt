/* 
  * Copyright (C) 2010 Melfas, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <mach/vreg.h>
#include <linux/input.h>
#include <linux/miscdevice.h>

#include <linux/input/melfas-touchkey.h>

#define FW_VERSION	            0x15
#define HW_VERSION              0x03

#define TS_READ_START_ADDR      0
#define TS_READ_REGS_LEN 		5
#define TS_READ_REGS_THR 		13

#define I2C_RETRY_CNT			10

#define PRESS_KEY				1
#define RELEASE_KEY				0

#define DEBUG_PRINT 			1

#define SET_DOWNLOAD_BY_GPIO	1

#define KEYSTATUS_MASK 0x0F

#if SET_DOWNLOAD_BY_GPIO
#include "melfas_download.h"
#endif // SET_DOWNLOAD_BY_GPIO

extern int touch_is_pressed;
extern int dump_enable_flag;
//static int wcdma_call = 0;

extern struct class *sec_class;
struct device *sec_touchkey;

struct melfas_touchkey_data 
{
	struct i2c_client *client; 
	struct input_dev *input_dev;
	struct work_struct  work;
	struct early_suspend early_suspend;
};

struct melfas_touchkey_data *touchkey_driver = NULL;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_touchkey_early_suspend(struct early_suspend *h);
static void melfas_touchkey_late_resume(struct early_suspend *h);
#endif

extern unsigned int board_hw_revision;
static void melfas_touchkey_power_on(void);
static void melfas_touchkey_power_off(void);

static int mcs5080_enabled = 0;
static int mcs5080_led_pending = 0;

static ssize_t touchkey_current_firmware_ver_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	int err = 0;
	uint8_t reg_addr[TS_READ_REGS_LEN];
    
	printk("called %s \n",__func__);  

    if (board_hw_revision >= 2)    
    {
        reg_addr[0] = TS_READ_START_ADDR;   
        err = i2c_master_send(touchkey_driver->client, reg_addr, 1);        
        err = i2c_master_recv(touchkey_driver->client, reg_addr, TS_READ_REGS_LEN);
    }
    
	return sprintf(buf,"%2X\n", reg_addr[1]);
}

static ssize_t touchkey_recommended_firmware_ver_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	int ret = 0;
	ret = sprintf(buf, "%2X\n", FW_VERSION);
	return ret;
}

static ssize_t touchkey_firmware_update_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{	
	int err = 0;
	uint8_t reg_addr[TS_READ_REGS_LEN];    

	printk("called %s \n",__func__);    

    err = mcsdl_download_binary_data(0x90);     

    reg_addr[0] = TS_READ_START_ADDR;   
    err = i2c_master_send(touchkey_driver->client, reg_addr, 1);        
    err = i2c_master_recv(touchkey_driver->client, reg_addr, TS_READ_REGS_LEN);

	printk("[Touchkey] Updated F/W version: 0x%2X\n", reg_addr[1]);	    
  
	return sprintf(buf,"%2X\n", reg_addr[1]);    

}

static ssize_t touchkey_threshold_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	int err = 0;
	uint8_t reg_addr[TS_READ_REGS_THR];    

	printk("called %s \n",__func__);

    reg_addr[0] = TS_READ_START_ADDR;   
    err = i2c_master_send(touchkey_driver->client, reg_addr, 1);        
    err = i2c_master_recv(touchkey_driver->client, reg_addr, TS_READ_REGS_THR);

	return sprintf(buf,"%d\n", reg_addr[12]);
}

static ssize_t touchkey_left_difference_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	int err = 0;
	uint8_t reg_addr[TS_READ_REGS_LEN];    

	printk("called %s \n",__func__);

    reg_addr[0] = TS_READ_START_ADDR;   
    err = i2c_master_send(touchkey_driver->client, reg_addr, 1);        
    err = i2c_master_recv(touchkey_driver->client, reg_addr, TS_READ_REGS_LEN);

	return sprintf(buf,"%d\n", reg_addr[3]);
}

static ssize_t touchkey_right_difference_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	int err = 0;
	uint8_t reg_addr[TS_READ_REGS_LEN];        

	printk("called %s \n",__func__);
    
    reg_addr[0] = TS_READ_START_ADDR;   
    err = i2c_master_send(touchkey_driver->client, reg_addr, 1);        
    err = i2c_master_recv(touchkey_driver->client, reg_addr, TS_READ_REGS_LEN);

	return sprintf(buf,"%d\n", reg_addr[4]);
}
/*
static ssize_t set_touchkey_for_noise_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{	
    int err = 0;
    uint8_t data;
    uint8_t threshold_lv60[2] = {0x15, 0x0F};
    uint8_t threshold_reversion = 0x16;

    printk("called %s \n", __func__);	

    if( buf[0]=='1' )
        data = 1;
    else
        data = 0;  

    printk("[Touchkey] WCDMA Call = %d \n", data);

    if(data == 1) // WCDMA Call Start
    {
        if (board_hw_revision >= 2)
            err = i2c_master_send(touchkey_driver->client, threshold_lv60, 2);
        wcdma_call = 1;
    }
    else // WCDMA Call End
    {
        if (board_hw_revision >= 2)    
            err = i2c_master_send(touchkey_driver->client, &threshold_reversion, 1);
        wcdma_call = 0;       
    }

    return size;
}

static void set_touchkey_for_noise()
{
    int err = 0;
    uint8_t threshold_lv60[2] = {0x15, 0x0F};    

    printk("called %s \n", __func__);	    

    if (board_hw_revision >= 2)
    {
        err = i2c_master_send(touchkey_driver->client, threshold_lv60, 2);
        if(err < 0)
            printk(KERN_ERR "[Touchkey] %s : i2c_master_send [%d]\n", __func__, err);			
    }
}
*/
static ssize_t touchkey_led_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int err = 0;
    uint8_t data;
    uint8_t led_lv10[2] = {0x14, 0x0A};        

    if( buf[0]=='1' )
        data = 1;
    else
        data = 2;

    printk("[Touchkey] LED Data = %d \n", data);	    

    if(mcs5080_enabled == 0)
    { 
        if(data == 1)  // pending (on)
            mcs5080_led_pending = 1;
        return size;    
    }

    if (board_hw_revision >= 2)
    {
        err = i2c_master_send(touchkey_driver->client, &data, 1); // LED on(data=1) or off(data=2)
        if (data == 1)
            err = i2c_master_send(touchkey_driver->client, led_lv10, 2);
    }

    return size;
}

int touchkey_update_open (struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t touchkey_update_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

ssize_t touchkey_update_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    return count;
}

int touchkey_update_release (struct inode *inode, struct file *filp)
{
    return 0;
}

struct file_operations touchkey_update_fops =
{
    .owner   = THIS_MODULE,
    .read    = touchkey_update_read,
    .write   = touchkey_update_write,
    .open    = touchkey_update_open,
    .release = touchkey_update_release,
};

static struct miscdevice touchkey_update_device= {
    .minor = MISC_DYNAMIC_MINOR,
    .name = MELFAS_TOUCHKEY_DEV_NAME,
    .fops = &touchkey_update_fops,
};

static DEVICE_ATTR(touchkey_current_firmware_ver, S_IRUGO, touchkey_current_firmware_ver_show, NULL);
static DEVICE_ATTR(touchkey_recommended_firmware_ver, S_IRUGO, touchkey_recommended_firmware_ver_show, NULL);
static DEVICE_ATTR(touchkey_firmware_update, S_IRUGO, touchkey_firmware_update_show, NULL);
static DEVICE_ATTR(touchkey_threshold, S_IRUGO, touchkey_threshold_show, NULL);
static DEVICE_ATTR(touchkey_left_difference, S_IRUGO, touchkey_left_difference_show, NULL);
static DEVICE_ATTR(touchkey_right_difference, S_IRUGO, touchkey_right_difference_show, NULL);
//static DEVICE_ATTR(set_touchkey_for_noise, S_IRUGO | S_IWUSR | S_IWGRP, NULL, set_touchkey_for_noise_store);
static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL, touchkey_led_control);

static void init_hw(void)
{
    gpio_direction_input(KEY_INT);
}

static void melfas_touchkey_work_func(struct work_struct *work)
{
	struct melfas_touchkey_data *ts = container_of(work, struct melfas_touchkey_data, work);
	int ret = 0, i;
	uint8_t buf[TS_READ_REGS_LEN];
//	uint8_t keyCode=0, updownEvt =0;/*, esdStatus=0, firmwareVersion=0, moduleRevision=0, intensity = 0; */
    int keycode, pressed;

#if 0 //DEBUG_PRINT
	printk(KERN_ERR "[Touchkey] melfas_touchkey_work_func\n");

	if(ts ==NULL)
        printk(KERN_ERR "[Touchkey] melfas_touchkey_work_func : TS NULL\n");
#endif 

	/**
	Simple send transaction:
		S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P
	Simple recv transaction:
		S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P
	*/

	buf[0] = TS_READ_START_ADDR;

    for(i=0; i<I2C_RETRY_CNT; i++)
    {
        ret = i2c_master_send(ts->client, buf, 1);
#if 0//DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);
#endif 
    
    	ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);
#if 0 //DEBUG_PRINT			
	printk(KERN_ERR "[Touchkey] melfas_touchkey_work_func : i2c_master_recv [%d]\n", ret);			
#endif
        if(ret >=0)
            break; // i2c success
    }

	if (ret < 0)
	{
		printk(KERN_ERR "[Touchkey] melfas_touchkey_work_func: i2c failed\n");
		return ;	
	}
#if 0
    keyCode = buf[0] & 0x07; // Key code MASK
    updownEvt = buf[0] & 0x08; // Up/Down event MASK

    if (keyCode == 0x01) 
        keycode = KEY_MENU;
    else // keyCode == 0x02
        keycode = KEY_BACK;
    
/*    
    esdStatus = buf[0] & 0x30; // ESD Status MASK
    firmwareVersion = buf[1];
    moduleRevision = buf[2];

    if( keyCode == 0x1 ) // 1st Button Key
        input_report_key(ts->input_dev, KEY_MENU, updownEvt ? PRESS_KEY : RELEASE_KEY);		

    else // 2nd Button Key
        input_report_key(ts->input_dev, KEY_BACK, updownEvt ? PRESS_KEY : RELEASE_KEY);		
*/
    if(updownEvt) // Key Released
    {
        input_report_key(ts->input_dev, keycode, 0);
        input_sync(ts->input_dev);
        printk("[Touchkey] touchkey release keycode: %d\n", keycode);
    }
    else // Key Pressed
    {
        if(touch_is_pressed == 1)
        {
            printk("[Touchkey] touchkey pressed but don't send event because touch is pressed. \n");
        }
        else 
        {           
            if(keycode == KEY_BACK)
            {
                // if Back Key is Pressed, Release Multitouch
            }
            
            input_report_key(ts->input_dev, keycode, 1);
            input_sync(ts->input_dev);
            printk("[Touchkey] touchkey press keycode: %d\n", keycode);
        }
    }

#else
    switch( buf[0] & KEYSTATUS_MASK )
    {
    case 0x01: // 1st Button Key Down
        keycode = KEY_MENU;        
        pressed = 1;
//        printk("[Touchkey] KEY_MENU is pressed\n");        
        break;
    case 0x09: // 1st Button Key Up
        keycode = KEY_MENU;        
        pressed = 0;
//        printk("[Touchkey] KEY_MENU is releaseed\n");        
        break;
    case 0x02: //2nd Button Key Down
        keycode = KEY_BACK;        
        pressed = 1;
//        printk("[Touchkey] KEY_BACK is pressed\n");        
        break;
    case 0x0A: // 2nd Button Key Up
        keycode = KEY_BACK;
        pressed = 0;
//        printk("[Touchkey] KEY_BACK is releaseed\n");        
        break;
    default:
        printk("[Touchkey] KEYSTATUS_MASK - default : buf[0] = %X\n", buf[0]);            
        break;
    }

    if (touch_is_pressed == 1 && pressed == 1)
    {
        printk("[Touchkey] touchkey pressed but don't send event because touch is pressed. \n");    
    }
    else if (buf[0] == 0)
    {
        printk("[Touchkey] touchkey resume ERROR - Garbage Value Occured. \n");        
    }
    else
    {           
        if (dump_enable_flag)
            printk("[Touchkey] KEY_CODE : %d, Pressed : %d\n", keycode, pressed);                       
        input_report_key(ts->input_dev, keycode, pressed);    
    	input_sync(ts->input_dev);    
    }
    
#endif

#if 0//DEBUG_PRINT
	printk(KERN_ERR "[Touchkey] melfas_touchkey_work_func: keyID : %d, updownEvt: %d\n", keyCode, updownEvt);
#endif	
		
	enable_irq(ts->client->irq);
}

static irqreturn_t melfas_touchkey_irq_handler(int irq, void *handle)
{
	struct melfas_touchkey_data *ts = (struct melfas_touchkey_data *)handle;
#if 0 //DEBUG_PRINT
	printk(KERN_ERR "[Touchkey] melfas_touchkey_irq_handler\n");
#endif
		
	disable_irq_nosync(ts->client->irq);
	schedule_work(&ts->work);
	
	return IRQ_HANDLED;
}

static int melfas_touchkey_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct melfas_touchkey_data *ts;
	int ret = 0, i; 
	
	uint8_t buf[TS_READ_REGS_LEN];
	
#if DEBUG_PRINT
	printk(KERN_ERR "[Touchkey] kim ms : melfas_touchkey_probe\n");
#endif

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        printk(KERN_ERR "[Touchkey] melfas_touchkey_probe: need I2C_FUNC_I2C\n");
        ret = -ENODEV;
        goto err_check_functionality_failed;
    }

    ts = kmalloc(sizeof(struct melfas_touchkey_data), GFP_KERNEL);
    if (ts == NULL)
    {
        printk(KERN_ERR "[Touchkey] melfas_touchkey_probe: failed to create a state of melfas_touchekey\n");
        ret = -ENOMEM;
        goto err_alloc_data_failed;
    }

    INIT_WORK(&ts->work, melfas_touchkey_work_func);

    init_hw();	    
    melfas_touchkey_power_on();

    ts->client = client;
    i2c_set_clientdata(client, ts);

    touchkey_driver = ts;
    
//    ret = i2c_master_send(ts->client, &buf, 1);
    
#if DEBUG_PRINT
	printk(KERN_ERR "[Touchkey] melfas_touchkey_probe: i2c_master_send() [%d], Add[%d]\n", ret, ts->client->addr);
#endif

#if SET_DOWNLOAD_BY_GPIO
	buf[0] = TS_READ_START_ADDR;

    for(i=0; i < 10; i++)
    {
        ret = i2c_master_send(ts->client, buf, 1);
        if(ret < 0)
            printk(KERN_ERR "[Touchkey] melfas_touchkey_work_func : i2c_master_send [%d]\n", ret);			

        ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);
        if(ret < 0)
            printk(KERN_ERR "[Touchkey] melfas_touchkey_work_func : i2c_master_recv [%d]\n", ret);			

        if(ret >= 0)
            break;
    }

	if(buf[1] < FW_VERSION)
	{
		ret = mcsdl_download_binary_data(0x90);		
//		ret = mcsdl_download_binary_file();
		if(ret > 0)
		{
			printk(KERN_ERR "[Touchkey] SET Download Fail - error code [%d]\n", ret);			
		}
	}

#endif // SET_DOWNLOAD_BY_GPIO
	
	ts->input_dev = input_allocate_device();
    if (!ts->input_dev)
    {
		printk(KERN_ERR "[Touchkey] melfas_touchkey_probe: Not enough memory\n");
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	} 

	ts->input_dev->name = "melfas_touchkey";

	ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
	

	ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
	ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);		

    ret = input_register_device(ts->input_dev);
    if (ret)
    {
        printk(KERN_ERR "[Touchkey] melfas_touchkey_probe: Failed to register device\n");
        ret = -ENOMEM;
        goto err_input_register_device_failed;
    }

    if (ts->client->irq)
    {
#if DEBUG_PRINT
        printk(KERN_ERR "[Touchkey] melfas_touchkey_probe: trying to request irq: %s-%d\n", ts->client->name, ts->client->irq);
#endif
        ret = request_irq(client->irq, melfas_touchkey_irq_handler, IRQF_TRIGGER_FALLING, ts->client->name, ts);
        if (ret > 0)
        {
            printk(KERN_ERR "[Touchkey] melfas_touchkey_probe: Can't allocate irq %d, ret %d\n", ts->client->irq, ret);
            ret = -EBUSY;
            goto err_request_irq;
        }
    }

	schedule_work(&ts->work);

#if DEBUG_PRINT	
	printk(KERN_ERR "[Touchkey] melfas_touchkey_probe: succeed to register input device\n");
#endif

#if CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = melfas_touchkey_early_suspend;
	ts->early_suspend.resume = melfas_touchkey_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif
	
#if DEBUG_PRINT
	printk(KERN_INFO "[Touchkey] melfas_touchkey_probe: Start touchscreen. name: %s, irq: %d\n", ts->client->name, ts->client->irq);
#endif
	return 0;

err_request_irq:
	printk(KERN_ERR "[Touchkey] melfas_touchkey: err_request_irq failed\n");
	free_irq(client->irq, ts);
err_input_register_device_failed:
	printk(KERN_ERR "[Touchkey] melfas_touchkey: err_input_register_device failed\n");
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	printk(KERN_ERR "[Touchkey] melfas_touchkey: err_input_dev_alloc failed\n");
err_alloc_data_failed:
	printk(KERN_ERR "[Touchkey] melfas_touchkey: err_alloc_data failed_\n");	
err_detect_failed:
	printk(KERN_ERR "[Touchkey] melfas_touchkey: err_detect failed\n");
	kfree(ts);
err_check_functionality_failed:
	printk(KERN_ERR "[Touchkey] melfas_touchkey: err_check_functionality failed_\n");

	return ret;
}

static void melfas_touchkey_power_on(void)
{
    struct vreg *vreg_touch18;
    uint8_t led_on = 1;
    uint8_t led_lv10[2] = {0x14, 0x0A};            
    
    int ret = 0, err = 0;

    vreg_touch18 = vreg_get(NULL, "vreg_touch18");

    ret = vreg_set_level(vreg_touch18, 1800);

    gpio_set_value(KEY_INT, 1);
    gpio_direction_output(KEY_SCL, 1);    
    gpio_direction_output(KEY_SDA, 1);    

    gpio_direction_output(KEY_LED_33V, 1);

    ret = vreg_enable(vreg_touch18);        

    mdelay(100);    

    mcs5080_enabled = 1;

    if(mcs5080_led_pending == 1)
    {
        if (board_hw_revision >= 2)
        {
            err = i2c_master_send(touchkey_driver->client, &led_on, 1); // LED on(data=1) or off(data=2)
            err = i2c_master_send(touchkey_driver->client, led_lv10, 2);			
        }
    }

    printk("[Touchkey] melfas_touchkey: power_on.\n");                  
}

static void melfas_touchkey_power_off(void)
{
    struct vreg *vreg_touch18;
    int ret = 0;

    vreg_touch18 = vreg_get(NULL, "vreg_touch18");

    gpio_set_value(KEY_INT, 0);
    gpio_direction_output(KEY_SCL, 0);  
    gpio_direction_output(KEY_SDA, 0);  

    ret = vreg_disable(vreg_touch18);

    gpio_direction_output(KEY_LED_33V, 0);    

    mcs5080_enabled = 0;
    mcs5080_led_pending = 0;

    printk("[Touchkey] melfas_touchkey: power_off.\n");                      
}

static int melfas_touchkey_remove(struct i2c_client *client)
{
    struct melfas_touchkey_data *ts = i2c_get_clientdata(client);

    unregister_early_suspend(&ts->early_suspend);
    free_irq(client->irq, ts);
    input_unregister_device(ts->input_dev);
    kfree(ts);
    return 0;
}

static int melfas_touchkey_suspend(struct i2c_client *client, pm_message_t mesg)
{
	//power off
}

static int melfas_touchkey_resume(struct i2c_client *client)
{
	//power on
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_touchkey_early_suspend(struct early_suspend *h)
{
    printk("[Touchkey] melfas_touchkey: early Suspend\n");

    struct melfas_touchkey_data *ts;
    ts = container_of(h, struct melfas_touchkey_data, early_suspend);
    //melfas_touchkey_suspend(ts->client, PMSG_SUSPEND);
    disable_irq_nosync(ts->client->irq);	
    melfas_touchkey_power_off();
}

static void melfas_touchkey_late_resume(struct early_suspend *h)
{
    printk("[Touchkey] melfas_touchkey: late Resume\n");

    struct melfas_touchkey_data *ts;
    ts = container_of(h, struct melfas_touchkey_data, early_suspend);
    //melfas_touchkey_resume(ts->client);
    melfas_touchkey_power_on();
/*    
    if (wcdma_call == 1)
        set_touchkey_for_noise();
*/        
    enable_irq(ts->client->irq);           
}
#endif

static const struct i2c_device_id melfas_touchkey_id[] =
{
    { MELFAS_TOUCHKEY_DEV_NAME, 0 },
    { }
};

static struct i2c_driver melfas_touchkey_driver =
{
    .driver = {
    .name = MELFAS_TOUCHKEY_DEV_NAME,
    },
    .id_table = melfas_touchkey_id,
    .probe = melfas_touchkey_probe,
    .remove = __devexit_p(melfas_touchkey_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = melfas_touchkey_suspend,
	.resume = melfas_touchkey_resume,
#endif
};

static int __devinit melfas_touchkey_init(void)
{
    int ret = 0;
    
    sec_touchkey= device_create(sec_class, NULL, 0, NULL, "sec_touchkey");

	if (IS_ERR(sec_touchkey))
		printk(KERN_ERR "[Touchkey] Failed to create device(sec_touchkey)!\n");

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_current_firmware_ver)< 0)
		printk(KERN_ERR "[Touchkey] Failed to create device file(%s)!\n", dev_attr_touchkey_current_firmware_ver.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_recommended_firmware_ver)< 0)
		printk(KERN_ERR "[Touchkey] Failed to create device file(%s)!\n", dev_attr_touchkey_recommended_firmware_ver.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_firmware_update)< 0)
		printk(KERN_ERR "[Touchkey] Failed to create device file(%s)!\n", dev_attr_touchkey_firmware_update.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_threshold)< 0)
		printk(KERN_ERR "[Touchkey] Failed to create device file(%s)!\n", dev_attr_touchkey_threshold.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_left_difference)< 0)
		printk(KERN_ERR "[Touchkey] Failed to create device file(%s)!\n", dev_attr_touchkey_left_difference.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_right_difference)< 0)
		printk(KERN_ERR "[Touchkey] Failed to create device file(%s)!\n", dev_attr_touchkey_right_difference.attr.name);   
/*
	if (device_create_file(sec_touchkey, &dev_attr_set_touchkey_for_noise)< 0)
		printk(KERN_ERR "[Touchkey] Failed to create device file(%s)!\n", dev_attr_set_touchkey_for_noise.attr.name);      
*/
	ret = misc_register(&touchkey_update_device);
    
	if (ret)
		printk(KERN_ERR "[TouchKey] %s misc_register fail\n", __func__);
	
	if (device_create_file(touchkey_update_device.this_device, &dev_attr_brightness) < 0)
	{
		printk(KERN_ERR "%s device_create_file fail dev_attr_brightness\n", __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n", dev_attr_brightness.attr.name);
	}

	return i2c_add_driver(&melfas_touchkey_driver);
}

static void __exit melfas_touchkey_exit(void)
{
	i2c_del_driver(&melfas_touchkey_driver);
}

MODULE_DESCRIPTION("Driver for Melfas MTSI Touchscreen Controller");
MODULE_AUTHOR("MinSang, Kim <kimms@melfas.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

module_init(melfas_touchkey_init);
module_exit(melfas_touchkey_exit);
