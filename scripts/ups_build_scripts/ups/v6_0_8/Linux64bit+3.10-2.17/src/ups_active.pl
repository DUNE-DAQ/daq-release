#!/usr/bin/env perl

# this little script just lists the ups products which are currently setup.

printf( "Active ups products:\n");
open(QL, "printenv | grep '^SETUP_' | grep -v SETUPS_DIR | sort |") || die "Failed: $!\n";
while( $line = <QL> ) {
    chop($line);
    @words=split(/=/,$line);
    #print "$words[1]\n";
    @pl=split(/\s+/,$words[1]);
    printf("%-16s  %-15s -f %-20s", $pl[0],$pl[1],$pl[3]);
    if($#pl > 5 ) {
        printf(" -q %-15s -z %s",$pl[7],$pl[5]);
    } else {
        printf("%s -z %s",' ' x 19,$pl[5]);
    }
    printf("\n");
}
close(QL);

exit;
