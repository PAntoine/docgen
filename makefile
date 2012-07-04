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

export CC ?= gcc
export MKDIR ?= mkdir
export RM_F ?= rm -rf
export CFLAGS = -ansi -pedantic -funsigned-char

ifdef DEBUG
CFLAGS += -g
export DEBUG_FUNC = gdb -return-child-result -x $(CURDIR)/gdbbatch --args
endif

ifdef VALGRIND
export DEBUG_FUNC = valgrind --track-origins=yes --leak-check=full
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
RELEASE_LIST = source include tests/source tests/include tests/makefile readme.md makefile gdbbatch \
			   docs/specification.txt docs/TODO.txt
	
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
LINKER_OBJECTS 		= object/document_linker.o
COMPILER_OBJECTS	= object/symbols.o object/input_formats.o object/c_cpp_input_functions.o object/document_source_compiler.o
PROCESSOR_OBJECTS	= object/supported_formats.o object/text_output_functions.o object/document_processor.o

#--------------------------------------------------------------------------------
# Document targets.
#--------------------------------------------------------------------------------
HEADER_DOCS			= $(subst include/,object/,$(subst .h,.h.pdso,$(HEADER_FILES)))
LINKER_DOCS			= $(subst .o,.c.pdso,$(LINKER_OBJECTS) $(COMMON_OBJECTS))
COMPILER_DOCS		= $(subst .o,.c.pdso,$(COMPILER_OBJECTS) $(COMMON_OBJECTS))
PROCESSOR_DOCS		= $(subst .o,.c.pdso,$(PROCESSOR_OBJECTS) $(COMMON_OBJECTS))
PDP_DOCS			= $(wildcard docs/pdp/source/*.md)

#--------------------------------------------------------------------------------
# Executable Targets.
#--------------------------------------------------------------------------------
.PHONY: tests $(PDP_DOCS)

all: exes tests

exes: pdsc pdsl pdp pdsc_docs pdsl_docs pdp_docs

pdsc : object include/atoms.h $(COMMON_OBJECTS) $(COMPILER_OBJECTS)
	@$(CC) $(CFLAGS) -o pdsc $(COMPILER_OBJECTS) $(COMMON_OBJECTS)

pdsl : object $(COMMON_OBJECTS) $(LINKER_OBJECTS)
	@$(CC) $(CFLAGS) -o pdsl $(LINKER_OBJECTS) $(COMMON_OBJECTS)

pdp : object $(COMMON_OBJECTS) $(PROCESSOR_OBJECTS)
	@$(CC) $(CFLAGS) -o pdp $(PROCESSOR_OBJECTS) $(COMMON_OBJECTS)

tests: pdsc pdsl pdp
	@$(MAKE) -C tests

include/symbols.h source/symbols.c: source/symbols.list
	@buildgraph -q source/symbols.list source/symbols -h include/ -p "SYMBOLS_"

include/atoms.h source/atoms.c: source/atoms.list
	@buildgraph -q source/atoms.list source/atoms -h include/ -p "ATOM_"

object/%.o : source/%.c
	@$(CC) $(CFLAGS) -c -o $@ $< -I include -DPATH_SEPARATOR="$(PATH_SEPARATOR)"

#--------------------------------------------------------------------------------
# Document Targets.
#--------------------------------------------------------------------------------
pdsc_docs:	object/pdsc.gout
pdsl_docs:	object/pdsl.gout
pdp_docs:	$(PDP_DOCS)

$(PDP_DOCS): object/pdp.gout
	@$(DEBUG_FUNC) ./pdp -iobject/pdp.gout $@

object/pdsc.gout: $(COMPILER_DOCS) $(HEADER_DOCS)
	@echo Linking Compiler Documents...
	@$(DEBUG_FUNC) ./pdsl $(COMPILER_DOCS) $(HEADER_DOCS) -o $@

object/pdsl.gout: $(LINKER_DOCS) $(HEADER_DOCS)
	@echo Linking Linker Documents...
	@$(DEBUG_FUNC) ./pdsl $(LINKER_DOCS) $(HEADER_DOCS) -o $@

object/pdp.gout: $(PROCESSOR_DOCS) $(HEADER_DOCS)
	@echo Linking processor Documents...
	@$(DEBUG_FUNC) ./pdsl $(PROCESSOR_DOCS) $(HEADER_DOCS) -o $@

object/%.h.pdso : include/%.h pdsc
	@$(DEBUG_FUNC) ./pdsc -o $@ $< $(DOC_COMPILE_FLAGS)

object/%.c.pdso : source/%.c pdsc
	@$(DEBUG_FUNC) ./pdsc -o $@ $< $(DOC_COMPILE_FLAGS)

object:
	@$(MKDIR) object 

release:
	git checkout master^0
	git reset --soft release
	git rm --ignore-unmatch -rf $(PRUNE_LIST)
	git commit
	git branch new_temp
	git checkout new_temp
	git branch -M release
	git tag $(VERSION_NAME)
	git checkout -f master

clean:
	@$(RM) source/atoms.c
	@$(RM) include/atoms.h
	@$(RM) object/*
	@$(MAKE) -C tests clean
	@$(RM) pdsc
	@$(RM) pdsl

