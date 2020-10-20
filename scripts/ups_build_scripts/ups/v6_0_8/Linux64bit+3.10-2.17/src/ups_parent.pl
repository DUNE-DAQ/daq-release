#!/usr/bin/env perl

$^W=1;

# debugging -- off for production, should do something with -v[vv...]
$debug = 0;
$| = 1;

@printlist = parseargs();
print STDERR 1, "\% completed.  \r";

#
# we will have a tree for each product...
#
%trees = ();

make_hflavorlist();
print STDERR int(30*100/$entries), "\% completed.  \r";
#
# this builds a parent tree for *everything* now
#
find_parents();

#
# Now we dump each tree...
#
if ($debug) {
    for $prod (keys(%trees)) {
	print "product $prod\n";
	dump_tree($trees{$prod});
    }
}

# for $prod (sort(keys(%trees))) {
for $prod (@printlist) {
    print "\n";
    %visited = ();
    recurse_tree($trees{$prod}, $prod, '', '');
}

#--------------------------------------------------

#
# list of all toplevel ups flavor -3 values we might see.
# if we get a -2-ish flavor, (i.e. IRIX+6) we pick a good guess
# on the end.  If it's a -1-ish flavor we don't care, 'cause
# "-H IRIX" is sufficient if there are no IRIX+x flavors...
#

%plus_three_guess = (
 	"IRIX+5" => "IRIX+5.4",
 	"IRIX+6" => "IRIX+6.5",
	"SunOS+5" => "SunOS+5.6",
        "Linux+2" => "Linux+2.2",
	"OSF1+V4" => "OSF1+V4.0",
);

sub make_hflavorlist {

    $ourflavor = `ups flavor -3`;
    chomp($ourflavor);
    %hflavorlist = ();
    $entries = 30;
    open(FLAVORS,"ups list -aK flavor 2>/dev/null |");
    while (<FLAVORS>) {
       $entries++;		# count how many
       s/\A\s*"//o;		#  trim quotes
       s/"\s*\Z//o;
       next unless m/\+/o;	# don't care...

       if ( $plus_three_guess{$_} ) {
	   $_ = $plus_three_guess{$_};
       } else {
           s/\Z/.x/ unless m/\./o;
       }
       $hflavorlist{$_} = 1;
    }
    close(FLAVORS);
    if (! $hflavorlist{$ourflavor}) {
	$hflavorlist{$ourflavor} = 1;
    }
}

#
# pick on a few arguments, and run ups list...
#
sub parseargs {
    my ($a, $root, @stuff, @rest, @res);

    @res = ();
    $a = join(' ', @ARGV);
    $dashz=0;
    $savek = "";

    #
    # if they gave us a -z, set $PRODUCTS, and whine if they used -K
    #
    for (@ARGV) {
        if ($dashz) {
 	   $::ENV{'PRODUCTS'} = $_;
	   $dashz = 0
        }
        if ($dashk) {
	   $savek = $_;
           $_ = "";
        }
	if (/^-.*z/) {
	   $dashz = 1
        }
	if (/^-(.*)K(.*)/) {
	    $savek = $2;
            $others = $1;
            if ($savek eq "") {
               $dashk = 1;
            }
            if ($others ne "") {
	       $_ = "-$others";
            }
        }
    }

    # don't redirect stderr, this is how they find out about 
    # command line errors, etc.

    $cmd = "ups list -K+:database $a |";
    print "cmd is $cmd\n" if $debug;
    open(LIST, $cmd);

    while (<LIST>) {
	print "got $_" if $debug;
	@stuff = m/"(.*?)"/g;
	push(@res, makenode(@stuff));
    }

    # close will be false if the ups list had errors...
    if (close(LIST)) {
        return @res;
    } else {
	return ();
    }
}

#
# Walk the tree and pretty-print it.
# if we've been to this node before, print it in parenthesis, and don't
# do it's kids again...
#
sub recurse_tree {
    my ($tree, $node, $prefix, $how) = @_;
    my ($n, @kids, $pre, $k, $i);

    ($pre = $prefix) =~ s/.  $/|__/;
     
    if ( !defined($visited{$node}) ) {
	@kids = sort(keys(%{$tree->{$node}}));
        if ( $savek ne "" ) {
            system ("ups list -K$savek $node");
        } else {
	    print $pre, $node, $how, "\n";
        }
	$visited{$node} = 1;
	$k = $#kids;
	$i = 0;
	foreach $n (@kids) {
	    # visit it..
            $how = $tree->{$node}->{$n};
	    if ( $i == $k ) {
		recurse_tree( $tree, $n, $prefix . "   ", $how );
	    } else {
		recurse_tree( $tree, $n, $prefix . "|  ", $how );
	    }
	    $i++;
	}
    } else {
        if ( $savek eq "" ) {
	    print $pre, "(", $node, $how, ")\n";
        }
    }
}

#
# Debug dump of the whole tree graph data structure
#
sub dump_tree {
    my ($tree) = $@;

    print "Tree:\n";
    foreach $k (keys(%{$tree})) {
        @kids = keys(%{$tree->{$k}});
	foreach $n (@kids) {
	    print "$k -> $n\n";
	}
    }
    print "End of Tree:\n";
}

#
# go through the whole database, and look at each product with
# dodeps
#
sub find_parents {
    my ($count, $cmd);

    $cmd = "ups list -a -K+:database 2>/dev/null |";
    print "cmd is $cmd\n" if $debug;
    open(LIST, $cmd);

    $count = 30;
    while( <LIST> ) {
       print "got $_" if $debug;
       @words = m/"(.*?)"/g;
       dodeps(@words);
       $count++;
       print STDERR int($count*100/$entries), "\% completed.  \r";
    }
    close(LIST);
}

#
# look at the full dependency tree for the product
# (turning -f into -H to get a sane dependency list)
# if its full dependencies involve the product in question,
# dump the direct dependencies into the tree (backwards)
#
%didthat = ();

sub dodeps {
   my (@words, $parent, $root);
   my ($cmd,$hflavorpat, $hflavor,$sp, $direct);
   my (@direct, @addto);

   $parent = makenode(@_);

   #
   # here we guess one (or more) longer flavors to go with
   # any -f option we have to get around problems like where we have
   # both dependency graphs below possible (assuming we are 
   # parent-ing bleem):
   # -------------
   # foo -f IRIX
   # |__bar -f IRIX+6
   #    |__baz -f NULL
   #       |__bleem -f IRIX
   # -------------
   # foo -f IRIX
   # |__bar -f IRIX+5
   #    |__baz -f NULL
   #       |__bleem -f IRIX
   # -------------
   # you actually need to do two (or more) different ups depends of 
   # foo to find out about the two instances of bar...

   # turn the flavor into a regexp
   $hflavorpat = @_[2];
   $hflavorpat =~ s/\W/\\$&/go;
   $hflavorpat =~ s/NULL/.*/o;

   print "doing dependencies for $parent\n" if $debug;
   foreach $hflavor (keys(%hflavorlist)) {
       if ( $hflavor =~ m/$hflavorpat/ ) {

	   $sp = $parent;
	   $sp =~ s/ -z .*//o;
	   $sp =~ s/ -f \S+/ -H $hflavor/;

	   print "doing dependencies for $sp\n" if $debug;
	   $cmd = "ups depend $sp 2>/dev/null |";

	   # weed out duplicates!
           # this rewriting to -H stuff could give duplicates, and
           # it's already pretty darn slow...

           next if $didthat{$cmd};
           $didthat{$cmd} = 1;

	   print "cmd is $cmd\n" if $debug;
	   open(DEPEND, $cmd);

	   $useit = 0;
           @direct = ();
           @addto = ();
	   while(<DEPEND>) {
	       print "got $_" if $debug;
               chomp();
               #
	       # trailing blanks and -j flags really confuse things..
	       #
               s/\s*\Z//o;
	       s/ -j//o;

 	       if ( m/\A\|__/o ) {
                  $direct = 1;
               } else {
	          $direct = 0;
               }

	       s/\A[|_ ]*//o;
	       push(@addto, $_);

	       s/ -g .*//o;

               if ($direct) {
		  print "first level...\n" if $debug;
	          push(@direct, $_);
               }
	   }
	   close(DEPEND);
	 
	   print "from depend on $parent:\n" if $debug;

           for $root (@addto) {
	        if ($root =~ s/ -g .*//o ) {
		    $how = " [via $&]";
                } else {
		    $how = " ";
                }
	        print "entries for $root:\n" if $debug;
		for $child (@direct) {
	            addedge($root, $parent, $child, $how);
                }
           }
       }
   }
}

sub addedge {
    my ($root, $parent, $child, $how) = @_;
    if (! defined($trees{$root}) ) {
       $trees{$root} = {};
    }
    $tree = $trees{$root};
    return if ($parent eq $child);
    print "adding $child -> $parent\n" if $debug;
    if (!defined $tree->{$child}) {
       $tree->{$child} = {};
    }
    $tree->{$child}->{$parent} = $how;
}

sub makenode {
    my  $res;

    $res = @_[0] . ' ' . @_[1] . ' -f ' . @_[2];
    if (@_[3]) {
 	$res .=  ' -q ' . @_[3] 
    }
    if (@_[5]) {
        $res .= ' -z ' . @_[5];
    }
    return $res;
}
