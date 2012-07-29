#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: makefile
#   @desc: 
#
# @author: pantoine
#   @date: 04/07/2012 22:50:11
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

PLUGINS ?= $(subst /,,$(dir $(wildcard */makefile)))
DOCS_PLUGINS  = $(addprefix docs_,$(PLUGINS))
BUILD_PLUGINS = $(addprefix build_,$(PLUGINS))
CLEAN_PLUGINS = $(addprefix clean_,$(PLUGINS))


#---------------------------------------------------------------------------------
# build targets
#---------------------------------------------------------------------------------
.PHONY: $(BUILD_PLUGINS)

build: $(BUILD_PLUGINS)

clean: $(CLEAN_PLUGINS)

docs:  $(DOCS_PLUGINS)

$(DOCS_PLUGINS):
	@$(MAKE) -s -C $(subst docs_,,$@) docs

$(BUILD_PLUGINS):
	@$(MAKE) -s -C $(subst build_,,$@) build

$(CLEAN_PLUGINS):
	@$(MAKE) -s -C $(subst clean_,,$@) clean

