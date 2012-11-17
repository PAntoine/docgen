#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: makefile
#   @desc: This makefile will build the manpage plugin for generating
#          documentation in the manpage (groff) format.
#
# @author: 
#   @date: 06/07/2012 19:09:53
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

DEPENDENCES = common
INCLUDE_DIRS += -I $(BUILD_ROOT)/common/include
TARGET_DIR = $(PLUGINS_TARGET_DIR)

#---------------------------------------------------------------------------------
# The build includes
#---------------------------------------------------------------------------------
include $(BUILD_ROOT)/build/make_shared.mak
include $(BUILD_ROOT)/build/make_docs.mak
