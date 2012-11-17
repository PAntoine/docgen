#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: platform_config
#   @desc: This file handles the platform configuration.
#
# @author: pantoine
#   @date: 05/07/2012 09:14:38
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

# build os detection.
ifeq (,$(findstring :,$(CURDIR)))
# Build Machine is Linux
 export NUL = /dev/null
 export GOOD_SLASH = $(strip / )
 export BAD_SLASH = $(strip \ )
 export BUILD_OS = linux
 export RM = rm
 export MV = mv
 export RM_F = rm -f
 export COPY = cp
 export COPYDIR = cp -r
 export CD = cd
 export RMSUBDIR = rm -rf
 export MKDIR = mkdir
 export NICE_BUILD_TIME = $(shell date +%Y/%m/%d\ %H:%M:%S)
 export BUILD_TIME = $(shell date +%Y%m%d_%H%M%S)
 export BUILD_DATE = $(shell date +%d%m%Y)
 export BUILD_PLATFORM=linux
 export ECHO=echo
 export EXE_SUFFIX=
 export LIBRARY_PREFIX=lib
 export LIBRARY_SUFFIX=.a
 export SHARED_LIBRARY_SUFFIX=.so
 export MACHINE_NAME = native
 export TEST_EMPTY_STRING = test -z "$$($(1))"
 export CURRENT_BRANCH = $(strip $(shell git branch | grep "*" | sed "s/* //"))
 else
#Build Machine is Windows
 export NUL = nul
 export GOOD_SLASH =$(strip \ )
 export BAD_SLASH = $(strip / )
 export BUILD_OS = windows
 export MV = move
 export RM = del
 export RM_F = del /q
 export COPY = copy
 export COPYDIR = copy /S
 export CD = cd
 export RMSUBDIR = rmdir /Q /S
 export MKDIR = mkdir
 export NATIVE_EXE = .exe
 export BUILD_TIME = $(subst .,,$(subst :,,$(subst /,,$(shell echo %DATE:~6%%DATE:~3,2%%DATE:~0,2%_%TIME:~0,8%))))
 export BUILD_DATE =$(subst /,_,$(shell echo %DATE%))
 export BUILD_PLATFORM=windows
 export ECHO=echo
 export EXE_SUFFIX=.exe
 export LIBRARY_PREFIX=lib
 export LIBRARY_SUFFIX=.lib
 export SHARED_LIBRARY_SUFFIX=.dll
 export MACHINE_NAME = native
endif

export PATH+=:$(BUILD_ROOT)/output

ifdef DEBUG
CFLAGS += -g
export DEBUG_FUNC = gdb -return-child-result -x $(CURDIR)/gdbbatch --args
endif

ifdef VALGRIND
export DEBUG_FUNC = valgrind --track-origins=yes --leak-check=full
endif


