/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio_event.h>
#include <linux/i2c-gpio.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_hsusb.h>
#include <mach/rpc_hsusb.h>
#include <mach/rpc_pmapp.h>
#include <mach/usbdiag.h>
#include <mach/usb_gadget_fserial.h>
#include <mach/msm_memtypes.h>
#include <mach/msm_serial_hs.h>
#include <linux/usb/android_composite.h>
#include <linux/platform_device.h>
#include <linux/io.h>
//#include <mach/gpio-v1.h>
#ifdef CONFIG_MACH_JENA
#ifdef CONFIG_MACH_VASTO
#include <mach/gpio_vasto.h>
#include <mach/sec_switch.h>
#else
#include <mach/gpio_jena.h>
#endif
#endif
#include <mach/vreg.h>
#include <mach/pmic.h>
#include <mach/socinfo.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <asm/mach/mmc.h>
#include <linux/i2c.h>
#include <linux/i2c/sx150x.h>
//#include <linux/gpio.h>
#include <linux/android_pmem.h>
#include <linux/bootmem.h>
#include <linux/mfd/marimba.h>
#include <mach/vreg.h>
#include <linux/power_supply.h>
#include <mach/rpc_pmapp.h>

#ifdef CONFIG_BATTERY_MSM
#include <mach/msm_battery.h>
#endif
#include <linux/smsc911x.h>
#include <linux/atmel_maxtouch.h>
#include <linux/i2c/mxt224.h>
#include "devices.h"
#include "timer.h"
#include "devices-msm7x2xa.h"
#include "pm.h"
#include "proc_comm.h"
#ifdef CONFIG_SAMSUNG_JACK
#include <linux/sec_jack.h>
//#include "proc_comm.h"
#endif
#include <mach/rpc_server_handset.h>
#include <mach/socinfo.h>
#include <linux/i2c/europa_tsp_gpio.h>

#include <linux/fsaxxxx_usbsw.h>
#include <linux/battery_charger.h>

#ifdef CONFIG_BATTERY_SEC
#include <mach/sec_battery.h>
#endif

#ifdef CONFIG_KEYPAD_MELFAS_TOUCH
#include <linux/input/melfas-touchkey.h>
#endif

#ifdef CONFIG_KEYBOARD_NEXTCHIP_TOUCH
#include <linux/input/nextchip-touchkey.h>
#endif

static struct vasto_charging_status_callbacks {
	void	(*tsp_set_charging_cable) (int type);
} charging_cbs;

static bool is_cable_attached;

#ifndef CONFIG_SENSORS_GP2A
#ifdef CONFIG_PROXIMITY_SENSOR
#include <linux/gp2a.h>
#endif
#endif
#ifdef CONFIG_CHARGER_SMB328A
#include <linux/smb328a_charger.h>
#endif
#ifdef CONFIG_MACH_VASTO
#include <linux/skbuff.h>	// 2011.01.21 static wifi skb
#include  "mach/media.h"
#endif

#ifdef CONFIG_BATTERY_SEC
extern unsigned int is_lpm_boot;
#endif

extern int current_jack_type;

#define _CONFIG_MACH_JENA // Temporary flag
#define _CONFIG_MACH_TREBON // Temporary flag


#ifdef CONFIG_MACH_VASTO
#define CONFIG_SD_POWER_CONTROL_BY_GPIO
#endif

#define PMEM_KERNEL_EBI1_SIZE	0x3A000
#define MSM_PMEM_AUDIO_SIZE	0x5B000
#define BAHAMA_SLAVE_ID_FM_ADDR         0x2A
#define BAHAMA_SLAVE_ID_QMEMBIST_ADDR   0x7B
#define FM_GPIO	83

#ifdef CONFIG_MACH_VASTO
//BT
#define GPIO_BT_WAKE           110     // BT_WAKE
#define GPIO_BT_REG_ON         34     //BT_PWR, BT_REG_ON
#define GPIO_BT_RESET		 72     // BT_RESET
#define GPIO_BT_HOST_WAKE 124    //BT_HOST_WAKE
#define GPIO_WLAN_RST_BT     33    //WLAN_RESET
static int bt_power_on_enter = 0 ;//bt power on

//WIFI  Gpio_vasto.h
//#define GPIO_WLAN_RESET_N       33  //WLAN_RESET , WLAN_RST , WLAN_RESET_N
//#define GPIO_BT_PWR                  34  //WLAN_BT_EN , BT_PWR , WLAN_BT_REG_EN  
//#define GPIO_WLAN_HOST_WAKE  42  //WLAN_HOST_WAKE
//#define GPIO_BT_nRST		      72  //BT_NRST , BT_RESET
#else

#if 1  //defined(SDIO_ISR_THREAD)
#define WLAN_HOST_WAKE   
#endif 

#endif  //CONFIG_MACH_VASTO
#ifdef CONFIG_SAMSUNG_JACK

#ifdef  CONFIG_MACH_VASTO
#define GPIO_JACK_S_35	78 // GPIO78 EAR_DET byeongguk.kim_20110715
#else
#define GPIO_JACK_S_35	48
#endif

#if 0 //def  CONFIG_MACH_VASTO //kth_temp 0
#define GPIO_SEND_END	79
#else
#define GPIO_SEND_END	92
#endif

#ifdef CONFIG_MACH_VASTO  //DHD_USE_STATIC_BUF 
/*
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0 (8192 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1 (9900 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2 (8192 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD (4800 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG (14100 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_PMEM (8192 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_PMEM_GPU1 (4200 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_TEXSTREAM (4800 * SZ_1K)
*/
static struct s5p_media_device crespo_media_devs[] = {
/*
	[0] = {
		.id = S5P_MDEV_MFC,
		.name = "mfc",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0,
		.paddr = 0,
	},
	[1] = {
		.id = S5P_MDEV_MFC,
		.name = "mfc",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1,
		.paddr = 0,
	},
	[2] = {
		.id = S5P_MDEV_FIMC0,
		.name = "fimc0",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0,
		.paddr = 0,
	},
	[3] = {
		.id = S5P_MDEV_FIMC1,
		.name = "fimc1",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1,
		.paddr = 0,
	},
	[4] = {
		.id = S5P_MDEV_FIMC2,
		.name = "fimc2",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2,
		.paddr = 0,
	},
	[5] = {
		.id = S5P_MDEV_JPEG,
		.name = "jpeg",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG,
		.paddr = 0,
	},
	[6] = {
		.id = S5P_MDEV_FIMD,
		.name = "fimd",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD,
		.paddr = 0,
	},
	[7] = {
		.id = S5P_MDEV_PMEM,
		.name = "pmem",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_PMEM,
		.paddr = 0,
	},
	[8] = {
		.id = S5P_MDEV_PMEM_GPU1,
		.name = "pmem_gpu1",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_PMEM_GPU1,
		.paddr = 0,
	},
	[9] = {
		.id = S5P_MDEV_TEXSTREAM,
		.name = "texstream",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_TEXSTREAM,
		.paddr = 0,
	},
*/	
#ifdef CONFIG_WLAN_SAMSUNG_MEMSIZE_BCM
	[0] = {  //[10] = {
		.id = S3C_MDEV_WIFI,
		.name = "wifi",
		.bank = 0,
		.memsize = CONFIG_WLAN_SAMSUNG_MEMSIZE_BCM * SZ_1K,
		.paddr = 0,
	},
#endif	
};

//extern void s5p_reserve_bootmem(struct s5p_media_device *mdevs, int nr_mdevs);
#endif   // CONFIG_MACH_VASTO

static struct sec_jack_zone jack_zones[] = {
	[0] = {
		.adc_high	= 3,
		.delay_ms	= 10,
		.check_count	= 5,
		.jack_type	= SEC_HEADSET_3POLE,
	},
	[1] = {
		.adc_high	= 99,
		.delay_ms	= 10,
		.check_count	= 10,
		.jack_type	= SEC_HEADSET_3POLE,
	},
	[2] = {
		.adc_high	= 9999,
		.delay_ms	= 10,
		.check_count	= 5,
		.jack_type	= SEC_HEADSET_4POLE,
	},
};

int get_msm7x27a_det_jack_state(void)
{
	/* Active Low */
#ifdef  CONFIG_MACH_VASTO
	return(gpio_get_value(GPIO_JACK_S_35));
#else
	return(gpio_get_value(GPIO_JACK_S_35)) ^ 1;
#endif
}
EXPORT_SYMBOL(get_msm7x27a_det_jack_state);

static int get_msm7x27a_send_key_state(void)
{
	/* Active High */
       
#if 0  //kth_temp 0
	pr_info("get_msm7x27a_send_key_state\n");
	return(gpio_get_value(GPIO_SEND_END));
#else
	return 0;
#endif
}

#define SMEM_PROC_COMM_MICBIAS_ONOFF		PCOM_OEM_MICBIAS_ONOFF
#define SMEM_PROC_COMM_MICBIAS_ONOFF_REG5	PCOM_OEM_MICBIAS_ONOFF_REG5
#define SMEM_PROC_COMM_GET_ADC				PCOM_OEM_SAMSUNG_GET_ADC

enum {
	SMEM_PROC_COMM_GET_ADC_BATTERY = 0x0,
	SMEM_PROC_COMM_GET_ADC_TEMP,
	SMEM_PROC_COMM_GET_ADC_VF,
	SMEM_PROC_COMM_GET_ADC_ALL, // data1 : VF(MSB 2 bytes) vbatt_adc(LSB 2bytes), data2 : temp_adc
	SMEM_PROC_COMM_GET_ADC_EAR_ADC,		// 3PI_ADC
	SMEM_PROC_COMM_GET_ADC_MAX,
};

enum {
	SMEM_PROC_COMM_MICBIAS_CONTROL_OFF = 0x0,
	SMEM_PROC_COMM_MICBIAS_CONTROL_ON,
	SMEM_PROC_COMM_MICBIAS_CONTROL_MAX
};

static void set_msm7x27a_micbias_state_reg5(bool state)
{
	/* int res = 0;
	 * int data1 = 0;
	 * int data2 = 0;
	 * if (!state)
	 * {
		 * data1 = SMEM_PROC_COMM_MICBIAS_CONTROL_OFF;
		 * res = msm_proc_comm(SMEM_PROC_COMM_MICBIAS_ONOFF_REG5, &data1, &data2);
		 * if(res < 0)
		 * {
			 * pr_err("sec_jack: micbias_reg5 %s  fail \n",state?"on":"off");
		 * }
	 * } */
}

static bool cur_state = false;

static void set_msm7x27a_micbias_state(bool state)
{

	//if(cur_state == state)
	//{
	//	pr_info("sec_jack : earmic_bias same as cur_state\n");
	//	return;
	//}

	static int init = false;
	static struct wake_lock wake_lock;
	if( init == false ){
		wake_lock_init(&wake_lock, WAKE_LOCK_SUSPEND, "micbias_state");
		init = true;
	}
	wake_lock_timeout(&wake_lock, HZ*2);

	if(state)
	{
#ifdef  CONFIG_MACH_VASTO
		pmic_hsed_enable(PM_HSED_CONTROLLER_0, PM_HSED_ENABLE_ALWAYS); //kth_110929
#else
		pmic_hsed_enable(PM_HSED_CONTROLLER_0, PM_HSED_ENABLE_ALWAYS);
#endif
		msleep(130);
		cur_state = true;
	}
	else
	{
		pmic_hsed_enable(PM_HSED_CONTROLLER_0, PM_HSED_ENABLE_OFF);
		cur_state = false;
	}

	report_headset_status(state); //kth_temp 1

	pr_info("sec_jack : earmic_bias %s\n", state?"on":"off");

}

static int sec_jack_get_adc_value(void)
{
	 int res = 0;
	 unsigned int data1 = 0;
	 unsigned int data2 = 0;

	//res = msm_proc_comm(PCOM_OEM_EAR_TYPE_GET, &data1, &data2);
	//if(res < 0)
	//{
	//	pr_info("sec_jack: get_adc fail \n");
	//	return res;
	//}
	//pr_info("sec_jack_get_adc_value %d", data1);
	//return data1;
       pr_info("sec_jack_get_adc_value current_jack_type = %d", current_jack_type);
       return current_jack_type;
}

void sec_jack_gpio_init(void)
{
	gpio_tlmm_config(GPIO_CFG(GPIO_JACK_S_35, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_ENABLE); //gpio 48 JACK_INT_N
	if(gpio_request(GPIO_JACK_S_35, "h2w_detect")<0)
		pr_err("sec_jack:gpio_request fail\n");
	if(gpio_direction_input(GPIO_JACK_S_35)<0)
		pr_err("sec_jack:gpio_direction fail\n");
}

static struct sec_jack_platform_data sec_jack_data = {
	.get_det_jack_state	= get_msm7x27a_det_jack_state,
	.get_send_key_state	= get_msm7x27a_send_key_state,
	.set_micbias_state	= set_msm7x27a_micbias_state,
	.set_micbias_state_reg5	= set_msm7x27a_micbias_state_reg5,
	.get_adc_value	= sec_jack_get_adc_value,
	.zones		= jack_zones,
	.num_zones	= ARRAY_SIZE(jack_zones),
	.det_int	= MSM_GPIO_TO_INT(GPIO_JACK_S_35),
	.send_int	= MSM_GPIO_TO_INT(GPIO_SEND_END),
};

static struct platform_device sec_device_jack = {
	.name           = "sec_jack",
	.id             = -1,
	.dev            = {
		.platform_data  = &sec_jack_data,
	},
};
#endif

#if defined(CONFIG_TOUCHSCREEN_QT602240) || defined(CONFIG_TOUCHSCREEN_MXT768E)
static void mxt224_power_on(void)
{
#if 0
	s3c_gpio_cfgpin(GPIO_TSP_LDO_ON, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TSP_LDO_ON, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TSP_LDO_ON, GPIO_LEVEL_HIGH);
	mdelay(70);
	s3c_gpio_setpull(GPIO_TSP_INT, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_TSP_INT, S3C_GPIO_SFN(0xf));
	/*mdelay(40); */
	/* printk("mxt224_power_on is finished\n"); */
#else

    struct vreg *vreg_touch30;    
    struct vreg *vreg_touch18;
    int ret = 0;

    vreg_touch30 = vreg_get(NULL, "vreg_touch30");    
    vreg_touch18 = vreg_get(NULL, "vreg_touch18");

    ret = vreg_set_level(vreg_touch30, 3000);        
    ret = vreg_set_level(vreg_touch18, 1800);

	gpio_set_value(TSP_INT, 1);  
	gpio_direction_output(TSP_SCL, 1);  
	gpio_direction_output(TSP_SDA, 1);          
    
    ret = vreg_enable(vreg_touch30);    
    ret = vreg_enable(vreg_touch18);        

	mdelay(70);    

	//gpio_set_value(KEY_LED, 1);      

    printk("[TSP] mxt224_power_on.\n");              
        
#endif
}

static void mxt224_power_off(void)
{
#if 0
	s3c_gpio_cfgpin(GPIO_TSP_INT, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TSP_INT, S3C_GPIO_PULL_DOWN);

	s3c_gpio_cfgpin(GPIO_TSP_LDO_ON, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TSP_LDO_ON, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TSP_LDO_ON, GPIO_LEVEL_LOW);
	/* printk("mxt224_power_off is finished\n"); */
#else

    struct vreg *vreg_touch18;
    struct vreg *vreg_touch30;    
    int ret = 0;

    vreg_touch18 = vreg_get(NULL, "vreg_touch18");
    vreg_touch30 = vreg_get(NULL, "vreg_touch30");    
  
	gpio_set_value(TSP_INT, 0);  
	gpio_direction_output(TSP_SCL, 0);  
	gpio_direction_output(TSP_SDA, 0);  

    ret = vreg_disable(vreg_touch18);
    ret = vreg_disable(vreg_touch30);   

//	gpio_set_value(KEY_LED, 0);          

    printk("[TSP] mxt224_power_off.\n");              

#endif
}

static void mxt224_register_callback(void *function)
{
	charging_cbs.tsp_set_charging_cable = function;
}

static void mxt224_read_ta_status(bool *ta_status)
{
	*ta_status = is_cable_attached;
}

#if defined(CONFIG_MACH_C1Q1_REV02) || defined(CONFIG_MACH_P6_REV02)
#ifdef CONFIG_TOUCHSCREEN_MXT768E
static u8 t7_config[] = {GEN_POWERCONFIG_T7,
				64, 255, 20};
static u8 t8_config[] = {GEN_ACQUISITIONCONFIG_T8,
				64, 0, 20, 20, 0, 0, 20, 0, 50, 25};
static u8 t9_config[] = {TOUCH_MULTITOUCHSCREEN_T9,
				139, 0, 0, 16, 26, 0, 208, 50, 2, 6, 0, 5, 1,
				0, MXT224_MAX_MT_FINGERS, 10, 10, 5, 255, 3,
				255, 3, 0, 0, 0, 0, 136, 60, 136, 40, 40, 15, 0, 0};

static u8 t15_config[] = {TOUCH_KEYARRAY_T15,
				1, 16, 26, 1, 6, 0, 64, 255, 3, 0, 0};

static u8 t18_config[] = {SPT_COMCONFIG_T18,
				0, 0};

static u8 t40_config[] = {PROCI_GRIPSUPPRESSION_T40,
				0, 0, 0, 0, 0};

static u8 t42_config[] = {PROCI_TOUCHSUPPRESSION_T42,
				0, 0, 0, 0, 0, 0, 0, 0};

static u8 t43_config[] = {SPT_DIGITIZER_T43,
				0, 0, 0, 0};

static u8 t48_config[] = {PROCG_NOISESUPPRESSION_T48,
				1, 0, 65, 0, 12, 24, 36, 48, 8, 16, 11, 40, 0, 0,
				0, 0, 0};


static u8 t46_config[] = {SPT_CTECONFIG_T46,
				0, 0, 8, 32, 0, 0, 0, 0};
static u8 end_config[] = {RESERVED_T255};

static const u8 *mxt224_config[] = {
	t7_config,
	t8_config,
	t9_config,
	t15_config,
	t18_config,
	t40_config,
	t42_config,
	t43_config,
	t46_config,
	t48_config,
	end_config,
};
#else
static u8 t7_config[] = {GEN_POWERCONFIG_T7,
				64, 255, 20};
static u8 t8_config[] = {GEN_ACQUISITIONCONFIG_T8,
				36, 0, 20, 20, 0, 0, 10, 10, 50, 25};
static u8 t9_config[] = {TOUCH_MULTITOUCHSCREEN_T9,
				139, 0, 0, 18, 11, 0, 16, MXT224_THRESHOLD, 2, 1, 0, 3, 1,
				0, MXT224_MAX_MT_FINGERS, 10, 10, 10, 31, 3,
				223, 1, 0, 0, 0, 0, 0, 0, 0, 0, 10, 5, 5, 5};

static u8 t15_config[] = {TOUCH_KEYARRAY_T15,
				131, 16, 11, 2, 1, 0, 0, 40, 3, 0, 0};

static u8 t18_config[] = {SPT_COMCONFIG_T18,
				0, 0};

static u8 t48_config[] = {PROCG_NOISESUPPRESSION_T48,
				3, 0, 2, 10, 6, 12, 18, 24, 20, 30, 0, 0, 0, 0,
				0, 0, 0};


static u8 t46_config[] = {SPT_CTECONFIG_T46,
				0, 2, 0, 0, 0, 0, 0};
static u8 end_config[] = {RESERVED_T255};

static const u8 *mxt224_config[] = {
	t7_config,
	t8_config,
	t9_config,
	t15_config,
	t18_config,
	t46_config,
	t48_config,
	end_config,
};
#endif
#else
/*
	Configuration for MXT224
*/
static u8 t7_config[] = {GEN_POWERCONFIG_T7,
				48,		/* IDLEACQINT */
				255,	/* ACTVACQINT */
				25 		/* ACTV2IDLETO: 25 * 200ms = 5s */};
static u8 t8_config[] = {GEN_ACQUISITIONCONFIG_T8,
	10, 0, 5, 1, 0, 0, 9, 30};/*byte 3: 0*/
static u8 t9_config[] = {TOUCH_MULTITOUCHSCREEN_T9,
				131, 0, 0, 19, 11, 0, 32, MXT224_THRESHOLD, 2, 1, 0,
				15,		/* MOVHYSTI */
				1, 11, MXT224_MAX_MT_FINGERS, 5, 40, 10, 31, 3,
				223, 1, 0, 0, 0, 0, 143, 55, 143, 90, 18};
static u8 t15_config[] = {TOUCH_KEYARRAY_T15,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static u8 t18_config[] = {SPT_COMCONFIG_T18,
				0, 1};
static u8 t20_config[] = {PROCI_GRIPFACESUPPRESSION_T20,
				7, 0, 0, 0, 0, 0, 0, 30, 20, 4, 15, 10};
static u8 t22_config[] = {PROCG_NOISESUPPRESSION_T22,
				143, 0, 0, 0, 0, 0, 0, 3, 30, 0, 0,  29, 34, 39,
				49, 58, 3};
static u8 t23_config[] = {TOUCH_PROXIMITY_T23,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static u8 t28_config[] = {SPT_CTECONFIG_T28,
				0, 0, 3, 16, 19, 60};
static u8 end_config[] = {RESERVED_T255};

static const u8 *mxt224_config[] = {
	t7_config,
	t8_config,
	t9_config,
	t15_config,	
	t18_config,
	t20_config,
	t22_config,
	t23_config,	
	t28_config,
	end_config,
};

/*
	Configuration for MXT224-E
*/
static u8 t7_config_e[] = {GEN_POWERCONFIG_T7,
				48,		/* IDLEACQINT */
				255,	/* ACTVACQINT */
				25 		/* ACTV2IDLETO: 25 * 200ms = 5s */};

static u8 t8_config_e_ta[] = {GEN_ACQUISITIONCONFIG_T8,
				22, 0, 5, 1, 0, 0, 4, 35, 40, 55};
static u8 t8_config_e[] = {GEN_ACQUISITIONCONFIG_T8,
				27, 0, 5, 1, 0, 0, 4, 35, 40, 55};
#if 1 /* MXT224E_0V5_CONFIG */	
/* NEXTTCHDI added */
static u8 t9_config_e[] = {TOUCH_MULTITOUCHSCREEN_T9,
				139, 0, 0, 19, 11, 0, 32, MXT224E_THRESHOLD, 2, 1,
				10, 15, 1, 46, MXT224_MAX_MT_FINGERS, 5, 40, 10, 31, 3,
				223, 1, 10, 10, 10, 10, 143, 40, 143, 80,
				18, 15, 50, 50, 0};
#else
static u8 t9_config_e[] = {TOUCH_MULTITOUCHSCREEN_T9,
				139, 0, 0, 19, 11, 0, 16, MXT224_THRESHOLD, 2, 1, 10, 3, 1,
				0, MXT224_MAX_MT_FINGERS, 10, 40, 10, 31, 3,
				223, 1, 10, 10, 10, 10, 143, 40, 143, 80, 18, 15, 50, 50};
#endif

static u8 t15_config_e[] = {TOUCH_KEYARRAY_T15,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static u8 t18_config_e[] = {SPT_COMCONFIG_T18,
				0, 0};

static u8 t23_config_e[] = {TOUCH_PROXIMITY_T23,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static u8 t25_config_e[] = {SPT_SELFTEST_T25,
				0, 0, 0, 0, 0, 0, 0, 0};

static u8 t40_config_e[] = {PROCI_GRIPSUPPRESSION_T40,
				0, 0, 0, 0, 0};

static u8 t42_config_e[] = {PROCI_TOUCHSUPPRESSION_T42,
				0, 0, 0, 0, 0, 0, 0, 0};

static u8 t46_config_e_ta[] = {SPT_CTECONFIG_T46,
				0, 3, 16, 32, 0, 0, 1, 0, 0};

static u8 t46_config_e[] = {SPT_CTECONFIG_T46,
				0, 3, 16, 59, 0, 0, 1, 0, 0};

static u8 t47_config_e[] = {PROCI_STYLUS_T47,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#if 1 /*MXT224E_0V5_CONFIG */
static u8 t48_config_e_ta[] = {PROCG_NOISESUPPRESSION_T48,
				3, 132, 82, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 6, 6, 0, 0, 64, 4, 64,
				10, 0, 9, 5, 0, 10, 0, 20, 0, 0,
				0, 0, 0, 0, 0, 40, 2, 15, 1, 80,
				10, 5, 40, 246, 246, 10, 10, 150, 50, 143,
				80, 18, 15, 0};

static u8 t48_config_e[] = {PROCG_NOISESUPPRESSION_T48,
				3, 132, 64, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 6, 6, 0, 0, 48, 4, 48,
				10, 0, 10, 5, 0, 20, 0, 5, 0, 0,
				0, 0, 0, 0, 32, 50, 2, 15, 1, 46,
				10, 5, 40, 10, 10, 10, 10, 143, 40, 143,
				80, 18, 15, 0};

#else
/*static u8 t48_config_e[] = {PROCG_NOISESUPPRESSION_T48,
				1, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 60, 31, 6,
				50, 64, 100};*/
#endif

static u8 end_config_e[] = {RESERVED_T255};
static u8 t38_config_e[] = {SPT_USERDATA_T38, 0, 1, 12, 19, 38, 40, 0, 0};//110927 gumi noise

static const u8 *mxt224e_config[] = {
	t7_config_e,
	t8_config_e,
	t9_config_e,
	t15_config_e,
	t18_config_e,
	t23_config_e,
	t25_config_e,
	t40_config_e,
	t42_config_e,
	t46_config_e,
	t47_config_e,
	t48_config_e,
	t38_config_e,//110927 gumi noise
	end_config_e,
};

#endif


static struct mxt224_platform_data mxt224_data = {
	.max_finger_touches = MXT224_MAX_MT_FINGERS,
	.gpio_read_done = TSP_INT,
#if defined(CONFIG_MACH_C1Q1_REV02) || defined(CONFIG_MACH_P6_REV02)
	.config = mxt224_config,
#else
	.config = mxt224_config,
	.config_e = mxt224e_config,
	.t48_ta_cfg = t48_config_e_ta,	
#endif
	.min_x = 0,
#ifdef CONFIG_TOUCHSCREEN_MXT768E
	.max_x = 1023,
#else
	.max_x = 480,
#endif
	.min_y = 0,
#ifdef CONFIG_TOUCHSCREEN_MXT768E
	.max_y = 1023,
#else
	.max_y = 800,
#endif
	.min_z = 0,
	.max_z = 255,
	.min_w = 0,
	.max_w = 30,
	.power_on = mxt224_power_on,
	.power_off = mxt224_power_off,
	.register_cb = mxt224_register_callback,
	.read_ta_status = mxt224_read_ta_status,
};

//static struct qt602240_platform_data qt602240_platform_data = {
	//.x_line		  = 19,
	//.y_line		  = 11,
	//.x_size		  = 800,
	//.y_size		  = 480,
	//.blen			  = 0x11,
	//.threshold		  = 0x28,
	//.voltage		  = 2800000,		  /* 2.8V */
	//.orient		  = QT602240_DIAGONAL,
//};
#endif


enum {
	GPIO_EXPANDER_IRQ_BASE	= NR_MSM_IRQS + NR_GPIO_IRQS,
	GPIO_EXPANDER_GPIO_BASE	= NR_MSM_GPIOS,
	/* SURF expander */
	GPIO_CORE_EXPANDER_BASE	= GPIO_EXPANDER_GPIO_BASE,
	GPIO_BT_SYS_REST_EN	= GPIO_CORE_EXPANDER_BASE,
	GPIO_WLAN_EXT_POR_N,
	GPIO_DISPLAY_PWR_EN,
	GPIO_BACKLIGHT_EN,
	GPIO_PRESSURE_XCLR,
	GPIO_VREG_S3_EXP,
	GPIO_UBM2M_PWRDWN,
	GPIO_ETM_MODE_CS_N,
	GPIO_HOST_VBUS_EN,
	xGPIO_SPI_MOSI,
	xGPIO_SPI_MISO,
	xGPIO_SPI_CLK,
	xGPIO_SPI_CS0_N,
	GPIO_CORE_EXPANDER_IO13,
	GPIO_CORE_EXPANDER_IO14,
	GPIO_CORE_EXPANDER_IO15,
	/* Camera expander */
	GPIO_CAM_EXPANDER_BASE	= GPIO_CORE_EXPANDER_BASE + 16,
	GPIO_CAM_GP_STROBE_READY	= GPIO_CAM_EXPANDER_BASE,
	GPIO_CAM_GP_AFBUSY,
	GPIO_CAM_GP_CAM_PWDN,
	GPIO_CAM_GP_CAM1MP_XCLR,
	GPIO_CAM_GP_CAMIF_RESET_N,
	GPIO_CAM_GP_STROBE_CE,
	GPIO_CAM_GP_LED_EN1,
	GPIO_CAM_GP_LED_EN2,
};

#if defined(CONFIG_GPIO_SX150X)
enum {
	SX150X_CORE,
	SX150X_CAM,
};

static struct sx150x_platform_data sx150x_data[] __initdata = {
	[SX150X_CORE]	= {
		.gpio_base		= GPIO_CORE_EXPANDER_BASE,
		.oscio_is_gpo		= false,
		.io_pullup_ena		= 0,
		.io_pulldn_ena		= 0,
		.io_open_drain_ena	= 0,
		.irq_summary		= -1,
	},
	[SX150X_CAM]	= {
		.gpio_base		= GPIO_CAM_EXPANDER_BASE,
		.oscio_is_gpo		= false,
		.io_pullup_ena		= 0,
		.io_pulldn_ena		= 0,
		.io_open_drain_ena	= 0,
		.irq_summary		= -1,
	},
};
#endif

extern unsigned int board_hw_revision;
extern unsigned int kernel_uart_flag;

	/* FM Platform power and shutdown routines */
#define FPGA_MSM_CNTRL_REG2 0x90008010
static void config_pcm_i2s_mode(int mode)
{
	void __iomem *cfg_ptr;
	u8 reg2;

	cfg_ptr = ioremap_nocache(FPGA_MSM_CNTRL_REG2, sizeof(char));

	if (!cfg_ptr)
		return;
	if (mode) {
		/*enable the pcm mode in FPGA*/
		reg2 = readb_relaxed(cfg_ptr);
		if (reg2 == 0) {
			reg2 = 1;
			writeb_relaxed(reg2, cfg_ptr);
		}
	} else {
		/*enable i2s mode in FPGA*/
		reg2 = readb_relaxed(cfg_ptr);
		if (reg2 == 1) {
			reg2 = 0;
			writeb_relaxed(reg2, cfg_ptr);
		}
	}
	iounmap(cfg_ptr);
}

static unsigned fm_i2s_config_power_on[] = {
	/*FM_I2S_SD*/
	GPIO_CFG(68, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*FM_I2S_WS*/
	GPIO_CFG(70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*FM_I2S_SCK*/
	GPIO_CFG(71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static unsigned fm_i2s_config_power_off[] = {
	/*FM_I2S_SD*/
	GPIO_CFG(68, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*FM_I2S_WS*/
	GPIO_CFG(70, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*FM_I2S_SCK*/
	GPIO_CFG(71, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

static unsigned bt_config_power_on[] = {
    /*BT_WAKE_PIN 110 */
    GPIO_CFG(GPIO_BT_WAKE, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*WLAN_BT_EN 34 */
    GPIO_CFG(GPIO_BT_REG_ON, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*BT_RESET 72 */
    GPIO_CFG(GPIO_BT_RESET, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*GPIO_BT_HOST_WAKE 124 */
    GPIO_CFG(GPIO_BT_HOST_WAKE, 0, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),

    /*RFR*/
    GPIO_CFG(43, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*CTS*/
    GPIO_CFG(44, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*RX*/
    GPIO_CFG(45, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*TX*/
    GPIO_CFG(46, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    
    /*PCM_DOUT*/
    GPIO_CFG(68, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*PCM_DIN*/
    GPIO_CFG(69, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*PCM_SYNC*/
    GPIO_CFG(70, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
    /*PCM_CLK*/
    GPIO_CFG(71, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	
};
static unsigned bt_config_pcm_on[] = {
	/*PCM_DOUT*/
	GPIO_CFG(68, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*PCM_DIN*/
	GPIO_CFG(69, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*PCM_SYNC*/
	GPIO_CFG(70, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	/*PCM_CLK*/
	GPIO_CFG(71, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};
static unsigned bt_config_power_off[] = {
    /*BT_WAKE_PIN 110 */
    //GPIO_CFG(GPIO_BT_WAKE, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*WLAN_BT_EN 34 */
    //GPIO_CFG(GPIO_BT_REG_ON, 0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*BT_RESET 72 */
    //GPIO_CFG(GPIO_BT_RESET, 0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*BT_RESET 124 */
    //GPIO_CFG(GPIO_BT_HOST_WAKE, 0, GPIO_CFG_INPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),

    /*RFR*/
    GPIO_CFG(43, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*CTS*/
    GPIO_CFG(44, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*RX*/
    GPIO_CFG(45, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*TX*/
    GPIO_CFG(46, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),

    /*PCM_DOUT*/
    GPIO_CFG(68, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*PCM_DIN*/
    GPIO_CFG(69, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*PCM_SYNC*/
    GPIO_CFG(70, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
    /*PCM_CLK*/
    GPIO_CFG(71, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	
};
static unsigned bt_config_pcm_off[] = {
	/*PCM_DOUT*/
	GPIO_CFG(68, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*PCM_DIN*/
	GPIO_CFG(69, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*PCM_SYNC*/
	GPIO_CFG(70, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	/*PCM_CLK*/
	GPIO_CFG(71, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};


static int config_i2s(int mode)
{
	int pin, rc = 0;

	if (mode == FM_I2S_ON) {
		if (machine_is_msm7x27a_surf())
			config_pcm_i2s_mode(0);
		pr_err("%s mode = FM_I2S_ON", __func__);
		for (pin = 0; pin < ARRAY_SIZE(fm_i2s_config_power_on);
			pin++) {
				rc = gpio_tlmm_config(
					fm_i2s_config_power_on[pin],
					GPIO_CFG_ENABLE
					);
				if (rc < 0)
					return rc;
			}
	} else if (mode == FM_I2S_OFF) {
		pr_err("%s mode = FM_I2S_OFF", __func__);
		for (pin = 0; pin < ARRAY_SIZE(fm_i2s_config_power_off);
			pin++) {
				rc = gpio_tlmm_config(
					fm_i2s_config_power_off[pin],
					GPIO_CFG_ENABLE
					);
				if (rc < 0)
					return rc;
			}
	}
	return rc;
}
static int config_pcm(int mode)
{
	int pin, rc = 0;

	if (mode == BT_PCM_ON) {
		if (machine_is_msm7x27a_surf())
			config_pcm_i2s_mode(1);
		pr_err("%s mode =BT_PCM_ON", __func__);
		for (pin = 0; pin < ARRAY_SIZE(bt_config_pcm_on);
			pin++) {
				rc = gpio_tlmm_config(bt_config_pcm_on[pin],
					GPIO_CFG_ENABLE);
				if (rc < 0)
					return rc;
			}
	} else if (mode == BT_PCM_OFF) {
		pr_err("%s mode =BT_PCM_OFF", __func__);
		for (pin = 0; pin < ARRAY_SIZE(bt_config_pcm_off);
			pin++) {
				rc = gpio_tlmm_config(bt_config_pcm_off[pin],
					GPIO_CFG_ENABLE);
				if (rc < 0)
					return rc;
			}

	}

	return rc;
}

static int msm_bahama_setup_pcm_i2s(int mode)
{
	int fm_state = 0, bt_state = 0;
	int rc = 0;
	struct marimba config = { .mod_id =  SLAVE_ID_BAHAMA};

	fm_state = marimba_get_fm_status(&config);
	bt_state = marimba_get_bt_status(&config);

	switch (mode) {
	case BT_PCM_ON:
	case BT_PCM_OFF:
		if (!fm_state)
			rc = config_pcm(mode);
		break;
	case FM_I2S_ON:
		rc = config_i2s(mode);
		break;
	case FM_I2S_OFF:
		if (bt_state)
			rc = config_pcm(BT_PCM_ON);
		else
			rc = config_i2s(mode);
		break;
	default:
		rc = -EIO;
		pr_err("%s:Unsupported mode", __func__);
	}
	return rc;
}

static struct vreg *fm_regulator;
static int fm_radio_setup(struct marimba_fm_platform_data *pdata)
{
	int rc = 0;
	const char *id = "FMPW";
	uint32_t irqcfg;

	/* Voting for 1.8V Regulator */
	fm_regulator = vreg_get(NULL , "vreg_msme");
	if (IS_ERR(fm_regulator)) {
		pr_err("%s: vreg get failed with : (%ld)\n",
			__func__, PTR_ERR(fm_regulator));
		return -EINVAL;
	}

	/* Set the voltage level to 1.8V */
	rc = vreg_set_level(fm_regulator, 1800);
	if (rc < 0) {
		pr_err("%s: set regulator level failed with :(%d)\n",
			__func__, rc);
		goto fm_vreg_fail;
	}

	/* Enabling the 1.8V regulator */
	rc = vreg_enable(fm_regulator);
	if (rc) {
		pr_err("%s: enable regulator failed with :(%d)\n",
			__func__, rc);
		goto fm_vreg_fail;
	}

	/* Voting for 19.2MHz clock */
	rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
			PMAPP_CLOCK_VOTE_ON);
	if (rc < 0) {
		pr_err("%s: clock vote failed with :(%d)\n",
			 __func__, rc);
		goto fm_clock_vote_fail;
	}

	/* Configuring the FM GPIO */
	irqcfg = GPIO_CFG(FM_GPIO, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
			GPIO_CFG_2MA);

	rc = gpio_tlmm_config(irqcfg, GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
			 __func__, irqcfg, rc);
		goto fm_gpio_config_fail;
	}
//2011.07.06 qcomm
	msleep(100);
	return 0;

fm_gpio_config_fail:
	pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
		PMAPP_CLOCK_VOTE_OFF);
fm_clock_vote_fail:
	vreg_disable(fm_regulator);

fm_vreg_fail:
	vreg_put(fm_regulator);

	return rc;
};

static void fm_radio_shutdown(struct marimba_fm_platform_data *pdata)
{
	int rc;
	const char *id = "FMPW";

	/* Releasing the GPIO line used by FM */
	uint32_t irqcfg = GPIO_CFG(FM_GPIO, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
		GPIO_CFG_2MA);

	rc = gpio_tlmm_config(irqcfg, GPIO_CFG_ENABLE);
	if (rc)
		pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
			 __func__, irqcfg, rc);

	/* Releasing the 1.8V Regulator */
	if (fm_regulator != NULL) {
		rc = vreg_disable(fm_regulator);

		if (rc)
			pr_err("%s: disable regulator failed:(%d)\n",
				__func__, rc);
		fm_regulator = NULL;
	}

	/* Voting off the clock */
	rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
		PMAPP_CLOCK_VOTE_OFF);

	if (rc < 0)
		pr_err("%s: voting off failed with :(%d)\n",
			__func__, rc);
}


#ifndef ATH_POLLING
static void (*wlan_status_notify_cb)(int card_present, void *dev_id);
void *wlan_devid;

static int register_wlan_status_notify(void (*callback)(int card_present, void *dev_id), void *dev_id)
{
	printk("%s --enter\n", __func__);

	wlan_status_notify_cb = callback;
	wlan_devid = dev_id;
	return 0;
}

static unsigned int wlan_status(struct device *dev)
{
	int rc;

	printk("%s entered\n", __func__);

	rc = gpio_get_value(GPIO_WLAN_RESET_N/*gpio_wlan_reset_n*/);

	return rc;
}
#endif /* ATH_POLLING */


static struct marimba_fm_platform_data marimba_fm_pdata = {
	.fm_setup = fm_radio_setup,
	.fm_shutdown = fm_radio_shutdown,
	.irq = MSM_GPIO_TO_INT(FM_GPIO),
	.vreg_s2 = NULL,
	.vreg_xo_out = NULL,
	/* Configuring the FM SoC as I2S Master */
	.is_fm_soc_i2s_master = true,
	.config_i2s_gpio = msm_bahama_setup_pcm_i2s,
};

#if defined(CONFIG_BT) //&& defined(CONFIG_MARIMBA_CORE)

extern int bluesleep_start(void);
extern void bluesleep_stop(void);

static struct platform_device msm_bt_power_device = {
	.name = "bt_power",
};

static struct resource bluesleep_resources[] = {
{
    .name = "gpio_host_wake",
    .start = GPIO_BT_HOST_WAKE,
    .end = GPIO_BT_HOST_WAKE,
    .flags = IORESOURCE_IO,
    },
    {
    .name = "gpio_ext_wake",
    .start = GPIO_BT_WAKE,
    .end = GPIO_BT_WAKE,
    .flags = IORESOURCE_IO,
    },
    {
    .name = "host_wake",
    .start = MSM_GPIO_TO_INT(GPIO_BT_HOST_WAKE),
    .end = MSM_GPIO_TO_INT(GPIO_BT_HOST_WAKE),
    .flags = IORESOURCE_IRQ,
    },
};


static struct platform_device msm_bluesleep_device = {
    .name = "bluesleep",
    .id     = -1,
    .num_resources  = ARRAY_SIZE(bluesleep_resources),
    .resource   = bluesleep_resources,
};



	struct bahama_config_register {
		u8 reg;
		u8 value;
		u8 mask;
	};
static const char * const vregs_bahama_name[] = {
	"vreg_msme",
	"vbt",
};
static struct vreg *vregs_bahama[ARRAY_SIZE(vregs_bahama_name)];

static int bahama_bt(int on)
{

	int rc = 0;
	int i;

	struct marimba config = { .mod_id =  SLAVE_ID_BAHAMA};

	struct bahama_variant_register {
		const size_t size;
		const struct bahama_config_register *set;
	};

	const struct bahama_config_register *p;

	u8 version;

	const struct bahama_config_register v10_bt_on[] = {
		{ 0xE9, 0x00, 0xFF },
		{ 0xF4, 0x80, 0xFF },
		{ 0xE4, 0x00, 0xFF },
		{ 0xE5, 0x00, 0x0F },
#ifdef CONFIG_WLAN
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0xE9, 0x21, 0xFF },
		{ 0x01, 0x0C, 0x1F },
		{ 0x01, 0x08, 0x1F },
	};

	const struct bahama_config_register v20_bt_on_fm_off[] = {
		{ 0x11, 0x0C, 0xFF },
		{ 0x13, 0x01, 0xFF },
		{ 0xF4, 0x80, 0xFF },
		{ 0xF0, 0x00, 0xFF },
		{ 0xE9, 0x00, 0xFF },
#ifdef CONFIG_WLAN
		{ 0x81, 0x00, 0x7F },
		{ 0x82, 0x00, 0xFF },
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0x8E, 0x15, 0xFF },
		{ 0x8F, 0x15, 0xFF },
		{ 0x90, 0x15, 0xFF },

		{ 0xE9, 0x21, 0xFF },
	};

	const struct bahama_config_register v20_bt_on_fm_on[] = {
		{ 0x11, 0x0C, 0xFF },
		{ 0x13, 0x01, 0xFF },
		{ 0xF4, 0x86, 0xFF },
		{ 0xF0, 0x06, 0xFF },
		{ 0xE9, 0x00, 0xFF },
#ifdef CONFIG_WLAN
		{ 0x81, 0x00, 0x7F },
		{ 0x82, 0x00, 0xFF },
		{ 0xE6, 0x38, 0x7F },
		{ 0xE7, 0x06, 0xFF },
#endif
		{ 0xE9, 0x21, 0xFF },
	};

	const struct bahama_config_register v10_bt_off[] = {
		{ 0xE9, 0x00, 0xFF },
	};

	const struct bahama_config_register v20_bt_off_fm_off[] = {
		{ 0xF4, 0x84, 0xFF },
		{ 0xF0, 0x04, 0xFF },
		{ 0xE9, 0x00, 0xFF }
	};

	const struct bahama_config_register v20_bt_off_fm_on[] = {
		{ 0xF4, 0x86, 0xFF },
		{ 0xF0, 0x06, 0xFF },
		{ 0xE9, 0x00, 0xFF }
	};
	const struct bahama_variant_register bt_bahama[2][3] = {
	{
		{ ARRAY_SIZE(v10_bt_off), v10_bt_off },
		{ ARRAY_SIZE(v20_bt_off_fm_off), v20_bt_off_fm_off },
		{ ARRAY_SIZE(v20_bt_off_fm_on), v20_bt_off_fm_on }
	},
	{
		{ ARRAY_SIZE(v10_bt_on), v10_bt_on },
		{ ARRAY_SIZE(v20_bt_on_fm_off), v20_bt_on_fm_off },
		{ ARRAY_SIZE(v20_bt_on_fm_on), v20_bt_on_fm_on }
	}
	};

	u8 offset = 0; /* index into bahama configs */
	on = on ? 1 : 0;
	version = marimba_read_bahama_ver(&config);

	if (version == BAHAMA_VER_UNSUPPORTED) {
		dev_err(&msm_bt_power_device.dev,
			"%s: unsupported version\n",
			__func__);
		return -EIO;
	}

	if (version == BAHAMA_VER_2_0) {
		if (marimba_get_fm_status(&config))
			offset = 0x01;
	}

	p = bt_bahama[on][version + offset].set;

	dev_info(&msm_bt_power_device.dev,
		"%s: found version %d\n", __func__, version);

	for (i = 0; i < bt_bahama[on][version + offset].size; i++) {
		u8 value = (p+i)->value;
		rc = marimba_write_bit_mask(&config,
			(p+i)->reg,
			&value,
			sizeof((p+i)->value),
			(p+i)->mask);
		if (rc < 0) {
			dev_err(&msm_bt_power_device.dev,
				"%s: reg %x write failed: %d\n",
				__func__, (p+i)->reg, rc);
			return rc;
		}
		dev_info(&msm_bt_power_device.dev,
			"%s: reg 0x%02x write value 0x%02x mask 0x%02x\n",
				__func__, (p+i)->reg,
				value, (p+i)->mask);
		value = 0;
		rc = marimba_read_bit_mask(&config,
				(p+i)->reg, &value,
				sizeof((p+i)->value), (p+i)->mask);
		if (rc < 0)
			dev_err(&msm_bt_power_device.dev, "%s marimba_read_bit_mask- error",
					__func__);
		dev_info(&msm_bt_power_device.dev,
			"%s: reg 0x%02x read value 0x%02x mask 0x%02x\n",
				__func__, (p+i)->reg,
				value, (p+i)->mask);
	}
	/* Update BT Status */
	if (on)
		marimba_set_bt_status(&config, true);
	else
		marimba_set_bt_status(&config, false);
	return rc;
}
static int bluetooth_switch_regulators(int on)
{
	int i, rc = 0;

	for (i = 0; i < ARRAY_SIZE(vregs_bahama_name); i++) {
		if (!vregs_bahama[i]) {
			pr_err("%s: vreg_get %s failed(%d)\n",
			__func__, vregs_bahama_name[i], rc);
			goto vreg_fail;
		}
		rc = on ? vreg_set_level(vregs_bahama[i], i ? 2900 :
			1800) : 0;

		if (rc < 0) {
			pr_err("%s: vreg set level failed (%d)\n",
					__func__, rc);
			goto vreg_set_level_fail;
		}

		rc = on ? vreg_enable(vregs_bahama[i]) :
			  vreg_disable(vregs_bahama[i]);

		if (rc < 0) {
			pr_err("%s: vreg %s %s failed(%d)\n",
				__func__, vregs_bahama_name[i],
			       on ? "enable" : "disable", rc);
			goto vreg_fail;
			}
	}
	return rc;

vreg_fail:
	while (i) {
		if (on)
			vreg_disable(vregs_bahama[--i]);
		}
vreg_set_level_fail:
	vreg_put(vregs_bahama[0]);
	vreg_put(vregs_bahama[1]);
	return rc;
}

static unsigned int msm_bahama_setup_power(void)
{
	int rc = 0;
	struct vreg *vreg_s3 = NULL;

	vreg_s3 = vreg_get(NULL, "vreg_msme");
	if (IS_ERR(vreg_s3)) {
		pr_err("%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(vreg_s3));
		return PTR_ERR(vreg_s3);
	}
	rc = vreg_set_level(vreg_s3, 1800);
	if (rc < 0) {
		pr_err("%s: vreg set level failed (%d)\n",
				__func__, rc);
		goto vreg_fail;
	}
	rc = vreg_enable(vreg_s3);
	if (rc < 0) {
		pr_err("%s: vreg enable failed (%d)\n",
		       __func__, rc);
		goto vreg_fail;
	}

	/*setup Bahama_sys_reset_n*/
//	rc = gpio_request(GPIO_BT_PWR, "bahama sys_rst_n");
	if (rc < 0) {
		pr_err("%s: gpio_request %d = %d\n", __func__,
			GPIO_BT_PWR, rc);
		goto vreg_fail;
	}
//	rc = gpio_direction_output(GPIO_BT_PWR, 1);
	if (rc < 0) {
		pr_err("%s: gpio_direction_output %d = %d\n", __func__,
			GPIO_BT_PWR, rc);
		goto gpio_fail;
	}
//2011.07.06 qcomm - bt on failed	
	msleep(100);
	return rc;

gpio_fail:
//	gpio_free(GPIO_BT_PWR);
vreg_fail:
	vreg_put(vreg_s3);
	return rc;
}

static unsigned int msm_bahama_shutdown_power(int value)
{
	int rc = 0;
	struct vreg *vreg_s3 = NULL;

	vreg_s3 = vreg_get(NULL, "vreg_msme");
	if (IS_ERR(vreg_s3)) {
		pr_err("%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(vreg_s3));
		return PTR_ERR(vreg_s3);
	}
	rc = vreg_disable(vreg_s3);
	if (rc) {
		pr_err("%s: vreg disable failed (%d)\n",
		       __func__, rc);
		vreg_put(vreg_s3);
		return rc;
	}

	return rc;
}


static unsigned int msm_bahama_core_config(int type)
{
	int rc = 0;

	if (type == BAHAMA_ID) {
		int i;
		struct marimba config = { .mod_id = SLAVE_ID_BAHAMA };
		const struct bahama_config_register v20_init[] = {
			/* reg, value, mask */
			{ 0xF4, 0x84, 0xFF }, /* AREG */
			{ 0xF0, 0x04, 0xFF } /* DREG */
		};
		if (marimba_read_bahama_ver(&config) == BAHAMA_VER_2_0) {
			for (i = 0; i < ARRAY_SIZE(v20_init); i++) {
				u8 value = v20_init[i].value;
				rc = marimba_write_bit_mask(&config,
					v20_init[i].reg,
					&value,
					sizeof(v20_init[i].value),
					v20_init[i].mask);
				if (rc < 0) {
					pr_err("%s: reg %d write failed: %d\n",
						__func__, v20_init[i].reg, rc);
					return rc;
				}
				pr_debug("%s: reg 0x%02x value 0x%02x"
					" mask 0x%02x\n",
					__func__, v20_init[i].reg,
					v20_init[i].value, v20_init[i].mask);
			}
		}
	}
	pr_debug("core type: %d\n", type);
	return rc;
}

static int bluetooth_power(int on)
{
	int pin, rc;

	printk("%s %s --enter\n", __func__, on ? "on" : "down");

	if (on) 
	{	
	       bt_power_on_enter = 1; //bt power on
	       
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_on); pin++) {
			rc = gpio_tlmm_config(bt_config_power_on[pin], GPIO_CFG_ENABLE);
			if (rc < 0) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_on[pin], rc);
				return -EIO;
			}
		}
        pr_info("bluetooth_power BT_WAKE:%d, HOST_WAKE:%d, REG_ON:%d\n", gpio_get_value(GPIO_BT_WAKE), gpio_get_value(GPIO_BT_HOST_WAKE), gpio_get_value(GPIO_BT_REG_ON));

		gpio_set_value(GPIO_BT_WAKE, 1); /* BT_WAKE */
		gpio_set_value(GPIO_BT_REG_ON, 1); /* WLAN_BT_REG_EN */
		mdelay(150);
		gpio_set_value(GPIO_BT_RESET, 1); /* BT_RESET */
        mdelay(100);  //Delay between turning bluetooth power and starting bluesleep
        pr_info("bluetooth_power BT_WAKE:%d, HOST_WAKE:%d, REG_ON:%d\n", gpio_get_value(GPIO_BT_WAKE), gpio_get_value(GPIO_BT_HOST_WAKE), gpio_get_value(GPIO_BT_REG_ON));        

		bluesleep_start();
	}
	else 
	{
		bluesleep_stop();

		gpio_set_value(GPIO_BT_RESET, 0);  /* BT_RESET */
		if(gpio_get_value(GPIO_WLAN_RST_BT) == 0)			
		{					 
			gpio_set_value(GPIO_BT_REG_ON,0);  /* WLAN_BT_REG_EN */
			mdelay(50/*150*/);
		}
		gpio_set_value(GPIO_BT_WAKE, 0);  /* BT_WAKE */

		if(bt_power_on_enter == 1){ //bt power on
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off); pin++) {		
			rc = gpio_tlmm_config(bt_config_power_off[pin],
					      GPIO_CFG_ENABLE);
			if (rc < 0) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_off[pin], rc);
				return -EIO;
			}
		  }
	    }
	}
	return 0;
}


/*
static int bluetooth_power(int on)
{
	int pin, rc = 0;
	const char *id = "BTPW";
	int cid = 0;

	cid = adie_get_detected_connectivity_type();
	if (cid != BAHAMA_ID) {
		pr_err("%s: unexpected adie connectivity type: %d\n",
					__func__, cid);
		return -ENODEV;
	}
	if (on) {
		//setup power for BT SOC
		rc = bluetooth_switch_regulators(on);
		if (rc < 0) {
			pr_err("%s: bluetooth_switch_regulators rc = %d",
					__func__, rc);
			goto exit;
		}
		//setup BT GPIO lines
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_on);
			pin++) {
			rc = gpio_tlmm_config(bt_config_power_on[pin],
					GPIO_CFG_ENABLE);
			if (rc < 0) {
				pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
						__func__,
						bt_config_power_on[pin],
						rc);
				goto fail_power;
			}
		}
		//Setup BT clocks
		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
			PMAPP_CLOCK_VOTE_ON);
		if (rc < 0) {
			pr_err("Failed to vote for TCXO_D1 ON\n");
			goto fail_clock;
		}
		//2011.07.06 qcomm - bt on failed	
		msleep(100);

		//I2C config for Bahama
		rc = bahama_bt(1);
		if (rc < 0) {
			pr_err("%s: bahama_bt rc = %d", __func__, rc);
			goto fail_i2c;
		}
		msleep(20);

		//setup BT PCM lines
		rc = msm_bahama_setup_pcm_i2s(BT_PCM_ON);
		if (rc < 0) {
			pr_err("%s: msm_bahama_setup_pcm_i2s , rc =%d\n",
				__func__, rc);
				goto fail_power;
			}
		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
				  PMAPP_CLOCK_VOTE_PIN_CTRL);
		if (rc < 0)
			pr_err("%s:Pin Control Failed, rc = %d",
					__func__, rc);

	} else {
		rc = bahama_bt(0);
		if (rc < 0)
			pr_err("%s: bahama_bt rc = %d", __func__, rc);
fail_i2c:
		rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_D1,
				  PMAPP_CLOCK_VOTE_OFF);
		if (rc < 0)
			pr_err("%s: Failed to vote Off D1\n", __func__);
fail_clock:
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off);
			pin++) {
				rc = gpio_tlmm_config(bt_config_power_off[pin],
					GPIO_CFG_ENABLE);
				if (rc < 0) {
					pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
					__func__, bt_config_power_off[pin], rc);
				}
			}
		rc = msm_bahama_setup_pcm_i2s(BT_PCM_OFF);
		if (rc < 0) {
			pr_err("%s: msm_bahama_setup_pcm_i2s, rc =%d\n",
					__func__, rc);
				}
fail_power:
		rc = bluetooth_switch_regulators(0);
		if (rc < 0) {
			pr_err("%s: switch_regulators : rc = %d",\
					__func__, rc);
			goto exit;
		}
	}
	return rc;
exit:
	pr_err("%s: failed with rc = %d", __func__, rc);
	return rc;
}
*/
static int __init bt_power_init(void)
{
/*
	int i, rc = 0;
	for (i = 0; i < ARRAY_SIZE(vregs_bahama_name); i++) {
			vregs_bahama[i] = vreg_get(NULL,
						vregs_bahama_name[i]);
			if (IS_ERR(vregs_bahama[i])) {
				pr_err("%s: vreg get %s failed (%ld)\n",
				       __func__, vregs_bahama_name[i],
				       PTR_ERR(vregs_bahama[i]));
				rc = PTR_ERR(vregs_bahama[i]);
				goto vreg_get_fail;
			}
		}
*/
    int rc = 0;
	msm_bt_power_device.dev.platform_data = &bluetooth_power;

	return rc;
/*
vreg_get_fail:
	while (i)
		vreg_put(vregs_bahama[--i]);
	return rc;
*/
}

static struct marimba_platform_data marimba_pdata = {
	.slave_id[SLAVE_ID_BAHAMA_FM]        = BAHAMA_SLAVE_ID_FM_ADDR,
	.slave_id[SLAVE_ID_BAHAMA_QMEMBIST]  = BAHAMA_SLAVE_ID_QMEMBIST_ADDR,
	.bahama_setup                        = msm_bahama_setup_power,
	.bahama_shutdown                     = msm_bahama_shutdown_power,
	.bahama_core_config                  = msm_bahama_core_config,
	.fm				     = &marimba_fm_pdata,
};

#endif

#if defined(CONFIG_I2C) && defined(CONFIG_GPIO_SX150X)
static struct i2c_board_info core_exp_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("sx1509q", 0x3e),
		.platform_data =  &sx150x_data[SX150X_CORE],
	},
};
static struct i2c_board_info cam_exp_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("sx1508q", 0x22),
		.platform_data	= &sx150x_data[SX150X_CAM],
	},
};
#endif

#if defined(CONFIG_BT)// && defined(CONFIG_MARIMBA_CORE)
#ifndef CONFIG_SENSORS_GP2A
#ifdef CONFIG_PROXIMITY_SENSOR
static int gp2a_power(bool on)
{
/*
	struct regulator *regulator;

	ldo15_init_data.constraints.state_mem.enabled = on;
	ldo15_init_data.constraints.state_mem.disabled = !on;

	if (on) {
		regulator = regulator_get(NULL, "vled");
		if (IS_ERR(regulator))
			return 0;
		regulator_enable(regulator);
		regulator_put(regulator);
	} else {
		regulator = regulator_get(NULL, "vled");
		if (IS_ERR(regulator))
			return 0;
		if (regulator_is_enabled(regulator))
			regulator_force_disable(regulator);
		regulator_put(regulator);
	}
*/
	
	gpio_tlmm_config(GPIO_CFG( 29, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE); 
	
	return 0;
}

static struct gp2a_platform_data gp2a_pdata = {
	.p_out = 29,
	.power = gp2a_power,
};
#endif
#endif

static struct i2c_board_info bahama_devices[] = {
#if 0
{
	I2C_BOARD_INFO("marimba", 0xc),
	.platform_data = &marimba_pdata,
},
#endif
{
	I2C_BOARD_INFO("bma222", 0x08),
},
#ifdef CONFIG_SENSORS_MMC328X_MAG
{
    I2C_BOARD_INFO("mmc328x", 0x30),
},
#endif

#ifndef CONFIG_SENSORS_GP2A
#ifdef CONFIG_PROXIMITY_SENSOR
{
	I2C_BOARD_INFO("gp2a", 0x44 ),
	.platform_data = &gp2a_pdata,
},
#endif
#endif
};
#endif


#ifdef CONFIG_SENSORS_GP2A
static struct i2c_board_info proximity_i2c_device[] = {
	{
		I2C_BOARD_INFO("gp2a", 0x44 ),	
	},
};

static struct i2c_gpio_platform_data sensor_i2c_gpio_data = {
	.sda_pin    = 37,
	.scl_pin    = 38,
};

static struct platform_device sensor_i2c_gpio_device = {  
	.name       = "i2c-gpio",
	.id     =  5,
	.dev        = {
		.platform_data  = &sensor_i2c_gpio_data,
	}, 
};
#endif

static struct platform_device msm_vibrator_device = {
	.name	= "msm_vibrator",
	.id		= -1,
};

static struct i2c_gpio_platform_data touch_i2c_gpio_data = {
	.sda_pin    = GPIO_TSP_SDA,
	.scl_pin    = GPIO_TSP_SCL,
};

static struct platform_device touch_i2c_gpio_device = {  
	.name       = "i2c-gpio",
	.id     =  2,
	.dev        = {
		.platform_data  = &touch_i2c_gpio_data,
	},
};


static struct platform_device msm_wlan_pm_device = {   
        .name       = "wlan_ar6000_pm", 
       .id         = -1,
};


/* I2C 2 */
static struct i2c_board_info touch_i2c_devices[] = {
#if defined(CONFIG_TOUCHSCREEN_QT602240) || defined(CONFIG_TOUCHSCREEN_MXT768E) 
    {
#ifdef CONFIG_TOUCHSCREEN_MXT768E
        I2C_BOARD_INFO(MXT224_DEV_NAME, 0x4c),
#else
        I2C_BOARD_INFO(MXT224_DEV_NAME, 0x4a),
#endif
        .platform_data  = &mxt224_data,
        .irq = MSM_GPIO_TO_INT( TSP_INT ),               
    },
#elif defined CONFIG_TOUCHSCREEN_SYNAPTICS_I2C_RMI4
	{
		I2C_BOARD_INFO("synaptics-rmi-ts1", 0x20),
        .irq = MSM_GPIO_TO_INT( TSP_INT ),       
	},
#endif	
};

static void tkey_gpio_init(void)
{
#ifdef CONFIG_BATTERY_SEC
	if(is_lpm_boot)
		return;
#endif

    if (board_hw_revision>= 2) // MELFAS
        gpio_request(GPIO_KEY_INT, "TOUCHKEY_INT");
    else // NEXTCHIP
    {
        gpio_request(GPIO_KEY_INT, "TOUCHKEY_INT");
        gpio_request(GPIO_KEY_TEST, "TOUCHKEY_TEST");
        gpio_request(GPIO_KEY_RST, "TOUCHKEY_RESET");
        
        gpio_direction_input(KEY_INT);
        gpio_direction_output(KEY_TEST, 0);   
        gpio_direction_output(KEY_RST, 0);           
    }
/*        
#ifdef CONFIG_KEYBOARD_NEXTCHIP_TOUCH
    gpio_request(GPIO_KEY_INT, "TOUCHKEY_INT");
    gpio_request(GPIO_KEY_TEST, "TOUCHKEY_TEST");
    gpio_request(GPIO_KEY_RST, "TOUCHKEY_RESET");

	gpio_direction_input(KEY_INT);
	gpio_direction_output(KEY_TEST, 0);   
	gpio_direction_output(KEY_RST, 0);       
#elif CONFIG_KEYPAD_MELFAS_TOUCH
    gpio_request(GPIO_KEY_INT, "TOUCHKEY_INT");
#endif
*/
    printk("[Touchkey] tkey_gpio_init.\n");          
}

static struct i2c_gpio_platform_data tkey_i2c_gpio_data = {
	.sda_pin = GPIO_KEY_SDA,
	.scl_pin = GPIO_KEY_SCL,
	.udelay = 3,	/* closest to 400KHz */
};

static struct platform_device tkey_ic2_gpio_device = {
	.name = "i2c-gpio",
	.id = 6,
	.dev = {
		.platform_data = &tkey_i2c_gpio_data,
	},
};

static struct i2c_board_info  melfas_tkey_i2c_devices[] = {
    {
        I2C_BOARD_INFO("melfas_touchkey", 0x20),
        .irq = MSM_GPIO_TO_INT(KEY_INT),        
    },
};

static struct i2c_board_info  nextchip_tkey_i2c_devices[] = {
	{
		I2C_BOARD_INFO("nextchip_touchkey", 0xC0>>1),
		.irq = MSM_GPIO_TO_INT(KEY_INT),		
	},
};
/*
static struct i2c_board_info  tkey_i2c_devices[] = {
#ifdef CONFIG_KEYBOARD_NEXTCHIP_TOUCH    
	{
		I2C_BOARD_INFO("nextchip_touchkey", 0xC0>>1),
		.irq = MSM_GPIO_TO_INT(KEY_INT),		
	},
#elif CONFIG_KEYPAD_MELFAS_TOUCH	
    {
        I2C_BOARD_INFO("melfas_touchkey", 0x20),
        .irq = MSM_GPIO_TO_INT(KEY_INT),        
    },
#endif
};
*/
int fsa_cable_type = CABLE_TYPE_UNKNOWN;

int fsa880_get_charger_status(void);
int fsa880_get_charger_status(void)
{
	return fsa_cable_type;
}

#if !defined(CONFIG_MACH_VASTO)
static void jena_usb_power(int onoff, char *path)
{
#if 0
	struct vreg *usb_vreg = vreg_get("");

	if (IS_ERR(regulator))
		return;

	if (onoff) {
		if (!regulator_use_count(regulator))
			regulator_enable(regulator);
	} else {
		if (regulator_use_count(regulator))
			regulator_force_disable(regulator);
	}
#endif
}

void trebon_chg_connected(enum chg_type chgtype)
{
	char *chg_types[] = {"STD DOWNSTREAM PORT",
			"CARKIT",
			"DEDICATED CHARGER",
			"INVALID"};
	unsigned int data1 = 0;	
	unsigned int data2 = 0;	
	int ret = 0;

	switch (chgtype) {
	case USB_CHG_TYPE__SDP:
		ret = msm_proc_comm(PCOM_CHG_USB_IS_PC_CONNECTED,
				data1, data2);
		break;
	case USB_CHG_TYPE__WALLCHARGER:
		ret = msm_proc_comm(PCOM_CHG_USB_IS_CHARGER_CONNECTED,
				data1, data2);
		break;
	case USB_CHG_TYPE__INVALID:
		ret = msm_proc_comm(PCOM_CHG_USB_IS_DISCONNECTED,
				data1, data2);
		break;
	default:
		break;
	}

	if (ret < 0)
		pr_err("%s: connection err, ret=%d\n", __func__, ret);

	pr_info("\nCharger Type: %s\n", chg_types[chgtype]);
}

static void jena_usb_cb(u8 attached, struct fsausb_ops *ops)
{
	pr_info("[%s] Board file [FSA880]: USB Callback \n", __func__);

	/* USB */
	if(attached == FSA_ATTACHED) {
		ops->attach_handler(CABLE_TYPE_USB);
		fsa_cable_type = CABLE_TYPE_USB;
#if 0
		hsusb_chg_connected(USB_CHG_TYPE__SDP);
#else
		trebon_chg_connected(USB_CHG_TYPE__SDP);
#endif
	} else {
		ops->detach_handler();
		fsa_cable_type = CABLE_TYPE_UNKNOWN;
#if 0
		hsusb_chg_connected(USB_CHG_TYPE__INVALID);
#else
		trebon_chg_connected(USB_CHG_TYPE__INVALID);
#endif
	}

	/* msm7227a is onechip with CP and AP
	 * don't need to change power by path
	 */
	jena_usb_power(attached, NULL);
}

static void jena_charger_cb(u8 attached, struct fsausb_ops *ops)
{
	printk("\nBoard file [FSA880]: Charger Callback \n");

	/* TA */
	if(attached == FSA_ATTACHED) {
		ops->attach_handler(CABLE_TYPE_TA);
		fsa_cable_type = CABLE_TYPE_TA;
#if 0
		hsusb_chg_connected(USB_CHG_TYPE__WALLCHARGER);
#else
		trebon_chg_connected(USB_CHG_TYPE__WALLCHARGER);
#endif
	} else {
		ops->detach_handler();
		fsa_cable_type = CABLE_TYPE_UNKNOWN;
#if 0
		hsusb_chg_connected(USB_CHG_TYPE__INVALID);
#else
		trebon_chg_connected(USB_CHG_TYPE__INVALID);
#endif
	}
}

static void jena_jig_cb(u8 attached, struct fsausb_ops *ops)
{
	printk("\nBoard file [FSA880]: Jig Callback \n");
}

static void jena_fsa880_reset_cb(void)
{
	printk("\nBoard file [FSA880]: Reset Callback \n");
}
#endif
#if defined(CONFIG_BATTERY_SEC)
unsigned int is_lpcharging_state(void)
{
	u32 val = is_lpm_boot;
	
	pr_info("%s: is_lpm_boot:%d\n", __func__, val);

	return val;
}
EXPORT_SYMBOL(is_lpcharging_state);

static unsigned int sec_bat_get_lpcharging_state(void)
{
	u32 val = is_lpm_boot;
  
	pr_info("%s: LP charging:%d\n", __func__, val);

	return val;
}

static struct sec_bat_platform_data sec_bat_pdata = {
	.charger_name		= "smb328a-charger",
	.get_lpcharging_state	= sec_bat_get_lpcharging_state,
};

static struct platform_device sec_device_battery = {
	.name = "sec-battery",
	.id = -1,
	.dev.platform_data = &sec_bat_pdata,
};
#endif /* CONFIG_BATTERY_SEC */

#if defined(CONFIG_MACH_VASTO)
extern struct class *sec_class;
struct device *switch_dev;
static int device_attached;

int is_sec_switch_jig_attached(void)
{
	return (device_attached == DEV_TYPE_JIG);
}

static int sec_switch_get_attached_device(void)
{
	return device_attached;
}

static struct sec_switch_platform_data sec_switch_pdata = {
	.get_attached_device = sec_switch_get_attached_device,
};

struct platform_device sec_device_switch = {
	.name	= "sec_switch",
	.id	= 1,
	.dev	= {
		.platform_data	= &sec_switch_pdata,
	}
};

static void msm7x27a_switch_init(void)
{
	sec_class = class_create(THIS_MODULE, "sec");
	pr_err("msm7x27a_switch_init!\n");

	if (IS_ERR(sec_class))
		pr_err("Failed to create class(sec)!\n");

	switch_dev = device_create(sec_class, NULL, 0, NULL, "switch");

	if (IS_ERR(switch_dev))
		pr_err("Failed to create device(switch)!\n");

	pr_err("msm7x27a_switch_init end!\n");
};

static void vasto_usb_cb(u8 attached, struct fsausb_ops *ops)
{
#ifdef CONFIG_BATTERY_SEC	
	union power_supply_propval value;
	int ret = 0;

	struct power_supply *psy = power_supply_get_by_name("battery");
#endif

	printk(KERN_ERR "fsa880_usb_cb attached %d\n", attached);

	device_attached = attached ? DEV_TYPE_USB : DEV_TYPE_NONE;

	/* USB */
	if(attached == FSA_ATTACHED) {
		ops->attach_handler(CABLE_TYPE_USB);
		fsa_cable_type = CABLE_TYPE_USB;
	} else {
		ops->detach_handler();
		fsa_cable_type = CABLE_TYPE_UNKNOWN;
	}

#ifdef CONFIG_JACK_MON
	jack_event_handler("usb", attached);
#endif

#ifdef CONFIG_BATTERY_SEC
	switch(fsa_cable_type) {
		case CABLE_TYPE_USB:
			value.intval = POWER_SUPPLY_TYPE_USB;
			is_cable_attached = true;			
			break;
		case CABLE_TYPE_UNKNOWN:
			value.intval = POWER_SUPPLY_TYPE_BATTERY;
			is_cable_attached = false;			
			break;
		default:
			pr_err("%s: invalid cable status:%d\n", __func__, fsa_cable_type);
			return;
	}

   if (charging_cbs.tsp_set_charging_cable)
       charging_cbs.tsp_set_charging_cable(value.intval);    

   if(psy)	
    	ret = psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	if (ret) {
		pr_err("%s: fail to set power_suppy ONLINE property(%d)\n",
			__func__, ret);
	}
#endif

	/* msm7227a is onechip with CP and AP
	 * don't need to change power by path
	 */
//	jena_usb_power(attached, NULL);
}

static void vasto_charger_cb(u8 attached, struct fsausb_ops *ops)
{
#ifdef CONFIG_BATTERY_SEC	
	union power_supply_propval value;
	int ret = 0;

	struct power_supply *psy = power_supply_get_by_name("battery");
#endif
	printk(KERN_ERR "fsa880_charger_cb attached %d\n", attached);

	device_attached = attached ? DEV_TYPE_CHARGER : DEV_TYPE_NONE;

	/* TA */
	if(attached == FSA_ATTACHED) {
		ops->attach_handler(CABLE_TYPE_TA);
		fsa_cable_type = CABLE_TYPE_TA;
	} else {
		ops->detach_handler();
		fsa_cable_type = CABLE_TYPE_UNKNOWN;
	}

#ifdef CONFIG_JACK_MON
	jack_event_handler("charger", attached);
#endif

#ifdef CONFIG_BATTERY_SEC
	switch(fsa_cable_type) {
		case CABLE_TYPE_TA:
			value.intval = POWER_SUPPLY_TYPE_MAINS;
			is_cable_attached = true;			
			break;
		case CABLE_TYPE_UNKNOWN:
			value.intval = POWER_SUPPLY_TYPE_BATTERY;
			is_cable_attached = false;			
			break;
		default:
			pr_err("%s: invalid cable status:%d\n", __func__, fsa_cable_type);
			return;
	}

   if (charging_cbs.tsp_set_charging_cable)
       charging_cbs.tsp_set_charging_cable(value.intval);    

   if(psy)
    	ret = psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	if (ret) {
		pr_err("%s: fail to set power_suppy ONLINE property(%d)\n",
			__func__, ret);
	}
#endif

}

static void vasto_jig_cb(u8 attached, struct fsausb_ops *ops)
{
	printk("\nBoard file [FSA880]: Jig Callback \n");
#ifdef CONFIG_JACK_MON
	jack_event_handler("jig", attached);
#endif
	device_attached = attached ? DEV_TYPE_JIG : DEV_TYPE_NONE;
}

static void vasto_fsa880_reset_cb(void)
{
	printk("\nBoard file [FSA880]: Reset Callback \n");
#ifdef CONFIG_JACK_MON
	jack_event_handler("", 0);
#endif
}
/* For uUSB Switch */
static struct fsausb_platform_data vasto_fsa880_pdata = {
       .intb_gpio      = MSM_GPIO_TO_INT(GPIO_MUSB_INT),
       .usb_cb         = vasto_usb_cb,
       .uart_cb        = NULL,
       .charger_cb     = vasto_charger_cb,
       .jig_cb         = vasto_jig_cb,
       .reset_cb       = vasto_fsa880_reset_cb,
};

/* I2C 3 */
static struct i2c_gpio_platform_data fsa880_i2c_gpio_data = {
	.sda_pin    = GPIO_MUS_SDA,
	.scl_pin    = GPIO_MUS_SCL,
};

static struct platform_device fsa880_i2c_gpio_device = {  
	.name       = "i2c-gpio",
	.id     =  3,
	.dev        = {
		.platform_data  = &fsa880_i2c_gpio_data,
	},
};

static struct i2c_board_info fsa880_i2c_devices[] = {
	{
		I2C_BOARD_INFO("FSA880", 0x4A >> 1),
		.platform_data =  &vasto_fsa880_pdata,
		.irq = MSM_GPIO_TO_INT(GPIO_MUSB_INT),
	},
};

static struct i2c_board_info fsa9280_i2c_devices[] = {
	{
		I2C_BOARD_INFO("FSA9280", 0x4A >> 1),
		.platform_data =  &vasto_fsa880_pdata,
		.irq = MSM_GPIO_TO_INT(GPIO_MUSB_INT),
	},
};

#else
/* For uUSB Switch */
static struct fsausb_platform_data jena_fsa880_pdata = {
       .intb_gpio      = MSM_GPIO_TO_INT(GPIO_MUSB_INT),
       .usb_cb         = jena_usb_cb,
       .uart_cb        = NULL,
       .charger_cb     = jena_charger_cb,
       .jig_cb         = jena_jig_cb,
       .reset_cb       = jena_fsa880_reset_cb,
};

/* I2C 3 */
static struct i2c_gpio_platform_data fsa880_i2c_gpio_data = {
	.sda_pin    = GPIO_MUS_SDA,
	.scl_pin    = GPIO_MUS_SCL,
};

static struct platform_device fsa880_i2c_gpio_device = {  
	.name       = "i2c-gpio",
	.id     =  3,
	.dev        = {
		.platform_data  = &fsa880_i2c_gpio_data,
	},
};

static struct i2c_board_info fsa880_i2c_devices[] = {
	{
		I2C_BOARD_INFO("FSA880", 0x4A >> 1),
		.platform_data =  &jena_fsa880_pdata,
		.irq = MSM_GPIO_TO_INT(GPIO_MUSB_INT),
	},
};
#endif

#if defined(CONFIG_CHARGER_SMB328A)
/* For Switching Charger */
void smb328a_hw_init(void)
{

	printk("smb328a_hw_init..\n");

	gpio_tlmm_config(GPIO_CFG(GPIO_SMB328A_SCL, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(GPIO_SMB328A_SDA, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	
	gpio_tlmm_config(GPIO_CFG(GPIO_SMB328A_INT, 0, GPIO_CFG_INPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
}

static struct smb328a_platform_data smb328a_pdata = {
	.hw_init = smb328a_hw_init,
};

/* I2C 3 */
static struct i2c_gpio_platform_data smb328a_i2c_gpio_data = {
	.sda_pin    = GPIO_SMB328A_SDA,
	.scl_pin    = GPIO_SMB328A_SCL,
};

static struct platform_device smb328a_i2c_gpio_device = {  
	.name       = "i2c-gpio",
	.id     =  4,
	.dev        = {
		.platform_data  = &smb328a_i2c_gpio_data,
	},
};

static struct i2c_board_info smb328a_i2c_devices[] = {
   {
		I2C_BOARD_INFO("smb328a", (0x69 >> 1)),
		.platform_data = &smb328a_pdata,
		.irq	= MSM_GPIO_TO_INT(GPIO_SMB328A_INT),
	},
};
#endif
#if defined(CONFIG_I2C)
static void __init register_i2c_devices(void)
{
#if 0//defined(CONFIG_GPIO_SX150X)
	i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
				cam_exp_i2c_info,
				ARRAY_SIZE(cam_exp_i2c_info));

	if (machine_is_msm7x27a_surf())
		sx150x_data[SX150X_CORE].io_open_drain_ena = 0xe0f0;

	core_exp_i2c_info[0].platform_data =
			&sx150x_data[SX150X_CORE];

	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				core_exp_i2c_info,
				ARRAY_SIZE(core_exp_i2c_info));
	#if defined(CONFIG_BT) && defined(CONFIG_MARIMBA_CORE)
	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				bahama_devices,
				ARRAY_SIZE(bahama_devices));
	#endif
#else				
	#if defined(CONFIG_BT) && defined(CONFIG_MARIMBA_CORE)
	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				bahama_devices,
				ARRAY_SIZE(bahama_devices));
	#endif
#endif
}
#endif

static struct msm_gpio qup_i2c_gpios_io[] = {
	{ GPIO_CFG(60, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_scl" },
	{ GPIO_CFG(61, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_sda" },
	{ GPIO_CFG(131, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_scl" },
	{ GPIO_CFG(132, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_sda" },
};

static struct msm_gpio qup_i2c_gpios_hw[] = {
	{ GPIO_CFG(60, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_scl" },
	{ GPIO_CFG(61, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_sda" },
	{ GPIO_CFG(131, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_scl" },
	{ GPIO_CFG(132, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_sda" },
};

static void gsbi_qup_i2c_gpio_config(int adap_id, int config_type)
{
	int rc;

	if (adap_id < 0 || adap_id > 1)
		return;

	/* Each adapter gets 2 lines from the table */
	if (config_type)
		rc = msm_gpios_request_enable(&qup_i2c_gpios_hw[adap_id*2], 2);
	else
		rc = msm_gpios_request_enable(&qup_i2c_gpios_io[adap_id*2], 2);
	if (rc < 0)
		pr_err("QUP GPIO request/enable failed: %d\n", rc);
}

static struct msm_i2c_platform_data msm_gsbi0_qup_i2c_pdata = {
#ifdef CONFIG_MACH_VASTO
	.clk_freq		= 380000,
#else
	.clk_freq		= 100000,
#endif
	.clk			= "gsbi_qup_clk",
	.pclk			= "gsbi_qup_pclk",
	.msm_i2c_config_gpio	= gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm_gsbi1_qup_i2c_pdata = {
	.clk_freq		= 100000,
	.clk			= "gsbi_qup_clk",
	.pclk			= "gsbi_qup_pclk",
	.msm_i2c_config_gpio	= gsbi_qup_i2c_gpio_config,
};

#ifdef CONFIG_ARCH_MSM7X27A
#if defined(_CONFIG_MACH_JENA) || defined(_CONFIG_MACH_TREBON)

#define MSM_PMEM_MDP_SIZE       0x2176000
#define MSM_PMEM_ADSP_SIZE      0x1000000
#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
/* prim = 480 x 800 x 3(bpp) x 3(pages) */
#define MSM_FB_SIZE             480 * 800 * 4 * 3
#else
/* prim = 480 x 800 x 4(bpp) x 2(pages) */
#define MSM_FB_SIZE             480 * 800 * 4 * 2
#endif /* CONFIG_FB_MSM_TRIPLE_BUFFER */

#else

#define MSM_PMEM_MDP_SIZE       0x2176000
#define MSM_PMEM_ADSP_SIZE      0x1000000
#define MSM_FB_SIZE             0x195000

#endif /* defined(_CONFIG_MACH_JENA) || ... */
#endif /* CONFIG_ARCH_MSM7X27A */

static char *usb_functions_default[] = {
	"usb_mass_storage",
};

static char *usb_functions_default_adb[] = {
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
#ifdef CONFIG_USB_F_SERIAL
	"modem",
#endif
	"diag",
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};

static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
#ifdef CONFIG_USB_F_SERIAL
	"modem",
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif
	"usb_mass_storage",
	"adb",
#ifdef CONFIG_USB_ANDROID_RMNET
	"rmnet",
#endif
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif

};

static struct android_usb_product usb_products[] = {
	{
		.product_id     = 0x681d,
		.num_functions	= ARRAY_SIZE(usb_functions_default),
		.functions      = usb_functions_default,
	},
	{
		.product_id	= 0x689e,
		.num_functions	= ARRAY_SIZE(usb_functions_default_adb),
		.functions	= usb_functions_default_adb,
	},
	{
		.product_id	= 0x6881,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},
};

#ifdef	CONFIG_MACH_VASTO
static struct usb_mass_storage_platform_data mass_storage_pdata = {
#if defined(CONFIG_MACH_VASTO_SKT) || defined(CONFIG_MACH_VASTO_KT)
	.nluns		= 2,
#else
	.nluns		= 1,
#endif	
	.vendor		= "SAMSUNG ",
#if defined(CONFIG_MACH_VASTO_SKT)
	.product	= "SHW-M340S",
#elif defined(CONFIG_MACH_VASTO_KT)
	.product	= "SHW-M340K",
#endif
	.release	= 0x0100,
};
#else
static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns		= 1,
	.vendor		= "SAMSUNG ",
	.product        = "GT-S5830 Card",
	.release	= 0x0100,
};
#endif

static struct platform_device usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &mass_storage_pdata,
	},
};

static struct usb_ether_platform_data rndis_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x04e8,
	.vendorDescr	= "Qualcomm Incorporated",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= 0x04e8,
//	.product_id	= 0x681d,
	.product_id	= 0x689e,
	.version	= 0x0100,
	.product_name	= "Samsung Android USB Device",
	.manufacturer_name = "SAMSUNG Electronics Co., Ltd.",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
	.serial_number = "1234567890ABCDEF",
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

static int __init board_serialno_setup(char *serialno)
{
	int i;
	char *src = serialno;

	/* create a fake MAC address from our serial number.
	 * first byte is 0x02 to signify locally administered.
	 */
	rndis_pdata.ethaddr[0] = 0x02;
	for (i = 0; *src; i++) {
		/* XOR the USB serial across the remaining bytes */
		rndis_pdata.ethaddr[i % (ETH_ALEN - 1) + 1] ^= *src++;
	}

	android_usb_pdata.serial_number = serialno;
	return 1;
}
__setup("androidboot.serialno=", board_serialno_setup);

#ifdef CONFIG_USB_EHCI_MSM_72K
static void msm_hsusb_vbus_power(unsigned phy_info, int on)
{
	int rc = 0;
	unsigned gpio;

	gpio = GPIO_HOST_VBUS_EN;

		rc = gpio_request(gpio, "i2c_host_vbus_en");
		if (rc < 0) {
			pr_err("failed to request %d GPIO\n", gpio);
			return;
		}
	gpio_direction_output(gpio, !!on);
	gpio_set_value_cansleep(gpio, !!on);
	gpio_free(gpio);
}

static struct msm_usb_host_platform_data msm_usb_host_pdata = {
	.phy_info       = (USB_PHY_INTEGRATED | USB_PHY_MODEL_45NM),
};

static void __init msm7x2x_init_host(void)
{
	msm_add_host(0, &msm_usb_host_pdata);
}
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
static int hsusb_rpc_connect(int connect)
{
	if (connect)
		return msm_hsusb_rpc_connect();
	else
		return msm_hsusb_rpc_close();
}

static struct vreg *vreg_3p3;
static struct vreg *vreg_1p8;

static int msm_hsusb_ldo_init(int init)
{
	if (init) {
#ifdef CONFIG_MACH_VASTO
    	vreg_3p3 = vreg_get(NULL, "vreg_usb30");
#else    
		if (board_hw_revision == 1)
			vreg_3p3 = vreg_get(NULL, "vreg_usb30");
		else
		vreg_3p3 = vreg_get(NULL, "vreg_usb33");
#endif
		if (IS_ERR(vreg_3p3))
			return PTR_ERR(vreg_3p3);

		vreg_1p8 = vreg_get(NULL, "vreg_usb18");
		if (IS_ERR(vreg_1p8))
			return PTR_ERR(vreg_1p8);
	} else {
		vreg_put(vreg_3p3);
		vreg_put(vreg_1p8);
	}

	return 0;
}

static int msm_hsusb_ldo_enable(int enable)
{
	static int ldo_status;

	if (!vreg_3p3 || IS_ERR(vreg_3p3))
		return -ENODEV;
	if (!vreg_1p8 || IS_ERR(vreg_1p8))
		return -ENODEV;

	if (ldo_status == enable)
		return 0;

	ldo_status = enable;

	if (enable) {
		vreg_enable(vreg_3p3);
		return vreg_enable(vreg_1p8);
	}

	vreg_disable(vreg_1p8);
	return vreg_disable(vreg_3p3);
}

#ifndef CONFIG_USB_EHCI_MSM_72K
static int msm_hsusb_pmic_notif_init(void (*callback)(int online), int init)
{
	int ret = 0;

	if (init)
		ret = msm_pm_app_rpc_init(callback);
	else
		msm_pm_app_rpc_deinit(callback);

	return ret;
}
#endif

static struct msm_otg_platform_data msm_otg_pdata = {
#ifndef CONFIG_USB_EHCI_MSM_72K
	.pmic_vbus_notif_init	 = msm_hsusb_pmic_notif_init,
#else
	.vbus_power		 = msm_hsusb_vbus_power,
#endif
	.rpc_connect		 = hsusb_rpc_connect,
	.core_clk		 = 1,
	.pemp_level		 = PRE_EMPHASIS_WITH_20_PERCENT,
	.cdr_autoreset		 = CDR_AUTO_RESET_DISABLE,
	.drv_ampl		 = HS_DRV_AMPLITUDE_75_PERCENT, //HS_DRV_AMPLITUDE_DEFAULT,
	.se1_gating		 = SE1_GATING_DISABLE,
	.ldo_init		 = msm_hsusb_ldo_init,
	.ldo_enable		 = msm_hsusb_ldo_enable,
	.chg_init		 = hsusb_chg_init,
	.phy_can_powercollapse = 1,
	/* XXX: block charger current setting */
#if 0
	.chg_connected		 = hsusb_chg_connected,
	.chg_vbus_draw		 = hsusb_chg_vbus_draw,
#endif
};
#endif

static struct resource smc91x_resources[] = {
	[0] = {
		.start = 0x90000300,
		.end   = 0x900003ff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = MSM_GPIO_TO_INT(4),
		.end   = MSM_GPIO_TO_INT(4),
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device smc91x_device = {
	.name           = "smc91x",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(smc91x_resources),
	.resource       = smc91x_resources,
};

#ifdef CONFIG_MACH_VASTO
//--> 2011.01.21 static wifi skb 
#define DHD_SKB_HDRSIZE 		336
#define DHD_SKB_1PAGE_BUFSIZE	((PAGE_SIZE*1)-DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE	((PAGE_SIZE*2)-DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE	((PAGE_SIZE*4)-DHD_SKB_HDRSIZE)

#define WLAN_SKB_BUF_NUM	17

struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];
EXPORT_SYMBOL(wlan_static_skb);

int __init init_wifi_mem(void)
{
	int i;
	int j;

	printk("init_wifi_mem\n");
	for (i = 0; i < 8; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_1PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}
	
	for (; i < 16; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_2PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}
	
	wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_4PAGE_BUFSIZE);
	if (!wlan_static_skb[i])
		goto err_skb_alloc;

	printk("init_wifi_mem success\n");
	return 0;

 err_skb_alloc:
	pr_err("Failed to skb_alloc for WLAN\n");
	for (j = 0 ; j <WLAN_SKB_BUF_NUM ; j++)
		dev_kfree_skb(wlan_static_skb[j]);

	return -ENOMEM;
}
//<-- 2011.01.21 static wifi skb 

void wlan_setup_power(int on, int detect)
{
	printk("%s %s --enter\n", __func__, on ? "on" : "down");

	if (detect != 1) {
			printk(/*KERN_DEBUG*/ "(on=%d, flag=%d)\n", on, detect);
	//For Starting/Stopping Tethering service
			if (on)
				gpio_direction_output(GPIO_WLAN_RESET_N, 1);
			else
				gpio_direction_output(GPIO_WLAN_RESET_N, 0);
			return;
	}

	if (on) {
		printk(KERN_DEBUG "WLAN GPIO state : GPIO_BT_PWR = %d, GPIO_WLAN_RESET_N = %d\n", 
					   gpio_get_value(GPIO_BT_PWR), gpio_get_value(GPIO_WLAN_RESET_N));

		if (gpio_get_value(GPIO_BT_PWR) == 0) {			
			gpio_direction_output(GPIO_BT_PWR, 1);	/* BT_WLAN_REG_ON */					
			mdelay(30);
		}			
		gpio_direction_output(GPIO_WLAN_RESET_N, 1);	/* WLAN_RESET */ 
              gpio_tlmm_config(GPIO_CFG(GPIO_WLAN_HOST_WAKE,0,GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);      
	}
	else {
		printk(KERN_DEBUG "WLAN GPIO state : GPIO_BT_PWR = %d, GPIO_WLAN_RESET_N = %d\n", 
					   gpio_get_value(GPIO_BT_PWR), gpio_get_value(GPIO_WLAN_RESET_N));

		gpio_direction_output(GPIO_WLAN_RESET_N, 0);	/* WLAN_RESET */				
		if(gpio_get_value(GPIO_BT_nRST) == 0) 	// PASCAL_WIFI_SETUP				
		{					 
			msleep(30);					 
			gpio_direction_output(GPIO_BT_PWR, 0);	/* BT_WLAN_REG_ON */				
		}
	}	
#ifndef ATH_POLLING
	mdelay(100);
	if (detect) {
		/* Detect card */
		if (wlan_status_notify_cb)
			wlan_status_notify_cb(on, wlan_devid);
		else
			printk(KERN_ERR "WLAN: No notify available\n");
	}
#endif /* ATH_POLLING */
}
EXPORT_SYMBOL(wlan_setup_power);

#else  //CONFIG_MACH_VASTO

#ifdef WLAN_HOST_WAKE
struct wlansleep_info {
	unsigned host_wake;
	unsigned host_wake_irq;
	struct wake_lock wake_lock;
};


static struct wlansleep_info *wsi;
static struct tasklet_struct hostwake_task;


static void wlan_hostwake_task(unsigned long data)
{
	printk(KERN_INFO "WLAN: wake lock timeout 0.5 sec...\n");

	wake_lock_timeout(&wsi->wake_lock, HZ / 2);
}


static irqreturn_t wlan_hostwake_isr(int irq, void *dev_id)
{
//please fix    gpio_clear_detect_status(wsi->host_wake_irq);

	/* schedule a tasklet to handle the change in the host wake line */
	tasklet_schedule(&hostwake_task);
	return IRQ_HANDLED;
}


static int wlan_host_wake_init(void)
{
	int ret;

	wsi = kzalloc(sizeof(struct wlansleep_info), GFP_KERNEL);
	if (!wsi)
		return -ENOMEM;

	wake_lock_init(&wsi->wake_lock, WAKE_LOCK_SUSPEND, "bluesleep");
	tasklet_init(&hostwake_task, wlan_hostwake_task, 0);

	wsi->host_wake = GPIO_WLAN_HOST_WAKE;
	wsi->host_wake_irq = MSM_GPIO_TO_INT(wsi->host_wake);

//please fix    gpio_configure(wsi->host_wake, GPIOF_INPUT);
	ret = request_irq(wsi->host_wake_irq, wlan_hostwake_isr,
						IRQF_DISABLED | IRQF_TRIGGER_RISING,
						"wlan hostwake", NULL);
	if (ret < 0) {
		printk(KERN_ERR "WLAN: Couldn't acquire WLAN_HOST_WAKE IRQ");
		return -1;
	}

	ret = enable_irq_wake(wsi->host_wake_irq);
	if (ret < 0) {
		printk(KERN_ERR "WLAN: Couldn't enable WLAN_HOST_WAKE as wakeup interrupt");
		free_irq(wsi->host_wake_irq, NULL);
		return -1;
	}

	return 0;
}


static void wlan_host_wake_exit(void)
{
	if (disable_irq_wake(wsi->host_wake_irq))
		printk(KERN_ERR "WLAN: Couldn't disable hostwake IRQ wakeup mode \n");

	free_irq(wsi->host_wake_irq, NULL);

	wake_lock_destroy(&wsi->wake_lock);
	kfree(wsi);
}
#endif /* WLAN_HOST_WAKE */


#if 1 // vasto_gpio def _CONFIG_MACH_TREBON
static int wlan_setup_ldo(int on)
{
      // To do
      return 0;
}
#else
#define GPIO_WLAN_MAX (2)

static unsigned wlan_gpio_table[GPIO_WLAN_MAX] = {
	GPIO_WLAN_18V_EN,
	GPIO_WLAN_33V_EN
};

static int wlan_setup_ldo(int on)
{
	int i = 0;
	int rc = 0;

	printk("%s %s\n", __func__, on ? "on" : "off");

	for (i = 0; i < GPIO_WLAN_MAX; i++)
	{
		// Set config
		if (gpio_tlmm_config(GPIO_CFG(wlan_gpio_table[i], 0,
							GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
							GPIO_CFG_2MA), GPIO_CFG_ENABLE)) {
			printk(KERN_ERR "%s: gpio_tlmm_config for %d failed\n",
					__func__, wlan_gpio_table[i]);
			return -1;
		}

		// Request
		if (gpio_request(wlan_gpio_table[i], "wlan_ar6000_pm")) {
			printk(KERN_ERR "%s: gpio_request for %d failed\n",
					__func__, wlan_gpio_table[i]);
			return -1;
		}

		// Direction Output On/Off
		rc = gpio_direction_output(wlan_gpio_table[i], on);
		gpio_free(wlan_gpio_table[i]);

		if (rc) {
			printk(KERN_ERR "%s: gpio_direction_output for %d failed\n",
					__func__, wlan_gpio_table[i]);
			return -1;
		}
	}
	return 0;
}
#endif


void wlan_setup_power(int on, int detect)
{
	int rc = 0;
#if 0
	// Set config
	if (gpio_tlmm_config(GPIO_CFG(GPIO_WLAN_RESET_N, 0,
						GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN,
						GPIO_CFG_2MA), GPIO_CFG_ENABLE)) {
		printk(KERN_ERR "%s: gpio_tlmm_config for %d failed\n",
				__func__, GPIO_WLAN_RESET_N);
		return;
	}
#endif

	printk("%s %s --enter\n", __func__, on ? "on" : "down");

	// Request
	if (gpio_request(GPIO_WLAN_RESET_N, "wlan_ar6000_pm")) {
		printk(KERN_ERR "%s: gpio_request for %d failed\n",
				__func__, GPIO_WLAN_RESET_N);
		return;
	}

	if (on) {
		printk("%s -1\n", __func__);
		//if (gpio_get_value(GPIO_BT_PWR) == 0) {
			printk("%s -2\n", __func__);
			if (wlan_setup_ldo(1))
				return;

			mdelay(30);
		//}		

		printk("%s -3\n", __func__);

		rc = gpio_direction_output(GPIO_WLAN_RESET_N, 1);
		gpio_free(GPIO_WLAN_RESET_N);

		if (rc) {
			printk(KERN_ERR "%s: gpio_direction_output for %d failed\n",
					__func__, GPIO_WLAN_RESET_N);
			return;
		}

#ifdef WLAN_HOST_WAKE
		wlan_host_wake_init();
#endif /* WLAN_HOST_WAKE */
	}
	else {
		printk("%s -4\n", __func__);
#ifdef WLAN_HOST_WAKE
		wlan_host_wake_exit();
#endif /* WLAN_HOST_WAKE */

		rc = gpio_direction_output(GPIO_WLAN_RESET_N, 0);
		gpio_free(GPIO_WLAN_RESET_N);

		if (rc) {
			printk(KERN_ERR "%s: gpio_direction_output for %d failed\n",
					__func__, GPIO_WLAN_RESET_N);
			return;
		}

#if 1 //defined(CONFIG_MACH_ROOKIE) || defined(CONFIG_MACH_ESCAPE) || defined(CONFIG_MACH_GIO)/* Atheros */
		//if (gpio_get_value(GPIO_BT_PWR) == 0) {
			mdelay(30);
			if (wlan_setup_ldo(0))
				return;
		//}
#else
		if(gpio_get_value(GPIO_BT_RESET) == 0) {
			msleep(150);

			if (gpio_request(GPIO_BT_WLAN_REG_ON, "wlan_ar6000_pm") < 0) {
				printk(KERN_ERR "GPIO_BT_WLAN_REG_ON gpio_request fail. \n");
			}

			gpio_direction_output(GPIO_BT_WLAN_REG_ON, 0);	/* BT_WLAN_REG_ON */
		}
#endif
	}

#ifndef ATH_POLLING
	mdelay(100);

	if (detect) {
		/* Detect card */
		if (wlan_status_notify_cb)
			wlan_status_notify_cb(on, wlan_devid);
		else
			printk(KERN_ERR "WLAN: No notify available\n");
	}
#endif /* ATH_POLLING */
}
EXPORT_SYMBOL(wlan_setup_power);

#endif  //CONFIG_MACH_VASTO

#if (defined(CONFIG_MMC_MSM_SDC1_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC2_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC3_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC4_SUPPORT))

static unsigned long vreg_sts, gpio_sts;
static struct vreg *vreg_mmc;
static struct vreg *vreg_emmc;

struct sdcc_vreg {
	struct vreg *vreg_data;
	unsigned level;
};

static struct sdcc_vreg sdcc_vreg_data[4];

struct sdcc_gpio {
	struct msm_gpio *cfg_data;
	uint32_t size;
	struct msm_gpio *sleep_cfg_data;
};

/**
 * Due to insufficient drive strengths for SDC GPIO lines some old versioned
 * SD/MMC cards may cause data CRC errors. Hence, set optimal values
 * for SDC slots based on timing closure and marginality. SDC1 slot
 * require higher value since it should handle bad signal quality due
 * to size of T-flash adapters.
 */
static struct msm_gpio sdc1_cfg_data[] = {
	{GPIO_CFG(51, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_14MA),
								"sdc1_dat_3"},
	{GPIO_CFG(52, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_14MA),
								"sdc1_dat_2"},
	{GPIO_CFG(53, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_14MA),
								"sdc1_dat_1"},
	{GPIO_CFG(54, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_14MA),
								"sdc1_dat_0"},
	{GPIO_CFG(55, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_14MA),
								"sdc1_cmd"},
	{GPIO_CFG(56, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_14MA),
								"sdc1_clk"},
};

#ifdef CONFIG_MACH_VASTO  // sd verg timing (H/W request)
static struct msm_gpio sdc1_sleep_cfg_data[] = {
	{GPIO_CFG(51, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc1_dat_3"},
	{GPIO_CFG(52, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc1_dat_2"},
	{GPIO_CFG(53, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc1_dat_1"},
	{GPIO_CFG(54, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc1_dat_0"},
	{GPIO_CFG(55, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc1_cmd"},
	{GPIO_CFG(56, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc1_clk"},
};
#endif

static struct msm_gpio sdc2_cfg_data[] = {
	{GPIO_CFG(GPIO_WLAN_SD_CLK, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
								"sdc2_clk"},
	{GPIO_CFG(GPIO_WLAN_SD_CMD, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_cmd"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_3, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_dat_3"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_2, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_dat_2"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_1, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_dat_1"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_0, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc2_dat_0"},
};
#ifdef CONFIG_MACH_VASTO
static struct msm_gpio sdc2_sleep_cfg_data[] = {
	{GPIO_CFG(GPIO_WLAN_SD_CLK, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc2_clk"},
	{GPIO_CFG(GPIO_WLAN_SD_CMD, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_cmd"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_3, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_dat_3"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_2, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_dat_2"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_1, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_dat_1"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_0, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
								"sdc2_dat_0"},
};
#else
static struct msm_gpio sdc2_sleep_cfg_data[] = {
	{GPIO_CFG(GPIO_WLAN_SD_CLK, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc2_clk"},
	{GPIO_CFG(GPIO_WLAN_SD_CMD, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc2_cmd"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_3, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc2_dat_3"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_2, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc2_dat_2"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_1, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc2_dat_1"},
	{GPIO_CFG(GPIO_WLAN_SD_DATA_0, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc2_dat_0"},
};
#endif
static struct msm_gpio sdc3_cfg_data[] = {
	{GPIO_CFG(88, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
								"sdc3_clk"},
	{GPIO_CFG(89, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_cmd"},
	{GPIO_CFG(90, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_3"},
	{GPIO_CFG(91, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_2"},
	{GPIO_CFG(92, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_1"},
	{GPIO_CFG(93, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_0"},
#ifdef CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT
	{GPIO_CFG(19, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_7"},
	{GPIO_CFG(20, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_6"},
	{GPIO_CFG(21, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_5"},
	{GPIO_CFG(108, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc3_dat_4"},
#endif
};

#ifdef CONFIG_SD_POWER_CONTROL_BY_GPIO
static struct msm_gpio sdc3_sleep_cfg_data[] = {
	{GPIO_CFG(88, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_clk"},
	{GPIO_CFG(89, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_cmd"},
	{GPIO_CFG(90, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_dat_3"},
	{GPIO_CFG(91, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_dat_2"},
	{GPIO_CFG(92, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_dat_1"},
	{GPIO_CFG(93, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_dat_0"},
#ifdef CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT
	{GPIO_CFG(19, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_dat_7"},
	{GPIO_CFG(20, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_dat_6"},
	{GPIO_CFG(21, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_dat_5"},
	{GPIO_CFG(108, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
								"sdc3_dat_4"},
#endif
};
#endif

static struct msm_gpio sdc4_cfg_data[] = {
	{GPIO_CFG(19, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_dat_3"},
	{GPIO_CFG(20, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_dat_2"},
	{GPIO_CFG(21, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_dat_1"},
	{GPIO_CFG(107, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_cmd"},
	{GPIO_CFG(108, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_10MA),
								"sdc4_dat_0"},
	{GPIO_CFG(109, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
								"sdc4_clk"},
};

static struct sdcc_gpio sdcc_cfg_data[] = {
	{
		.cfg_data = sdc1_cfg_data,
#ifdef CONFIG_MACH_VASTO      
  		.sleep_cfg_data = sdc1_sleep_cfg_data,
#endif  		
		.size = ARRAY_SIZE(sdc1_cfg_data),
	},
	{
		.cfg_data = sdc2_cfg_data,
		.size = ARRAY_SIZE(sdc2_cfg_data),
		.sleep_cfg_data = sdc2_sleep_cfg_data,
	},
	{
		.cfg_data = sdc3_cfg_data,
		.size = ARRAY_SIZE(sdc3_cfg_data),
#ifdef CONFIG_SD_POWER_CONTROL_BY_GPIO
		.sleep_cfg_data = sdc3_sleep_cfg_data,
#endif		
	},
	{
		.cfg_data = sdc4_cfg_data,
		.size = ARRAY_SIZE(sdc4_cfg_data),
	},
};

static int msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct sdcc_gpio *curr;

	curr = &sdcc_cfg_data[dev_id - 1];
	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return rc;

	if (enable) {
		set_bit(dev_id, &gpio_sts);
		rc = msm_gpios_request_enable(curr->cfg_data, curr->size);
		if (rc)
			pr_err("%s: Failed to turn on GPIOs for slot %d\n",
					__func__,  dev_id);
	} else {
		clear_bit(dev_id, &gpio_sts);
		if (curr->sleep_cfg_data) {
			rc = msm_gpios_enable(curr->sleep_cfg_data, curr->size);
			msm_gpios_free(curr->sleep_cfg_data, curr->size);
			return rc;
		}
		msm_gpios_disable_free(curr->cfg_data, curr->size);
	}
	return rc;
}

static int msm_sdcc_setup_vreg(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct sdcc_vreg *curr;

	curr = &sdcc_vreg_data[dev_id - 1];

	printk("%s : %d : %d : level : %d\n", __func__, dev_id, enable, curr->level);

	if (!(test_bit(dev_id, &vreg_sts)^enable))
		return rc;

	if (enable) {
		set_bit(dev_id, &vreg_sts);
#ifdef CONFIG_SD_POWER_CONTROL_BY_GPIO
		if (dev_id == 3)
		{
			gpio_direction_output(41, 1);
		}
		else
#endif          
		{
			rc = vreg_set_level(curr->vreg_data, curr->level);
			if (rc)
				pr_err("%s: vreg_set_level() = %d\n", __func__, rc);

			rc = vreg_enable(curr->vreg_data);
			if (rc)
				pr_err("%s: vreg_enable() = %d\n", __func__, rc);
		}
	} else {
		clear_bit(dev_id, &vreg_sts);
#ifdef CONFIG_SD_POWER_CONTROL_BY_GPIO
		if (dev_id == 3)
		{
			gpio_direction_output(41, 0);
		}
		else
#endif          
		{
			rc = vreg_disable(curr->vreg_data);
			if (rc)
				pr_err("%s: vreg_disable() = %d\n", __func__, rc);
		}		
		msleep(50);
	}
	return rc;
}

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);

	rc = msm_sdcc_setup_gpio(pdev->id, !!vdd);
	if (rc)
		goto out;

	rc = msm_sdcc_setup_vreg(pdev->id, !!vdd);
out:
	return rc;
}

#define GPIO_SDC1_HW_DET 94

#if defined(CONFIG_MMC_MSM_SDC1_SUPPORT) \
	&& defined(CONFIG_MMC_MSM_CARD_HW_DETECTION)
static unsigned int msm7x2xa_sdcc_slot_status(struct device *dev)
{
	int status;

	printk("%s entered\n", __func__);

	status = gpio_tlmm_config(GPIO_CFG(GPIO_SDC1_HW_DET, 2, GPIO_CFG_INPUT,
			GPIO_CFG_PULL_UP, GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (status)
		pr_err("%s:Failed to configure tlmm for GPIO %d\n", __func__,
				GPIO_SDC1_HW_DET);

	status = gpio_request(GPIO_SDC1_HW_DET, "SD_HW_Detect");
	if (status) {
		pr_err("%s:Failed to request GPIO %d\n", __func__,
				GPIO_SDC1_HW_DET);
	} else {
		status = gpio_direction_input(GPIO_SDC1_HW_DET);
		if (!status)
			status = gpio_get_value(GPIO_SDC1_HW_DET);
		gpio_free(GPIO_SDC1_HW_DET);
	}

	status = status?0:1 ; //PMMC
	printk("<=PMMC=> %s : status : %d \n", __func__, status);
	return status;
}
#endif

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static struct mmc_platform_data sdc1_plat_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status      = msm7x2xa_sdcc_slot_status,
	.status_irq  = MSM_GPIO_TO_INT(GPIO_SDC1_HW_DET),
	.irq_flags   = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif
};
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static struct mmc_platform_data sdc2_plat_data = {
	/*
	 * SDC2 supports only 1.8V, claim for 2.85V range is just
	 * for allowing buggy cards who advertise 2.8V even though
	 * they can operate at 1.8V supply.
	 */
	.ocr_mask	= MMC_VDD_28_29 | MMC_VDD_165_195,
	.translate_vdd  = msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_SDIO_SUPPORT
	.sdiowakeup_irq = MSM_GPIO_TO_INT(66),
#endif
#ifndef ATH_POLLING
	.status = wlan_status,
	.register_status_notify = register_wlan_status_notify,
#endif /* ATH_POLLING */
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000, //24576000, ///*144000,//*/ 
#ifdef CONFIG_MMC_MSM_SDC2_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
};
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static struct mmc_platform_data sdc3_plat_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
#ifdef CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 1,
};
#endif

#if (defined(CONFIG_MMC_MSM_SDC4_SUPPORT)\
		&& !defined(CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT))
static struct mmc_platform_data sdc4_plat_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
};
#endif
#endif

#ifdef CONFIG_SERIAL_MSM_HS
static struct msm_serial_hs_platform_data msm_uart_dm1_pdata = {
	.inject_rx_on_wakeup	= 1,
	.rx_to_inject		= 0xFD,
};
#endif
static struct msm_pm_platform_data msm7x27a_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 16000,
					.residency = 20000,
	},
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 12000,
					.residency = 20000,
	},
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 0,
					.suspend_enabled = 1,
					.latency = 2000,
					.residency = 0,
	},
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 2,
					.residency = 0,
	},
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static unsigned pmem_mdp_size = MSM_PMEM_MDP_SIZE;
static int __init pmem_mdp_size_setup(char *p)
{
	pmem_mdp_size = memparse(p, NULL);
	return 0;
}

early_param("pmem_mdp_size", pmem_mdp_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;
static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}

early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned fb_size = MSM_FB_SIZE;
static int __init fb_size_setup(char *p)
{
	fb_size = memparse(p, NULL);
	return 0;
}

early_param("fb_size", fb_size_setup);


#define LCDC_CONFIG_PROC          21
#define LCDC_UN_CONFIG_PROC       22
#define LCDC_API_PROG             0x30000066
#define LCDC_API_VERS             0x00010001

#define	GPIO_SPI_CLK		30
#define	GPIO_SPI_CS		26
#define	GPIO_SPI_SDI		57
#define	GPIO_LCD_RESET_N	22

//static struct msm_rpc_endpoint *lcdc_ep;

static int msm_fb_lcdc_config(int on)
{
#if 1
	/*
	 * for avoiding potential rpc error
	 */
	return 0;
#else
	int rc = 0;
	struct rpc_request_hdr hdr;

	if (on)
		pr_info("lcdc config\n");
	else
		pr_info("lcdc un-config\n");

	lcdc_ep = msm_rpc_connect_compatible(LCDC_API_PROG, LCDC_API_VERS, 0);

	if (IS_ERR(lcdc_ep)) {
		printk(KERN_ERR "%s: msm_rpc_connect failed! rc = %ld\n",
			__func__, PTR_ERR(lcdc_ep));
		return -EINVAL;
	}

	rc = msm_rpc_call(lcdc_ep,
		(on) ? LCDC_CONFIG_PROC : LCDC_UN_CONFIG_PROC,
		&hdr, sizeof(hdr), 5 * HZ);

	if (rc)
		printk(KERN_ERR
			"%s: msm_rpc_call failed! rc = %d\n", __func__, rc);

	msm_rpc_close(lcdc_ep);
	return rc;
#endif
}

static const char * const msm_fb_lcdc_vreg[] = {
		"gp2",
		"msme1",
};

static const int msm_fb_lcdc_vreg_mV[] = {
	2850,
	1800,
};

struct vreg *lcdc_vreg[ARRAY_SIZE(msm_fb_lcdc_vreg)];

#if 0 // toshiba panel
static uint32_t lcdc_gpio_initialized;

static void lcdc_toshiba_gpio_init(void)
{
	int i, rc = 0;
	if (!lcdc_gpio_initialized) {
		if (gpio_request(GPIO_SPI_CLK, "spi_clk")) {
			pr_err("failed to request gpio spi_clk\n");
			return;
		}
		if (gpio_request(GPIO_SPI_CS0_N, "spi_cs")) {
			pr_err("failed to request gpio spi_cs0_N\n");
			goto fail_gpio6;
		}
		if (gpio_request(GPIO_SPI_MOSI, "spi_mosi")) {
			pr_err("failed to request gpio spi_mosi\n");
			goto fail_gpio5;
		}
		if (gpio_request(GPIO_SPI_MISO, "spi_miso")) {
			pr_err("failed to request gpio spi_miso\n");
			goto fail_gpio4;
		}
		if (gpio_request(GPIO_DISPLAY_PWR_EN, "gpio_disp_pwr")) {
			pr_err("failed to request gpio_disp_pwr\n");
			goto fail_gpio3;
		}
		if (gpio_request(GPIO_BACKLIGHT_EN, "gpio_bkl_en")) {
			pr_err("failed to request gpio_bkl_en\n");
			goto fail_gpio2;
		}
		pmapp_disp_backlight_init();

		for (i = 0; i < ARRAY_SIZE(msm_fb_lcdc_vreg); i++) {
			lcdc_vreg[i] = vreg_get(0, msm_fb_lcdc_vreg[i]);

			rc = vreg_set_level(lcdc_vreg[i],
						msm_fb_lcdc_vreg_mV[i]);

			if (rc < 0) {
				pr_err("%s: set regulator level failed "
					"with :(%d)\n", __func__, rc);
				goto fail_gpio1;
			}
		}
		lcdc_gpio_initialized = 1;
	}
	return;

fail_gpio1:
	for (; i > 0; i--)
			vreg_put(lcdc_vreg[i - 1]);

	gpio_free(GPIO_BACKLIGHT_EN);
fail_gpio2:
	gpio_free(GPIO_DISPLAY_PWR_EN);
fail_gpio3:
	gpio_free(GPIO_SPI_MISO);
fail_gpio4:
	gpio_free(GPIO_SPI_MOSI);
fail_gpio5:
	gpio_free(GPIO_SPI_CS0_N);
fail_gpio6:
	gpio_free(GPIO_SPI_CLK);
	lcdc_gpio_initialized = 0;
}

static uint32_t lcdc_gpio_table[] = {
	GPIO_SPI_CLK,
	GPIO_SPI_CS0_N,
	GPIO_SPI_MOSI,
	GPIO_DISPLAY_PWR_EN,
	GPIO_BACKLIGHT_EN,
	GPIO_SPI_MISO,
};

static void config_lcdc_gpio_table(uint32_t *table, int len, unsigned enable)
{
	int n;

	if (lcdc_gpio_initialized) {
		/* All are IO Expander GPIOs */
		for (n = 0; n < (len - 1); n++)
			gpio_direction_output(table[n], 1);
	}
}

static void lcdc_toshiba_config_gpios(int enable)
{
	config_lcdc_gpio_table(lcdc_gpio_table,
		ARRAY_SIZE(lcdc_gpio_table), enable);
}
#endif

static int lcdc_gpio_num[] = {
	GPIO_SPI_CLK,
	GPIO_SPI_CS,
	GPIO_SPI_SDI,
	GPIO_LCD_RESET_N,
};

static void lcdc_gpio_init(void)
{
	if (gpio_request(GPIO_SPI_CLK, "spi_clk")) {
		pr_err("failed to request gpio spi_clk\n");
	}
	if (gpio_request(GPIO_SPI_CS, "spi_cs")) {
		pr_err("failed to request gpio spi_cs\n");
	}
	if (gpio_request(GPIO_SPI_SDI, "spi_mosi")) {
		pr_err("failed to request gpio spi_sdi\n");
	}
	if (gpio_request(GPIO_LCD_RESET_N, "gpio_lcd_reset_n")) {
		pr_err("failed to request gpio lcd_reset_n\n");
	}

	return;
}

static uint32_t lcdc_gpio_table[] = {
	GPIO_CFG(GPIO_SPI_CLK, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_SPI_CS, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_SPI_SDI, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_LCD_RESET_N,  0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static void config_lcdc_gpio_table(uint32_t *table, int len, unsigned enable)
{
	int n, rc;

	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n],
			enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}
static int msm_fb_lcdc_power_save(int on)
{
#if 0
	/* struct vreg *vreg[ARRAY_SIZE(msm_fb_lcdc_vreg)]; */
	int rc = 0;

	/* Doing the init of the LCDC GPIOs very late as they are from
		an I2C-controlled IO Expander */
	lcdc_toshiba_gpio_init();

	if (lcdc_gpio_initialized) {
		gpio_set_value_cansleep(GPIO_DISPLAY_PWR_EN, on);
		gpio_set_value_cansleep(GPIO_BACKLIGHT_EN, on);
	}

	pmapp_disp_backlight_init();
	rc = pmapp_disp_backlight_set_brightness(100);

#endif
    return 0;
}

static struct lcdc_platform_data lcdc_pdata = {
	.lcdc_gpio_config = NULL,
	.lcdc_power_save   = msm_fb_lcdc_power_save,
};

#if defined(CONFIG_FB_MSM_LCDC_S6E63M0_WVGA)
static void lcdc_s6d04m0_config_gpios(int enable)
{
	config_lcdc_gpio_table(lcdc_gpio_table,
		ARRAY_SIZE(lcdc_gpio_table), enable);
}

static struct msm_panel_common_pdata lcdc_s6e63m0_panel_data = {
	.panel_config_gpio = lcdc_s6d04m0_config_gpios,
	.gpio_num          = lcdc_gpio_num,
};

static struct platform_device lcdc_s6e63m0_panel_device = {
	.name   = "lcdc_s6e63m0_wvga",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_s6e63m0_panel_data,
	}
};
#endif

/*
 * S6D16A0X_HVGA 
 */
#if defined(CONFIG_FB_MSM_LCDC_S6D16A0X_HVGA)
static void lcdc_s6d16a0x_config_gpios(int enable)
{
	config_lcdc_gpio_table(lcdc_gpio_table,
		ARRAY_SIZE(lcdc_gpio_table), enable);
}

static struct msm_panel_common_pdata lcdc_s6d16a0x_panel_data = {
	.panel_config_gpio = lcdc_s6d16a0x_config_gpios,
	.gpio_num	  = lcdc_gpio_num,
};

static struct platform_device lcdc_s6d16a0x_panel_device = {
#if defined(CONFIG_FB_MSM_LCDC_TREBON_HVGA) 
	.name   = "lcdc_trebon_hvga",
#else
	.name   = "lcdc_s6d16a0x_hvga",
#endif
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_s6d16a0x_panel_data,
	}
};
#endif

#if 0 // toshiba panel
static int lcd_panel_spi_gpio_num[] = {
		GPIO_SPI_MOSI,  /* spi_sdi */
		GPIO_SPI_MISO,  /* spi_sdoi */
		GPIO_SPI_CLK,   /* spi_clk */
		GPIO_SPI_CS0_N, /* spi_cs  */
};

static struct msm_panel_common_pdata lcdc_toshiba_panel_data = {
	.panel_config_gpio = lcdc_toshiba_config_gpios,
	.gpio_num	  = lcd_panel_spi_gpio_num,
};

static struct platform_device lcdc_toshiba_panel_device = {
	.name   = "lcdc_toshiba_fwvga_pt",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_toshiba_panel_data,
	}
};
#endif

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
	int ret = -EPERM;

#if defined (CONFIG_FB_MSM_LCDC_S6E63M0_WVGA)
		if (!strcmp(name, "lcdc_s6e63m0_wvga"))
			ret = 0;
		else
			ret = -ENODEV;
#endif
#if defined(CONFIG_FB_MSM_LCDC_S6D16A0X_HVGA)
		if (!strcmp(name, "lcdc_s6d16a0x_hvga"))
			ret = 0;
		else
			ret = -ENODEV;
#elif defined(CONFIG_FB_MSM_LCDC_TREBON_HVGA) 
		if (!strcmp(name, "lcdc_trebon_hvga"))
			ret = 0;
		else
			ret = -ENODEV;
#else
#if 0 // toshiba panel
	if (machine_is_msm7x27a_surf()) {
		if (!strncmp(name, "lcdc_toshiba_fwvga_pt", 21))
			ret = 0;
	} else {
		ret = -ENODEV;
	}

#endif
#endif
    return ret;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
	.dev    = {
		.platform_data = &msm_fb_pdata,
	}
};

#ifdef CONFIG_FB_MSM_MIPI_DSI
static int mipi_renesas_set_bl(int level)
{
	int ret;

	ret = pmapp_disp_backlight_set_brightness(level);

	if (ret)
		pr_err("%s: can't set lcd backlight!\n", __func__);

	return ret;
}

static struct msm_panel_common_pdata mipi_renesas_pdata = {
	.pmic_backlight = mipi_renesas_set_bl,
};


static struct platform_device mipi_dsi_renesas_panel_device = {
	.name = "mipi_renesas",
	.id = 0,
	.dev    = {
		.platform_data = &mipi_renesas_pdata,
	}
};
#endif


static void __init msm7x27a_init_mmc(void)
{
	int rc;
    
	vreg_emmc = vreg_get(NULL, "vreg_msme");
	if (IS_ERR(vreg_emmc)) {
		pr_err("%s: vreg get failed (%ld)\n",
				__func__, PTR_ERR(vreg_emmc));
		return;
	}

	vreg_mmc = vreg_get(NULL, "vreg_tflash");
	if (IS_ERR(vreg_mmc)) {
		pr_err("%s: vreg get failed (%ld)\n",
				__func__, PTR_ERR(vreg_mmc));
		return;
	}

#ifdef CONFIG_MACH_VASTO
	if (gpio_request(GPIO_BT_PWR, "wlan_bt_enable"))
		pr_err("failed to request gpio wlan_bt_enable\n");
	if (gpio_request(GPIO_WLAN_RESET_N, "wlan_reset"))
		pr_err("failed to request wlan_reset\n");
	if (gpio_request(GPIO_WLAN_HOST_WAKE, "wlan_wake_lock"))
		pr_err("failed to request wlan_wake_lock\n");	
#endif

	/* eMMC slot */
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
#ifdef CONFIG_SD_POWER_CONTROL_BY_GPIO
	if (gpio_request(41, "sdc3_vdd"))
		pr_err("failed to request gpio sdc3_vdd\n");    
	gpio_direction_output(41, 1);  
	rc = gpio_tlmm_config(GPIO_CFG(41, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO %d\n", __func__, rc);    
#else
	sdcc_vreg_data[2].vreg_data = vreg_emmc;
#if defined(CONFIG_MACH_VASTO)
	sdcc_vreg_data[2].level = 1800;
#else
	sdcc_vreg_data[2].level = 3000;
#endif
#endif
	msm_add_sdcc(3, &sdc3_plat_data);
#endif
	/* Micro-SD slot */
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	sdcc_vreg_data[0].vreg_data = vreg_mmc;
	sdcc_vreg_data[0].level = 2850;
	msm_add_sdcc(1, &sdc1_plat_data);
#endif
	/* SDIO WLAN slot */
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	//sdcc_vreg_data[1].vreg_data = vreg_mmc;
	sdcc_vreg_data[1].vreg_data = vreg_emmc;
	sdcc_vreg_data[1].level = 1800/*2850*/;
	msm_add_sdcc(2, &sdc2_plat_data);
#endif
	/* Not Used */
#if (defined(CONFIG_MMC_MSM_SDC4_SUPPORT)\
		&& !defined(CONFIG_MMC_MSM_SDC3_8_BIT_SUPPORT))
	sdcc_vreg_data[3].vreg_data = vreg_mmc;
	sdcc_vreg_data[3].level = 2850;
	msm_add_sdcc(4, &sdc4_plat_data);
#endif
}


#define SND(desc, num) { .name = #desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
	SND(HANDSET, 0),
	SND(MONO_HEADSET, 2),
	SND(HEADSET, 3),
	SND(SPEAKER, 6),
	SND(TTY_HEADSET, 8),
	SND(TTY_VCO, 9),
	SND(TTY_HCO, 10),
	SND(BT, 12),
	SND(IN_S_SADC_OUT_HANDSET, 16),
	SND(VOICE_RECOGNITION, 24), // voice recognition
	SND(IN_S_SADC_OUT_SPEAKER_PHONE, 25),
	SND(FM_DIGITAL_STEREO_HEADSET, 26),
	SND(FM_DIGITAL_SPEAKER_PHONE, 27),
	SND(FM_DIGITAL_BT_A2DP_HEADSET, 28),
	SND(FM_STEREO_HEADSET, 29),
	SND(FM_SPEAKER_PHONE, 30),
	SND(STEREO_HEADSET_AND_SPEAKER, 31),
	SND(HEADSET_AND_SPEAKER, 32),
	SND(STEREO_HEADSET_3POLE, 34),
	SND(MP3_SPEAKER_PHONE, 35),
	SND(MP3_STEREO_HEADSET, 36),
	SND(BT_NSEC_OFF, 37),	
	SND(HANDSET_VOIP, 38),	
	SND(SPEAKER_VOIP, 39),	
	SND(STEREO_HEADSET_VOIP, 40),	
	SND(STEREO_HEADSET_3POLE_VOIP, 41),	
	SND(FORCE_ONLY_SPEAKER, 42),	//kkuram force only speaker	
	SND(CURRENT, 44),				//kkuram force only speaker	39->40 
};
#undef SND

static struct msm_snd_endpoints msm_device_snd_endpoints = {
	.endpoints = snd_endpoints_list,
	.num = sizeof(snd_endpoints_list) / sizeof(struct snd_endpoint)
};

static struct platform_device msm_device_snd = {
	.name = "msm_snd",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_snd_endpoints
	},
};

#define DEC0_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC3_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC4_FORMAT (1<<MSM_ADSP_CODEC_MIDI)

static unsigned int dec_concurrency_table[] = {
	/* Audio LP */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DMA)), 0,
	0, 0, 0,

	/* Concurrency 1 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	 /* Concurrency 2 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 3 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 4 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 5 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 6 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	0, 0, 0, 0,

	/* Concurrency 7 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),
};

#define DEC_INFO(name, queueid, decid, nr_codec) { .module_name = name, \
	.module_queueid = queueid, .module_decid = decid, \
	.nr_codec_support = nr_codec}

static struct msm_adspdec_info dec_info_list[] = {
	DEC_INFO("AUDPLAY0TASK", 13, 0, 11), /* AudPlay0BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY1TASK", 14, 1, 11),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 11),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 11),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / \
					ARRAY_SIZE(dec_info_list)),
	.dec_concurrency_table = dec_concurrency_table,
	.dec_info_list = dec_info_list,
};

static struct platform_device msm_device_adspdec = {
	.name = "msm_adspdec",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_adspdec_database
	},
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};
static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

#ifdef CONFIG_BATTERY_MSM
static u32 msm_calculate_batt_capacity(u32 current_voltage);

static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design     = 2800,
	.voltage_max_design     = 4300,
	.avail_chg_sources      = AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
	.calculate_capacity     = &msm_calculate_batt_capacity,
};

static u32 msm_calculate_batt_capacity(u32 current_voltage)
{
	u32 low_voltage	 = msm_psy_batt_data.voltage_min_design;
	u32 high_voltage = msm_psy_batt_data.voltage_max_design;

	return (current_voltage - low_voltage) * 100
			/ (high_voltage - low_voltage);
}

static struct platform_device msm_batt_device = {
	.name               = "msm-battery",
	.id                 = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};
#endif

static struct smsc911x_platform_config smsc911x_config = {
	.irq_polarity	= SMSC911X_IRQ_POLARITY_ACTIVE_HIGH,
	.irq_type	= SMSC911X_IRQ_TYPE_PUSH_PULL,
	.flags		= SMSC911X_USE_16BIT,
};

static struct resource smsc911x_resources[] = {
	[0] = {
		.start	= 0x90000000,
		.end	= 0x90007fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= MSM_GPIO_TO_INT(48),
		.end	= MSM_GPIO_TO_INT(48),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct platform_device smsc911x_device = {
	.name		= "smsc911x",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(smsc911x_resources),
	.resource	= smsc911x_resources,
	.dev		= {
		.platform_data	= &smsc911x_config,
	},
};

static struct msm_gpio smsc911x_gpios[] = {
	{ GPIO_CFG(48, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_6MA),
							 "smsc911x_irq"  },
	{ GPIO_CFG(49, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_6MA),
							 "eth_fifo_sel" },
};

#define ETH_FIFO_SEL_GPIO	49
static void msm7x27a_cfg_smsc911x(void)
{
	int res;

	res = msm_gpios_request_enable(smsc911x_gpios,
				 ARRAY_SIZE(smsc911x_gpios));
	if (res) {
		pr_err("%s: unable to enable gpios for SMSC911x\n", __func__);
		return;
	}

	/* ETH_FIFO_SEL */
	res = gpio_direction_output(ETH_FIFO_SEL_GPIO, 0);
	if (res) {
		pr_err("%s: unable to get direction for gpio %d\n", __func__,
							 ETH_FIFO_SEL_GPIO);
		msm_gpios_disable_free(smsc911x_gpios,
						 ARRAY_SIZE(smsc911x_gpios));
		return;
	}
	gpio_set_value(ETH_FIFO_SEL_GPIO, 0);
}

#ifdef CONFIG_MSM_CAMERA
static uint32_t camera_off_gpio_table[] = {
#ifdef CONFIG_MACH_VASTO
	GPIO_CFG(61, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(60, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
#else
	GPIO_CFG(61, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
	GPIO_CFG(60, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
#endif	
	GPIO_CFG(15, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

static uint32_t camera_on_gpio_table[] = {
#ifdef CONFIG_MACH_VASTO
	GPIO_CFG(61, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(60, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
#else
	GPIO_CFG(61, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
	GPIO_CFG(60, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
#endif	
	GPIO_CFG(15, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

#ifdef CONFIG_MSM_CAMERA_FLASH
static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_CURRENT_DRIVER,
	._fsrc.current_driver_src.led1 = GPIO_CAM_GP_LED_EN1,
	._fsrc.current_driver_src.led2 = GPIO_CAM_GP_LED_EN2,
};
#endif

static struct vreg *vreg_gp1;
static struct vreg *vreg_gp2;
static struct vreg *vreg_gp3;
static void msm_camera_vreg_config(int vreg_en)
{
	/* XXX: this is for msm7227a reference board camemra
	 * delete this for jena and trebon
	 */
#if 0
	int rc;

	if (vreg_gp1 == NULL) {
		vreg_gp1 = vreg_get(NULL, "msme1");
		if (IS_ERR(vreg_gp1)) {
			pr_err("%s: vreg_get(%s) failed (%ld)\n",
				__func__, "msme1", PTR_ERR(vreg_gp1));
			return;
		}

		rc = vreg_set_level(vreg_gp1, 1800);
		if (rc) {
			pr_err("%s: GP1 set_level failed (%d)\n",
				__func__, rc);
			return;
		}
	}

	if (vreg_gp2 == NULL) {
		vreg_gp2 = vreg_get(NULL, "gp2");
		if (IS_ERR(vreg_gp2)) {
			pr_err("%s: vreg_get(%s) failed (%ld)\n",
				__func__, "gp2", PTR_ERR(vreg_gp2));
			return;
		}

		rc = vreg_set_level(vreg_gp2, 2850);
		if (rc) {
			pr_err("%s: GP2 set_level failed (%d)\n",
				__func__, rc);
		}
	}

	if (vreg_gp3 == NULL) {
		vreg_gp3 = vreg_get(NULL, "usb2");
		if (IS_ERR(vreg_gp3)) {
			pr_err("%s: vreg_get(%s) failed (%ld)\n",
				__func__, "gp3", PTR_ERR(vreg_gp3));
			return;
		}

		rc = vreg_set_level(vreg_gp3, 1800);
		if (rc) {
			pr_err("%s: GP3 set level failed (%d)\n",
				__func__, rc);
		}
	}

	if (vreg_en) {
		rc = vreg_enable(vreg_gp1);
		if (rc) {
			pr_err("%s: GP1 enable failed (%d)\n",
				__func__, rc);
			return;
		}

		rc = vreg_enable(vreg_gp2);
		if (rc) {
			pr_err("%s: GP2 enable failed (%d)\n",
				__func__, rc);
		}

		rc = vreg_enable(vreg_gp3);
		if (rc) {
			pr_err("%s: GP3 enable failed (%d)\n",
				__func__, rc);
		}
	} else {
		rc = vreg_disable(vreg_gp1);
		if (rc)
			pr_err("%s: GP1 disable failed (%d)\n",
				__func__, rc);

		rc = vreg_disable(vreg_gp2);
		if (rc) {
			pr_err("%s: GP2 disable failed (%d)\n",
				__func__, rc);
		}

		rc = vreg_disable(vreg_gp3);
		if (rc) {
			pr_err("%s: GP3 disable failed (%d)\n",
				__func__, rc);
		}
	}
#endif
}

static int config_gpio_table(uint32_t *table, int len)
{
	int rc = 0, i = 0;

	for (i = 0; i < len; i++) {
		rc = gpio_tlmm_config(table[i], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s not able to get gpio\n", __func__);
			for (i--; i >= 0; i--)
				gpio_tlmm_config(camera_off_gpio_table[i],
							GPIO_CFG_ENABLE);
			break;
		}
	}
	return rc;
}

static struct msm_camera_sensor_info msm_camera_sensor_s5k4e1_data;
static struct msm_camera_sensor_info msm_camera_sensor_ov9726_data;
static int config_camera_on_gpios_rear(void)
{
	int rc = 0;

	if (machine_is_msm7x27a_ffa())
		msm_camera_vreg_config(1);

	rc = config_gpio_table(camera_on_gpio_table,
			ARRAY_SIZE(camera_on_gpio_table));
	if (rc < 0) {
		pr_err("%s: CAMSENSOR gpio table request"
		"failed\n", __func__);
		return rc;
	}

	return rc;
}

static void config_camera_off_gpios_rear(void)
{
	if (machine_is_msm7x27a_ffa())
		msm_camera_vreg_config(0);

	config_gpio_table(camera_off_gpio_table,
			ARRAY_SIZE(camera_off_gpio_table));
}

static int config_camera_on_gpios_front(void)
{
	int rc = 0;

	if (machine_is_msm7x27a_ffa())
		msm_camera_vreg_config(1);

	rc = config_gpio_table(camera_on_gpio_table,
			ARRAY_SIZE(camera_on_gpio_table));
	if (rc < 0) {
		pr_err("%s: CAMSENSOR gpio table request"
			"failed\n", __func__);
		return rc;
	}

	return rc;
}

static void config_camera_off_gpios_front(void)
{
	if (machine_is_msm7x27a_ffa())
		msm_camera_vreg_config(0);

	config_gpio_table(camera_off_gpio_table,
			ARRAY_SIZE(camera_off_gpio_table));
}

struct msm_camera_device_platform_data msm_camera_device_data_rear = {
	.camera_gpio_on  = config_camera_on_gpios_rear,
	.camera_gpio_off = config_camera_off_gpios_rear,
	.ioext.csiphy = 0xA1000000,
	.ioext.csisz  = 0x00100000,
	.ioext.csiirq = INT_CSI_IRQ_1,
	.ioclk.mclk_clk_rate = 24000000,
	.ioclk.vfe_clk_rate  = 192000000,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
};

struct msm_camera_device_platform_data msm_camera_device_data_front = {
	.camera_gpio_on  = config_camera_on_gpios_front,
	.camera_gpio_off = config_camera_off_gpios_front,
	.ioext.csiphy = 0xA0F00000,
	.ioext.csisz  = 0x00100000,
	.ioext.csiirq = INT_CSI_IRQ_0,
	.ioclk.mclk_clk_rate = 24000000,
	.ioclk.vfe_clk_rate  = 192000000,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
};

#ifdef CONFIG_S5K4E1
static struct msm_camera_sensor_platform_info s5k4e1_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_s5k4e1 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k4e1_data = {
	.sensor_name    = "s5k4e1",
	.sensor_reset_enable = 1,
	.sensor_reset   = GPIO_CAM_GP_CAMIF_RESET_N,
	.sensor_pwd             = 85,
	.vcm_pwd                = GPIO_CAM_GP_CAM_PWDN,
	.vcm_enable             = 1,
	.pdata                  = &msm_camera_device_data_rear,
	.flash_data             = &flash_s5k4e1,
	.sensor_platform_info   = &s5k4e1_sensor_7627a_info,
	.csi_if                 = 1
};

static struct platform_device msm_camera_sensor_s5k4e1 = {
	.name   = "msm_camera_s5k4e1",
	.dev    = {
		.platform_data = &msm_camera_sensor_s5k4e1_data,
	},
};
#endif

#ifdef CONFIG_IMX072
static struct msm_camera_sensor_platform_info imx072_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_imx072 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_imx072_data = {
	.sensor_name    = "imx072",
	.sensor_reset_enable = 1,
	.sensor_reset   = 85, /* TODO 106,*/
	.sensor_pwd             = 123,
	.vcm_pwd                = GPIO_CAM_GP_CAM_PWDN,
	.vcm_enable             = 1,
	.pdata                  = &msm_camera_device_data_rear,
	.flash_data             = &flash_imx072,
	.sensor_platform_info = &imx072_sensor_7627a_info,
	.csi_if                 = 1
};

static struct platform_device msm_camera_sensor_imx072 = {
	.name   = "msm_camera_imx072",
	.dev    = {
		.platform_data = &msm_camera_sensor_imx072_data,
	},
};
#endif

#ifdef CONFIG_WEBCAM_OV9726
static struct msm_camera_sensor_platform_info ov9726_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_ov9726 = {
	.flash_type             = MSM_CAMERA_FLASH_NONE,
	.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_ov9726_data = {
	.sensor_name    = "ov9726",
	.sensor_reset_enable = 0,
	.sensor_reset   = GPIO_CAM_GP_CAM1MP_XCLR,
	.sensor_pwd             = 85,
	.vcm_pwd                = 1,
	.vcm_enable             = 0,
	.pdata                  = &msm_camera_device_data_front,
	.flash_data             = &flash_ov9726,
	.sensor_platform_info   = &ov9726_sensor_7627a_info,
	.csi_if                 = 1
};

static struct platform_device msm_camera_sensor_ov9726 = {
	.name   = "msm_camera_ov9726",
	.dev    = {
		.platform_data = &msm_camera_sensor_ov9726_data,
	},
};
#endif

#ifdef CONFIG_S5K5CCAF
static struct msm_camera_sensor_platform_info s5k5ccaf_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_s5k5ccaf = {
	.flash_type             = MSM_CAMERA_FLASH_NONE,
//	.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k5ccaf_data = {
	.sensor_name    = "s5k5ccaf",
	.sensor_reset_enable = 0,
	.vcm_enable             = 0,
	.pdata                  = &msm_camera_device_data_rear,
	.flash_data             = &flash_s5k5ccaf,
	.sensor_platform_info   = &s5k5ccaf_sensor_7627a_info,
	.csi_if                 = 1 // 0: Parallel interface , 1: MIPI interface
};

static struct platform_device msm_camera_sensor_s5k5ccaf = {
	.name   = "msm_camera_s5k5ccaf",
	.dev    = {
		.platform_data = &msm_camera_sensor_s5k5ccaf_data,
	},
};
#endif

#ifdef CONFIG_MT9V113
static struct msm_camera_sensor_platform_info mt9v113_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_mt9v113 = {
	.flash_type             = MSM_CAMERA_FLASH_NONE,
//	.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9v113_data = {
	.sensor_name    = "mt9v113",
	.sensor_reset_enable = 0,
	.vcm_enable             = 0,
	.pdata                  = &msm_camera_device_data_front,
	.flash_data             = &flash_mt9v113,
	.sensor_platform_info   = &mt9v113_sensor_7627a_info,
	.csi_if                 = 1 // 0: Parallel interface , 1: MIPI interface
};

static struct platform_device msm_camera_sensor_mt9v113 = {
	.name   = "msm_camera_mt9v113",
	.dev    = {
		.platform_data = &msm_camera_sensor_mt9v113_data,
	},
};
#endif

#ifdef CONFIG_MT9E013
static struct msm_camera_sensor_platform_info mt9e013_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_mt9e013 = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9e013_data = {
	.sensor_name    = "mt9e013",
	.sensor_reset   = 0,
	.sensor_reset_enable = 1,
	.sensor_pwd     = 85,
	.vcm_pwd        = 1,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data_rear,
	.flash_data     = &flash_mt9e013,
	.sensor_platform_info   = &mt9e013_sensor_7627a_info,
	.csi_if         = 1
};

static struct platform_device msm_camera_sensor_mt9e013 = {
	.name      = "msm_camera_mt9e013",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9e013_data,
	},
};
#endif

static struct i2c_board_info i2c_camera_devices[] = {
	#ifdef CONFIG_S5K4E1
	{
		I2C_BOARD_INFO("s5k4e1", 0x36),
	},
	{
		I2C_BOARD_INFO("s5k4e1_af", 0x8c >> 1),
	},
	#endif
	#ifdef CONFIG_WEBCAM_OV9726
	{
		I2C_BOARD_INFO("ov9726", 0x10),
	},
	#endif
	#ifdef CONFIG_S5K5CCAF
	{
		I2C_BOARD_INFO("s5k5ccaf", 0x78 >> 1),
	},
	#endif
	#ifdef CONFIG_MT9V113
	{
		I2C_BOARD_INFO("mt9v113", 0x7A >> 1),
	},
	#endif
	#ifdef CONFIG_IMX072
	{
		I2C_BOARD_INFO("imx072", 0x34),
	},
	#endif
	#ifdef CONFIG_MT9E013
	{
		I2C_BOARD_INFO("mt9e013", 0x6C >> 2),
	},
	#endif
	{
		I2C_BOARD_INFO("sc628a", 0x37),
	},
};
#endif

#if defined(CONFIG_TDMB) || defined(CONFIG_TDMB_MODULE)
static void tdmb_gpio_on(void)
{
	printk("tdmb_gpio_on\n");
	
	gpio_set_value(GPIO_TDMB_EN, 1);
	msleep(10);

	gpio_set_value(GPIO_TDMB_RST, 0);
	msleep(2);
	
	gpio_set_value(GPIO_TDMB_RST, 1);
	msleep(10);
	
}

static void tdmb_gpio_off(void)
{
	printk("tdmb_gpio_off\n");

	gpio_set_value(GPIO_TDMB_RST, 0);
	msleep(1);
	gpio_set_value(GPIO_TDMB_EN, 0);

}

static struct tdmb_platform_data tdmb_pdata = {
	.gpio_on = tdmb_gpio_on,
	.gpio_off = tdmb_gpio_off,
	.irq = MSM_GPIO_TO_INT(GPIO_TDMB_INT),
};

static struct platform_device tdmb_device = {
	.name			= "tdmb",
	.id 			= -1,
	.dev			= {
		.platform_data = &tdmb_pdata,
	},
};
static int __init tdmb_dev_init(void)
{
	printk("tdmb_dev_init\n");

	gpio_tlmm_config(GPIO_CFG(GPIO_TDMB_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE); 
	gpio_tlmm_config(GPIO_CFG(GPIO_TDMB_RST, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE); 
	gpio_tlmm_config(GPIO_CFG(GPIO_TDMB_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE); 

	gpio_request(GPIO_TDMB_EN, "TDMB_EN");
	gpio_direction_output(GPIO_TDMB_EN, 0);
	gpio_request(GPIO_TDMB_RST, "TDMB_RST");
	gpio_direction_output(GPIO_TDMB_RST, 0);
	gpio_request(GPIO_TDMB_INT, "TDMB_INT");
	gpio_direction_input(GPIO_TDMB_INT);

	platform_device_register(&tdmb_device);
    return 0;
}
#endif 

#if defined(CONFIG_SERIAL_MSM_HSL_CONSOLE) \
		&& defined(CONFIG_MSM_SHARED_GPIO_FOR_UART2DM)
static struct msm_gpio uart2dm_gpios[] = {
	{GPIO_CFG(19, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
							"uart2dm_rfr_n" },
	{GPIO_CFG(20, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
							"uart2dm_cts_n" },
	{GPIO_CFG(21, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
							"uart2dm_rx"    },
	{GPIO_CFG(108, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
							"uart2dm_tx"    },
};

static void msm7x27a_cfg_uart2dm_serial(void)
{
	int ret;
	ret = msm_gpios_request_enable(uart2dm_gpios,
					ARRAY_SIZE(uart2dm_gpios));
	if (ret)
		pr_err("%s: unable to enable gpios for uart2dm\n", __func__);
}
#else
static void msm7x27a_cfg_uart2dm_serial(void) { }
#endif

static struct platform_device *rumi_sim_devices[] __initdata = {
	&msm_device_dmov,
	&msm_device_smd,
	&smc91x_device,
	&msm_device_uart1,
	&msm_device_nand,
	&msm_device_uart_dm1,
	&msm_gsbi0_qup_i2c_device,
	&msm_gsbi1_qup_i2c_device,
	&msm_wlan_pm_device,
#ifdef CONFIG_SENSORS_GP2A
	&sensor_i2c_gpio_device,
#endif
};

static struct platform_device *surf_ffa_devices[] __initdata = {
	&msm_device_dmov,
	&msm_device_smd,
	&msm_device_uart1,
	&msm_device_uart_dm1,
	//&msm_device_uart_dm2,
	&msm_device_nand,
	&msm_gsbi0_qup_i2c_device,
	&msm_gsbi1_qup_i2c_device,
	&msm_device_otg,
	&msm_device_gadget_peripheral,
	&android_usb_device,
	&android_pmem_device,
	&android_pmem_adsp_device,
	&usb_mass_storage_device,
	&rndis_device,
	&usb_diag_device,
	&usb_gadget_fserial_device,
	&android_pmem_audio_device,
	&msm_device_snd,
	&msm_device_adspdec,
	&msm_fb_device,
#ifdef CONFIG_SENSORS_GP2A
	&sensor_i2c_gpio_device,
#endif
	//&lcdc_toshiba_panel_device,
#if defined(CONFIG_FB_MSM_LCDC_S6E63M0_WVGA)
	&lcdc_s6e63m0_panel_device,
#endif
#if defined(CONFIG_FB_MSM_LCDC_S6D16A0X_HVGA)
	&lcdc_s6d16a0x_panel_device,
#endif
#ifdef CONFIG_BATTERY_MSM
	&msm_batt_device,
#endif	
#ifdef CONFIG_BATTERY_SEC
	&sec_device_battery,
#endif	
	//&smsc911x_device,
#ifdef CONFIG_S5K4E1
	&msm_camera_sensor_s5k4e1,
#endif
#ifdef CONFIG_IMX072
	&msm_camera_sensor_imx072,
#endif
#ifdef CONFIG_WEBCAM_OV9726
	&msm_camera_sensor_ov9726,
#endif
#ifdef CONFIG_S5K5CCAF
	&msm_camera_sensor_s5k5ccaf,
#endif
#ifdef CONFIG_MT9V113
	&msm_camera_sensor_mt9v113,
#endif
#ifdef CONFIG_MT9E013
	&msm_camera_sensor_mt9e013,
#endif
#ifdef CONFIG_FB_MSM_MIPI_DSI
	&mipi_dsi_renesas_panel_device,
#endif
	&msm_kgsl_3d0,
#ifdef CONFIG_BT
	&msm_bt_power_device,
	&msm_bluesleep_device,	
#endif
	&msm_wlan_pm_device,
	&touch_i2c_gpio_device,
#if defined (CONFIG_KEYBOARD_NEXTCHIP_TOUCH) || defined (CONFIG_KEYPAD_MELFAS_TOUCH)
    &tkey_ic2_gpio_device,
#endif
	&fsa880_i2c_gpio_device,
#ifdef CONFIG_CHARGER_SMB328A
	&smb328a_i2c_gpio_device,
#endif	
	&msm_vibrator_device,
#ifdef CONFIG_SAMSUNG_JACK
	&sec_device_jack,
#endif

#ifdef CONFIG_MACH_VASTO
	&sec_device_switch,  // samsung switch driver
#endif
};

static unsigned pmem_kernel_ebi1_size = PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char *p)
{
	pmem_kernel_ebi1_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi1_size", pmem_kernel_ebi1_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;
static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);

static void __init msm_msm7x2x_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	size = fb_size ? : MSM_FB_SIZE;
	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
		size, addr, __pa(addr));
}

static struct memtype_reserve msm7x27a_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
	android_pmem_adsp_pdata.size = pmem_adsp_size;
	android_pmem_pdata.size = pmem_mdp_size;
	android_pmem_audio_pdata.size = pmem_audio_size;
#endif
}

static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	msm7x27a_reserve_table[p->memory_type].size += p->size;
}

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_pdata);
	reserve_memory_for(&android_pmem_audio_pdata);
	msm7x27a_reserve_table[MEMTYPE_EBI1].size += pmem_kernel_ebi1_size;
#endif
}

static void __init msm7x27a_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
}

static int msm7x27a_paddr_to_memtype(unsigned int paddr)
{
	return MEMTYPE_EBI1;
}

static struct reserve_info msm7x27a_reserve_info __initdata = {
	.memtype_reserve_table = msm7x27a_reserve_table,
	.calculate_reserve_sizes = msm7x27a_calculate_reserve_sizes,
	.paddr_to_memtype = msm7x27a_paddr_to_memtype,
};

static void __init msm7x27a_reserve(void)
{
	reserve_info = &msm7x27a_reserve_info;
	msm_reserve();
}

static void __init msm_device_i2c_init(void)
{
	msm_gsbi0_qup_i2c_device.dev.platform_data = &msm_gsbi0_qup_i2c_pdata;
	msm_gsbi1_qup_i2c_device.dev.platform_data = &msm_gsbi1_qup_i2c_pdata;

	gpio_tlmm_config(GPIO_CFG(GPIO_TSP_SDA, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(GPIO_TSP_SCL, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(GPIO_MUS_SDA, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(GPIO_MUS_SCL, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);

	gpio_tlmm_config(GPIO_CFG(GPIO_KEY_SDA, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(GPIO_KEY_SCL, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);    
#ifdef CONFIG_SENSORS_GP2A	
	gpio_tlmm_config(GPIO_CFG(37, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_8MA),GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(38, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_8MA),GPIO_CFG_ENABLE);	
#endif

	printk("[TSP] %s =======gpio_request==test======ln=%d\n",__func__, __LINE__);
}

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,
	.mdp_rev = MDP_REV_303,
};

#define GPIO_LCDC_BRDG_PD	128
#define GPIO_LCDC_BRDG_RESET_N	129

#define LCDC_RESET_PHYS		0x90008014
static	void __iomem *lcdc_reset_ptr;

static unsigned mipi_dsi_gpio[] = {
	GPIO_CFG(GPIO_LCDC_BRDG_RESET_N, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
		GPIO_CFG_2MA),       /* LCDC_BRDG_RESET_N */
	GPIO_CFG(GPIO_LCDC_BRDG_PD, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
		GPIO_CFG_2MA),       /* LCDC_BRDG_RESET_N */
};

enum {
       DSI_SINGLE_LANE = 1,
       DSI_TWO_LANES,
};

static int msm_fb_get_lane_config(void)
{
       int rc = DSI_TWO_LANES;

	if (cpu_is_msm7x25a() || cpu_is_msm7x25aa()) {
               rc = DSI_SINGLE_LANE;
               pr_info("DSI Single Lane\n");
       } else {
               pr_info("DSI Two Lanes\n");
       }
       return rc;
}

static int msm_fb_dsi_client_reset(void)
{
	int rc = 0;

	rc = gpio_request(GPIO_LCDC_BRDG_RESET_N, "lcdc_brdg_reset_n");
	if (rc < 0) {
		pr_err("failed to request lcd brdg reset_n\n");
		return rc;
	}

	rc = gpio_request(GPIO_LCDC_BRDG_PD, "lcdc_brdg_pd");
	if (rc < 0) {
		pr_err("failed to request lcd brdg pd\n");
		return rc;
	}

	rc = gpio_tlmm_config(mipi_dsi_gpio[0], GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("Failed to enable LCDC Bridge reset enable\n");
		goto gpio_error;
	}

	rc = gpio_tlmm_config(mipi_dsi_gpio[1], GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("Failed to enable LCDC Bridge pd enable\n");
		goto gpio_error2;
	}

	rc = gpio_direction_output(GPIO_LCDC_BRDG_RESET_N, 1);
	rc |= gpio_direction_output(GPIO_LCDC_BRDG_PD, 1);
	gpio_set_value_cansleep(GPIO_LCDC_BRDG_PD, 0);

	if (!rc) {
		if (machine_is_msm7x27a_surf()) {
			lcdc_reset_ptr = ioremap_nocache(LCDC_RESET_PHYS,
				sizeof(uint32_t));

			if (!lcdc_reset_ptr)
				return 0;
		}
		return rc;
	} else {
		goto gpio_error;
	}

gpio_error2:
	pr_err("Failed GPIO bridge pd\n");
	gpio_free(GPIO_LCDC_BRDG_PD);

gpio_error:
	pr_err("Failed GPIO bridge reset\n");
	gpio_free(GPIO_LCDC_BRDG_RESET_N);
	return rc;
}

static const char * const msm_fb_dsi_vreg[] = {
	"gp2",
	"msme1",
	"mddi"
};

static const int msm_fb_dsi_vreg_mV[] = {
	2850,
	1800,
	1200
};

static struct vreg *dsi_vreg[ARRAY_SIZE(msm_fb_dsi_vreg)];
static int dsi_gpio_initialized;

static int mipi_dsi_panel_power(int on)
{
	int i, rc = 0;
	uint32_t lcdc_reset_cfg;

	/* I2C-controlled GPIO Expander -init of the GPIOs very late */
	if (!dsi_gpio_initialized) {
		pmapp_disp_backlight_init();

		rc = gpio_request(GPIO_DISPLAY_PWR_EN, "gpio_disp_pwr");
		if (rc < 0) {
			pr_err("failed to request gpio_disp_pwr\n");
			return rc;
		}

		rc = gpio_direction_output(GPIO_DISPLAY_PWR_EN, 1);
		if (rc < 0) {
			pr_err("failed to enable display pwr\n");
			goto fail_gpio1;
		}

		if (machine_is_msm7x27a_surf()) {
			rc = gpio_request(GPIO_BACKLIGHT_EN, "gpio_bkl_en");
			if (rc < 0) {
				pr_err("failed to request gpio_bkl_en\n");
				goto fail_gpio1;
			}

			rc = gpio_direction_output(GPIO_BACKLIGHT_EN, 1);
			if (rc < 0) {
				pr_err("failed to enable backlight\n");
				goto fail_gpio2;
			}
		}

		for (i = 0; i < ARRAY_SIZE(msm_fb_dsi_vreg); i++) {
			dsi_vreg[i] = vreg_get(0, msm_fb_dsi_vreg[i]);

			if (IS_ERR(dsi_vreg[i])) {
				pr_err("%s: vreg get failed with : (%ld)\n",
					__func__, PTR_ERR(dsi_vreg[i]));
				goto fail_gpio2;
			}

			rc = vreg_set_level(dsi_vreg[i],
				msm_fb_dsi_vreg_mV[i]);

			if (rc < 0) {
				pr_err("%s: set regulator level failed "
					"with :(%d)\n",	__func__, rc);
				goto vreg_fail1;
			}
		}
		dsi_gpio_initialized = 1;
	}

		gpio_set_value_cansleep(GPIO_DISPLAY_PWR_EN, on);
		if (machine_is_msm7x27a_surf()) {
			gpio_set_value_cansleep(GPIO_BACKLIGHT_EN, on);
		}

		if (on) {
			gpio_set_value_cansleep(GPIO_LCDC_BRDG_PD, 0);

			if (machine_is_msm7x27a_surf()) {
				lcdc_reset_cfg = readl_relaxed(lcdc_reset_ptr);
				rmb();
				lcdc_reset_cfg &= ~1;

				writel_relaxed(lcdc_reset_cfg, lcdc_reset_ptr);
				msleep(20);
				wmb();
				lcdc_reset_cfg |= 1;
				writel_relaxed(lcdc_reset_cfg, lcdc_reset_ptr);
			} else {
				gpio_set_value_cansleep(GPIO_LCDC_BRDG_RESET_N,
					0);
				msleep(20);
				gpio_set_value_cansleep(GPIO_LCDC_BRDG_RESET_N,
					1);
			}

			if (pmapp_disp_backlight_set_brightness(100))
				pr_err("backlight set brightness failed\n");
		} else {
			gpio_set_value_cansleep(GPIO_LCDC_BRDG_PD, 1);

			if (pmapp_disp_backlight_set_brightness(0))
				pr_err("backlight set brightness failed\n");
		}

		/*Configure vreg lines */
		for (i = 0; i < ARRAY_SIZE(msm_fb_dsi_vreg); i++) {
			if (on) {
				rc = vreg_enable(dsi_vreg[i]);

				if (rc) {
					printk(KERN_ERR "vreg_enable: %s vreg"
						"operation failed\n",
						msm_fb_dsi_vreg[i]);

					goto vreg_fail2;
				}
			} else {
				rc = vreg_disable(dsi_vreg[i]);

				if (rc) {
					printk(KERN_ERR "vreg_disable: %s vreg "
						"operation failed\n",
						msm_fb_dsi_vreg[i]);
					goto vreg_fail2;
				}
			}
		}

	return rc;

vreg_fail2:
	if (on) {
		for (; i > 0; i--)
			vreg_disable(dsi_vreg[i - 1]);
	} else {
		for (; i > 0; i--)
			vreg_enable(dsi_vreg[i - 1]);
	}

	return rc;

vreg_fail1:
	for (; i > 0; i--)
		vreg_put(dsi_vreg[i - 1]);

fail_gpio2:
	gpio_free(GPIO_BACKLIGHT_EN);
fail_gpio1:
	gpio_free(GPIO_DISPLAY_PWR_EN);
	dsi_gpio_initialized = 0;
	return rc;
}

#define MDP_303_VSYNC_GPIO 97

#ifdef CONFIG_FB_MSM_MDP303
static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.vsync_gpio = MDP_303_VSYNC_GPIO,
	.dsi_power_save   = mipi_dsi_panel_power,
	.dsi_client_reset = msm_fb_dsi_client_reset,
	.get_lane_config = msm_fb_get_lane_config,
};
#endif

static void __init msm_fb_add_devices(void)
{
	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("lcdc", &lcdc_pdata);
#ifdef CONFIG_FB_MSM_MDP303
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
#endif
}

#define MSM_EBI2_PHYS			0xa0d00000
#define MSM_EBI2_XMEM_CS2_CFG1		0xa0d10030

static void __init msm7x27a_init_ebi2(void)
{
	uint32_t ebi2_cfg;
	void __iomem *ebi2_cfg_ptr;

	ebi2_cfg_ptr = ioremap_nocache(MSM_EBI2_PHYS, sizeof(uint32_t));
	if (!ebi2_cfg_ptr)
		return;

	ebi2_cfg = readl(ebi2_cfg_ptr);
	if (machine_is_msm7x27a_rumi3() || machine_is_msm7x27a_surf())
		ebi2_cfg |= (1 << 4); /* CS2 */

	writel(ebi2_cfg, ebi2_cfg_ptr);
	iounmap(ebi2_cfg_ptr);

	/* Enable A/D MUX[bit 31] from EBI2_XMEM_CS2_CFG1 */
	ebi2_cfg_ptr = ioremap_nocache(MSM_EBI2_XMEM_CS2_CFG1,
							 sizeof(uint32_t));
	if (!ebi2_cfg_ptr)
		return;

	ebi2_cfg = readl(ebi2_cfg_ptr);
	if (machine_is_msm7x27a_surf())
		ebi2_cfg |= (1 << 31);

	writel(ebi2_cfg, ebi2_cfg_ptr);
	iounmap(ebi2_cfg_ptr);
}

#define ATMEL_TS_I2C_NAME "maXTouch"
static struct vreg *vreg_l12;
static struct vreg *vreg_s3;

#define ATMEL_TS_GPIO_IRQ 82

static int atmel_ts_power_on(bool on)
{
	int rc;

	rc = on ? vreg_enable(vreg_l12) : vreg_disable(vreg_l12);
	if (rc) {
		pr_err("%s: vreg %sable failed (%d)\n",
		       __func__, on ? "en" : "dis", rc);
		return rc;
	}

	rc = on ? vreg_enable(vreg_s3) : vreg_disable(vreg_s3);
	if (rc) {
		pr_err("%s: vreg %sable failed (%d) for S3\n",
		       __func__, on ? "en" : "dis", rc);
		!on ? vreg_enable(vreg_l12) : vreg_disable(vreg_l12);
		return rc;
	}
	/* vreg stabilization delay */
	msleep(50);
	return 0;
}

static int atmel_ts_platform_init(struct i2c_client *client)
{
	int rc;

	vreg_l12 = vreg_get(NULL, "gp2");
	if (IS_ERR(vreg_l12)) {
		pr_err("%s: vreg_get for L2 failed\n", __func__);
		return PTR_ERR(vreg_l12);
	}

	rc = vreg_set_level(vreg_l12, 2850);
	if (rc) {
		pr_err("%s: vreg set level failed (%d) for l2\n",
		       __func__, rc);
		goto vreg_put_l2;
	}

	vreg_s3 = vreg_get(NULL, "vreg_msme");
	if (IS_ERR(vreg_s3)) {
		pr_err("%s: vreg_get for S3 failed\n", __func__);
		rc = PTR_ERR(vreg_s3);
		goto vreg_put_l2;
	}

	rc = vreg_set_level(vreg_s3, 1800);
	if (rc) {
		pr_err("%s: vreg set level failed (%d) for S3\n",
		       __func__, rc);
		goto vreg_put_s3;
	}

	rc = gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("%s: gpio_tlmm_config for %d failed\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto vreg_put_s3;
	}

	/* configure touchscreen interrupt gpio */
	rc = gpio_request(ATMEL_TS_GPIO_IRQ, "atmel_maxtouch_gpio");
	if (rc) {
		pr_err("%s: unable to request gpio %d\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto ts_gpio_tlmm_unconfig;
	}

	rc = gpio_direction_input(ATMEL_TS_GPIO_IRQ);
	if (rc < 0) {
		pr_err("%s: unable to set the direction of gpio %d\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto free_ts_gpio;
	}
	return 0;

free_ts_gpio:
	gpio_free(ATMEL_TS_GPIO_IRQ);
ts_gpio_tlmm_unconfig:
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
vreg_put_s3:
	vreg_put(vreg_s3);
vreg_put_l2:
	vreg_put(vreg_l12);
	return rc;
}

static int atmel_ts_platform_exit(struct i2c_client *client)
{
	gpio_free(ATMEL_TS_GPIO_IRQ);
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	vreg_disable(vreg_s3);
	vreg_put(vreg_s3);
	vreg_disable(vreg_l12);
	vreg_put(vreg_l12);
	return 0;
}

static u8 atmel_ts_read_chg(void)
{
	return gpio_get_value(ATMEL_TS_GPIO_IRQ);
}

static u8 atmel_ts_valid_interrupt(void)
{
	return !atmel_ts_read_chg();
}

#define ATMEL_X_OFFSET 13
#define ATMEL_Y_OFFSET 0

static struct mxt_platform_data atmel_ts_pdata = {
	.numtouch = 4,
	.init_platform_hw = atmel_ts_platform_init,
	.exit_platform_hw = atmel_ts_platform_exit,
	.power_on = atmel_ts_power_on,
	.display_res_x = 480,
	.display_res_y = 864,
	.min_x = ATMEL_X_OFFSET,
	.max_x = (505 - ATMEL_X_OFFSET),
	.min_y = ATMEL_Y_OFFSET,
	.max_y = (863 - ATMEL_Y_OFFSET),
	.valid_interrupt = atmel_ts_valid_interrupt,
	.read_chg = atmel_ts_read_chg,
};

static struct i2c_board_info atmel_ts_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO(ATMEL_TS_I2C_NAME, 0x4a),
		.platform_data = &atmel_ts_pdata,
		.irq = MSM_GPIO_TO_INT(ATMEL_TS_GPIO_IRQ),
	},
};


#if 0
void keypad_gpio_init(void)
{
//	gpio_configure(39, GPIOF_INPUT);
	gpio_tlmm_config(GPIO_CFG(39, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	if(gpio_request(39, "volume up")<0)
		pr_err("sec_jack:gpio_request fail\n");
	if(gpio_direction_input(39)<0)
		pr_err("sec_jack:gpio_direction fail\n");
}
#endif

#ifdef CONFIG_MACH_VASTO
#define KP_INDEX(row, col) ((row)*ARRAY_SIZE(kp_col_gpios) + (col))
//if (board_hw_revision==0){
	static unsigned int kp_row_gpios[] = {31, 32}; // kbc(0) ~ kbc(1), output
	static unsigned int kp_col_gpios[] = {39, 36 };  // kbr(0) ~ kbr(1), input

	static const unsigned short keymap[ARRAY_SIZE(kp_col_gpios) *
					  ARRAY_SIZE(kp_row_gpios)] = {
 		[KP_INDEX(0, 0)] = KEY_VOLUMEUP,
		
		[KP_INDEX(0, 1)] = KEY_HOME,

		[KP_INDEX(1, 0)] = KEY_VOLUMEDOWN,
	};

static struct gpio_event_matrix_info kp_matrix_info = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= keymap,
	.output_gpios	= kp_row_gpios,
	.input_gpios	= kp_col_gpios,
	.noutputs	= ARRAY_SIZE(kp_row_gpios),
	.ninputs	= ARRAY_SIZE(kp_col_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_delay.tv.nsec = 20 * NSEC_PER_MSEC,	
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_DEBOUNCE,
};

static struct gpio_event_info *kp_info[] = {
	&kp_matrix_info.info
};

static struct gpio_event_platform_data kp_pdata = {
	.name		= "sec_keypad",
	.info		= kp_info,
	.info_count	= ARRAY_SIZE(kp_info)
};

static struct platform_device kp_pdev = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &kp_pdata,
	},
};  
#else
#define KP_INDEX(row, col) ((row)*ARRAY_SIZE(kp_col_gpios) + (col))
//if (board_hw_revision==0){
	static unsigned int kp_row_gpios[] = {39, 36, 37, 38};
	static unsigned int kp_col_gpios[] = {31 };

	static const unsigned short keymap[ARRAY_SIZE(kp_col_gpios) *
					  ARRAY_SIZE(kp_row_gpios)] = {
		[KP_INDEX(0, 0)] = KEY_VOLUMEUP,
		
		[KP_INDEX(1, 0)] = KEY_VOLUMEDOWN,

		[KP_INDEX(2, 0)] = KEY_HOME,

		[KP_INDEX(3, 0)] = KEY_VOLUMEUP,
	};

/*
}else{
	static unsigned int kp_row_gpios[] = {36, 37, 39};
	static unsigned int kp_col_gpios[] = {31};

	static const unsigned short keymap[ARRAY_SIZE(kp_col_gpios) *
					  ARRAY_SIZE(kp_row_gpios)] = {
		[KP_INDEX(0, 0)] = KEY_VOLUMEDOWN,

		[KP_INDEX(1, 0)] = KEY_HOME,

		[KP_INDEX(2, 0)] = KEY_VOLUMEUP,

		[KP_INDEX(3, 0)] = KEY_VOLUMEUP,
	};

}
*/

/* SURF keypad platform device information */
static struct gpio_event_matrix_info kp_matrix_info = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= keymap,
	.output_gpios	= kp_row_gpios,
	.input_gpios	= kp_col_gpios,
	.noutputs	= ARRAY_SIZE(kp_row_gpios),
	.ninputs	= ARRAY_SIZE(kp_col_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS,
};

static struct gpio_event_info *kp_info[] = {
	&kp_matrix_info.info
};

static struct gpio_event_platform_data kp_pdata = {
	.name		= "7x27a_kp",
	.info		= kp_info,
	.info_count	= ARRAY_SIZE(kp_info)
};

static struct platform_device kp_pdev = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &kp_pdata,
	},
};
  
#endif  


static struct msm_handset_platform_data hs_platform_data = {
#if defined(CONFIG_MACH_VASTO)
	.hs_name = "sec7k_handset",
#else
	.hs_name = "sec_jack",
#endif	
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_pdev = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

#define LED_GPIO_PDM		96
#define UART1DM_RX_GPIO		45
static void __init msm7x2x_init(void)
{
	/* FIXME: disable clock id 86 initialy.
	 * this clock block CP sleep.
	 * this problem looks like Qualcomm issue.
	 * Use follow code temporary until solve that.
	 */
#if 1
	int id = 86;

	msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);
#endif

	/* pm8029 regulator ldo table initialize */
	msm7x27a_vreg_init(board_hw_revision);

#if 0
	if (!kernel_uart_flag)
		msm_clock_init(msm_clocks_7x27a_uart, msm_num_clocks_7x27a_uart);
	else
	msm_clock_init(msm_clocks_7x27a, msm_num_clocks_7x27a);

	msm_acpu_clock_init(&msm7x2x_clock_data);
#endif

	/* Common functions for SURF/FFA/RUMI3 */
	msm_device_i2c_init();
	msm7x27a_init_mmc();
	msm7x27a_init_ebi2();
	//msm7x27a_cfg_uart2dm_serial();
#ifdef CONFIG_SERIAL_MSM_HS
	msm_uart_dm1_pdata.wakeup_irq = gpio_to_irq(UART1DM_RX_GPIO);
	msm_device_uart_dm1.dev.platform_data = &msm_uart_dm1_pdata;
#endif

	if (machine_is_msm7x27a_rumi3()) {
		platform_add_devices(rumi_sim_devices,
				ARRAY_SIZE(rumi_sim_devices));
	}
	if (machine_is_msm7x27a_surf() || machine_is_msm7x27a_ffa()) {
#ifdef CONFIG_USB_MSM_OTG_72K
		msm_otg_pdata.swfi_latency =
			msm7x27a_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
		msm_device_otg.dev.platform_data = &msm_otg_pdata;
#endif
		//msm7x27a_cfg_smsc911x();
#ifdef CONFIG_SAMSUNG_JACK
		sec_jack_gpio_init();
#endif
		platform_add_devices(msm_footswitch_devices,
			     msm_num_footswitch_devices);
		platform_add_devices(surf_ffa_devices,
				ARRAY_SIZE(surf_ffa_devices));

		if (!kernel_uart_flag)
		{
			platform_device_register(&msm_device_uart3);
		}
	
		lcdc_gpio_init();
		msm_fb_add_devices();
#ifdef CONFIG_USB_EHCI_MSM_72K
//		msm7x2x_init_host();
#endif
	}

	msm_pm_set_platform_data(msm7x27a_pm_data,
				ARRAY_SIZE(msm7x27a_pm_data));

#if defined(CONFIG_I2C)// && defined(CONFIG_GPIO_SX150X)
	register_i2c_devices();
#endif
#if defined(CONFIG_BT) //&& defined(CONFIG_MARIMBA_CORE)
	bt_power_init();
#endif

// HASH0521
	i2c_register_board_info( 2, touch_i2c_devices, ARRAY_SIZE(touch_i2c_devices));
#ifdef CONFIG_SENSORS_GP2A
	i2c_register_board_info( 5, proximity_i2c_device, ARRAY_SIZE(proximity_i2c_device));
#endif

//   printk(KERN_ERR "board_hw_revision : %d \n", board_hw_revision);

#ifdef CONFIG_MACH_VASTO
	if(board_hw_revision >= 2)
		i2c_register_board_info(3, fsa9280_i2c_devices,
				ARRAY_SIZE(fsa9280_i2c_devices));    
	else
		i2c_register_board_info(3, fsa880_i2c_devices,
				ARRAY_SIZE(fsa880_i2c_devices));
  
#else
	i2c_register_board_info(3, fsa880_i2c_devices,
			ARRAY_SIZE(fsa880_i2c_devices));
#endif

#if defined(CONFIG_CHARGER_SMB328A)
	i2c_register_board_info(4, smb328a_i2c_devices,
			ARRAY_SIZE(smb328a_i2c_devices));
#endif

/*
	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
		atmel_ts_i2c_info,
		ARRAY_SIZE(atmel_ts_i2c_info));
*/
	i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
			i2c_camera_devices,
			ARRAY_SIZE(i2c_camera_devices));
#if defined (CONFIG_KEYBOARD_NEXTCHIP_TOUCH) || defined (CONFIG_KEYPAD_MELFAS_TOUCH)
	/* Touch Key */
	tkey_gpio_init();
//	i2c_register_board_info(6, tkey_i2c_devices, ARRAY_SIZE(tkey_i2c_devices));

    if (board_hw_revision>= 2) // MELFAS
    	i2c_register_board_info(6, melfas_tkey_i2c_devices, ARRAY_SIZE(melfas_tkey_i2c_devices));
    else // NEXTCHIP
    	i2c_register_board_info(6, nextchip_tkey_i2c_devices, ARRAY_SIZE(nextchip_tkey_i2c_devices));    
#endif    

//	keypad_gpio_init();
	platform_device_register(&kp_pdev);
	platform_device_register(&hs_pdev);

#if !defined(CONFIG_MACH_VASTO)
	/* configure it as a pdm function*/
	if (gpio_tlmm_config(GPIO_CFG(LED_GPIO_PDM, 3,
				GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE))
		pr_err("%s: gpio_tlmm_config for %d failed\n",
			__func__, LED_GPIO_PDM);
	else
#endif    
		platform_device_register(&led_pdev);

#ifdef CONFIG_MSM_RPC_VIBRATOR
	if (machine_is_msm7x27a_ffa())
		msm_init_pmic_vibrator();
#endif
       /*7x25a kgsl initializations*/
       msm7x25a_kgsl_3d0_init();

#ifdef CONFIG_MACH_VASTO
	//2011.01.21 static wifi skb 
	init_wifi_mem();
#endif
#if defined(CONFIG_TDMB) || defined(CONFIG_TDMB_MODULE)
    tdmb_dev_init();
#endif

#ifdef CONFIG_MACH_VASTO
    msm7x27a_switch_init();
#endif
}

static void __init msm7x2x_init_early(void)
{
	msm7x2x_misc_init();
	msm_msm7x2x_allocate_memory_regions();
#ifdef CONFIG_MACH_VASTO  //DHD_USE_STATIC_BUF 
       s5p_reserve_bootmem(crespo_media_devs, ARRAY_SIZE(crespo_media_devs));
#endif
}

MACHINE_START(MSM7X27A_RUMI3, "QCT MSM7x27a RUMI3")
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= msm_common_io_init,
	.reserve	= msm7x27a_reserve,
	.init_irq	= msm_init_irq,
	.init_machine	= msm7x2x_init,
	.timer		= &msm_timer,
	.init_early     = msm7x2x_init_early,
MACHINE_END
MACHINE_START(MSM7X27A_SURF, "QCT MSM7x27a SURF")
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= msm_common_io_init,
	.reserve	= msm7x27a_reserve,
	.init_irq	= msm_init_irq,
	.init_machine	= msm7x2x_init,
	.timer		= &msm_timer,
	.init_early     = msm7x2x_init_early,
MACHINE_END
MACHINE_START(MSM7X27A_FFA, "QCT MSM7x27a FFA")
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= msm_common_io_init,
	.reserve	= msm7x27a_reserve,
	.init_irq	= msm_init_irq,
	.init_machine	= msm7x2x_init,
	.timer		= &msm_timer,
	.init_early     = msm7x2x_init_early,
MACHINE_END

