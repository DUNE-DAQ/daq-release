SHELL = /bin/sh
# 
#  1) This file is designed to be invoked by a shell script called premake
#  which selects supported options.
#
#=============================================================================
# TARGETS
#=============================================================================

default:	
	echo Please invoke this makefile using upspremake. >&2
	exit 1

all :
	@touch catman/dummy.1
	@rm -f catman/*.1
	@../bin/ups -? 2> catman/ups.1 || true
	@../bin/ups configure -? 2> catman/ups_configure.1 || true
	@../bin/ups copy -? 2> catman/ups_copy.1 || true
	@../bin/ups create -? 2> catman/ups_create.1 || true
	@../bin/ups declare -? 2> catman/ups_declare.1 || true
	@../bin/ups depend -? 2> catman/ups_depend.1 || true
	@../bin/ups exist -? 2> catman/ups_exist.1 || true
	@../bin/ups flavor -? 2> catman/ups_flavor.1 || true
	@../bin/ups get -? 2> catman/ups_get.1 || true
	@../bin/ups list -? 2> catman/ups_list.1 || true
	@../bin/ups modify -? 2> catman/ups_modify.1 || true
	@../bin/ups setup -? 2> catman/ups_setup.1 || true
	@../bin/ups start -? 2> catman/ups_start.1 || true
	@../bin/ups stop -? 2> catman/ups_stop.1 || true
	@../bin/ups tailor -? 2> catman/ups_tailor.1 || true
	@../bin/ups touch -? 2> catman/ups_touch.1 || true
	@../bin/ups unconfigure -? 2> catman/ups_unconfigure.1 || true
	@../bin/ups undeclare -? 2> catman/ups_undeclare.1 || true
	@../bin/ups unsetup -? 2> catman/ups_unsetup.1 || true
	@../bin/ups verify -? 2> catman/ups_verify.1 || true

#
# Now standard targets
#
clean :
	../bin/upsClean

spotless : clean
	/bin/rm -f catman/*.1
	/bin/rm -f up

