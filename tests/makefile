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
OBJECT_FILES = $(subst source/,object/,$(subst .c,.pdso,$(SOURCE_FILES)))

DOC_COMPILE_FLAGS = -DSTATE_ONE=test_one

all: object docs

docs: $(OBJECT_FILES) ../pdsl
	$(DEBUG_FUNC) ../pdsl $(OBJECT_FILES) -o object/test

object/%.pdso : source/%.c ../pdsc
	$(DEBUG_FUNC) ../pdsc -o $@ $< $(DOC_COMPILE_FLAGS)

object:
	@$(MKDIR) object 

clean:
	@$(RM_F) object
