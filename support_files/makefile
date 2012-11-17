#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
# Name  : makefile
# Desc  : This make file controls the support files.
#         mostly this makefile is used to copy the support file to where they
#         need to be.
#
#         If $(SUPPORT_FILE_TARGET) is specified then the files are installed 
#         there, else they default to the object directory in the root of the
#         build tree.
#
# Author: peterantoine
# Date  : 07/09/2012 12:06:03
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# work out the files that need updating.
#---------------------------------------------------------------------------------
SUPPORT_FILE_TARGET ?= $(BUILD_ROOT)$(GOOD_SLASH)output
SOURCE_DIRECTORIES	= $(filter-out makefile,$(subst $(CURDIR)/,,$(wildcard $(CURDIR)/*)))
TARGET_DIRECTORIES	= $(addprefix $(SUPPORT_FILE_TARGET)$(GOOD_SLASH),$(SOURCE_DIRECTORIES))
TARGET_FILES		= $(addprefix $(SUPPORT_FILE_TARGET)$(GOOD_SLASH),$(foreach dir,$(SOURCE_DIRECTORIES),$(wildcard $(dir)/*)))

#---------------------------------------------------------------------------------
# build targets
#---------------------------------------------------------------------------------
clean:
dry_run_release:
source_release:
binary_release:

docs: build

tests: build

build: $(TARGET_DIRECTORIES) $(TARGET_FILES)

$(TARGET_DIRECTORIES):
	@$(MKDIR) $@

$(SUPPORT_FILE_TARGET)$(GOOD_SLASH)%: %
	@$(COPY) $< $@

