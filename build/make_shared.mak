#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: make_shared
#   @desc: This makefile will make a shared object.
#
# @author: pantoine
#   @date: 04/07/2012 21:59:09
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

TARGET_DIR			?= $(BUILD_ROOT)$(GOOD_SLASH)output
TARGET_NAME			= $(lastword $(subst /, ,$(CURDIR)))
SHARED_NAME			= $(TARGET_NAME)$(SHARED_LIBRARY_SUFFIX)
SHARED_OBJECT		= $(TARGET_DIR)$(GOOD_SLASH)$(LIBRARY_PREFIX)$(TARGET_NAME)$(SHARED_LIBRARY_SUFFIX)

C_SOURCE_FILES		= $(wildcard source/*.c)
CPP_SOURCE_FILES	= $(wildcard source/*.cpp)
OBJECT_FILES		= $(subst source/,object/,$(subst .c,.o,$(C_SOURCE_FILES)) $(subst .cpp,.o,$(CPP_SOURCE_FILES)))
BUILD_OBJECTS		= $(addprefix $(CURDIR)$(GOOD_SLASH),$(OBJECT_FILES))
DEPENDENCY_LIBS		= $(addsuffix $(LIBRARY_SUFFIX),$(addprefix $(BUILD_ROOT)/output/lib,$(DEPENDENCES)))

include $(BUILD_ROOT)/build/dependency.mak

#---------------------------------------------------------------------------------
# Generic build targets
#---------------------------------------------------------------------------------
.PHONY:	$(DEPENDENCES)

build: object $(DEPENDENCES) $(SHARED_OBJECT)

clean: clean_deps
	-@$(RMSUBDIR) object
	-@$(RM_F) $(SHARED_OBJECT)

#---------------------------------------------------------------------------------
# Local build targets
#---------------------------------------------------------------------------------
$(DEPENDENCES):
	@$(MAKE) -C $(BUILD_ROOT)/$@ build

object:
	@$(MKDIR) object

$(SHARED_OBJECT): make_$(TARGET_NAME).deps $(BUILD_OBJECTS) $(DEPENDENCY_LIBS)
	@$(CXX) -fPIC -shared -Wl,-soname,$(SHARED_NAME) -o $@ $(BUILD_OBJECTS) $(INCLUDE_LIBS) $(DEPENDENCY_LIBS)

$(CURDIR)/object/%.o: $(CURDIR)/source/%.c
	$(CC) $(CFLAGS) -c -fPIC -o $@ $< -I $(CURDIR)/include $(INCLUDE_DIRS)

$(CURDIR)/object/%.o: $(CURDIR)/source/%.cpp
	@$(CXX) $(CFLAGS) -c -fPIC -o $@ $< -I $(CURDIR)/include $(INCLUDE_DIRS)

