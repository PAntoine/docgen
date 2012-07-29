#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: makefile
#   @desc: This is the root makefile for the document generator suite of
#          applications.
#
# @author: pantoine
#   @date: 05/07/2012 08:46:15
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

include build/platform_config.mak

#--------------------------------------------------------------------------------
# Default build configuration
#--------------------------------------------------------------------------------
export BUILD_ROOT		= $(CURDIR)
export ALL_COMPONENTS	= $(subst /,,$(dir $(wildcard */makefile)))

# user can override this on the command line	
export BUILD_TYPE ?= $(BUILD_OS)_$(MACHINE_NAME)
export MAKECMDGOALS ?= build

# list of files/directories to be released
SOURCE_RELEASE_DIRS = pdp pdsc pdsl $(filter-out plugins/makefile,$(wildcard plugins/*))
EXTRA_RELEASE_FILES = readme.md makefile gdbbatch docs/specification.txt docs/TODO.txt plugins/makefile

BINARY_RELEASE_LIST	= docs/html docs/manpage docs/text docs/specification.txt docs/TODO.txt readme.md

#--------------------------------------------------------------------------------
# Components that are required to be built.
#--------------------------------------------------------------------------------
export COMPONENTS ?= pdp pdsl pdsc plugins
export DOCS_COMPONENTS = $(addprefix docs_,$(COMPONENTS))
export BUILD_COMPONENTS = $(addprefix build_,$(COMPONENTS))

# lets load the defaults
include build/$(BUILD_TYPE).mak

#--------------------------------------------------------------------------------
# build targets
#--------------------------------------------------------------------------------
.PHONY:	build $(ALL_COMPONENTS) $(DOCS_COMPONENTS) $(BUILD_COMPONENTS)

build: output $(BUILD_COMPONENTS)

docs: build $(DOCS_COMPONENTS)

clean: $(ALL_COMPONENTS)
	@$(MAKE) -s -C tests clean
	@-$(RMSUBDIR) object
	@-$(RMSUBDIR) output

tests: build

fine:
$(DOCS_COMPONENTS):
	@$(MAKE) -s -C $(subst docs_,,$@) docs

$(BUILD_COMPONENTS):
	@$(MAKE) -s -C $(subst build_,,$@) build

$(ALL_COMPONENTS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

output:
	@$(MKDIR) output

#--------------------------------------------------------------------------------
# special targets
#--------------------------------------------------------------------------------
include build/release.mak

