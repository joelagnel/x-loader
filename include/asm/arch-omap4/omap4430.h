/*
 * (C) Copyright 2006-2009
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
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

#ifndef _OMAP4430_SYS_H_
#define _OMAP4430_SYS_H_

#include <asm/arch/sizes.h>

/*
 * 4430 specific Section
 */

/* Stuff on L3 Interconnect */
#define SMX_APE_BASE			0x68000000

/* L3 Firewall */
#define A_REQINFOPERM0		(SMX_APE_BASE + 0x05048)
#define A_READPERM0		(SMX_APE_BASE + 0x05050)
#define A_WRITEPERM0		(SMX_APE_BASE + 0x05058)

/* GPMC */
#define OMAP44XX_GPMC_BASE		(0x50000000)

/* DMM */
#define OMAP44XX_DMM_BASE		0x4E000000

/* SMS */
#define OMAP44XX_SMS_BASE               0x6C000000

/* SDRC */
#define OMAP44XX_SDRC_BASE              0x6D000000


/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */
#define OMAP44XX_CORE_L4_IO_BASE	0x4A000000

#define OMAP44XX_WAKEUP_L4_IO_BASE	0x4A300000

#define OMAP44XX_L4_PER			0x48000000

#define OMAP44XX_L4_IO_BASE		OMAP44XX_CORE_L4_IO_BASE

/* CONTROL */
#define OMAP44XX_CTRL_GEN_CORE_BASE	(OMAP44XX_L4_IO_BASE+0x2000)
#define OMAP44XX_CTRL_ID_CODE		(OMAP44XX_CTRL_GEN_CORE_BASE + 0x204)

#define OMAP44XX_CTRL_BASE		0x4a100000
#define OMAP44XX_WKUP_CTRL_BASE		0x4A31E000

/* DDR */
#define OMAP44XX_CONTROL_LPDDR2IO1_0	(OMAP44XX_CTRL_BASE + 0x638)
#define OMAP44XX_CONTROL_LPDDR2IO1_1	(OMAP44XX_CTRL_BASE + 0x63c)
#define OMAP44XX_CONTROL_LPDDR2IO1_2	(OMAP44XX_CTRL_BASE + 0x640)
#define OMAP44XX_CONTROL_LPDDR2IO1_3	(OMAP44XX_CTRL_BASE + 0x644)
#define OMAP44XX_CONTROL_LPDDR2IO2_0	(OMAP44XX_CTRL_BASE + 0x648)
#define OMAP44XX_CONTROL_LPDDR2IO2_1	(OMAP44XX_CTRL_BASE + 0x64c)
#define OMAP44XX_CONTROL_LPDDR2IO2_2	(OMAP44XX_CTRL_BASE + 0x650)
#define OMAP44XX_CONTROL_LPDDR2IO2_3	(OMAP44XX_CTRL_BASE + 0x654)

/* eFUSE */
#define OMAP44XX_CONTROL_EFUSE_1	(OMAP44XX_CTRL_BASE + 0x700)
#define OMAP44XX_CONTROL_EFUSE_2	(OMAP44XX_CTRL_BASE + 0x704)
#define OMAP44XX_CONTROL_EFUSE_3	(OMAP44XX_CTRL_BASE + 0x708)
#define OMAP44XX_CONTROL_EFUSE_4	(OMAP44XX_CTRL_BASE + 0x70c)

/* PRM */
#define OMAP44XX_PRM_VC_VAL_BYPASS	(OMAP44XX_WAKEUP_L4_IO_BASE + 0x7ba0)
#define OMAP44XX_PRM_VC_CFG_I2C_MODE	(OMAP44XX_WAKEUP_L4_IO_BASE + 0x7ba8)
#define OMAP44XX_PRM_VC_CFG_I2C_CLK	(OMAP44XX_WAKEUP_L4_IO_BASE + 0x7bac)

/* IRQ */
#define OMAP44XX_PRM_IRQSTATUS_MPU_A9	(OMAP44XX_WAKEUP_L4_IO_BASE + 0x6010)



/* TAP information  dont know for 3430*/
#define OMAP44XX_TAP_BASE	(0x49000000) /*giving some junk for virtio */

/* UART */
#define OMAP44XX_UART1			(OMAP44XX_L4_PER+0x6a000)
#define OMAP44XX_UART2			(OMAP44XX_L4_PER+0x6c000)
#define OMAP44XX_UART3			(OMAP44XX_L4_PER+0x20000)

/* General Purpose Timers */
#define OMAP44XX_GPT1			0x48318000
#define OMAP44XX_GPT2			0x48032000
#define OMAP44XX_GPT3			0x48034000
#define OMAP44XX_GPT4			0x48036000
#define OMAP44XX_GPT5			0x40138000
#define OMAP44XX_GPT6			0x4013A000
#define OMAP44XX_GPT7			0x4013C000
#define OMAP44XX_GPT8			0x4013E000
#define OMAP44XX_GPT9			0x48040000
#define OMAP44XX_GPT10			0x48086000
#define OMAP44XX_GPT11			0x48088000
#define OMAP44XX_GPT12			0x48304000

/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE			(0x4A322000)
#define WD2_BASE			(0x4A314000)
#define WD3_BASE			(0x40130000)

/* GPIO banks */
#define OMAP44XX_GPIO_BASE1		0x4a310000
#define OMAP44XX_GPIO_BASE2		0x48055000

#define OMAP44XX_GPIO_BASE6		0x4805D000

/*      common GPIO offsets */

#define __GPIO_REVISION			0
#define __GPIO_SYSCONFIG		0x10
#define __GPIO_IRQSTATUS_RAW_0		0x24
#define __GPIO_IRQSTATUS_RAW_1		0x28
#define __GPIO_IRQSTATUS_0		0x2c
#define __GPIO_IRQSTATUS_1		0x30
#define __GPIO_IRQSTATUS_SET_0		0x34
#define __GPIO_IRQSTATUS_SET_1		0x38
#define __GPIO_IRQSTATUS_CLR_0		0x3c
#define __GPIO_IRQSTATUS_CLR_1		0x40
#define __GPIO_IRQWAKEN_0		0x44
#define __GPIO_IRQWAKEN_1		0x48
#define __GPIO_SYSSTATUS		0x114
#define __GPIO_IRQSTATUS1		0x118
#define __GPIO_CTRL			0x130
#define __GPIO_OE			0x134
#define __GPIO_DATAIN			0x138
#define __GPIO_DATAOUT			0x13c

/* SCRM */

#define OMAP44XX_SCRM_BASE		0x4a30a000

#define OMAP44XX_SCRM_ALTCLKSRC		(OMAP44XX_SCRM_BASE + 0x110)
#define OMAP44XX_SCRM_AUXCLK1		(OMAP44XX_SCRM_BASE + 0x314)
#define OMAP44XX_SCRM_AUXCLK3		(OMAP44XX_SCRM_BASE + 0x31c)


/* 32KTIMER */
#define SYNC_32KTIMER_BASE		(0x48320000)
#define S32K_CR				(SYNC_32KTIMER_BASE+0x10)

/*
 * SDP4430 specific Section
 */

/*
 *  The 443x's chip selects are programmable.  The mask ROM
 *  does configure CS0 to 0x08000000 before dispatch.  So, if
 *  you want your code to live below that address, you have to
 *  be prepared to jump though hoops, to reset the base address.
 *  Same as in SDP4430
 */
#ifdef CONFIG_OMAP44XX
/* base address for indirect vectors (internal boot mode) */
#define SRAM_OFFSET0                    0x40000000
#define SRAM_OFFSET1                    0x00300000
#define SRAM_OFFSET2                    0x0000D000
#define SRAM_OFFSET3                    0x00000800
#define SRAM_VECT_CODE                  (SRAM_OFFSET0|SRAM_OFFSET1|SRAM_OFFSET2|SRAM_OFFSET3)
#define LOW_LEVEL_SRAM_STACK            0x4030DFFC
#endif

#if defined(CONFIG_4430SDP)
/* FPGA on Debug board.*/
# define ETH_CONTROL_REG			(DEBUG_BASE+0x30b)
# define LAN_RESET_REGISTER		(DEBUG_BASE+0x1c)

# define DIP_SWITCH_INPUT_REG2		(DEBUG_BASE+0x60)
# define LED_REGISTER			(DEBUG_BASE+0x40)
# define FPGA_REV_REGISTER		(DEBUG_BASE+0x10)
# define EEPROM_MAIN_BRD			(DEBUG_BASE+0x10000+0x1800)
# define EEPROM_CONN_BRD			(DEBUG_BASE+0x10000+0x1900)
# define EEPROM_UI_BRD			(DEBUG_BASE+0x10000+0x1A00)
# define EEPROM_MCAM_BRD			(DEBUG_BASE+0x10000+0x1B00)
# define ENHANCED_UI_EE_NAME		"750-2075"
#endif

#endif  /* _OMAP4430_SYS_H_ */
