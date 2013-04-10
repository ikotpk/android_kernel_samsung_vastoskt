/*
 * Copyright 2006-2010, Cypress Semiconductor Corporation.
 * Copyright (C) 2010, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef _MELFAS_TOUCHKEY_H__
#define _MELFAS_TOUCHKEY_H__

#define MELFAS_TOUCHKEY_DEV_NAME "melfas_touchkey"

#define KEY_INT         80
#define KEY_SCL         84
#define KEY_SDA         85
#define KEY_LED_33V     48

struct touchkey_platform_data {
	int keycode_cnt;
	const int *keycode;
	void (*touchkey_onoff) (int);
};

enum {
	TOUCHKEY_OFF,
	TOUCHKEY_ON,
};

#endif /* _MELFAS_TOUCHKEY_H__ */
