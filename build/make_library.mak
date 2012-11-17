#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: make_library
#   @desc: This make file will build a make a library.
#
# @author: pantoine
#   @date: 04/07/2012 21:16:04
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

TARGET_DIR			?= $(BUILD_ROOT)$(GOOD_SLASH)output
TARGET_NAME			= $(lastword $(subst /, ,$(CURDIR)))
LIBRARY_OBJECT		= $(TARGET_DIR)$(GOOD_SLASH)$(LIBRARY_PREFIX)$(TARGET_NAME)$(LIBRARY_SUFFIX)

C_SOURCE_FILES		= $(wildcard source/*.c)
CPP_SOURCE_FILES	= $(wildcard source/*.cpp)
OBJECT_FILES		= $(subst source/,object/,$(subst .c,.o,$(C_SOURCE_FILES)) $(subst .cpp,.o,$(CPP_SOURCE_FILES)))
BUILD_OBJECTS		= $(addprefix $(CURDIR)$(GOOD_SLASH),$(OBJECT_FILES))

include $(BUILD_ROOT)/build/dependency.mak

#---------------------------------------------------------------------------------
# Generic build targets
#---------------------------------------------------------------------------------
build: object  $(LIBRARY_OBJECT)

clean: clean_deps
	-@$(RMSUBDIR) object
	-@$(RM_F) $(LIBRARY_OBJECT)

#---------------------------------------------------------------------------------
# Local build targets
#---------------------------------------------------------------------------------
object:
	@$(MKDIR) object

$(LIBRARY_OBJECT): make_$(TARGET_NAME).deps $(BUILD_OBJECTS)
	@$(AR) $(AROPTS) $(@) $(BUILD_OBJECTS)

$(CURDIR)/object/%.o: $(CURDIR)/source/%.c
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $< -I $(CURDIR)/include $(INCLUDE_DIRS)

$(CURDIR)/object/%.o: $(CURDIR)/source/%.cpp
	@$(CXX) $(CFLAGS) -fPIC -c -o $@ $< -I $(CURDIR)/include $(INCLUDE_DIRS)

$(CURDIR)/include/%.h:
	@echo inlcude file $@

