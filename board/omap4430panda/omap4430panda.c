/*
 * (C) Copyright 2004-2009
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/clocks.h>
#include <asm/arch/mem.h>
#include <i2c.h>
#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined(CFG_NAND_LEGACY)
#include <linux/mtd/nand_legacy.h>
#endif

/* EMIF and DMM registers */
#define EMIF1_BASE			0x4c000000
#define EMIF2_BASE			0x4d000000
#define DMM_BASE			0x4e000000
/* EMIF */
#define EMIF_MOD_ID_REV			0x0000
#define EMIF_STATUS			0x0004
#define EMIF_SDRAM_CONFIG		0x0008
#define EMIF_LPDDR2_NVM_CONFIG		0x000C
#define EMIF_SDRAM_REF_CTRL		0x0010
#define EMIF_SDRAM_REF_CTRL_SHDW	0x0014
#define EMIF_SDRAM_TIM_1		0x0018
#define EMIF_SDRAM_TIM_1_SHDW		0x001C
#define EMIF_SDRAM_TIM_2		0x0020
#define EMIF_SDRAM_TIM_2_SHDW		0x0024
#define EMIF_SDRAM_TIM_3		0x0028
#define EMIF_SDRAM_TIM_3_SHDW		0x002C
#define EMIF_LPDDR2_NVM_TIM		0x0030
#define EMIF_LPDDR2_NVM_TIM_SHDW	0x0034
#define EMIF_PWR_MGMT_CTRL		0x0038
#define EMIF_PWR_MGMT_CTRL_SHDW		0x003C
#define EMIF_LPDDR2_MODE_REG_DATA	0x0040
#define EMIF_LPDDR2_MODE_REG_CFG	0x0050
#define EMIF_L3_CONFIG			0x0054
#define EMIF_L3_CFG_VAL_1		0x0058
#define EMIF_L3_CFG_VAL_2		0x005C
#define IODFT_TLGC			0x0060
#define EMIF_PERF_CNT_1			0x0080
#define EMIF_PERF_CNT_2			0x0084
#define EMIF_PERF_CNT_CFG		0x0088
#define EMIF_PERF_CNT_SEL		0x008C
#define EMIF_PERF_CNT_TIM		0x0090
#define EMIF_READ_IDLE_CTRL		0x0098
#define EMIF_READ_IDLE_CTRL_SHDW	0x009c
#define EMIF_ZQ_CONFIG			0x00C8
#define EMIF_DDR_PHY_CTRL_1		0x00E4
#define EMIF_DDR_PHY_CTRL_1_SHDW	0x00E8
#define EMIF_DDR_PHY_CTRL_2		0x00EC

#define DMM_LISA_MAP_0 			0x0040
#define DMM_LISA_MAP_1 			0x0044
#define DMM_LISA_MAP_2 			0x0048
#define DMM_LISA_MAP_3 			0x004C

#define MR0_ADDR			0
#define MR1_ADDR			1
#define MR2_ADDR			2
#define MR4_ADDR			4
#define MR10_ADDR			10
#define MR16_ADDR			16
#define REF_EN				0x40000000
/* defines for MR1 */
#define MR1_BL4				2
#define MR1_BL8				3
#define MR1_BL16			4

#define MR1_BT_SEQ			0
#define BT_INT				1

#define MR1_WC				0
#define MR1_NWC				1

#define MR1_NWR3			1
#define MR1_NWR4			2
#define MR1_NWR5			3
#define MR1_NWR6			4
#define MR1_NWR7			5
#define MR1_NWR8			6

#define MR1_VALUE	((MR1_NWR3 << 5) | (MR1_WC << 4) | (MR1_BT_SEQ << 3)  \
							| (MR1_BL8 << 0))

/* defines for MR2 */
#define MR2_RL3_WL1			1
#define MR2_RL4_WL2			2
#define MR2_RL5_WL2			3
#define MR2_RL6_WL3			4

/* defines for MR10 */
#define MR10_ZQINIT			0xFF
#define MR10_ZQRESET			0xC3
#define MR10_ZQCL			0xAB
#define MR10_ZQCS			0x56


/* TODO: FREQ update method is not working so shadow registers programming
 * is just for same of completeness. This would be safer if auto
 * trasnitions are working
 */
#define FREQ_UPDATE_EMIF
/* EMIF Needs to be configured@19.2 MHz and shadow registers
 * should be programmed for new OPP.
 */
/* Elpida 2x2Gbit */
#define SDRAM_CONFIG_INIT		0x80800EB1
#define DDR_PHY_CTRL_1_INIT		0x849FFFF5
#define READ_IDLE_CTRL			0x000501FF
#define PWR_MGMT_CTRL			0x4000000f
#define PWR_MGMT_CTRL_OPP100		0x4000000f
#define ZQ_CONFIG			0x500b3215

#define CS1_MR(mr)	((mr) | 0x80000000)
struct ddr_regs{
	u32 tim1;
	u32 tim2;
	u32 tim3;
	u32 phy_ctrl_1;
	u32 ref_ctrl;
	u32 config_init;
	u32 config_final;
	u32 zq_config;
	u8 mr1;
	u8 mr2;
};
const struct ddr_regs ddr_regs_380_mhz = {
	.tim1		= 0x10cb061a,
	.tim2		= 0x20350d52,
	.tim3		= 0x00b1431f,
	.phy_ctrl_1	= 0x849FF408,
	.ref_ctrl	= 0x000005ca,
	.config_init	= 0x80000eb1,
	.config_final	= 0x80001ab1,
	.zq_config	= 0x500b3215,
	.mr1		= 0x83,
	.mr2		= 0x4
};

/*
 * Unused timings - but we may need them later
 * Keep them commented
 */
#if 0
const struct ddr_regs ddr_regs_400_mhz = {
	.tim1		= 0x10eb065a,
	.tim2		= 0x20370dd2,
	.tim3		= 0x00b1c33f,
	.phy_ctrl_1	= 0x849FF408,
	.ref_ctrl	= 0x00000618,
	.config_init	= 0x80000eb1,
	.config_final	= 0x80001ab1,
	.zq_config	= 0x500b3215,
	.mr1		= 0x83,
	.mr2		= 0x4
};

const struct ddr_regs ddr_regs_200_mhz = {
	.tim1		= 0x08648309,
	.tim2		= 0x101b06ca,
	.tim3		= 0x0048a19f,
	.phy_ctrl_1	= 0x849FF405,
	.ref_ctrl	= 0x0000030c,
	.config_init	= 0x80000eb1,
	.config_final	= 0x80000eb1,
	.zq_config	= 0x500b3215,
	.mr1		= 0x23,
	.mr2		= 0x1
};
#endif

const struct ddr_regs ddr_regs_200_mhz_2cs = {
	.tim1		= 0x08648309,
	.tim2		= 0x101b06ca,
	.tim3		= 0x0048a19f,
	.phy_ctrl_1	= 0x849FF405,
	.ref_ctrl	= 0x0000030c,
	.config_init	= 0x80000eb9,
	.config_final	= 0x80000eb9,
	.zq_config	= 0xD00b3215,
	.mr1		= 0x23,
	.mr2		= 0x1
};

const struct ddr_regs ddr_regs_400_mhz_2cs = {
	/* tRRD changed from 10ns to 12.5ns because of the tFAW requirement*/
	.tim1		= 0x10eb0662,
	.tim2		= 0x20370dd2,
	.tim3		= 0x00b1c33f,
	.phy_ctrl_1	= 0x849FF408,
	.ref_ctrl	= 0x00000618,
	.config_init	= 0x80000eb9,
	.config_final	= 0x80001ab9,
	.zq_config	= 0xD00b3215,
	.mr1		= 0x83,
	.mr2		= 0x4
};

/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b" : "=r" (loops) : "0"(loops));
}


void big_delay(unsigned int count)
{
	int i;
	for (i = 0; i < count; i++)
		delay(1);
}

void reset_phy(unsigned int base)
{
	__raw_writel(__raw_readl(base + IODFT_TLGC) | (1 << 10),
							     base + IODFT_TLGC);
}

/* this flashes the Panda LEDs forever, if called after muxconf */

void spam_leds(void)
{
	unsigned int v = __raw_readl(OMAP44XX_GPIO_BASE1 + __GPIO_OE);

	/* set both LED gpio to output */
	__raw_writel((v & ~(0x03 << 7)), OMAP44XX_GPIO_BASE1 + __GPIO_OE);

	v = __raw_readl(OMAP44XX_GPIO_BASE1 + __GPIO_DATAOUT);
	while (1) {
		__raw_writel((v & ~(0x03 << 7)),
					  OMAP44XX_GPIO_BASE1 + __GPIO_DATAOUT);
		big_delay(3000000);
		__raw_writel((v | (0x03 << 7)),
					  OMAP44XX_GPIO_BASE1 + __GPIO_DATAOUT);
		big_delay(3000000);
	}
}

/* TODO: FREQ update method is not working so shadow registers programming
 * is just for same of completeness. This would be safer if auto
 * trasnitions are working
 */
static int emif_config(unsigned int base)
{
	const struct ddr_regs *ddr_regs = &ddr_regs_400_mhz_2cs;

	switch (omap_revision()) {
	case OMAP4430_ES1_0:
		ddr_regs = &ddr_regs_380_mhz;
		break;
	case OMAP4430_ES2_0:
		ddr_regs = &ddr_regs_200_mhz_2cs;
		break;
	}

	/*
	 * set SDRAM CONFIG register
	 * EMIF_SDRAM_CONFIG[31:29] REG_SDRAM_TYPE = 4 for LPDDR2-S4
	 * EMIF_SDRAM_CONFIG[28:27] REG_IBANK_POS = 0
	 * EMIF_SDRAM_CONFIG[13:10] REG_CL = 3
	 * EMIF_SDRAM_CONFIG[6:4] REG_IBANK = 3 - 8 banks
	 * EMIF_SDRAM_CONFIG[3] REG_EBANK = 0 - CS0
	 * EMIF_SDRAM_CONFIG[2:0] REG_PAGESIZE = 2  - 512- 9 column
	 * JDEC specs - S4-2Gb --8 banks -- R0-R13, C0-c8
	 */
	__raw_writel(__raw_readl(base + EMIF_LPDDR2_NVM_CONFIG) & 0xbfffffff,
						 base + EMIF_LPDDR2_NVM_CONFIG);
	__raw_writel(ddr_regs->config_init, base + EMIF_SDRAM_CONFIG);

	/* PHY control values */
	__raw_writel(DDR_PHY_CTRL_1_INIT, base + EMIF_DDR_PHY_CTRL_1);
	__raw_writel(ddr_regs->phy_ctrl_1, base + EMIF_DDR_PHY_CTRL_1_SHDW);

	/*
	 * EMIF_READ_IDLE_CTRL
	 */
	__raw_writel(READ_IDLE_CTRL, base + EMIF_READ_IDLE_CTRL);
	__raw_writel(READ_IDLE_CTRL, base + EMIF_READ_IDLE_CTRL);

	/*
	 * EMIF_SDRAM_TIM_1
	 */
	__raw_writel(ddr_regs->tim1, base + EMIF_SDRAM_TIM_1);
	__raw_writel(ddr_regs->tim1, base + EMIF_SDRAM_TIM_1_SHDW);

	/*
	 * EMIF_SDRAM_TIM_2
	 */
	__raw_writel(ddr_regs->tim2, base + EMIF_SDRAM_TIM_2);
	__raw_writel(ddr_regs->tim2, base + EMIF_SDRAM_TIM_2_SHDW);

	/*
	 * EMIF_SDRAM_TIM_3
	 */
	__raw_writel(ddr_regs->tim3, base + EMIF_SDRAM_TIM_3);
	__raw_writel(ddr_regs->tim3, base + EMIF_SDRAM_TIM_3_SHDW);

	__raw_writel(ddr_regs->zq_config, base + EMIF_ZQ_CONFIG);

	/*
	 * poll MR0 register (DAI bit)
	 * REG_CS[31] = 0 -- Mode register command to CS0
	 * REG_REFRESH_EN[30] = 1 -- Refresh enable after MRW
	 * REG_ADDRESS[7:0] = 00 -- Refresh enable after MRW
	 */

	__raw_writel(MR0_ADDR, base + EMIF_LPDDR2_MODE_REG_CFG);

	while (__raw_readl(base + EMIF_LPDDR2_MODE_REG_DATA) & 1)
		;

	__raw_writel(CS1_MR(MR0_ADDR), base + EMIF_LPDDR2_MODE_REG_CFG);

	while (__raw_readl(base + EMIF_LPDDR2_MODE_REG_DATA) & 1)
		;


	/* set MR10 register */
	__raw_writel(MR10_ADDR, base + EMIF_LPDDR2_MODE_REG_CFG);
	__raw_writel(MR10_ZQINIT, base + EMIF_LPDDR2_MODE_REG_DATA);
	__raw_writel(CS1_MR(MR10_ADDR), base + EMIF_LPDDR2_MODE_REG_CFG);
	__raw_writel(MR10_ZQINIT, base + EMIF_LPDDR2_MODE_REG_DATA);

	/* wait for tZQINIT=1us  */
	delay(10);

	/* set MR1 register */
	__raw_writel(MR1_ADDR, base + EMIF_LPDDR2_MODE_REG_CFG);
	__raw_writel(ddr_regs->mr1, base + EMIF_LPDDR2_MODE_REG_DATA);
	__raw_writel(CS1_MR(MR1_ADDR), base + EMIF_LPDDR2_MODE_REG_CFG);
	__raw_writel(ddr_regs->mr1, base + EMIF_LPDDR2_MODE_REG_DATA);

	/* set MR2 register RL=6 for OPP100 */
	__raw_writel(MR2_ADDR, base + EMIF_LPDDR2_MODE_REG_CFG);
	__raw_writel(ddr_regs->mr2, base + EMIF_LPDDR2_MODE_REG_DATA);
	__raw_writel(CS1_MR(MR2_ADDR), base + EMIF_LPDDR2_MODE_REG_CFG);
	__raw_writel(ddr_regs->mr2, base + EMIF_LPDDR2_MODE_REG_DATA);

	/* Set SDRAM CONFIG register again here with final RL-WL value */
	__raw_writel(ddr_regs->config_final, base + EMIF_SDRAM_CONFIG);
	__raw_writel(ddr_regs->phy_ctrl_1, base + EMIF_DDR_PHY_CTRL_1);

	/*
	 * EMIF_SDRAM_REF_CTRL
	 * refresh rate = DDR_CLK / reg_refresh_rate
	 * 3.9 uS = (400MHz)	/ reg_refresh_rate
	 */
	__raw_writel(ddr_regs->ref_ctrl, base + EMIF_SDRAM_REF_CTRL);
	__raw_writel(ddr_regs->ref_ctrl, base + EMIF_SDRAM_REF_CTRL_SHDW);

	/* set MR16 register */
	__raw_writel(MR16_ADDR | REF_EN, base + EMIF_LPDDR2_MODE_REG_CFG);
	__raw_writel(0, base + EMIF_LPDDR2_MODE_REG_DATA);
	__raw_writel(CS1_MR(MR16_ADDR | REF_EN),
					       base + EMIF_LPDDR2_MODE_REG_CFG);
	__raw_writel(0, base + EMIF_LPDDR2_MODE_REG_DATA);

	/* LPDDR2 init complete */

	return 0;
}
/*****************************************
 * Routine: ddr_init
 * Description: Configure DDR
 * EMIF1 -- CS0 -- DDR1 (256 MB)
 * EMIF2 -- CS0 -- DDR2 (256 MB)
 *****************************************/
static void ddr_init(void)
{
	unsigned int base_addr;

	switch (omap_revision()) {

	case OMAP4430_ES1_0:
		/* Configurte the Control Module DDRIO device */
		__raw_writel(0x1c1c1c1c, OMAP44XX_CONTROL_LPDDR2IO1_0);
		__raw_writel(0x1c1c1c1c, OMAP44XX_CONTROL_LPDDR2IO1_1);
		__raw_writel(0x1c1c1c1c, OMAP44XX_CONTROL_LPDDR2IO1_2);
		__raw_writel(0x1c1c1c1c, OMAP44XX_CONTROL_LPDDR2IO2_0);
		__raw_writel(0x1c1c1c1c, OMAP44XX_CONTROL_LPDDR2IO2_1);
		__raw_writel(0x1c1c1c1c, OMAP44XX_CONTROL_LPDDR2IO2_2);
		/* LPDDR2IO set to NMOS PTV  !!! really EFUSE2? */
		__raw_writel(0x00ffc000, OMAP44XX_CONTROL_EFUSE_2);

		/* Both EMIFs 128 byte interleaved */
		__raw_writel(0x80540300, DMM_BASE + DMM_LISA_MAP_0);
		break;
	case OMAP4430_ES2_0:
		__raw_writel(0x9e9e9e9e, OMAP44XX_CONTROL_LPDDR2IO1_0);
		__raw_writel(0x9e9e9e9e, OMAP44XX_CONTROL_LPDDR2IO1_1);
		__raw_writel(0x9e9e9e9e, OMAP44XX_CONTROL_LPDDR2IO1_2);
		__raw_writel(0x9e9e9e9e, OMAP44XX_CONTROL_LPDDR2IO2_0);
		__raw_writel(0x9e9e9e9e, OMAP44XX_CONTROL_LPDDR2IO2_1);
		__raw_writel(0x9e9e9e9e, OMAP44XX_CONTROL_LPDDR2IO2_2);
		/* LPDDR2IO set to NMOS PTV  !!! really EFUSE2? */
		__raw_writel(0x00ffc000, OMAP44XX_CONTROL_EFUSE_2);
		/* fall thru */
	default:
		/* Both EMIFs 128 byte interleaved */
		__raw_writel(0x80640300, DMM_BASE + DMM_LISA_MAP_0);
		break;
	}

	/*
	 * DMM Configuration
	 */

	__raw_writel(0x00000000, DMM_BASE + DMM_LISA_MAP_2);
	__raw_writel(0xFF020100, DMM_BASE + DMM_LISA_MAP_3);

	/* DDR needs to be initialised @ 19.2 MHz
	 * So put core DPLL in bypass mode
	 * Configure the Core DPLL but don't lock it
	 */
	configure_core_dpll_no_lock();

	/*
	 * the following is re-enabled because without the EMIF going idle,
	 * the shadow DPLL update scheme can delay for minutes until it is
	 * able to apply the new settings... it waits until EMIF idle.
	 *
	 * This is seen in the case the ROM enabled USB boot being tried before
	 * normal boot over MMC.
	 */

	/* No IDLE: BUG in SDC */
	sr32(CM_MEMIF_CLKSTCTRL, 0, 32, 0x2);
	while ((__raw_readl(CM_MEMIF_CLKSTCTRL) & 0x700) != 0x700)
		;

	__raw_writel(0, EMIF1_BASE + EMIF_PWR_MGMT_CTRL);
	__raw_writel(0, EMIF2_BASE + EMIF_PWR_MGMT_CTRL);

	base_addr = EMIF1_BASE;
	emif_config(base_addr);

	/* Configure EMIF24D */
	base_addr = EMIF2_BASE;
	emif_config(base_addr);
	/* Lock Core using shadow CM_SHADOW_FREQ_CONFIG1 */
	lock_core_dpll_shadow();
	/* TODO: SDC needs few hacks to get DDR freq update working */

	/* Set DLL_OVERRIDE = 0 */
	__raw_writel(0, CM_DLL_CTRL);

	delay(200);

	/* Check for DDR PHY ready for EMIF1 & EMIF2 */
	while (!(__raw_readl(EMIF1_BASE + EMIF_STATUS) & 4) ||
				   !(__raw_readl(EMIF2_BASE + EMIF_STATUS) & 4))
		;

	/* Reprogram the DDR PYHY Control register */
	/* PHY control values */

	sr32(CM_MEMIF_EMIF_1_CLKCTRL, 0, 32, 0x1);
	sr32(CM_MEMIF_EMIF_2_CLKCTRL, 0, 32, 0x1);

	/* Put the Core Subsystem PD to ON State */

	__raw_writel(1 << 31, EMIF1_BASE + EMIF_PWR_MGMT_CTRL);
	__raw_writel(1 << 31, EMIF2_BASE + EMIF_PWR_MGMT_CTRL);

	/* SYSTEM BUG:
	 * In n a specific situation, the OCP interface between the DMM and
	 * EMIF may hang.
	 * 1. A TILER port is used to perform 2D burst writes of
	 * 	 width 1 and height 8
	 * 2. ELLAn port is used to perform reads
	 * 3. All accesses are routed to the same EMIF controller
	 *
	 * Work around to avoid this issue REG_SYS_THRESH_MAX value should
	 * be kept higher than default 0x7. As per recommondation 0x0A will
	 * be used for better performance with REG_LL_THRESH_MAX = 0x00
	 */
	if (omap_revision() == OMAP4430_ES1_0) {
		__raw_writel(0x0A0000FF, EMIF1_BASE + EMIF_L3_CONFIG);
		__raw_writel(0x0A0000FF, EMIF2_BASE + EMIF_L3_CONFIG);
	}

	/*
	 * DMM : DMM_LISA_MAP_0(Section_0)
	 * [31:24] SYS_ADDR 		0x80
	 * [22:20] SYS_SIZE		0x7 - 2Gb
	 * [19:18] SDRC_INTLDMM		0x1 - 128 byte
	 * [17:16] SDRC_ADDRSPC 	0x0
	 * [9:8] SDRC_MAP 		0x3
	 * [7:0] SDRC_ADDR		0X0
	 */
	reset_phy(EMIF1_BASE);
	reset_phy(EMIF2_BASE);

	__raw_writel(0, OMAP44XX_SDRC_CS0);
	__raw_writel(0, OMAP44XX_SDRC_CS0);

	/* MEMIF Clock Domain -> HW_AUTO */
	sr32(CM_MEMIF_CLKSTCTRL, 0, 32, 0x3);
}
/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init(void)
{
	unsigned int v;

	/*
	 * If the ROM has started the OTG stuff, stop it and
	 * make it look as if uninitialized for Linux or U-Boot
	 */

	/* hold OTG phy in reset (GPIO_62 -> active low reset) */

	v = __raw_readl(OMAP44XX_GPIO_BASE2 + __GPIO_OE);
	__raw_writel((v & ~(1 << 30)), OMAP44XX_GPIO_BASE2 + __GPIO_OE);

	v = __raw_readl(OMAP44XX_GPIO_BASE2 + __GPIO_DATAOUT);
	__raw_writel((v & ~(1 << 30)), OMAP44XX_GPIO_BASE2 + __GPIO_DATAOUT);

	/* kill USB PLL */

	v = __raw_readl(CM_CLKMODE_DPLL_USB);
	__raw_writel((v & ~7) | 1, CM_CLKMODE_DPLL_USB);

	return 0;
}

/*************************************************************
 * Routine: get_mem_type(void) - returns the kind of memory connected
 * to GPMC that we are trying to boot form. Uses SYS BOOT settings.
 *************************************************************/
u32 get_mem_type(void)
{
	/* no nand, so return GPMC_NONE */
	return GPMC_NONE;
}

/*****************************************
 * Routine: secure_unlock
 * Description: Setup security registers for access
 * (GP Device only)
 *****************************************/
void secure_unlock_mem(void)
{
	/* Permission values for registers -Full fledged permissions to all */
	#define UNLOCK_1 0xFFFFFFFF
	#define UNLOCK_2 0x00000000
	#define UNLOCK_3 0x0000FFFF

	/* Protection Module Register Target APE (PM_RT)*/
	__raw_writel(UNLOCK_1, RT_REQ_INFO_PERMISSION_1);
	__raw_writel(UNLOCK_1, RT_READ_PERMISSION_0);
	__raw_writel(UNLOCK_1, RT_WRITE_PERMISSION_0);
	__raw_writel(UNLOCK_2, RT_ADDR_MATCH_1);

	__raw_writel(UNLOCK_3, GPMC_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, GPMC_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, GPMC_WRITE_PERMISSION_0);

	__raw_writel(UNLOCK_3, OCM_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, OCM_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, OCM_WRITE_PERMISSION_0);
	__raw_writel(UNLOCK_2, OCM_ADDR_MATCH_2);

	/* IVA Changes */
	__raw_writel(UNLOCK_3, IVA2_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, IVA2_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, IVA2_WRITE_PERMISSION_0);

	__raw_writel(UNLOCK_1, SMS_RG_ATT0); /* SDRC region 0 public */
}

/**********************************************************
 * Routine: try_unlock_sram()
 * Description: If chip is GP/EMU(special) type, unlock the SRAM for
 *  general use.
 ***********************************************************/
void try_unlock_memory(void)
{
	/* if GP device unlock device SRAM for general use */
	/* secure code breaks for Secure/Emulation device - HS/E/T*/
}


#if defined(CONFIG_MPU_600) || defined(CONFIG_MPU_1000)
static int scale_vcores(void)
{
	unsigned int rev = omap_revision();

	/* For VC bypass only VCOREx_CGF_FORCE  is necessary and
	 * VCOREx_CFG_VOLTAGE  changes can be discarded
	 */
	__raw_writel(0, OMAP44XX_PRM_VC_CFG_I2C_MODE);
	__raw_writel(0x6026, OMAP44XX_PRM_VC_CFG_I2C_CLK);

	/* set VCORE1 force VSEL */
	if (rev == OMAP4430_ES1_0)
		__raw_writel(0x3B5512, OMAP44XX_PRM_VC_VAL_BYPASS);
	else
		__raw_writel(0x3A5512, OMAP44XX_PRM_VC_VAL_BYPASS);

	__raw_writel(__raw_readl(OMAP44XX_PRM_VC_VAL_BYPASS) | 0x1000000,
						    OMAP44XX_PRM_VC_VAL_BYPASS);
	while (__raw_readl(OMAP44XX_PRM_VC_VAL_BYPASS) & 0x1000000)
		;

	__raw_writel(__raw_readl(OMAP44XX_PRM_IRQSTATUS_MPU_A9),
						 OMAP44XX_PRM_IRQSTATUS_MPU_A9);

	/* FIXME: set VCORE2 force VSEL, Check the reset value */
	if (rev == OMAP4430_ES1_0)
		__raw_writel(0x315B12, OMAP44XX_PRM_VC_VAL_BYPASS);
	else
		__raw_writel(0x295B12, OMAP44XX_PRM_VC_VAL_BYPASS);

	__raw_writel(__raw_readl(OMAP44XX_PRM_VC_VAL_BYPASS) | 0x1000000,
						    OMAP44XX_PRM_VC_VAL_BYPASS);
	while (__raw_readl(OMAP44XX_PRM_VC_VAL_BYPASS) & 0x1000000)
		;

	__raw_writel(__raw_readl(OMAP44XX_PRM_IRQSTATUS_MPU_A9),
						 OMAP44XX_PRM_IRQSTATUS_MPU_A9);

	/*/set VCORE3 force VSEL */
	switch (rev) {
	case OMAP4430_ES1_0:
		__raw_writel(0x316112, OMAP44XX_PRM_VC_VAL_BYPASS);
		break;
	case OMAP4430_ES2_0:
		__raw_writel(0x296112, OMAP44XX_PRM_VC_VAL_BYPASS);
		break;
	case OMAP4430_ES2_1:
	default:
		__raw_writel(0x2A6112, OMAP44XX_PRM_VC_VAL_BYPASS);
		break;
	}
	__raw_writel(__raw_readl(OMAP44XX_PRM_VC_VAL_BYPASS) | 0x1000000,
						    OMAP44XX_PRM_VC_VAL_BYPASS);
	while (__raw_readl(OMAP44XX_PRM_VC_VAL_BYPASS) & 0x1000000)
		;

	__raw_writel(__raw_readl(OMAP44XX_PRM_IRQSTATUS_MPU_A9),
						 OMAP44XX_PRM_IRQSTATUS_MPU_A9);

	return 0;
}
#endif

/**********************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 * - Called path is with SRAM stack.
 **********************************************************/

void s_init(void)
{
	unsigned int rev = omap_revision();

	/*
	 * this is required to survive the muxconf in the case the ROM
	 * started up USB OTG
	 */
/* Set VCORE1 = 1.3 V, VCORE2 = VCORE3 = 1.21V */
#if defined(CONFIG_MPU_600) || defined(CONFIG_MPU_1000)
	scale_vcores();
#endif

	prcm_init();

	set_muxconf_regs();

	delay(100);

	/* Writing to AuxCR in U-boot using SMI for GP/EMU DEV */
	/* Currently SMI in Kernel on ES2 devices seems to have an issue
	 * Once that is resolved, we can postpone this config to kernel
	 */
	/* setup_auxcr(get_device_type(), external_boot); */

	ddr_init();

	if (rev == OMAP4430_ES1_0)
		return;

	if (__raw_readl(OMAP44XX_GPIO_BASE6 + __GPIO_DATAIN) & (1 << 22)) {
		/* enable software ioreq */
		sr32(OMAP44XX_SCRM_AUXCLK3, 8, 1, 0x1);
		/* set for sys_clk (38.4MHz) */
		sr32(OMAP44XX_SCRM_AUXCLK3, 1, 2, 0x0);
		/* set divisor to 2 */
		sr32(OMAP44XX_SCRM_AUXCLK3, 16, 4, 0x1);
		/* set the clock source to active */
		sr32(OMAP44XX_SCRM_ALTCLKSRC, 0, 1, 0x1);
		/* enable clocks */
		sr32(OMAP44XX_SCRM_ALTCLKSRC, 2, 2, 0x3);
	} else {
		/* enable software ioreq */
		sr32(OMAP44XX_SCRM_AUXCLK1, 8, 1, 0x1);
		/* set for PER_DPLL */
		sr32(OMAP44XX_SCRM_AUXCLK1, 1, 2, 0x2);
		/* set divisor to 16 */
		sr32(OMAP44XX_SCRM_AUXCLK1, 16, 4, 0xf);
		/* set the clock source to active */
		sr32(OMAP44XX_SCRM_ALTCLKSRC, 0, 1, 0x1);
		/* enable clocks */
		sr32(OMAP44XX_SCRM_ALTCLKSRC, 2, 2, 0x3);
	}
}

/*******************************************************
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 ********************************************************/
int misc_init_r(void)
{
	return 0;
}

/******************************************************
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 ******************************************************/
void wait_for_command_complete(unsigned int wd_base)
{
	while (__raw_readl(wd_base + WWPS))
		;
}

/*******************************************************************
 * Routine:ether_init
 * Description: take the Ethernet controller out of reset and wait
 *  		   for the EEPROM load to complete.
 ******************************************************************/

/**********************************************
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 **********************************************/
int dram_init(void)
{
	return 0;
}

#define		CP(x)	(CONTROL_PADCONF_##x)
#define		WK(x)	(CONTROL_WKUP_##x)
/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */

struct omap4panda_mux {
	unsigned int ads;
	unsigned int value;
};

static const struct omap4panda_mux omap4panda_mux[] = {
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD0),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat0 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD1),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat1 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD2),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat2 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD3),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat3 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD4),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat4 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD5),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat5 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD6),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat6 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD7),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat7 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD8),
		     PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M3  /* gpio_32 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD9),
						PTU | IEN | M3  /* gpio_33 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD10),
						PTU | IEN | M3  /* gpio_34 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD11),
						PTU | IEN | M3  /* gpio_35 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD12),
						PTU | IEN | M3  /* gpio_36 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD13),
		      PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3  /* gpio_37 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD14),
		      PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3  /* gpio_38 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD15),
		      PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3  /* gpio_39 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A16), M3  /* gpio_40 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A17), PTD | M3  /* gpio_41 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A18),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row6 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A19),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row7 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A20),
						      IEN | M3  /* gpio_44 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A21), M3  /* gpio_45 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A22), M3  /* gpio_46 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A23),
				OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_col7 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A24), PTD | M3  /* gpio_48 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A25), PTD | M3  /* gpio_49 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NCS0), M3  /* gpio_50 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NCS1), IEN | M3  /* gpio_51 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NCS2), IEN | M3  /* gpio_52 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NCS3), IEN | M3  /* gpio_53 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NWP), M3  /* gpio_54 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_CLK), PTD | M3  /* gpio_55 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NADV_ALE), M3  /* gpio_56 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NOE),
		      PTU | IEN | OFF_EN | OFF_OUT_PTD | M1  /* sdmmc2_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NWE),
		  PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_cmd */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NBE0_CLE), M3  /* gpio_59 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NBE1), PTD | M3  /* gpio_60 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_WAIT0), PTU | IEN | M3  /* gpio_61 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_WAIT1),
		       PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3 /* gpio_62 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA11), PTD | M3  /* gpio_100 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA12), PTD | IEN | M3  /* gpio_101 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA13), PTD | M3  /* gpio_102 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA14), M1  /* dsi2_te0 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA15), PTD | M3  /* gpio_104 */ },
	{ OMAP44XX_CTRL_BASE + CP(HDMI_HPD), M0  /* hdmi_hpd */ },
	{ OMAP44XX_CTRL_BASE + CP(HDMI_CEC), M0  /* hdmi_cec */ },
	{ OMAP44XX_CTRL_BASE + CP(HDMI_DDC_SCL), PTU | M0  /* hdmi_ddc_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(HDMI_DDC_SDA),
					   PTU | IEN | M0  /* hdmi_ddc_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX0), IEN | M0  /* csi21_dx0 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY0), IEN | M0  /* csi21_dy0 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX1), IEN | M0  /* csi21_dx1 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY1), IEN | M0  /* csi21_dy1 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX2), IEN | M0  /* csi21_dx2 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY2), IEN | M0  /* csi21_dy2 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX3), PTD | M7  /* csi21_dx3 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY3), PTD | M7  /* csi21_dy3 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX4),
			 PTD | OFF_EN | OFF_PD | OFF_IN | M7  /* csi21_dx4 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY4),
			 PTD | OFF_EN | OFF_PD | OFF_IN | M7  /* csi21_dy4 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI22_DX0), IEN | M0  /* csi22_dx0 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI22_DY0), IEN | M0  /* csi22_dy0 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI22_DX1), IEN | M0  /* csi22_dx1 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI22_DY1), IEN | M0  /* csi22_dy1 */ },
	{ OMAP44XX_CTRL_BASE + CP(CAM_SHUTTER),
			OFF_EN | OFF_PD | OFF_OUT_PTD | M0  /* cam_shutter */ },
	{ OMAP44XX_CTRL_BASE + CP(CAM_STROBE),
			 OFF_EN | OFF_PD | OFF_OUT_PTD | M0  /* cam_strobe */ },
	{ OMAP44XX_CTRL_BASE + CP(CAM_GLOBALRESET),
		      PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3  /* gpio_83 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_CLK),
	   PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_STP),
			   OFF_EN | OFF_OUT_PTD | M4  /* usbb1_ulpiphy_stp */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DIR),
		 IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dir */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_NXT),
		 IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_nxt */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT0),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat0 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT1),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat1 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT2),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat2 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT3),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat3 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT4),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat4 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT5),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat5 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT6),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat6 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT7),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat7 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_HSIC_DATA),
		   IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* usbb1_hsic_data */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_HSIC_STROBE),
		 IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* usbb1_hsic_strobe */ },
	{ OMAP44XX_CTRL_BASE + CP(USBC1_ICUSB_DP),
					       IEN | M0  /* usbc1_icusb_dp */ },
	{ OMAP44XX_CTRL_BASE + CP(USBC1_ICUSB_DM),
					       IEN | M0  /* usbc1_icusb_dm */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_CLK),
			    PTU | OFF_EN | OFF_OUT_PTD | M0  /* sdmmc1_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_CMD),
		  PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_cmd */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT0),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat0 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT1),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat1 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT2),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat2 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT3),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat3 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT4),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat4 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT5),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat5 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT6),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat6 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT7),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat7 */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP2_CLKX),
		   IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_mcbsp2_clkx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP2_DR),
			 IEN | OFF_EN | OFF_OUT_PTD | M0  /* abe_mcbsp2_dr */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP2_DX),
			       OFF_EN | OFF_OUT_PTD | M0  /* abe_mcbsp2_dx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP2_FSX),
		    IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_mcbsp2_fsx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP1_CLKX),
					   IEN | M1  /* abe_slimbus1_clock */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP1_DR),
					    IEN | M1  /* abe_slimbus1_data */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP1_DX),
			       OFF_EN | OFF_OUT_PTD | M0  /* abe_mcbsp1_dx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP1_FSX),
		    IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_mcbsp1_fsx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_PDM_UL_DATA),
	     PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_pdm_ul_data */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_PDM_DL_DATA),
	     PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_pdm_dl_data */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_PDM_FRAME),
	       PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_pdm_frame */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_PDM_LB_CLK),
	      PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_pdm_lb_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_CLKS),
		    PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_clks */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_DMIC_CLK1), M0  /* abe_dmic_clk1 */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_DMIC_DIN1),
						IEN | M0  /* abe_dmic_din1 */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_DMIC_DIN2),
						IEN | M0  /* abe_dmic_din2 */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_DMIC_DIN3),
						IEN | M0  /* abe_dmic_din3 */ },
	{ OMAP44XX_CTRL_BASE + CP(UART2_CTS), PTU | IEN | M0  /* uart2_cts */ },
	{ OMAP44XX_CTRL_BASE + CP(UART2_RTS), M0  /* uart2_rts */ },
	{ OMAP44XX_CTRL_BASE + CP(UART2_RX), PTU | IEN | M0  /* uart2_rx */ },
	{ OMAP44XX_CTRL_BASE + CP(UART2_TX), M0  /* uart2_tx */ },
	{ OMAP44XX_CTRL_BASE + CP(HDQ_SIO), M3  /* gpio_127 */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C1_SCL), PTU | IEN | M0  /* i2c1_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C1_SDA), PTU | IEN | M0  /* i2c1_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C2_SCL), PTU | IEN | M0  /* i2c2_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C2_SDA), PTU | IEN | M0  /* i2c2_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C3_SCL), PTU | IEN | M0  /* i2c3_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C3_SDA), PTU | IEN | M0  /* i2c3_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C4_SCL), PTU | IEN | M0  /* i2c4_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C4_SDA), PTU | IEN | M0  /* i2c4_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CLK),
			IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi1_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_SOMI),
		       IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi1_somi */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_SIMO),
		       IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi1_simo */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CS0),
		  PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi1_cs0 */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CS1),
		  PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M3  /* mcspi1_cs1 */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CS2),
			      PTU | OFF_EN | OFF_OUT_PTU | M3  /* gpio_139 */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CS3), PTU | IEN | M3  /* gpio_140 */ },
	{ OMAP44XX_CTRL_BASE + CP(UART3_CTS_RCTX),
					       PTU | IEN | M0  /* uart3_tx */ },
	{ OMAP44XX_CTRL_BASE + CP(UART3_RTS_SD), M0  /* uart3_rts_sd */ },
	{ OMAP44XX_CTRL_BASE + CP(UART3_RX_IRRX), IEN | M0  /* uart3_rx */ },
	{ OMAP44XX_CTRL_BASE + CP(UART3_TX_IRTX), M0  /* uart3_tx */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_CLK),
		      PTU | IEN | OFF_EN | OFF_OUT_PTD | M0  /* sdmmc5_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_CMD),
		  PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_cmd */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_DAT0),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_dat0 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_DAT1),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_dat1 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_DAT2),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_dat2 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_DAT3),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_dat3 */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI4_CLK),
			IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi4_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI4_SIMO),
		       IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi4_simo */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI4_SOMI),
		       IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi4_somi */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI4_CS0),
		  PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi4_cs0 */ },
	{ OMAP44XX_CTRL_BASE + CP(UART4_RX), IEN | M0  /* uart4_rx */ },
	{ OMAP44XX_CTRL_BASE + CP(UART4_TX), M0  /* uart4_tx */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_CLK),
						     IEN | M3  /* gpio_157 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_STP),
						IEN | M5  /* dispc2_data23 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DIR),
						IEN | M5  /* dispc2_data22 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_NXT),
						IEN | M5  /* dispc2_data21 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT0),
						IEN | M5  /* dispc2_data20 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT1),
						IEN | M5  /* dispc2_data19 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT2),
						IEN | M5  /* dispc2_data18 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT3),
						IEN | M5  /* dispc2_data15 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT4),
						IEN | M5  /* dispc2_data14 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT5),
						IEN | M5  /* dispc2_data13 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT6),
						IEN | M5  /* dispc2_data12 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT7),
						IEN | M5  /* dispc2_data11 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_HSIC_DATA),
			      PTD | OFF_EN | OFF_OUT_PTU | M3  /* gpio_169 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_HSIC_STROBE),
			      PTD | OFF_EN | OFF_OUT_PTU | M3  /* gpio_170 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TX0), PTD | IEN | M3  /* gpio_171 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TY0),
				OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_col1 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TX1),
				OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_col2 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TY1),
				OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_col3 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TX2), PTU | IEN | M3  /* gpio_0 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TY2), PTU | IEN | M3  /* gpio_1 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RX0),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row0 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RY0),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row1 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RX1),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row2 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RY1),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row3 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RX2),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row4 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RY2),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row5 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBA0_OTG_CE),
		 PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M0  /* usba0_otg_ce */ },
	{ OMAP44XX_CTRL_BASE + CP(USBA0_OTG_DP),
		      IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* usba0_otg_dp */ },
	{ OMAP44XX_CTRL_BASE + CP(USBA0_OTG_DM),
		      IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* usba0_otg_dm */ },
	{ OMAP44XX_CTRL_BASE + CP(FREF_CLK1_OUT), M0  /* fref_clk1_out */ },
	{ OMAP44XX_CTRL_BASE + CP(FREF_CLK2_OUT),
					       PTD | IEN | M3  /* gpio_182 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_NIRQ1), PTU | IEN | M0  /* sys_nirq1 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_NIRQ2), M7  /* sys_nirq2 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT0), PTU | IEN | M3  /* gpio_184 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT1), M3  /* gpio_185 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT2), PTD | IEN | M3  /* gpio_186 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT3), M3  /* gpio_187 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT4), M3  /* gpio_188 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT5), PTD | IEN | M3  /* gpio_189 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU0), IEN | M0  /* dpm_emu0 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU1), IEN | M0  /* dpm_emu1 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU2), IEN | M0  /* dpm_emu2 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU3), IEN | M5  /* dispc2_data10 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU4), IEN | M5  /* dispc2_data9 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU5), IEN | M5  /* dispc2_data16 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU6), IEN | M5  /* dispc2_data17 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU7), IEN | M5  /* dispc2_hsync */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU8), IEN | M5  /* dispc2_pclk */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU9), IEN | M5  /* dispc2_vsync */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU10), IEN | M5  /* dispc2_de */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU11), IEN | M5  /* dispc2_data8 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU12), IEN | M5  /* dispc2_data7 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU13), IEN | M5  /* dispc2_data6 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU14), IEN | M5  /* dispc2_data5 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU15), IEN | M5  /* dispc2_data4 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU16), M3  /* gpio_27 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU17), IEN | M5  /* dispc2_data2 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU18), IEN | M5  /* dispc2_data1 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU19), IEN | M5  /* dispc2_data0 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SIM_IO), IEN | M0  /* sim_io */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SIM_CLK), M0  /* sim_clk */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SIM_RESET), M0  /* sim_reset */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SIM_CD),
						 PTU | IEN | M0  /* sim_cd */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SIM_PWRCTRL),
							M0  /* sim_pwrctrl */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SR_SCL),
						 PTU | IEN | M0  /* sr_scl */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SR_SDA),
						 PTU | IEN | M0  /* sr_sda */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_XTAL_IN), M0  /* # */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_SLICER_IN),
						     M0  /* fref_slicer_in */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK_IOREQ),
						     M0  /* fref_clk_ioreq */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_CLK0_OUT),
						    M2  /* sys_drm_msecure */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK3_REQ),
						      PTU | IEN | M0  /* # */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_CLK3_OUT),
						      M0  /* fref_clk3_out */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK4_REQ),
						      PTU | IEN | M0  /* # */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_CLK4_OUT), M0  /* # */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SYS_32K), IEN | M0  /* sys_32k */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SYS_NRESPWRON),
						      M0  /* sys_nrespwron */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SYS_NRESWARM),
						       M0  /* sys_nreswarm */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SYS_PWR_REQ),
						  PTU | M0  /* sys_pwr_req */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SYS_PWRON_RESET),
							  M3  /* gpio_wk29 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SYS_BOOT6),
						     IEN | M3  /* gpio_wk9 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SYS_BOOT7),
						    IEN | M3  /* gpio_wk10 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK3_REQ),
							   M3 /* gpio_wk30 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK4_REQ), M3 /* gpio_wk7 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_CLK4_OUT), M3 /* gpio_wk8 */ },
};

/**********************************************************
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers
 *              specific to the hardware. Many pins need
 *              to be moved from protect to primary mode.
 *********************************************************/
void set_muxconf_regs(void)
{
	int n;

	for (n = 0; n < sizeof omap4panda_mux / sizeof omap4panda_mux[0]; n++)
		__raw_writew(omap4panda_mux[n].value, omap4panda_mux[n].ads);
}

/******************************************************************************
 * Routine: update_mux()
 * Description:Update balls which are different between boards.  All should be
 *             updated to match functionality.  However, I'm only updating ones
 *             which I'll be using for now.  When power comes into play they
 *             all need updating.
 *****************************************************************************/
void update_mux(u32 btype, u32 mtype)
{
	/* REVISIT  */
}

void board_hang(void)
{
	spam_leds();
}

int nand_init(void)
{
	return 0;
}
