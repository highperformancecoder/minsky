#!/usr/bin/perl

## input file is the generated label.pl file in generated minsky docs
#$URL="";
#do $ARGV[0];
#
for $file (@ARGV)
{
    open INPUT,$file;
    while ($_=<INPUT>)
    {
        @labels=/<A NAME=\"([^"]*)\"/g;
        ($node)=$file=~/.*\/([^\/]*)/;
        for $label (@labels)
        {
            $external_labels{$label}="/".$node;
        }
    }
}

print "array set externalLabel {";
for $key (keys %external_labels)
{
    print "{$key} {$ external_labels{$key}} ";
}
print "}";

