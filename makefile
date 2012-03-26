#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
# Name  : makefile
# Desc  : This is the makefile for the document generator project.
#    
# Author: pantoine
# Date  : 06/11/2011 12:01:21
#---------------------------------------------------------------------------------
#                     Copyright (c) 2011 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

.PHONY: tests

export CC ?= gcc
export MKDIR ?= mkdir
export RM_F ?= rm -rf
export CFLAGS = -ansi -pedantic -funsigned-char

ifdef DEBUG
CFLAGS += -g
export DEBUG_FUNC = gdb --args
endif

ifdef VALGRIND
export DEBUG_FUNC = valgrind
endif

export PATH_SEPARATOR = 0x2f

ifeq ("$(MAKECMDGOALS)","release")
ifeq ("$(VERSION_NAME)","")
$(error You must specify the version name for the release)
endif
endif

SPECIAL_FILES = source/atoms.c source/symbols.c source/document_linker.c source/document_source_compiler.c source/document_processor.c
HEADER_FILES = $(wildcard include/*.h)
SOURCE_FILES = source/atoms.c source/symbols.c $(filter-out $(SPECIAL_FILES),$(wildcard source/*.c))
OBJECT_FILES = $(subst source,object,$(subst .c,.o,$(SOURCE_FILES)))

all: exes tests

exes: pdsc pdsl

pdsc : object $(OBJECT_FILES) source/document_source_compiler.c
	@$(CC) $(CFLAGS) source/document_source_compiler.c -o pdsc $(OBJECT_FILES) -I include -DPATH_SEPARATOR="$(PATH_SEPARATOR)"

pdsl : object $(OBJECT_FILES) source/document_linker.c
	@$(CC) $(CFLAGS) source/document_linker.c -o pdsl $(OBJECT_FILES) -I include -DPATH_SEPARATOR="$(PATH_SEPARATOR)"

pdp : object $(OBJECT_FILES) source/document_processor.c
	@$(CC) $(CFLAGS) source/document_processor.c -o pdp $(OBJECT_FILES) -I include -DPATH_SEPARATOR="$(PATH_SEPARATOR)"

tests: pdsc pdsl pdp
	@$(MAKE) -C tests

include/symbols.h source/symbols.c: source/symbols.list
	@buildgraph -q source/symbols.list source/symbols -h include/ -p "SYMBOLS_"

include/atoms.h source/atoms.c: source/atoms.list
	@buildgraph -q source/atoms.list source/atoms -h include/ -p "ATOM_"

object/%.o : source/%.c $(HEADER_FILES)
	@$(CC) $(CFLAGS) -c -o $@ $< -I include -DPATH_SEPARATOR="$(PATH_SEPARATOR)"

object:
	@$(MKDIR) object 

release:
	git checkout master^0
	git reset --soft release
	git commit -m "$(VERSION_NAME)"
	git branch new_temp
	git checkout new_temp
	git branch -M release
	git tag $(VERSION_NAME)
	git checkout master

clean:
	@$(RM) source/atoms.c
	@$(RM) include/atoms.h
	@$(RM) object/*
	@$(MAKE) -C tests clean
	@$(RM) pdsc
	@$(RM) pdsl
