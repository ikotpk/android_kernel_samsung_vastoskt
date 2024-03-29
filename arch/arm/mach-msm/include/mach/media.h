/* linux/arch/arm/plat-s5p/include/plat/media.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Samsung Media device descriptions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#if defined(CONFIG_MACH_VASTO) 

#ifndef _S5P_MEDIA_H
#define _S5P_MEDIA_H

#include <linux/types.h>
#include <asm/setup.h> 

/* 3 fimc indexes should be fixed as n, n+1 and n+2 */
#define S5P_MDEV_FIMC0      0
#define S5P_MDEV_FIMC1      1
#define S5P_MDEV_FIMC2      2
#define S5P_MDEV_TV         3
#define S5P_MDEV_MFC        4
#define S5P_MDEV_JPEG       5
#define S5P_MDEV_PMEM       6
#define S5P_MDEV_PMEM_GPU1  7
#define S5P_MDEV_PMEM_ADSP  8
#define S5P_MDEV_TEXSTREAM  9
#define S5P_MDEV_FIMD       10
#define S3C_MDEV_WIFI       11
#define S5P_MDEV_MAX        12

struct s5p_media_device {
	u32		id;
	const char	*name;
	u32		bank;
	size_t		memsize;
	dma_addr_t	paddr;
};

extern struct meminfo meminfo;
extern dma_addr_t s5p_get_media_memory_bank(int dev_id, int bank);
extern size_t s5p_get_media_memsize_bank(int dev_id, int bank);
extern void s5p_reserve_bootmem(struct s5p_media_device *mdevs, int nr_mdevs);

#endif

#endif

