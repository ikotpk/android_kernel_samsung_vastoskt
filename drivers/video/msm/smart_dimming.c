/* linux/drivers/video/samsung/smartdimming.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com

 * Samsung Smart Dimming for OCTA
 *
 * Minwoo Kim, <minwoo7945.kim@samsung.com>
 *
*/


#include "smart_dimming.h"
#include "s6e63m0_volt_tbl.h"

/* Refer EXCEL Page2 named Gamma MTP Offset setting
 REF VOLTAGE 4.2 * 1024 */
#define VREG0_REG_VOLT 4301

/* Refer EXCEL Page2 named Gamma MTP Offset setting V1 ~ V15 */

//v1 - v19 offset 값
const u8 v1_offset_table[18] = {	
	75, 69, 63, 57, 51, 46, 41, 36, 31, 27, 23, 19, 15, 12, 9, 6, 3, 0 
	//49, 44, 39, 34, 29, 24, 20, 16, 12, 8, 6, 4, 2, 0
};
//v19 -v43 offset 값
const u8 v15_offset_table[24] = {
	101, 94, 87, 80, 74, 68, 62, 56, 51, 46, 41, 36, 32, 28, 24, 20, 17, 14, 11, 8, 6, 4, 2, 0
	//132, 124, 116, 108, 100, 92, 84, 76, 69, 62, 55, 48, 42, 36, 30, 24, 18, 12, 6, 0
};

//gamma 변곡 point 별 offset 개수.
const u8 range_table_count[IV_TABLE_MAX] = {
	1, 18, 24, 44, 84, 84, 1
	// 1, 14, 20, 24, 28, 84, 84, 1
};

// 32768(2^15) /  gamma MTP offset setting 분모. 
// 0, 81, 108, 44, 84, 84
/* V1 : To make => 32768 // 2^15 */
const u32 table_radio[IV_TABLE_MAX] = {
	0, 405, 303, 745, 390, 390, 0
	//0, 607, 234, 1365, 1170, 390, 390, 0
};

const u32 dv_value[IV_MAX] = {
	0, 19, 43, 87, 171, /*171,*/ 255
};

const char color_name[3] = {'R', 'G', 'B'};


const u8 *offset_table[IV_TABLE_MAX] = {
	NULL,
	v1_offset_table,
	v15_offset_table,
	NULL,
	NULL,
	NULL,
	NULL,
	//NULL
};

#if 0
const unsigned char gamma_300cd_m3_panel[]  = {
    0x59, 0x10, 0x61, 0xCF, 0xE2, 0xC2,
    0xC3, 0xD9, 0xC3, 0xCC, 0xDF, 0xC8,
    0xA4, 0xC2, 0x9D, 0xB9, 0xCD, 0xB2,
    0x00, 0x9F, 0x00, 0xA3, 0x00, 0xCD,

};

const unsigned char gamma_300cd_sm2_panel[] = {
    0x1F, 0x1F, 0x43, 0xF7, 0xEC, 0xE1,
    0xDB, 0xDF, 0xD7, 0xE0, 0xDF, 0xDA,
    0xBC, 0xB8, 0xAD, 0xC2, 0xBF, 0xB7,
    0x00, 0xBA, 0x00, 0xCB, 0x00, 0xD7,
};
#endif
/*
const unsigned char gamma_300cd_8500k[] = {

     0x18, 0x08, 0x24, 
     0x70, 0x6E, 0x4E,
     0xBC, 0xC0, 0xAF, 
     0xB3, 0xB8, 0xA5,
     0xC5, 0xC7, 0xBB,
     0x00, 0xB9, 0x00, 
     0xB8, 0x00, 0xFC//300, 8500k
  
 };

const unsigned char gamma_300cd_7500k[] = {

	0x18, 0x08, 0x24,
	0x3A, 0x48, 0x16,
	0xB0, 0xB7, 0xA4,
	0xAA, 0xB3, 0x9E,
	0xC1, 0xC3, 0xB5,
	0x00, 0xB1, 0x00,
	0xAE, 0x00, 0xF3  //300, 7500k
 };*/


const unsigned char gamma_300cd[] = {

	0x18, 0x08, 0x24,
	0x3A, 0x48, 0x16,
	0xB0, 0xB7, 0xA4,
	0xAA, 0xB3, 0x9E,
	0xC1, 0xC3, 0xB5,
	0x00, 0xB1, 0x00,
	0xAE, 0x00, 0xF3  //300, 7500k
 };
static s16 s9_to_s16(s16 v)
{
	return (s16)(v << 7) >> 7;
}


u32 calc_v1_volt(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX])
{
	u32 ret = 0;

    if(gamma >= V1_VOLTAGE_COUNT) {
        printk("[SMART:ERROR] : %s Exceed Max Gamma Value (%d)",__func__,gamma);
        gamma = V1_VOLTAGE_COUNT-1;
    }

	ret = v1_voltage_tbl[gamma] >> 10;

	return ret;
}


u32 calc_v15_volt(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX])
{
	/* for CV : 20, DV :320 */
	int ret = 0;
	u32 v1, v35;
	u32 ratio = 0;

	v1 = adjust_volt[rgb_index][AD_IV1];
	v35 = adjust_volt[rgb_index][AD_IV35];

    if(gamma >= V15_VOLTAGE_COUNT) {
        printk("[SMART:ERROR] : %s Exceed Max Gamma Value (%d)",__func__,gamma);
        gamma = V15_VOLTAGE_COUNT-1;
    }

	ratio = cv20_dv320_ratio_tbl[gamma];

	ret = (v1 << 10) - ((v1-v35)*ratio);
	ret = ret >> 10;

	return ret;
}



u32 calc_v35_volt(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX])
{
	/* for CV : 65, DV :320 */
	int ret = 0;
	u32 v1, v59;
	u32 ratio = 0;

	v1 = adjust_volt[rgb_index][AD_IV1];
	v59 = adjust_volt[rgb_index][AD_IV59];

    if(gamma >= V35_VOLTAGE_COUNT) {
        printk("[SMART:ERROR] : %s Exceed Max Gamma Value (%d)",__func__,gamma);
        gamma = V35_VOLTAGE_COUNT-1;
    }
	ratio = cv64_dv320_ratio_tbl[gamma];

	ret = (v1 << 10) - ((v1-v59)*ratio);
	ret = ret >> 10;

	return ret;
}


u32 calc_v59_volt(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX])
{
	/* for CV : 65, DV :320 */
	int ret = 0;
	u32 v1, v87;
	u32 ratio = 0;

	v1 = adjust_volt[rgb_index][AD_IV1];
	v87 = adjust_volt[rgb_index][AD_IV87];

    if(gamma >= V59_VOLTAGE_COUNT) {
        printk("[SMART:ERROR] : %s Exceed Max Gamma Value (%d)",__func__,gamma);
        gamma = V59_VOLTAGE_COUNT-1;

    }
	ratio = cv64_dv320_ratio_tbl[gamma];

	ret = (v1 << 10) - ((v1-v87)*ratio);
	ret = ret >> 10;

	return ret;
}


u32 calc_v87_volt(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX])
{
	/* for CV : 65, DV :320 */
	int ret = 0;
	u32 v1, v171;
	u32 ratio = 0;

	v1 = adjust_volt[rgb_index][AD_IV1];
	v171 = adjust_volt[rgb_index][AD_IV255];
    if(gamma >= V87_VOLTAGE_COUNT) {
        printk("[SMART:ERROR] : %s Exceed Max Gamma Value (%d)",__func__,gamma);
        gamma = V87_VOLTAGE_COUNT-1;

    }
	ratio = cv64_dv320_ratio_tbl[gamma];

	ret = (v1 << 10) - ((v1-v171)*ratio);
	ret = ret >> 10;

	return ret;
}

#if 0
u32 calc_v171_volt(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX])
{
	/* for CV : 65, DV :320 */
	int ret = 0;
	u32 v1, v255;
	u32 ratio = 0;

	v1 = adjust_volt[rgb_index][AD_IV1];
	v255 = adjust_volt[rgb_index][AD_IV255];

    if(gamma >= V171_VOLTAGE_COUNT) {
        printk("[SMART:ERROR] : %s Exceed Max Gamma Value (%d)",__func__,gamma);
        gamma = V171_VOLTAGE_COUNT-1;

    }

	ratio = cv64_dv320_ratio_tbl[gamma];

	ret = (v1 << 10) - ((v1-v255)*ratio);
	ret = ret >> 10;

	return ret;
}
#endif


u32 calc_v255_volt(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX])
{
	u32 ret = 0;

    if(gamma >= V255_VOLTAGE_COUNT) {
        printk("[SMART:ERROR] : %s Exceed Max Gamma Value (%d)",__func__,gamma);
        gamma = V255_VOLTAGE_COUNT-1;
    }

	ret = v255_voltage_tbl[gamma] >> 10;

	return ret;
}


#define MTP_REVERSE 1

#if 0
//origin
u8 calc_voltage_table(struct str_smart_dim *smart, const u8 *mtp)
{	
	int gil;
	int c, i, j;
#if defined(MTP_REVERSE)
	int offset1 = 0;
#endif
	int offset = 0;
	s16 t1, t2;
	s16 adjust_mtp[CI_MAX][IV_MAX];
	/* u32 adjust_volt[CI_MAX][AD_IVMAX] = {0, }; */
	u8 range_index;
	u8 table_index = 0;

	u32 v1, v2;
	u32 ratio;

	u32(*calc_volt[IV_MAX])(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX]) = {
		calc_v1_volt,
		calc_v15_volt,
		calc_v35_volt,
		calc_v59_volt,
		calc_v87_volt,
		//calc_v171_volt,
		calc_v255_volt,
	};

	u8 calc_seq[5] = {IV_1, /*IV_171,*/ IV_87, IV_59, IV_35, IV_15};
	u8 ad_seq[5] = {AD_IV1,/* AD_IV171,*/ AD_IV87, AD_IV59, AD_IV35, AD_IV15};

	memset(adjust_mtp, 0, sizeof(adjust_mtp));

	for (c = CI_RED; c < CI_MAX; c++) {
		offset = IV_255*CI_MAX+c*2;
		printk("\noffset : %d , c : %d\n", offset, c);
#if defined(MTP_REVERSE)
		offset1 = c*7+5;
		printk("\noffset1 : %d , c : %d\n", offset, c);
		t1 = s9_to_s16(mtp[offset1]<<8|mtp[offset1+1]);
#else
		t1 = s9_to_s16(mtp[offset]<<8|mtp[offset+1]);
#endif
		t2 = s9_to_s16(smart->default_gamma[offset]<<8|
			smart->default_gamma[offset+1]) + t1;

		printk("\nsmart->mtp[%d][%d] = mtp[%d]\n", c, IV_255, offset1);
		printk("\nadjust->mtp[%d][%d] = mtp[%d]\n", c, IV_255, offset);
		
		smart->mtp[c][IV_255] = t1;
		adjust_mtp[c][IV_255] = t2;

		smart->adjust_volt[c][AD_IV255] = calc_volt[IV_255](t2, c, smart->adjust_volt);
		/* for V0 All RGB Voltage Value is Reference Voltage */
		smart->adjust_volt[c][AD_IV0] = VREG0_REG_VOLT;

	}

	for (c = CI_RED; c < CI_MAX; c++) {
		for (i = IV_1; i < IV_255; i++) {
#if defined(MTP_REVERSE)
			t1 = (s8)mtp[(calc_seq[i])+(c*7)];
#else
			t1 = (s8)mtp[CI_MAX*calc_seq[i]+c];
#endif
			
			t2 = smart->default_gamma[CI_MAX*calc_seq[i]+c] + t1;

			printk("\nsmart->mtp[%d][%d] = mtp[%d]\n", c, calc_seq[i], (calc_seq[i])+(c*7));
			printk("\nadjust->mtp[%d][%d] = mtp[%d]\n", c, calc_seq[i], CI_MAX*calc_seq[i]+c);
			
			smart->mtp[c][calc_seq[i]] = t1;
			adjust_mtp[c][calc_seq[i]] = t2;
			smart->adjust_volt[c][ad_seq[i]] = calc_volt[calc_seq[i]](t2, c, smart->adjust_volt);
        }
	}


	for (i = 0; i < AD_IVMAX; i++) {
		for (c = CI_RED; c < CI_MAX; c++)
			smart->ve[table_index].v[c] = smart->adjust_volt[c][i];
		range_index = 0;

		for (j = table_index+1; j < table_index+range_table_count[i]; j++) {
			for (c = CI_RED; c < CI_MAX; c++) {
				if (smart->t_info[i].offset_table != NULL)
					ratio = smart->t_info[i].offset_table[range_index] * smart->t_info[i].rv;
				else
					ratio = (range_table_count[i]-(range_index+1)) * smart->t_info[i].rv;

				v1 = smart->adjust_volt[c][i+1] << 15;
				v2 = (smart->adjust_volt[c][i] - smart->adjust_volt[c][i+1])*ratio;
				smart->ve[j].v[c] = ((v1+v2) >> 15);
			}
			range_index++;
		}
		table_index = j;
	}


#if 1
	printk("++++++++++++++++++++++++++++++ MTP VALUE ++++++++++++++++++++++++++++++\n");
	for (i = IV_1; i < IV_MAX; i++) {
		printk("V Level : %d - ", i);
		for (c = CI_RED; c < CI_MAX; c++)
			printk("  %c : 0x%08x(%04d)", color_name[c], smart->mtp[c][i], smart->mtp[c][i]);
		printk("\n");
	}

	printk("\n\n++++++++++++++++++++++++++++++ ADJUST VALUE ++++++++++++++++++++++++++++++\n");
	for (i = IV_1; i < IV_MAX; i++) {
		printk("V Level : %d - ", i);
		for (c = CI_RED; c < CI_MAX; c++)
			printk("  %c : 0x%08x(%04d)", color_name[c],
				adjust_mtp[c][i], adjust_mtp[c][i]);
		printk("\n");
	}

	printk("\n\n++++++++++++++++++++++++++++++ ADJUST VOLTAGE ++++++++++++++++++++++++++++++\n");
	for (i = AD_IV0; i < AD_IVMAX; i++) {
		printk("V Level : %d - ", i);
		for (c = CI_RED; c < CI_MAX; c++)
			printk("  %c : %04dV", color_name[c], smart->adjust_volt[c][i]);
		printk("\n");
	}

	printk("\n\n++++++++++++++++++++++++++++++++++++++  VOLTAGE TABLE ++++++++++++++++++++++++++++++++++++++\n");
	for (i = 0; i < 256; i++) {
		printk("Gray Level : %03d - ", i);
		for (c = CI_RED; c < CI_MAX; c++)
			printk("  %c : %04dV", color_name[c], smart->ve[i].v[c]);
		printk("\n");
	}
#endif
	return 0;
}

#else

u8 calc_voltage_table(struct str_smart_dim *smart, const u8 *mtp, bool isMainLcd)
{	
	int gil;
	int c, i, j;
#if defined(MTP_REVERSE)
	int offset1 = 0;
#endif
	int offset = 0;
	s16 t1, t2;
	s16 adjust_mtp[CI_MAX][IV_MAX];
	/* u32 adjust_volt[CI_MAX][AD_IVMAX] = {0, }; */
	u8 range_index;
	u8 table_index = 0;

	u32 v1, v2;
	u32 ratio;

	s16 (*mtp_data)[IV_MAX];
	u32 (*adjust_volt)[AD_IVMAX];
	struct str_voltage_entry *ve;
	//if(isMainLcd) {
		mtp_data= smart->mtp;
		adjust_volt = smart->adjust_volt;
		ve = smart->ve;
//	} else {
	//	mtp_data = smart ->sub_mtp;
		//adjust_volt = smart->sub_adjust_volt;
	//	ve = smart->sub_ve;
	//}
	
	u32(*calc_volt[IV_MAX])(s16 gamma, int rgb_index, u32 adjust_volt[CI_MAX][AD_IVMAX]) = {
		calc_v1_volt,
		calc_v15_volt,
		calc_v35_volt,
		calc_v59_volt,
		calc_v87_volt,
		//calc_v171_volt,
		calc_v255_volt,
	};

	u8 calc_seq[5] = {IV_1, /*IV_171,*/ IV_87, IV_59, IV_35, IV_15};
	u8 ad_seq[5] = {AD_IV1,/* AD_IV171,*/ AD_IV87, AD_IV59, AD_IV35, AD_IV15};

	memset(adjust_mtp, 0, sizeof(adjust_mtp));

	for (c = CI_RED; c < CI_MAX; c++) {
		offset = IV_255*CI_MAX+c*2;
	//	offset = offset +1;
		printk("\noffset : %d , c : %d\n", offset, c);
#if defined(MTP_REVERSE)
		offset1 = c*7+5;
		printk("\noffset1 : %d , c : %d\n", offset, c);
		t1 = s9_to_s16(mtp[offset1]<<8|mtp[offset1+1]);
#else
		t1 = s9_to_s16(mtp[offset]<<8|mtp[offset+1]);
#endif
		t2 = s9_to_s16(smart->default_gamma[offset]<<8|
			smart->default_gamma[offset+1]) + t1;

		printk("\nsmart->mtp[%d][%d] = mtp[%d]\n", c, IV_255, offset1);
		printk("\nadjust->mtp[%d][%d] = mtp[%d]\n", c, IV_255, offset);
		
		mtp_data[c][IV_255] = t1;
		adjust_mtp[c][IV_255] = t2;

		adjust_volt[c][AD_IV255] = calc_volt[IV_255](t2, c, adjust_volt);
		/* for V0 All RGB Voltage Value is Reference Voltage */
		adjust_volt[c][AD_IV0] = VREG0_REG_VOLT;

	}

	for (c = CI_RED; c < CI_MAX; c++) {
		for (i = IV_1; i < IV_255; i++) {
#if defined(MTP_REVERSE)
			t1 = (s8)mtp[(calc_seq[i])+(c*7)];
#else
			t1 = (s8)mtp[CI_MAX*calc_seq[i]+c];
#endif
			
			t2 = smart->default_gamma[CI_MAX*calc_seq[i]+c] + t1;

			printk("\nsmart->mtp[%d][%d] = mtp[%d]\n", c, calc_seq[i], (calc_seq[i])+(c*7));
			printk("\nadjust->mtp[%d][%d] = mtp[%d]\n", c, calc_seq[i], CI_MAX*calc_seq[i]+c);
			
			mtp_data[c][calc_seq[i]] = t1;
			adjust_mtp[c][calc_seq[i]] = t2;
			adjust_volt[c][ad_seq[i]] = calc_volt[calc_seq[i]](t2, c, adjust_volt);
        }
	}


	for (i = 0; i < AD_IVMAX; i++) {
		for (c = CI_RED; c < CI_MAX; c++)
			ve[table_index].v[c] = adjust_volt[c][i];
		range_index = 0;

		for (j = table_index+1; j < table_index+range_table_count[i]; j++) {
			for (c = CI_RED; c < CI_MAX; c++) {
				if (smart->t_info[i].offset_table != NULL)
					ratio = smart->t_info[i].offset_table[range_index] * smart->t_info[i].rv;
				else
					ratio = (range_table_count[i]-(range_index+1)) * smart->t_info[i].rv;

				v1 = adjust_volt[c][i+1] << 15;
				v2 = (adjust_volt[c][i] - adjust_volt[c][i+1])*ratio;
				ve[j].v[c] = ((v1+v2) >> 15);
			}
			range_index++;
		}
		table_index = j;
	}


#if 0
	//if(isMainLcd) {
		printk("++++++++++++++++++++++++++++++ MTP VALUE ++++++++++++++++++++++++++++++\n");
		for (i = IV_1; i < IV_MAX; i++) {
			printk("V Level : %d - ", i);
			for (c = CI_RED; c < CI_MAX; c++)
				printk("  %c : 0x%08x(%04d)", color_name[c], smart->mtp[c][i], smart->mtp[c][i]);
			printk("\n");
		}

		printk("\n\n++++++++++++++++++++++++++++++ ADJUST VALUE ++++++++++++++++++++++++++++++\n");
		for (i = IV_1; i < IV_MAX; i++) {
			printk("V Level : %d - ", i);
			for (c = CI_RED; c < CI_MAX; c++)
				printk("  %c : 0x%08x(%04d)", color_name[c],
					adjust_mtp[c][i], adjust_mtp[c][i]);
			printk("\n");
		}

		printk("\n\n++++++++++++++++++++++++++++++ ADJUST VOLTAGE ++++++++++++++++++++++++++++++\n");
		for (i = AD_IV0; i < AD_IVMAX; i++) {
			printk("V Level : %d - ", i);
			for (c = CI_RED; c < CI_MAX; c++)
				printk("  %c : %04dV", color_name[c], smart->adjust_volt[c][i]);
			printk("\n");
		}

		printk("\n\n++++++++++++++++++++++++++++++++++++++  VOLTAGE TABLE ++++++++++++++++++++++++++++++++++++++\n");
		for (i = 0; i < 256; i++) {
			printk("Gray Level : %03d - ", i);
			for (c = CI_RED; c < CI_MAX; c++)
				printk("  %c : %04dV", color_name[c], smart->ve[i].v[c]);
			printk("\n");
		}
/*	} else {

		printk("++++++++++++++++++++++++++++++ MTP VALUE ++++++++++++++++++++++++++++++\n");
		for (i = IV_1; i < IV_MAX; i++) {
			printk("V Level : %d - ", i);
			for (c = CI_RED; c < CI_MAX; c++)
				printk("  %c : 0x%08x(%04d)", color_name[c], smart->sub_mtp[c][i], smart->sub_mtp[c][i]);
			printk("\n");
		}

		printk("\n\n++++++++++++++++++++++++++++++ ADJUST VALUE ++++++++++++++++++++++++++++++\n");
		for (i = IV_1; i < IV_MAX; i++) {
			printk("V Level : %d - ", i);
			for (c = CI_RED; c < CI_MAX; c++)
				printk("  %c : 0x%08x(%04d)", color_name[c],
					adjust_mtp[c][i], adjust_mtp[c][i]);
			printk("\n");
		}

		printk("\n\n++++++++++++++++++++++++++++++ ADJUST VOLTAGE ++++++++++++++++++++++++++++++\n");
		for (i = AD_IV0; i < AD_IVMAX; i++) {
			printk("V Level : %d - ", i);
			for (c = CI_RED; c < CI_MAX; c++)
				printk("  %c : %04dV", color_name[c], smart->sub_adjust_volt[c][i]);
			printk("\n");
		}

		printk("\n\n++++++++++++++++++++++++++++++++++++++  VOLTAGE TABLE ++++++++++++++++++++++++++++++++++++++\n");
		for (i = 0; i < 256; i++) {
			printk("Gray Level : %03d - ", i);
			for (c = CI_RED; c < CI_MAX; c++)
				printk("  %c : %04dV", color_name[c], smart->sub_ve[i].v[c]);
			printk("\n");
		}
	
	}*/
	
#endif
	return 0;
}
#endif


int init_table_info(struct str_smart_dim *smart, unsigned int lcd_id)
{
	int i;
	int offset = 0;

	for (i = 0; i < IV_TABLE_MAX; i++) {
		smart->t_info[i].count = (u8)range_table_count[i];
		smart->t_info[i].offset_table = offset_table[i];
		smart->t_info[i].rv = table_radio[i];
		offset += range_table_count[i];
	}
	smart->flooktbl = flookup_tbl;
	smart->g300_gra_tbl = gamma_300_gra_table;
	smart->g22_tbl = gamma_22_tbl;
	printk("Panel 2nd ID : %x\n", smart->panelid[1]);

#if 0
	for (i = 0; i < GAMMA_300CD_MAX; i++) {
		if (smart->panelid[1] == gamma_id_list[i])
			break;
	}

    printk("%s : Default Gamma Table\n", __func__);
	if (i >= GAMMA_300CD_MAX) {
		printk("[SMART DIMMING-WARNING] %s Can't found default gamma table\n", __func__);
		smart->default_gamma = gamma_300cd_list[5];
	} else {
		smart->default_gamma = gamma_300cd_list[i];
	}

    if (lcd_id == 1) {
    /* FOR M3 Panel */
        smart->default_gamma = gamma_300cd_m3_panel;
    } else if (lcd_id == 2) {
    /* For SM2 Panel */
        smart->default_gamma = gamma_300cd_sm2_panel;
    } else {
    /* Set Default */
        smart->default_gamma = gamma_300cd_sm2_panel;
    }
#endif

    /* Set Default */
	smart->default_gamma = gamma_300cd;

	return 0;
}

#define VALUE_DIM_1000 1000

u32 lookup_vtbl_idx(struct str_smart_dim *smart, u32 gamma)
{
	u32 lookup_index;
	u16 table_count, table_index;
	u32 gap, i;
	u32 minimum = smart->g300_gra_tbl[255];
	u32 candidate = 0;
	u32 offset = 0;


	/* printk("Input Gamma Value : %d\n",gamma); */

	lookup_index = (gamma >> 10) + 1;
	if (lookup_index > MAX_GRADATION) {
        printk("[SMART:WARN] : %s Exceed Max lookup_index Value (%d)",__func__,lookup_index);
		lookup_index = MAX_GRADATION - 1;
	}

	/* printk("lookup index : %d\n",lookup_index); */

	if (smart->flooktbl[lookup_index].count) {
		if (smart->flooktbl[lookup_index-1].count) {
			table_index = smart->flooktbl[lookup_index-1].entry;
			table_count = smart->flooktbl[lookup_index].count + smart->flooktbl[lookup_index-1].count;
		} else {
			table_index = smart->flooktbl[lookup_index].entry;
			table_count = smart->flooktbl[lookup_index].count;
		}
	} else {
		offset += 1;
		while (!(smart->flooktbl[lookup_index+offset].count || smart->flooktbl[lookup_index-offset].count))
			offset++;

		if (smart->flooktbl[lookup_index-offset].count)
			table_index = smart->flooktbl[lookup_index-offset].entry;
		else
			table_index = smart->flooktbl[lookup_index+offset].entry;
		table_count = smart->flooktbl[lookup_index+offset].count + smart->flooktbl[lookup_index-offset].count;
	}


	for (i = 0; i < table_count; i++) {
		if (gamma > smart->g300_gra_tbl[table_index])
			gap = gamma - smart->g300_gra_tbl[table_index];
		else
			gap = smart->g300_gra_tbl[table_index] - gamma;

		if (gap == 0) {
			candidate = table_index;
			break;
		}

		if (gap < minimum) {
			minimum = gap;
			candidate = table_index;
		}
		table_index++;
	}
#if 0
	printk("cal : found index  : %d\n", candidate);
	printk("gamma : %d, found index : %d found gamma : %d\n",
		gamma, candidate, smart->g300_gra_tbl[candidate]);
#endif
	return candidate;

}


u32 calc_v1_reg(int ci, u32 dv[CI_MAX][IV_MAX])
{
	u32 ret;
	u32 v1;

/*
V1 = 4.8 - 4.8(5+i)/600
600V1 = 2880 - 24 - 4.8i
i = (2880-24)/4.8 - 600V1/4.8
  = 592 - 125V1
*/
//포인트별 전압공식
//(600-5)<<10 - (600/4.2volt*v1)
	v1 = dv[ci][IV_1];
	ret = (595 << 10) - (143 * v1);
	ret = ret >> 10;

	return ret;


}
u32 calc_v15_reg(int ci, u32 dv[CI_MAX][IV_MAX])
{
	u32 t1, t2;
	u32 v1, v15, v35;
	u32 ret;

	v1 = dv[ci][IV_1];
	v15 = dv[ci][IV_15];
	v35 = dv[ci][IV_35];

//V19 : (65/320)

#if 0
	t1 = (v1 - v15) * 1000;
	t2 = v1 - v35;

	ret = 320*(t1/t2)-(20*1000);

	ret = ret/1000;
#else
	t1 = (v1 - v15) << 10;
	t2 = (v1 - v35) ? (v1 - v35) : (v1) ? v1 : 1;
	ret = (320 * (t1/t2)) - (65 << 10);
	ret >>= 10;

#endif
	return ret;

}
u32 calc_v35_reg(int ci, u32 dv[CI_MAX][IV_MAX])
{
	u32 t1, t2;
	u32 v1, v35, v57;
	u32 ret;

	v1 = dv[ci][IV_1];
	v35 = dv[ci][IV_35];
	v57 = dv[ci][IV_59];

//V43 : (65/320)
#if 0
	t1 = (v1 - v35) * 1000;
	t2 = v1 - v57;
	ret = 320*(t1/t2) - (65 * 1000);

	ret = ret/1000;
#else
	t1 = (v1 - v35) << 10;
	t2 = (v1 - v57) ? (v1 - v57) : (v1) ? v1 : 1;
	ret = (320 * (t1/t2)) - (65 << 10);

	ret >>= 10;
#endif

	return ret;
}

u32 calc_v50_reg(int ci, u32 dv[CI_MAX][IV_MAX])
{
	u32 t1, t2;
	u32 v1, v57, v87;
	u32 ret;

	v1 = dv[ci][IV_1];
	v57 = dv[ci][IV_59];
	v87 = dv[ci][IV_87];

#if 0
	t1 = (v1 - v57) * 1000;
	t2 = v1 - v87;
	ret = 320*(t1/t2) - (65 * 1000);
	ret = ret/1000;
#else
	t1 = (v1 - v57) << 10;
	t2 = (v1 - v87) ? (v1 - v87) : (v1) ? v1 : 1;
	ret = (320 * (t1/t2)) - (65 << 10);
	ret >>= 10;
#endif
	return ret;
}
u32 calc_v87_reg(int ci, u32 dv[CI_MAX][IV_MAX])
{
	u32 t1, t2;
	u32 v1, v87, v171;
	u32 ret;

	v1 = dv[ci][IV_1];
	v87 = dv[ci][IV_87];
	v171 = dv[ci][IV_255];

#if 0
	t1 = (v1 - v87) * 1000;
	t2 = v1 - v171;
	ret = 320*(t1/t2) - (65 * 1000);
	ret = ret/1000;
#else
	t1 = (v1 - v87) << 10;
	t2 = (v1 - v171) ? (v1 - v171) : (v1) ? v1 : 1;
	ret = (320 * (t1/t2)) - (65 << 10);
	ret >>= 10;
#endif

	return ret;
}

#if 0
u32 calc_v171_reg(int ci, u32 dv[CI_MAX][IV_MAX])
{
	u32 t1, t2;
	u32 v1, v171, v255;
	u32 ret;

	v1 = dv[ci][IV_1];
	v171 = dv[ci][IV_171];
	v255 = dv[ci][IV_255];

#if 0
	t1 = (v1 - v171) * 1000;
	t2 = v1 - v255;
	ret = 320*(t1/t2) - (65 * 1000);
	ret = ret/1000;
#else
	t1 = (v1 - v171) << 10;
	t2 = (v1 - v255) ? (v1 - v255) : (v1) ? v1 : 1;
	ret = (320 * (t1/t2)) - (64 << 10);
	ret >>= 10;
#endif

	return ret;
}
#endif


/*
V255 = 4.8 - 4.8(100+i)/600
600V255 = 2880 - 480 - 4.8i
i = (2880-480)/4.8 - 600V255/4.8
  = 500 - 125*V255
*/


u32 calc_v255_reg(int ci, u32 dv[CI_MAX][IV_MAX])
{
	u32 ret;
	u32 v255;
	
//포인트별 전압공식
//(600-120)<<10 - (600/4.2volt*v255)

	v255 = dv[ci][IV_255];

	ret = (480 << 10) - (143 * v255);
	ret = ret >> 10;

	return ret;
}


u32 calc_gamma_table(struct str_smart_dim *smart, u32 gv, u8 result[], bool isMainLcd)
{
	u32 i, c;
	u32 temp;
	u32 lidx;
	u32 dv[CI_MAX][IV_MAX];
	s16 gamma[CI_MAX][IV_MAX];
	u16 offset;

	s16 (*mtp_data)[IV_MAX];
	u32 (*adjust_volt)[AD_IVMAX];
	struct str_voltage_entry *ve;



//	if(isMainLcd) {
		mtp_data= smart->mtp;
		adjust_volt = smart->adjust_volt;
		ve = smart->ve;
//	} else {
	//	mtp_data = smart ->sub_mtp;
	//	adjust_volt = smart->sub_adjust_volt;
	//	ve = smart->sub_ve;
//	}
	

	
	u32(*calc_reg[IV_MAX])(int ci, u32 dv[CI_MAX][IV_MAX]) = {
		calc_v1_reg,
		calc_v15_reg,
		calc_v35_reg,
		calc_v50_reg,
		calc_v87_reg,
		//calc_v171_reg,
		calc_v255_reg,
	};

	memset(gamma, 0, sizeof(gamma));

	for (c = CI_RED; c < CI_MAX; c++)
		dv[c][0] = adjust_volt[c][AD_IV1];


	for (i = IV_15; i < IV_MAX; i++) {
		temp = smart->g22_tbl[dv_value[i]] * gv;
		lidx = lookup_vtbl_idx(smart, temp);
		for (c = CI_RED; c < CI_MAX; c++)
			dv[c][i] = ve[lidx].v[c];
	}

	/* for IV1 does not calculate value */
	/* just use default gamma value (IV1) */
	for (c = CI_RED; c < CI_MAX; c++)
		gamma[c][IV_1] = smart->default_gamma[c];

	for (i = IV_15; i < IV_MAX; i++) {
		for (c = CI_RED; c < CI_MAX; c++)
			gamma[c][i] = (s16)calc_reg[i](c, dv) - mtp_data[c][i];
	}

	for (c = CI_RED; c < CI_MAX; c++) {
		offset = IV_255*CI_MAX+c*2;
		result[offset+1] = gamma[c][IV_255];
	}

	for (c = CI_RED; c < CI_MAX; c++) {
		for (i = IV_1; i < IV_255; i++)
			result[(CI_MAX*i)+c] = gamma[c][i];
	}

#if 0
	printk("\n\n++++++++++++++++++++++++++++++ FOUND VOLTAGE ++++++++++++++++++++++++++++++\n");
	for (i = IV_1; i < IV_MAX; i++) {
		printk("V Level : %d - ", i);
		for (c = CI_RED; c < CI_MAX; c++)
			printk("%c : %04dV", color_name[c], dv[c][i]);
		printk("\n");
	}


	printk("\n\n++++++++++++++++++++++++++++++ FOUND REG ++++++++++++++++++++++++++++++\n");
	for (i = IV_1; i < IV_MAX; i++) {
		printk("V Level : %d - ", i);
		for (c = CI_RED; c < CI_MAX; c++)
			printk("%c : %3d, 0x%2x", color_name[c], gamma[c][i], gamma[c][i]);
		printk("\n");
	}
#endif
	return 0;
}





