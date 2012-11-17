#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: makefile
#   @desc: This make file will build the common library.
#          It also will build the generated source files and compile them
#          into the library.
#
# @author: pantoine
#   @date: 05/07/2012 09:30:41
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

BG_SOURCE = $(wildcard source/*.list)
BG_TARGET = $(subst .list,.c,$(BG_SOURCE))

#---------------------------------------------------------------------------------
# The build targets
#---------------------------------------------------------------------------------
# override 'docs' so that all the common include files are processed
docs: $(wildcard $(CURDIR)/include/*.h)

# override 'build' to generate the special files
build: $(BG_TARGET)

include/symbols.h source/symbols.c: source/symbols.list
	@buildgraph -q source/symbols.list source/symbols -h include/ -p "SYMBOLS_"

include/atoms.h source/atoms.c: source/atoms.list
	@buildgraph -q source/atoms.list source/atoms -h include/ -p "ATOM_"

#---------------------------------------------------------------------------------
# The build includes
#---------------------------------------------------------------------------------
include $(BUILD_ROOT)/build/make_library.mak
include $(BUILD_ROOT)/build/make_docs.mak
