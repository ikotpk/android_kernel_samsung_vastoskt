/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/delay.h>
#include <mach/gpio.h>
#include "msm_fb.h"
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <mach/irqs.h>
#include <mach/vreg.h>
#include "lcdc_s6e63m0.h"

//#define LCD_DET_ENABLE


#ifdef SMART_DIMMING
#include "smart_dimming.h"

#define MIN_BRIGHTNESS	0
#define MAX_BRIGHTNESS	255
#define DEFAULT_GAMMA_LEVEL	13
#define DEFAULT_BRIGHTNESS	150

struct lcd_info {
	unsigned int			bl;
	unsigned int			acl_enable;
	unsigned int			cur_acl;
	unsigned int			current_bl;

	unsigned int			ldi_enable;
	unsigned int			power;
	//unsigned int			make_table_flag;
	struct mutex			lock;

	struct device		*dev;
	//struct lcd_device		*ld;
	unsigned int 		max_brightness;
	unsigned int 		brightness;
	//struct backlight_device		*bd;
	
	
	struct lcd_platform_data	*lcd_pd;
	struct early_suspend		early_suspend;
	struct msm_fb_data_type  *mfd;

	unsigned int			lcd_id;

	const unsigned char	**gamma_table;

	unsigned int			support_elvss;
//	struct str_elvss		elvss;

#ifdef SMART_DIMMING //
	struct str_smart_dim		smart;
	struct mutex			bl_lock;
#endif
};

struct lcd_info *lcd_temp;

#endif

#ifdef LCD_DET_ENABLE
#define GPIO_LCD_DET	122
boolean irq_disabled = FALSE;
static int ESD_count = 0;
static irqreturn_t s6e63m0_esd_irq_handler(int irq, void *handle);
#endif

static int ldi_enable = 0;

static int current_gamma_level = -1;
#ifdef CONFIG_USES_SMART_ELVSS
static int current_elvss_level = -1;
unsigned char id3 = 0;
#endif
static int lcd_type = 0;
static int spi_cs;
static int spi_sclk;
static int spi_sdi;
static int lcd_reset;
static int delayed_backlight_value = -1;
static boolean First_Disp_Power_On = FALSE;
static struct msm_panel_common_pdata *lcdc_s6e63m0_pdata;
static struct device *lcd_dev;
extern struct class *sec_class;
#ifdef CONFIG_USES_ACL
static int acl_enable = 0;
static int cur_acl = 0;
static struct class *acl_class;
static struct device *switch_aclset_dev;
#endif
#ifdef GAMMASET_CONTROL
struct class *gammaset_class;
struct device *switch_gammaset_dev;
#endif
static int on_19gamma = 0;

int lcd_on_state_for_debug = 0;
EXPORT_SYMBOL(lcd_on_state_for_debug);

#define DEFAULT_LCD_ON_BACKLIGHT_LEVEL 23

static DEFINE_SPINLOCK(lcd_ctrl_irq_lock);
static DEFINE_SPINLOCK(bl_ctrl_lock);
static DEVICE_ATTR(lcd_power , 0664, s6e63m0_show_power, s6e63m0_store_power);
static DEVICE_ATTR(lcdtype_file_cmd , 0664, s6e63m0_show_lcd_type, NULL);
#ifdef CONFIG_USES_ACL
static DEVICE_ATTR(aclset_file_cmd,0664, aclset_file_cmd_show, aclset_file_cmd_store);
#endif
#ifdef GAMMASET_CONTROL //for 1.9/2.2 gamma control from platform
static DEVICE_ATTR(gammaset_file_cmd,0664, gammaset_file_cmd_show, gammaset_file_cmd_store);
#endif
static int s6e63m0_adb_brightness_update(struct lcd_info *lcd, u32 br, u32 force) ;

static const unsigned short *p22Gamma_set[] = {
    NULL,// display off
    s6e63m0_22gamma_30cd,// 1 (dim)
    s6e63m0_22gamma_40cd,
    s6e63m0_22gamma_50cd,
    s6e63m0_22gamma_60cd,
    s6e63m0_22gamma_70cd,// 5
    s6e63m0_22gamma_80cd,
    s6e63m0_22gamma_90cd,
    s6e63m0_22gamma_100cd,
    s6e63m0_22gamma_110cd,
    s6e63m0_22gamma_120cd,// 10
    s6e63m0_22gamma_130cd,
    s6e63m0_22gamma_140cd,
    s6e63m0_22gamma_150cd,
    s6e63m0_22gamma_160cd,
    s6e63m0_22gamma_170cd,// 15// default
    s6e63m0_22gamma_170cd,
    s6e63m0_22gamma_180cd,
    s6e63m0_22gamma_190cd,
    s6e63m0_22gamma_200cd,
    s6e63m0_22gamma_210cd,// 20
    s6e63m0_22gamma_220cd,
    s6e63m0_22gamma_230cd,
    s6e63m0_22gamma_240cd,
    s6e63m0_22gamma_240cd,// 24
};

const unsigned short *p19Gamma_set[] = {
    NULL,// display off
	s6e63m0_19gamma_30cd,// 1 (dim)
	s6e63m0_19gamma_40cd,
	s6e63m0_19gamma_50cd,
	s6e63m0_19gamma_60cd,
	s6e63m0_19gamma_70cd,// 5
	s6e63m0_19gamma_80cd,
	s6e63m0_19gamma_90cd,
	s6e63m0_19gamma_100cd,
	s6e63m0_19gamma_110cd,
	s6e63m0_19gamma_120cd,// 10
	s6e63m0_19gamma_130cd,
	s6e63m0_19gamma_140cd,
	s6e63m0_19gamma_150cd,
	s6e63m0_19gamma_160cd,
	s6e63m0_19gamma_170cd,// 15// default
	s6e63m0_19gamma_170cd,
	s6e63m0_19gamma_180cd,
	s6e63m0_19gamma_190cd,
	s6e63m0_19gamma_200cd,
	s6e63m0_19gamma_210cd,// 20
	s6e63m0_19gamma_220cd,
	s6e63m0_19gamma_230cd,
	s6e63m0_19gamma_240cd,
	s6e63m0_19gamma_240cd,// 24
};

#ifdef CONFIG_USES_ACL
static const unsigned short *ACL_cutoff_set[] = {
    acl_cutoff_off, //0
    acl_cutoff_8p,
    acl_cutoff_14p,
    acl_cutoff_20p,
    acl_cutoff_24p,
    acl_cutoff_28p, //5
    acl_cutoff_32p,
    acl_cutoff_35p,
    acl_cutoff_37p,
    acl_cutoff_40p, //9
    acl_cutoff_45p, //10
    acl_cutoff_47p, //11
    acl_cutoff_48p, //12
    acl_cutoff_50p, //13
    acl_cutoff_60p, //14
    acl_cutoff_75p, //15
    acl_cutoff_43p, //16
};
#endif /* CONFIG_USES_ACL */

/*
struct brightness_level brt_table[] = {
    { 0, 5 },// Off
    { 20, 1 },// Dimming pulse
    { MIN_BRIGHTNESS_VALUE, 2 },// Min
    { 42, 3},
    { 54, 4},
    { 65, 5},
    { 76, 6},
    { 87, 7},
    { 98, 8},
    { 109, 9},
    { 120, 10},
    { 130, 11 },
    { 140, 12 },
    { 150, 13 },
    { 160, 14 },
    { 170, 15 }, // default
    { 180, 16 },
    { 190, 17 },
    { 200, 18 },
    { 210, 19 },
    { 219, 20 },
    { 228, 21 },
    { 237, 22 },
    { 246, 23 },
    { MAX_BRIGHTNESS_VALUE, 24 },// Max
};
*/
struct brightness_level brt_table[] = {
    { 0, 5 },// Off
    { DIM_BL, 1 },// Dimming pulse
    { MIN_BRIGHTNESS_VALUE, 2 },// Min
    { 45, 3},
    { 54, 4},
    { 65, 5},
    { 76, 6},
    { 87, 7},
    { 98, 8},
    { 109, 9},
    { 120, 10},
    { 130, 11 },
    { 140, 12 },
    { 150, 13 },
    { 160, 14 },
    { 170, 15 }, // default
    { 180, 16 },
    { 190, 17 },
    { 200, 18 },
    { 210, 19 },
    { 215, 20 },
    { 220, 21 },
    { 225, 22 },
    { 230, 23 },
    { MAX_BRIGHTNESS_VALUE, 24 },// Max
};
static struct s6e63m0_state_type s6e63m0_state = {
    .disp_initialized = FALSE,
    .display_on = FALSE,
    .disp_powered_up = FALSE,
};

static int lcdc_s6e63m0_get_ldi_state(void)
{
    return ldi_enable;
}

static void lcdc_s6e63m0_set_ldi_state(int OnOff)
{
    ldi_enable = OnOff;
}


#define LCD_SDI_DATA	gpio_get_value(spi_sdi)
#define LCD_SDO_DATA	gpio_get_value(spi_sdo)
#define LCD_SDO_SWITCH_INPUT		gpio_direction_input(spi_sdo);

#define LCD_SDI_SWITCH_INPUT			gpio_direction_input(spi_sdi);
#define LCD_SDI_SWITCH_OUTPUT_LOW	gpio_direction_output(spi_sdi, 0);
#define LCD_SDI_SWITCH_OUTPUT_HIGH	gpio_direction_output(spi_sdi, 1);

static void read_ldi_register(unsigned char addr, unsigned char *buf, int count)
{
 long i, j;

 LCD_CSX_HIGH
 udelay(DEFAULT_USLEEP);
 LCD_SCL_HIGH 
 udelay(DEFAULT_USLEEP);

 /* Write Command */
 LCD_CSX_LOW
 udelay(DEFAULT_USLEEP);
 LCD_SCL_LOW 
 udelay(DEFAULT_USLEEP);  
 LCD_SDI_LOW 
 udelay(DEFAULT_USLEEP);
 
 LCD_SCL_HIGH 
 udelay(DEFAULT_USLEEP); 

    for (i = 7; i >= 0; i--) { 
  LCD_SCL_LOW
  udelay(DEFAULT_USLEEP);
  if ((addr >> i) & 0x1)
   LCD_SDI_HIGH
  else
   LCD_SDI_LOW
  udelay(DEFAULT_USLEEP); 
  LCD_SCL_HIGH
  udelay(DEFAULT_USLEEP); 
 }

 //swith input
 LCD_SDI_SWITCH_INPUT

 if(count>1)
  {
  //dummy clock cycle
  LCD_SCL_LOW
  udelay(DEFAULT_USLEEP);  
  LCD_SCL_HIGH
  udelay(DEFAULT_USLEEP); 
  }

 /* Read Parameter */
 if (count > 0) {
  for (j = 0; j < count; j++) {

   for (i = 7; i >= 0; i--) { 
    LCD_SCL_LOW
    udelay(DEFAULT_USLEEP); 
    // read bit
    if(LCD_SDI_DATA)
     buf[j] |= (0x1<<i);
    else
     buf[j] &= ~(0x1<<i);
    LCD_SCL_HIGH
    udelay(DEFAULT_USLEEP); 
   }

  }
 }

 LCD_CSX_HIGH
 udelay(DEFAULT_USLEEP); 

 //switch output
 LCD_SDI_SWITCH_OUTPUT_LOW
}





static void lcdc_s6e63m0_write_no_spinlock(struct setting_table *table)
{
    long i, j;
    
    LCD_CSX_HIGH
    udelay(DEFAULT_USLEEP);
    LCD_SCL_HIGH 
    udelay(DEFAULT_USLEEP);

    /* Write Command */
    LCD_CSX_LOW
    udelay(DEFAULT_USLEEP);
    LCD_SCL_LOW
    udelay(DEFAULT_USLEEP);
    LCD_SDI_LOW
    udelay(DEFAULT_USLEEP);
    
    LCD_SCL_HIGH
    udelay(DEFAULT_USLEEP);

    for (i = 7; i >= 0; i--) {
        LCD_SCL_LOW
        udelay(DEFAULT_USLEEP);
        if ((table->command >> i) & 0x1)
            LCD_SDI_HIGH
        else
            LCD_SDI_LOW
        udelay(DEFAULT_USLEEP);
        LCD_SCL_HIGH
        udelay(DEFAULT_USLEEP);
    }

    LCD_CSX_HIGH
    udelay(DEFAULT_USLEEP);

    /* Write Parameter */
    if ((table->parameters) > 0) {
        for (j = 0; j < table->parameters; j++) {
            LCD_CSX_LOW 
            udelay(DEFAULT_USLEEP);
            
            LCD_SCL_LOW
            udelay(DEFAULT_USLEEP);
            LCD_SDI_HIGH
            udelay(DEFAULT_USLEEP);
            LCD_SCL_HIGH
            udelay(DEFAULT_USLEEP);

            for (i = 7; i >= 0; i--) {
                LCD_SCL_LOW
                udelay(DEFAULT_USLEEP);
                if ((table->parameter[j] >> i) & 0x1)
                    LCD_SDI_HIGH
                else
                    LCD_SDI_LOW
                udelay(DEFAULT_USLEEP);
                LCD_SCL_HIGH
                udelay(DEFAULT_USLEEP);
            }

            LCD_CSX_HIGH
            udelay(DEFAULT_USLEEP);
        }
    }
    mdelay(table->wait);
}

static void lcdc_s6e63m0_write(struct setting_table *table)
{
    long i, j;
    unsigned long irqflags;

    spin_lock_irqsave(&lcd_ctrl_irq_lock, irqflags);
    LCD_CSX_HIGH
    udelay(DEFAULT_USLEEP);
    LCD_SCL_HIGH 
    udelay(DEFAULT_USLEEP);

    /* Write Command */
    LCD_CSX_LOW
    udelay(DEFAULT_USLEEP);
    LCD_SCL_LOW 
    udelay(DEFAULT_USLEEP);
    LCD_SDI_LOW 
    udelay(DEFAULT_USLEEP);
    
    LCD_SCL_HIGH 
    udelay(DEFAULT_USLEEP); 

       for (i = 7; i >= 0; i--) {
        LCD_SCL_LOW
        udelay(DEFAULT_USLEEP);
        if ((table->command >> i) & 0x1)
            LCD_SDI_HIGH
        else
            LCD_SDI_LOW
        udelay(DEFAULT_USLEEP);
        LCD_SCL_HIGH
        udelay(DEFAULT_USLEEP);
    }

    LCD_CSX_HIGH
    udelay(DEFAULT_USLEEP);

    /* Write Parameter */
    if ((table->parameters) > 0) {
        for (j = 0; j < table->parameters; j++) {
            LCD_CSX_LOW 
            udelay(DEFAULT_USLEEP);
            
            LCD_SCL_LOW 
            udelay(DEFAULT_USLEEP);
            LCD_SDI_HIGH 
            udelay(DEFAULT_USLEEP);
            LCD_SCL_HIGH 
            udelay(DEFAULT_USLEEP);

            for (i = 7; i >= 0; i--) {
                LCD_SCL_LOW
                udelay(DEFAULT_USLEEP);
                if ((table->parameter[j] >> i) & 0x1)
                    LCD_SDI_HIGH
                else
                    LCD_SDI_LOW
                udelay(DEFAULT_USLEEP);
                LCD_SCL_HIGH
                udelay(DEFAULT_USLEEP);
            }

            LCD_CSX_HIGH
            udelay(DEFAULT_USLEEP);
        }
    }
    spin_unlock_irqrestore(&lcd_ctrl_irq_lock, irqflags);
    mdelay(table->wait);
}


#ifdef SMART_DIMMING

#define READ_DCX	128	//H-SYNC
#define READ_RDX	129	//LCD_EN
#define READ_CSX	26  //CS
#define READ_WRX	30  //SCL

#define READ_DCX_HIGH    gpio_set_value(READ_DCX, 1);
#define READ_RDX_HIGH	 gpio_set_value(READ_RDX, 1);
#define READ_CSX_HIGH	 gpio_set_value(READ_CSX, 1);
#define READ_WRX_HIGH	 gpio_set_value(READ_WRX, 1); 

#define READ_DCX_LOW     gpio_set_value(READ_DCX, 0);
#define READ_RDX_LOW	 gpio_set_value(READ_RDX, 0);
#define READ_CSX_LOW	 gpio_set_value(READ_CSX, 0);
#define READ_WRX_LOW	 gpio_set_value(READ_WRX, 0); 

static int lcd_gpio[8] = {126, 125, 118, 117, 116, 115, 114, 113};
static int lcd_control[4] = {26,129,128,30};//113~126 : gpio 0~7




static void s6e63m0_mpu_read(u8 cmd, u8 *data, size_t len)
{
	int i;
	char d;
	int delay = 10;

	READ_CSX_LOW
	READ_WRX_HIGH
	READ_RDX_HIGH
	READ_DCX_LOW
	for(i = 0 ; i < 8 ; i++)gpio_tlmm_config(GPIO_CFG(lcd_gpio[i],  0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_6MA),GPIO_ENABLE);

	READ_DCX_LOW
	udelay(delay);
	READ_WRX_LOW

	for(i = 0 ; i < 8 ; i++)gpio_set_value(lcd_gpio[i], (cmd >> i) & 1); 
	

	udelay(delay);
	READ_WRX_HIGH
	udelay(delay);
	READ_DCX_HIGH
	
	for (i = 0; i < 8; i++) {
		gpio_tlmm_config(GPIO_CFG(lcd_gpio[i],  0, GPIO_INPUT, GPIO_NO_PULL, GPIO_6MA),GPIO_ENABLE);
	}
		
	udelay(delay);
	READ_RDX_LOW
	udelay(delay);
	READ_RDX_HIGH
	udelay(delay);

	while (len--) {
		d = 0;
		READ_RDX_LOW
		udelay(delay);
		for (i = 0; i < 8; i++)
			d |= gpio_get_value(lcd_gpio[i]) << i;
		*data++ = d;
		READ_RDX_HIGH
		udelay(delay);
	}
	READ_RDX_HIGH
	READ_CSX_HIGH	
}


#define LCDC_MUX_CTL_PHYS (0xa9000278)
static void lcdc_read_mtp(unsigned char value)
{
    static	void __iomem *lcdc_mux_ctl_ptr = 0;
    uint32_t lcdc_mux_ctl_cfg = 0;
    
    lcdc_mux_ctl_ptr = ioremap_nocache(LCDC_MUX_CTL_PHYS, sizeof(uint32_t));
    if(!lcdc_mux_ctl_ptr){
        DPRINT("[%s] null pointer error!!!\n", __func__);
        return;
    }
    lcdc_mux_ctl_cfg = readl_relaxed(lcdc_mux_ctl_ptr);
    DPRINT("[%s] [%x] = %x\n", __func__, lcdc_mux_ctl_ptr, lcdc_mux_ctl_cfg);
    writel_relaxed(value, lcdc_mux_ctl_ptr);
    mdelay(100);

}


 static void ldi_mpu_read_mode(void)
{
	int i, j = 0;
	int ret = 0;
	int rc = 0;

	for(i = 0 ; i<  7; i++)
	{  lcdc_s6e63m0_write(&mtp_offset_read[i]); }

	lcdc_read_mtp(0x00);
	s6e63m0_mpu_read(mtp_read_data.command, mtp_read_data.parameter,mtp_read_data.parameters);
	lcdc_read_mtp(0x01);
	
	printk("MTP READ DATA[21] = { ");
	for(i = 0 ; i < mtp_read_data.parameters ; i++)printk("%x, ",mtp_read_data.parameter[i]);

      //  lcdc init
        gpio_set_value(lcd_reset, 1);
        mdelay(10);
        gpio_set_value(lcd_reset, 0);
        mdelay(20);
        gpio_set_value(lcd_reset, 1);
        mdelay(10);
	 s6e63m0_state.disp_powered_up=1;
	 s6e63m0_state.display_on = 0;
	 lcdc_s6e63m0_disp_on();
	

 }

#endif

static void lcdc_s6e63m0_spi_init(void)
{
    /* Setting the Default GPIO's */
    spi_sclk = *(lcdc_s6e63m0_pdata->gpio_num);
    spi_cs   = *(lcdc_s6e63m0_pdata->gpio_num + 1);
    spi_sdi  = *(lcdc_s6e63m0_pdata->gpio_num + 2);
    lcd_reset= *(lcdc_s6e63m0_pdata->gpio_num + 3);

    /* Set the output so that we dont disturb the slave device */
    gpio_set_value(spi_sclk, 0);
    gpio_set_value(spi_sdi, 0);

    /* Set the Chip Select De-asserted */
    gpio_set_value(spi_cs, 0);

}

static void lcdc_s6e63m0_vreg_config(int on)
{
	int rc;
	struct vreg *vreg_lcd =NULL;

    DPRINT("start %s\n", __func__);
	if (vreg_lcd == NULL) {
		vreg_lcd = vreg_get(NULL, "vlcd");

		if (IS_ERR(vreg_lcd)) {
			printk(KERN_ERR "%s: vreg_get(%s) failed (%ld)\n",
				__func__, "vlcd", PTR_ERR(vreg_lcd));
			return;
		}

		rc = vreg_set_level(vreg_lcd, 3000);
		if (rc) {
			printk(KERN_ERR "%s: LCD set_level failed (%d)\n",
				__func__, rc);
		}
	}

	if (on) {
		rc = vreg_enable(vreg_lcd);
		if (rc) {
			printk(KERN_ERR "%s: LCD enable failed (%d)\n",
				 __func__, rc);
		}
	} else {
		rc = vreg_disable(vreg_lcd);
		if (rc) {
			printk(KERN_ERR "%s: LCD disable failed (%d)\n",
				 __func__, rc);
		}
	}
}

static void lcdc_s6e63m0_disp_powerup(void)
{
    DPRINT("start %s\n", __func__);

    if (!s6e63m0_state.disp_powered_up && !s6e63m0_state.display_on) {
        /* Reset the hardware first */
        lcdc_s6e63m0_vreg_config(VREG_ENABLE);
        //TODO: turn on ldo
        gpio_tlmm_config(GPIO_CFG(lcd_reset, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);

        //LCD_RESET_N_HI
        gpio_set_value(lcd_reset, 1);
        mdelay(10);
        //LCD_RESET_N_LO
        gpio_set_value(lcd_reset, 0);
        mdelay(20);
        //LCD_RESET_N_HI
        gpio_set_value(lcd_reset, 1);
        mdelay(10);

        /* Include DAC power up implementation here */
        s6e63m0_state.disp_powered_up = TRUE;
    }
}

static void lcdc_s6e63m0_disp_powerdown(void)
{
    DPRINT("start %s\n", __func__);

    /* Reset Assert */
    gpio_tlmm_config(GPIO_CFG(lcd_reset, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
    gpio_set_value(lcd_reset, 0);        
    lcdc_s6e63m0_vreg_config(VREG_DISABLE);
    mdelay(10);        /* ensure power is stable */

    /* turn off LDO */
    //TODO: turn off LDO

    s6e63m0_state.disp_powered_up = FALSE;
}

static void lcdc_s6e63m0_disp_on(void)
{
    int i;
    DPRINT("start %s\n", __func__);

    if (s6e63m0_state.disp_powered_up && !s6e63m0_state.display_on) {
        //mdelay(20);
        S6E63M0_WRITE_LIST(power_on_sequence);
        s6e63m0_state.display_on = TRUE;
    }
}

static int lcdc_s6e63m0_get_gamma_value_from_bl(int bl)
{
    int gamma_value = 0;
    int gamma_val_x10 = 0;

    if(bl >= MIN_BL) {
        gamma_val_x10 = 10*(MAX_GAMMA_VALUE-1)*bl/(MAX_BL-MIN_BL) + (10 - 10*(MAX_GAMMA_VALUE-1)*(MIN_BL)/(MAX_BL-MIN_BL));
        gamma_value = (gamma_val_x10+5)/10;
    }else{
        gamma_value = 0;
    }

    return gamma_value;
}

#ifdef CONFIG_USES_ACL
static void lcdc_s6e63m0_set_acl_parameter(int gamma)
{
    if(acl_enable)
    {
        if((cur_acl == 0) && (gamma != 1)){
            S6E63M0_WRITE_LIST(acl_cutoff_init);
            mdelay(20);
        }
        
        switch (gamma){
        case 0 ... 2: /* 30cd ~ 50cd */
            if (cur_acl != 0){
                S6E63M0_WRITE_LIST(acl_cutoff_off); //set 0% ACL
                cur_acl = 0;
                DPRINT("ACL_cutoff_set Percentage : 0!!\n");
            }
            break;
        case 3 ... 13: /* 70cd ~ 180cd */
            if (cur_acl != 40){
                S6E63M0_WRITE_LIST(acl_cutoff_40p);
                cur_acl = 40;
                DPRINT("ACL_cutoff_set Percentage : 40!!\n");
            }
            break;
        case 14: /* 190cd */
            if (cur_acl != 43){
                S6E63M0_WRITE_LIST(acl_cutoff_40p);
                cur_acl = 43;
                DPRINT("ACL_cutoff_set Percentage : 43!!\n");
            }
            break;
        case 15: /* 200cd */
            if (cur_acl != 45){
                S6E63M0_WRITE_LIST(acl_cutoff_40p);
                cur_acl = 45;
                DPRINT("ACL_cutoff_set Percentage : 45!!\n");
            }
            break;
        case 16: /* 210cd */
            if (cur_acl != 47){
                S6E63M0_WRITE_LIST(acl_cutoff_40p);
                cur_acl = 47;
                DPRINT("ACL_cutoff_set Percentage : 47!!\n");
            }
            break;
        case 17: /* 220cd */
            if (cur_acl != 48){
                S6E63M0_WRITE_LIST(acl_cutoff_40p);
                cur_acl = 48;
                DPRINT("ACL_cutoff_set Percentage : 48!!\n");
            }
            break;
        default:
            if (cur_acl !=50){
                S6E63M0_WRITE_LIST(acl_cutoff_40p);
                cur_acl = 50;
                DPRINT("ACL_cutoff_set Percentage : 50!!\n");
            }
            break;
        }
    }
    else
    {
        if(cur_acl != 0)
        {
            S6E63M0_WRITE_LIST(acl_cutoff_off); //set 0% ACL
            cur_acl = 0;
            DPRINT("ACL_cutoff_set Percentage : 0!!\n");
        }
    }
    return;
}

static ssize_t aclset_file_cmd_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    DPRINT("called %s \n",__func__);
    return sprintf(buf,"%u\n", acl_enable);
}

static ssize_t aclset_file_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int value, i, gamma;
    sscanf(buf, "%d", &value);
    DPRINT("in aclset_file_cmd_store, input value = %d \n", value);

    if (!lcdc_s6e63m0_get_ldi_state()){
        DPRINT("return because LDI is disabled, input value = %d \n", value);
        return size;
    }

    if ((value != 0) && (value != 1)){
        DPRINT("aclset_file_cmd_store value is same : value(%d)\n", value);
        return size;
    }

    if (acl_enable != value){
        acl_enable = value;
        lcdc_s6e63m0_set_acl_parameter(current_gamma_level);
	}

    return size;
}
#endif

#ifdef CONFIG_USES_SMART_ELVSS
unsigned char read_Smart_Dynamic_ELVSS()
{
	unsigned char buf=0;
	read_ldi_register(0xdc, &buf, 1);
	printk("read id3 = %02x\n", buf);
	return buf;
}

static void lcdc_s6e63m0_set_elvss(int gamma)
{
    int elvss_idx = 0;
    unsigned char offset_value = 0x00;
	
    struct setting_table  s6e63m0_elvss_id3= 
    { 0xB2,    4, 
        { 0x00,},
    0 };
    
    switch(gamma)
    {
    case 0 ... 5: // ~ 100cd 
        elvss_idx = 0;
	 offset_value = 0x0d;
        break;
    case 6 ... 11: // 110cd ~ 160cd
        elvss_idx = 1;
	 offset_value = 0x09;		
        break;
    case 12 ... 15: // 170cd ~ 200cd
        elvss_idx = 2;
	 offset_value = 0x07;		
        break;
    default: // 210cd ~
        elvss_idx = 3;
        break;
    }

	if(id3 == 0x00)
	{
		if(current_elvss_level != elvss_idx)
		{
		    lcdc_s6e63m0_write(&s6e63m0_elvss_set[elvss_idx]);
		    //lcdc_s6e63m0_write(&s6e63m0_elvss_on);
		    
		    DPRINT("%s (elvss_idx: %d)\n", __func__, elvss_idx);
	   	    current_elvss_level = elvss_idx;
		}
	}
	else	{
		id3 = id3 & 0x3f;
		if(id3 > 0x28) id3 = 0x28;
		s6e63m0_elvss_id3.parameter[0] = id3+offset_value;
		s6e63m0_elvss_id3.parameter[1] = id3+offset_value;
		s6e63m0_elvss_id3.parameter[2] = id3+offset_value;
		s6e63m0_elvss_id3.parameter[3] = id3+offset_value;
		 lcdc_s6e63m0_write(&s6e63m0_elvss_id3);
	}
}
#endif

/*
static void lcdc_s6e63m0_set_brightness(int level)
{
    if(level>0 && current_gamma_level!=level)
    {
        if(on_19gamma)
            lcdc_s6e63m0_write(p19Gamma_set[level]);
        else
            lcdc_s6e63m0_write(p22Gamma_set[level]);

        lcdc_s6e63m0_write(gamma_update);

        current_gamma_level = level;
        DPRINT("brightness: %d on_19gamma: %d\n",level,on_19gamma);
    }
    //lcdc_s6e63m0_write(display_on_seq);
#ifdef CONFIG_USES_SMART_ELVSS
    lcdc_s6e63m0_set_elvss(level);
#endif
#ifdef CONFIG_USES_ACL
    lcdc_s6e63m0_set_acl_parameter(level);
#endif
}*/

static ssize_t s6e63m0_show_power(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", s6e63m0_state.display_on );
}

static ssize_t s6e63m0_store_power(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
#if 1
    char *endp;
    int power = simple_strtoul(buf, &endp, 0);    

    DPRINT("s6e63m0_store_power is called: %d", power);

    if (power == 1)
        lcdc_s6e63m0_panel_on(dev);
    else if(power == 0)
        lcdc_s6e63m0_panel_off(dev);
    else if(power == 2){
        lcdc_s6e63m0_write(disp_on_sequence);
//        lcdc_s6e63m0_write(p22Gamma_set[20]);//brt_table[1].platform_level
        s6e63m0_adb_brightness_update(lcd_temp, brt_table[20].platform_level, 1);	
        lcdc_s6e63m0_write(gamma_update);
    }

#endif
    return 0;
}

static ssize_t s6e63m0_show_lcd_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    int count = 0;
    
    if(lcd_type == 0)
        count = sprintf(buf, "SMD_S6E63M0\n");
        
    return count;
}

#ifdef GAMMASET_CONTROL //for 1.9/2.2 gamma control from platform
static ssize_t gammaset_file_cmd_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	DPRINT("called %s \n",__func__);

	return sprintf(buf,"%u\n", current_gamma_level);
}
static ssize_t gammaset_file_cmd_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	
    sscanf(buf, "%d", &value);

	DPRINT("[gamma set] in gammaset_file_cmd_store, input value = %d \n",value);
	if (!lcdc_s6e63m0_get_ldi_state())	{
		DPRINT("[gamma set] return because LDI is disabled, input value = %d \n", value);
		return size;
	}

	if ((value != 0) && (value != 1))	{
		DPRINT("\ngammaset_file_cmd_store value(%d) on_19gamma(%d) \n", value,on_19gamma);
		return size;
	}

	if (value != on_19gamma)	{
		on_19gamma = value;
		//lcdc_s6e63m0_set_brightness(current_gamma_level);
		s6e63m0_adb_brightness_update(lcd_temp, brt_table[current_gamma_level].platform_level, 1);	
	}

	return size;
}
#endif

#ifdef SMART_DIMMING

static int s6e63m0_update_brightness(struct lcd_info *lcd, u32 brightness)
{
	int ret = 0;
	int i = 0;
	struct setting_table gamma_reg;

	gamma_reg.command = 0xFA;
	gamma_reg.parameters = 22;
	gamma_reg.wait = 0;

	for(i = 0; i < 35 ; i++)gamma_reg.parameter[i] = 0;
	gamma_reg.parameter[0] = 0x02;	

	calc_gamma_table(&lcd->smart, brightness, (gamma_reg.parameter)+1,NULL);

	lcdc_s6e63m0_write(&gamma_reg);
     	lcdc_s6e63m0_write(gamma_update);

	return ret;
}


static int s6e63m0_adb_brightness_update(struct lcd_info *lcd, u32 br, u32 force)
{
	u32 gamma;
	int ret = 0;
	int i;


       for(i = 0; i < MAX_BRT_STAGE; i++) {
             if(br <= brt_table[i].platform_level ) {
                 lcd->bl = brt_table[i].driver_level; //24
		   gamma = brt_table[i].platform_level; //255
                 break;
             		}
       	}
	
	//	if(lcd->bl >19) lcd->bl = 19; 
	/* printk("%s was called\n",__func__); */
	/* printk("ldi_enable : %d\n",lcd->ldi_enable); */
	 printk("current_bl : %d, lcd->bl : %d br : %d\n",lcd->current_bl, lcd->bl, gamma); 

	if ((force) || ((lcd->ldi_enable) && (lcd->current_bl != lcd->bl))) 
	{
	 
		s6e63m0_update_brightness(lcd, gamma);

#ifdef CONFIG_USES_SMART_ELVSS
		lcdc_s6e63m0_set_elvss( lcd->bl);
#endif
#ifdef CONFIG_USES_ACL
		lcdc_s6e63m0_set_acl_parameter( lcd->bl);
#endif
//		lcd->current_bl = lcd->bl;
	       current_gamma_level = lcd->current_bl = lcd->bl;
	}
	return ret;
}

#endif



static int lcdc_s6e63m0_panel_on(struct platform_device *pdev)
{
    int i,j=0;

   	;
    struct lcd_info *lcd;
	int result_read_register = 0;
    DPRINT("start %s\n", __func__);

    if (!s6e63m0_state.disp_initialized) {
        /* Configure reset GPIO that drives DAC */
        lcdc_s6e63m0_pdata->panel_config_gpio(1);
        lcdc_s6e63m0_spi_init();    /* LCD needs SPI */
        lcdc_s6e63m0_disp_powerup();
        lcdc_s6e63m0_disp_on();
 	 id3 = read_Smart_Dynamic_ELVSS();		
#ifdef LCD_DET_ENABLE
        if (irq_disabled) 
        {      
            enable_irq ( MSM_GPIO_TO_INT ( GPIO_LCD_DET ) );
            irq_disabled = FALSE;
            printk ( "@@@@@     %s - irq-disabled is changed to %d\n", __func__, irq_disabled );
        }
#endif
        s6e63m0_state.disp_initialized = TRUE;
    }

    if(!First_Disp_Power_On)
    {
    
#ifdef SMART_DIMMING
		ldi_mpu_read_mode();

		lcd = lcd_temp = kzalloc(sizeof(struct lcd_info), GFP_KERNEL);
		
		lcd->max_brightness = MAX_BRIGHTNESS;
		lcd->brightness = DEFAULT_BRIGHTNESS;
		lcd->bl = DEFAULT_GAMMA_LEVEL;
		lcd->current_bl = lcd->bl;
		lcd->acl_enable = 0;
		lcd->cur_acl = 0;
		lcd->power = FB_BLANK_UNBLANK;
		lcd->ldi_enable = 1;

		init_table_info(&lcd->smart,NULL);	 //lcd->lcd_id); //default gamma 300
		calc_voltage_table(&lcd->smart, mtp_read_data.parameter, NULL);

#endif
	
        First_Disp_Power_On = TRUE;
	 s6e63m0_adb_brightness_update(lcd, lcd->brightness, 1);	
    }

    if(delayed_backlight_value != -1) {
	 s6e63m0_adb_brightness_update(lcd_temp, delayed_backlight_value, 1);	      
        DPRINT("delayed backlight on %d\n", delayed_backlight_value);
    }
    lcd_on_state_for_debug = 1;

    return 0;
}

static int lcdc_s6e63m0_panel_off(struct platform_device *pdev)
{
    int i;
    unsigned long irqflags;

    DPRINT("start %s\n", __func__);

    if (s6e63m0_state.disp_powered_up && s6e63m0_state.display_on) {
        s6e63m0_state.display_on = FALSE;
        s6e63m0_state.disp_initialized = FALSE;
#ifdef LCD_DET_ENABLE
        disable_irq_nosync ( MSM_GPIO_TO_INT ( GPIO_LCD_DET ) );
        irq_disabled = TRUE;
        printk ( "@@@@@     %s - irq-disabled is changed to %d\n", __func__, irq_disabled );
#endif
        if (!lcdc_s6e63m0_get_ldi_state()) {
            spin_lock_irqsave(&lcd_ctrl_irq_lock, irqflags);
            for (i = 0; i < POWER_OFF_SEQ; i++) {
                lcdc_s6e63m0_write_no_spinlock(&power_off_sequence[i]);
            }
            spin_unlock_irqrestore(&lcd_ctrl_irq_lock, irqflags);
        } else {
            for (i = 0; i < POWER_OFF_SEQ; i++) {
                lcdc_s6e63m0_write(&power_off_sequence[i]);
            }
        }
        lcdc_s6e63m0_pdata->panel_config_gpio(0);
        lcdc_s6e63m0_disp_powerdown();
    }

    lcd_on_state_for_debug = 0;
    current_gamma_level = -1;
    lcd_temp->current_bl = 0;
#ifdef CONFIG_USES_SMART_ELVSS
    current_elvss_level = -1;
#endif
    return 0;
}

static void lcdc_s6e63m0_set_backlight(struct msm_fb_data_type *mfd)
{    
    int bl_level = mfd->bl_level;
    int gamma_level = 0;
    int brightness;
    int i;

#if 1
    if(bl_level > 0){
        if(bl_level < DIM_BL) { //  < 30
            // dimming set
            gamma_level = brt_table[1].driver_level;  //27
	     brightness = brt_table[1].platform_level ;	//255
        } 
         else if(bl_level < MIN_BRIGHTNESS_VALUE) { // < 40
            gamma_level = brt_table[2].driver_level;  
	     brightness = brt_table[2].platform_level ;	
         	}		 	
	  else if (bl_level >= MAX_BRIGHTNESS_VALUE) {
            // max brightness set 
            gamma_level = brt_table[MAX_BRT_STAGE-1].driver_level;
	     brightness = brt_table[MAX_BRT_STAGE-1].platform_level ;
        } 
	  else {
            for(i = 0; i < MAX_BRT_STAGE; i++) {
                if(bl_level <= brt_table[i].platform_level ) {
                    gamma_level = brt_table[i].driver_level;
		      brightness = brt_table[i].platform_level ;
                    break;
                }
            }
        }
    } else {
        lcdc_s6e63m0_write(disp_off_sequence);
	 lcd_temp->current_bl = 0;
        delayed_backlight_value = -1;
        DPRINT("bl: %d \n",bl_level);
        return;
    }
#else
    gamma_level = lcdc_s6e63m0_get_gamma_value_from_bl(bl_level);
#endif

    // LCD should be turned on prior to backlight
    if(s6e63m0_state.disp_initialized == FALSE && gamma_level > 0){
      //  delayed_backlight_value = gamma_level;
        delayed_backlight_value = brightness;
        DPRINT("delayed_backlight_value = gamma_level\n");
        return;
    } else {
        delayed_backlight_value = -1;
    }

   s6e63m0_adb_brightness_update(lcd_temp, brightness, 0);
//   mutex_unlock(&lcd_temp->bl_lock);
   printk("bl: %d, gamma: %d\n",bl_level,gamma_level);
  //  lcdc_s6e63m0_set_brightness(gamma_level);
}

static int __devinit lcdc_s6e63m0_probe(struct platform_device *pdev)
{
    int err = 0;
    
    DPRINT("start %s(%d)\n", __func__, pdev->id);

    if (pdev->id == 0) {
        lcdc_s6e63m0_pdata = pdev->dev.platform_data;
        
        lcdc_s6e63m0_spi_init();
        if( !spi_sclk || !spi_cs || !spi_sdi || !lcd_reset)
        {
            DPRINT("SPI Init Error. %d,%d,%d,%d\n",spi_sclk,spi_cs,spi_sdi,lcd_reset);
            spi_cs = 26;
            spi_sclk = 30;
            spi_sdi = 57;
            lcd_reset = 22;
        }    

        /* sys fs */
        if(sec_class==NULL) sec_class = class_create(THIS_MODULE, "sec");
        if(IS_ERR(sec_class))
            pr_err("Failed to create class(sec)!\n");
     
        lcd_dev = device_create(sec_class, NULL, 0, NULL, "sec_lcd");
        if(IS_ERR(lcd_dev))
            pr_err("Failed to create device(lcd)!\n");
     
        if(device_create_file(lcd_dev, &dev_attr_lcdtype_file_cmd) < 0)
            pr_err("Failed to create device file(%s)!\n", dev_attr_lcdtype_file_cmd.attr.name);
        if(device_create_file(lcd_dev, &dev_attr_lcd_power) < 0)
            pr_err("Failed to create device file(%s)!\n", dev_attr_lcd_power.attr.name); 

        lcdc_s6e63m0_set_ldi_state(1);
#ifdef CONFIG_USES_ACL
        DPRINT("making aclset sysfile start\n");
        //        acl_class = class_create(THIS_MODULE, "aclset");
        //        if (IS_ERR(acl_class))
        //            DPRINT("Failed to create class(acl_class)!\n");
        //        switch_aclset_dev = device_create(acl_class, NULL, 0, NULL, "switch_aclset");
        switch_aclset_dev = device_create(sec_class, NULL, 0, NULL, "switch_aclset");
        
        if (IS_ERR(switch_aclset_dev))
            DPRINT("Failed to create device(switch_aclset_dev)!\n");

        if (device_create_file(switch_aclset_dev, &dev_attr_aclset_file_cmd) < 0)
            DPRINT("Failed to create device file(%s)!\n", dev_attr_aclset_file_cmd.attr.name);
#endif
#ifdef GAMMASET_CONTROL
        //	gammaset_class = class_create(THIS_MODULE, "gammaset");
        //	if (IS_ERR(gammaset_class))
        //		DPRINT("Failed to create class(gammaset_class)!\n");
        //	switch_gammaset_dev = device_create(gammaset_class, NULL, 0, NULL, "switch_gammaset");
        switch_gammaset_dev = device_create(sec_class, NULL, 0, NULL, "switch_gammaset");

        if (IS_ERR(switch_gammaset_dev))
            DPRINT("Failed to create device(switch_gammaset_dev)!\n");

        if (device_create_file(switch_gammaset_dev, &dev_attr_gammaset_file_cmd) < 0)
            DPRINT("Failed to create device file(%s)!\n", dev_attr_gammaset_file_cmd.attr.name);
#endif

#ifdef LCD_DET_ENABLE
        gpio_tlmm_config ( GPIO_CFG ( GPIO_LCD_DET,  0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA ), GPIO_CFG_ENABLE );

        err = request_irq ( MSM_GPIO_TO_INT ( GPIO_LCD_DET ), s6e63m0_esd_irq_handler, IRQF_TRIGGER_RISING, "LCD_ESD_DET", (void*)pdev->dev.platform_data );
        if (err)
            printk ( "%s, request_irq failed %d(ESD_DET), ret= %d\n", __func__, GPIO_LCD_DET, err );
        else
            printk ( "@@@@@     %s - irq is requested normally\n", __func__ );
#endif

        return 0;
    }
    msm_fb_add_device(pdev);

    return 0;
}

static void lcdc_s6e63m0_shutdown(struct platform_device *pdev)
{
    DPRINT("start %s\n", __func__);
    lcdc_s6e63m0_set_ldi_state(0);
    current_gamma_level = -1;
    //lcdc_s6e63m0_panel_off(pdev);
    gpio_set_value(lcd_reset, 0);    
}

static struct platform_driver this_driver = {
    .probe = lcdc_s6e63m0_probe,
    .shutdown = lcdc_s6e63m0_shutdown,
    .driver = {
        .name   = "lcdc_s6e63m0_wvga",
        .owner  = THIS_MODULE,
    },
};

static struct msm_fb_panel_data s6e63m0_panel_data = {
    .on = lcdc_s6e63m0_panel_on,
    .off = lcdc_s6e63m0_panel_off,
    .set_backlight = lcdc_s6e63m0_set_backlight,
};

static struct platform_device this_device = {
    .name = "lcdc_panel",
    .id = 1,
    .dev = {
        .platform_data = &s6e63m0_panel_data,
    }
};

static boolean panel_init = FALSE;
static int __init lcdc_s6e63m0_panel_init(void)
{
    int ret;
	int i, j=0;
    struct msm_panel_info *pinfo;

#ifdef CONFIG_FB_MSM_MDDI_AUTO_DETECT
    if (msm_fb_detect_client("lcdc_s6e63m0_wvga"))
    {
        DPRINT("%s: msm_fb_detect_client failed!\n", __func__);
        return 0;
    }
#endif
    DPRINT("start %s\n", __func__);
    
    ret = platform_driver_register(&this_driver);
    if (ret)
    {
        DPRINT("%s: platform_driver_register failed! ret=%d\n", __func__, ret);
        return ret;
    }

    pinfo = &s6e63m0_panel_data.panel_info;
    pinfo->xres = LCDC_FB_XRES;
    pinfo->yres = LCDC_FB_YRES;
    pinfo->type = LCDC_PANEL;
    pinfo->pdest = DISPLAY_1;
    pinfo->wait_cycle = 0;
    pinfo->bpp = 24;
    pinfo->fb_num = 2;
    pinfo->clk_rate = 24576* 1000;
    pinfo->bl_max = 255;
    pinfo->bl_min = 1;

    pinfo->lcdc.h_back_porch = LCDC_HBP;
    pinfo->lcdc.h_front_porch = LCDC_HFP;
    pinfo->lcdc.h_pulse_width = LCDC_HPW;
    pinfo->lcdc.v_back_porch = LCDC_VBP;
    pinfo->lcdc.v_front_porch = LCDC_VFP;
    pinfo->lcdc.v_pulse_width = LCDC_VPW;
    pinfo->lcdc.border_clr = 0;     /* blk */
    pinfo->lcdc.underflow_clr = 0xff0000;       /* red */
    pinfo->lcdc.hsync_skew = 0;

    DPRINT("%s\n", __func__);

    ret = platform_device_register(&this_device);
    if (ret)
    {
        DPRINT("%s: platform_device_register failed! ret=%d\n", __func__, ret);
        platform_driver_unregister(&this_driver);
    }

    panel_init = TRUE;
    return ret;
}

#ifdef LCD_DET_ENABLE
void s6e63m0_esd ( void )
{
	if ( panel_init )
	{
		printk ( "@@@@@     %s - ESD start\n", __func__ );
		++ESD_count;
		printk ( "@@@@@     %s - ESD count - %d.\n", __func__, ESD_count );
		lcdc_s6e63m0_panel_off ( NULL );
		mdelay(20);
		lcdc_s6e63m0_panel_on ( NULL );
		printk ( "@@@@@     %s - ESD end\n", __func__ );
	}
}

static DECLARE_WORK ( lcd_esd_work, s6e63m0_esd );

static irqreturn_t s6e63m0_esd_irq_handler(int irq, void *handle)
{
	printk ( "@@@@@     %s - IRQ (bl = %d)\n", __func__ , current_gamma_level);

    // lcd off 시 ESD IRQ 발생함 > current_gamma_level을 사용하여 임시로 막자.
	if ( s6e63m0_state.disp_initialized && current_gamma_level>0 )
	{
		schedule_work ( &lcd_esd_work );
	}

	return IRQ_HANDLED;
}
#endif

module_init(lcdc_s6e63m0_panel_init);


