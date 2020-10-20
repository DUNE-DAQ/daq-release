SHELL = /bin/sh
# 
#  1) This file is designed to be invoked by a shell script called premake
#  which selects supported options.
#
#=============================================================================
# TARGETS
#=============================================================================

default:	
	@echo Please invoke this makefile using premake. >&2
	@exit 1

all :	help_file


help_file : 
	@rm -f  ups_help.dat
	@./build_help_file.pl > ups_help.dat

#
# Now standard targets
#
clean :
	@ ../bin/upsClean

spotless : clean
	@ rm -f ./ups_help.dat
