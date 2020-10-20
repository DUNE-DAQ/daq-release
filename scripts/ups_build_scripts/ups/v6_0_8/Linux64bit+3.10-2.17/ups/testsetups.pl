#!/usr/bin/env perl

require "/usr/local/etc/setups.pl";

ups::use_perl("v5_006_1a");

print @INC;

ups::setup("upd");
print "UPD_DIR is ", $::ENV{UPD_DIR}, "\n";
