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

#--------------------------------------------------------------------------------
# Default build configuration
#--------------------------------------------------------------------------------
export BUILD_ROOT		= $(CURDIR)
export ALL_COMPONENTS	= $(subst /,,$(dir $(wildcard */makefile)))

include build/platform_config.mak

# user can override this on the command line	
export BUILD_TYPE ?= $(BUILD_OS)_$(MACHINE_NAME)
export MAKECMDGOALS ?= build

# list of files/directories to be released
# SOURCE_RELEASE_DIRS is a list of "structured" directories, where the others are hodge-podge
# so the order is not predictable. Also the ignored release list are files that are not
# normally tracked by are required for release.
SOURCE_RELEASE_DIRS = common pdp pdsc pdsl tests $(filter-out plugins/makefile,$(wildcard plugins/*))
EXTRA_RELEASE_FILES = build readme.md makefile gdbbatch docs/specification.md docs/TODO.md docs/release_note.md plugins/makefile support_files
IGNORED_RELEASE_LIST = docs/html docs/manpage docs/text

BINARY_RELEASE_LIST	= docs/html docs/manpage docs/text docs/specification.md docs/TODO.md readme.md output tests/include tests/source tests/makefile

#--------------------------------------------------------------------------------
# Components that are required to be built.
#--------------------------------------------------------------------------------
COMPONENTS ?= pdp pdsl pdsc plugins
DOCS_COMPONENTS = $(addprefix docs_,$(COMPONENTS))
BUILD_COMPONENTS = $(addprefix build_,$(COMPONENTS))
CLEAN_COMPONENTS = $(addprefix clean_,$(ALL_COMPONENTS))

# lets load the defaults
include build/$(BUILD_TYPE).mak

#--------------------------------------------------------------------------------
# build targets
#--------------------------------------------------------------------------------
.PHONY:	build $(ALL_COMPONENTS) $(DOCS_COMPONENTS) $(BUILD_COMPONENTS) support_files

build: output $(BUILD_COMPONENTS)

docs: build tests $(DOCS_COMPONENTS)

clean: $(CLEAN_COMPONENTS)
	@-$(RMSUBDIR) object
	@-$(RMSUBDIR) output

$(DOCS_COMPONENTS):
	@$(MAKE) -s -C $(subst docs_,,$@) docs

$(BUILD_COMPONENTS): support_files
	@$(MAKE) -s -C $(subst build_,,$@) build

$(CLEAN_COMPONENTS):
	@$(MAKE) -s -C $(subst clean_,,$@) clean

tests: build 

$(ALL_COMPONENTS):
	@$(MAKE) -s -C $@ $(MAKECMDGOALS)

output:
	@$(MKDIR) output

#--------------------------------------------------------------------------------
# special targets
#--------------------------------------------------------------------------------
include build/release.mak

