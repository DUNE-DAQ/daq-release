#!/usr/bin/env perl
#
# build_help_file:
#
# Usage: build_help_file [-o <output file>] [-c <c source file>] [-t <help file>]
#
# 28-Apr-1998, lr
#
##########################################################################
use strict;

my $prog = 'build_help_file';
my $usage = "Usage: build_help_file [-o <output file>] [-c <c source file>] [-t <help file>]\n";

#
# set default source files
#

my $ft = '';
my $fsc = "$ENV{UPS_DIR}/src/upsact.c";
my $fsh = "$ENV{UPS_DIR}/doc/ups_help.in";

#
# parse arguments
#

while ( @ARGV ) {

  $_ = shift @ARGV;
  if ( /^-h/ || /^-\?/ ) { print "$usage"; exit 1; }
  elsif ( /^-o/ ) { $ft = shift; }
  elsif ( /^-t/ ) { $fsh = shift; }
  elsif ( /^-c/ ) { $fsc = shift; }
}

#
# check if files are there
#

die "$prog: can't open file '$fsc'\n" unless ( -f $fsc && open( FSC, $fsc ) );
die "$prog: can't open file '$fsh'\n" unless ( -f $fsh && open( FSH, $fsh ) );

# id needed, redirect stdout

die "$prog: can't open file '$ft'\n" unless ( ! $ft || open STDOUT, ">$ft" );

#
# read in ups command options from c file
#

# move to the g_cmd_info array

while ( <FSC> ) {

  last if ( /\s*t_cmd_info\s*g_cmd_info/ );
}

# read g_cmd_info items 

my %copt = ();

while ( <FSC> ) {

  last if ( /e_unk/ );
  next if ( !/\A\s*\{/ );
  chomp;
  
  my ($cmd, $opt) = get_cop();

  print STDERR "$prog: having problems parsing '$fsc'\n" 
      unless ( $cmd ne '' && $opt ne '' );

  $cmd =~ s/\A\s*"(\w+)"/$1/o;
  $opt =~ s/\A\s*"(.*?)"/$1/o;

  if ( $cmd && $opt ) {

    $opt =~ s/://go;
    $copt{$cmd} = $opt;
  }
}

#
# read ups commands from help file, build a new file and print results
#

my $oncmd = 0;

while ( <FSH> ) {

  $oncmd = 0 if ( /^OPTIONS/ );

  if ( $oncmd && $_ && ! /^:/ && ! /^\#/ ) {

    my @arr = split /:/, $_;

    if ( $#arr >= 0 ) {

      my $cmd = $arr[0];
      $cmd =~ s/\A\s*(.*?)\s*\Z/$1/o;

      if ( exists $copt{$cmd} ) {

	$_ = sprintf "%-12s:%s:\n", $cmd, $copt{$cmd};
	delete $copt{$cmd};
      }
      else {

	print STDERR "$prog: unrecognized ups command '$cmd' found in $fsh\n";
      }
    }
      
  }

  $oncmd = 1 if ( /^COMMANDS/ );

  print STDOUT $_;
}


# 
# check if all ups commands was found in help file
#
my $key;
foreach $key (keys %copt) {
  print STDERR "$prog: no help for 'ups $key' was found in $fsh\n";
}

##########################################################################
#
# subs
#
sub get_cop
{
  my @items = split /,/, $_;

  while ( $#items < 8 && !/\}\s*/ ) {

    $_ = <FSC>;
    push @items, split /,/, $_;
  }

  my $cmd = '';
  my $opt = '';
  my $n = 0;
  my $item;

  foreach $item (@items) {

    $item =~ s/\A\s*(.*?)\s*/$1/o;
    $n++ if ( $item ne "" && $item ne '{' && $item ne '}' );

    $cmd = $item if ( $n == 2 && ! $cmd);
    $opt = $item if ( $n == 3 && ! $opt);

    last if ( $cmd ne '' && $opt ne '' );
  }

  return ($cmd, $opt);
}

1;
