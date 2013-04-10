/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/pmic.h>

#include <mach/camera.h>
#include <mach/vreg.h>
#include <linux/io.h> 
#include <linux/wakelock.h> //for faceunlock fucntion

//#include "msm_camera_gpio.h"

#ifdef CONFIG_MACH_VASTO_SKT
#include "mt9v113_skt.h" // for skt
#else
#include "mt9v113.h" // for kt
#endif

#define SENSOR_DEBUG 0


#define CAM_MEGA_STBY	16
#define CAM_MEGA_RST	49
#define CAM_VGA_STBY	23
#define CAM_VGA_RST		123
#define CAM_GPIO_SCL	60
#define CAM_GPIO_SDA	61
#define PMIC_CAM_EN			0 // GPIO1
#define PMIC_CAM_MEGA_EN	5 // GPIO6
#define PMIC_CAM_VGA_EN		7 // GPIO8


//#define CONFIG_LOAD_FILE

static unsigned int config_csi1;

//Gopeace LeeSangmin DJ26 add
static unsigned short mt9v113_sensor_version = 0xff;

struct mt9v113_work {
	struct work_struct work;
};

static struct  mt9v113_work *mt9v113_sensorw;
static struct  i2c_client *mt9v113_client;

struct mt9v113_ctrl 
{
	const struct msm_camera_sensor_info *sensordata;

};

//Gopeace LeeSangmin Add
struct mt9v113_data_save
{
	char mBrightness;
	char mDTP;
	char mInit;
	char mPrettyEffect;
	char mVtMode;
	char mFPS;
};

static struct mt9v113_ctrl *mt9v113_ctrl;
static struct mt9v113_data_save mt9v113_data;

static DECLARE_WAIT_QUEUE_HEAD(mt9v113_wait_queue);
//DECLARE_MUTEX(mt9v113_sem);
static void mt9v113_restart(void);

/*=============================================================
	EXTERNAL DECLARATIONS
==============================================================*/
extern int cpufreq_direct_set_policy(unsigned int cpu, const char *buf);
/*=============================================================*/

#ifdef CONFIG_LOAD_FILE
static int mt9v113_regs_table_write(char *name);
#endif


/**
 * mt9v113_i2c_read: Read (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @cmd: command register
 * @data: data to be read
 *
 * Returns 0 on success, <0 on error
 */

static int mt9v113_sensor_read(unsigned short subaddr, unsigned short *data)
{
	int ret;
	unsigned char buf[2];
	struct i2c_msg msg = { mt9v113_client->addr, 0, 2, buf };
	
	buf[0] = (subaddr >> 8);
	buf[1] = (subaddr & 0xFF);

	ret = i2c_transfer(mt9v113_client->adapter, &msg, 1) == 1 ? 0 : -EIO;
	if (ret == -EIO) 
		goto error;

	msg.flags = I2C_M_RD;
	
	ret = i2c_transfer(mt9v113_client->adapter, &msg, 1) == 1 ? 0 : -EIO;
	if (ret == -EIO) 
		goto error;

	*data = ((buf[0] << 8) | buf[1]);

error:
	return ret;
}

static int mt9v113_sensor_write( unsigned short subaddr, unsigned short val)
{
	unsigned char buf[4];
	struct i2c_msg msg = { mt9v113_client->addr, 0, 4, buf };

	//printk("[PGH] on write func mt9v113_client->addr : %x\n", mt9v113_client->addr);
	//printk("[PGH] on write func mt9v113_client->adapter->nr : %d\n", mt9v113_client->adapter->nr);
    
	//CAMDRV_DEBUG("[PGH] on write func subaddr:%x, val:%x\n", subaddr, val);
	
	buf[0] = (subaddr >> 8);
	buf[1] = (subaddr & 0xFF);
	buf[2] = (val >> 8);
	buf[3] = (val & 0xFF);

	return i2c_transfer(mt9v113_client->adapter, &msg, 1) == 1 ? 0 : -EIO;
}

static int mt9v113_sensor_polling(void)
{
	int i=0;
	unsigned short data;
	CAMDRV_DEBUG("mt9v113_sensor_polling start");
	
	for( i=0; i<10 ; i++ ){
		mdelay(30);
		mt9v113_sensor_write(0x098C,0xA103);
		mt9v113_sensor_read(0x0990,&data);
		if( data==0 ) break;
	}
	
	CAMDRV_DEBUG("mt9v113_sensor_polling end");
}

static int mt9v113_sensor_write_list( const unsigned long *list,int size, char *name)
{
	int ret, i;
	unsigned short subaddr;
	unsigned short value;

	ret = 0 ;
#ifdef CONFIG_LOAD_FILE	
	ret = mt9v113_regs_table_write(name);
#else

	printk("mt9v113_sensor_write_list : %s\n",name);
	  
	for (i = 0; i < size; i++)
	{
		//CAMDRV_DEBUG("[PGH] %x      %x\n", list[i].subaddr, list[i].value);
		subaddr = ((list[i])>> 16); //address
		value = ((list[i])& 0xFFFF); //value
		if(subaddr == 0xffff)
		{
			if( value == 0xffff )
			{
				mt9v113_sensor_polling();
			}
			else
			{
				CAMDRV_DEBUG("SETFILE DELAY : %dms",value);
				msleep(value);
			}
		}
		else
		{
		    if(mt9v113_sensor_write(subaddr, value) < 0)
		    {
			    printk("[MT9V113]sensor_write_list fail...-_-\n");
			    return -1;
		    }
		}
	}
#endif
	return ret;
}



void mt9v113_effect_control( char value)
{
	switch( value)
	{
		case EXT_CFG_EFFECT_NORMAL :
		{
			CAMDRV_DEBUG( "EXT_CFG_EFFECT_NORMAL");
			mt9v113_sensor_write_list( mt9v113_Effect_Normal, MT9V113_EFFECT_NORMAL_REGS,\
			 "mt9v113_Effect_Normal"); 
		}
		break;		

		case EXT_CFG_EFFECT_NEGATIVE :
		{
			CAMDRV_DEBUG( "EXT_CFG_EFFECT_NEGATIVE");
			mt9v113_sensor_write_list( mt9v113_Effect_Negative, MT9V113_EFFECT_NEGATIVE_REGS,\
			 "mt9v113_Effect_Negative"); 
		}
		break;	
		
		case EXT_CFG_EFFECT_MONO :
		{
			CAMDRV_DEBUG("EXT_CFG_EFFECT_MONO");
			mt9v113_sensor_write_list( mt9v113_Effect_Black_White, MT9V113_EFFECT_BLACK_WHITE_REGS,\
			 "mt9v113_Effect_Black_White"); 
		}
		break;	

		case EXT_CFG_EFFECT_SEPIA :
		{
			CAMDRV_DEBUG("EXT_CFG_EFFECT_SEPIA");
			mt9v113_sensor_write_list( mt9v113_Effect_Sepia, MT9V113_EFFECT_SEPIA_REGS,\
			 "mt9v113_Effect_Sepia"); 
		}
		break;	
/*
		case EXT_CFG_EFFECT_GREEN :
		{
			CAMDRV_DEBUG("EXT_CFG_EFFECT_GREEN");
			mt9v113_sensor_write_list( mt9v113_Effect_Green, MT9V113_EFFECT_GREEN_REGS,\
			 "mt9v113_Effect_Green"); 
		}
		break;	

		case EXT_CFG_EFFECT_AQUA :
		{
			CAMDRV_DEBUG("EXT_CFG_EFFECT_AQUA");
			mt9v113_sensor_write_list( mt9v113_Effect_Aqua, MT9V113_EFFECT_AQUA_REGS,\
			 "mt9v113_Effect_Aqua"); 
		}
		break;	
*/
		default :
		{
			printk("<=PCAM=> Unexpected Effect mode : %d\n",  value);
		}
		break;
				
	}
}


void mt9v113_pretty_control( char value)
{
	switch( value)
	{
		case EXT_CFG_PRETTY_LEVEL_0:
		{
			CAMDRV_DEBUG( "EXT_CFG_PRETTY_LEVEL_0");
			mt9v113_sensor_write_list( mt9v113_Pretty_Level_0, MT9V113_PRETTY_LEVEL_0_REGS,\
			 "mt9v113_Pretty_Level_0"); 
		}
		break;		

		case EXT_CFG_PRETTY_LEVEL_1:
		{
			CAMDRV_DEBUG( "EXT_CFG_PRETTY_LEVEL_1");
			mt9v113_sensor_write_list( mt9v113_Pretty_Level_1, MT9V113_PRETTY_LEVEL_1_REGS,\
			 "mt9v113_Pretty_Level_1"); 
		}
		break;	
		
		case EXT_CFG_PRETTY_LEVEL_2:
		{
			CAMDRV_DEBUG( "EXT_CFG_PRETTY_LEVEL_2");
			mt9v113_sensor_write_list( mt9v113_Pretty_Level_2, MT9V113_PRETTY_LEVEL_2_REGS,\
			 "mt9v113_Pretty_Level_2"); 
		}
		break;	

		case EXT_CFG_PRETTY_LEVEL_3:
		{
			CAMDRV_DEBUG( "EXT_CFG_PRETTY_LEVEL_3");
			mt9v113_sensor_write_list( mt9v113_Pretty_Level_3, MT9V113_PRETTY_LEVEL_3_REGS,\
			 "mt9v113_Pretty_Level_3"); 
		}
		break;	

		default :
		{
			printk( "<=PCAM=> Unexpected Pretty Effect mode : %d\n",  value);
		}
		break;
	}
}

void mt9v113_VT_pretty_control( char value)
{
	switch( value)
	{
		case EXT_CFG_PRETTY_LEVEL_0:
		{
			CAMDRV_DEBUG( "EXT_CFG_PRETTY_LEVEL_0");
			mt9v113_sensor_write_list( mt9v113_VT_Pretty_Level_0, MT9V113_VT_PRETTY_LEVEL_0_REGS,\
			 "mt9v113_VT_Pretty_Level_0"); 
		}
		break;		

		case EXT_CFG_PRETTY_LEVEL_1:
		{
			CAMDRV_DEBUG( "EXT_CFG_PRETTY_LEVEL_1");
			mt9v113_sensor_write_list( mt9v113_VT_Pretty_Level_1, MT9V113_VT_PRETTY_LEVEL_1_REGS,\
			 "mt9v113_VT_Pretty_Level_1"); 
		}
		break;	
		
		case EXT_CFG_PRETTY_LEVEL_2:
		{
			CAMDRV_DEBUG( "EXT_CFG_PRETTY_LEVEL_2");
			mt9v113_sensor_write_list( mt9v113_VT_Pretty_Level_2, MT9V113_VT_PRETTY_LEVEL_2_REGS,\
			 "mt9v113_VT_Pretty_Level_2"); 
		}
		break;	

		case EXT_CFG_PRETTY_LEVEL_3:
		{
			CAMDRV_DEBUG( "EXT_CFG_PRETTY_LEVEL_3");
			mt9v113_sensor_write_list( mt9v113_VT_Pretty_Level_3, MT9V113_VT_PRETTY_LEVEL_3_REGS,\
			 "mt9v113_VT_Pretty_Level_3"); 
		}
		break;	

		default :
		{
			printk( "<=PCAM=> Unexpected Pretty Effect mode : %d\n",  value);
		}
		break;
	}
}


void mt9v113_whitebalance_control(char value)
{

	switch(value)
	{
		case EXT_CFG_WB_AUTO :{
		CAMDRV_DEBUG( "EXT_CFG_WB_AUTO");
		mt9v113_sensor_write_list( mt9v113_WB_Auto, MT9V113_WB_AUTO_REGS,\
		 "mt9v113_WB_Auto"); 
		}
		break;	

		case EXT_CFG_WB_DAYLIGHT:{
		CAMDRV_DEBUG( "EXT_CFG_WB_DAYLIGHT");
		mt9v113_sensor_write_list( mt9v113_WB_Daylight, MT9V113_WB_DAYLIGHT_REGS,\
		 "mt9v113_WB_Daylight"); 
		}
		break;	

		case EXT_CFG_WB_CLOUDY :{
		CAMDRV_DEBUG( "EXT_CFG_WB_CLOUDY");
		mt9v113_sensor_write_list( mt9v113_WB_Cloudy, MT9V113_WB_CLOUDY_REGS,\
		 "mt9v113_WB_Cloudy"); 
		}
		break;	

		case EXT_CFG_WB_FLUORESCENT :{
		CAMDRV_DEBUG( "EXT_CFG_WB_FLUORESCENT");
		mt9v113_sensor_write_list( mt9v113_WB_Fluorescent, MT9V113_WB_FLUORESCENT_REGS,\
		 "mt9v113_WB_Fluorescent"); 
		}
		break;	
		
		case EXT_CFG_WB_INCANDESCENT :{
		CAMDRV_DEBUG( "EXT_CFG_WB_INCANDESCENT");
		mt9v113_sensor_write_list( mt9v113_WB_Incandescent, MT9V113_WB_INCANDESCENT_REGS,\
		 "mt9v113_WB_Incandescent"); 
		}
		break;	

		default :{
			printk("<=PCAM=> Unexpected WHITEBALANCE mode : %d\n",  value);
		}
		break;
		
	}// end of switch

}

void mt9v113_VT_brightness_control(char value)
{
	printk("<=PCAM=> VT Brightness Control 0x%x\n", value);

	switch(value)
	{

		case EXT_CFG_BR_STEP_P_4 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_P_4");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_p_4, MT9V113_VT_BRIGHTNESS_P_4_REGS,\
		 "mt9v113_VT_brightness_p_4"); 
		}
		break;

		case EXT_CFG_BR_STEP_P_3 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_P_3");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_p_3, MT9V113_VT_BRIGHTNESS_P_3_REGS,\
		 "mt9v113_VT_brightness_p_3"); 
		}
		break;

		case EXT_CFG_BR_STEP_P_2 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_P_2");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_p_2, MT9V113_VT_BRIGHTNESS_P_2_REGS,\
		 "mt9v113_VT_brightness_p_2"); 
		}
		break;

		case EXT_CFG_BR_STEP_P_1 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_P_1");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_p_1, MT9V113_VT_BRIGHTNESS_P_1_REGS,\
		 "mt9v113_VT_brightness_p_1"); 
		}
		break;

		case EXT_CFG_BR_STEP_0 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_0");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_0, MT9V113_VT_BRIGHTNESS_0_REGS, \
		"mt9v113_VT_brightness_0"); 
		}
		break;

		case EXT_CFG_BR_STEP_M_1 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_M_1");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_m_1, MT9V113_VT_BRIGHTNESS_M_1_REGS, \
		"mt9v113_VT_brightness_m_1"); 
		}
		break;

		case EXT_CFG_BR_STEP_M_2 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_M_2");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_m_2, MT9V113_VT_BRIGHTNESS_M_2_REGS, \
		"mt9v113_VT_brightness_m_2"); 
		}
		break;

		case EXT_CFG_BR_STEP_M_3 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_M_3");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_m_3, MT9V113_VT_BRIGHTNESS_M_3_REGS, \
		"mt9v113_VT_brightness_m_3"); 
		}
		break;

		case EXT_CFG_BR_STEP_M_4 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_M_4");
		mt9v113_sensor_write_list(mt9v113_VT_brightness_m_4, MT9V113_VT_BRIGHTNESS_M_4_REGS, \
		"mt9v113_VT_brightness_m_4"); 
		}
		break;

		default :{
			printk("<=PCAM=> Unexpected VT_BR mode : %d\n",  value);
		}
		break;

	}
}

void mt9v113_brightness_control(char value)
{
	printk("<=PCAM=> Brightness Control 0x%x\n", value);

	switch(value)
	{
		case EXT_CFG_BR_STEP_P_4 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_P_4");
		mt9v113_sensor_write_list(mt9v113_brightness_p_4, MT9V113_BRIGHTNESS_P_4_REGS,\
		 "mt9v113_brightness_p_4"); 
		}
		break;

		case EXT_CFG_BR_STEP_P_3 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_P_3");
		mt9v113_sensor_write_list(mt9v113_brightness_p_3, MT9V113_BRIGHTNESS_P_3_REGS,\
		 "mt9v113_brightness_p_3"); 
		}
		break;

		case EXT_CFG_BR_STEP_P_2 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_P_2");
		mt9v113_sensor_write_list(mt9v113_brightness_p_2, MT9V113_BRIGHTNESS_P_2_REGS,\
		 "mt9v113_brightness_p_2"); 
		}
		break;

		case EXT_CFG_BR_STEP_P_1 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_P_1");
		mt9v113_sensor_write_list(mt9v113_brightness_p_1, MT9V113_BRIGHTNESS_P_1_REGS,\
		 "mt9v113_brightness_p_1"); 
		}
		break;

		case EXT_CFG_BR_STEP_0 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_0");
		mt9v113_sensor_write_list(mt9v113_brightness_0, MT9V113_BRIGHTNESS_0_REGS, \
		"mt9v113_brightness_0"); 
		}
		break;

		case EXT_CFG_BR_STEP_M_1 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_M_1");
		mt9v113_sensor_write_list(mt9v113_brightness_m_1, MT9V113_BRIGHTNESS_M_1_REGS, \
		"mt9v113_brightness_m_1"); 
		}
		break;

		case EXT_CFG_BR_STEP_M_2 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_M_2");
		mt9v113_sensor_write_list(mt9v113_brightness_m_2, MT9V113_BRIGHTNESS_M_2_REGS, \
		"mt9v113_brightness_m_2"); 
		}
		break;

		case EXT_CFG_BR_STEP_M_3 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_M_3");
		mt9v113_sensor_write_list(mt9v113_brightness_m_3, MT9V113_BRIGHTNESS_M_3_REGS, \
		"mt9v113_brightness_m_3"); 
		}
		break;

		case EXT_CFG_BR_STEP_M_4 :{
		CAMDRV_DEBUG("EXT_CFG_BR_STEP_M_4");
		mt9v113_sensor_write_list(mt9v113_brightness_m_4, MT9V113_BRIGHTNESS_M_4_REGS, \
		"mt9v113_brightness_m_4"); 
		}
		break;

		default :{
			printk("<=PCAM=> Unexpected BR mode : %d\n",  value);
		}
		break;

	}
}

void mt9v113_FPS_control(char value)
{
	printk("mt9v113_FPS_control() value:%d", value );
	
	switch( value)
	{
		case EXT_CFG_FRAME_AUTO :
		{
			CAMDRV_DEBUG( "EXT_CFG_FRAME_AUTO\n");
		}					
		break;
		
		case EXT_CFG_FRAME_FIX_15 :
		{
			CAMDRV_DEBUG( "EXT_CFG_FRAME_FIX_15\n");		
			mt9v113_sensor_write_list( mt9v113_15_fps, MT9V113_15_FPS_REGS,\
			 "mt9v113_15_fps"); 	
		}
		break;

		case EXT_CFG_FRAME_FIX_7 :
		{
			CAMDRV_DEBUG( "EXT_CFG_FRAME_FIX_7\n");		
			mt9v113_sensor_write_list( mt9v113_7_fps, MT9V113_7_FPS_REGS,\
			 "mt9v113_7_fps"); 	
		}
		break;

		default :
		{
			CAMDRV_DEBUG( "<=PCAM=> Unexpected EXT_CFG_FRAME_CONTROL mode : %d\n", value);
		}
		break;				
	}
}

void mt9v113_VT_FPS_control(char value)
{
	printk("mt9v113_VT_FPS_control() value:%d", value );
	
	switch( value)
	{
		case EXT_CFG_FRAME_AUTO :
		{
			CAMDRV_DEBUG( "EXT_CFG_FRAME_AUTO\n");
		}					
		break;
		
		case EXT_CFG_FRAME_FIX_15 :
		{
			CAMDRV_DEBUG( "EXT_CFG_FRAME_FIX_15\n");		
			mt9v113_sensor_write_list( mt9v113_VT_15_fps, MT9V113_VT_15_FPS_REGS,\
			 "mt9v113_15_fps"); 	
		}
		break;

		case EXT_CFG_FRAME_FIX_7 :
		{
			CAMDRV_DEBUG( "EXT_CFG_FRAME_FIX_7\n");		
			mt9v113_sensor_write_list( mt9v113_VT_7_fps, MT9V113_VT_7_FPS_REGS,\
			 "mt9v113_7_fps"); 	
		}
		break;

		default :
		{
			CAMDRV_DEBUG( "<=PCAM=> Unexpected EXT_CFG_FRAME_CONTROL mode : %d\n", value);
		}
		break;				
	}
}

#if 0
void sensor_metering_control(char value)
{
	printk("<=PCAM=> Metering Control\n");
	return;

	switch(value)
	{
		case EXT_CFG_METERING_NORMAL :{
		CAMDRV_DEBUG("EXT_CFG_METERING_NORMAL");
		mt9v113_sensor_write_list(mt9v113_metering_normal, S5K5CA_METERING_NORMAL_REGS, \
		"mt9v113_metering_normal"); 
		}
		break;
		
		case EXT_CFG_METERING_SPOT :{
		CAMDRV_DEBUG("EXT_CFG_METERING_SPOT");
		mt9v113_sensor_write_list(mt9v113_metering_spot, S5K5CA_METERING_SPOT_REGS, \
		"mt9v113_metering_spot"); 
		}
		break;

		case EXT_CFG_METERING_CENTER :{
		CAMDRV_DEBUG("EXT_CFG_METERING_CENTER");
		mt9v113_sensor_write_list(mt9v113_metering_center, S5K5CA_METERING_CENTER_REGS, \
		"mt9v113_metering_center"); 
		}
		break;

		default :{
			printk("<=PCAM=> Unexpected METERING mode : %d\n",  value);
		}
		break;
	}
}
#endif

static void mt9v113_set_power( int status)
{
    struct vreg *vreg_cam_vcamc;

	/* for faceunlock fucntion */
	static int init = false;
	static struct wake_lock wakelock;
	if( init == false ) {
		CAMDRV_DEBUG("mt9v113_set_power : wakelock init\n");
		wake_lock_init(&wakelock, WAKE_LOCK_SUSPEND, "_mt9v113_\n");
		init = true;
	}

    vreg_cam_vcamc	= vreg_get(NULL, "vcamc");

	if(status == 1) //POWER ON
	{
		wake_lock(&wakelock); //for faceunlock fucntion

		CAMDRV_DEBUG("mt9v113_set_power : POWER ON\n");
		
		// gpio & pmic config
		gpio_tlmm_config(GPIO_CFG(CAM_MEGA_RST, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(CAM_MEGA_STBY, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(CAM_VGA_RST, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(CAM_VGA_STBY, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		
		pmic_gpio_direction_output(PMIC_CAM_MEGA_EN);
		pmic_gpio_direction_output(PMIC_CAM_VGA_EN);
		pmic_gpio_direction_output(PMIC_CAM_EN);
		
		gpio_set_value(CAM_MEGA_RST, 0);
		gpio_set_value(CAM_MEGA_STBY, 0);
		gpio_set_value(CAM_VGA_STBY, 0);
		gpio_set_value(CAM_VGA_RST, 0);
		pmic_gpio_set_value(PMIC_CAM_EN, 0);
		pmic_gpio_set_value(PMIC_CAM_MEGA_EN, 0);
		pmic_gpio_set_value(PMIC_CAM_VGA_EN, 0);
		
		vreg_set_level(vreg_cam_vcamc, 1200);
		// end of gpio & pmic config

		// start power sequence
		gpio_set_value(CAM_VGA_RST, 1);
		mdelay(2); // >1ms

		vreg_enable(vreg_cam_vcamc); // VCAMC_1.2V
		udelay(1); // >0us

		pmic_gpio_set_value(PMIC_CAM_EN, 1); // VCAM_AF_3.0V & VCAMIO_1.8V (Main & VGA IO)
		udelay(30); // >20us
		
		pmic_gpio_set_value(PMIC_CAM_VGA_EN, 1); // VGA_CORE_1.8V
		udelay(20); // >15us
		
		pmic_gpio_set_value(PMIC_CAM_MEGA_EN, 1); // VCAMA_2.8V
		udelay(30); // >20us

		gpio_set_value(CAM_VGA_RST, 0);
		mdelay(55); // >50ms

		msm_camio_clk_enable(CAMIO_CAM_MCLK_CLK);
		msm_camio_clk_rate_set(24000000);
		udelay(20); // >10us (temp)
		
		gpio_set_value(CAM_MEGA_STBY, 1);
		mdelay(10); // >6.5ms
		
		gpio_set_value(CAM_MEGA_RST, 1);
		mdelay(10); // >5ms

		gpio_set_value(CAM_MEGA_STBY, 0);
		mdelay(2); // >1ms
		
		gpio_set_value(CAM_VGA_RST, 1);
		mdelay(2); // >1ms
	}
	else //POWER OFF
	{
		CAMDRV_DEBUG("mt9v113_set_power : POWER OFF\n");

		gpio_set_value(CAM_MEGA_RST, 0);
		udelay(60); // >50us
		
		msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);

		gpio_set_value(CAM_MEGA_STBY, 0);
		gpio_set_value(CAM_VGA_RST, 0);
		gpio_set_value(CAM_VGA_STBY, 0);
		
		pmic_gpio_set_value(PMIC_CAM_EN, 0);
		pmic_gpio_set_value(PMIC_CAM_VGA_EN, 0);
		pmic_gpio_set_value(PMIC_CAM_MEGA_EN, 0);
		vreg_disable(vreg_cam_vcamc);

		// for sleep current
		gpio_tlmm_config(GPIO_CFG(CAM_GPIO_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(CAM_GPIO_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(CAM_GPIO_SCL, 0);
		gpio_set_value(CAM_GPIO_SDA, 0);

		wake_unlock(&wakelock); //for faceunlock fucntion
	}
}

static void mt9v113_restart(void)
{
    struct vreg *vreg_cam_vcamc;

    vreg_cam_vcamc	= vreg_get(NULL, "vcamc");

	CAMDRV_DEBUG("mt9v113_restart");

	gpio_set_value(CAM_MEGA_RST, 0);
	udelay(60); // >50us
	
	msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);

	gpio_set_value(CAM_MEGA_STBY, 0);
	gpio_set_value(CAM_VGA_RST, 0);
	gpio_set_value(CAM_VGA_STBY, 0);
	
	pmic_gpio_set_value(PMIC_CAM_EN, 0);
	pmic_gpio_set_value(PMIC_CAM_VGA_EN, 0);
	pmic_gpio_set_value(PMIC_CAM_MEGA_EN, 0);
	vreg_disable(vreg_cam_vcamc);

	mdelay(10);
	// power on
	vreg_set_level(vreg_cam_vcamc, 1200);
	// end of gpio & pmic config

	// start power sequence
	gpio_set_value(CAM_VGA_RST, 1);
	mdelay(2); // >1ms

	vreg_enable(vreg_cam_vcamc); // VCAMC_1.2V
	udelay(1); // >0us

	pmic_gpio_set_value(PMIC_CAM_EN, 1); // VCAM_AF_3.0V & VCAMIO_1.8V (Main & VGA IO)
	udelay(30); // >20us
	
	pmic_gpio_set_value(PMIC_CAM_VGA_EN, 1); // VGA_CORE_1.8V
	udelay(20); // >15us
	
	pmic_gpio_set_value(PMIC_CAM_MEGA_EN, 1); // VCAMA_2.8V
	udelay(30); // >20us

	gpio_set_value(CAM_VGA_RST, 0);
	mdelay(55); // >50ms

	msm_camio_clk_enable(CAMIO_CAM_MCLK_CLK);
	msm_camio_clk_rate_set(24000000);
	udelay(20); // >10us (temp)
	
	gpio_set_value(CAM_MEGA_STBY, 1);
	mdelay(10); // >6.5ms
	
	gpio_set_value(CAM_MEGA_RST, 1);
	mdelay(10); // >5ms

	gpio_set_value(CAM_MEGA_STBY, 0);
	mdelay(2); // >1ms
	
	gpio_set_value(CAM_VGA_RST, 1);
	mdelay(2); // >1ms

	// pre init for mipi
	mt9v113_sensor_write_list( mt9v113_pre_init_reg, MT9V113_PRE_INIT_REGS,\
	 "mt9v113_pre_init_reg"); 
}

void mt9v113_DTP_control( char value)
{
	switch( value)
	{
		case EXT_CFG_DTP_OFF:
		{
			CAMDRV_DEBUG( "DTP OFF");
			mt9v113_sensor_write_list( mt9v113_dtp_off, MT9V113_DTP_OFF_REGS, "mt9v113_dtp_off");
			//mt9v113_reset();
		}
		break;

		case EXT_CFG_DTP_ON:
		{
			CAMDRV_DEBUG( "DTP ON");
			mt9v113_sensor_write_list( mt9v113_dtp_on, MT9V113_DTP_ON_REGS, "mt9v113_dtp_on");
		}
		break;

		default:
		{
			printk( "<=PCAM=> unexpected DTP control on PCAM\n");
		}
		break;
	}
}

void mt9v113_CPU_control(char value)
{
	switch(value)
	{
		case EXT_CFG_CPU_ONDEMAND:{
		CAMDRV_DEBUG("EXT_CFG_CPU_ONDEMAND");
		cpufreq_direct_set_policy(0, "ondemand");
		}
		break;

		case EXT_CFG_CPU_PERFORMANCE:{
		CAMDRV_DEBUG("EXT_CFG_CPU_PERFORMANCE");
		cpufreq_direct_set_policy(0, "performance");
		}
		break;
	}
}

int mt9v113_sensor_ext_config(void __user *arg)
{
	long   rc = 0;
	ioctl_pcam_info_8bit		ctrl_info;

	CAMDRV_DEBUG("START");

	if(copy_from_user((void *)&ctrl_info, (const void *)arg, sizeof(ctrl_info)))
	{
		printk("<=PCAM=> %s fail copy_from_user!\n", __func__);
	}

	printk("<=PCAM=> TEST %d %d %d %d %d \n", ctrl_info.mode, ctrl_info.address,\
	 ctrl_info.value_1, ctrl_info.value_2, ctrl_info.value_3);

	if( mt9v113_data.mDTP ){
		if( ctrl_info.mode!=EXT_CFG_DTP_CONTROL ){
			printk("[mt9v113] skip sensor_ext_config : DTP on");
			return 0;
		}
	}

	switch(ctrl_info.mode)
	{
		case EXT_CFG_GET_INFO:
		{
		}
		break;

		case EXT_CFG_VT_MODE_CONTROL:
		{
			if( 1 == ctrl_info.value_1 || 2 == ctrl_info.value_1)
			{
				printk( "EXT_CFG_VT_MODE\n");
				mt9v113_data.mVtMode = 1;
				//mVtMode = 1;
			}
			else if( 0 == ctrl_info.value_1)
			{
				printk( "EXT_CFG_NORMAL_MODE\n");
				mt9v113_data.mVtMode = 0;
				//mVtMode = 0;
			}
			else
			{
				CAMDRV_DEBUG( "EXT_CFG_MODE CHANGE ERROR\n");
			}
		}
		break;

		case EXT_CFG_FRAME_CONTROL:
		{
			mt9v113_data.mFPS = ctrl_info.value_1;
			if( mt9v113_data.mInit ) {
				if( 0 == mt9v113_data.mVtMode)
					mt9v113_FPS_control( mt9v113_data.mFPS);
				if( 1 == mt9v113_data.mVtMode)
					mt9v113_VT_FPS_control( mt9v113_data.mFPS);
			}
		}
		break;

		case EXT_CFG_BR_CONTROL:
		{
			mt9v113_data.mBrightness = ctrl_info.value_1;
			if( mt9v113_data.mInit ) {
				if( 0 == mt9v113_data.mVtMode)
					mt9v113_brightness_control( mt9v113_data.mBrightness);
				if( 1 == mt9v113_data.mVtMode)
					mt9v113_VT_brightness_control( mt9v113_data.mBrightness);
			}
		}//end of EXT_CFG_BR_CONTROL
		break;

		case EXT_CFG_PRETTY_CONTROL:
		{
			mt9v113_data.mPrettyEffect = ctrl_info.value_1;
			if( mt9v113_data.mInit ) {
				if( 0 == mt9v113_data.mVtMode) {
					mt9v113_pretty_control( mt9v113_data.mPrettyEffect);
				}
				if( 1 == mt9v113_data.mVtMode) {
					mt9v113_VT_pretty_control( mt9v113_data.mPrettyEffect);
				}
			}
		}
		break;
			
		case EXT_CFG_DTP_CONTROL:
		{
			mt9v113_data.mDTP = ctrl_info.value_1;
			if( mt9v113_data.mInit ) {
				mt9v113_DTP_control( mt9v113_data.mDTP);
			}
		}
		break;
		
		case  EXT_CFG_CPU_CONTROL:
		{
			mt9v113_CPU_control(ctrl_info.value_1);
		}
		break;

		default :{
			printk("<=PCAM=> Unexpected mode on mt9v113_sensor_control : %d\n", ctrl_info.mode);
		}
		break;
	}

	if(copy_to_user((void *)arg, (const void *)&ctrl_info, sizeof(ctrl_info)))
	{
		printk("<=PCAM=> %s fail on copy_to_user!\n", __func__);
	}
    return rc;
}


/*===========================================================================
FUNCTION      CAMSENSOR_mt9v113_CHECK_SENSOR_REV
===========================================================================*/

static unsigned char mt9v113_check_sensor_rev( void)
{
    unsigned char id = 0xff;
    int ret0, ret1;
  
    mt9v113_sensor_version = 0xff;
/*    
    ret0 = mt9v113_sensor_write( 0x03, 0x00);
	ret1 = mt9v113_sensor_read( 0x04, &id);
    
  	printk("mt9v113_check_sensor_rev!!! ret0:%d ret1:%d\n",ret0,ret1);
  
    if( id == 0x8c )
    {
        printk("============================\n");
        printk("[cam] MT9V113				\n");
        printk("============================\n");

        mt9v113_sensor_version 	= 0x8c; //MT9V113
    }
    else 
    {
		// retry check sensor revision
		ret0 = mt9v113_sensor_write( 0x03, 0x00);
		ret1 = mt9v113_sensor_read( 0x04, &id);

		printk("mt9v113_check_sensor_rev!!! ret0:%d ret1:%d\n",ret0,ret1);
		
		if( id == 0x8c )
		{
			printk("============================\n");
			printk("[cam] MT9V113 			\n");
			printk("============================\n");
		
			mt9v113_sensor_version	= 0x8c; //MT9V113
		}
		else
		{
			printk("-----------------------------------------------\n");
			printk("   [cam] INVALID SENSOR  : %d	\n",id);
			printk("-----------------------------------------------\n");
			
			mt9v113_sensor_version = 0xFF; //No sensor
		}
    }*/
    return mt9v113_sensor_version;
}

static long mt9v113_set_effect(int mode, int effect)
{
    long rc = 0;

    switch (mode) {
        case SENSOR_PREVIEW_MODE:
        	CAMDRV_DEBUG("SENSOR_PREVIEW_MODE");
        	break;

        case SENSOR_SNAPSHOT_MODE:
        	CAMDRV_DEBUG("SENSOR_SNAPSHOT_MODE");
        	break;

        default:
        	printk("[PGH] %s default\n", __func__);
        	break;
    }

    switch (effect) {
        case CAMERA_EFFECT_OFF: {
            CAMDRV_DEBUG("CAMERA_EFFECT_OFF");
    }
        break;

        case CAMERA_EFFECT_MONO: {
            CAMDRV_DEBUG("CAMERA_EFFECT_MONO");
    }
        break;

        case CAMERA_EFFECT_NEGATIVE: {
            CAMDRV_DEBUG("CAMERA_EFFECT_NEGATIVE");
    }
        break;

        case CAMERA_EFFECT_SOLARIZE: {
            CAMDRV_DEBUG("CAMERA_EFFECT_SOLARIZE");
    }
        break;

        case CAMERA_EFFECT_SEPIA: {
            CAMDRV_DEBUG("CAMERA_EFFECT_SEPIA");
    }
        break;

    default: {
        printk("<=PCAM=> unexpeceted effect  %s/%d\n", __func__, __LINE__);
        return -EINVAL;
    }

    }

    return rc;
}


static int mt9v113_debugprint_preview_data( void)
{
/*	unsigned char read_value = 0;

	printk( "mt9v113_preview start\n");
	printk( "==============================\n");
	mt9v113_sensor_write( 0xef, 0x03);
	mt9v113_sensor_read( 0x33, &read_value);
	printk("Normal Light Contrast : 0x%2x\n", read_value);
	mt9v113_sensor_read( 0x34, &read_value);
	printk("Low Light Contrast    : 0x%2x\n", read_value);
	mt9v113_sensor_read( 0x01, &read_value);
	printk("AE Target             : 0x%2x\n", read_value);
	mt9v113_sensor_read( 0x02, &read_value);
	printk("AE Threshold          : 0x%2x\n", read_value);
	mt9v113_sensor_read( 0x67, &read_value);
	printk("AE Speed              : 0x%2x\n", read_value);
	printk( "==============================\n");
*/
	return 0;
}

int mt9v113_mipi_mode(void)
{
	int rc = 0;
	struct msm_camera_csi_params mt9v113_csi_params;
	
	CAMDRV_DEBUG("%s E\n",__FUNCTION__);

	if (!config_csi1) {
		CAMDRV_DEBUG("%s : config_csi1\n",__FUNCTION__);
		mt9v113_csi_params.lane_cnt = 1;
		mt9v113_csi_params.data_format = CSI_8BIT;
		mt9v113_csi_params.lane_assign = 0xe4;
		mt9v113_csi_params.dpcm_scheme = 0;
		mt9v113_csi_params.settle_cnt = 0x14;
		rc = msm_camio_csi_config(&mt9v113_csi_params);
		if (rc < 0)
			printk(KERN_ERR "config csi controller failed \n");
		config_csi1 = 1;
		msleep(5); //=> Please add some delay 
	}
	CAMDRV_DEBUG("%s X\n",__FUNCTION__);
	return rc;
}


static int mt9v113_start_preview( void)
{
	mt9v113_debugprint_preview_data();

	mt9v113_mipi_mode();
		
	if( mt9v113_data.mDTP == 1)
	{
		mt9v113_sensor_write_list( mt9v113_init_reg, MT9V113_INIT_REGS, "mt9v113_init_reg");     
		mt9v113_sensor_write_list( mt9v113_dtp_on, MT9V113_DTP_ON_REGS, "mt9v113_dtp_on");
	}
	else
	{
		if( !mt9v113_data.mInit )
		{
			if( 0 == mt9v113_data.mVtMode)
			{
				printk( "mt9v113 Normal Preview start\n");
				mt9v113_sensor_write_list( mt9v113_init_reg, MT9V113_INIT_REGS, "mt9v113_init_reg");     
				mt9v113_FPS_control( mt9v113_data.mFPS);
				if( mt9v113_data.mBrightness!=EXT_CFG_BR_STEP_0 ) {
					mt9v113_brightness_control( mt9v113_data.mBrightness);
				}
				if( mt9v113_data.mPrettyEffect!=0 ) {
					mt9v113_pretty_control( mt9v113_data.mPrettyEffect);
				}
			}
			else if( 1 == mt9v113_data.mVtMode)
			{
				printk( "mt9v113 VtMode Preview start\n");
				mt9v113_sensor_write_list( mt9v113_VT_init_reg, MT9V113_VT_INIT_REGS, "mt9v113_VT_init_reg"); 
				if( mt9v113_data.mBrightness!=EXT_CFG_BR_STEP_0 ) {
					mt9v113_VT_brightness_control( mt9v113_data.mBrightness);
				}
			}
		}
	}
	mt9v113_data.mInit = 1;

	return 0;
}


static long mt9v113_set_sensor_mode(int mode)
{
	printk( "mt9v113_set_sensor_mode start : %d\n", mode);

	switch (mode) 
	{
		case SENSOR_PREVIEW_MODE:
		{
			CAMDRV_DEBUG("PREVIEW~~~\n");
			mt9v113_start_preview();
		}
		break;
			
		case SENSOR_SNAPSHOT_MODE:
		{
			CAMDRV_DEBUG("SNAPSHOT~~~\n");
		}
		break;

		case SENSOR_RAW_SNAPSHOT_MODE:
		{
			CAMDRV_DEBUG("RAW_SNAPSHOT NOT SUPPORT!!");
		}
		break;

		default:
		{
			return -EINVAL;
		}
	}

	return 0;
}

#ifdef CONFIG_LOAD_FILE

#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include <asm/uaccess.h>

static char *mt9v113_regs_table = NULL;

static int mt9v113_regs_table_size;

void mt9v113_regs_table_init(void)
{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
//	int i;
	int ret;
	mm_segment_t fs = get_fs();

	printk("%s %d\n", __func__, __LINE__);

	set_fs(get_ds());
#if 0
	filp = filp_open("/data/camera/s5k5ca.h", O_RDONLY, 0);
#else
	filp = filp_open("/mnt/sdcard/mt9v113.h", O_RDONLY, 0);
#endif
	if (IS_ERR(filp)) {
		printk("file open error\n");
		return;
	}
	l = filp->f_path.dentry->d_inode->i_size;	
	printk("l = %ld\n", l);
	dp = kmalloc(l, GFP_KERNEL);
	if (dp == NULL) {
		printk("Out of Memory\n");
		filp_close(filp, current->files);
	}
	pos = 0;
	memset(dp, 0, l);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);
	if (ret != l) {
		printk("Failed to read file ret = %d\n", ret);
		kfree(dp);
		filp_close(filp, current->files);
		return;
	}

	filp_close(filp, current->files);
	
	set_fs(fs);

	mt9v113_regs_table = dp;
	
	mt9v113_regs_table_size = l;

	*((mt9v113_regs_table + mt9v113_regs_table_size) - 1) = '\0';

	printk("mt9v113_regs_table 0x%x, %ld\n", dp, l);
}

void mt9v113_regs_table_exit(void)
{
	printk("%s %d\n", __func__, __LINE__);
	if (mt9v113_regs_table) {
		kfree(mt9v113_regs_table);
		mt9v113_regs_table = NULL;
	}	
}

static int mt9v113_is_hexnum(char* num)
{
	int i;
	for( i=2; num[i]!='\0' ; i++ )
	{
		if( !((num[i]>='0' && num[i]<='9') || (num[i]>='a' && num[i]<='f') || (num[i]>='A' && num[i]<='F')) )
		{
			return 0;
		}
	}
	return 1;
}

static int mt9v113_regs_table_write(char *name)
{
	char *start, *end, *reg;//, *data;	
	unsigned short addr, value;
	unsigned long data;
	char data_buf[11];

	addr = value = 0;

	*(data_buf + 10) = '\0';

	start = strstr(mt9v113_regs_table, name);
	
	end = strstr(start, "};");

	while (1) { 
		/* Find Address */	
		reg = strstr(start,"0x");		
		if (reg)
			start = (reg + 10);
		
		if ((reg == NULL) || (reg > end))
		{
			printk("[mt9v113] write end of %s\n",name);
			break;
		}
		/* Write Value to Address */	
		memcpy(data_buf, reg, 10);	

		if( mt9v113_is_hexnum(data_buf)==0 )
		{
			printk("[mt9v113] it's not hex number %s\n",data_buf);
			continue;
		}
		
		data = (unsigned long)simple_strtoul(data_buf, NULL, 16); 
		//printk("data 0x%08x\n", data);
		addr = (data >> 16);
		value = (data & 0xffff);
		
		if (addr == 0xffff)
		{
			if( value == 0xffff )
			{
				mt9v113_sensor_polling();
			}
			else
			{
				CAMDRV_DEBUG("SETFILE DELAY : %dms",value);
				msleep(value);
			}
		}	
		else
		{
			if( mt9v113_sensor_write(addr, value) < 0 )
			{
				printk("[mt9v113] %s fail on sensor_write : addr[0x%04x], value[0x%04x]\n", __func__, addr, value);
			}
		}
	}

	return 0;
}
#endif



int mt9v113_sensor_init(const struct msm_camera_sensor_info *data)
{
	int rc = 0;
	
	mt9v113_data.mBrightness = EXT_CFG_BR_STEP_0;
	mt9v113_data.mDTP = 0;
	mt9v113_data.mFPS = 0;
	mt9v113_data.mVtMode = 0;
	mt9v113_data.mPrettyEffect= 0;
	mt9v113_data.mInit = 0;
	
	config_csi1 = 0;

	mt9v113_ctrl = kzalloc(sizeof(struct mt9v113_ctrl), GFP_KERNEL);
	if (!mt9v113_ctrl) {
		CDBG("mt9v113_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	if (data)
		mt9v113_ctrl->sensordata = data;

	mt9v113_set_power(1);
	
#ifdef CONFIG_LOAD_FILE
	mt9v113_regs_table_init();
#endif	

	mt9v113_sensor_write_list( mt9v113_pre_init_reg, MT9V113_PRE_INIT_REGS, "mt9v113_pre_init_reg"); 

init_done:
	return rc;

init_fail:
	kfree(mt9v113_ctrl);
	return rc;
}

static int mt9v113_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&mt9v113_wait_queue);
	return 0;
}

int mt9v113_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;
	long   rc = 0;

	if (copy_from_user(&cfg_data,
			(void *)argp,
			sizeof(struct sensor_cfg_data)))
		return -EFAULT;

	/* down(&mt9v113_sem); */

	CDBG("mt9v113_ioctl, cfgtype = %d, mode = %d\n",
		cfg_data.cfgtype, cfg_data.mode);

		switch (cfg_data.cfgtype) {
		case CFG_SET_MODE:
			rc = mt9v113_set_sensor_mode(
						cfg_data.mode);
			break;

		case CFG_SET_EFFECT:
			rc = mt9v113_set_effect(cfg_data.mode,
						cfg_data.cfg.effect);
			break;

		case CFG_GET_AF_MAX_STEPS:
		default:
			rc = -EINVAL;
			break;
		}

	/* up(&mt9v113_sem); */

	return rc; 
}

int mt9v113_sensor_release(void)
{
	int rc = 0;

	//If did not init below that, it can keep the previous status. it depend on concept by PCAM

	mt9v113_data.mBrightness = EXT_CFG_BR_STEP_0;
	mt9v113_data.mDTP = 0;
	mt9v113_data.mFPS = 0;
	mt9v113_data.mVtMode = 0;
	mt9v113_data.mPrettyEffect= 0;
	mt9v113_data.mInit = 0;

	printk("<=PCAM=> mt9v113_sensor_release\n");

	kfree(mt9v113_ctrl);

#ifdef CONFIG_LOAD_FILE
	mt9v113_regs_table_exit();
#endif

	mt9v113_set_power(0);
	return rc;
}

static int mt9v113_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}

	mt9v113_sensorw =
		kzalloc(sizeof(struct mt9v113_work), GFP_KERNEL);

	if (!mt9v113_sensorw) {
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, mt9v113_sensorw);
	mt9v113_init_client(client);
	mt9v113_client = client;

	printk("mt9v113_probe succeeded!\n");

	return 0;

probe_failure:
	kfree(mt9v113_sensorw);
	mt9v113_sensorw = NULL;
	CDBG("mt9v113_probe failed!\n");
	return rc;
}

static const struct i2c_device_id mt9v113_i2c_id[] = {
	{ "mt9v113", 0},
	{ },
};

static struct i2c_driver mt9v113_i2c_driver = {
	.id_table = mt9v113_i2c_id,
	.probe  = mt9v113_i2c_probe,
	.remove = __exit_p(mt9v113_i2c_remove),
	.driver = {
		.name = "mt9v113",
	},
};

static int mt9v113_sensor_probe(const struct msm_camera_sensor_info *info,
				struct msm_sensor_ctrl *s)
{
    unsigned char id;
	int rc = i2c_add_driver(&mt9v113_i2c_driver);
	if (rc < 0 || mt9v113_sensorw == NULL) {
		rc = -ENOTSUPP;
		goto probe_done;
	}
	
	mt9v113_set_power(1);
/*
	rc = mt9v113_sensor_write_list( mt9v113_init_reg, MT9V113_INIT_REGS, "mt9v113_init_reg");	 
	if( rc < 0 )
	{
		printk("mt9v113_sensor_write_list i2c fail");
	}
*/
	s->s_init = mt9v113_sensor_init;
	s->s_release = mt9v113_sensor_release;
	s->s_config  = mt9v113_sensor_config;
	s->s_ext_config = mt9v113_sensor_ext_config; // for samsung camsensor control
    
	s->s_camera_type = FRONT_CAMERA_2D;
	s->s_mount_angle = 270;

probe_done:

	mt9v113_set_power(0);
	
	printk("%s %s:%d\n", __FILE__, __func__, __LINE__);
	return rc;
}

static int __mt9v113_probe(struct platform_device *pdev)
{
	printk("======== MT9V113 probe ==========\n");
	return msm_camera_drv_start(pdev, mt9v113_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __mt9v113_probe,
	.driver = {
		.name = "msm_camera_mt9v113",
		.owner = THIS_MODULE,
	},
};

static int __init mt9v113_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

module_init(mt9v113_init);
