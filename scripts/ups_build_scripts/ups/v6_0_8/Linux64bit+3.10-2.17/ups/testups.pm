#!/usr/bin/env perl

use lib "/usr/local/etc";
use ups;

ups::use_perl("v5_006_1a");

print @INC;

ups::setup("upd");
print "UPD_DIR is ", $::ENV{UPD_DIR}, "\n";

print ups::ups("list","upd");
