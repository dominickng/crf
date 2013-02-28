#!/usr/bin/perl
#usage: ./pos_compare.perl gold_pos hypo_pos
#  both gold and hypo in Ratnaparkhi's format

if ($#ARGV != 1) {
    print "Usage: pos_compare.perl gold_pos hypo_pos\n";
    print "both gold and hypo in Ratnaparkhi's format\n";
    exit;
}

open(GOLD, $ARGV[0]);
open(HYPO, $ARGV[1]);

$postotal = 0;
$posmatch = 0;

while($gline = <GOLD>){
    chomp $gline;
	@garr = split(/\s+/, $gline);
    $hline = <HYPO>;
    chomp $hline;
    @harr = split(/\s+/, $hline);
    
    for ($i=0; $i<=$#garr; $i++){
        $postotal ++;
        ($word, $gpos) = split(/_/, $garr[$i]);
        ($word, $hpos) = split(/_/, $harr[$i]);
        if ($gpos eq $hpos){
            $posmatch ++;
        }
    }    
}

print "Total: $postotal \n";
print "Match: $posmatch \n";
$accuracy = 1.0 * $posmatch / $postotal;
print "Accuracy: $accuracy \n";


