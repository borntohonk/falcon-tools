/*
 * Copyright (c) 2018 naehrwert
 * Copyright (c) 2018 CTCaer
 * Copyright (c) 2018 balika011
 * Copyright (c) 2022 EliseZeroTwo <mail@elise.moe>
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

#include <string.h>

#include "tsec.h"
#include "tsec_t210.h"
#include "se_t210.h"
#include "clock.h"
#include "smmu.h"
#include "t210.h"
#include "heap.h"
#include "mc.h"
#include "util.h"
#include "gfx.h"

/* #include "../gfx/gfx.h"
extern gfx_con_t gfx_con; */

static int _tsec_dma_wait_idle()
{
	u32 timeout = get_tmr_ms() + 10000;
	while (!(TSEC(TSEC_DMATRFCMD) & TSEC_DMATRFCMD_IDLE))
		if (get_tmr_ms() > timeout)
			return 0;

	return 1;
}

unsigned char *dump_falcon_dmem() {
	u32 *buffer = malloc(0x4000);
	TSEC(TSEC_FALCON_DMEMC0) = (0 << 2) | (1 << 25);

	for (unsigned int x = 0; x < (0x4000 / 4); x++) {
		buffer[x] = TSEC(TSEC_FALCON_DMEMD0);
	}
	
	return (unsigned char *)buffer;
}

unsigned char *dump_falcon_imem() {
	u32 *buffer = malloc(0x8000);
	TSEC(TSEC_FALCON_IMEMC) = (0 << 2) | (1 << 25);

	for (unsigned int x = 0; x < (0x8000 / 4); x++) {
		buffer[x] = TSEC(TSEC_FALCON_IMEMD);
	}
	
	return (unsigned char *)buffer;
}

static int _tsec_dma_pa_to_internal_100(int not_imem, int i_offset, int pa_offset)
{
	u32 cmd;

	if (not_imem)
		cmd = TSEC_DMATRFCMD_SIZE_256B; // DMA 256 bytes
	else
		cmd = TSEC_DMATRFCMD_IMEM;      // DMA IMEM (Instruction memmory)

	TSEC(TSEC_DMATRFMOFFS) = i_offset;
	TSEC(TSEC_DMATRFFBOFFS) = pa_offset;
	TSEC(TSEC_DMATRFCMD) = cmd;

	return _tsec_dma_wait_idle();
}

extern unsigned char tsec_fw_bin;
extern unsigned char tsec_fw_bin_end;
extern size_t tsec_fw_bin_size;

tsec_res_t *tsec_run(unsigned int entrypoint) {
	tsec_res_t *res = (tsec_res_t *)malloc(sizeof(tsec_res_t));
	memset(res, 0, sizeof(tsec_res_t));

	clock_enable_host1x();
	clock_enable_tsec();
	clock_enable_sor_safe();
	clock_enable_sor0();
	clock_enable_sor1();
	clock_enable_kfuse();

	TSEC(TSEC_DMACTL) = 0;
	TSEC(TSEC_IRQMSET) = 0xFFF2;
	TSEC(TSEC_IRQDEST) = 0xFFF0;
	TSEC(TSEC_ITFEN) = 0x03;
	if (!_tsec_dma_wait_idle())
	{
		print("DMA not idle 1\n");
		goto out;
	}

	unsigned int firmware_size = ALIGN(tsec_fw_bin_size, 0x100);
	void *fwbuf = malloc(firmware_size + 0x1000);
	memset(fwbuf, 0, firmware_size + 0x1000);
	void *aligned_buf = (void *)ALIGN((unsigned int)fwbuf, 0x100);
	memcpy(aligned_buf, &tsec_fw_bin, tsec_fw_bin_size);
	TSEC(TSEC_DMATRFBASE) = (unsigned int)aligned_buf >> 8;

	for (u32 addr = 0; addr < firmware_size; addr += 0x100)
	{
		if (!_tsec_dma_pa_to_internal_100(false, addr, addr))
		{
			print("DMA failed\n");
			goto out_free;
		}
	}
	TSEC(TSEC_FALCON_BOOTVEC) = entrypoint;

	HOST1X(0x3300) = 0x34C2E1DA;
	TSEC(TSEC_STATUS) = 0;
	TSEC(TSEC_BOOTKEYVER) = 1;
	TSEC(TSEC_CPUCTL) = TSEC_CPUCTL_STARTCPU;

	if (!_tsec_dma_wait_idle())
	{
		print("DMA not idle 2\n");
		goto out_free;
	}

	u32 timeout = get_tmr_ms() + 2000;
	while (TSEC(TSEC_STATUS) != 0xF100F) {
		if (get_tmr_ms() > timeout)
		{
			print("TSEC timeout\n");
			break;
		}
	}

	res->imem = dump_falcon_imem();
	res->dmem = dump_falcon_dmem();
	res->mbx0 = TSEC(TSEC_FALCON_MAILBOX0);
	res->mbx1 = TSEC(TSEC_FALCON_MAILBOX1);
	res->exci = TSEC(TSEC_FALCON_EXCI);
	res->sec_err = TSEC(TSEC_SCP_SEC_ERR);
	res->trace_pc = TSEC(TSEC_FALCON_TRACEPC);
	res->scp_stat0 = TSEC(TSEC_SCP_STAT0);
	res->scp_stat1 = TSEC(TSEC_SCP_STAT1);
	res->scp_stat2 = TSEC(TSEC_SCP_STAT2);
	HOST1X(0x3300) = 0;
	res->sor1[0] = SOR1(SOR_NV_PDISP_SOR_DP_HDCP_BKSV_LSB);
	res->sor1[1] = SOR1(SOR_NV_PDISP_SOR_TMDS_HDCP_BKSV_LSB);
	res->sor1[2] = SOR1(SOR_NV_PDISP_SOR_TMDS_HDCP_CN_MSB);
	res->sor1[3] = SOR1(SOR_NV_PDISP_SOR_TMDS_HDCP_CN_LSB);
	SOR1(SOR_NV_PDISP_SOR_DP_HDCP_BKSV_LSB) = 0;
	SOR1(SOR_NV_PDISP_SOR_TMDS_HDCP_BKSV_LSB) = 0;
	SOR1(SOR_NV_PDISP_SOR_TMDS_HDCP_CN_MSB) = 0;
	SOR1(SOR_NV_PDISP_SOR_TMDS_HDCP_CN_LSB) = 0;

	out_free:
	free(fwbuf);

	out:
	clock_disable_kfuse();
	clock_disable_sor1();
	clock_disable_sor0();
	clock_disable_sor_safe();
	clock_disable_tsec();
	clock_disable_host1x();

	return res;
}
