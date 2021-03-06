#!/usr/local/bin/perl
#
###eval "exec /usr/products/IRIX/perl/v4.0/bin/perl -S $0 $*"
#    if $running_under_some_shell;
#			# this emulates #! processing on NIH machines.
#			# (remove #! line above if indigestible)

#++
# FACILITY:	make
#
# ABSTRACT:	perl script which will take make dependencies generated by cc -M
#		(or equivalent) and modify them as follows:
#
#		   o	Dependencies on standard include areas can be removed
#			(optionally).  This may be necessary when compiling on
#			different platforms since not all vendors/OSes place
#			standard header files in the same places.
#
#		   o	Full path names in the dependencies file are changed
#			to use the product_DIR environment variables.  This
#			permits position (directory) independence when building
#			a product across platforms or locations, etc.
#
# ENVIRONMENT:	perl script.
#		makeDependFix.perl
#
# INPUTS:
#
#   stdin	Dependencies.
#
#   HARDPATH	Boolean flag (0 == FALSE, non-0 == TRUE) indicating whether
#		dependency paths should remain hardwired (TRUE) or substituted
#		with product _DIR (FALSE) to provide some position independence.
#
#   NOSTD	White-space separated list of the paths for standard headers.
#		If present, dependencies in theses paths will be removed from
#		the resulting dependency file.
#
#   CCLINE	White-space separated list of cc switches used in the cc -M
#		line.  Only the -I switches specifying include paths are used.
#
#   PRODUCTS	A white-space separated list of paths associated with UPS pro-
#		ducts databases.  This usually corresponds to the environment
#		variable of the same name.  If present, only environment var-
#		iable names that correspond to UPS products are used make -I
#		paths position independent.  PRODUCTS is only used if HARDPATH
#		is false or absent.
#
#   MOP		Boolean flag (0 == FALSE, non-0 == TRUE) indicating whether
#		various macros are inserted in the output.
#
#   MOS		The value of this option is inserted directly in the output
#		located as target${ALT_OBJ}.o:dependancies.
#
#   MOD		MultiObject Dependancy file; where dependencies go.  If null,
#		then tty
#
#   MKFILE	The makefile - look for explicit rules and default rule, and
#		make sure the '#START_DEPEND' flag is there.
#
# OUTPUTS:
#
#   MOD		Possibly the file for the modified dependencies.
#   stdout	makefile and possibly the modified dependencies.
#
#   stderr	Informational and error messages.  If this script's output is
#		being piped, it may be useful to redirect stderr.
#
# AUTHOR:	Tom Nicinski, Creation date: 11-Aug-1993
# REVISED:	Ron Rechenmacher, 10-Oct-1994
#			Added multiple or cross platform support.
#--
#*******************************************************************************
#
# Parse out command line switches of the form FOO=bar.  These must be entered
# prior to any file names on the command line.
#
#   o	Generate scalar variables from them.
#

eval ('$'.$1.'$2;') while (($ARGV [0] =~ /^([A-Za-z_]+=)(.*)/) && shift);

#
# NOTE:	1-indexing is used throughout.
#

$[ = 1;						# Set array base to 1
$, = '';					# Set output field  separator
$\ = "\n";					# Set output record separator

#
# Fix up inputs.
#
#   o	Reduce white space to a single space.
#   o	Trim leading and trailing white space.
#

$gbl_whiteSpace = "[ \t\\r]+";


$NOSTD    =~ s/[ \t\r\n]+/ /g;			# Compress white space
$NOSTD    =~ s/(^[ \t]+|[ \t]+$)//g;		# Trim leading/trailing white
$PRODUCTS =~ s/[ \t\r\n]+/ /g;			# Compress white space
$PRODUCTS =~ s/(^[ \t]+|[ \t]+$)//g;		# Trim leading/trailing white

#
#   o	Split NOSTD and PRODUCTS into arrays.
#
#	   o	The NOSTD array will be converted to perl regular expressions.
#		Also, the path specification will be ended with a slash (/) if
#		it already isn't.  This is necessary to avoid the following
#		situation (shown by example):
#
#		   NOSTD='/usr/include'
#
#		and a .c file contains
#
#		   #include "/usr/includeJunk/stdio.h"
#
#		This dependency would be dropped out since it would appear to
#		come from /usr/include. Appending the ending slash (/) to the
#		NOSTD element prevents such matches.
#

@lcl_NOSTDs   = split ($gbl_whiteSpace, $NOSTD);
@lcl_PRODUCTS = split ($gbl_whiteSpace, $PRODUCTS);

foreach $lcl_NOSTD (@lcl_NOSTDs)
   {
   $lcl_NOSTD =~ s/([^\/]$)/\1\//;		# Cap end with slash '/'
   printf (STDERR "%-54s will be dropped\n", sprintf ("dependencies in %s", $lcl_NOSTD));
   # NOTE: if we don't do the following line, we can include re's in the
   #       NOSTD option!
   $lcl_NOSTD =  &make_REize ($lcl_NOSTD);
   }

#
# Locate all environment variables that are to be used to convert -I paths to
# the products _DIR name (plus anything beyond that).
#
#   o	Only environment variables with _DIR at the end of their name are
#	considered.
#

if (!$HARDPATH)
   {
   #
   # If _DIR paths are to be tested for belonging to a UPS product, perform the
   # directory listings once to save a considerable amount of time.
   #
   #   o   @lcl_PRODUCTSoff maintains the starting indices for ranges of product
   #       names for each declared products directory. This is necessary as perl
   #       does not support multidimensional arrays.
   #

   $#lcl_PRODUCTSoff = @lcl_PRODUCTS; @lcl_PRODUCTSoff = (); # Preallocate space
    @lcl_PRODUCTSoff [0 .. 1] = ($[, $[);

   for ($lcl_PRODUCTSidx = 1; $lcl_PRODUCTSidx <= $#lcl_PRODUCTS; $lcl_PRODUCTSidx++)
      {
      opendir  (PRODUCTSdir, $lcl_PRODUCTS [$lcl_PRODUCTSidx]);
      @lcl_PRODUCTSdir = (@lcl_PRODUCTSdir, grep (!/^\.\.?$/, readdir (PRODUCTSdir)));
      $lcl_PRODUCTSoff [$lcl_PRODUCTSidx + 1] = @lcl_PRODUCTSdir;
      closedir (PRODUCTSdir);
      }
      $lcl_PRODUCTSoff [$lcl_PRODUCTSidx + 1] = @lcl_PRODUCTSdir;

   #
   # Locate candidate UPS products.
   #

   @lcl_envPaths   = sort (grep (/^.*_DIR$/, keys (%ENV)));

   $#lcl_prodDirs  = @lcl_envPaths; @lcl_prodDirs  = (); # Preallocate space
   $#lcl_prodPaths = @lcl_envPaths; @lcl_prodPaths = (); # Preallocate space

    $lcl_prodCnt       = 0;			# No products from environment
    $lcl_prodDirs  [0] = '';			# Used to reduce conditionals
    $lcl_prodPaths [0] = '';			# Used to reduce conditionals

   if (@lcl_PRODUCTS == 0)
      {
      foreach $lcl_prodDir (@lcl_envPaths)
         {
                         $lcl_prodCnt++;
         $lcl_prodDirs  [$lcl_prodCnt] =       $lcl_prodDir;
         $lcl_prodPaths [$lcl_prodCnt] = $ENV {$lcl_prodDir};
         }
      }
   else
      {
      foreach $lcl_prodDir (@lcl_envPaths)
         {
         ($lcl_prodNAME = $lcl_prodDir) =~ s/_DIR$//;
          $lcl_prodNAME = &make_REize ($lcl_prodNAME);	# For the grep

         for ($lcl_PRODUCTSidx = 1; $lcl_PRODUCTSidx <= $#lcl_PRODUCTS; $lcl_PRODUCTSidx++)
            {
            @lcl_prodName = grep (/^($lcl_prodNAME)$/i, @lcl_PRODUCTSdir [$lcl_PRODUCTSoff [$lcl_PRODUCTSidx]
                                                                      .. ($lcl_PRODUCTSoff [$lcl_PRODUCTSidx + 1] - 1)]);
            if (@lcl_prodName != 0)
               {
               if (-f $lcl_PRODUCTS [$lcl_PRODUCTSidx] . '/' . $lcl_prodName [$[])
                  {
                                  $lcl_prodCnt++;
                  $lcl_prodDirs  [$lcl_prodCnt] =       $lcl_prodDir;
                  $lcl_prodPaths [$lcl_prodCnt] = $ENV {$lcl_prodDir};
                  last;
                  }
               }
            }
         }
      }

   #
   # Match -I paths with product directory "names" (_DIR format).
   #
   # For each -I in the CC -M command line, the beginning of the -I path is
   # matched against the selected product paths (prodPaths and prodDirs).
   # The longest _DIR path is used to replace the beginning of the -I path.
   # This is more than adequate as there are only two cases to consider
   # when an -I path matches more than one product path:
   #
   #   o   The product paths are unrelated. In this case, there is no means of
   #       knowing which path is the proper path to use (you would need to do
   #       to do the cc -M (or equivalent) prior to the garnering of product
   #       paths and substitute -I paths with product directory names (_DIR)
   #       based on the way each platform's compiler(s) scans the -I paths
   #       (e.g., IRIX cc will scan in the order of the -Is, except if a .h
   #       file includes another .h, it will search for the second .h in the
   #       directory the first .h was located, etc.)).
   #
   #   o   The product paths are related.  More than likely, they will have the
   #       same prefix path.  Thus, the longer path should be more specific when
   #       matched against the -I path.
   #
   # NOTE: When using a path in a regular expression but is not to be treated as
   #       an re, that re must be modified to quote any re metacharacters.  This
   #       is particular to the re evaluator being used (in this case, perl).
   #

    @lcl_Ipaths   = split ($gbl_whiteSpace, $CCLINE);
    $gbl_IpathCnt =  0;
   $#gbl_Ipaths   =  @lcl_Ipaths; @gbl_Ipaths = ();	# Preallocate space

   Ipath: foreach $lcl_Ipath (@lcl_Ipaths)
      {
      next Ipath if (($lcl_Ipath =~ s/^-I//) == 0);	# Ignore non -I switches

      #
      # Make sure the -I path is ended with a slash (see the notes under NOSTD
      # fixing).
      #

      $lcl_Ipath =~ s/([^\/])$/\1\//;

      #
      # Remove overlaps between @lcl_NOSTDs and the -I paths from the -I paths.
      #
      #   o   The dropping of standard include areas take precedence.
      #

      foreach $lcl_NOSTD (@lcl_NOSTDs)
         {
         next Ipath if ($lcl_Ipath =~ /$lcl_NOSTD/);
         }

      #
      # Locate the longest product path that matches against the -I path.
      #

      $lcl_prodMatchIdx = 0;			# No match yet
      for ($lcl_prodIdx = 1; $lcl_prodIdx <= $#lcl_prodPaths; $lcl_prodIdx++)
         {
         if (index ($lcl_Ipath, $lcl_prodPaths [$lcl_prodIdx]) != $[)
            {
            next;
            }
         if (($lcl_prodMatchIdx == 0) || 
             (length ($lcl_prodPaths [$lcl_prodIdx]) > 
              length ($lcl_prodPaths [$lcl_prodMatchIdx])))
            {
            $lcl_prodMatchIdx = $lcl_prodIdx;
            }
         }

      if ($lcl_prodMatchIdx != 0)
         {
         $lcl_IpathSubDir = substr($lcl_Ipath, length ($lcl_prodPaths [$lcl_prodMatchIdx]) + 1, length ($lcl_Ipath));

                         $gbl_IpathCnt++;
         $gbl_Ipaths    [$gbl_IpathCnt] =         $lcl_Ipath;
         $gbl_IpathLens [$gbl_IpathCnt] = length ($lcl_Ipath); # For speed later
         $gbl_IpathSubs [$gbl_IpathCnt] = "\$(" . $lcl_prodDirs [$lcl_prodMatchIdx] . ')' . $lcl_IpathSubDir;

         printf (STDERR "%-50s --> %s\n",    $gbl_Ipaths [$gbl_IpathCnt], $gbl_IpathSubs [$gbl_IpathCnt]);
         }
      else
         {
         printf (STDERR "%-54s unchanged\n", $lcl_Ipath);
         }
      }
   }	# if (!HARDPATH)

#*******************************************************************************
#
#*******************************************************************************
#
# If any paths match those which should be substituted with a more position-in-
# dependent form (xxx_DIR variable substitution), do so.
#
#   o	There is no distinction made between targets and sources.  As a matter
#	of fact, these are just simple substitutions.
#

if ($MKFILE)
{   $mkfile = "$MKFILE";
}
else
{   $mkfile = "Makefile";	# This is the Default
    print STDERR "using default makefile: Makefile";
    # Note: There will be problems if the stdout of the script is redirect 
    #       to the same file as the input.
}
open(MKFILE_FH, $mkfile) || die "Couldn't open $mkfile: $!\n";
$rules{'.o'} = "\t\$(CC) \$(CFLAGS) -c -o \$@ \$<";	# good default/initial
#print STDERR "#this is the rule for .o: $rules{'.o'}";
@explicit_rules = ();	#initialize the list to null
$add_start_depend = 1;
#print STDERR "#starting $mkfile";
while (<MKFILE_FH>)
{
    chop;	# NOTE: Tom set the output record separator for print ref p.113
    if (/^#START_DEPEND/)
    {   $add_start_depend = 0;   # it's already there.
#print STDERR "#found START_DEPEND";
	print;
	last;
    }
    elsif (/^#/)
    {	;	# do nothing for a comment line
    }
    elsif ( /^\.c([^\/ \t]*\.o)[ \t]*:[ \t]*$/ )  # any .c.o rule, i.e. .c.68.o
    {   $rule = $1;				 #        or .c.68o
	print;
	$_ = <MKFILE_FH>;	# need to explicitely set $_
	chop;
	$rules{$rule} = "$_";
	print;
	while ( ($_ = <MKFILE_FH>) =~ /^\t.*/ )
	{   chop;
	    $rules{$rule} .= "\n$_";
	    print;
	}
	chop;
#print STDERR "#this is the rule for $rule: $rules{$rule}";
    }
    elsif (/((([^ \t]+\.o)[ \t]*)+):/)  # a target:dependancy line -- assumes explicit rule follows
    {   $explicit    =  $1;		# save the target
	$explicit    =~ s/\$\([^\)]{1,}\)//g;   #remove macros
			    # For the target part, I assume the macro definititions
			    # will match what is generated by the mop/mos
			    # These lines should match lines below.
	@explicit_rules = (@explicit_rules,$explicit);	#initialize the list to null
#print STDERR "#this is explicit_rules: @explicit_rules";
    }
    elsif (/(([^ \t]+\.o)[ \t]*)+\\$/)
    {   print;				# go ahead and print the line now
	chop;				# chop the continuation character off
	$continued_explicit = "$_";	# now save the line
#print STDERR "#this is continued_explicit: $continued_explicit";
	while ( ($_ = <MKFILE_FH>) =~ /\\$/ )
	{   chop;			# chop the newline
	    print;			# print the line right away
	    chop;			# chop the continuation character
	    $continued_explicit .= "$_";# combine the lines
#print STDERR "#this is continued_explicit: $continued_explicit";
	}
	chop;
	# Now, only add the continued line if its a rule line
	if (/((([^ \t]+\.o)[ \t]*)+):/)
	{   $explicit    = ($continued_explicit . $1);
	    $explicit    =~ s/\$\([^\)]{1,}\)//g;   #remove macros
			    # For the target part, I assume the macro definititions
			    # will match what is generated by the mop/mos
			    # These lines should match lines below.
	    @explicit_rules = (@explicit_rules,$explicit);	#initialize the list to null
#print STDERR "#this is explicit_rules (from continue): @explicit_rules";
	}
    }
    print;
}
#print STDERR "#done with START_DEPEND while";
if ($add_start_depend)
{   print "#START_DEPEND";
}
if ( ($MOA) || ($MOD) )
{   # continue with the rest of the makefile -- we'll be appending the ALT_OBJ
    # dependancies OR continue to get the include directive
    while (<MKFILE_FH>)
    {	chop;
	print;
    }
}
close( MKFILE_FH );		# completely done with input
$mltpltfrm = 1;
$delay_rule = 0;
$print_rule = 0;
# There is the question of whether or not extra `dots' should be allowed to
# be introduced into the suffix.  I'm allowing them for now.
if ( $MOS =~ /^\.(.*[^.])$/ )	# special case where .x is equivalent to x.
{   $MOS = "$1.";
}
$ofh = STDOUT;		# default
if ($MOD)
{   $t = $MOD;
    $t =~ s/\$\([^\)]{1,}\)//g;	#remove macros
    rename( $t, "$t~" );
    open(DPFILE_FH, ">$t") || die "Couldn't open $t: $!\n";
    $ofh = DPFILE_FH;
    if ($MOA)
    {   open( OLDDPFILE_FH, "$t~" );
	while (<OLDDPFILE_FH>)
	{   chop;
	    print $ofh $_;
	}
	close( OLDDPFILE_FH );
    }
}

InpLine: while (<>)
   {
   chop;					# Strip record separator

   if (/^[ \t]*(#.*)*$/)			# Ignore blank/comment (#) lines
      {
      print $ofh $_;
      next InpLine;
      }

   #
   # Substitute hardwired paths with more position-independent forms.
   #
   # NOTE: There is possibility of improper substitutions.  Consider:
   #
   #          $gbl_Ipaths [0] = "/PRODUCTS/TOM/inc";      $gbl_IpathSubs [0] = "$(/PRODUCTS/LYNN_DIR)/inc"
   #          $gbl_Ipaths [1] = "/PRODUCTS/LYNN_DIR/lib"; $gbl_IpathSubs [1] = "$(LYNN_DIR)/lib"
   #
   #       and the input line is
   #
   #          $_ = "/PRODUCTS/TOM/inc/calvin.h : /PRODUCTS/TOM/inc/rachel.h"
   #
   #       Substituting      with $gbl_Ipaths [0] results in:
   #
   #          $_ = "$(/PRODUCTS/LYNN_DIR)/inc/calvin.h : $(/PRODUCTS/LYNN_DIR)/inc/rachel.h"
   #
   #       Now, substituting with $gbl_Ipaths [1] results in:
   #
   #          $_ = "$($(LYNN_DIR))/inc/calvin.h : $($(LYNN_DIR))/inc/rachel.h"
   #
   #       which is not accepted by make (as make does not handle recursive
   #       substitution of variables).
   #

   if (!$HARDPATH)
      {
      for ($lcl_IpathIdx = 1; $lcl_IpathIdx <= $gbl_IpathCnt; $lcl_IpathIdx++)
         {
         while (($lcl_repIdx = index ($_, $gbl_Ipaths [$lcl_IpathIdx]) - 1) >= 0)
            {
            $_ = substr($_, 1, $lcl_repIdx) . $gbl_IpathSubs [$lcl_IpathIdx]
               . substr($_,    $lcl_repIdx +  $gbl_IpathLens [$lcl_IpathIdx] + 1);
            }
         }
      }

   #
   # Drop out dependencies on standard header areas.
   #
   # NOTE: This code does not handle the following situation properly:
   #
   #          o   A dependency is split across multiple lines (with a continu-
   #              ation mark (\)).  All targets in all lines comprising the
   #              dependency are removed (they're all in standard header areas).
   #              The lines with continuation marks will   N O T   be deleted,
   #              since the continuation mark is still there.  That's not the
   #              expected behavior. In addition, the last line will be deleted;
   #              the previous line will now continue onto ???
   #
   # NOTE: For some reason, s///g does not seem to work below.  You need to do
   #       substitutions repeatedly.
   #

   foreach $lcl_NOSTD (@lcl_NOSTDs)
      {
      $lcl_subRE = "(^|:|[ \t]+)[ \t]*" . $lcl_NOSTD . "[^ \t]*([ \t]+|\\\\|\$)";
      while (s/$lcl_subRE/\1\3/g) {};	# \1\3 to prevent loss of : and \
      }

   #
   #   o   Drop the complete line if the dependency was reduced to worthlessness
   #

   if (   ($_ =~ /^[ \t]*[^ \t]*[ \t]*:[ \t]*$/)	# No source
       || ($_ =~ /^[ \t]*:.*/)				# No target
       || ($_ =~ /^[ \t]*:?[ \t]*\\[ \t]*$/)		# Empty continuation line
       || ($_ =~ /^[ \t]*$/)                            # Empty line
      )
      {
      next InpLine;
      }


    if ( ($MOP) || ($MOS) )
    {
	#
	# Add Ron's robust functionality.
	#
	# There shouldn't be any space at the beginning of a "target line"??

	# Assume .c dependancy check is first.

	# There is a question of whether or not I should check for explicit
	# rules before or after I add the prefix/suffix.
	# add in macro for *each-line-the-begins-with-and-object*.
	s/^([^ \t]{1,}\.)(o[ \t]*:.*)/$MOP\1$MOS\2/;  # add in {pre,suf}fix

	# The end of the following re is ".c followed by 0 or more of the
	# of the re 'a space or tab followed by anything'"
	if (/^([^ \t]{1,}\.o)[ \t]*:[ \t]*([^ \t]{1,})\.c([ \t].*)*$/)
	{					 # may need rule
	    $explicit = $1;	# setup for checking if rule already exists
	    $cfile = $2;	# the cfile - to be substituted in the rule
	    $3 =~ /(.)$/;
	    $lastchar = $1;	# setup to see if we'll output the rule now or later

	    #temporary diversion: lets see if last line ended with a slash which will
	    #                     screw things up (As NOTEd above) (could delay outputing lines to
	    #			  allow removal of continuation...???)
#	    if ($save_last_line =~ /[ \t]*/ && $second_to_last_line =~ /.*[ \t]([^ \t]{1,})[ \t]*\\$/) 
#	    if ($save_last_line =~ /.*[ \t]([^ \t]{1,})[ \t]*\\$/) 
	    if ($save_last_line =~ /([^ \t]?[ \t]+)?([^ \t]+)[ \t]*\\$/) 
	    {
		print $ofh "\t$2";		# This is a kludge to fix last dependency line
#print STDERR "--- kludged ---> save_last_line is >${save_last_line}<";
		if ($delay_rule)		#          continuing to this (the next) target line.
		{   print $ofh "$this_rule";	# Also print the rule if we were planning to.
		    $delay_rule = 0;
		}
	    }

	    $explicit    =~ s/\$\([^\)]{1,}\)//g;	#remove macros
#print STDERR "#check for explicit: $explicit";
	    if (grep(/$explicit/,@explicit_rules))	# rule hasn't already been defined
	    {   ;
#print STDERR "#rules for $explicit already exists";
	    }
	    else
	    {   # need to print some rule
		if ($lastchar  eq "\\")
		{
		    $delay_rule = 1;   # delay the printing of the rule til after continuation
		}
		else
		{
		    $print_rule = 1;
		}
		/(\.[^ \t]*\.o)[ \t]*/;	# setup to find which rule
		$rule = $1;
		$rule =~ s/\$\([^\)]{1,}\)//g;	#remove macros
		$this_rule = $rules{$rule};
		if ( ! ($this_rule) )
		{   $this_rule = $rules{'.o'};  #just use what we have for .c.o
#print STDERR "#couldn't find rule for $rule, using $rules{'.o'}";
		}
		$this_rule =~ s/\$\*/$cfile/g;   #rep.make macros that work for-
		$this_rule =~ s/\$</$cfile.c/g;  # inference rules, not explicit
	    }
	}
	elsif ( (/.*[^\\]$/) && ($delay_rule) )
	{
	    $delay_rule = 0;
	    $print_rule = 1;
	}
	print $ofh $_;
#	$second_to_last_line = $save_last_line;
	$save_last_line = $_;
	if ($print_rule)
	{
	    $print_rule = 0;
	    print $ofh "$this_rule";
	}
    }
    else
    {
	#
	# All done.
	#

	print $ofh $_;

    }
   }

if ($MOD)
{   close( DPFILE_FH );
}

#*******************************************************************************
#
#*******************************************************************************

sub make_REize
   {
   local ($string) = @_;

#++
# FUNCTIONAL DESCRIPTION:
#
#	Convert the input string to an awk (egrep) regular expression (quoting
#	any regular expression metacharacters).
#
# CALLING SEQUENCE:	re = make_REize(string)
#
#   string		The input string to convert to a regular expression.
#
# RETURN VALUES:
#
#   re			The resulting regular expression.
#
# SIDE EFFECTS:		None
#
# SYSTEM CONCERNS:	None
#--
#*******************************************************************************

   $string =~ s/\\/\\\\/g;		# Ordering of these substitions is
   $string =~ s/\[/\\[/g;		# ... important
   $string =~ s/\]/\\]/g;

   $string =~ s/[.]/\\./g;
   $string =~ s/[*]/\\*/g;
   $string =~ s/\^/\\^/g;
   $string =~ s/\$/\\\\\$/g;
   $string =~ s/[(]/[(]/g;
   $string =~ s/[)]/[)]/g;
   $string =~ s/[{]/[{]/g;
   $string =~ s/[}]/[}]/g;
   $string =~ s/[+]/\\+/g;
   $string =~ s/[?]/\\?/g;
   $string =~ s/[|]/[|]/g;

   ($string);
   }

#*******************************************************************************
