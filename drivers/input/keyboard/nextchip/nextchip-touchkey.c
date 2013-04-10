#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <asm/gpio.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/earlysuspend.h>
#include <asm/io.h>

#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include <mach/vreg.h>
#include <linux/hrtimer.h>

#ifdef CONFIG_KEYBOARD_NEXTCHIP_TOUCH
#include <linux/input/nextchip-touchkey.h>
#endif

/*
extern struct class *sec_class;
struct device *sec_touchkey;
*/

#define TKEY_LATEST_FIRMWARE    0xD6

extern unsigned int board_hw_revision;

struct i2c_nextchip_touchkey_driver {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct hrtimer timer;      
	struct work_struct  work_timer;    
	struct early_suspend early_suspend;
};
struct i2c_nextchip_touchkey_driver *nextchip_touchkey_driver = NULL;

extern unsigned char I2Cm_ReadBytes(unsigned char SlaveAdr, unsigned char *RxArray, 
									unsigned char SubAdr0, unsigned char SubAdr1, 
									unsigned char RxByteCount);
extern void TKey_Firmware_Update(void);

static int touchkey_keycode[2] = { KEY_MENU, KEY_BACK };

struct work_struct touchkey_work;
struct workqueue_struct *touchkey_wq;
struct workqueue_struct *check_ic_wq;
/*
static ssize_t touchkey_current_firmware_ver_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned char firm_ver = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &firm_ver, 0x81, 0xC1, 1);    
	return sprintf(buf,"%2X\n",firm_ver);
}

static ssize_t touchkey_recommended_firmware_ver_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	int ret = 0;
	ret = sprintf(buf, "%2X\n", TKEY_LATEST_FIRMWARE);
	return ret;
}

static ssize_t touchkey_left_threshold_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned char THR_L = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &THR_L, 0x81, 0xCA, 1);        
	return sprintf(buf,"%2X\n",THR_L);
}

static ssize_t touchkey_right_threshold_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned char THR_R = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &THR_R, 0x81, 0xCB, 1);        
	return sprintf(buf,"%2X\n",THR_R);
}

static ssize_t touchkey_left_rawdata_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned int RAW_L = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &RAW_L, 0x81, 0xD0, 2);        
	return sprintf(buf,"%4X\n",RAW_L);
}

static ssize_t touchkey_right_rawdata_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned int RAW_R = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &RAW_R, 0x81, 0xD2, 2);        
	return sprintf(buf,"%4X\n",RAW_R);
}

static ssize_t touchkey_left_baseline_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned int BASE_L = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &BASE_L, 0x81, 0xD4, 2);        
	return sprintf(buf,"%4X\n",BASE_L);
}

static ssize_t touchkey_right_baseline_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned int BASE_R = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &BASE_R, 0x81, 0xD6, 2);        
	return sprintf(buf,"%4X\n",BASE_R);
}

static ssize_t touchkey_left_difference_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned int DIFF_L = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &DIFF_L, 0x81, 0xD8, 2);        
	return sprintf(buf,"%4X\n",DIFF_L);
}

static ssize_t touchkey_right_difference_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned int DIFF_R = 0;
	int err = 0;

	printk("called %s \n",__func__);
	err = I2Cm_ReadBytes(0xc0, &DIFF_R, 0x81, 0xDA, 2);        
	return sprintf(buf,"%4X\n",DIFF_R);
}
static ssize_t touch_led_control(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t size)
{
	int state = 0;
	state = gpio_get_value(KEY_LED);
	
	if(*buf == '1') {
		if(state == 0)
			gpio_set_value(KEY_LED, 1);
	}
	else {
		if(state == 1)
			gpio_set_value(KEY_LED, 0);
	}
	return size;
}

static DEVICE_ATTR(touchkey_current_firmware_ver, S_IRUGO, touchkey_current_firmware_ver_show, NULL);
static DEVICE_ATTR(touchkey_recommended_firmware_ver, S_IRUGO, touchkey_recommended_firmware_ver_show, NULL);
static DEVICE_ATTR(touchkey_left_threshold, S_IRUGO, touchkey_left_threshold_show, NULL);
static DEVICE_ATTR(touchkey_right_threshold, S_IRUGO, touchkey_right_threshold_show, NULL);
static DEVICE_ATTR(touchkey_left_rawdata, S_IRUGO, touchkey_left_rawdata_show, NULL);
static DEVICE_ATTR(touchkey_right_rawdata, S_IRUGO, touchkey_right_rawdata_show, NULL);
static DEVICE_ATTR(touchkey_left_baseline, S_IRUGO, touchkey_left_baseline_show, NULL);
static DEVICE_ATTR(touchkey_right_baseline, S_IRUGO, touchkey_right_baseline_show, NULL);
static DEVICE_ATTR(touchkey_left_difference, S_IRUGO, touchkey_left_difference_show, NULL);
static DEVICE_ATTR(touchkey_right_difference, S_IRUGO, touchkey_right_difference_show, NULL);

static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL, touch_led_control);
*/
static void nextchip_power_on(void)
{
    struct vreg *vreg_touch18;
    int ret = 0;

    vreg_touch18 = vreg_get(NULL, "vreg_touch18");

    ret = vreg_set_level(vreg_touch18, 1800);

	gpio_set_value(KEY_INT, 1);
	gpio_direction_output(KEY_SCL, 1);    
	gpio_direction_output(KEY_SDA, 1);    

    ret = vreg_enable(vreg_touch18);        

	mdelay(70);    

	gpio_direction_output(KEY_RST, 1); 

    printk("[Touchkey] nextchip_power_on.\n");                  

}

static void nextchip_power_off(void)
{
    struct vreg *vreg_touch18;
    int ret = 0;

    vreg_touch18 = vreg_get(NULL, "vreg_touch18");
  
	gpio_set_value(KEY_INT, 0);
	gpio_direction_output(KEY_RST, 0);  
	gpio_direction_output(KEY_SCL, 0);  
	gpio_direction_output(KEY_SDA, 0);  

    ret = vreg_disable(vreg_touch18);

    printk("[Touchkey] nextchip_power_off.\n");                      
}

static void nextchip_early_suspend(struct early_suspend *h)
{
    printk("[Touchkey] Suspend\n");
    int ret=0;

	disable_irq_nosync(KEY_INT);

	ret = cancel_work_sync(&nextchip_touchkey_driver->work_timer);    
	hrtimer_cancel(&nextchip_touchkey_driver->timer);   

    nextchip_power_off();
}

static void nextchip_late_resume(struct early_suspend *h)
{
    printk("[Touchkey] Resume\n");

	hrtimer_start(&nextchip_touchkey_driver->timer, ktime_set(2, 0), HRTIMER_MODE_REL);    

	enable_irq(KEY_INT);
    
    nextchip_power_on();    
}

int nextchip_reset( void )
{
    printk("[Touchkey] nextchip_reset\n");

	gpio_direction_output(KEY_RST, 0);      
    mdelay(10);
	gpio_direction_output(KEY_RST, 1);  
}    

static void check_ic_work_func(struct work_struct *work)
{
//    printk("[Touchkey] check_ic_work_func\n");

	int err = 0;
	unsigned char firm_ver = 0;

	err = I2Cm_ReadBytes(0xc0, &firm_ver, 0x81, 0xC1, 1);
/*    
	if(err)
		printk("[Touchkey] I2C Error. err=%d\n", err);

	printk("[Touchkey] firmware version %x\n", firm_ver);
*/	
    if ( (firm_ver >> 4) ==  0xD)
        return;
    else
        nextchip_reset();
}

static enum hrtimer_restart nextchip_watchdog_timer_func(struct hrtimer *timer)
{
	queue_work(check_ic_wq, &nextchip_touchkey_driver->work_timer);
	hrtimer_start(&nextchip_touchkey_driver->timer, ktime_set(0, 500000000), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

void touchkey_work_func(struct work_struct *p)
{
	unsigned char nKeyData;
	int err = 0;
	int keycode= 0;
	int pressed=0;

	err = I2Cm_ReadBytes(0xC0, &nKeyData, 0x81, 0xC0, 1);

	if(err)
	{
		printk("[Touchkey] I2C Error. err=%d\n", err);
		return;
	}

	switch( nKeyData )
	{
	case 0x01:	//Left Button
		keycode = KEY_BACK;        
		pressed = 1;
		printk("[Touchkey] KEY_BACK is pressed\n");        
		break;
	case 0x09:	// Left Button release
		keycode = KEY_BACK;        
		pressed = 0;
		printk("[Touchkey] KEY_BACK is releaseed\n");        
		break;
	case 0x02:	//Right Button
		keycode = KEY_MENU;        
		pressed = 1;
		printk("[Touchkey] KEY_MENU is pressed\n");        
		break;
	case 0x0A:	// Right Button release
		keycode = KEY_MENU;
		pressed = 0;
		printk("[Touchkey] KEY_MENU is releaseed\n");        
		break;
	}

	input_report_key(nextchip_touchkey_driver->input_dev, keycode, pressed);

	enable_irq(KEY_INT);

//    gpio_tlmm_config(GPIO_CFG(KEY_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
    
}


static irqreturn_t touchkey_interrupt(int irq, void *dummy)
{
	disable_irq_nosync(KEY_INT);
	queue_work(touchkey_wq, &touchkey_work);

	return IRQ_HANDLED;
}

static void init_hw(void)
{
#if 1
    struct vreg *vreg_touch18;
    int ret = 0;

	gpio_direction_input(KEY_INT);
	gpio_direction_output(KEY_TEST, 0);
	mdelay(2);
	gpio_direction_output(KEY_RST, 1);

    vreg_touch18 = vreg_get(NULL, "vreg_touch18");

    ret = vreg_set_level(vreg_touch18, 1800);
    ret = vreg_enable(vreg_touch18);            

	mdelay(1000);
#endif    
}
/*
int touchkey_update_open(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t touchkey_update_read(struct file * filp, char *buf, size_t count,
			     loff_t * f_pos)
{
	return 1;
}

int touchkey_update_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations touchkey_update_fops = {
	.owner = THIS_MODULE,
	.read = touchkey_update_read,
	.open = touchkey_update_open,
	.release = touchkey_update_release,
};

static struct miscdevice touchkey_update_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "nextchip_touchkey",
	.fops = &touchkey_update_fops,
};
*/
static int i2c_touchkey_probe(struct i2c_client *client,
			      const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct input_dev *input_dev;
	int err = 0;
	int irq;
	unsigned char firm_ver = 0;
	unsigned char THR_L = 0;    
	unsigned char THR_R = 0;    
	unsigned int RAW_L = 0;    
	unsigned int RAW_R = 0;    
	unsigned int BASE_L = 0;    
	unsigned int BASE_R = 0;    
	unsigned int DIFF_L = 0;    
	unsigned int DIFF_R = 0;    

	printk("[Touchkey] %s\n", __func__);
	input_dev = input_allocate_device();

	if (!input_dev) {
		return -ENOMEM;
	}

	input_dev->name = "nextchip_touchkey";
	input_dev->phys = "nextchip_touchkey/input0";
	//input_dev->id.bustype = BUS_HOST;

	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(touchkey_keycode[0], input_dev->keybit);
	set_bit(touchkey_keycode[1], input_dev->keybit);

	err = input_register_device(input_dev);
	if (err) {
		printk("[Touchkey] err input_register_device. %d\n", err);
		input_free_device(input_dev);
		return err;
	}

	nextchip_touchkey_driver = kzalloc(sizeof(struct i2c_nextchip_touchkey_driver), GFP_KERNEL);
	if (nextchip_touchkey_driver == NULL) {
		dev_err(dev, "failed to create our state\n");
		return -ENOMEM;
	}

	nextchip_touchkey_driver->input_dev = input_dev;
	init_hw();	
	//nextchip_power_on();
	INIT_WORK(&nextchip_touchkey_driver->work_timer, check_ic_work_func );        

	err = request_irq(client->irq, touchkey_interrupt, IRQF_TRIGGER_FALLING, "nextchip_touchkey",  NULL);
   
	hrtimer_init(&nextchip_touchkey_driver->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	nextchip_touchkey_driver->timer.function = nextchip_watchdog_timer_func;

	if(err) {
		printk(KERN_ERR "[TouchKey] %s Can't allocate irq. err=%d ..\n", __func__, err);
		return -EBUSY;
	}

	nextchip_touchkey_driver->early_suspend.suspend = nextchip_early_suspend;
	nextchip_touchkey_driver->early_suspend.resume = nextchip_late_resume;
	register_early_suspend(&nextchip_touchkey_driver->early_suspend);

	err = I2Cm_ReadBytes(0xc0, &firm_ver, 0x81, 0xC1, 1);
	err = I2Cm_ReadBytes(0xc0, &THR_L, 0x81, 0xCA, 1);    
	err = I2Cm_ReadBytes(0xc0, &THR_R, 0x81, 0xCB, 1);    
	err = I2Cm_ReadBytes(0xc0, &RAW_L, 0x81, 0xD0, 2);    
	err = I2Cm_ReadBytes(0xc0, &RAW_R, 0x81, 0xD2, 2);    
	err = I2Cm_ReadBytes(0xc0, &BASE_L, 0x81, 0xD4, 2);    
	err = I2Cm_ReadBytes(0xc0, &BASE_R, 0x81, 0xD6, 2);    
	err = I2Cm_ReadBytes(0xc0, &DIFF_L, 0x81, 0xD8, 2);    
	err = I2Cm_ReadBytes(0xc0, &DIFF_R, 0x81, 0xDA, 2);    

	if(err)
		printk("[Touchkey] I2C Error. err=%d\n", err);

	printk("[Touchkey] firmware version 0x%02X\n", firm_ver);
	printk("[Touchkey] Threshold L : 0x%02X, R : 0x%02X\n", THR_L, THR_R);    
	printk("[Touchkey] Raw Data L : 0x%04X, R : 0x%04X\n", RAW_L, RAW_R);        
	printk("[Touchkey] Baseline L : 0x%04X, R : 0x%04X\n", BASE_L, BASE_R);        
	printk("[Touchkey] Difference L : 0x%04X, R : 0x%04X\n", DIFF_L, DIFF_R);        


    if (firm_ver < TKEY_LATEST_FIRMWARE)
    {
    	disable_irq_nosync(KEY_INT);
        TKey_Firmware_Update();
        enable_irq(KEY_INT);
    }

    hrtimer_start(&nextchip_touchkey_driver->timer, ktime_set(5, 0), HRTIMER_MODE_REL);    

	return 0;
}

static const struct i2c_device_id nextchip_touchkey_id[] = {
	{"nextchip_touchkey", 0},
	{}
};


struct i2c_driver touchkey_i2c_driver = {
	.driver = {
		   .name = "nextchip_touchkey_driver",
		   },
	.id_table = nextchip_touchkey_id,
	.probe = i2c_touchkey_probe,
};


static int __init touchkey_init(void)
{
	int ret = 0;
    
    if (board_hw_revision >= 2)
        return ret;
/*
    sec_touchkey= device_create(sec_class, NULL, 0, NULL, "sec_touchkey");

	if (IS_ERR(sec_touchkey))
		printk(KERN_ERR "[TSP] Failed to create device(sec_touchkey)!\n");

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_current_firmware_ver)< 0)
		printk(KERN_ERR "[TSP] Failed to create device file(%s)!\n", dev_attr_touchkey_current_firmware_ver.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_recommended_firmware_ver)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_recommended_firmware_ver.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_left_threshold)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_left_threshold.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_right_threshold)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_right_threshold.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_left_rawdata)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_left_rawdata.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_right_rawdata)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_right_rawdata.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_left_baseline)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_left_baseline.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_right_baseline)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_right_baseline.attr.name);
    
	if (device_create_file(sec_touchkey, &dev_attr_touchkey_left_difference)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_left_difference.attr.name);

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_right_difference)< 0)
			printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_touchkey_right_difference.attr.name);

	ret = misc_register(&touchkey_update_device);
	if (ret) {
		printk(KERN_ERR "[TouchKey] %s misc_register fail\n", __func__);
	}
	
	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_brightness) < 0) {
		printk(KERN_ERR
		       "[TouchKey] %s device_create_file fail dev_attr_touch_update\n",
		       __func__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_brightness.attr.name);
	}
*/    
	touchkey_wq = create_singlethread_workqueue("nextchip_touchkey_wq");
	if (!touchkey_wq) {
		return -ENOMEM;
	}
	check_ic_wq = create_singlethread_workqueue("check_ic_wq");	
	if (!check_ic_wq) {
		return -ENOMEM;
	}

	INIT_WORK(&touchkey_work, touchkey_work_func);
	ret = i2c_add_driver(&touchkey_i2c_driver);

	if (ret) {
		printk(KERN_ERR"[TouchKey] Touch keypad registration failed, module not inserted.ret= %d\n", ret);
	}
	printk("[TouchKey] Nextchip Touch Key init\n");
	
	return ret;
}

static void __exit touchkey_exit(void)
{

	i2c_del_driver(&touchkey_i2c_driver);
/*	misc_deregister(&touchkey_update_device);*/

	if (touchkey_wq) {
		destroy_workqueue(touchkey_wq);
	}
	if (check_ic_wq) {
		destroy_workqueue(check_ic_wq);
	}
    
	printk("[TouchKey] Touch Key exit\n");
}

module_init(touchkey_init);
module_exit(touchkey_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("nextchip touch keypad");
