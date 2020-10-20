SHELL = /bin/sh
# 
#  1) This file is designed to be invoked by a shell script called premake
#  which selects supported options.
#
#  2) This file re-builds ups.
#
#  3) Environment:
# Environmental variables -
#
# Expected variables on the command line:
# CC=        The name of the appropriate compiler, with suitable flags
# CCCHK=     A set of compiler flags which check ANSI C strictly
#

.SUFFIXES : .c .o .A

.c.o :
	$(CC) -c -o $@ $(CCCHK) $(CFLAGS) $*.c

.c.A :
	@ echo '        $(CIA_DIR)/bin/cia -c $(CFLAGScia) $*.c'
	@               $(CIA_DIR)/bin/cia -c $(CFLAGScia) $*.c

#
SRC	= ../src
INC	= ../inc
LIB	= ../lib
BIN	= ../bin
DOC	= ../doc

INCDIRS   = -I$(INC)
CFLAGS	  = $(UPS_CFLAGS) $(INCDIRS)
CFLAGScia = $(INCDIRS)

#
#

OBJECTS = upserr.o upslst.o upsmem.o upsutl.o upstyp.o \
	  upsfil.o upsugo.o upsmat.o upskey.o upshlp.o \
	  upsver.o upsact.o upsget.o upstbl.o upscpy.o \
	  upsuname.o

COMMAND_OBJECTS = ups_list.o ups_setup.o ups_start.o ups_stop.o \
	          ups_configure.o ups_unconfigure.o ups_unsetup.o \
	          ups_tailor.o ups_unk.o ups_declare.o \
	          ups_depend.o ups_get.o ups_undeclare.o ups_flavor.o \
	          ups_copy.o ups_modify.o ups_touch.o ups_verify.o

CIAOBJECTS = upserr.A upslst.A upsmem.A upsutl.A upstyp.A \
	     upsfil.A upsugo.A upsmat.A ups_list.A  \
	     upskey.A upshlp.A ups_unk.A ups_setup.A \
	     upsver.A upsact.A upsget.A ups_start.A \
	     ups_stop.A ups_configure.A ups_unconfigure.A \
	     ups_unsetup.A ups_tailor.A upstbl.A \
             ups_list.A ups_depend.A ups_get.A ups_declare.A \
	     ups_undeclare.A ups_flavor.A ups_copy.A ups_modify.A \
             ups_unk.A ups_touch.A ups_verify.A upscpy.A \
	     upsuname.A

LDFLAGS = $(LLIBS)

#=============================================================================
# TARGETS
#=============================================================================

default:	
	@echo Please invoke this makefile using upspremake. >&2
	@exit 1

all : library main old

tag_file : 
	$(EMACS_DIR)/bin/etags -t *.c ../inc/*.h

main : $(BIN)/ups $(BIN)/ups_active $(BIN)/ups_parent $(BIN)/setup_fail $(BIN)/setup_win

# Old stuff people just expect to work that really isn't part of ups
old: $(BIN)/funame \
	$(BIN)/dropit

library : $(LIB)/libups.a

cia :	$(CIAOBJECTS)
	cia $(CIAOBJECTS)

#
# Build main line executable
#
$(BIN)/ups : ups_main.o library
	$(LD) ups_main.o -o $@  $(LDFLAGS)

$(BIN)/ups_active : ups_active.pl
	cp ups_active.pl $@
	chmod +x $@

$(BIN)/ups_parent : ups_parent.pl
	cp ups_parent.pl $@
	chmod +x $@

$(BIN)/setup_fail:
	echo "false" > $@

$(BIN)/setup_win:
	echo "true" > $@

#
# Build the old products people expect that arn't really dependencies
$(BIN)/funame : upsuname.o
	$(CC) $(CFLAGS) -c funame.c 
	$(LD) funame.o -o $@ upsuname.o $(OLD_LIBS) $(LDFLAGS)

$(BIN)/dropit :
	$(CC) $(CFLAGS) -c dropit.c 
	$(LD)  dropit.o -o $@ $(LDFLAGS)

#
# Compile UPS library source
#
$(LIB)/libups.a : $(OBJECTS) $(COMMAND_OBJECTS)
	@echo "=== UPS BUILD : Building Unix library (libups.a) ==="
	$(AR) $(ARFLAGS) $(LIB)/libups.a $?
	- if [ "$(RANLIB)" != "" -a -x "$(RANLIB)" ]; then \
		$(RANLIB) $(LIB)/libups.a; \
	fi
	@echo "=== UPS BUILD : Unix library (libups.a) built ==="
	@echo

#
# Now standard targets
#
clean spotless :
	@ ../bin/upsClean

make :
	@if [ "$(CCENV)" = "" ]; then ccenv=`uname`; else ccenv=$(CCENV); fi; \
	echo \
	"upsmake_make -cc '$(CC) $(CFLAGS)' -nostd -file Makefile" \
                                                        "-env $$ccenv *.c"; \
	../bin/upsmake_make -cc '$(CC) $(CFLAGS)' -nostd -file Makefile \
                                                         -env $$ccenv *.c

# DO NOT HAND EDIT BELOW THE START_DEPEND LINE. USE "upspremake make" IN THE
# src DIRECTORY TO GENERATE IT
#START_DEPEND
dropit.o: dropit.c
funame.o: funame.c
ups_configure.o: ups_configure.c
ups_configure.o: $(UPS_DIR)/inc/upserr.h
ups_configure.o: $(UPS_DIR)/inc/upsugo.h
ups_configure.o: $(UPS_DIR)/inc/upslst.h
ups_configure.o: $(UPS_DIR)/inc/upstyp.h
ups_configure.o: $(UPS_DIR)/inc/upsutl.h
ups_configure.o: $(UPS_DIR)/inc/upsmat.h
ups_configure.o: $(UPS_DIR)/inc/upsact.h
ups_configure.o: $(UPS_DIR)/inc/ups_main.h
ups_copy.o: ups_copy.c
ups_copy.o: $(UPS_DIR)/inc/upserr.h
ups_copy.o: $(UPS_DIR)/inc/upsugo.h
ups_copy.o: $(UPS_DIR)/inc/upslst.h
ups_copy.o: $(UPS_DIR)/inc/upstyp.h
ups_copy.o: $(UPS_DIR)/inc/upsutl.h
ups_copy.o: $(UPS_DIR)/inc/upsget.h
ups_copy.o: $(UPS_DIR)/inc/upsmat.h
ups_copy.o: $(UPS_DIR)/inc/upsact.h
ups_copy.o: $(UPS_DIR)/inc/upsmem.h
ups_copy.o: $(UPS_DIR)/inc/upsfil.h
ups_copy.o: $(UPS_DIR)/inc/upstbl.h
ups_copy.o: $(UPS_DIR)/inc/ups_main.h
ups_copy.o: $(UPS_DIR)/inc/ups_declare.h
ups_copy.o: $(UPS_DIR)/inc/ups_copy.h
ups_declare.o: ups_declare.c
ups_declare.o: $(UPS_DIR)/inc/ups.h
ups_declare.o: $(UPS_DIR)/inc/upsact.h
ups_declare.o: $(UPS_DIR)/inc/upstyp.h
ups_declare.o: $(UPS_DIR)/inc/upslst.h
ups_declare.o: $(UPS_DIR)/inc/upsugo.h
ups_declare.o: $(UPS_DIR)/inc/upsmat.h
ups_declare.o: $(UPS_DIR)/inc/upscpy.h
ups_declare.o: $(UPS_DIR)/inc/upserr.h
ups_declare.o: $(UPS_DIR)/inc/upsfil.h
ups_declare.o: $(UPS_DIR)/inc/upstbl.h
ups_declare.o: $(UPS_DIR)/inc/upsget.h
ups_declare.o: $(UPS_DIR)/inc/upshlp.h
ups_declare.o: $(UPS_DIR)/inc/upskey.h
ups_declare.o: $(UPS_DIR)/inc/upsmem.h
ups_declare.o: $(UPS_DIR)/inc/upsutl.h
ups_declare.o: $(UPS_DIR)/inc/upsver.h
ups_declare.o: $(UPS_DIR)/inc/ups_list.h
ups_declare.o: $(UPS_DIR)/inc/ups_configure.h
ups_declare.o: $(UPS_DIR)/inc/ups_copy.h
ups_declare.o: $(UPS_DIR)/inc/ups_declare.h
ups_declare.o: $(UPS_DIR)/inc/ups_depend.h
ups_declare.o: $(UPS_DIR)/inc/ups_flavor.h
ups_declare.o: $(UPS_DIR)/inc/ups_get.h
ups_declare.o: $(UPS_DIR)/inc/ups_main.h
ups_declare.o: $(UPS_DIR)/inc/ups_modify.h
ups_declare.o: $(UPS_DIR)/inc/ups_setup.h
ups_declare.o: $(UPS_DIR)/inc/ups_start.h
ups_declare.o: $(UPS_DIR)/inc/ups_stop.h
ups_declare.o: $(UPS_DIR)/inc/ups_tailor.h
ups_declare.o: $(UPS_DIR)/inc/ups_touch.h
ups_declare.o: $(UPS_DIR)/inc/ups_unconfigure.h
ups_declare.o: $(UPS_DIR)/inc/ups_unk.h
ups_declare.o: $(UPS_DIR)/inc/ups_unsetup.h
ups_declare.o: $(UPS_DIR)/inc/ups_undeclare.h
ups_declare.o: $(UPS_DIR)/inc/ups_verify.h
ups_depend.o: ups_depend.c
ups_depend.o: $(UPS_DIR)/inc/ups_depend.h
ups_depend.o: $(UPS_DIR)/inc/upsugo.h
ups_depend.o: $(UPS_DIR)/inc/upslst.h
ups_depend.o: $(UPS_DIR)/inc/upstyp.h
ups_depend.o: $(UPS_DIR)/inc/upserr.h
ups_depend.o: $(UPS_DIR)/inc/upsact.h
ups_depend.o: $(UPS_DIR)/inc/upsmat.h
ups_depend.o: $(UPS_DIR)/inc/ups_main.h
ups_flavor.o: ups_flavor.c
ups_flavor.o: $(UPS_DIR)/inc/ups.h
ups_flavor.o: $(UPS_DIR)/inc/upsact.h
ups_flavor.o: $(UPS_DIR)/inc/upstyp.h
ups_flavor.o: $(UPS_DIR)/inc/upslst.h
ups_flavor.o: $(UPS_DIR)/inc/upsugo.h
ups_flavor.o: $(UPS_DIR)/inc/upsmat.h
ups_flavor.o: $(UPS_DIR)/inc/upscpy.h
ups_flavor.o: $(UPS_DIR)/inc/upserr.h
ups_flavor.o: $(UPS_DIR)/inc/upsfil.h
ups_flavor.o: $(UPS_DIR)/inc/upstbl.h
ups_flavor.o: $(UPS_DIR)/inc/upsget.h
ups_flavor.o: $(UPS_DIR)/inc/upshlp.h
ups_flavor.o: $(UPS_DIR)/inc/upskey.h
ups_flavor.o: $(UPS_DIR)/inc/upsmem.h
ups_flavor.o: $(UPS_DIR)/inc/upsutl.h
ups_flavor.o: $(UPS_DIR)/inc/upsver.h
ups_flavor.o: $(UPS_DIR)/inc/ups_list.h
ups_flavor.o: $(UPS_DIR)/inc/ups_configure.h
ups_flavor.o: $(UPS_DIR)/inc/ups_copy.h
ups_flavor.o: $(UPS_DIR)/inc/ups_declare.h
ups_flavor.o: $(UPS_DIR)/inc/ups_depend.h
ups_flavor.o: $(UPS_DIR)/inc/ups_flavor.h
ups_flavor.o: $(UPS_DIR)/inc/ups_get.h
ups_flavor.o: $(UPS_DIR)/inc/ups_main.h
ups_flavor.o: $(UPS_DIR)/inc/ups_modify.h
ups_flavor.o: $(UPS_DIR)/inc/ups_setup.h
ups_flavor.o: $(UPS_DIR)/inc/ups_start.h
ups_flavor.o: $(UPS_DIR)/inc/ups_stop.h
ups_flavor.o: $(UPS_DIR)/inc/ups_tailor.h
ups_flavor.o: $(UPS_DIR)/inc/ups_touch.h
ups_flavor.o: $(UPS_DIR)/inc/ups_unconfigure.h
ups_flavor.o: $(UPS_DIR)/inc/ups_unk.h
ups_flavor.o: $(UPS_DIR)/inc/ups_unsetup.h
ups_flavor.o: $(UPS_DIR)/inc/ups_undeclare.h
ups_flavor.o: $(UPS_DIR)/inc/ups_verify.h
ups_get.o: ups_get.c
ups_get.o: $(UPS_DIR)/inc/upserr.h
ups_get.o: $(UPS_DIR)/inc/upsugo.h
ups_get.o: $(UPS_DIR)/inc/upslst.h
ups_get.o: $(UPS_DIR)/inc/upstyp.h
ups_get.o: $(UPS_DIR)/inc/upsutl.h
ups_get.o: $(UPS_DIR)/inc/upsmat.h
ups_get.o: $(UPS_DIR)/inc/upsact.h
ups_get.o: $(UPS_DIR)/inc/upsmem.h
ups_get.o: $(UPS_DIR)/inc/upsget.h
ups_get.o: $(UPS_DIR)/inc/ups_main.h
ups_list.o: ups_list.c
ups_list.o: $(UPS_DIR)/inc/ups.h
ups_list.o: $(UPS_DIR)/inc/upsact.h
ups_list.o: $(UPS_DIR)/inc/upstyp.h
ups_list.o: $(UPS_DIR)/inc/upslst.h
ups_list.o: $(UPS_DIR)/inc/upsugo.h
ups_list.o: $(UPS_DIR)/inc/upsmat.h
ups_list.o: $(UPS_DIR)/inc/upscpy.h
ups_list.o: $(UPS_DIR)/inc/upserr.h
ups_list.o: $(UPS_DIR)/inc/upsfil.h
ups_list.o: $(UPS_DIR)/inc/upstbl.h
ups_list.o: $(UPS_DIR)/inc/upsget.h
ups_list.o: $(UPS_DIR)/inc/upshlp.h
ups_list.o: $(UPS_DIR)/inc/upskey.h
ups_list.o: $(UPS_DIR)/inc/upsmem.h
ups_list.o: $(UPS_DIR)/inc/upsutl.h
ups_list.o: $(UPS_DIR)/inc/upsver.h
ups_list.o: $(UPS_DIR)/inc/ups_list.h
ups_list.o: $(UPS_DIR)/inc/ups_configure.h
ups_list.o: $(UPS_DIR)/inc/ups_copy.h
ups_list.o: $(UPS_DIR)/inc/ups_declare.h
ups_list.o: $(UPS_DIR)/inc/ups_depend.h
ups_list.o: $(UPS_DIR)/inc/ups_flavor.h
ups_list.o: $(UPS_DIR)/inc/ups_get.h
ups_list.o: $(UPS_DIR)/inc/ups_main.h
ups_list.o: $(UPS_DIR)/inc/ups_modify.h
ups_list.o: $(UPS_DIR)/inc/ups_setup.h
ups_list.o: $(UPS_DIR)/inc/ups_start.h
ups_list.o: $(UPS_DIR)/inc/ups_stop.h
ups_list.o: $(UPS_DIR)/inc/ups_tailor.h
ups_list.o: $(UPS_DIR)/inc/ups_touch.h
ups_list.o: $(UPS_DIR)/inc/ups_unconfigure.h
ups_list.o: $(UPS_DIR)/inc/ups_unk.h
ups_list.o: $(UPS_DIR)/inc/ups_unsetup.h
ups_list.o: $(UPS_DIR)/inc/ups_undeclare.h
ups_list.o: $(UPS_DIR)/inc/ups_verify.h
ups_main.o: ups_main.c
ups_main.o: $(UPS_DIR)/inc/ups_main.h
ups_main.o: $(UPS_DIR)/inc/upslst.h
ups_main.o: $(UPS_DIR)/inc/upstyp.h
ups_main.o: $(UPS_DIR)/inc/ups_setup.h
ups_main.o: $(UPS_DIR)/inc/upsugo.h
ups_main.o: $(UPS_DIR)/inc/ups_unsetup.h
ups_main.o: $(UPS_DIR)/inc/ups_start.h
ups_main.o: $(UPS_DIR)/inc/ups_stop.h
ups_main.o: $(UPS_DIR)/inc/ups_configure.h
ups_main.o: $(UPS_DIR)/inc/ups_unconfigure.h
ups_main.o: $(UPS_DIR)/inc/ups_list.h
ups_modify.o: ups_modify.c
ups_modify.o: $(UPS_DIR)/inc/ups.h
ups_modify.o: $(UPS_DIR)/inc/upsact.h
ups_modify.o: $(UPS_DIR)/inc/upstyp.h
ups_modify.o: $(UPS_DIR)/inc/upslst.h
ups_modify.o: $(UPS_DIR)/inc/upsugo.h
ups_modify.o: $(UPS_DIR)/inc/upsmat.h
ups_modify.o: $(UPS_DIR)/inc/upscpy.h
ups_modify.o: $(UPS_DIR)/inc/upserr.h
ups_modify.o: $(UPS_DIR)/inc/upsfil.h
ups_modify.o: $(UPS_DIR)/inc/upstbl.h
ups_modify.o: $(UPS_DIR)/inc/upsget.h
ups_modify.o: $(UPS_DIR)/inc/upshlp.h
ups_modify.o: $(UPS_DIR)/inc/upskey.h
ups_modify.o: $(UPS_DIR)/inc/upsmem.h
ups_modify.o: $(UPS_DIR)/inc/upsutl.h
ups_modify.o: $(UPS_DIR)/inc/upsver.h
ups_modify.o: $(UPS_DIR)/inc/ups_list.h
ups_modify.o: $(UPS_DIR)/inc/ups_configure.h
ups_modify.o: $(UPS_DIR)/inc/ups_copy.h
ups_modify.o: $(UPS_DIR)/inc/ups_declare.h
ups_modify.o: $(UPS_DIR)/inc/ups_depend.h
ups_modify.o: $(UPS_DIR)/inc/ups_flavor.h
ups_modify.o: $(UPS_DIR)/inc/ups_get.h
ups_modify.o: $(UPS_DIR)/inc/ups_main.h
ups_modify.o: $(UPS_DIR)/inc/ups_modify.h
ups_modify.o: $(UPS_DIR)/inc/ups_setup.h
ups_modify.o: $(UPS_DIR)/inc/ups_start.h
ups_modify.o: $(UPS_DIR)/inc/ups_stop.h
ups_modify.o: $(UPS_DIR)/inc/ups_tailor.h
ups_modify.o: $(UPS_DIR)/inc/ups_touch.h
ups_modify.o: $(UPS_DIR)/inc/ups_unconfigure.h
ups_modify.o: $(UPS_DIR)/inc/ups_unk.h
ups_modify.o: $(UPS_DIR)/inc/ups_unsetup.h
ups_modify.o: $(UPS_DIR)/inc/ups_undeclare.h
ups_modify.o: $(UPS_DIR)/inc/ups_verify.h
ups_setup.o: ups_setup.c
ups_setup.o: $(UPS_DIR)/inc/upserr.h
ups_setup.o: $(UPS_DIR)/inc/upsugo.h
ups_setup.o: $(UPS_DIR)/inc/upslst.h
ups_setup.o: $(UPS_DIR)/inc/upstyp.h
ups_setup.o: $(UPS_DIR)/inc/upsutl.h
ups_setup.o: $(UPS_DIR)/inc/upsmat.h
ups_setup.o: $(UPS_DIR)/inc/upsact.h
ups_setup.o: $(UPS_DIR)/inc/ups_main.h
ups_start.o: ups_start.c
ups_start.o: $(UPS_DIR)/inc/upserr.h
ups_start.o: $(UPS_DIR)/inc/upsugo.h
ups_start.o: $(UPS_DIR)/inc/upslst.h
ups_start.o: $(UPS_DIR)/inc/upstyp.h
ups_start.o: $(UPS_DIR)/inc/upsutl.h
ups_start.o: $(UPS_DIR)/inc/upsmat.h
ups_start.o: $(UPS_DIR)/inc/upsact.h
ups_start.o: $(UPS_DIR)/inc/ups_main.h
ups_stop.o: ups_stop.c
ups_stop.o: $(UPS_DIR)/inc/upserr.h
ups_stop.o: $(UPS_DIR)/inc/upsugo.h
ups_stop.o: $(UPS_DIR)/inc/upslst.h
ups_stop.o: $(UPS_DIR)/inc/upstyp.h
ups_stop.o: $(UPS_DIR)/inc/upsutl.h
ups_stop.o: $(UPS_DIR)/inc/upsmat.h
ups_stop.o: $(UPS_DIR)/inc/upsact.h
ups_stop.o: $(UPS_DIR)/inc/ups_main.h
ups_tailor.o: ups_tailor.c
ups_tailor.o: $(UPS_DIR)/inc/upserr.h
ups_tailor.o: $(UPS_DIR)/inc/upsfil.h
ups_tailor.o: $(UPS_DIR)/inc/upstyp.h
ups_tailor.o: $(UPS_DIR)/inc/upslst.h
ups_tailor.o: $(UPS_DIR)/inc/upstbl.h
ups_tailor.o: $(UPS_DIR)/inc/upsugo.h
ups_tailor.o: $(UPS_DIR)/inc/upsutl.h
ups_tailor.o: $(UPS_DIR)/inc/upsmat.h
ups_tailor.o: $(UPS_DIR)/inc/upsmem.h
ups_tailor.o: $(UPS_DIR)/inc/upsact.h
ups_tailor.o: $(UPS_DIR)/inc/ups_main.h
ups_touch.o: ups_touch.c
ups_touch.o: $(UPS_DIR)/inc/ups.h
ups_touch.o: $(UPS_DIR)/inc/upsact.h
ups_touch.o: $(UPS_DIR)/inc/upstyp.h
ups_touch.o: $(UPS_DIR)/inc/upslst.h
ups_touch.o: $(UPS_DIR)/inc/upsugo.h
ups_touch.o: $(UPS_DIR)/inc/upsmat.h
ups_touch.o: $(UPS_DIR)/inc/upscpy.h
ups_touch.o: $(UPS_DIR)/inc/upserr.h
ups_touch.o: $(UPS_DIR)/inc/upsfil.h
ups_touch.o: $(UPS_DIR)/inc/upstbl.h
ups_touch.o: $(UPS_DIR)/inc/upsget.h
ups_touch.o: $(UPS_DIR)/inc/upshlp.h
ups_touch.o: $(UPS_DIR)/inc/upskey.h
ups_touch.o: $(UPS_DIR)/inc/upsmem.h
ups_touch.o: $(UPS_DIR)/inc/upsutl.h
ups_touch.o: $(UPS_DIR)/inc/upsver.h
ups_touch.o: $(UPS_DIR)/inc/ups_list.h
ups_touch.o: $(UPS_DIR)/inc/ups_configure.h
ups_touch.o: $(UPS_DIR)/inc/ups_copy.h
ups_touch.o: $(UPS_DIR)/inc/ups_declare.h
ups_touch.o: $(UPS_DIR)/inc/ups_depend.h
ups_touch.o: $(UPS_DIR)/inc/ups_flavor.h
ups_touch.o: $(UPS_DIR)/inc/ups_get.h
ups_touch.o: $(UPS_DIR)/inc/ups_main.h
ups_touch.o: $(UPS_DIR)/inc/ups_modify.h
ups_touch.o: $(UPS_DIR)/inc/ups_setup.h
ups_touch.o: $(UPS_DIR)/inc/ups_start.h
ups_touch.o: $(UPS_DIR)/inc/ups_stop.h
ups_touch.o: $(UPS_DIR)/inc/ups_tailor.h
ups_touch.o: $(UPS_DIR)/inc/ups_touch.h
ups_touch.o: $(UPS_DIR)/inc/ups_unconfigure.h
ups_touch.o: $(UPS_DIR)/inc/ups_unk.h
ups_touch.o: $(UPS_DIR)/inc/ups_unsetup.h
ups_touch.o: $(UPS_DIR)/inc/ups_undeclare.h
ups_touch.o: $(UPS_DIR)/inc/ups_verify.h
ups_unconfigure.o: ups_unconfigure.c
ups_unconfigure.o: $(UPS_DIR)/inc/upserr.h
ups_unconfigure.o: $(UPS_DIR)/inc/upsugo.h
ups_unconfigure.o: $(UPS_DIR)/inc/upslst.h
ups_unconfigure.o: $(UPS_DIR)/inc/upstyp.h
ups_unconfigure.o: $(UPS_DIR)/inc/upsutl.h
ups_unconfigure.o: $(UPS_DIR)/inc/upsmat.h
ups_unconfigure.o: $(UPS_DIR)/inc/upsact.h
ups_unconfigure.o: $(UPS_DIR)/inc/ups_main.h
ups_undeclare.o: ups_undeclare.c
ups_undeclare.o: $(UPS_DIR)/inc/ups.h
ups_undeclare.o: $(UPS_DIR)/inc/upsact.h
ups_undeclare.o: $(UPS_DIR)/inc/upstyp.h
ups_undeclare.o: $(UPS_DIR)/inc/upslst.h
ups_undeclare.o: $(UPS_DIR)/inc/upsugo.h
ups_undeclare.o: $(UPS_DIR)/inc/upsmat.h
ups_undeclare.o: $(UPS_DIR)/inc/upscpy.h
ups_undeclare.o: $(UPS_DIR)/inc/upserr.h
ups_undeclare.o: $(UPS_DIR)/inc/upsfil.h
ups_undeclare.o: $(UPS_DIR)/inc/upstbl.h
ups_undeclare.o: $(UPS_DIR)/inc/upsget.h
ups_undeclare.o: $(UPS_DIR)/inc/upshlp.h
ups_undeclare.o: $(UPS_DIR)/inc/upskey.h
ups_undeclare.o: $(UPS_DIR)/inc/upsmem.h
ups_undeclare.o: $(UPS_DIR)/inc/upsutl.h
ups_undeclare.o: $(UPS_DIR)/inc/upsver.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_list.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_configure.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_copy.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_declare.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_depend.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_flavor.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_get.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_main.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_modify.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_setup.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_start.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_stop.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_tailor.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_touch.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_unconfigure.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_unk.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_unsetup.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_undeclare.h
ups_undeclare.o: $(UPS_DIR)/inc/ups_verify.h
ups_unk.o: ups_unk.c
ups_unk.o: $(UPS_DIR)/inc/ups_unk.h
ups_unk.o: $(UPS_DIR)/inc/upsugo.h
ups_unk.o: $(UPS_DIR)/inc/upslst.h
ups_unk.o: $(UPS_DIR)/inc/upstyp.h
ups_unk.o: $(UPS_DIR)/inc/upserr.h
ups_unk.o: $(UPS_DIR)/inc/upsutl.h
ups_unk.o: $(UPS_DIR)/inc/upsmat.h
ups_unk.o: $(UPS_DIR)/inc/upsact.h
ups_unsetup.o: ups_unsetup.c
ups_unsetup.o: $(UPS_DIR)/inc/upserr.h
ups_unsetup.o: $(UPS_DIR)/inc/upsugo.h
ups_unsetup.o: $(UPS_DIR)/inc/upslst.h
ups_unsetup.o: $(UPS_DIR)/inc/upstyp.h
ups_unsetup.o: $(UPS_DIR)/inc/upsutl.h
ups_unsetup.o: $(UPS_DIR)/inc/upsmat.h
ups_unsetup.o: $(UPS_DIR)/inc/upsact.h
ups_unsetup.o: $(UPS_DIR)/inc/ups_main.h
ups_verify.o: ups_verify.c
ups_verify.o: $(UPS_DIR)/inc/upserr.h
ups_verify.o: $(UPS_DIR)/inc/upsugo.h
ups_verify.o: $(UPS_DIR)/inc/upslst.h
ups_verify.o: $(UPS_DIR)/inc/upstyp.h
ups_verify.o: $(UPS_DIR)/inc/upsutl.h
ups_verify.o: $(UPS_DIR)/inc/upsmat.h
ups_verify.o: $(UPS_DIR)/inc/upsact.h
ups_verify.o: $(UPS_DIR)/inc/upsget.h
ups_verify.o: $(UPS_DIR)/inc/upsmem.h
ups_verify.o: $(UPS_DIR)/inc/ups_main.h
ups_verify.o: $(UPS_DIR)/inc/ups_verify.h
upsact.o: upsact.c
upsact.o: $(UPS_DIR)/inc/upsact.h
upsact.o: $(UPS_DIR)/inc/upstyp.h
upsact.o: $(UPS_DIR)/inc/upslst.h
upsact.o: $(UPS_DIR)/inc/upsugo.h
upsact.o: $(UPS_DIR)/inc/upsmat.h
upsact.o: $(UPS_DIR)/inc/upserr.h
upsact.o: $(UPS_DIR)/inc/upsver.h
upsact.o: $(UPS_DIR)/inc/upsutl.h
upsact.o: $(UPS_DIR)/inc/upsmem.h
upsact.o: $(UPS_DIR)/inc/upskey.h
upsact.o: $(UPS_DIR)/inc/upsget.h
upscpy.o: upscpy.c
upscpy.o: $(UPS_DIR)/inc/ups.h
upscpy.o: $(UPS_DIR)/inc/upsact.h
upscpy.o: $(UPS_DIR)/inc/upstyp.h
upscpy.o: $(UPS_DIR)/inc/upslst.h
upscpy.o: $(UPS_DIR)/inc/upsugo.h
upscpy.o: $(UPS_DIR)/inc/upsmat.h
upscpy.o: $(UPS_DIR)/inc/upscpy.h
upscpy.o: $(UPS_DIR)/inc/upserr.h
upscpy.o: $(UPS_DIR)/inc/upsfil.h
upscpy.o: $(UPS_DIR)/inc/upstbl.h
upscpy.o: $(UPS_DIR)/inc/upsget.h
upscpy.o: $(UPS_DIR)/inc/upshlp.h
upscpy.o: $(UPS_DIR)/inc/upskey.h
upscpy.o: $(UPS_DIR)/inc/upsmem.h
upscpy.o: $(UPS_DIR)/inc/upsutl.h
upscpy.o: $(UPS_DIR)/inc/upsver.h
upscpy.o: $(UPS_DIR)/inc/ups_list.h
upscpy.o: $(UPS_DIR)/inc/ups_configure.h
upscpy.o: $(UPS_DIR)/inc/ups_copy.h
upscpy.o: $(UPS_DIR)/inc/ups_declare.h
upscpy.o: $(UPS_DIR)/inc/ups_depend.h
upscpy.o: $(UPS_DIR)/inc/ups_flavor.h
upscpy.o: $(UPS_DIR)/inc/ups_get.h
upscpy.o: $(UPS_DIR)/inc/ups_main.h
upscpy.o: $(UPS_DIR)/inc/ups_modify.h
upscpy.o: $(UPS_DIR)/inc/ups_setup.h
upscpy.o: $(UPS_DIR)/inc/ups_start.h
upscpy.o: $(UPS_DIR)/inc/ups_stop.h
upscpy.o: $(UPS_DIR)/inc/ups_tailor.h
upscpy.o: $(UPS_DIR)/inc/ups_touch.h
upscpy.o: $(UPS_DIR)/inc/ups_unconfigure.h
upscpy.o: $(UPS_DIR)/inc/ups_unk.h
upscpy.o: $(UPS_DIR)/inc/ups_unsetup.h
upscpy.o: $(UPS_DIR)/inc/ups_undeclare.h
upscpy.o: $(UPS_DIR)/inc/ups_verify.h
upserr.o: upserr.c
upserr.o: $(UPS_DIR)/inc/upserr.h
upsfil.o: upsfil.c
upsfil.o: $(UPS_DIR)/inc/upsutl.h
upsfil.o: $(UPS_DIR)/inc/upstyp.h
upsfil.o: $(UPS_DIR)/inc/upslst.h
upsfil.o: $(UPS_DIR)/inc/upsugo.h
upsfil.o: $(UPS_DIR)/inc/upstbl.h
upsfil.o: $(UPS_DIR)/inc/upsmem.h
upsfil.o: $(UPS_DIR)/inc/upserr.h
upsfil.o: $(UPS_DIR)/inc/upskey.h
upsfil.o: $(UPS_DIR)/inc/upsfil.h
upsfil.o: $(UPS_DIR)/inc/upsver.h
upsfil.o: $(UPS_DIR)/inc/upsget.h
upsget.o: upsget.c
upsget.o: $(UPS_DIR)/inc/ups.h
upsget.o: $(UPS_DIR)/inc/upsact.h
upsget.o: $(UPS_DIR)/inc/upstyp.h
upsget.o: $(UPS_DIR)/inc/upslst.h
upsget.o: $(UPS_DIR)/inc/upsugo.h
upsget.o: $(UPS_DIR)/inc/upsmat.h
upsget.o: $(UPS_DIR)/inc/upscpy.h
upsget.o: $(UPS_DIR)/inc/upserr.h
upsget.o: $(UPS_DIR)/inc/upsfil.h
upsget.o: $(UPS_DIR)/inc/upstbl.h
upsget.o: $(UPS_DIR)/inc/upsget.h
upsget.o: $(UPS_DIR)/inc/upshlp.h
upsget.o: $(UPS_DIR)/inc/upskey.h
upsget.o: $(UPS_DIR)/inc/upsmem.h
upsget.o: $(UPS_DIR)/inc/upsutl.h
upsget.o: $(UPS_DIR)/inc/upsver.h
upsget.o: $(UPS_DIR)/inc/ups_list.h
upsget.o: $(UPS_DIR)/inc/ups_configure.h
upsget.o: $(UPS_DIR)/inc/ups_copy.h
upsget.o: $(UPS_DIR)/inc/ups_declare.h
upsget.o: $(UPS_DIR)/inc/ups_depend.h
upsget.o: $(UPS_DIR)/inc/ups_flavor.h
upsget.o: $(UPS_DIR)/inc/ups_get.h
upsget.o: $(UPS_DIR)/inc/ups_main.h
upsget.o: $(UPS_DIR)/inc/ups_modify.h
upsget.o: $(UPS_DIR)/inc/ups_setup.h
upsget.o: $(UPS_DIR)/inc/ups_start.h
upsget.o: $(UPS_DIR)/inc/ups_stop.h
upsget.o: $(UPS_DIR)/inc/ups_tailor.h
upsget.o: $(UPS_DIR)/inc/ups_touch.h
upsget.o: $(UPS_DIR)/inc/ups_unconfigure.h
upsget.o: $(UPS_DIR)/inc/ups_unk.h
upsget.o: $(UPS_DIR)/inc/ups_unsetup.h
upsget.o: $(UPS_DIR)/inc/ups_undeclare.h
upsget.o: $(UPS_DIR)/inc/ups_verify.h
upshlp.o: upshlp.c
upshlp.o: $(UPS_DIR)/inc/upstyp.h
upshlp.o: $(UPS_DIR)/inc/upslst.h
upshlp.o: $(UPS_DIR)/inc/upsutl.h
upshlp.o: $(UPS_DIR)/inc/upsugo.h
upskey.o: upskey.c
upskey.o: $(UPS_DIR)/inc/upstyp.h
upskey.o: $(UPS_DIR)/inc/upslst.h
upskey.o: $(UPS_DIR)/inc/upsmem.h
upskey.o: $(UPS_DIR)/inc/upsutl.h
upskey.o: $(UPS_DIR)/inc/upsugo.h
upskey.o: $(UPS_DIR)/inc/upskey.h
upslst.o: upslst.c
upslst.o: $(UPS_DIR)/inc/upslst.h
upslst.o: $(UPS_DIR)/inc/upsmem.h
upsmat.o: upsmat.c
upsmat.o: $(UPS_DIR)/inc/upsmat.h
upsmat.o: $(UPS_DIR)/inc/upsugo.h
upsmat.o: $(UPS_DIR)/inc/upslst.h
upsmat.o: $(UPS_DIR)/inc/upstyp.h
upsmat.o: $(UPS_DIR)/inc/upserr.h
upsmat.o: $(UPS_DIR)/inc/upsmem.h
upsmat.o: $(UPS_DIR)/inc/upsfil.h
upsmat.o: $(UPS_DIR)/inc/upstbl.h
upsmat.o: $(UPS_DIR)/inc/upsutl.h
upsmat.o: $(UPS_DIR)/inc/upsget.h
upsmat.o: $(UPS_DIR)/inc/upsver.h
upsmem.o: upsmem.c
upsmem.o: $(UPS_DIR)/inc/upserr.h
upsmem.o: $(UPS_DIR)/inc/upsmem.h
upstbl.o: upstbl.c
upstbl.o: $(UPS_DIR)/inc/upstbl.h
upstbl.o: $(UPS_DIR)/inc/upserr.h
upstyp.o: upstyp.c
upstyp.o: $(UPS_DIR)/inc/upsutl.h
upstyp.o: $(UPS_DIR)/inc/upstyp.h
upstyp.o: $(UPS_DIR)/inc/upslst.h
upstyp.o: $(UPS_DIR)/inc/upsugo.h
upstyp.o: $(UPS_DIR)/inc/upsmem.h
upstyp.o: $(UPS_DIR)/inc/upserr.h
upsugo.o: upsugo.c
upsugo.o: $(UPS_DIR)/inc/ups.h
upsugo.o: $(UPS_DIR)/inc/upsact.h
upsugo.o: $(UPS_DIR)/inc/upstyp.h
upsugo.o: $(UPS_DIR)/inc/upslst.h
upsugo.o: $(UPS_DIR)/inc/upsugo.h
upsugo.o: $(UPS_DIR)/inc/upsmat.h
upsugo.o: $(UPS_DIR)/inc/upscpy.h
upsugo.o: $(UPS_DIR)/inc/upserr.h
upsugo.o: $(UPS_DIR)/inc/upsfil.h
upsugo.o: $(UPS_DIR)/inc/upstbl.h
upsugo.o: $(UPS_DIR)/inc/upsget.h
upsugo.o: $(UPS_DIR)/inc/upshlp.h
upsugo.o: $(UPS_DIR)/inc/upskey.h
upsugo.o: $(UPS_DIR)/inc/upsmem.h
upsugo.o: $(UPS_DIR)/inc/upsutl.h
upsugo.o: $(UPS_DIR)/inc/upsver.h
upsugo.o: $(UPS_DIR)/inc/ups_list.h
upsugo.o: $(UPS_DIR)/inc/ups_configure.h
upsugo.o: $(UPS_DIR)/inc/ups_copy.h
upsugo.o: $(UPS_DIR)/inc/ups_declare.h
upsugo.o: $(UPS_DIR)/inc/ups_depend.h
upsugo.o: $(UPS_DIR)/inc/ups_flavor.h
upsugo.o: $(UPS_DIR)/inc/ups_get.h
upsugo.o: $(UPS_DIR)/inc/ups_main.h
upsugo.o: $(UPS_DIR)/inc/ups_modify.h
upsugo.o: $(UPS_DIR)/inc/ups_setup.h
upsugo.o: $(UPS_DIR)/inc/ups_start.h
upsugo.o: $(UPS_DIR)/inc/ups_stop.h
upsugo.o: $(UPS_DIR)/inc/ups_tailor.h
upsugo.o: $(UPS_DIR)/inc/ups_touch.h
upsugo.o: $(UPS_DIR)/inc/ups_unconfigure.h
upsugo.o: $(UPS_DIR)/inc/ups_unk.h
upsugo.o: $(UPS_DIR)/inc/ups_unsetup.h
upsugo.o: $(UPS_DIR)/inc/ups_undeclare.h
upsugo.o: $(UPS_DIR)/inc/ups_verify.h
upsutl.o: upsutl.c
upsutl.o: $(UPS_DIR)/inc/upsutl.h
upsutl.o: $(UPS_DIR)/inc/upstyp.h
upsutl.o: $(UPS_DIR)/inc/upslst.h
upsutl.o: $(UPS_DIR)/inc/upsugo.h
upsutl.o: $(UPS_DIR)/inc/upserr.h
upsutl.o: $(UPS_DIR)/inc/upsmem.h
upsutl.o: $(UPS_DIR)/inc/upsfil.h
upsutl.o: $(UPS_DIR)/inc/upstbl.h
upsutl.o: $(UPS_DIR)/inc/upsget.h
upsutl.o: $(UPS_DIR)/inc/upsact.h
upsutl.o: $(UPS_DIR)/inc/upsmat.h
upsutl.o: $(UPS_DIR)/inc/upsver.h
upsver.o: upsver.c
upsver.o: $(UPS_DIR)/inc/upserr.h
upsver.o: $(UPS_DIR)/inc/upsver.h
