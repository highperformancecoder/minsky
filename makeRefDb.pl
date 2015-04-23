#!/usr/bin/perl

# input file is the generated label.pl file in generated minsky docs
$URL="";
do $ARGV[0];

print "array set externalLabel {";
for $key (keys %external_labels)
{
    print "{$key} {$ external_labels{$key}} ";
}
print "}";

