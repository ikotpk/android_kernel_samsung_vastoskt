/*
 *  sec_battery.c
 *  Samsung Mobile Battery Driver
 *
 *  Copyright (C) 2011 Samsung Electronics
 *
 *  <jongmyeong.ko@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/proc_fs.h>
#include <linux/android_alarm.h>
#include <linux/msm_adc.h>
#include <linux/earlysuspend.h>
#include <mach/sec_battery.h>
#include <mach/msm_rpcrouter.h>

#include "../../arch/arm/mach-msm/proc_comm.h"

#define GET_TOPOFF_WITH_REGISTER
//#define CHK_TOPOFF_WITH_REGISTER_ONLY

#define POLLING_INTERVAL	(40 * 1000)
#define MEASURE_DSG_INTERVAL	(20 * 1000)
#define MEASURE_CHG_INTERVAL	(5 * 1000)

/* new concept : in case of time-out charging stop,
  Do not update FULL for UI,
  Use same time-out value for first charing and re-charging
*/
#define FULL_CHARGING_TIME	(6 * 60 * 60 * HZ)	/* 6hr */
#define RECHARGING_TIME		(2 * 60 * 60 * HZ)	/* 2hr */
#if 0
#if defined (CONFIG_TARGET_LOCALE_USA)
#define RECHARGING_TIME		(90 * 60 * HZ)	/* 1.5hr */
#else
#define RECHARGING_TIME		(2 * 60 * 60 * HZ)	/* 2hr */
#endif
#endif

#define RECHARGING_VOLTAGE	4160	/* 4.13 V */
#define HIGH_BLOCK_TEMP_ADC			706
#define HIGH_RECOVER_TEMP_ADC			700
#define EVT_HIGH_BLOCK_TEMP_ADC		707
#define EVT_HIGH_RECOVER_TEMP_ADC	701
#define LOW_BLOCK_TEMP_ADC			514
#define LOW_RECOVER_TEMP_ADC			524

#define NB_HIGH_BLOCK_TEMP_ADC			525
#define NB_HIGH_RECOVER_TEMP_ADC			615
#define NB_EVT_HIGH_BLOCK_TEMP_ADC		250
#define NB_EVT_HIGH_RECOVER_TEMP_ADC	345
#define NB_LOW_BLOCK_TEMP_ADC			1560
#define NB_LOW_RECOVER_TEMP_ADC			1505

#define HIGH_BLOCK_TEMP_ADC_PMICTHERM		656
#define HIGH_RECOVER_TEMP_ADC_PMICTHERM 	603
#define LOW_BLOCK_TEMP_ADC_PMICTHERM		504
#define LOW_RECOVER_TEMP_ADC_PMICTHERM  	514
#define HIGH_BLOCK_TEMP_ADC_SETTHERM		790
#define HIGH_RECOVER_TEMP_ADC_SETTHERM	    618
#define LOW_BLOCK_TEMP_ADC_SETTHERM		 237
#define LOW_RECOVER_TEMP_ADC_SETTHERM	     260

#define CURRENT_OF_FULL_CHG		190 	/* 170mA */

#if defined(CONFIG_MACH_VASTO)
#define FUEL_GAUGE_TEST_ORI_LEVEL1 60
#define FUEL_GAUGE_TEST_ORI_LEVEL2 80
#define FUEL_GAUGE_TEST_ORI_LEVEL3 90
#define FUEL_GAUGE_TEST_ORI_LEVEL4 100
#define FUEL_GAUGE_TEST_COMP_LEVEL1 10
#define FUEL_GAUGE_TEST_COMP_LEVEL2 40
#define FUEL_GAUGE_TEST_COMP_LEVEL3 75
#define FUEL_GAUGE_TEST_COMP_LEVEL4 100
#endif

#if defined(CONFIG_MACH_VASTO)
#define TEMP_ADC_MAX 1024
#endif

#define FG_T_SOC		0
#define FG_T_VCELL		1
/* all count duration = (count - 1) * poll interval */
#define RE_CHG_COND_COUNT	8
#define RE_CHG_MIN_COUNT		2
#define TEMP_BLOCK_COUNT		2
#define BAT_DET_COUNT			2
#define FULL_CHG_COND_COUNT 	2
#define USB_FULL_COND_COUNT		3
#define USB_FULL_COND_VOLTAGE    4150
#define FULL_CHARGE_COND_VOLTAGE    4000
#define INIT_CHECK_COUNT	4

#if defined (CONFIG_TARGET_LOCALE_USA)
/* Offset Bit Value */
#define OFFSET_VIBRATOR_ON		(0x1 << 0)
#define OFFSET_CAMERA_ON		(0x1 << 1)
#define OFFSET_MP3_PLAY			(0x1 << 2)
#define OFFSET_VIDEO_PLAY		(0x1 << 3)
#define OFFSET_VOICE_CALL_2G		(0x1 << 4)
#define OFFSET_VOICE_CALL_3G		(0x1 << 5)
#define OFFSET_DATA_CALL		(0x1 << 6)
#define OFFSET_LCD_ON			(0x1 << 7)
#define OFFSET_TA_ATTACHED		(0x1 << 8)
#define OFFSET_CAM_FLASH		(0x1 << 9)
#define OFFSET_BOOTING			(0x1 << 10)
#define OFFSET_WIFI				(0x1 << 11)
#define OFFSET_GPS				(0x1 << 12)

#define COMPENSATE_VIBRATOR		0
#define COMPENSATE_CAMERA			0
#define COMPENSATE_MP3				0
#define COMPENSATE_VIDEO			0
#define COMPENSATE_VOICE_CALL_2G	0
#define COMPENSATE_VOICE_CALL_3G	0
#define COMPENSATE_DATA_CALL		0
#define COMPENSATE_LCD				0
#define COMPENSATE_TA				0
#define COMPENSATE_CAM_FALSH		0
#define COMPENSATE_BOOTING		0
#define COMPENSATE_WIFI			0
#define COMPENSATE_GPS				0

#define TOTAL_EVENT_TIME  (30*60*1000)  /* 30 minites */

#define EVT_CASE	(OFFSET_MP3_PLAY | OFFSET_VOICE_CALL_2G | \
	OFFSET_VOICE_CALL_3G | OFFSET_DATA_CALL | OFFSET_VIDEO_PLAY |\
                       OFFSET_CAMERA_ON | OFFSET_WIFI | OFFSET_GPS)

static int event_occur = 0;	
static unsigned int event_start_time_msec = 0;
static unsigned int event_total_time_msec = 0;
#endif


#define VOLTAGE_MIN_DESIGN      3400
#define VOLTAGE_MAX_DESIGN      4130


#define __CONTROL_CHARGING_SUDDEN_LEVEL_UP__

#define BATT_LOW_VOLT	3410
#define BATT_LEVEL1_VOLT	3550
#define BATT_LEVEL2_VOLT	3660
#define BATT_LEVEL3_VOLT	3720
#define BATT_LEVEL4_VOLT	3760
#define BATT_LEVEL5_VOLT	3820
#define BATT_LEVEL6_VOLT	3930
#define BATT_FULL_VOLT	4200
#define BATT_RECHAR_VOLT	4160


#define BATT_LOW_ADC 3410
#define BATT_LEVEL1_ADC 3550
#define BATT_LEVEL2_ADC 3660
#define BATT_LEVEL3_ADC 3720
#define BATT_LEVEL4_ADC 3760
#define BATT_LEVEL5_ADC 3820
#define BATT_LEVEL6_ADC 3930
#define BATT_LEVEL7_ADC 4000
#define BATT_LEVEL8_ADC 4100
#define BATT_LEVEL9_ADC 4150
#define BATT_LEVEL10_ADC 4190
#define BATT_FULL_ADC 4200
#define DEBUG 1


#define CELOX_BATTERY_CHARGING_CONTROL 

#if defined (CELOX_BATTERY_CHARGING_CONTROL)
static int is_charging_disabled = 0; // Tells if charging forcefully disabled.
#endif
enum cable_type_t {
	CABLE_TYPE_NONE = 0,
	CABLE_TYPE_USB,
	CABLE_TYPE_AC,
	CABLE_TYPE_MISC,
	CABLE_TYPE_UNKNOWN,
};

enum batt_full_t {
	BATT_NOT_FULL = 0,
	BATT_FULL,
};

enum {
	BAT_NOT_DETECTED,
	BAT_DETECTED
};

static int g_chg_en = 0;
static int prev_scaled_level=0;
static int chg_polling_cnt=0;

extern int fsa_cable_type;
static ssize_t sec_bat_show_property(struct device *dev,
				     struct device_attribute *attr, char *buf);

static ssize_t sec_bat_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count);

int calculate_batt_voltage(int vbatt_adc);

struct battest_info {
	int rechg_count;
	int full_count;
	int test_value;
	int test_esuspend;
	bool is_rechg_state;
};

/*
struct adc_sample {
	int average_adc;
	int adc_arr[ADC_TOTAL_COUNT];
	int index;
};
*/

struct sec_temperature_spec {
	int high_block;
	int high_recovery;
	int low_block;
	int low_recovery;
};

struct sec_bat_info {
	struct device *dev;

	char *charger_name;

	unsigned int adc_arr_size;
	struct sec_bat_adc_table_data *adc_table;

	//unsigned int adc_channel;
	//struct adc_sample temper_adc_sample;

	struct power_supply psy_bat;
	struct power_supply psy_usb;
	struct power_supply psy_ac;

	struct wake_lock vbus_wake_lock;
	struct wake_lock monitor_wake_lock;
	struct wake_lock cable_wake_lock;
	struct wake_lock test_wake_lock;
	struct wake_lock measure_wake_lock;

	enum cable_type_t cable_type;
	enum batt_full_t batt_full_status;

	int adc_channel_main;
	unsigned int batt_temp;	/* Battery Temperature (C) */
	int batt_temp_high_cnt;
	int batt_temp_low_cnt;
	unsigned int batt_health;
	unsigned int batt_vcell;
	unsigned int batt_soc;
	unsigned int batt_presoc;
	unsigned int vf;  
	unsigned int polling_interval;
	unsigned int measure_interval;
	int charging_status;
	//int charging_full_count;

	unsigned int temp;
	unsigned int batt_temp_adc;
	unsigned int batt_temp_aver;
	unsigned int batt_temp_adc_aver;
   	unsigned int batt_vf_adc;
	unsigned int batt_vol_adc;
	unsigned int batt_vol_adc_cal;
	unsigned int batt_vol_aver;
	unsigned int batt_vol_adc_aver;    
	unsigned int batt_current_adc;
	unsigned int present;
	struct battest_info	test_info;
	struct workqueue_struct *monitor_wqueue;
	struct work_struct monitor_work;
	struct delayed_work	cable_work;
	struct delayed_work polling_work;
	struct delayed_work measure_work;
	struct early_suspend bat_early_suspend;
	struct sec_temperature_spec temper_spec;

	unsigned long charging_start_time;
	unsigned long charging_passed_time;
	unsigned int recharging_status;
	unsigned int batt_lpm_state;
	unsigned int is_top_off;
	unsigned int voice_call_state;
	unsigned int full_cond_voltage;
	unsigned int full_cond_count;

	unsigned int (*get_lpcharging_state) (void);
	bool charging_enabled;
	bool is_timeout_chgstop;
	bool is_esus_state;
	bool lpm_chg_mode;
	bool is_rechg_triggered;
	bool dcin_intr_triggered;

	int initial_check_count;
	struct proc_dir_entry *entry;
#if defined (CONFIG_TARGET_LOCALE_USA)
	unsigned int device_state;
#endif

};

struct rpc_req_bat_info {
unsigned int chg_api_version;
unsigned int batt_api_version;

struct msm_rpc_client *batt_client;
struct msm_rpc_endpoint *chg_ep;
} rpc_bat_info;

static char *supply_list[] = {
	"battery",
};

static enum power_supply_property sec_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TECHNOLOGY,
};

static enum power_supply_property sec_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

const char *oem_str[] = {
	"PCOM_OEM_VF_GET",
	"PCOM_OEM_TEMP_GET",
	"PCOM_OEM_TEMP_AVER_GET",
	"PCOM_OEM_BATT_AVER_GET",
	"PCOM_OEM_CHARGE_STATE_GET",
	"PCOM_OEM_SAMSUNG_LAST",
};

static int pm_msm_proc_comm(u32 cmd, u32 *data1, u32 *data2)
{
	pr_debug("%s,\td1=%d,\td2=%d\n",
			oem_str[cmd - PCOM_OEM_VF_GET], *data1, *data2);
	return msm_proc_comm(cmd, data1, data2);
}

static int sec_bat_adc(void)
{
	unsigned int data1 = 0;
	unsigned int data2 = 0;

	msm_proc_comm(PCOM_OEM_BATT_AVER_GET, &data1, &data2);
	//if(data1<BATT_FULL_VOLT)
	//	data1 = data1 - 20;
	return data1;
}

#if 0
static int sec_bat_vol_adc(void)
{
	unsigned int data1 = 0;
	unsigned int data2 = 0;

	msm_proc_comm(PCOM_OEM_BATT_GET_ADC, &data1, &data2);
	return data2;
}
#endif

static int sec_bat_ichg(void)
{
	unsigned int data1 = 0;
	unsigned int data2 = 0;

	msm_proc_comm(PCOM_OEM_VICHG_GET_ADC, &data1, &data2);
	return data2;
}
void sec_bat_ichg_clear(void)
{
	unsigned int data1 = 0;
	unsigned int data2 = 0;

	msm_proc_comm(PCOM_OEM_VICHG_CLEAR_ADC, &data1, &data2);
	printk("%s : VICHG Table is clear \n", __func__);
	return;
}

static int sec_bat_check_vf(struct sec_bat_info *info)
{
	int health = info->batt_health;
	
	if (info->present == 0) {
		if (info->test_info.test_value == 999) {
			printk("%s : test case : %d\n", __func__, info->test_info.test_value);
			health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		} else
			health = POWER_SUPPLY_HEALTH_DEAD;
	} else {
		health = POWER_SUPPLY_HEALTH_GOOD;
	}

	/* update health */
	if (health != info->batt_health) {
		if (health == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE ||
			health == POWER_SUPPLY_HEALTH_DEAD){
			info->batt_health = health;
			printk("%s : vf error update\n", __func__);
		} else if (info->batt_health != POWER_SUPPLY_HEALTH_OVERHEAT &&
			info->batt_health != POWER_SUPPLY_HEALTH_COLD &&
			health == POWER_SUPPLY_HEALTH_GOOD) {
			info->batt_health = health;
			printk("%s : recovery form vf error\n", __func__);
		}
	}
	
	return 0;
}

static int sec_bat_check_detbat(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;
	static int cnt = 0;
	int vf_state = BAT_DETECTED;
	int ret = 0;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}
	
	ret = psy->get_property(psy, POWER_SUPPLY_PROP_PRESENT, &value);
	if (ret < 0) {
		dev_err(info->dev, "%s: fail to get status(%d)\n",
			__func__, ret);
		return -ENODEV;
	}

	if ((info->cable_type != CABLE_TYPE_NONE) &&
		(value.intval == BAT_NOT_DETECTED)) {
		if (cnt <= BAT_DET_COUNT)
			cnt++;
		if(cnt >= BAT_DET_COUNT)
			vf_state = BAT_NOT_DETECTED;
		else
			vf_state = BAT_DETECTED;
	} else {
		vf_state = BAT_DETECTED;
		cnt=0;
	}
	
	if (info->present == 1 &&
		vf_state == BAT_NOT_DETECTED) {
		printk("%s : detbat state(->%d) changed\n",
			__func__, vf_state);
		info->present = 0;
		cancel_work_sync(&info->monitor_work);
		wake_lock(&info->monitor_wake_lock);
		queue_work(info->monitor_wqueue, &info->monitor_work);
	} else if (info->present == 0 &&
		vf_state == BAT_DETECTED) {
		printk("%s : detbat state(->%d) changed\n",
			__func__, vf_state);
		info->present = 1;
		cancel_work_sync(&info->monitor_work);
		wake_lock(&info->monitor_wake_lock);
		queue_work(info->monitor_wqueue, &info->monitor_work);
	}

	return value.intval;
}

static int sec_bat_get_property(struct power_supply *ps,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct sec_bat_info *info = container_of(ps, struct sec_bat_info,
						 psy_bat);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (info->test_info.test_value == 999) {
			printk("%s : test case : %d\n", __func__, info->test_info.test_value);
			val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
		} else if(info->is_timeout_chgstop && 
				  info->charging_status == POWER_SUPPLY_STATUS_FULL) {
			/* new concept : in case of time-out charging stop,
			   Do not update FULL for UI,
			   Use same time-out value for first charing and re-charging
			*/
			val->intval = POWER_SUPPLY_STATUS_CHARGING;
		} else {
			val->intval = info->charging_status;
		}
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = info->batt_health;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = info->present;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = info->batt_temp;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		/* battery is always online */
		/* val->intval = 1; */
		if (info->charging_status == POWER_SUPPLY_STATUS_DISCHARGING &&
			info->cable_type != CABLE_TYPE_NONE) {
			val->intval = CABLE_TYPE_NONE;
		} else
		val->intval = info->cable_type;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = info->batt_vcell;
		if (val->intval == -1)
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (!info->is_timeout_chgstop &&info->charging_status == POWER_SUPPLY_STATUS_FULL) {
			val->intval = 100;
			break;
		}
		val->intval = info->batt_soc;
		if (val->intval == -1)
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int sec_bat_handle_charger_topoff(struct sec_bat_info *info)
{
	struct power_supply *psy =
	    power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;
	int ret = 0;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}
	
	if (info->batt_full_status == BATT_NOT_FULL) {
		info->charging_status = POWER_SUPPLY_STATUS_FULL;
		info->batt_full_status = BATT_FULL;
		info->recharging_status = false;
		info->charging_passed_time = 0;
		info->charging_start_time = 0;
		/* disable charging */
		value.intval = POWER_SUPPLY_STATUS_DISCHARGING;
		ret = psy->set_property(psy, POWER_SUPPLY_PROP_STATUS,
			&value);
		info->charging_enabled = false;
		g_chg_en = 0;
		sec_bat_ichg_clear();
	}
	return ret;
}

static int sec_bat_is_charging(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	ret = psy->get_property(psy, POWER_SUPPLY_PROP_STATUS, &value);
	if (ret < 0) {
		dev_err(info->dev, "%s: fail to get status(%d)\n", __func__,
			ret);
		return ret;
	}

	return value.intval;
}

static int sec_bat_adjust_charging_current(struct sec_bat_info *info, int chg_current)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval val_chg_current;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	val_chg_current.intval = chg_current;

	ret = psy->set_property(psy, POWER_SUPPLY_PROP_CURRENT_ADJ, &val_chg_current);
	if (ret < 0) {
		dev_err(info->dev, "%s: fail to adjust charging current (%d)\n", __func__,
			ret);
		return ret;
	}

	return 0;
}

static int sec_bat_get_charging_current(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval val_chg_current;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	ret = psy->get_property(psy, POWER_SUPPLY_PROP_CURRENT_ADJ, &val_chg_current);
	if (ret < 0) {
		dev_err(info->dev, "%s: fail to charging current (%d)\n", __func__,
			ret);
		return ret;
	}
	
	//printk("%s : retun value = %d\n", __func__, value.intval);
	return val_chg_current.intval;
}

static int sec_bat_is_invalid_bmd(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	ret = psy->get_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	if (ret < 0) {
		dev_err(info->dev, "%s: fail to get online status(%d)\n", __func__,
			ret);
		return ret;
	}
	
	//printk("%s : retun value = %d\n", __func__, value.intval);
	return value.intval;
}

static int sec_bat_set_property(struct power_supply *ps,
				enum power_supply_property psp,
				const union power_supply_propval *val)
{
	struct sec_bat_info *info = container_of(ps, struct sec_bat_info,
						 psy_bat);
	//struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	//union power_supply_propval value;
	int chg_status = 0;
	unsigned long wdelay = msecs_to_jiffies(500);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		chg_status = sec_bat_is_charging(info);
		pr_info("%s: %d\n", __func__, chg_status);
		if (val->intval == POWER_SUPPLY_STATUS_CHARGING) {
			pr_info("%s: charger inserted!!\n", __func__);
			info->dcin_intr_triggered = true;
			cancel_delayed_work(&info->measure_work);
			wake_lock(&info->measure_wake_lock);
			queue_delayed_work(info->monitor_wqueue, &info->measure_work, HZ);
		} else if (val->intval == POWER_SUPPLY_STATUS_DISCHARGING) {
			pr_info("%s: charger removed!!\n", __func__);
			info->dcin_intr_triggered = false;
			cancel_delayed_work(&info->measure_work);
			wake_lock(&info->measure_wake_lock);
			queue_delayed_work(info->monitor_wqueue, &info->measure_work, HZ);
		} else {
			pr_err("%s: unknown chg intr state!!\n", __func__);
			return -EINVAL;
		}
#if 0
		dev_info(info->dev, "%s: topoff intr\n", __func__);
		if (val->intval != POWER_SUPPLY_STATUS_FULL)
			return -EINVAL;

		if (info->batt_full_status == BATT_NOT_FULL) {
			info->recharging_status = false;
			info->batt_full_status = BATT_FULL;
			info->charging_status = POWER_SUPPLY_STATUS_FULL;
			/* disable charging */
			value.intval = POWER_SUPPLY_STATUS_DISCHARGING;
			psy->set_property(psy, POWER_SUPPLY_PROP_STATUS,
					  &value);
			info->charging_enabled = false;
		}
#endif
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		/* TODO: lowbatt interrupt: called by fuel gauge */
		dev_info(info->dev, "%s: lowbatt intr\n", __func__);
		if (val->intval != POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL)
			return -EINVAL;
		wake_lock(&info->monitor_wake_lock);
		queue_work(info->monitor_wqueue, &info->monitor_work);

		break;
	case POWER_SUPPLY_PROP_ONLINE:
		/* cable is attached or detached. called by usb switch ic */
		dev_info(info->dev, "%s: cable was changed(%d)\n", __func__,
			 val->intval);
		switch (val->intval) {
		case POWER_SUPPLY_TYPE_BATTERY:
			info->cable_type = CABLE_TYPE_NONE;
			break;
		case POWER_SUPPLY_TYPE_MAINS:
			info->cable_type = CABLE_TYPE_AC;
			break;
		case POWER_SUPPLY_TYPE_USB:
			info->cable_type = CABLE_TYPE_USB;
			break;
#if 0      
		case POWER_SUPPLY_TYPE_MISC:
			info->cable_type = CABLE_TYPE_MISC;
			break;
#endif      
		default:
			return -EINVAL;
		}
		wake_lock(&info->cable_wake_lock);
		queue_delayed_work(info->monitor_wqueue, &info->cable_work, wdelay);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sec_usb_get_property(struct power_supply *ps,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct sec_bat_info *info = container_of(ps, struct sec_bat_info,
						 psy_usb);

	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	/* Set enable=1 only if the USB charger is connected */
	val->intval = (info->cable_type == CABLE_TYPE_USB);

	return 0;
}

static int sec_ac_get_property(struct power_supply *ps,
			       enum power_supply_property psp,
			       union power_supply_propval *val)
{
	struct sec_bat_info *info = container_of(ps, struct sec_bat_info,
						 psy_ac);

	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	/* Set enable=1 only if the AC charger is connected */
	if (info->charging_status == POWER_SUPPLY_STATUS_DISCHARGING &&
			info->cable_type != CABLE_TYPE_NONE) {
			val->intval = 0;
	} else {
		if(info->cable_type == CABLE_TYPE_MISC){
			if (!info->dcin_intr_triggered)
				val->intval = 0;
			else
				val->intval = 1;
		} else {
	val->intval = (info->cable_type == CABLE_TYPE_AC) ||
					(info->cable_type == CABLE_TYPE_UNKNOWN);
		}
	}

	return 0;
}

#if defined(CONFIG_TARGET_LOCALE_USA)
static int is_over_event_time(void)
{
	unsigned int total_time=0;

	//pr_info("[BAT]:%s\n", __func__);

	if(!event_start_time_msec)
	{
// [junghyunseok edit for CTIA of behold3 20100413	
		return 1;
	}
		
		total_time = TOTAL_EVENT_TIME;

	if(jiffies_to_msecs(jiffies) >= event_start_time_msec)
	{
		event_total_time_msec = jiffies_to_msecs(jiffies) - event_start_time_msec;
	}
	else
	{
		event_total_time_msec = 0xFFFFFFFF - event_start_time_msec + jiffies_to_msecs(jiffies);
	}

	if (event_total_time_msec > total_time && event_start_time_msec)
	{
		pr_info("[BAT]:%s:abs time is over.:event_start_time_msec=%u, event_total_time_msec=%u\n", __func__, event_start_time_msec, event_total_time_msec);
		return 1;
	}	
	else
	{
		return 0;
	}
}

static void sec_set_time_for_event(int mode)
{

	//pr_info("[BAT]:%s\n", __func__);

	if (mode)
	{
		/* record start time for abs timer */
		event_start_time_msec = jiffies_to_msecs(jiffies);
		//pr_info("[BAT]:%s: start_time(%u)\n", __func__, event_start_time_msec);
	}
	else
	{
		/* initialize start time for abs timer */
		event_start_time_msec = 0;
		event_total_time_msec = 0;
		//pr_info("[BAT]:%s: start_time_msec(%u)\n", __func__, event_start_time_msec);
	}
}
#endif

static int sec_bat_check_temper_adc(struct sec_bat_info *info)
{
	int ret = 0;
	int adc_data = 0, adc_deg_data = 0;
	int rescale_adc = 0;
	int health = info->batt_health;
	#if defined(CONFIG_MACH_VASTO)
	int inverse_temp_adc=0;
	#endif

#if defined(CONFIG_MACH_VASTO)
	ret = pm_msm_proc_comm(PCOM_OEM_TEMP_AVER_GET, &adc_data, &adc_deg_data);
#else
	ret = pm_msm_proc_comm(PCOM_OEM_TEMP_GET, &adc_data, &adc_deg_data);
#endif
//	printk("%s: channel : %d, raw adc is %d (temper.)\n",
//		__func__, info->adc_channel_main, adc_deg_data);

	if (ret) {
		pr_err("%s : read error! skip update\n", __func__);
	} else {
		#if defined(CONFIG_MACH_VASTO)
		inverse_temp_adc = TEMP_ADC_MAX - adc_data;
		info->batt_temp_adc = inverse_temp_adc;
		#else
		info->batt_temp_adc = adc_data;
		#endif
		info->batt_temp = adc_deg_data;
	}

	//rescale_adc = info->batt_temp;
	rescale_adc = info->batt_temp_adc;
	
    if (info->test_info.test_value == 1) {
		printk("%s : test case : %d\n", __func__,
			info->test_info.test_value);
        rescale_adc = info->temper_spec.high_block + 1;
        if (info->cable_type == CABLE_TYPE_NONE)
            rescale_adc = info->temper_spec.high_recovery - 1;
		//info->batt_temp = rescale_adc;
		info->batt_temp_adc = rescale_adc;
    }

	if (info->cable_type == CABLE_TYPE_NONE ||
		info->test_info.test_value == 999) {
		info->batt_temp_high_cnt = 0;
		info->batt_temp_low_cnt = 0;
		health = POWER_SUPPLY_HEALTH_GOOD;
		goto skip_hupdate;
	}
	
	if (rescale_adc >= info->temper_spec.high_block) {
		if (health != POWER_SUPPLY_HEALTH_OVERHEAT)
			if (info->batt_temp_high_cnt <= TEMP_BLOCK_COUNT)
				info->batt_temp_high_cnt++;
	} else if (rescale_adc <= info->temper_spec.high_recovery &&
		rescale_adc >= info->temper_spec.low_recovery) {
		if (health == POWER_SUPPLY_HEALTH_OVERHEAT ||
		    health == POWER_SUPPLY_HEALTH_COLD) {
			info->batt_temp_high_cnt = 0;
			info->batt_temp_low_cnt = 0;
		}
	} else if (rescale_adc <= info->temper_spec.low_block) {
		if (health != POWER_SUPPLY_HEALTH_COLD)
			if (info->batt_temp_low_cnt <= TEMP_BLOCK_COUNT)
				info->batt_temp_low_cnt++;
	}

	if (info->batt_temp_high_cnt >= TEMP_BLOCK_COUNT)
		health = POWER_SUPPLY_HEALTH_OVERHEAT;
	else if (info->batt_temp_low_cnt >= TEMP_BLOCK_COUNT)
		health = POWER_SUPPLY_HEALTH_COLD;
	else
		health = POWER_SUPPLY_HEALTH_GOOD;
skip_hupdate:
	if (info->batt_health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE &&
		info->batt_health != POWER_SUPPLY_HEALTH_DEAD &&
		health != info->batt_health) {
		info->batt_health = health;
		cancel_work_sync(&info->monitor_work);
		wake_lock(&info->monitor_wake_lock);
		queue_work(info->monitor_wqueue, &info->monitor_work);
	}

	return 0;
}

static void check_chgcurrent(struct sec_bat_info *info)
{
	int ichg;

	if(info->charging_enabled)
		ichg = sec_bat_ichg();
	else
		ichg = 0;
	//if(ichg < 50)
	//	ichg = 0;

	info->batt_current_adc = ichg;
	printk("%s: ichg is %d\n", __func__, info->batt_current_adc);
#if 0
	ret = sec_bat_read_adc(info, CHANNEL_ADC_CHG_MONITOR,
								&adc_data, &adc_physical);
	//printk("%s: channel : %d, raw adc is %d, result is %d\n",
	//	__func__, CHANNEL_ADC_CHG_MONITOR, adc_data, adc_physical);

	if (ret)
		adc_physical = info->batt_current_adc;
	info->batt_current_adc = adc_physical;

	//printk("[chg_cur] %d, %d\n", info->batt_current_adc, chg_current_adc);
	dev_dbg(info->dev,
		"[battery] charging current = %d\n", info->batt_current_adc);
#endif  
}


static void sec_check_chgcurrent(struct sec_bat_info *info)
{
#if defined(GET_TOPOFF_WITH_REGISTER)
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;
	int ret;
#endif
	static int cnt = 0;
#if 1 //ifdef ADC_QUEUE_FEATURE
	bool is_full_condition = false;
#endif
	
    if (info->charging_enabled) {
        check_chgcurrent(info);
#if 0 //ifndef ADC_QUEUE_FEATURE
		/* AGAIN_FEATURE */
		if (info->batt_current_adc <= CURRENT_OF_FULL_CHG)
			check_chgcurrent(info);
#endif

		//if (info->batt_vcell >= FULL_CHARGE_COND_VOLTAGE) {
		if (info->batt_vcell >= info->full_cond_voltage) {
#if defined(GET_TOPOFF_WITH_REGISTER)
			/* check full state with smb328a register */
			/* check 36h bit6 */
			ret = psy->get_property(psy, POWER_SUPPLY_PROP_CHARGE_FULL, &value);
			if (ret < 0) {
				dev_err(info->dev, "%s: fail to get charge full(%d)\n", __func__,
					ret);
				return;
			}

			if (info->test_info.test_value == 3)
				value.intval = 0;
		
			info->is_top_off = value.intval;
#endif /* GET_TOPOFF_WITH_REGISTER */

#if defined(CHK_TOPOFF_WITH_REGISTER_ONLY)
			if (info->is_top_off==1)
#else /* mainly, check topoff with vichg adc value */
			if (info->test_info.test_value == 3) {
				info->batt_current_adc = CURRENT_OF_FULL_CHG + 1;
			}
#if 1//ifdef ADC_QUEUE_FEATURE
			/* if ((info->batt_current_adc <= CURRENT_OF_FULL_CHG) ||
			       (info->is_adc_wq_freezed && info->is_top_off==1)) */
			if //(info->is_adc_wq_freezed || !info->is_adc_ok ||
				((info->batt_current_adc == 0)) {
				if (info->is_top_off==1) {
					is_full_condition = true;
					//printk("%s : is_top_off (%d, %d, %d)\n",
					//	__func__, info->is_adc_wq_freezed, info->is_adc_ok,
					//	info->batt_current_adc);
					printk("%s : is_top_off (%d)\n",__func__, info->batt_current_adc);
				} else {
					is_full_condition = false;
				}
			} else { /* adc data is ok */
				if (info->batt_current_adc <= CURRENT_OF_FULL_CHG) {
					is_full_condition = true;
				} else {
					is_full_condition = false;
				}
			}

			if (is_full_condition)
#else
			if (info->batt_current_adc <= CURRENT_OF_FULL_CHG)
#endif /* ADC_QUEUE_FEATURE */
#endif /* CHK_TOPOFF_WITH_REGISTER_ONLY */
			{
				cnt++; /* accumulated counting */
				printk("%s : full state? %d, %d\n", __func__, info->batt_current_adc, cnt);
				//if (cnt >= FULL_CHG_COND_COUNT) {
				if (cnt >= info->full_cond_count) {
					printk("%s : full state!! %d/%d\n", __func__, cnt, FULL_CHG_COND_COUNT);
					//printk("%s : full state!! %d/%d\n", __func__, cnt, info->full_cond_count);
					sec_bat_handle_charger_topoff(info);
					cnt = 0;
				}
			}
		} else {
			cnt = 0;
		}
	} else {
		cnt = 0;
		info->batt_current_adc = 0;
	}
	info->test_info.full_count = cnt;
}

#if 0
static void sec_check_chgcurrent(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval value;
	int ret;
	static int cnt = 0;

	bool is_full_condition = false;
	
    if (info->charging_enabled) {
        check_chgcurrent(info); /* just check it */

		/* check full state with smb328a register */
		/* check 36h bit6 */
		ret = psy->get_property(psy, POWER_SUPPLY_PROP_CHARGE_FULL, &value);
		if (ret < 0) {
			dev_err(info->dev, "%s: fail to get charge full(%d)\n", __func__,
				ret);
			return;
		}

		if (info->test_info.test_value == 3)
			value.intval = 0;
		
		info->is_top_off = value.intval;
		if (info->is_top_off==1) {
			if (info->batt_vcell >= FULL_CHARGE_COND_VOLTAGE) {
				cnt++;
				printk("%s : full state? %d, %d\n", __func__, info->batt_current_adc, cnt);
				if (cnt >= FULL_CHG_COND_COUNT) {
					printk("%s : full state!! %d/%d\n", __func__, cnt, FULL_CHG_COND_COUNT);
					sec_bat_handle_charger_topoff(info);
					cnt = 0;
				}
			}
		} else
			cnt = 0;
	} else {
		cnt = 0;
		info->batt_current_adc = 0;
	}
	info->test_info.full_count = cnt;
}

#endif

static int sec_check_recharging(struct sec_bat_info *info)
{
	static int cnt = 0;
	int ret;

	if (info->charging_status != POWER_SUPPLY_STATUS_FULL ||
		info->recharging_status != false) {
		cnt = 0;
		return 0;
	}

	info->batt_vcell = calculate_batt_voltage(sec_bat_adc());
	
	if (info->batt_vcell > RECHARGING_VOLTAGE) {
		cnt = 0;
		return 0;
	} else {
		/* AGAIN_FEATURE */
		//info->batt_vcell = sec_bat_adc();
		//info->batt_vcell = calculate_batt_voltage(sec_bat_adc());
		//info->batt_vol_adc = sec_bat_vol_adc();
		if (info->batt_vcell <= RECHARGING_VOLTAGE) {
			cnt++;
			printk("%s : rechg condition ? %d\n", __func__, cnt);
			if (cnt >= RE_CHG_COND_COUNT) {
				printk("%s : rechg condition(1) OK - %d\n", __func__, cnt);
				cnt = 0;
				info->test_info.is_rechg_state = true;
				ret = 1;
			} else if (cnt >= RE_CHG_MIN_COUNT && info->batt_vcell <= FULL_CHARGE_COND_VOLTAGE) {
				printk("%s : rechg condition(2) OK - %d\n", __func__, cnt);
				cnt = 0;
				info->test_info.is_rechg_state = true;
				ret = 1;
			} else
				ret = 0;
		} else {
			cnt = 0;
			ret = 0;
		}
	}
	info->test_info.rechg_count = cnt;
	
	return ret;
}

static int sec_bat_notify_vcell2charger(struct sec_bat_info *info)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval val_vcell;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	/* Notify Voltage Now */
	val_vcell.intval = info->batt_vcell;
	ret = psy->set_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW,
				&val_vcell);
	if (ret) {
		dev_err(info->dev, "%s: fail to notify vcell now(%d)\n",
			__func__, ret);
		return ret;
	}
	
	return 0;
}

#if 0
static int msm_batt_capacity(u32 current_voltage)
{
	u32 low_voltage = VOLTAGE_MIN_DESIGN;
	u32 high_voltage = VOLTAGE_MAX_DESIGN;

	if (current_voltage <= low_voltage)
		return 0;
	else if (current_voltage >= high_voltage)
		return 100;
	else
		return (current_voltage - low_voltage) * 100
			/ (high_voltage - low_voltage);
}
#endif


int calculate_batt_level(struct sec_bat_info *info)
{
	int scaled_level = 0;
#if 0
	int scaled_level_new = 0;
#endif
	int batt_volt = info->batt_vcell;
if (g_chg_en==0)
{
	chg_polling_cnt=0;
	if(batt_volt >= BATT_RECHAR_VOLT) //100%
	{
		scaled_level = 100;
		//if ((prev_scaled_level<scaled_level) && (prev_scaled_level!=0))
		//	scaled_level=prev_scaled_level;
	}
	else if(batt_volt >=  BATT_LEVEL6_VOLT) //99% ~ 80%
	{
		scaled_level = ((batt_volt -BATT_LEVEL6_VOLT+1)*19)/(BATT_RECHAR_VOLT-BATT_LEVEL6_VOLT);
		scaled_level = scaled_level+80;
		//if ((prev_scaled_level<scaled_level) && (prev_scaled_level!=0))
		//	scaled_level=prev_scaled_level;
	}
	else if(batt_volt >= BATT_LEVEL5_VOLT) //79% ~ 65%
	{
		scaled_level = ((batt_volt -BATT_LEVEL5_VOLT)*15)/(BATT_LEVEL6_VOLT-BATT_LEVEL5_VOLT);
		scaled_level = scaled_level+65;
	}
	else if(batt_volt >= BATT_LEVEL4_VOLT) //64% ~ 50%
	{
		scaled_level = ((batt_volt -BATT_LEVEL4_VOLT)*15)/(BATT_LEVEL5_VOLT-BATT_LEVEL4_VOLT);
		scaled_level = scaled_level+50;
	}
	else if(batt_volt >= BATT_LEVEL3_VOLT) //49% ~ 35%
	{
		scaled_level = ((batt_volt -BATT_LEVEL3_VOLT)*15)/(BATT_LEVEL4_VOLT-BATT_LEVEL3_VOLT);
		scaled_level = scaled_level+35;
	}
	else if(batt_volt >= BATT_LEVEL2_VOLT) //34% ~ 20%
	{
		scaled_level = ((batt_volt -BATT_LEVEL2_VOLT)*15)/(BATT_LEVEL3_VOLT-BATT_LEVEL2_VOLT);
		scaled_level = scaled_level+20;
	}
	else if(batt_volt >= BATT_LEVEL1_VOLT) //19% ~ 5%
	{
		scaled_level = ((batt_volt -BATT_LEVEL1_VOLT)*15)/(BATT_LEVEL2_VOLT-BATT_LEVEL1_VOLT);
		scaled_level = scaled_level+5;
	}
	else if(batt_volt > BATT_LOW_VOLT) //4% ~ 1%
	{
		scaled_level = ((batt_volt -BATT_LOW_VOLT)*4)/(BATT_LEVEL1_VOLT-BATT_LOW_VOLT);
		scaled_level = scaled_level+1;
	}
	else
	{
		if( info->cable_type == CABLE_TYPE_NONE ) scaled_level = 0; 
		else scaled_level = 1;
	}

}
else
{

	if(batt_volt >= BATT_RECHAR_VOLT) //100%
	{
		if (prev_scaled_level>=91)
		{
			if (prev_scaled_level>=91 && prev_scaled_level<94)
				scaled_level=91;
			if (prev_scaled_level>=94 && prev_scaled_level<97)
				scaled_level=94;
			if (prev_scaled_level>=97 && prev_scaled_level<100)
				scaled_level=97;
			if (prev_scaled_level==100)
				scaled_level=100;
	
			if (chg_polling_cnt!=0)
			{
				if (scaled_level!=100)
				{
					//if ((chg_polling_cnt%60)==0)
					if ((chg_polling_cnt%15)==0)
						scaled_level+=3;
					if (scaled_level==100)
							chg_polling_cnt=0;
				}
			}
		}
		else
		{
			scaled_level=91;
		}
	}


	else if(batt_volt >=  BATT_LEVEL6_VOLT) //99% ~ 80%
	{
		scaled_level = ((batt_volt -BATT_LEVEL6_VOLT+1)*10)/(BATT_RECHAR_VOLT-BATT_LEVEL6_VOLT);
 		scaled_level = scaled_level+80;

	if (prev_scaled_level>scaled_level)
 			scaled_level=prev_scaled_level;

	}
	else if(batt_volt >= BATT_LEVEL5_VOLT) //79% ~ 65%
	{
		scaled_level = ((batt_volt -BATT_LEVEL5_VOLT)*15)/(BATT_LEVEL6_VOLT-BATT_LEVEL5_VOLT);
 		scaled_level = scaled_level+65;
	}
	else if(batt_volt >= BATT_LEVEL4_VOLT) //64% ~ 50%
	{
		scaled_level = ((batt_volt -BATT_LEVEL4_VOLT)*15)/(BATT_LEVEL5_VOLT-BATT_LEVEL4_VOLT);
 		scaled_level = scaled_level+50;
	}
	else if(batt_volt >= BATT_LEVEL3_VOLT) //49% ~ 35%
	{
		scaled_level = ((batt_volt -BATT_LEVEL3_VOLT)*15)/(BATT_LEVEL4_VOLT-BATT_LEVEL3_VOLT);
		scaled_level = scaled_level+35;
	}
	else if(batt_volt >= BATT_LEVEL2_VOLT) //34% ~ 20%
	{
		scaled_level = ((batt_volt -BATT_LEVEL2_VOLT)*15)/(BATT_LEVEL3_VOLT-BATT_LEVEL2_VOLT);
 		scaled_level = scaled_level+20;
	}
	else if(batt_volt >= BATT_LEVEL1_VOLT) //19% ~ 5%
	{
		scaled_level = ((batt_volt -BATT_LEVEL1_VOLT)*15)/(BATT_LEVEL2_VOLT-BATT_LEVEL1_VOLT);
 		scaled_level = scaled_level+5;
	}
	else if(batt_volt > BATT_LOW_VOLT) //4% ~ 1%
	{
		scaled_level = ((batt_volt -BATT_LOW_VOLT)*4)/(BATT_LEVEL1_VOLT-BATT_LOW_VOLT);
 		scaled_level = scaled_level+1;
	}
	else
	{
		if( info->cable_type == CABLE_TYPE_NONE ) scaled_level = 0; 
		else scaled_level = 1;
	}

	}
	prev_scaled_level=scaled_level;
  	return scaled_level;
}

#define BATT_CAL_CHG 310

int calculate_batt_voltage(int vbatt_adc)
{
	int batt_volt = 0;
	static int prevVal = 0;
	int chg_comp=0;

	int ichg_comp=0;
	ichg_comp = sec_bat_ichg();
	
#ifdef __CONTROL_CHARGING_SUDDEN_LEVEL_UP__
	if(!prevVal)
	prevVal = vbatt_adc;


	if(vbatt_adc >=BATT_LEVEL10_ADC){
		if(ichg_comp>880)
			chg_comp = BATT_CAL_CHG;
		else
			chg_comp = (ichg_comp - CURRENT_OF_FULL_CHG -160)/2;
		}
	//if((vbatt_adc < (BATT_FULL_ADC-10)) && (vbatt_adc > BATT_LEVEL10_ADC))
	//	chg_comp = BATT_CAL_CHG;
	 if((vbatt_adc <= BATT_LEVEL10_ADC) && (vbatt_adc > BATT_LEVEL9_ADC))
		chg_comp = BATT_CAL_CHG;
        if((vbatt_adc <= BATT_LEVEL9_ADC) && (vbatt_adc > BATT_LEVEL8_ADC))
		chg_comp = BATT_CAL_CHG+10;
        if((vbatt_adc <= BATT_LEVEL8_ADC) && (vbatt_adc > BATT_LEVEL7_ADC))
		chg_comp = BATT_CAL_CHG+20;
       if((vbatt_adc <= BATT_LEVEL7_ADC) && (vbatt_adc > BATT_LEVEL6_ADC))
		chg_comp = BATT_CAL_CHG+20;
	if((vbatt_adc <= BATT_LEVEL6_ADC) && (vbatt_adc > BATT_LEVEL5_ADC))
		chg_comp = BATT_CAL_CHG+20;
	if((vbatt_adc <= BATT_LEVEL5_ADC) && (vbatt_adc > BATT_LEVEL4_ADC))
		chg_comp = BATT_CAL_CHG+20;
	if((vbatt_adc <= BATT_LEVEL4_ADC) && (vbatt_adc > BATT_LEVEL3_ADC))
		chg_comp = BATT_CAL_CHG+20;
	if((vbatt_adc <= BATT_LEVEL3_ADC) && (vbatt_adc > BATT_LEVEL2_ADC))
		chg_comp = BATT_CAL_CHG+20;
	if((vbatt_adc <= BATT_LEVEL2_ADC) && (vbatt_adc > BATT_LEVEL1_ADC))
		chg_comp = BATT_CAL_CHG+20;
	if(vbatt_adc <= BATT_LEVEL1_ADC)
		chg_comp = BATT_CAL_CHG+20;

		if(g_chg_en)
		{
			if( prevVal < (vbatt_adc-chg_comp))
			{	
				vbatt_adc = vbatt_adc-chg_comp;		
				//printk("[Battery] vbatt_adc-BATT_CAL_CHG \n");
			}	
			else
			{
				vbatt_adc = prevVal;
				//printk("[Battery] chg_en & prevVal \n");
			}	
		}
		else
		{
			if(vbatt_adc<RECHARGING_VOLTAGE){
			if(prevVal<vbatt_adc)
			{
				vbatt_adc = prevVal;
				//printk("[Battery] prevVal \n");
			}
		}
	}
	prevVal = vbatt_adc;
#endif	
	
	//dev_err("[Battery] %s : vbatt_adc %d \n", __func__, vbatt_adc);

	
	if(vbatt_adc >= BATT_FULL_ADC)
	{
		batt_volt = BATT_FULL_VOLT;
	}
	else if(vbatt_adc >=  BATT_LEVEL6_ADC) //4.200v ~ 3.990v
	{
		batt_volt = ((vbatt_adc -BATT_LEVEL6_ADC)*(BATT_FULL_VOLT-BATT_LEVEL6_VOLT))/(BATT_FULL_ADC-BATT_LEVEL6_ADC);
 		batt_volt = batt_volt+BATT_LEVEL6_VOLT;
	}
	else if(vbatt_adc >=  BATT_LEVEL5_ADC) //3.990v ~ 3.860v
	{
		batt_volt = ((vbatt_adc -BATT_LEVEL5_ADC)*(BATT_LEVEL6_VOLT-BATT_LEVEL5_VOLT))/(BATT_LEVEL6_ADC-BATT_LEVEL5_ADC);
 		batt_volt = batt_volt+BATT_LEVEL5_VOLT;
	}
	else if(vbatt_adc >=  BATT_LEVEL4_ADC) //3.860v ~ 3.760v
	{
		batt_volt = ((vbatt_adc -BATT_LEVEL4_ADC)*(BATT_LEVEL5_VOLT-BATT_LEVEL4_VOLT))/(BATT_LEVEL5_ADC-BATT_LEVEL4_ADC);
 		batt_volt = batt_volt+BATT_LEVEL4_VOLT;
	}
	else if(vbatt_adc >=  BATT_LEVEL3_ADC) //3.760v ~ 3.700v
	{
		batt_volt = ((vbatt_adc -BATT_LEVEL3_ADC)*(BATT_LEVEL4_VOLT-BATT_LEVEL3_VOLT))/(BATT_LEVEL4_ADC-BATT_LEVEL3_ADC);
 		batt_volt = batt_volt+BATT_LEVEL3_VOLT;
	}
	else if(vbatt_adc >=  BATT_LEVEL2_ADC) //3.700v ~ 3.640v
	{
		batt_volt = ((vbatt_adc -BATT_LEVEL2_ADC)*(BATT_LEVEL3_VOLT-BATT_LEVEL2_VOLT))/(BATT_LEVEL3_ADC-BATT_LEVEL2_ADC);
 		batt_volt = batt_volt+BATT_LEVEL2_VOLT;
	}
	else if(vbatt_adc >=  BATT_LEVEL1_ADC) //3.640v ~ 3.600v
	{
		batt_volt = ((vbatt_adc -BATT_LEVEL1_ADC)*(BATT_LEVEL2_VOLT-BATT_LEVEL1_VOLT))/(BATT_LEVEL2_ADC-BATT_LEVEL1_ADC);
 		batt_volt = batt_volt+BATT_LEVEL1_VOLT;
	}
	else if(vbatt_adc >  BATT_LOW_ADC) //3.600v ~ 3.400v
	{
		batt_volt = ((vbatt_adc -BATT_LOW_ADC)*(BATT_LEVEL1_VOLT-BATT_LOW_VOLT))/(BATT_LEVEL1_ADC-BATT_LOW_ADC);
 		batt_volt = batt_volt+BATT_LOW_VOLT;
	}
	else batt_volt = BATT_LOW_VOLT;

	//dev_err("[Battery] %s : vbatt_adc %d & batt_volt %d\n", __func__, vbatt_adc, batt_volt);

	return batt_volt;
}

static void sec_bat_update_info(struct sec_bat_info *info)
{  
	info->batt_presoc = info->batt_soc;
  	info->batt_vcell = calculate_batt_voltage(sec_bat_adc());
	info->batt_soc = calculate_batt_level(info);
	sec_bat_notify_vcell2charger(info);

	if (info->batt_vcell >= BATT_RECHAR_VOLT)
		chg_polling_cnt++;
	else
		chg_polling_cnt=0;
}

static int sec_bat_enable_charging(struct sec_bat_info *info, bool enable)
{
	struct power_supply *psy = power_supply_get_by_name(info->charger_name);
	union power_supply_propval val_type, val_chg_current, val_topoff, val_vcell;
	int ret;

	if (!psy) {
		dev_err(info->dev, "%s: fail to get charger ps\n", __func__);
		return -ENODEV;
	}

	info->batt_full_status = BATT_NOT_FULL;

	if (enable) {		/* Enable charging */
		switch (info->cable_type) {
		case CABLE_TYPE_USB:
			val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
			val_chg_current.intval = 500;	/* USB 500 mode */
			info->full_cond_count = USB_FULL_COND_COUNT;
			info->full_cond_voltage = USB_FULL_COND_VOLTAGE;
			break;
		case CABLE_TYPE_AC:
			val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
			val_chg_current.intval = 1000;	/* input : 1000mA, output : 1000mA */
			info->full_cond_count = FULL_CHG_COND_COUNT;
			info->full_cond_voltage = FULL_CHARGE_COND_VOLTAGE;
			break;
		case CABLE_TYPE_MISC:
			val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
			val_chg_current.intval = 700;	/* input : 700, output : 700mA */
			info->full_cond_count = FULL_CHG_COND_COUNT;
			info->full_cond_voltage = FULL_CHARGE_COND_VOLTAGE;
			break;
		case CABLE_TYPE_UNKNOWN:
			val_type.intval = POWER_SUPPLY_STATUS_CHARGING;
			val_chg_current.intval = 450;	/* input : 450, output : 500mA */
			info->full_cond_count = USB_FULL_COND_COUNT;
			info->full_cond_voltage = USB_FULL_COND_VOLTAGE;
			break;
		default:
			dev_err(info->dev, "%s: Invalid func use\n", __func__);
			return -EINVAL;
		}

		/* Set charging current */
		ret = psy->set_property(psy, POWER_SUPPLY_PROP_CURRENT_NOW,
					&val_chg_current);
		if (ret) {
			dev_err(info->dev, "%s: fail to set charging cur(%d)\n",
				__func__, ret);
			return ret;
		}

		/* Set topoff current */
		/* from 25mA to 200mA, in 25mA step */
		val_topoff.intval = 200;
		ret = psy->set_property(psy, POWER_SUPPLY_PROP_CHARGE_FULL,
					&val_topoff);
		if (ret) {
			dev_err(info->dev, "%s: fail to set topoff val(%d)\n",
				__func__, ret);
			return ret;
		}

		/* Notify Voltage Now */
		info->batt_vcell = calculate_batt_voltage(sec_bat_adc());
		val_vcell.intval = info->batt_vcell;
		ret = psy->set_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW,
					&val_vcell);
		if (ret) {
			dev_err(info->dev, "%s: fail to notify vcell now(%d)\n",
				__func__, ret);
			return ret;
		}

		/* Adjust Charging Current */
		if (info->lpm_chg_mode &&
			info->cable_type == CABLE_TYPE_AC) {
			printk("%s : lpm-mode, adjust charging current (to 1A)\n",
				__func__);
			sec_bat_adjust_charging_current(info, 1000); /* 1A */
		}

		info->charging_start_time = jiffies;
	} else {		/* Disable charging */
		val_type.intval = POWER_SUPPLY_STATUS_DISCHARGING;
		info->charging_passed_time = 0;
		info->charging_start_time = 0;
	}

	ret = psy->set_property(psy, POWER_SUPPLY_PROP_STATUS, &val_type);
	if (ret) {
		dev_err(info->dev, "%s: fail to set charging status(%d)\n",
			__func__, ret);
		return ret;
	}

	info->charging_enabled = enable;

	return 0;
}

static void sec_bat_handle_unknown_disable(struct sec_bat_info *info)
{
	printk(" %s : cable_type = %d\n", __func__, info->cable_type);
		
	info->batt_full_status = BATT_NOT_FULL;
	info->recharging_status = false;
	info->test_info.is_rechg_state = false;
	info->charging_start_time = 0;
	info->charging_status = POWER_SUPPLY_STATUS_DISCHARGING;
	info->is_timeout_chgstop = false;
	sec_bat_enable_charging(info, false);
	g_chg_en=0;
	sec_bat_ichg_clear();

	//power_supply_changed(&info->psy_ac);
	//power_supply_changed(&info->psy_usb);
	power_supply_changed(&info->psy_bat);
}

static void sec_bat_cable_work(struct work_struct *work)
{
	struct sec_bat_info *info = container_of(work, struct sec_bat_info,
							cable_work.work);

	/*
	if (info->cable_type == CABLE_TYPE_UNKNOWN)
		info->cable_type = CABLE_TYPE_NONE;
	if (info->cable_type != CABLE_TYPE_NONE) {
		sec_bat_enable_charging(info, true);
		info->cable_type = CABLE_TYPE_NONE;
	}
	*/
		
	switch (info->cable_type) {
	case CABLE_TYPE_NONE:
		/* TODO : check DCIN state again*/
#if 1
		if ((sec_bat_is_charging(info) == POWER_SUPPLY_STATUS_CHARGING) &&
			info->dcin_intr_triggered) {
			printk("cable none : vdcin ok, skip!!!\n");
			return;
		}
#endif
		info->batt_full_status = BATT_NOT_FULL;
		info->recharging_status = false;
		info->test_info.is_rechg_state = false;
		info->charging_start_time = 0;
		info->charging_status = POWER_SUPPLY_STATUS_DISCHARGING;
		info->is_timeout_chgstop = false;
		sec_bat_enable_charging(info, false);
		wake_lock_timeout(&info->vbus_wake_lock, 5 * HZ);
		cancel_delayed_work(&info->measure_work);
		info->measure_interval = MEASURE_DSG_INTERVAL;
		wake_lock(&info->measure_wake_lock);
		queue_delayed_work(info->monitor_wqueue, &info->measure_work, HZ/2);
		//schedule_delayed_work(&info->measure_work, 0);
		g_chg_en=0;
		sec_bat_ichg_clear();
		break;
	case CABLE_TYPE_MISC:
		if (!info->dcin_intr_triggered && !info->lpm_chg_mode) {
			wake_lock_timeout(&info->vbus_wake_lock, 5 * HZ);
			printk("%s : dock inserted, but dcin nok skip charging!\n",
					__func__);
			sec_bat_enable_charging(info, true);
			info->charging_enabled = false;
			g_chg_en=1;
			break;
		}
	case CABLE_TYPE_USB:
	case CABLE_TYPE_AC:
	case CABLE_TYPE_UNKNOWN:
		/* TODO : check DCIN state again*/
		info->charging_status = POWER_SUPPLY_STATUS_CHARGING;
		sec_bat_enable_charging(info, true);
		wake_lock(&info->vbus_wake_lock);
		cancel_delayed_work(&info->measure_work);
		info->measure_interval = MEASURE_CHG_INTERVAL;
		wake_lock(&info->measure_wake_lock);
		queue_delayed_work(info->monitor_wqueue, &info->measure_work, HZ/2);
		//schedule_delayed_work(&info->measure_work, 0);
		g_chg_en=1;
		sec_bat_ichg_clear();
		break;
	default:
		dev_err(info->dev, "%s: Invalid cable type\n", __func__);
		break;;
	}

	//power_supply_changed(&info->psy_ac);
	//power_supply_changed(&info->psy_usb);
	power_supply_changed(&info->psy_bat);
	/* TBD */
	//wake_lock(&info->monitor_wake_lock);
	//queue_work(info->monitor_wqueue, &info->monitor_work);

	wake_unlock(&info->cable_wake_lock);
}

static void sec_bat_charging_time_management(struct sec_bat_info *info)
{
	unsigned long charging_time;

	if (info->charging_start_time == 0) {
		dev_dbg(info->dev, "%s: charging_start_time has never\
			 been used since initializing\n", __func__);
		return;
	}

	if (jiffies >= info->charging_start_time)
		charging_time = jiffies - info->charging_start_time;
	else
		charging_time = 0xFFFFFFFF - info->charging_start_time
		    + jiffies;

	info->charging_passed_time = charging_time;
	
	switch (info->charging_status) {
	case POWER_SUPPLY_STATUS_FULL:
		if (time_after(charging_time, (unsigned long)RECHARGING_TIME) &&
		    info->recharging_status == true) {
			sec_bat_enable_charging(info, false);
			info->recharging_status = false;
			info->is_timeout_chgstop = true;
			g_chg_en = 0;
			sec_bat_ichg_clear();
			dev_info(info->dev, "%s: Recharging timer expired\n",
				 __func__);
		}
		break;
	case POWER_SUPPLY_STATUS_CHARGING:
		if (time_after(charging_time,
			       (unsigned long)FULL_CHARGING_TIME)) {
			sec_bat_enable_charging(info, false);
			info->charging_status = POWER_SUPPLY_STATUS_FULL;
			info->is_timeout_chgstop = true;
			g_chg_en = 0;
			sec_bat_ichg_clear();

			dev_info(info->dev, "%s: Charging timer expired\n",
				 __func__);
		}
		break;
	default:
		info->is_timeout_chgstop = false;
		dev_info(info->dev, "%s: Undefine Battery Status\n", __func__);
		return;
	}

	dev_dbg(info->dev, "Time past : %u secs\n",
		jiffies_to_msecs(charging_time) / 1000);

	return;
}


static void sec_bat_monitor_work(struct work_struct *work)
{
	struct sec_bat_info *info = container_of(work, struct sec_bat_info,
						 monitor_work);

	wake_lock(&info->monitor_wake_lock);

	sec_bat_charging_time_management(info);

	sec_bat_update_info(info);
	sec_bat_check_vf(info);
	sec_check_chgcurrent(info);

	switch (info->charging_status) {
	case POWER_SUPPLY_STATUS_FULL:
		
		/* if (sec_check_recharging(info) && */
		if (info->is_rechg_triggered &&
		    info->recharging_status == false) {
			info->recharging_status = true;
			sec_bat_enable_charging(info, true);
		    info->is_rechg_triggered = false;
			g_chg_en = 1;

			dev_info(info->dev,
				 "%s: Start Recharging, Vcell = %d\n", __func__,
				 info->batt_vcell);
		}
		/* break; */
	case POWER_SUPPLY_STATUS_CHARGING:
		if (info->batt_health == POWER_SUPPLY_HEALTH_OVERHEAT
		    || info->batt_health == POWER_SUPPLY_HEALTH_COLD) {
			sec_bat_enable_charging(info, false);
			g_chg_en=0;
			sec_bat_ichg_clear();
			info->charging_status =
			    POWER_SUPPLY_STATUS_NOT_CHARGING;
			info->test_info.is_rechg_state = false;

			dev_info(info->dev, "%s: Not charging\n", __func__);
		}
		else if (info->batt_health == POWER_SUPPLY_HEALTH_DEAD) {
			sec_bat_enable_charging(info, false);
			info->charging_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			info->test_info.is_rechg_state = false;
			g_chg_en = 0;
			sec_bat_ichg_clear();
			dev_info(info->dev, "%s: Not charging (VF err!)\n", __func__);
		}
		break;
	case POWER_SUPPLY_STATUS_DISCHARGING:
		dev_dbg(info->dev, "%s: Discharging\n", __func__);
		break;
	case POWER_SUPPLY_STATUS_NOT_CHARGING:
		if (info->batt_health == POWER_SUPPLY_HEALTH_GOOD) {
			dev_info(info->dev, "%s: recover health state\n",
				 __func__);
			if (info->cable_type != CABLE_TYPE_NONE) {
				sec_bat_enable_charging(info, true);
				info->charging_status
				    = POWER_SUPPLY_STATUS_CHARGING;
				g_chg_en=1;
			} else
				info->charging_status
				    = POWER_SUPPLY_STATUS_DISCHARGING;
		}
		break;
	default:
		dev_info(info->dev, "%s: Undefined Battery Status\n", __func__);
		wake_unlock(&info->monitor_wake_lock);
		return;
	}

	/* check default charger state, and set again */
	if (sec_bat_is_charging(info) == POWER_SUPPLY_STATUS_CHARGING && 
		info->charging_enabled) {
		if (sec_bat_is_invalid_bmd(info)) {
			printk("%s : default charger state, set again\n", __func__);
			wake_lock(&info->cable_wake_lock);
			queue_delayed_work(info->monitor_wqueue, &info->cable_work, 0);
		}
	}

	if (info->batt_soc != info->batt_presoc)
		printk("[fg] s1:%d, s2:%d, v:%d, t:%d, t_adc:%d\n", 
			info->batt_soc, info->batt_presoc,
			info->batt_vcell, info->batt_temp, info->batt_temp_adc);
	
	power_supply_changed(&info->psy_bat);

	wake_unlock(&info->monitor_wake_lock);

	return;
}

static void sec_bat_polling_work(struct work_struct *work)
{
	//unsigned long flags;
	struct sec_bat_info *info;
	info = container_of(work, struct sec_bat_info, polling_work.work);
	//int ret = 0;

	wake_lock(&info->monitor_wake_lock);
	queue_work(info->monitor_wqueue, &info->monitor_work);

	if (info->initial_check_count) {
		schedule_delayed_work(&info->polling_work, HZ);
		info->initial_check_count--;
	} else
		schedule_delayed_work(&info->polling_work,
				      msecs_to_jiffies(info->polling_interval));
}

static void sec_bat_measure_work(struct work_struct *work)
{
	struct sec_bat_info *info;
	unsigned long flags;
	int set_chg_current;
	//int ret = 0;
	bool isFirstCheck = false;
	info = container_of(work, struct sec_bat_info, measure_work.work);

	wake_lock(&info->measure_wake_lock);
	if (sec_check_recharging(info)) {
		printk("%s : rechg triggered!\n", __func__);
		info->is_rechg_triggered = true;
		cancel_work_sync(&info->monitor_work);
		wake_lock(&info->monitor_wake_lock);
		queue_work(info->monitor_wqueue, &info->monitor_work);
	}
	sec_bat_check_temper_adc(info);
	if (sec_bat_check_detbat(info) == BAT_NOT_DETECTED && info->present == 1)
		sec_bat_check_detbat(info); /* AGAIN_FEATURE */

	/* check dcin */
	if((sec_bat_is_charging(info) == POWER_SUPPLY_STATUS_CHARGING) &&
		(info->charging_status == POWER_SUPPLY_STATUS_DISCHARGING)) {
		printk("%s : dcin ok, but not charging, set cable type again!\n",
			__func__);
#if defined (CELOX_BATTERY_CHARGING_CONTROL)			
		if(0 == is_charging_disabled)
#endif		
		{
			local_irq_save(flags);
			if (info->cable_type == CABLE_TYPE_NONE)
				info->cable_type = CABLE_TYPE_UNKNOWN;
			local_irq_restore(flags);
			wake_lock(&info->cable_wake_lock);
			queue_delayed_work(info->monitor_wqueue, &info->cable_work, HZ);
		}
	} else if ((sec_bat_is_charging(info) == POWER_SUPPLY_STATUS_DISCHARGING) &&
		(info->charging_status != POWER_SUPPLY_STATUS_DISCHARGING)) {
		printk("%s : dcin nok, but still charging, just disable charging!\n",
			__func__);
		local_irq_save(flags);
		if (info->cable_type == CABLE_TYPE_UNKNOWN)
			info->cable_type = CABLE_TYPE_NONE;
		local_irq_restore(flags);
		sec_bat_handle_unknown_disable(info);
	}

	/* TEST : adjust fast charging current */
	/*
	static int count = 0;
	static int chg_current = 500;
	*/
	/*
	if (info->charging_enabled) {
		count++;
		if(count%2 == 0) {
			sec_bat_adjust_charging_current(info, chg_current);
			chg_current+=100;
		}

		if (count > 100000)
			count = 0;
		if (chg_current > 1100)
			chg_current = 500;
	}
	*/

	/* Adjust Charging Current */
	if (!info->lpm_chg_mode &&
		(info->cable_type == CABLE_TYPE_AC) &&
		info->charging_enabled) {
		set_chg_current = sec_bat_get_charging_current(info);
		if (set_chg_current >= 0) {
			if (info->is_esus_state) {
				if (info->voice_call_state == 0 &&
					set_chg_current != 1000) {
					printk("%s : adjust curretn to 1A\n", __func__);
					sec_bat_adjust_charging_current(info, 1000);
				}
			} else {
				if (set_chg_current != 1000) {
					printk("%s : adjust curretn to 1A\n", __func__);
					sec_bat_adjust_charging_current(info, 1000);
				}
			}
		} else {
			printk("%s : invalid charging current from charger (%d)\n",
				__func__, set_chg_current);
		}
	}

#if 1

		if (info->charging_enabled && 
			(((0 < info->batt_temp_high_cnt ) && (info->batt_temp_high_cnt < TEMP_BLOCK_COUNT))  || ((0 < info->batt_temp_low_cnt ) && (info->batt_temp_low_cnt < TEMP_BLOCK_COUNT)))) {
			isFirstCheck = true;
		} else {
			isFirstCheck = false;
		}
#endif
	if (info->initial_check_count) {
		queue_delayed_work(info->monitor_wqueue, &info->measure_work,
		      HZ);
	} 
	else if (isFirstCheck) {
		queue_delayed_work(info->monitor_wqueue, &info->measure_work,
					  HZ);

	} else {
		queue_delayed_work(info->monitor_wqueue, &info->measure_work,
		      msecs_to_jiffies(info->measure_interval));
	}
	//schedule_delayed_work(&info->measure_work,
	//	      msecs_to_jiffies(info->measure_interval));
	wake_unlock(&info->measure_wake_lock);
}

#define SEC_BATTERY_ATTR(_name)			\
{						\
	.attr = { .name = #_name,		\
		  .mode = 0664},	\
	.show = sec_bat_show_property,		\
	.store = sec_bat_store,			\
}

static struct device_attribute sec_battery_attrs[] = {
	SEC_BATTERY_ATTR(batt_vol),
	SEC_BATTERY_ATTR(batt_vol_adc), 
	SEC_BATTERY_ATTR(batt_soc),
	SEC_BATTERY_ATTR(batt_vfocv),
	SEC_BATTERY_ATTR(batt_temp),
	SEC_BATTERY_ATTR(batt_temp_adc),
	SEC_BATTERY_ATTR(charging_source),
	SEC_BATTERY_ATTR(batt_lp_charging),
	SEC_BATTERY_ATTR(batt_type),
	SEC_BATTERY_ATTR(batt_full_check),
	SEC_BATTERY_ATTR(batt_temp_check),
	SEC_BATTERY_ATTR(batt_temp_adc_spec),
	SEC_BATTERY_ATTR(batt_test_value),
	SEC_BATTERY_ATTR(batt_current_adc),
	SEC_BATTERY_ATTR(batt_esus_test),
	SEC_BATTERY_ATTR(system_rev),
	SEC_BATTERY_ATTR(fg_psoc),
	SEC_BATTERY_ATTR(batt_lpm_state),

	SEC_BATTERY_ATTR(talk_wcdma),
	SEC_BATTERY_ATTR(talk_gsm),
	SEC_BATTERY_ATTR(camera),
	SEC_BATTERY_ATTR(browser),


#if defined (CELOX_BATTERY_CHARGING_CONTROL)
	SEC_BATTERY_ATTR(batt_charging_enable),
#endif
	
#if defined (CONFIG_TARGET_LOCALE_USA)
	SEC_BATTERY_ATTR(camera),
	SEC_BATTERY_ATTR(mp3),
	SEC_BATTERY_ATTR(video),
	SEC_BATTERY_ATTR(talk_gsm),
	SEC_BATTERY_ATTR(talk_wcdma),
	SEC_BATTERY_ATTR(data_call),
	SEC_BATTERY_ATTR(wifi),
	SEC_BATTERY_ATTR(gps),	
	SEC_BATTERY_ATTR(device_state),
#endif
};

enum {
	BATT_VOL = 0,
	BATT_VOL_ADC, 
	BATT_SOC,
	BATT_VFOCV,
	BATT_TEMP,
	BATT_TEMP_ADC,
	CHARGING_SOURCE,
	BATT_LP_CHARGING,
	BATT_TYPE,
	BATT_FULL_CHECK,
	BATT_TEMP_CHECK,
	BATT_TEMP_ADC_SPEC,
	BATT_TEST_VALUE,
	BATT_CURRENT_ADC,
	BATT_ESUS_TEST,
	BATT_LPM_STATE,
	BATT_WCDMA_CALL,
	BATT_GSM_CALL,
	BATT_CAMERA,
	BATT_BROWSER,
#if defined (CELOX_BATTERY_CHARGING_CONTROL)	
	BATT_CHARGING_ENABLE,
#endif	
#if defined (CONFIG_TARGET_LOCALE_USA)
	BATT_CAMERA,
	BATT_MP3,
	BATT_VIDEO,
	BATT_VOICE_CALL_2G,
	BATT_VOICE_CALL_3G,
	BATT_DATA_CALL,
	BATT_WIFI,
	BATT_GPS,
	BATT_DEV_STATE,
#endif
};


#if defined (CONFIG_TARGET_LOCALE_USA)
static void sec_bat_set_compesation(struct sec_bat_info *info, int mode, int offset, int compensate_value)
{

	//pr_info("[BAT]:%s\n", __func__);

	if (mode)
	{
		if (!(info->device_state & offset))
		{
			info->device_state |= offset;
//			batt_compensation += compensate_value;
		}
	}
	else
	{
		if (info->device_state & offset)
		{
			info->device_state &= ~offset;
//			batt_compensation -= compensate_value; later, may be used (if we need to compensate)
		}
	}

//	is_calling_or_playing = s3c_bat_info.device_state;   // later, may be used (to prevent ap sleep when calling)
	//pr_info("[BAT]:%s: device_state=0x%x, compensation=%d\n", __func__, s3c_bat_info.device_state, batt_compensation);
	
}
#endif

static ssize_t sec_bat_show_property(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct sec_bat_info *info = dev_get_drvdata(dev->parent);
	int i = 0, val = 0;
	const ptrdiff_t off = attr - sec_battery_attrs;

	switch (off) {
	case BATT_VOL:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_vcell);
		break;
	case BATT_VOL_ADC:  
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_vol_adc);
		break;
	case BATT_SOC:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_soc);
		break;
	case BATT_VFOCV:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_vf_adc);
		break;
	case BATT_TEMP:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_temp);
		break;
	case BATT_TEMP_ADC:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->batt_temp_adc);
		break;
	case CHARGING_SOURCE:
		val = info->cable_type;
		//val = 2; /* for lpm test */
		if (info->lpm_chg_mode &&
			info->cable_type != CABLE_TYPE_NONE &&
			info->charging_status == POWER_SUPPLY_STATUS_DISCHARGING){
			val = CABLE_TYPE_NONE;
		}
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		break;
	case BATT_LP_CHARGING:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", info->get_lpcharging_state());
		break;
	case BATT_TYPE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n", "HITACHI_HITACHI");
		break;
	case BATT_FULL_CHECK:
		/* new concept : in case of time-out charging stop,
		  Do not update FULL for UI,
		  Use same time-out value for first charing and re-charging
		*/
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 
			(info->is_timeout_chgstop == false && 
			 info->charging_status == POWER_SUPPLY_STATUS_FULL)
			 ? 1 : 0);
		break;
	case BATT_TEMP_CHECK:
		i += scnprintf(buf + i, PAGE_SIZE - i,
			"%d\n", info->batt_health);
		break;
	case BATT_TEMP_ADC_SPEC:
		i += scnprintf(buf + i, PAGE_SIZE - i,
			"(HIGH: %d - %d,   LOW: %d - %d)\n",
				info->temper_spec.high_block, info->temper_spec.high_recovery,
				info->temper_spec.low_block, info->temper_spec.low_recovery);
		break;
	case BATT_TEST_VALUE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			info->test_info.test_value);
		break;
	case BATT_CURRENT_ADC:
		check_chgcurrent(info);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			info->batt_current_adc);
		break;
	case BATT_ESUS_TEST:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			info->test_info.test_esuspend);
		break;
	case BATT_LPM_STATE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			info->batt_lpm_state);
		break;
#if defined (CONFIG_TARGET_LOCALE_USA)
	case BATT_DEV_STATE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "0x%08x\n", info->device_state);
			break;
#endif
#if defined (CELOX_BATTERY_CHARGING_CONTROL)
	case BATT_CHARGING_ENABLE:
		{
			int val = (info->charging_enabled) ? 1 : 0;
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", val);
		}
		break;
#endif		
	default:
		i = -EINVAL;
	}

	return i;
}

static ssize_t sec_bat_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	int ret = 0, x = 0;
	const ptrdiff_t off = attr - sec_battery_attrs;
	struct sec_bat_info *info = dev_get_drvdata(dev->parent);

	switch (off) {
	case BATT_ESUS_TEST: /* early_suspend test */
		if (sscanf(buf, "%d\n", &x) == 1) {
			if (x == 0) {
				info->test_info.test_esuspend = 0;
				wake_lock_timeout(&info->test_wake_lock, 5 * HZ);
				cancel_delayed_work(&info->measure_work);
				info->measure_interval = MEASURE_DSG_INTERVAL;
				wake_lock(&info->measure_wake_lock);
				queue_delayed_work(info->monitor_wqueue, &info->measure_work, 0);
				//schedule_delayed_work(&info->measure_work, 0);
			} else {
				info->test_info.test_esuspend = 1;
				wake_lock(&info->test_wake_lock);
				cancel_delayed_work(&info->measure_work);
				info->measure_interval = MEASURE_CHG_INTERVAL;
				wake_lock(&info->measure_wake_lock);
				queue_delayed_work(info->monitor_wqueue, &info->measure_work, 0);
				//schedule_delayed_work(&info->measure_work, 0);
			}
			ret = count;
		}
		break;
	case BATT_TEST_VALUE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if (x == 0)
				info->test_info.test_value = 0;
			else if (x == 1)
				info->test_info.test_value = 1; // for temp warning event
            else if (x == 2)
				//info->test_info.test_value = 2; // for full event
				info->test_info.test_value = 0; // disable full test interface.
			else if (x == 3)
				info->test_info.test_value = 3; // for abs time event
			else if (x == 999) {
				info->test_info.test_value = 999; // for pop-up disable
				if((info->batt_health == POWER_SUPPLY_HEALTH_OVERHEAT) ||
					(info->batt_health == POWER_SUPPLY_HEALTH_COLD)) {
					info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
					wake_lock(&info->monitor_wake_lock);
					queue_work(info->monitor_wqueue, &info->monitor_work);
				}
			} else
				info->test_info.test_value = 0;
			printk("%s : test case : %d\n", __func__,
						info->test_info.test_value);
			ret = count;
		}
		break;
	case BATT_LPM_STATE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			info->batt_lpm_state = x;
			ret = count;
		}
		break;

	case BATT_WCDMA_CALL:
	case BATT_GSM_CALL:
		if (sscanf(buf, "%d\n", &x) == 1) {
			info->voice_call_state = x;
			printk("%s : voice call = %d, %d\n", __func__,
				x, info->voice_call_state);
		}
		break;
	case BATT_CAMERA:
		if (sscanf(buf, "%d\n", &x) == 1) {
			ret = count;
		}
		break;
	case BATT_BROWSER:
		if (sscanf(buf, "%d\n", &x) == 1) {
			ret = count;
		}
		break;
#if defined (CELOX_BATTERY_CHARGING_CONTROL)		
	case BATT_CHARGING_ENABLE:
		{
			printk("%s : BATT_CHARGING_ENABLE buf=[%s]\n", __func__, buf);
			if (sscanf(buf, "%d\n", &x) == 1)
				
			{
				if (x == 0)
				{
					is_charging_disabled = 1;
					info->cable_type = CABLE_TYPE_NONE;
					wake_lock(&info->cable_wake_lock);
					queue_delayed_work(info->monitor_wqueue, &info->cable_work, 0);
				}
				else if(x == 1)
				{
					is_charging_disabled = 0;
					info->cable_type = CABLE_TYPE_UNKNOWN;
					wake_lock(&info->cable_wake_lock);
					queue_delayed_work(info->monitor_wqueue, &info->cable_work, 0);
				}
				else
				{
					printk("%s : ****ERR BATT_CHARGING_ENABLE : Invalid Input\n", __func__);					
				}
				
				ret = 1;
			}
			
		}
		break;
#endif		
#if defined (CONFIG_TARGET_LOCALE_USA)
		case BATT_CAMERA:
			if (sscanf(buf, "%d\n", &x) == 1)
			{
				sec_bat_set_compesation(info, x, OFFSET_CAMERA_ON, COMPENSATE_CAMERA);
				ret = count;
			}
			//pr_info("[BAT]:%s: camera = %d\n", __func__, x);
			break;
		case BATT_MP3:
			if (sscanf(buf, "%d\n", &x) == 1)
			{
				sec_bat_set_compesation(info, x, OFFSET_MP3_PLAY,	COMPENSATE_MP3);
				ret = count;
			}
			//pr_info("[BAT]:%s: mp3 = %d\n", __func__, x);
			break;
		case BATT_VIDEO:
			if (sscanf(buf, "%d\n", &x) == 1)
			{
				sec_bat_set_compesation(info, x, OFFSET_VIDEO_PLAY, COMPENSATE_VIDEO);
				ret = count;
			}
			//pr_info("[BAT]:%s: video = %d\n", __func__, x);
			break;
		case BATT_VOICE_CALL_2G:
			if (sscanf(buf, "%d\n", &x) == 1)
			{
				sec_bat_set_compesation(info, x, OFFSET_VOICE_CALL_2G, COMPENSATE_VOICE_CALL_2G);
				ret = count;
			}
			//pr_info("[BAT]:%s: voice call 2G = %d\n", __func__, x);
			break;
		case BATT_VOICE_CALL_3G:
			if (sscanf(buf, "%d\n", &x) == 1)
			{
				sec_bat_set_compesation(info, x, OFFSET_VOICE_CALL_3G, COMPENSATE_VOICE_CALL_3G);
				ret = count;
			}
			//pr_info("[BAT]:%s: voice call 3G = %d\n", __func__, x);
			break;
		case BATT_DATA_CALL:
			if (sscanf(buf, "%d\n", &x) == 1)
			{
				sec_bat_set_compesation(info, x, OFFSET_DATA_CALL, COMPENSATE_DATA_CALL);
				ret = count;
			}
			//pr_info("[BAT]:%s: data call = %d\n", __func__, x);
			break;
		case BATT_WIFI:
			if (sscanf(buf, "%d\n", &x) == 1)
			{
				sec_bat_set_compesation(info, x, OFFSET_WIFI, COMPENSATE_WIFI);
				ret = count;
			}
			//pr_info("[BAT]:%s: wifi = %d\n", __func__, x);
			break;
		case BATT_GPS:
			if (sscanf(buf, "%d\n", &x) == 1)
			{
				sec_bat_set_compesation(info, x, OFFSET_GPS, COMPENSATE_GPS);
				ret = count;
			}
			//pr_info("[BAT]:%s: gps = %d\n", __func__, x);
			break;		
#endif

	default:
		ret = -EINVAL;
	}

	return ret;
}

static int sec_bat_create_attrs(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(sec_battery_attrs); i++) {
		rc = device_create_file(dev, &sec_battery_attrs[i]);
		if (rc)
			goto sec_attrs_failed;
	}
	goto succeed;

sec_attrs_failed:
	while (i--)
		device_remove_file(dev, &sec_battery_attrs[i]);
succeed:
	return rc;
}

static int sec_bat_read_proc(char *buf, char **start,
			off_t offset, int count, int *eof, void *data)
{
	struct sec_bat_info *info = data;
	struct timespec cur_time;
	ktime_t ktime;
	int len = 0;

	ktime = alarm_get_elapsed_realtime();
	cur_time = ktime_to_timespec(ktime);

	len = sprintf(buf, "%lu, %u, %u, %u, %d, %u, %d, %d, %d, %u, %u, \
%u, %d, %u, %lu\n",
		cur_time.tv_sec,
		info->batt_soc,
		info->batt_vcell,
		info->batt_current_adc,
		info->charging_enabled,
		info->batt_full_status,
		info->test_info.full_count,
		info->test_info.rechg_count,
		info->test_info.is_rechg_state,
		info->recharging_status,
		info->batt_health,
		info->charging_status,
		info->present,
		info->cable_type,
		info->charging_passed_time);
    return len;
}

static void sec_bat_early_suspend(struct early_suspend *handle)
{
	struct sec_bat_info *info = container_of(handle, struct sec_bat_info,
										bat_early_suspend);
	
	printk("%s...\n", __func__);
#if defined(CONFIG_MACH_VASTO)
	info->is_esus_state = false; // always 900mA
#else
	info->is_esus_state = true;
#endif

    return;
}

static void sec_bat_late_resume(struct early_suspend *handle)
{
	struct sec_bat_info *info = container_of(handle, struct sec_bat_info,
										bat_early_suspend);
	
	printk("%s...\n", __func__);
	info->is_esus_state = false;

    return;
}

static __devinit int sec_bat_probe(struct platform_device *pdev)
{
	struct sec_bat_platform_data *pdata = dev_get_platdata(&pdev->dev);
	struct sec_bat_info *info;
	int ret = 0;

#if defined (CELOX_BATTERY_CHARGING_CONTROL)
	is_charging_disabled = 0; 
#endif	
	dev_info(&pdev->dev, "%s: SEC Battery Driver Loading\n", __func__);

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	platform_set_drvdata(pdev, info);

	info->dev = &pdev->dev;
	if (!pdata->charger_name) {
		dev_err(info->dev, "%s: no fuel gauge or charger name\n",
			__func__);
		goto err_kfree;
	}
	info->charger_name = pdata->charger_name;

	info->get_lpcharging_state = pdata->get_lpcharging_state;
	info->present = 1;

	info->psy_bat.name = "battery",
	info->psy_bat.type = POWER_SUPPLY_TYPE_BATTERY,
	info->psy_bat.properties = sec_battery_props,
	info->psy_bat.num_properties = ARRAY_SIZE(sec_battery_props),
	info->psy_bat.get_property = sec_bat_get_property,
	info->psy_bat.set_property = sec_bat_set_property,
	info->psy_usb.name = "usb",
	info->psy_usb.type = POWER_SUPPLY_TYPE_USB,
	info->psy_usb.supplied_to = supply_list,
	info->psy_usb.num_supplicants = ARRAY_SIZE(supply_list),
	info->psy_usb.properties = sec_power_props,
	info->psy_usb.num_properties = ARRAY_SIZE(sec_power_props),
	info->psy_usb.get_property = sec_usb_get_property,
	info->psy_ac.name = "ac",
	info->psy_ac.type = POWER_SUPPLY_TYPE_MAINS,
	info->psy_ac.supplied_to = supply_list,
	info->psy_ac.num_supplicants = ARRAY_SIZE(supply_list),
	info->psy_ac.properties = sec_power_props,
	info->psy_ac.num_properties = ARRAY_SIZE(sec_power_props),
	info->psy_ac.get_property = sec_ac_get_property;

	wake_lock_init(&info->vbus_wake_lock, WAKE_LOCK_SUSPEND,
		       "vbus_present");
	wake_lock_init(&info->monitor_wake_lock, WAKE_LOCK_SUSPEND,
		       "sec-battery-monitor");
	wake_lock_init(&info->cable_wake_lock, WAKE_LOCK_SUSPEND,
		       "sec-battery-cable");
	wake_lock_init(&info->test_wake_lock, WAKE_LOCK_SUSPEND,
				"bat_esus_test");
	wake_lock_init(&info->measure_wake_lock, WAKE_LOCK_SUSPEND,
				"sec-battery-measure");

	info->batt_health = POWER_SUPPLY_HEALTH_GOOD;
	info->charging_status = sec_bat_is_charging(info);
	if (info->charging_status < 0)
		info->charging_status = POWER_SUPPLY_STATUS_DISCHARGING;
	else if (info->charging_status == POWER_SUPPLY_STATUS_CHARGING){
		info->cable_type = CABLE_TYPE_UNKNOWN; /* default */
		}
	info->batt_soc = 100;
	info->recharging_status = false;
	info->is_timeout_chgstop = false;
	info->is_esus_state = false;
	info->is_rechg_triggered = false;
	info->dcin_intr_triggered = false;
	info->present = 1;
	info->initial_check_count = INIT_CHECK_COUNT;

	info->adc_channel_main = CHANNEL_ADC_BATT_THERM;
	info->temper_spec.high_block = HIGH_BLOCK_TEMP_ADC_SETTHERM;
	info->temper_spec.high_recovery = HIGH_RECOVER_TEMP_ADC_SETTHERM;
	info->temper_spec.low_block = LOW_BLOCK_TEMP_ADC_SETTHERM;
	info->temper_spec.low_recovery = LOW_RECOVER_TEMP_ADC_SETTHERM;

	info->charging_start_time = 0;

	if (info->get_lpcharging_state) {
		if (info->get_lpcharging_state()) {
			info->polling_interval = POLLING_INTERVAL / 4;
			info->lpm_chg_mode = true;
		} else {
			info->polling_interval = POLLING_INTERVAL;
			info->lpm_chg_mode = false;
		}
	}

	if (info->charging_status == POWER_SUPPLY_STATUS_CHARGING)
		info->measure_interval = MEASURE_CHG_INTERVAL;
	else
		info->measure_interval = MEASURE_DSG_INTERVAL;

	/* init power supplier framework */
	ret = power_supply_register(&pdev->dev, &info->psy_bat);
	if (ret) {
		dev_err(info->dev, "%s: failed to register psy_bat\n",
			__func__);
		goto err_wake_lock;
	}

	ret = power_supply_register(&pdev->dev, &info->psy_usb);
	if (ret) {
		dev_err(info->dev, "%s: failed to register psy_usb\n",
			__func__);
		goto err_supply_unreg_bat;
	}

	ret = power_supply_register(&pdev->dev, &info->psy_ac);
	if (ret) {
		dev_err(info->dev, "%s: failed to register psy_ac\n", __func__);
		goto err_supply_unreg_usb;
	}

	/* create sec detail attributes */
	sec_bat_create_attrs(info->psy_bat.dev);

	info->entry = create_proc_entry("batt_info_proc", S_IRUGO, NULL);
	if (!info->entry)
		dev_err(info->dev, "%s: failed to create proc_entry\n",
			__func__);
    else {
		info->entry->read_proc = sec_bat_read_proc;
		info->entry->data = (struct sec_bat_info *)info;
    }

	info->monitor_wqueue =
	    create_freezable_workqueue(dev_name(&pdev->dev));
	if (!info->monitor_wqueue) {
		dev_err(info->dev, "%s: fail to create workqueue\n", __func__);
		goto err_supply_unreg_ac;
	}

	info->bat_early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 1;
	info->bat_early_suspend.suspend = sec_bat_early_suspend;
	info->bat_early_suspend.resume = sec_bat_late_resume;
	register_early_suspend(&info->bat_early_suspend);
	
	/* for lpm test */
	//wake_lock(&info->test_wake_lock);

	INIT_WORK(&info->monitor_work, sec_bat_monitor_work);
	INIT_DELAYED_WORK_DEFERRABLE(&info->cable_work, sec_bat_cable_work);

	INIT_DELAYED_WORK_DEFERRABLE(&info->polling_work, sec_bat_polling_work);
	schedule_delayed_work(&info->polling_work, 0);

	INIT_DELAYED_WORK_DEFERRABLE(&info->measure_work, sec_bat_measure_work);
	queue_delayed_work(info->monitor_wqueue, &info->measure_work, 0);

	return 0;

err_supply_unreg_ac:
	power_supply_unregister(&info->psy_ac);
err_supply_unreg_usb:
	power_supply_unregister(&info->psy_usb);
err_supply_unreg_bat:
	power_supply_unregister(&info->psy_bat);
err_wake_lock:
	wake_lock_destroy(&info->vbus_wake_lock);
	wake_lock_destroy(&info->monitor_wake_lock);
	wake_lock_destroy(&info->cable_wake_lock);
	wake_lock_destroy(&info->test_wake_lock);
	wake_lock_destroy(&info->measure_wake_lock);
err_kfree:
	kfree(info);

	return ret;
}

static int __devexit sec_bat_remove(struct platform_device *pdev)
{
	struct sec_bat_info *info = platform_get_drvdata(pdev);

	remove_proc_entry("batt_info_proc", NULL);

	flush_workqueue(info->monitor_wqueue);
	destroy_workqueue(info->monitor_wqueue);

	cancel_delayed_work(&info->cable_work);
	cancel_delayed_work(&info->polling_work);
	cancel_delayed_work(&info->measure_work);
	

	power_supply_unregister(&info->psy_bat);
	power_supply_unregister(&info->psy_usb);
	power_supply_unregister(&info->psy_ac);

	wake_lock_destroy(&info->vbus_wake_lock);
	wake_lock_destroy(&info->monitor_wake_lock);
	wake_lock_destroy(&info->cable_wake_lock);
	wake_lock_destroy(&info->test_wake_lock);
	wake_lock_destroy(&info->measure_wake_lock);

	kfree(info);

	return 0;
}

static int sec_bat_suspend(struct device *dev)
{
	struct sec_bat_info *info = dev_get_drvdata(dev);

	cancel_work_sync(&info->monitor_work);
	cancel_delayed_work(&info->cable_work);
	cancel_delayed_work(&info->polling_work);
	cancel_delayed_work(&info->measure_work);
	
	return 0;
}

static int sec_bat_resume(struct device *dev)
{
	struct sec_bat_info *info = dev_get_drvdata(dev);

	queue_delayed_work(info->monitor_wqueue,
							&info->measure_work, 0);
	wake_lock(&info->monitor_wake_lock);
	queue_work(info->monitor_wqueue, &info->monitor_work);

	schedule_delayed_work(&info->polling_work,
			      3*HZ);

	return 0;
}

static const struct dev_pm_ops sec_bat_pm_ops = {
	.suspend = sec_bat_suspend,
	.resume = sec_bat_resume,
};

static struct platform_driver sec_bat_driver = {
	.driver = {
		   .name = "sec-battery",
		   .owner = THIS_MODULE,
		   .pm = &sec_bat_pm_ops,
		   },
	.probe = sec_bat_probe,
	.remove = __devexit_p(sec_bat_remove),
};

static int __init sec_bat_init(void)
{
	return platform_driver_register(&sec_bat_driver);
}

static void __exit sec_bat_exit(void)
{
	platform_driver_unregister(&sec_bat_driver);
}

late_initcall(sec_bat_init);
module_exit(sec_bat_exit);

MODULE_DESCRIPTION("SEC battery driver");
MODULE_AUTHOR("<jongmyeong.ko@samsung.com>");
MODULE_AUTHOR("<ms925.kim@samsung.com>");
MODULE_AUTHOR("<joshua.chang@samsung.com>");
MODULE_LICENSE("GPL");

