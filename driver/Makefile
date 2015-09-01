##############################################################################
#
#    Copyright (C) 2005 - 2014 by Vivante Corp.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the license, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
###############################################################################

AQROOT=$(PWD)
AQARCH=$(AQROOT)/arch/XAQ2

include $(AQROOT)/config

AQVGARCH=$(AQROOT)/arch/GC350

export AQROOT AQARCH AQVGARCH


ARCH_TYPE         ?= arm
KERNEL_DIR        ?= /home/software/Linux/linux-2.6.21-arm1
CROSS_COMPILE     ?= /home/software/Linux/arm-2007q1/bin/arm-none-linux-gnueabi-
SDK_DIR           ?= $(AQROOT)/build/sdk

export KERNEL_DIR CROSS_COMPILE SDK_DIR

.PHONY: all clean install

all:
	@make -f Kbuild

clean:
	@make -f Kbuild ARCH=$(ARCH_TYPE)  clean

install:
	@make -f Kbuild install

