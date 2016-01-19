#!/bin/sh
#!perl -w
# call perl from PATH
eval 'exec perl -x -S $0 ${1+"$@"}'
  if 0;

#################################################################################
#
# The confidential and proprietary information contained in this file may
# only be used by a person authorised under and to the extent permitted
# by a subsisting licensing agreement from ARM Limited.
#
#                 (C) COPYRIGHT 2011-2014 ARM Limited or its affiliates.
#                       ALL RIGHTS RESERVED
#
# This entire notice must be reproduced on all copies of this file
# and copies of this file may only be made by a person if such person is
# permitted to do so under the terms of a subsisting license agreement
# from ARM Limited.
#
#              M0 Design Start Check tests Script
#              ==================================
#
#      SVN Information
#
#      Checked In          : $Date: 2012-08-31 12:34:14 +0100 (Fri, 31 Aug 2012) $
#
#      Revision            : $Revision: 220755 $
#
#      Release Information : Cortex-M7 AT590-r0p0-00rel0
#
################################################################################
# Script Usage: check_tests.pl
################################################################################

use strict;
use warnings;
use Getopt::Long;

my $dir = 'logs';
my $testlog = "";
my $testname = "";
my %teststatus;
my $test_pass_fail_skip = "";
my $test_count = 0;
my $pass_count = 0;
my $fail_count = 0;
my $kill_count = 0;
my $skip_count = 0;
my $time = 0;
my $exec_time = 0;

# Prototype subroutines
sub count_logs();

$dir = "logs";
if ( -d $dir ) {
   count_logs();
}

sub count_logs()
{
   opendir(DIR, $dir) or die $!;
   while (my $file = readdir(DIR)) {
      # Use a regular expression to ignore files beginning with a period
      next if ($file =~ m/^\./);
      if (($file =~ /.log/) & ($file !~ /compile.log/) & ($file !~ /tarmac/)) {
         $testlog = $dir . "/" . $file;
         open LOGFILE, $testlog or die "Error opening log file";
         $test_count++;

         $testname = $file;
         $testname =~ s/.log//;
         $testname =~ s/run_//;
         $time = 0;
         $test_pass_fail_skip = "KILL";

         while (my $log_row = <LOGFILE>) {
            if ($log_row =~ /#    Time:/) {
               # Record execution time
               ($time) = $log_row =~ /(\d+)/; 
            }

            if ($log_row =~ /UART: \*\* TEST FAILED \*\*/) {
               $test_pass_fail_skip = "FAIL";
               $fail_count++;
            }
            if ($log_row =~ /UART: \*\* TEST PASSED \*\*/) {
               $test_pass_fail_skip = "PASS";
               $pass_count++;
            }
            if ($log_row =~ /UART: \*\* TEST SKIPPED \*\*/) {
               $test_pass_fail_skip = "SKIP";
               $skip_count++;
            }

         } #while loop through log file lines

         $exec_time = $exec_time + $time;
         print "$test_pass_fail_skip  $testname\n";

         close LOGFILE;
      }
   } #while loop through log files
   $kill_count = $test_count - $pass_count - $fail_count - $skip_count;
   print "\n";
   print "Count:  $test_count\n";
   print "Passes: $pass_count, Fails: $fail_count, Kills: $kill_count, Skips: $skip_count\n";
   print "Time: $exec_time\n";
   closedir(DIR);
} #end sub count_logs
  
