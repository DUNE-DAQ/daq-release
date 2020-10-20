package ups;

BEGIN {
    my ($setups_sh, $c1);

    $setups_sh = "${Setups_Home}/setups.sh";
    $::ENV{UPS_SHELL} = "sh";
    $c1=` . $setups_sh; 
          echo "\$::ENV{UPS_DIR}=q(\$UPS_DIR); \$::ENV{PRODUCTS}=q(\$PRODUCTS); \$::ENV{SETUP_UPS}=q(\$SETUP_UPS);\$::ENV{PATH}=q(\$PATH);"`;

    unless (eval $c1) {
	die("setups.pl failed $@");
    };
}


sub setup {
 
    my ($file, $c1);

    $file = `$::ENV{UPS_DIR}/bin/ups setup $_[0]`;


    chomp($file);

    open( SETUP, ">>$file" );

    print SETUP <<'EOF';

echo "--------------cut here-------------"
perl -e 'for $v (keys(%::ENV)) { 
	    ($fix= $::ENV{$v}) =~ s/[()]/\$&/go;
	    print "\$::ENV{$v} = q($fix);\n" 
	 }'

EOF
    close(SETUP);

    $c1 = `/bin/sh $file`;
    $c1 =~ s/.*--------------cut here-------------//s;

    %::ENV = ();
    unless (eval $c1) {
	die("setup failed $@");
    };
}
 
sub unsetup {
 
    my ($file, $c1);

    $file = `$::ENV{UPS_DIR}/bin/ups unsetup $_[0]`;


    chomp($file);

    open( SETUP, ">>$file" );

    print SETUP <<'EOF';

echo "--------------cut here-------------"
perl -e 'for $v (keys(%::ENV)) { 
	    ($fix= $::ENV{$v}) =~ s/[()]/\$&/go;
	    print "\$::ENV{$v} = q($fix);\n" 
	 }'

EOF

    close(SETUP);

    $c1 = `/bin/sh $file`;
    $c1 =~ s/.*--------------cut here-------------//s;

    %::ENV = ();
    unless (eval $c1) {
	die("unsetup failed $@");
    };
}

sub use_perl {
    my ($v, $rest) = @_;
    use_prod( "perl", $v, "SETUP_PERL");
}

sub use_prod {
    my ($p, $v, $var) = @_;
    if ( $v eq "" ) {
	$vm = '.*';
    } else {
	$vm = $v;
    }
    if ( ! defined($::ENV{$var}) || $::ENV{$var} !~ m/$p $vm / ) {
	setup("$p $v");
	exec "$::ENV{PERL_DIR}/bin/perl", "-S", $::0, @::ARGV;
	die( "cannot exec $::ENV{PERL_DIR}/bin/perl" );
    }
}
 
1;
