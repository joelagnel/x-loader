#
# (C) Copyright 2000
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG -DDEBUG

LIB	= $(obj)libdrivers.a
COBJS	= serial.o ns16550.o omap24xx_i2c.o

ifeq ($(BOARD), omap3430sdp)
COBJS 	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), omap3430labrador)
COBJS 	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), omap3530beagle)
COBJS 	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), omap3evm)
COBJS 	+= k9f1g08r0a.o onenand.o
endif

ifeq ($(BOARD), overo)
COBJS 	+= k9f1g08r0a.o
endif

ifeq ($(BOARD), omap2420h4) 
COBJS 	+= k9k1216.o
endif

ifeq ($(BOARD), omap2430sdp)
COBJS    += k9k1216.o
endif

ifeq ($(BOARD), igep00x0)
COBJS    += onenand.o
endif

## Disabled for now:
##	  cs8900.o ct69000.o dataflash.o dc2114x.o ds1722.o \
##	  lan91c96.o mw_eeprom.o natsemi.o \
##	  smc91111.o smiLynxEM.o spi_eeprom.o sym53c8xx.o \
##

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
