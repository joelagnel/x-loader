/*
 * (C) Copyright 2010
 * ISEE 2007 SL <www.iseebcn.com>
 *
 * X-Loader Configuration settings for the IGEP0020 board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* serial printf facility takes about 3.5K */
#define CFG_PRINTF

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMCORTEXA8	1	/* This is an ARM V7 CPU core */
#define CONFIG_OMAP		1	/* in a TI OMAP core */
#define CONFIG_OMAP34XX		1	/* which is a 34XX */
#define CONFIG_OMAP3430		1	/* which is in a 3430 */
#define CONFIG_OMAP3_IGEP0020	1	/* working with IGEP0020 */

/* Enable the below macro if MMC boot support is required */
#define CONFIG_MMC	1
#if defined(CONFIG_MMC)
	#define CFG_CMD_MMC		1
	#define CFG_CMD_FAT		1
	#define CFG_I2C_SPEED		100000
	#define CFG_I2C_SLAVE		1
	#define CFG_I2C_BUS		0
	#define CFG_I2C_BUS_SELECT	1
	#define CONFIG_DRIVER_OMAP34XX_I2C 1
#endif

#include <asm/arch/cpu.h>        /* get chip and board defs */

/* Clock Defines */
#define V_OSCK	26000000  /* Clock output from T2 */

#if (V_OSCK > 19200000)
#define V_SCLK	(V_OSCK >> 1)
#else
#define V_SCLK	V_OSCK
#endif

#define PRCM_CLK_CFG2_332MHZ	1	/* VDD2=1.15v - 166MHz DDR */
#define PRCM_PCLK_OPP2			1	/* ARM=381MHz - VDD1=1.20v */

/* Memory type */
#define CONFIG_SDRAM_M65KX002AM 1	/* 2 dice of 2Gb, DDR x32, 4KB page */

/* The actual register values are defined in u-boot- mem.h */
/* SDRAM Bank Allocation method */
#define SDRC_R_B_C		1

#define OMAP34XX_GPMC_CS0_SIZE GPMC_SIZE_128M

#ifdef CFG_PRINTF

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	-4
#define CFG_NS16550_CLK		48000000
#define CFG_NS16550_COM3	OMAP34XX_UART3

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1		3	/* use UART3 */
#define CONFIG_CONS_INDEX	3

#define CONFIG_BAUDRATE		115200
#define CFG_PBSIZE		256

#endif /* CFG_PRINTF */

/*
 * Miscellaneous configurable options
 */
#define CFG_LOADADDR		0x80008000

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024) /* regular stack */

/*
 * Board oneNAND Info.
 */
#define CFG_ONENAND		1

#define ONENAND_BASE	ONENAND_MAP
#define ONENAND_ADDR	ONENAND_BASE

#define ONENAND_START_BLOCK 	4	 /* 0x00080000 */
#define ONENAND_END_BLOCK	6	 /* 0x00200000 */
#define ONENAND_PAGE_SIZE	2048     /* 2KB */
#define ONENAND_BLOCK_SIZE	0x20000  /* 128KB */

#endif /* __CONFIG_H */
