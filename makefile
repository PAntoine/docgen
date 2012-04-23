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
export DEBUG_FUNC = gdb -return-child-result -x $(CURDIR)/gdbbatch --args
endif

ifdef VALGRIND
export DEBUG_FUNC = valgrind
endif

export PATH_SEPARATOR = 0x2f

ifeq ("$(MAKECMDGOALS)","release")

ifeq ("$(VERSION_NAME)","")
$(error You must specify the version name for the release)
endif

#--------------------------------------------------------------------------------
# Workout what files are not wanted to be released.
#
# This might be a little to clever for it's own good. But, what it does is to
# generate all the subdirectories that have to be kept in the list to get the
# release directories. This means that we can use wildcard to get all the 
# effected directories then filter-out the list below then delete the 
# directories.
#
# CALL_MAP  - Starts of the recursive call, but it is only here to allow a
#             single call with a single list to do this.
# MAP       - This is the termination case using the $if.
# SUB_MAP   - This does the work. Recursively calls MAP, after removing the
#             head, then adds the current output list into a new list and
#             appends the head as a new item to that. After that it outputs
#             the current list.
#
# I would leave it alone if you don't get functional programming and the
# car/cdr list processing model.
#--------------------------------------------------------------------------------
RELEASE_LIST = source include tests/source tests/include tests/makefile readme.md makefile gdbbatch
	
SUB_MAP			=	$(call MAP,$(wordlist 2,$(words $(1)),$(1)),$(2)/$(word 2,$(1))) $(2)
MAP				=	$(if $(1),$(call SUB_MAP,$(1),$(2)))
CALL_MAP		=	$(call MAP,$(1),$(firstword $(1)))

RELEASABLE_TREE	= $(foreach name,$(RELEASE_LIST),$(call CALL_MAP,$(subst /, ,$(name))))
NEED_PRUNING	= $(sort $(filter-out $(RELEASE_LIST),$(foreach name,$(RELEASE_LIST),$(call CALL_MAP,$(subst /, ,$(name))))))
PRUNABLE_DIRS	= $(foreach dir_name,$(NEED_PRUNING),$(wildcard $(dir_name)/*)) $(filter-out $(RELEASE_LIST),$(wildcard *))
PRUNE_LIST		= $(filter-out $(RELEASABLE_TREE),$(PRUNABLE_DIRS))

$(info not releasing to following files/dirs: $(PRUNE_LIST))

endif

#--------------------------------------------------------------------------------
# old skool component file list. Makes the compilation easier.
#--------------------------------------------------------------------------------
HEADER_FILES 		= $(wildcard include/*.h)
COMMON_OBJECTS 		= object/atoms.o object/utilities.o object/error_codes.o
LINKER_OBJECTS 		= 
COMPILER_OBJECTS	= object/symbols.o object/input_formats.o object/c_cpp_input_functions.o
PROCESSOR_OBJECTS	= object/supported_formats.o object/text_output_functions.o

#--------------------------------------------------------------------------------
# Build targets.
#--------------------------------------------------------------------------------
all: exes tests

exes: pdsc pdsl pdp

pdsc : object $(COMMON_OBJECTS) $(COMPILER_OBJECTS) source/document_source_compiler.c
	@$(CC) $(CFLAGS) source/document_source_compiler.c -o pdsc $(COMPILER_OBJECTS) $(COMMON_OBJECTS) -I include -DPATH_SEPARATOR="$(PATH_SEPARATOR)"

pdsl : object $(COMMON_OBJECTS) $(LINKER_OBJECTS) source/document_linker.c
	@$(CC) $(CFLAGS) source/document_linker.c -o pdsl $(LINKER_OBJECTS) $(COMMON_OBJECTS) -I include -DPATH_SEPARATOR="$(PATH_SEPARATOR)"

pdp : object $(COMMON_OBJECTS) $(PROCESSOR_OBJECTS) source/document_processor.c
	@$(CC) $(CFLAGS) source/document_processor.c -o pdp $(PROCESSOR_OBJECTS) $(COMMON_OBJECTS) -I include -DPATH_SEPARATOR="$(PATH_SEPARATOR)"

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
	git rm --ignore-unmatch -rf $(PRUNE_LIST)
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

