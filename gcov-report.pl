#!/usr/bin/perl
# runs gcov, and produces a coverage report
foreach $_ (qx/find . -name "*.cc" -exec gcov -o . -r {} \\;/)
{
    if (/^File/)
    {
        # ignore header files
        if (/.cd'$/ || /.h'$/)
        {
            $ignore=1;
            next;
        }
        else
        {
            $ignore=0;
        }
        ($name)=/File '(.*)'/;
    }
    if (!$ignore && /^Lines/)
    {
        ($time,$lines)=/Lines executed:(.*)% of (.*)/;
        $times{$name}=$time;
        if ($name=~/\//)
        {
            ($dir)=$name=~/(.*)\//;
            $dir_exec{$dir}+=0.01*$time*$lines;
            $dir_total{$dir}+=$lines;
        }
        $exec+=0.01*$time*$lines;
        $total+=$lines;
    }

}

print "Total exec $exec of $total = ",100*$exec/$total,"%\n";

foreach $dir (keys %dir_exec)
{
    print "$dir: $dir_exec{$dir} of $dir_total{$dir} = ",100*$dir_exec{$dir}/$dir_total{$dir},"%\n";
}

foreach $name (sort(keys %times))
{
    print "$name $times{$name}%\n";
}

