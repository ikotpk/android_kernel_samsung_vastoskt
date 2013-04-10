
//********************************************************************************
//*
//*	File name	: NTS1200_1308_Write.c
//*
//*	Author		: Sahyang Hong <shhong95@nextchip.com>
//*
//*	Date		: 20. 05. 2011
//*
//*	Revision	: 1.00
//*
//*	Description	:
//*
//********************************************************************************
//*	Revision History
//********************************************************************************
//*	Rev		Date		Author			Description.
//*
//*	1.00	20/05/2011	Sahyang Hong	Initial Release
//*
//********************************************************************************
/*


  이 파일은 NTS1200, NTS1300 Series의 Firmware write의 이해를 돕기 위해 만들어진 참고용 Source입니다.

  MCU는 Silicon Labs의 C8051F340을 기준으로 하고 있습니다.

  * IIC SPEED : 400KHZ

  * GPIO Pin Define

    TEST Pin    : P1.3
    nRESET Pin  : P1.2
    SCL Pin     : P0.1
    SDA Pin     : P0.0



  * Low level 함수 설명

  > char SMB_Write(unsigned int addr, unsigned char leng, unsigned char *Write_data_in);

    - I2C 포트를 통해 해당 어드레스에 데이터를 Write하는 함수
    - Return Value
      Success : 1
      Fail    : 0

        <** Device Address = 0xE0 **>
        1.	START condition
        2.	Device Address (7 bits) + Significant bit (1 bit)  check the ACK bit
        3.	Slave Address (16 bits) : addr  check the ACK bit
        4.	Write DATA (1 byte)  check the ACK bit <= len수 만큼 실행
        5.	STOP condition


  > char SMB_Read(unsigned int addr, unsigned char leng);

    - I2C 포트를 통해 해당 어드레스에 데이터를 Read하는 함수
    - Return Value
      Success : 1
      Fail    : 0

        <** Device Address = 0xE0 **>
        1.	START condition
        2.	Device Address (7 bits) + Significant bit (1 bit)  check the ACK bit
        3.	Slave Address (16 bits) : addr  check the ACK bit
        4.	STOP condition
        5.	START condition
        6.	Device Address (7 bits) + Significant bit (1 bit)  generate the ACK bit
        7.	Read DATA (1 byte)  Generate the ACK bit <= len수 만큼 실행
        8.	STOP condition

*/
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

#include "vasto_NTS1308U_Ver0xD6.h"

#ifdef CONFIG_KEYBOARD_NEXTCHIP_TOUCH
#include <linux/input/nextchip-touchkey.h>
#endif

#define TOUCHKEY_EEPROM 0xE0

// Define standard constants
#define TRUE   1
#define FALSE  0
#define ON     1
#define OFF    0

// Define NTS Pin controls....
//sbit NTS_RESET = P1^2;                  // NTS RESET Control on port P1.2
//sbit NTS_TEST  = P1^3;                  // NTS TEST Control on port P1.3

#define	RESET_HIGH()    gpio_set_value(KEY_RST , 1)//(NTS_RESET = 1)
#define	RESET_LOW()		gpio_set_value(KEY_RST , 0)//(NTS_RESET = 0)

#define	TEST_HIGH()		gpio_set_value(KEY_TEST , 1)//(NTS_TEST = 1)
#define	TEST_LOW()		gpio_set_value(KEY_TEST , 0)//(NTS_TEST = 0)

#define	I2C_DELAY()		udelay(100);//Delay_us(100)


// 여기에 Firmware을 넣으시면 됩니다.

/*unsigned char NTS_firmware[] = {
    0x00, 0x00,  // 첫 2바이트는 펌웨어 사이즈 입니다. (MSB, LSB)
    ...
};*/

extern unsigned char I2Cm_ReadBytes(unsigned char SlaveAdr, unsigned char *RxArray, unsigned char SubAdr0, unsigned char SubAdr1, unsigned char RxByteCount);
extern unsigned char I2Cm_WriteBytes(unsigned char SlaveAdr, unsigned char *TxArray, unsigned char SubAdr0, unsigned char SubAdr1, unsigned char TxByteCount);

void TestMode_IN(void)
{
    RESET_LOW();
    mdelay(5);//Delay_ms(5);  // 5mSec
    TEST_HIGH();
    mdelay(25);//Delay_ms(25); // 25mSec
    RESET_HIGH();
    mdelay(30);//Delay_ms(30); // 30mSec
}

void TestMode_OUT(void)
{
    RESET_LOW();
    mdelay(5);//Delay_ms(5);  // 5mSec
    TEST_LOW();
    mdelay(25);//Delay_ms(25); // 25mSec
    RESET_HIGH();
    mdelay(30);//Delay_ms(30); // 30mSec
}


int SMB_Write(unsigned int addr, unsigned char leng, unsigned char * data_in)
{
	u8 addr0,addr1;
	addr0=(u8)((addr&0xff00)>>8);
	addr1=(u8)(addr&0x00ff);
	
	if ((int)I2Cm_WriteBytes(TOUCHKEY_EEPROM ,data_in, addr0, addr1, leng)==0)
		return TRUE;
	else
		return FALSE;
}


char ROM_BYTE_Write(unsigned int addr, unsigned char leng, unsigned char Write_data_in)
{
	unsigned char data_in[1];
	int	err;

	data_in[0] = Write_data_in;
	err = SMB_Write(addr, leng, data_in);

    // Delay for IIC_SPEED_400KHZ
    udelay(38);//Delay_us(38);

	return err;
}

char ROM_BYTE_Read(unsigned int addr, unsigned char leng, unsigned char *read_data_out)
{
#if 0
	int	err;
	unsigned char	i;
//	uint8_t i2c_addr = 0xE0;
	uint8_t wdog_val[1];   

//	err = SMB_Read(addr, leng);
	err = TKey_i2c_read( addr, wdog_val, sizeof(wdog_val));

	if(err == TRUE)					// normal case
	{
		for(i=0;i<leng;i++)
//			read_data_out[i] = SMB_Data[i];
            read_data_out[i] = wdog_val[i];

	}
	else
		read_data_out[0] = 0x00;

	udelay(20);//Delay_us(20);

	return err;
#endif	
	u8 addr0,addr1;
	addr0=(u8)((addr&0xff00)>>8);
	addr1=(u8)(addr&0x00ff);
		
	if((int)I2Cm_ReadBytes(TOUCHKEY_EEPROM, read_data_out, addr0, addr1, leng)==0)
		return TRUE;
	else
		return FALSE;
}

#define EEP_SFR_I2C			0x4000
#define	PLLPROT		        0x87
#define	MCLKSEL		        0x8B

#define	EEP_WEN				0x80
#define	EEP_WENL			0x08
#define	EEP_PDOWN			0x00
#define	EEP_STNBY			0x01
#define	EEP_READ			0x02
#define	EEP_LOAD1			0x03
#define	EEP_LOAD2			0x04
#define	EEP_ERPRG			0x05

int	NTS_MOSC_Set(void)
{
	int tErr = 0;

	tErr = ROM_BYTE_Write( EEP_SFR_I2C|PLLPROT, 1, EEP_WEN | 0x7F);									// Protection Unlock
	if(tErr!= TRUE)
		return -1;

	I2C_DELAY();

	tErr = ROM_BYTE_Write( EEP_SFR_I2C|MCLKSEL, 1, EEP_WEN | 0x02);									// MOSC : Internal Div Clock, Rev0 10.65MHz
	if(tErr!= TRUE)
		return -2;
	I2C_DELAY();

	tErr = ROM_BYTE_Write( EEP_SFR_I2C|PLLPROT, 1, EEP_WEN | 0x00);									// Protection Lock
	if(tErr!= TRUE)
		return -4;
	I2C_DELAY()

	return TRUE;
}

#define	EEPPROT				0x4098
#define	EEPPAG0				0x4099
#define EEPPAG1				0x409A
#define	EEPCFG				0x409B
#define	EEPMODE				0x409C
#define	EEPCOMM				0x409D

#define	EEPRDY				0x01
#define	AUTOADR				0x02

#define	MAIN_MODE			0x00

#define	ALL_PAGE_ERASE		0x07

#define	EEP_PAGE_SIZE		32

int EEPROM_EraseAll()
{
	int	addr = 0;
	unsigned char eep_data = 0;

	if(ROM_BYTE_Write(EEPPROT, 1, 0x9F) != TRUE)		// Protection register UNLOCK!
		return -1;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_STNBY) != TRUE)		// stnby mode (added by kyi)
		return -2;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPMODE, 1, EEP_WEN | ALL_PAGE_ERASE) != TRUE)		// eeprom main mode access
		return -3;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCFG, 1, EEP_WEN | AUTOADR) != TRUE)		// eeprom main mode access
		return -3;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPPAG0, 1, (unsigned char) (EEP_WEN | 0)) != TRUE)		// eeprom main mode access
		return -4;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPPAG1, 1, (unsigned char) (EEP_WEN | (0 >> 7))) != TRUE)		// eeprom main mode access
		return -5;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_LOAD1) != TRUE)		// eeprom main mode access
		return -6;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_LOAD2) != TRUE)		// eeprom main mode access
		return -7;
	I2C_DELAY();

	if(SMB_Write(0, EEP_PAGE_SIZE, &eep_data) != TRUE)		// data write
		return -8;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_ERPRG) != TRUE)		// eeprom main mode access
		return -9;
	I2C_DELAY();

    do
    {
        if(ROM_BYTE_Read(EEPCFG, 1, &eep_data) != TRUE)		// eeprom main mode access
            return -10;
        I2C_DELAY();
    }
    while((eep_data & 0x01) != EEPRDY);


    if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_STNBY) != TRUE)		// eeprom main mode access
		return -11;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPMODE, 1, EEP_WEN | MAIN_MODE) != TRUE)		// eeprom main mode access
		return -12;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPPROT, 1, EEP_WEN | 0x00) != TRUE)		// protection register LCOK!
		return -13;
	I2C_DELAY();

	return TRUE;
}

static int EEPROM_EnterWriteMode()
{
    if(ROM_BYTE_Write(EEPPROT, 1, 0x9F) != TRUE)		// Protection register UNLOCK!
		return -1;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_STNBY) != TRUE)		// stnby mode (added by kyi)
		return -2;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCFG, 1, EEP_WEN | AUTOADR) != TRUE)	// Auto Address increase mode
		return -4;
    I2C_DELAY();

	return TRUE;
}

static int EEPROM_EnterWriteMode_write(unsigned int addr, unsigned int length, unsigned char *DATA)
{
    unsigned char	eep_data = 0;

    if(ROM_BYTE_Write(EEPPAG0, 1, (unsigned char) (EEP_WEN | addr)) != TRUE)		// page0 addr write (low addr)
		return -5;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPPAG1, 1, (unsigned char) (EEP_WEN | (addr >> 7))) != TRUE)		// page1 addr write (high addr)
		return -6;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_LOAD1) != TRUE)		// load1 command
		return -7;
	I2C_DELAY();

	if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_LOAD2) != TRUE)		// load2 command
		return -8;
	I2C_DELAY();

	if(SMB_Write(0, length, DATA) != TRUE)		// data write
		return -9;

	if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_ERPRG) != TRUE)		// erase & programming
		return -10;
	I2C_DELAY();

	do{
		if(ROM_BYTE_Read(EEPCFG, 1, &eep_data) != TRUE)				// check ready bit
			return -11;
		I2C_DELAY();
	}while((eep_data & 0x01) != EEPRDY);

    if(ROM_BYTE_Write(EEPCOMM, 1, EEP_WENL | EEP_STNBY) != TRUE)		// stnby mode
		return -12;
	I2C_DELAY();

	return TRUE;
}

void TKey_Firmware_Update(void)
{
	printk("[Touchkey] Tkey_Firmware_Update +++++\n");

    int rtn;
    unsigned int i;
    unsigned int fwsize, NumPages;
    unsigned char  *firmware_data = NTS_firmware;

    // Step 1: Enter Test mode to access EEPROM...
    TestMode_IN();

    mdelay(200);//Delay_ms(200);

    // Step 1-1: Set NTS System clock to be more faster abt 2.5Mhz.
    if (NTS_MOSC_Set()==TRUE)
    {
        rtn = EEPROM_EraseAll();
    	printk("[Touchkey] EEPROM_EraseAll - rtn : %d\n", rtn);        
        if (rtn==TRUE)
        {
            // Enter Write Mode
            rtn=EEPROM_EnterWriteMode();
        }
    	printk("[Touchkey] EEPROM_EnterWriteMode - rtn : %d\n", rtn);                

        if (rtn==TRUE)
        {
            fwsize =  *firmware_data++ << 8;
            fwsize += *firmware_data++;

        	printk("[Touchkey] FW_SIZE = 0x%x\n", fwsize);                                            
            NumPages = fwsize / EEP_PAGE_SIZE;
        	printk("[Touchkey] Num_Page = 0x%x\n", NumPages);                                                        
            for(i=0;i<NumPages;i++)
            {
                rtn = EEPROM_EnterWriteMode_write(i, EEP_PAGE_SIZE, firmware_data);
				printk("[Touchkey] Firmware write data 0x%x, size %d \n",firmware_data,EEP_PAGE_SIZE,0);	                
                firmware_data += EEP_PAGE_SIZE;
            }

            if ((rtn==TRUE) && (fwsize % EEP_PAGE_SIZE))
            {
                rtn = EEPROM_EnterWriteMode_write(i, fwsize % EEP_PAGE_SIZE, firmware_data);
				printk("[Touchkey] Firmware write data 0x%x, size %d \n",firmware_data, fwsize % EEP_PAGE_SIZE,0);	                
            }
        }

        if (rtn==TRUE)
        {
            // protection register LCOK!
            ROM_BYTE_Write(EEPMODE, 1, EEP_WEN | MAIN_MODE);
            I2C_DELAY();
            ROM_BYTE_Write(EEPPROT, 1, EEP_WEN | 0x00);
            I2C_DELAY();
        }
    }
    else
    {
        //예외처리

    }

    // Step 3: Exit Test Mode
    TestMode_OUT();

	printk("[Touchkey] Tkey_Firmware_Update -----\n");    
}
















