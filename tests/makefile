#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
# Name  : makefile
# Desc  : This is the makefile for the tests.
#    
# Author: pantoine
# Date  : 06/11/2011 12:01:21
#---------------------------------------------------------------------------------
#                     Copyright (c) 2011 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

HEADER_FILES = $(wildcard include/*.h)
SOURCE_FILES = $(wildcard source/*.c)
SRC_OBJ_FILES = $(subst source/,object/,$(addsuffix .pdso,$(SOURCE_FILES)))
HDR_OBJ_FILES = $(subst include/,object/,$(addsuffix .pdso,$(HEADER_FILES)))
OBJECT_FILES = $(SRC_OBJ_FILES) $(HDR_OBJ_FILES)

DOC_COMPILE_FLAGS = -DSTATE_ONE=test_one

binary_release: tests
source_release: tests
dry_run_release: tests
build: tests
tests: object docs

docs: object/test.gout
	@echo Producing Document...
	@$(DEBUG_FUNC) pdp -iobject/test.gout -f html,text,manpage -d $(BUILD_ROOT)/output $(CURDIR)/source/test_file.md $(CURDIR)/source/sequence_only.md $(CURDIR)/source/state_machine_only.md
	@$(DEBUG_FUNC) pdp -iobject/test.gout -d $(BUILD_ROOT)/output $(CURDIR)/source/test_markdown.md -DMACRO="walking on air" -DGROUP_MACRO_NAME=config_file_api -DITEM_MACRO_NAME=DGTF_IF_AddString -f text,html,manpage
	@$(DEBUG_FUNC) pdp -iobject/test.gout -d $(BUILD_ROOT)/output $(CURDIR)/source/state_machine_only.md
	@$(DEBUG_FUNC) pdp -iobject/test.gout -d $(BUILD_ROOT)/output $(CURDIR)/source/sequence_only.md
	@$(DEBUG_FUNC) pdp -iobject/test.gout -f html,text,manpage -d $(BUILD_ROOT)/output $(BUILD_ROOT)/docs/specification.md
	@$(DEBUG_FUNC) pdp -iobject/test.gout -d $(BUILD_ROOT)/output $(CURDIR)/source/sequence_only.md -f html,text,manpage 
	@$(DEBUG_FUNC) pdp -iobject/test.gout -d $(BUILD_ROOT)/output $(CURDIR)/source/state_machine_only.md -f html,text,manpage 

object/test.gout: $(OBJECT_FILES) $(BUILD_ROOT)/output/pdsl
	@echo Linking Documents...
	@$(DEBUG_FUNC) pdsl $(OBJECT_FILES) -o object/test.gout

object/%.h.pdso : include/%.h $(BUILD_ROOT)/output/pdsc
	@echo Compiling headers...
	@$(DEBUG_FUNC) pdsc -o $@ $< $(DOC_COMPILE_FLAGS)

object/%.c.pdso : source/%.c $(BUILD_ROOT)/output/pdsc
	@echo Compiling Sources...
	@$(DEBUG_FUNC) pdsc -o $@ $< $(DOC_COMPILE_FLAGS)

object:
	@$(MKDIR) object 
	@$(MKDIR) output
	@$(COPYDIR) $(BUILD_ROOT)/support_files/* $(BUILD_ROOT)/output
	@$(RM) $(BUILD_ROOT)/output/makefile 

clean:
	@$(RMSUBDIR) output
	@$(RMSUBDIR) object
