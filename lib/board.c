/*
 * Copyright (C) 2005 Texas Instruments.
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <part.h>
#include <fat.h>
#include <asm/arch/mem.h>

const char version_string[] =
	"Texas Instruments X-Loader 1.5.0 (" __DATE__ " - " __TIME__ ")";

int print_info(void)
{
#ifdef CFG_PRINTF
        printf("\n\n%s\n", version_string);
#endif
	return 0;
}

/* !!! why is I2C dependent on MMC? */

#ifdef CONFIG_MMC
#ifdef CONFIG_DRIVER_OMAP34XX_I2C
static int init_func_i2c (void)
{
	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);
	return 0;
}
#endif
#endif

typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
#ifdef CFG_NS16550_SERIAL
 	serial_init,		/* serial communications setup */
#endif
	print_info,
  	nand_init,		/* board specific nand init */
#ifdef CONFIG_MMC
#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	init_func_i2c,
#endif
#endif
  	NULL,
};

void start_armboot (void)
{
  	init_fnc_t **init_fnc_ptr;
 	int size;
	uchar *buf;
	int *first_instruction;
#if defined(CFG_ONENAND) || defined(CFG_NAND_K9F1G08R0A)
	int i;
#endif

   	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr)
		if ((*init_fnc_ptr)())
			hang ();

	misc_init_r();

	buf =  (uchar*) CFG_LOADADDR;
	*(int *)buf = 0xffffffff;

#ifdef CONFIG_MMC
	/* first try mmc */
	if (mmc_init(1)) {
		size = file_fat_read("u-boot.bin", buf, 0);
		if (size > 0) {
#ifdef CFG_PRINTF
			printf("Loading u-boot.bin from mmc\n");
#endif
			buf += size;
		}
	}
#endif

	if (buf == (uchar *)CFG_LOADADDR) {
		/* if no u-boot on mmc, try onenand or nand, depending upon sysboot */
		if (get_mem_type() == GPMC_ONENAND){
#ifdef CFG_ONENAND
#ifdef CFG_PRINTF
       			printf("Loading u-boot.bin from onenand\n");
#endif
        		for (i = ONENAND_START_BLOCK; i < ONENAND_END_BLOCK; i++){
        			if (!onenand_read_block(buf, i))
        				buf += ONENAND_BLOCK_SIZE;
        		}
#endif
		} else if (get_mem_type() == GPMC_NAND){
#ifdef CFG_NAND_K9F1G08R0A
#ifdef CFG_PRINTF
       			printf("Loading u-boot.bin from nand\n");
#endif
        		for (i = NAND_UBOOT_START; i < NAND_UBOOT_END; i+= NAND_BLOCK_SIZE){
        			if (!nand_read_block(buf, i))
        				buf += NAND_BLOCK_SIZE; /* advance buf ptr */
        		}
#endif
		}
	}

	/* if u-boot not found on mmc or
         * nand read result is erased data
         * then serial boot 
         */
	first_instruction = (int *)CFG_LOADADDR;
	if((buf == (uchar *)CFG_LOADADDR) || (*first_instruction == 0xffffffff)) {
		printf("u-boot.bin not found or blank nand contents - attempting serial boot . . .\n");
		do_load_serial_bin(CFG_LOADADDR, 115200);
	}

	/* go run U-Boot and never return */
 	((init_fnc_t *)CFG_LOADADDR)();

	/* should never come here */
}

void hang (void)
{
	/* call board specific hang function */
	board_hang();

	/* if board_hang() returns, hang here */
#ifdef CFG_PRINTF
	printf("X-Loader hangs\n");
#endif
	for (;;);
}
