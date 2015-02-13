use strict;
use File::Basename;
use utf8;
no utf8;

my %codetable = ();

open F,$ARGV[0];
while(<F>) {
    next if(m/^#/);
    my ($foreign,$unicode,@r) = split /\s/;
    $codetable{hex $foreign} = hex $unicode;
}
close F;

sub foreign2utf8 {
    my $code = shift;
    $code = $codetable{int $code};
    if($code eq 0) {
        print "\n";
        exit 0;
    }
    my $s = pack "U", $code;
    utf8::encode $s;
    return $s;
}

my $a = <STDIN>;
chomp $a;
$a = basename $a;
$a =~ s/\#\((\d+)\)/foreign2utf8($1)/ge;
print "$a\n";
