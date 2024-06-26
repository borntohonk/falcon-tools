/*
 * Copyright (c) 2018 naehrwert
 * Copyright (c) 2018 CTCaer
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SDMMC_H_
#define _SDMMC_H_

#include "types.h"
#include "sdmmc_driver.h"

extern u32 sd_power_cycle_time_start;

typedef struct _mmc_cid
{
	u32 manfid;
	u8  prod_name[8];
	u8  card_bga;
	u8  prv;
	u32 serial;
	u16 oemid;
	u16	year;
	u8  hwrev;
	u8  fwrev;
	u8  month;
} mmc_cid_t;

typedef struct _mmc_csd
{
	u8  structure;
	u8  mmca_vsn;
	u16 cmdclass;
	u32 c_size;
	u32 r2w_factor;
	u32 max_dtr;
	u32 erase_size;		/* In sectors */
	u32 read_blkbits;
	u32 write_blkbits;
	u32 capacity;
	u8  write_protect;
	u16 busspeed;
} mmc_csd_t;

typedef struct _mmc_ext_csd
{
	u8  rev;
	u32 sectors;
	int bkops;        /* background support bit */
	int bkops_en;     /* manual bkops enable bit */
	u8  ext_struct;   /* 194 */
	u8  card_type;    /* 196 */
	u8  bkops_status; /* 246 */
	u16 dev_version;
	u8  boot_mult;
	u8  rpmb_mult;
} mmc_ext_csd_t;

typedef struct _sd_scr
{
	u8 sda_vsn;
	u8 sda_spec3;
	u8 bus_widths;
	u8 cmds;
} sd_scr_t;

typedef struct _sd_ssr
{
	u8 bus_width;
	u8 speed_class;
	u8 uhs_grade;
	u8 video_class;
	u8 app_class;
} sd_ssr_t;

/*! SDMMC storage context. */
typedef struct _sdmmc_storage_t
{
	sdmmc_t *sdmmc;
	u32 rca;
	int has_sector_access;
	u32 sec_cnt;
	int is_low_voltage;
	u32 partition;
	u8  raw_cid[0x10];
	u8  raw_csd[0x10];
	u8  raw_scr[8];
	u8  raw_ssr[0x40];
	mmc_cid_t     cid;
	mmc_csd_t     csd;
	mmc_ext_csd_t ext_csd;
	sd_scr_t      scr;
	sd_ssr_t      ssr;
} sdmmc_storage_t;

int sdmmc_storage_end(sdmmc_storage_t *storage);
int sdmmc_storage_read(sdmmc_storage_t *storage, u32 sector, u32 num_sectors, void *buf);
int sdmmc_storage_write(sdmmc_storage_t *storage, u32 sector, u32 num_sectors, void *buf);
int sdmmc_storage_init_mmc(sdmmc_storage_t *storage, sdmmc_t *sdmmc, u32 id, u32 bus_width, u32 type);
int sdmmc_storage_set_mmc_partition(sdmmc_storage_t *storage, u32 partition);
void sdmmc_storage_init_wait_sd();
int sdmmc_storage_init_sd(sdmmc_storage_t *storage, sdmmc_t *sdmmc, u32 id, u32 bus_width, u32 type);
int sdmmc_storage_init_gc(sdmmc_storage_t *storage, sdmmc_t *sdmmc);

#endif
