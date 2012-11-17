#*--------------------------------------------------------------------------------
#				Document Generator Project
#					by Peter Antoine
#
#	@file: make_executable
#	@desc: This makefile will build an executable.
#
# @author: pantoine
#	@date: 04/07/2012 21:00:37
#---------------------------------------------------------------------------------
#					  Copyright (c) 2012 Peter Antoine
#							 All rights Reserved.
#					 Released Under the Artistic Licence
#---------------------------------------------------------------------------------

TARGET_DIR			?= $(BUILD_ROOT)$(GOOD_SLASH)output
TARGET_NAME			= $(lastword $(subst /, ,$(CURDIR)))
EXECUABLE_OBJECT	= $(TARGET_DIR)$(GOOD_SLASH)$(TARGET_NAME)$(EXE_SUFFIX)

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

build: object $(DEPENDENCES) $(EXECUABLE_OBJECT)

clean: clean_deps
	-@$(RMSUBDIR) object > $(NUL)
	-@$(RM_F) $(EXECUABLE_OBJECT)

#---------------------------------------------------------------------------------
# Local build targets
#---------------------------------------------------------------------------------
$(DEPENDENCES):
	@$(MAKE) -C $(BUILD_ROOT)/$@ build

object:
	@$(MKDIR) object

$(EXECUABLE_OBJECT): make_$(TARGET_NAME).deps $(BUILD_OBJECTS) $(DEPENDENCY_LIBS)
	@$(CC) $(LDFLAGS) -fPIC -Wl,--start-group $(DEPENDENCY_LIBS) $(BUILD_OBJECTS) -Wl,--end-group -Wl,--export-dynamic $(INCLUDE_LIBS) -o $(EXECUABLE_OBJECT)

$(CURDIR)/object/%.o: $(CURDIR)/source/%.c
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $< -I $(CURDIR)/include $(INCLUDE_DIRS)

$(CURDIR)/object/%.o: $(CURDIR)/source/%.cpp
	@$(CXX) $(CFLAGS) -fPIC -c -o $@ $< -I $(CURDIR)/include $(INCLUDE_DIRS)

