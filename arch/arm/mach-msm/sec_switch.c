/*
 * UART/USB path switching driver for Samsung Electronics devices.
 *
 * Copyright (C) 2010 Samsung Electronics.
 *
 * Authors: Ikkeun Kim <iks.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/regulator/consumer.h>
#include <linux/moduleparam.h>
#include <linux/usb/android_composite.h>
#include <asm/mach/arch.h>
#include <mach/gpio.h>
#include <mach/sec_switch.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

struct sec_switch_struct {
	struct sec_switch_platform_data *pdata;
};

struct sec_switch_wq {
	struct delayed_work work_q;
	struct sec_switch_struct *sdata;
	struct list_head entry;
};

/* for sysfs control (/sys/class/sec/switch/) */
extern struct device *switch_dev;
static ssize_t device_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	struct sec_switch_struct *secsw = dev_get_drvdata(dev);
	int dev_type = secsw->pdata->get_attached_device();

	if (dev_type == DEV_TYPE_NONE)
		return sprintf(buf, "NONE\n");
	else if (dev_type == DEV_TYPE_USB)
		return sprintf(buf, "USB\n");
	else if (dev_type == DEV_TYPE_CHARGER)
		return sprintf(buf, "CHARGER\n");
	else if (dev_type == DEV_TYPE_JIG)
		return sprintf(buf, "JIG\n");
	else
		return sprintf(buf, "UNKNOWN\n");
}

static DEVICE_ATTR(device_type, 0664, device_type_show, NULL);

static void sec_switch_init_work(struct work_struct *work)
{
	struct delayed_work *dw = container_of(work, struct delayed_work, work);
	struct sec_switch_wq *wq = container_of(dw, struct sec_switch_wq, work_q);
	struct sec_switch_struct *secsw = wq->sdata;
	
	if (secsw->pdata) {
		cancel_delayed_work(&wq->work_q);
	} else {
		schedule_delayed_work(&wq->work_q, msecs_to_jiffies(1000));
		return;
	}
}

static int sec_switch_probe(struct platform_device *pdev)
{
	struct sec_switch_struct *secsw;
	struct sec_switch_platform_data *pdata = pdev->dev.platform_data;
	struct sec_switch_wq *wq;

	pr_err("sec_switch_probe enter %s\n", __func__);

	if (!pdata) {
		pr_err("%s : pdata is NULL.\n", __func__);
		return -ENODEV;
	}

	secsw = kzalloc(sizeof(struct sec_switch_struct), GFP_KERNEL);
	if (!secsw) {
		pr_err("%s : failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	secsw->pdata = pdata;

	dev_set_drvdata(switch_dev, secsw);

	/* create sysfs files */
	if (device_create_file(switch_dev, &dev_attr_device_type) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_device_type.attr.name);

	/* run work queue */
	wq = kmalloc(sizeof(struct sec_switch_wq), GFP_ATOMIC);
	if (wq) {
		wq->sdata = secsw;
		INIT_DELAYED_WORK(&wq->work_q, sec_switch_init_work);
		schedule_delayed_work(&wq->work_q, msecs_to_jiffies(100));
	} else
		return -ENOMEM;
	return 0;
}

static int sec_switch_remove(struct platform_device *pdev)
{
	struct sec_switch_struct *secsw = dev_get_drvdata(&pdev->dev);
	kfree(secsw);
	return 0;
}

static struct platform_driver sec_switch_driver = {
	.probe = sec_switch_probe,
	.remove = sec_switch_remove,
	.driver = {
			.name = "sec_switch",
			.owner = THIS_MODULE,
	},
};

static int __init sec_switch_init(void)
{
	return platform_driver_register(&sec_switch_driver);
}

static void __exit sec_switch_exit(void)
{
	platform_driver_unregister(&sec_switch_driver);
}

module_init(sec_switch_init);
module_exit(sec_switch_exit);

MODULE_AUTHOR("Ikkeun Kim <iks.kim@samsung.com>");
MODULE_DESCRIPTION("Samsung Electronics Corp Switch driver");
MODULE_LICENSE("GPL");
