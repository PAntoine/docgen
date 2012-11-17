#---------------------------------------------------------------------------------
#               Document Generator Project
#			        by Peter Antoine
#
#   @file: dependency
#   @desc: 
#
# @author: pantoine
#   @date: 04/07/2012 21:54:13
#---------------------------------------------------------------------------------
#                     Copyright (c) 2012 Peter Antoine
#                            All rights Reserved.
#                    Released Under the Artistic Licence
#---------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),dependancies)
-include make_$(TARGET_NAME).deps
endif

#---------------------------------------------------------------------------------
# build the dependencies for the source files, expect the source files to have
# the correct layout.
#---------------------------------------------------------------------------------
C_DEPS_FILES	= $(subst .c,.deps,$(addprefix $(CURDIR)$(GOOD_SLASH),$(C_SOURCE_FILES)))
CPP_DEPS_FILES	= $(subst .cpp,.deps,$(addprefix $(CURDIR)$(GOOD_SLASH),$(CPP_SOURCE_FILES)))

#---------------------------------------------------------------------------------
# actually make the dependencies.
#---------------------------------------------------------------------------------
make_$(TARGET_NAME).deps : $(addprefix $(CURDIR)$(GOOD_SLASH),$(C_SOURCE_FILES) $(CPP_SOURCE_FILES))
	@echo .silent: > make_$(TARGET_NAME).deps
	@$(MAKE) dependancies

clean_deps:
	@$(RM) make_$(TARGET_NAME).deps

dependancies: $(C_DEPS_FILES) $(CPP_DEPS_FILES)

$(C_DEPS_FILES):
	@$(CC) $(CFLAGS) $(OTHER_BUILD_FLAGS) -MM $(subst .deps,.c,$@)		\
	   -MT $(subst .deps,.o,$(subst source,object,$@)) 					\
	   -I $(CURDIR)/include $(INCLUDE_DIRS) >> make_$(TARGET_NAME).deps
	@$(CC) $(CFLAGS) $(OTHER_BUILD_FLAGS) -MM $(subst .deps,.c,$@)		\
	   -MT docs_srcs													\
	   -I $(CURDIR)/include $(INCLUDE_DIRS) >> make_$(TARGET_NAME).deps

$(CPP_DEPS_FILES):
	@$(CXX) $(CFLAGS) $(OTHER_BUILD_FLAGS) -MM $(subst .deps,.cpp,$@) 	\
		-MT $(subst .deps,.o,$(subst source,object,$@)) 				\
		-I $(CURDIR)/include $(INCLUDE_DIRS) >> make_$(TARGET_NAME).deps
	@$(CC) $(CFLAGS) $(OTHER_BUILD_FLAGS) -MM $(subst .deps,.c,$@)		\
	   -MT docs_srcs													\
	   -I $(CURDIR)/include $(INCLUDE_DIRS) >> make_$(TARGET_NAME).deps


