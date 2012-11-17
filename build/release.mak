#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: release
#   @desc: This file handles releasing the code. It will handle two flavours of
#          release, the source and binary. The source release will also release
#          the generated docs as will as the source. The binary will release the
#          same as well as the contents of the output dir.
#
# @author: pantoine
#   @date: 29/07/2012 08:33:38
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

RELEASE_STRUCTURE = source include makefile

ifeq ("$(MAKECMDGOALS)","source_release")
ifeq ("$(VERSION_NAME)","")
$(error You must provide a a VERSION_NAME to do a source_release)
endif
RELEASE_LIST = $(foreach dir,$(SOURCE_RELEASE_DIRS),$(addprefix $(dir)$(GOOD_SLASH),$(RELEASE_STRUCTURE))) $(EXTRA_RELEASE_FILES)
test_for_branch = true
CFLAGS			+= $(COPTS_RELEASE)
endif

ifeq ("$(MAKECMDGOALS)","binary_release")
RELEASE_LIST = $(BINARY_RELEASE_LIST)
test_for_branch = true
CFLAGS			+= $(COPTS_RELEASE)
endif

# The files that are wanted for release
RELEASE_LIST ?= $(foreach dir,$(SOURCE_RELEASE_DIRS),$(addprefix $(dir)$(GOOD_SLASH),$(RELEASE_STRUCTURE))) $(EXTRA_RELEASE_FILES)

ifeq ("$(test_for_branch)","true")
ifeq ("$(CURRENT_BRANCH)","")
$(info Looks like there is a problem with your repository, the build cannot)
$(info find a branch. The build cannot do any kind of release without a branch)
$(info otherwise it can seriously mess up you repository.)
$(info If there is no problem with you repository, check that you platform has)
$(info a function defines to get the CURRENT_BRANCH - see platform_config.mak )
$(info or the version of git you are using has not changed it\'s output.)
$(error cannot do a release without a branch name.)
endif
endif

#--------------------------------------------------------------------------------
# Other things used in the release
#--------------------------------------------------------------------------------
MESSAGE_STRING	= Released on $(NICE_BUILD_TIME) from branch $(CURRENT_BRANCH)
RELEASE_ROOT	= $(strip $(lastword $(subst $(GOOD_SLASH), ,$(BUILD_ROOT))))
RELEASE_NAME	= $(RELEASE_ROOT)_$(TARGET_PLATFORM)_$(BUILD_DATE)
RELEASE_FILES	= $(addprefix $(RELEASE_ROOT)$(GOOD_SLASH),$(RELEASE_LIST))

#--------------------------------------------------------------------------------
# Build Targets
#--------------------------------------------------------------------------------
check_repository:
	@echo Checking that all files are checked in...
	@git diff-index --quiet HEAD
	@echo Checking that there are no untracked files in the repo...
	@git ls-files --exclude-standard --others
	@$(call TEST_EMPTY_STRING,git ls-files --exclude-standard --others)

dry_run_release: clean build
	@echo and the following are going to be released:
	@echo $(RELEASE_LIST)

source_release: check_repository clean build 
	@git checkout $(CURRENT_BRANCH)^0
	@git reset --soft release
	@git add $(RELEASE_LIST) .gitattributes .gitignore .banner
	@git add -f $(IGNORED_RELEASE_LIST)
	@git commit
	@git branch new_temp
	@git checkout new_temp
	@git branch -M release
	@git tag $(VERSION_NAME) -m $(MESSAGE_STRING)
	@git checkout -f $(CURRENT_BRANCH)
	@git tag SOURCE_$(VERSION_NAME)

binary_release: check_repository clean build
	@tar -czf $(RELEASE_NAME).tar.gz -C .. $(RELEASE_FILES)
