SHELL = /bin/sh
# 
#  1) This file is designed to be invoked by a shell script called premake
#  which selects supported options.
#
#  2) This file re-builds the ups tests.
#
#  3) Environment:
# Environmental variables -
#
# Expected variables on the command line:
# CC=        The name of the appropriate compiler, with suitable flags
# CCCHK=     A set of compiler flags which check ANSI C strictly
#

.c.o :
	$(CC) -c $(CCCHK) $(CFLAGS) $*.c

#
SRC	= ../src
INC	= ../inc
LIB	= ../lib
BIN	= ../bin
DOC	= ../doc

CFLAGS	  = $(UPS_CFLAGS) $(INCDIRS)

#
#
OBJECTS = 

LDFLAGS = $(LLIBS)

UNIT_TESTS = test_upserr test_upslst test_upsmem test_upsutl test_upsugo \
	test_upsmat test_upsact test_upsfil test_ups_list

UPS_TEST_OBJECTS = upstst_cmdline.o upstst_cmdloop.o upstst_cmdproc.o \
	upstst_misc.o upstst_parse.o upstst_err.o upstst_fil.o	\
	upstst_ugo.o upstst_mat.o upstst_get.o upstst_act.o upstst_command.o
#=============================================================================
# TARGETS
#=============================================================================

default:	
	@echo Please invoke this makefile using premake. >&2
	@exit 1

all : unit_tests integration_tests

unit_tests : $(UNIT_TESTS)

integration_tests : ups_test

#
# Main programs.
#

test_ups_list : test_ups_list.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

test_upsact : test_upsact.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

test_upserr : test_upserr.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

test_upsfil : test_upsfil.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

test_upslst : test_upslst.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

test_upsmem : test_upsmem.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

test_upsutl : test_upsutl.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

test_upsmat : test_upsmat.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

test_upsugo: test_upsugo.o $(LIB)/libups.a
	$(LD) $@.o -o $@ $(LIB)/libups.a $(LDFLAGS)

ups_test : ups_test.o $(UPS_TEST_OBJECTS)
	$(LD) $@.o -o $@ $(LIB)/libups.a $(UPS_TEST_OBJECTS) $(LDFLAGS)

#
# Now standard targets
#
clean :
	@ ../bin/upsClean
	@ rm -f scripts/*/log/*.out scripts/*/log/*.out.*sh
	@ rm -rf scripts/copy/db1/bee3/ups
	@ rm -rf scripts/declare/dbclean scripts/declare/tostuff
	@ rm -rf scripts/declare/products/spot/ups/toinfo
	@ rm -rf scripts/undeclare/dbclean

spotless : clean
	@ rm -f $(UNIT_TESTS)
	@ rm -f ups_test libups_test.a

make :
	@if [ "$(CCENV)" = "" ]; then ccenv=`uname`; else ccenv=$(CCENV); fi; \
	echo \
	"../bin/upsmake_make -cc '$(CC) $(CFLAGS)' -nostd -file Makefile" \
                                                        "-env $$ccenv *.c"; \
	../bin/upsmake_make -cc '$(CC) $(CFLAGS)' -nostd -file Makefile \
                                                         -env $$ccenv *.c
#START_DEPEND
test_ups_list.o: test_ups_list.c
test_ups_list.o: $(UPS_DIR)/inc/ups.h
test_ups_list.o: $(UPS_DIR)/inc/upsact.h
test_ups_list.o: $(UPS_DIR)/inc/upstyp.h
test_ups_list.o: $(UPS_DIR)/inc/upslst.h
test_ups_list.o: $(UPS_DIR)/inc/upsugo.h
test_ups_list.o: $(UPS_DIR)/inc/upsmat.h
test_ups_list.o: $(UPS_DIR)/inc/upscpy.h
test_ups_list.o: $(UPS_DIR)/inc/upserr.h
test_ups_list.o: $(UPS_DIR)/inc/upsfil.h
test_ups_list.o: $(UPS_DIR)/inc/upstbl.h
test_ups_list.o: $(UPS_DIR)/inc/upsget.h
test_ups_list.o: $(UPS_DIR)/inc/upshlp.h
test_ups_list.o: $(UPS_DIR)/inc/upskey.h
test_ups_list.o: $(UPS_DIR)/inc/upsmem.h
test_ups_list.o: $(UPS_DIR)/inc/upsutl.h
test_ups_list.o: $(UPS_DIR)/inc/upsver.h
test_ups_list.o: $(UPS_DIR)/inc/ups_list.h
test_ups_list.o: $(UPS_DIR)/inc/ups_configure.h
test_ups_list.o: $(UPS_DIR)/inc/ups_copy.h
test_ups_list.o: $(UPS_DIR)/inc/ups_declare.h
test_ups_list.o: $(UPS_DIR)/inc/ups_depend.h
test_ups_list.o: $(UPS_DIR)/inc/ups_flavor.h
test_ups_list.o: $(UPS_DIR)/inc/ups_get.h
test_ups_list.o: $(UPS_DIR)/inc/ups_main.h
test_ups_list.o: $(UPS_DIR)/inc/ups_modify.h
test_ups_list.o: $(UPS_DIR)/inc/ups_setup.h
test_ups_list.o: $(UPS_DIR)/inc/ups_start.h
test_ups_list.o: $(UPS_DIR)/inc/ups_stop.h
test_ups_list.o: $(UPS_DIR)/inc/ups_tailor.h
test_ups_list.o: $(UPS_DIR)/inc/ups_touch.h
test_ups_list.o: $(UPS_DIR)/inc/ups_unconfigure.h
test_ups_list.o: $(UPS_DIR)/inc/ups_unk.h
test_ups_list.o: $(UPS_DIR)/inc/ups_unsetup.h
test_ups_list.o: $(UPS_DIR)/inc/ups_undeclare.h
test_ups_list.o: $(UPS_DIR)/inc/ups_verify.h
test_upsact.o: test_upsact.c
test_upsact.o: $(UPS_DIR)/inc/upsfil.h
test_upsact.o: $(UPS_DIR)/inc/upstyp.h
test_upsact.o: $(UPS_DIR)/inc/upslst.h
test_upsact.o: $(UPS_DIR)/inc/upstbl.h
test_upsact.o: $(UPS_DIR)/inc/upserr.h
test_upsact.o: $(UPS_DIR)/inc/upsact.h
test_upsact.o: $(UPS_DIR)/inc/upsugo.h
test_upsact.o: $(UPS_DIR)/inc/upsmat.h
test_upsact.o: $(UPS_DIR)/inc/ups_list.h
test_upserr.o: test_upserr.c
test_upserr.o: $(UPS_DIR)/inc/upserr.h
test_upsfil.o: test_upsfil.c
test_upsfil.o: $(UPS_DIR)/inc/upserr.h
test_upsfil.o: $(UPS_DIR)/inc/upsfil.h
test_upsfil.o: $(UPS_DIR)/inc/upstyp.h
test_upsfil.o: $(UPS_DIR)/inc/upslst.h
test_upsfil.o: $(UPS_DIR)/inc/upstbl.h
test_upshlp.o: test_upshlp.c
test_upshlp.o: $(UPS_DIR)/inc/upstyp.h
test_upshlp.o: $(UPS_DIR)/inc/upslst.h
test_upslst.o: test_upslst.c
test_upslst.o: $(UPS_DIR)/inc/upslst.h
test_upslst.o: $(UPS_DIR)/inc/upsmem.h
test_upsmat.o: test_upsmat.c
test_upsmat.o: $(UPS_DIR)/inc/upsmat.h
test_upsmat.o: $(UPS_DIR)/inc/upsugo.h
test_upsmat.o: $(UPS_DIR)/inc/upslst.h
test_upsmat.o: $(UPS_DIR)/inc/upstyp.h
test_upsmat.o: $(UPS_DIR)/inc/upserr.h
test_upsmat.o: $(UPS_DIR)/inc/upsfil.h
test_upsmat.o: $(UPS_DIR)/inc/upstbl.h
test_upsmat.o: $(UPS_DIR)/inc/upsmem.h
test_upsmem.o: test_upsmem.c
test_upsmem.o: $(UPS_DIR)/inc/upsmem.h
test_upsmem.o: $(UPS_DIR)/inc/upsutl.h
test_upsmem.o: $(UPS_DIR)/inc/upstyp.h
test_upsmem.o: $(UPS_DIR)/inc/upslst.h
test_upsmem.o: $(UPS_DIR)/inc/upsugo.h
test_upsmem.o: $(UPS_DIR)/inc/upserr.h
test_upsugo.o: test_upsugo.c
test_upsugo.o: $(UPS_DIR)/inc/ups.h
test_upsugo.o: $(UPS_DIR)/inc/upsact.h
test_upsugo.o: $(UPS_DIR)/inc/upstyp.h
test_upsugo.o: $(UPS_DIR)/inc/upslst.h
test_upsugo.o: $(UPS_DIR)/inc/upsugo.h
test_upsugo.o: $(UPS_DIR)/inc/upsmat.h
test_upsugo.o: $(UPS_DIR)/inc/upscpy.h
test_upsugo.o: $(UPS_DIR)/inc/upserr.h
test_upsugo.o: $(UPS_DIR)/inc/upsfil.h
test_upsugo.o: $(UPS_DIR)/inc/upstbl.h
test_upsugo.o: $(UPS_DIR)/inc/upsget.h
test_upsugo.o: $(UPS_DIR)/inc/upshlp.h
test_upsugo.o: $(UPS_DIR)/inc/upskey.h
test_upsugo.o: $(UPS_DIR)/inc/upsmem.h
test_upsugo.o: $(UPS_DIR)/inc/upsutl.h
test_upsugo.o: $(UPS_DIR)/inc/upsver.h
test_upsugo.o: $(UPS_DIR)/inc/ups_list.h
test_upsugo.o: $(UPS_DIR)/inc/ups_configure.h
test_upsugo.o: $(UPS_DIR)/inc/ups_copy.h
test_upsugo.o: $(UPS_DIR)/inc/ups_declare.h
test_upsugo.o: $(UPS_DIR)/inc/ups_depend.h
test_upsugo.o: $(UPS_DIR)/inc/ups_flavor.h
test_upsugo.o: $(UPS_DIR)/inc/ups_get.h
test_upsugo.o: $(UPS_DIR)/inc/ups_main.h
test_upsugo.o: $(UPS_DIR)/inc/ups_modify.h
test_upsugo.o: $(UPS_DIR)/inc/ups_setup.h
test_upsugo.o: $(UPS_DIR)/inc/ups_start.h
test_upsugo.o: $(UPS_DIR)/inc/ups_stop.h
test_upsugo.o: $(UPS_DIR)/inc/ups_tailor.h
test_upsugo.o: $(UPS_DIR)/inc/ups_touch.h
test_upsugo.o: $(UPS_DIR)/inc/ups_unconfigure.h
test_upsugo.o: $(UPS_DIR)/inc/ups_unk.h
test_upsugo.o: $(UPS_DIR)/inc/ups_unsetup.h
test_upsugo.o: $(UPS_DIR)/inc/ups_undeclare.h
test_upsugo.o: $(UPS_DIR)/inc/ups_verify.h
test_upsutl.o: test_upsutl.c
test_upsutl.o: $(UPS_DIR)/inc/upsugo.h
test_upsutl.o: $(UPS_DIR)/inc/upslst.h
test_upsutl.o: $(UPS_DIR)/inc/upstyp.h
test_upsutl.o: $(UPS_DIR)/inc/upsget.h
test_upsutl.o: $(UPS_DIR)/inc/upserr.h
test_upsutl.o: $(UPS_DIR)/inc/upsmat.h
ups_test.o: ups_test.c
ups_test.o: upstst_cmdtable.h
ups_test.o: $(UPS_DIR)/inc/ups.h
ups_test.o: $(UPS_DIR)/inc/upsact.h
ups_test.o: $(UPS_DIR)/inc/upstyp.h
ups_test.o: $(UPS_DIR)/inc/upslst.h
ups_test.o: $(UPS_DIR)/inc/upsugo.h
ups_test.o: $(UPS_DIR)/inc/upsmat.h
ups_test.o: $(UPS_DIR)/inc/upscpy.h
ups_test.o: $(UPS_DIR)/inc/upserr.h
ups_test.o: $(UPS_DIR)/inc/upsfil.h
ups_test.o: $(UPS_DIR)/inc/upstbl.h
ups_test.o: $(UPS_DIR)/inc/upsget.h
ups_test.o: $(UPS_DIR)/inc/upshlp.h
ups_test.o: $(UPS_DIR)/inc/upskey.h
ups_test.o: $(UPS_DIR)/inc/upsmem.h
ups_test.o: $(UPS_DIR)/inc/upsutl.h
ups_test.o: $(UPS_DIR)/inc/upsver.h
ups_test.o: $(UPS_DIR)/inc/ups_list.h
ups_test.o: $(UPS_DIR)/inc/ups_configure.h
ups_test.o: $(UPS_DIR)/inc/ups_copy.h
ups_test.o: $(UPS_DIR)/inc/ups_declare.h
ups_test.o: $(UPS_DIR)/inc/ups_depend.h
ups_test.o: $(UPS_DIR)/inc/ups_flavor.h
ups_test.o: $(UPS_DIR)/inc/ups_get.h
ups_test.o: $(UPS_DIR)/inc/ups_main.h
ups_test.o: $(UPS_DIR)/inc/ups_modify.h
ups_test.o: $(UPS_DIR)/inc/ups_setup.h
ups_test.o: $(UPS_DIR)/inc/ups_start.h
ups_test.o: $(UPS_DIR)/inc/ups_stop.h
ups_test.o: $(UPS_DIR)/inc/ups_tailor.h
ups_test.o: $(UPS_DIR)/inc/ups_touch.h
ups_test.o: $(UPS_DIR)/inc/ups_unconfigure.h
ups_test.o: $(UPS_DIR)/inc/ups_unk.h
ups_test.o: $(UPS_DIR)/inc/ups_unsetup.h
ups_test.o: $(UPS_DIR)/inc/ups_undeclare.h
ups_test.o: $(UPS_DIR)/inc/ups_verify.h
upstst_act.o: upstst_act.c
upstst_act.o: $(UPS_DIR)/inc/ups.h
upstst_act.o: $(UPS_DIR)/inc/upsact.h
upstst_act.o: $(UPS_DIR)/inc/upstyp.h
upstst_act.o: $(UPS_DIR)/inc/upslst.h
upstst_act.o: $(UPS_DIR)/inc/upsugo.h
upstst_act.o: $(UPS_DIR)/inc/upsmat.h
upstst_act.o: $(UPS_DIR)/inc/upscpy.h
upstst_act.o: $(UPS_DIR)/inc/upserr.h
upstst_act.o: $(UPS_DIR)/inc/upsfil.h
upstst_act.o: $(UPS_DIR)/inc/upstbl.h
upstst_act.o: $(UPS_DIR)/inc/upsget.h
upstst_act.o: $(UPS_DIR)/inc/upshlp.h
upstst_act.o: $(UPS_DIR)/inc/upskey.h
upstst_act.o: $(UPS_DIR)/inc/upsmem.h
upstst_act.o: $(UPS_DIR)/inc/upsutl.h
upstst_act.o: $(UPS_DIR)/inc/upsver.h
upstst_act.o: $(UPS_DIR)/inc/ups_list.h
upstst_act.o: $(UPS_DIR)/inc/ups_configure.h
upstst_act.o: $(UPS_DIR)/inc/ups_copy.h
upstst_act.o: $(UPS_DIR)/inc/ups_declare.h
upstst_act.o: $(UPS_DIR)/inc/ups_depend.h
upstst_act.o: $(UPS_DIR)/inc/ups_flavor.h
upstst_act.o: $(UPS_DIR)/inc/ups_get.h
upstst_act.o: $(UPS_DIR)/inc/ups_main.h
upstst_act.o: $(UPS_DIR)/inc/ups_modify.h
upstst_act.o: $(UPS_DIR)/inc/ups_setup.h
upstst_act.o: $(UPS_DIR)/inc/ups_start.h
upstst_act.o: $(UPS_DIR)/inc/ups_stop.h
upstst_act.o: $(UPS_DIR)/inc/ups_tailor.h
upstst_act.o: $(UPS_DIR)/inc/ups_touch.h
upstst_act.o: $(UPS_DIR)/inc/ups_unconfigure.h
upstst_act.o: $(UPS_DIR)/inc/ups_unk.h
upstst_act.o: $(UPS_DIR)/inc/ups_unsetup.h
upstst_act.o: $(UPS_DIR)/inc/ups_undeclare.h
upstst_act.o: $(UPS_DIR)/inc/ups_verify.h
upstst_act.o: upstst_parse.h
upstst_act.o: upstst_macros.h
upstst_cmdline.o: upstst_cmdline.c
upstst_cmdline.o: upstst_cmdline.h
upstst_cmdloop.o: upstst_cmdloop.c
upstst_cmdloop.o: upstst_cmdline.h
upstst_cmdloop.o: upstst_parse.h
upstst_cmdloop.o: upstst_cmdtable.h
upstst_cmdproc.o: upstst_cmdproc.c
upstst_cmdproc.o: upstst_cmdline.h
upstst_cmdproc.o: upstst_parse.h
upstst_cmdproc.o: upstst_cmdtable.h
upstst_command.o: upstst_command.c
upstst_command.o: $(UPS_DIR)/inc/ups.h
upstst_command.o: $(UPS_DIR)/inc/upsact.h
upstst_command.o: $(UPS_DIR)/inc/upstyp.h
upstst_command.o: $(UPS_DIR)/inc/upslst.h
upstst_command.o: $(UPS_DIR)/inc/upsugo.h
upstst_command.o: $(UPS_DIR)/inc/upsmat.h
upstst_command.o: $(UPS_DIR)/inc/upscpy.h
upstst_command.o: $(UPS_DIR)/inc/upserr.h
upstst_command.o: $(UPS_DIR)/inc/upsfil.h
upstst_command.o: $(UPS_DIR)/inc/upstbl.h
upstst_command.o: $(UPS_DIR)/inc/upsget.h
upstst_command.o: $(UPS_DIR)/inc/upshlp.h
upstst_command.o: $(UPS_DIR)/inc/upskey.h
upstst_command.o: $(UPS_DIR)/inc/upsmem.h
upstst_command.o: $(UPS_DIR)/inc/upsutl.h
upstst_command.o: $(UPS_DIR)/inc/upsver.h
upstst_command.o: $(UPS_DIR)/inc/ups_list.h
upstst_command.o: $(UPS_DIR)/inc/ups_configure.h
upstst_command.o: $(UPS_DIR)/inc/ups_copy.h
upstst_command.o: $(UPS_DIR)/inc/ups_declare.h
upstst_command.o: $(UPS_DIR)/inc/ups_depend.h
upstst_command.o: $(UPS_DIR)/inc/ups_flavor.h
upstst_command.o: $(UPS_DIR)/inc/ups_get.h
upstst_command.o: $(UPS_DIR)/inc/ups_main.h
upstst_command.o: $(UPS_DIR)/inc/ups_modify.h
upstst_command.o: $(UPS_DIR)/inc/ups_setup.h
upstst_command.o: $(UPS_DIR)/inc/ups_start.h
upstst_command.o: $(UPS_DIR)/inc/ups_stop.h
upstst_command.o: $(UPS_DIR)/inc/ups_tailor.h
upstst_command.o: $(UPS_DIR)/inc/ups_touch.h
upstst_command.o: $(UPS_DIR)/inc/ups_unconfigure.h
upstst_command.o: $(UPS_DIR)/inc/ups_unk.h
upstst_command.o: $(UPS_DIR)/inc/ups_unsetup.h
upstst_command.o: $(UPS_DIR)/inc/ups_undeclare.h
upstst_command.o: $(UPS_DIR)/inc/ups_verify.h
upstst_command.o: upstst_parse.h
upstst_command.o: upstst_macros.h
upstst_err.o: upstst_err.c
upstst_err.o: upstst_parse.h
upstst_err.o: upstst_macros.h
upstst_err.o: $(UPS_DIR)/inc/upserr.h
upstst_fil.o: upstst_fil.c
upstst_fil.o: $(UPS_DIR)/inc/ups.h
upstst_fil.o: $(UPS_DIR)/inc/upsact.h
upstst_fil.o: $(UPS_DIR)/inc/upstyp.h
upstst_fil.o: $(UPS_DIR)/inc/upslst.h
upstst_fil.o: $(UPS_DIR)/inc/upsugo.h
upstst_fil.o: $(UPS_DIR)/inc/upsmat.h
upstst_fil.o: $(UPS_DIR)/inc/upscpy.h
upstst_fil.o: $(UPS_DIR)/inc/upserr.h
upstst_fil.o: $(UPS_DIR)/inc/upsfil.h
upstst_fil.o: $(UPS_DIR)/inc/upstbl.h
upstst_fil.o: $(UPS_DIR)/inc/upsget.h
upstst_fil.o: $(UPS_DIR)/inc/upshlp.h
upstst_fil.o: $(UPS_DIR)/inc/upskey.h
upstst_fil.o: $(UPS_DIR)/inc/upsmem.h
upstst_fil.o: $(UPS_DIR)/inc/upsutl.h
upstst_fil.o: $(UPS_DIR)/inc/upsver.h
upstst_fil.o: $(UPS_DIR)/inc/ups_list.h
upstst_fil.o: $(UPS_DIR)/inc/ups_configure.h
upstst_fil.o: $(UPS_DIR)/inc/ups_copy.h
upstst_fil.o: $(UPS_DIR)/inc/ups_declare.h
upstst_fil.o: $(UPS_DIR)/inc/ups_depend.h
upstst_fil.o: $(UPS_DIR)/inc/ups_flavor.h
upstst_fil.o: $(UPS_DIR)/inc/ups_get.h
upstst_fil.o: $(UPS_DIR)/inc/ups_main.h
upstst_fil.o: $(UPS_DIR)/inc/ups_modify.h
upstst_fil.o: $(UPS_DIR)/inc/ups_setup.h
upstst_fil.o: $(UPS_DIR)/inc/ups_start.h
upstst_fil.o: $(UPS_DIR)/inc/ups_stop.h
upstst_fil.o: $(UPS_DIR)/inc/ups_tailor.h
upstst_fil.o: $(UPS_DIR)/inc/ups_touch.h
upstst_fil.o: $(UPS_DIR)/inc/ups_unconfigure.h
upstst_fil.o: $(UPS_DIR)/inc/ups_unk.h
upstst_fil.o: $(UPS_DIR)/inc/ups_unsetup.h
upstst_fil.o: $(UPS_DIR)/inc/ups_undeclare.h
upstst_fil.o: $(UPS_DIR)/inc/ups_verify.h
upstst_fil.o: upstst_parse.h
upstst_fil.o: upstst_macros.h
upstst_get.o: upstst_get.c
upstst_get.o: $(UPS_DIR)/inc/ups.h
upstst_get.o: $(UPS_DIR)/inc/upsact.h
upstst_get.o: $(UPS_DIR)/inc/upstyp.h
upstst_get.o: $(UPS_DIR)/inc/upslst.h
upstst_get.o: $(UPS_DIR)/inc/upsugo.h
upstst_get.o: $(UPS_DIR)/inc/upsmat.h
upstst_get.o: $(UPS_DIR)/inc/upscpy.h
upstst_get.o: $(UPS_DIR)/inc/upserr.h
upstst_get.o: $(UPS_DIR)/inc/upsfil.h
upstst_get.o: $(UPS_DIR)/inc/upstbl.h
upstst_get.o: $(UPS_DIR)/inc/upsget.h
upstst_get.o: $(UPS_DIR)/inc/upshlp.h
upstst_get.o: $(UPS_DIR)/inc/upskey.h
upstst_get.o: $(UPS_DIR)/inc/upsmem.h
upstst_get.o: $(UPS_DIR)/inc/upsutl.h
upstst_get.o: $(UPS_DIR)/inc/upsver.h
upstst_get.o: $(UPS_DIR)/inc/ups_list.h
upstst_get.o: $(UPS_DIR)/inc/ups_configure.h
upstst_get.o: $(UPS_DIR)/inc/ups_copy.h
upstst_get.o: $(UPS_DIR)/inc/ups_declare.h
upstst_get.o: $(UPS_DIR)/inc/ups_depend.h
upstst_get.o: $(UPS_DIR)/inc/ups_flavor.h
upstst_get.o: $(UPS_DIR)/inc/ups_get.h
upstst_get.o: $(UPS_DIR)/inc/ups_main.h
upstst_get.o: $(UPS_DIR)/inc/ups_modify.h
upstst_get.o: $(UPS_DIR)/inc/ups_setup.h
upstst_get.o: $(UPS_DIR)/inc/ups_start.h
upstst_get.o: $(UPS_DIR)/inc/ups_stop.h
upstst_get.o: $(UPS_DIR)/inc/ups_tailor.h
upstst_get.o: $(UPS_DIR)/inc/ups_touch.h
upstst_get.o: $(UPS_DIR)/inc/ups_unconfigure.h
upstst_get.o: $(UPS_DIR)/inc/ups_unk.h
upstst_get.o: $(UPS_DIR)/inc/ups_unsetup.h
upstst_get.o: $(UPS_DIR)/inc/ups_undeclare.h
upstst_get.o: $(UPS_DIR)/inc/ups_verify.h
upstst_get.o: upstst_parse.h
upstst_get.o: upstst_macros.h
upstst_mat.o: upstst_mat.c
upstst_mat.o: $(UPS_DIR)/inc/ups.h
upstst_mat.o: $(UPS_DIR)/inc/upsact.h
upstst_mat.o: $(UPS_DIR)/inc/upstyp.h
upstst_mat.o: $(UPS_DIR)/inc/upslst.h
upstst_mat.o: $(UPS_DIR)/inc/upsugo.h
upstst_mat.o: $(UPS_DIR)/inc/upsmat.h
upstst_mat.o: $(UPS_DIR)/inc/upscpy.h
upstst_mat.o: $(UPS_DIR)/inc/upserr.h
upstst_mat.o: $(UPS_DIR)/inc/upsfil.h
upstst_mat.o: $(UPS_DIR)/inc/upstbl.h
upstst_mat.o: $(UPS_DIR)/inc/upsget.h
upstst_mat.o: $(UPS_DIR)/inc/upshlp.h
upstst_mat.o: $(UPS_DIR)/inc/upskey.h
upstst_mat.o: $(UPS_DIR)/inc/upsmem.h
upstst_mat.o: $(UPS_DIR)/inc/upsutl.h
upstst_mat.o: $(UPS_DIR)/inc/upsver.h
upstst_mat.o: $(UPS_DIR)/inc/ups_list.h
upstst_mat.o: $(UPS_DIR)/inc/ups_configure.h
upstst_mat.o: $(UPS_DIR)/inc/ups_copy.h
upstst_mat.o: $(UPS_DIR)/inc/ups_declare.h
upstst_mat.o: $(UPS_DIR)/inc/ups_depend.h
upstst_mat.o: $(UPS_DIR)/inc/ups_flavor.h
upstst_mat.o: $(UPS_DIR)/inc/ups_get.h
upstst_mat.o: $(UPS_DIR)/inc/ups_main.h
upstst_mat.o: $(UPS_DIR)/inc/ups_modify.h
upstst_mat.o: $(UPS_DIR)/inc/ups_setup.h
upstst_mat.o: $(UPS_DIR)/inc/ups_start.h
upstst_mat.o: $(UPS_DIR)/inc/ups_stop.h
upstst_mat.o: $(UPS_DIR)/inc/ups_tailor.h
upstst_mat.o: $(UPS_DIR)/inc/ups_touch.h
upstst_mat.o: $(UPS_DIR)/inc/ups_unconfigure.h
upstst_mat.o: $(UPS_DIR)/inc/ups_unk.h
upstst_mat.o: $(UPS_DIR)/inc/ups_unsetup.h
upstst_mat.o: $(UPS_DIR)/inc/ups_undeclare.h
upstst_mat.o: $(UPS_DIR)/inc/ups_verify.h
upstst_mat.o: upstst_parse.h
upstst_mat.o: upstst_macros.h
upstst_misc.o: upstst_misc.c
upstst_misc.o: upstst_parse.h
upstst_misc.o: upstst_macros.h
upstst_misc.o: $(UPS_DIR)/inc/upserr.h
upstst_parse.o: upstst_parse.c
upstst_parse.o: upstst_parse.h
upstst_ugo.o: upstst_ugo.c
upstst_ugo.o: $(UPS_DIR)/inc/ups.h
upstst_ugo.o: $(UPS_DIR)/inc/upsact.h
upstst_ugo.o: $(UPS_DIR)/inc/upstyp.h
upstst_ugo.o: $(UPS_DIR)/inc/upslst.h
upstst_ugo.o: $(UPS_DIR)/inc/upsugo.h
upstst_ugo.o: $(UPS_DIR)/inc/upsmat.h
upstst_ugo.o: $(UPS_DIR)/inc/upscpy.h
upstst_ugo.o: $(UPS_DIR)/inc/upserr.h
upstst_ugo.o: $(UPS_DIR)/inc/upsfil.h
upstst_ugo.o: $(UPS_DIR)/inc/upstbl.h
upstst_ugo.o: $(UPS_DIR)/inc/upsget.h
upstst_ugo.o: $(UPS_DIR)/inc/upshlp.h
upstst_ugo.o: $(UPS_DIR)/inc/upskey.h
upstst_ugo.o: $(UPS_DIR)/inc/upsmem.h
upstst_ugo.o: $(UPS_DIR)/inc/upsutl.h
upstst_ugo.o: $(UPS_DIR)/inc/upsver.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_list.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_configure.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_copy.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_declare.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_depend.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_flavor.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_get.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_main.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_modify.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_setup.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_start.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_stop.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_tailor.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_touch.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_unconfigure.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_unk.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_unsetup.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_undeclare.h
upstst_ugo.o: $(UPS_DIR)/inc/ups_verify.h
upstst_ugo.o: upstst_parse.h
upstst_ugo.o: upstst_macros.h
