#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: linux_native
#   @desc: This file configures the build tools for the doc_gen build.
#
# @author: pantoine
#   @date: 05/07/2012 09:24:47
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

export LIBPREFIX = lib
export LIBSUFFIX = .a
export EXESUFFIX = .out

export AROPTS = -r -c
export CFLAGS = -ansi -pedantic -funsigned-char -Wno-long-long -Wmissing-field-initializers -Wmissing-declarations 
export COPTS_DEBUG = -g -O0
export COPTS_RUN = -g
export COPTS_RELEASE = -O2 -Werror
export LINK_OUT = -o

ifdef DEBUG
export CFLAGS = $(COPTS_DEBUG)
else
export CFLAGS = $(COPTS_RUN)
endif

export AR = ar
export AS = as
export CC = gcc
export CXX = g++
export LINK = ld
export RANLIB = ranlib

export TARGET_PLATFORM = linux_$(shell uname -m)

