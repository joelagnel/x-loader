#
# (C) Copyright 2004-2006, Texas Instruments, <www.ti.com>
# Jian Zhang <jzhang@ti.com>
#
# (C) Copyright 2000-2004
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

HOSTARCH := $(shell uname -m | \
	sed -e s/i.86/i386/ \
	    -e s/sun4u/sparc64/ \
	    -e s/arm.*/arm/ \
	    -e s/sa110/arm/ \
	    -e s/powerpc/ppc/ \
	    -e s/macppc/ppc/)

HOSTOS := $(shell uname -s | tr A-Z a-z | \
	    sed -e 's/\(cygwin\).*/cygwin/')

export	HOSTARCH

# Deal with colliding definitions from tcsh etc.
VENDOR=

#########################################################################
#
# X-loader build supports producing a object files to the separate external
# directory. Two use cases are supported:
#
# 1) Add O= to the make command line
# 'make O=/tmp/build all'
#
# 2) Set environement variable BUILD_DIR to point to the desired location
# 'export BUILD_DIR=/tmp/build'
# 'make'
#
# Command line 'O=' setting overrides BUILD_DIR environent variable.
#
# When none of the above methods is used the local build is performed and
# the object files are placed in the source directory.
#

ifdef O
ifeq ("$(origin O)", "command line")
BUILD_DIR := $(O)
endif
endif

ifneq ($(BUILD_DIR),)
saved-output := $(BUILD_DIR)

# Attempt to create a output directory.
$(shell [ -d ${BUILD_DIR} ] || mkdir -p ${BUILD_DIR})

# Verify if it was successful.
BUILD_DIR := $(shell cd $(BUILD_DIR) && /bin/pwd)
$(if $(BUILD_DIR),,$(error output directory "$(saved-output)" does not exist))
endif # ifneq ($(BUILD_DIR),)

OBJTREE		:= $(if $(BUILD_DIR),$(BUILD_DIR),$(CURDIR))
SRCTREE		:= $(CURDIR)
TOPDIR		:= $(SRCTREE)
LNDIR		:= $(OBJTREE)
export TOPDIR SRCTREE OBJTREE

MKCONFIG	:= $(SRCTREE)/mkconfig
export MKCONFIG

ifneq ($(OBJTREE),$(SRCTREE))
REMOTE_BUILD	:= 1
export REMOTE_BUILD
endif

# $(obj) and (src) are defined in config.mk but here in main Makefile
# we also need them before config.mk is included which is the case for
# some targets like unconfig, clean, clobber, distclean, etc.
ifneq ($(OBJTREE),$(SRCTREE))
obj := $(OBJTREE)/
src := $(SRCTREE)/
else
obj :=
src :=
endif
export obj src

# Make sure CDPATH settings don't interfere
unexport CDPATH

#########################################################################

ifeq ($(obj)include/config.mk,$(wildcard $(obj)include/config.mk))
# load ARCH, BOARD, and CPU configuration
include $(obj)include/config.mk
export	ARCH CPU BOARD VENDOR
# load other configuration
include $(TOPDIR)/config.mk

ifndef CROSS_COMPILE
CROSS_COMPILE = arm-none-linux-gnueabi-
#CROSS_COMPILE = arm-linux-
export	CROSS_COMPILE
endif

#########################################################################
# X-LOAD objects....order is important (i.e. start must be first)

OBJS  = cpu/$(CPU)/start.o
 
OBJS := $(addprefix $(obj),$(OBJS))

LIBS += board/$(BOARDDIR)/lib$(BOARD).a
LIBS += cpu/$(CPU)/lib$(CPU).a
LIBS += lib/lib$(ARCH).a
LIBS += fs/fat/libfat.a
LIBS += disk/libdisk.a
LIBS += drivers/libdrivers.a
LIBS += common/libcommon.a

LIBS := $(addprefix $(obj),$(sort $(LIBS)))
.PHONY : $(LIBS)

# Add GCC lib
PLATFORM_LIBS += -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

SUBDIRS	=  

__OBJS := $(subst $(obj),,$(OBJS))
__LIBS := $(subst $(obj),,$(LIBS))

#########################################################################
#########################################################################

ALL = $(obj)x-load.bin.ift $(obj)System.map

all ift:	$(ALL)

$(obj)x-load.bin.ift: $(obj)signGP $(obj)System.map $(obj)x-load.bin
	$(obj)./signGP $(obj)x-load.bin $(TEXT_BASE) $(CONFIG_HEADER)
	cp $(obj)x-load.bin.ift $(obj)MLO
 
$(obj)x-load.bin:	$(obj)x-load
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@
 
$(obj)x-load:	$(OBJS) $(LIBS) $(LDSCRIPT)
		UNDEF_SYM=`$(OBJDUMP) -x $(LIBS) |sed  -n -e 's/.*\(__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
		cd $(LNDIR) && $(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \
			--start-group $(__LIBS) --end-group $(PLATFORM_LIBS) \
			-Map x-load.map -o x-load

$(OBJS):
		$(MAKE) -C cpu/$(CPU) $(if $(REMOTE_BUILD),$@,$(notdir $@))

$(LIBS):
		$(MAKE) -C $(dir $(subst $(obj),,$@))
  
$(obj)System.map:	$(obj)x-load
		@$(NM) $< | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		sort > $(obj)System.map

oneboot:	$(obj)x-load.bin
		scripts/mkoneboot.sh

$(obj)signGP:	scripts/signGP.c
		$(HOSTCC) $(HOSTCFLAGS) -o $(obj)signGP  $<

#########################################################################
else
all $(obj)x-load $(obj)x-load.bin oneboot depend dep $(obj)System.map:
	@echo "System not configured - see README" >&2
	@ exit 1
endif

#########################################################################

unconfig:
	rm -f $(obj)include/config.h $(obj)include/config.mk

#========================================================================
# ARM
#========================================================================
#########################################################################
## OMAP2 (ARM1136) Systems
#########################################################################

omap2420h4_config :    unconfig
	@$(MKCONFIG) $(@:_config=) arm arm1136 omap2420h4

omap2430sdp_config :    unconfig
	@$(MKCONFIG) $(@:_config=) arm arm1136 omap2430sdp

#########################################################################
## OMAP3 (ARM-CortexA8) Systems
#########################################################################
omap3430sdp_config :    unconfig
	@$(MKCONFIG) $(@:_config=) arm omap3 omap3430sdp

omap3430labrador_config :    unconfig
	@$(MKCONFIG) $(@:_config=) arm omap3 omap3430labrador

omap3evm_config :	unconfig
	@$(MKCONFIG) $(@:_config=) arm omap3 omap3evm

overo_config :	unconfig
	@$(MKCONFIG) $(@:_config=) arm omap3 overo

omap3530beagle_config :    unconfig
	@$(MKCONFIG) $(@:_config=) arm omap3 omap3530beagle

igep00x0_config :    unconfig
	@$(MKCONFIG) $(@:_config=) arm omap3 igep00x0
	
#########################################################################
## OMAP4 (ARM-CortexA9) Systems
#########################################################################
omap4430panda_config :    unconfig
	@$(MKCONFIG) $(@:_config=) arm omap4 omap4430panda 1

#########################################################################

clean:
	find $(OBJTREE) -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'  -o -name '*.a'  \) -print \
		| xargs rm -f
 
clobber:	clean
	find $(OBJTREE) -type f \
		\( -name .depend -o -name '*.srec' -o -name '*.bin' \) \
		-print \
		| xargs rm -f
	rm -f $(OBJS) $(obj)*.bak $(obj)tags $(obj)TAGS
	rm -fr $(obj)*.*~
	rm -f $(obj)x-load $(obj)x-load.map $(ALL) $(obj)x-load.bin.ift $(obj)signGP $(obj)MLO
	rm -f $(obj)include/asm/proc $(obj)include/asm/arch

ifeq ($(OBJTREE),$(SRCTREE))
mrproper \
distclean:	clobber unconfig
else
mrproper \
distclean:	clobber unconfig
	rm -rf $(obj)*
endif

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	gtar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

#########################################################################
