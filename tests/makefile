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

all: object docs

docs: ../pdp object/test.gout
	@echo Producing Document...
	@$(DEBUG_FUNC) ../pdp -iobject/test.gout source/test_file.md

object/test.gout: $(OBJECT_FILES) ../pdsl
	@echo Linking Documents...
	@$(DEBUG_FUNC) ../pdsl $(OBJECT_FILES) -o object/test.gout

object/%.h.pdso : include/%.h ../pdsc
	@echo Compiling headers...
	@$(DEBUG_FUNC) ../pdsc -o $@ $< $(DOC_COMPILE_FLAGS)

object/%.c.pdso : source/%.c ../pdsc
	@echo Compiling Sources...
	@$(DEBUG_FUNC) ../pdsc -o $@ $< $(DOC_COMPILE_FLAGS)

object:
	@$(MKDIR) object 

clean:
	@$(RM_F) output
	@$(RM_F) object
