#------------------------------------------------------------------------------
# The confidential and proprietary information contained in this file may
# only be used by a person authorised under and to the extent permitted
# by a subsisting licensing agreement from ARM Limited.
#
#            (C) COPYRIGHT 2010-2015  ARM Limited or its affiliates.
#                ALL RIGHTS RESERVED
#
# This entire notice must be reproduced on all copies of this file
# and copies of this file may only be made by a person if such person is
# permitted to do so under the terms of a subsisting license agreement
# from ARM Limited.
#
#  Version and Release Control Information:
#
#  File Revision       : $Revision: 275084 $
#  File Date           : $Date: 2014-03-27 15:09:11 +0000 (Thu, 27 Mar 2014) $
#
#  Release Information : Cortex-M0 DesignStart-r1p0-00rel0
#------------------------------------------------------------------------------
# Purpose : Synthesis Script - Reports
#
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Write final reports
# ------------------------------------------------------------------------------

printvar > ../reports/synthesis/${rm_project_top}.vars

check_design -multiple_designs > \
  ../reports/synthesis/${rm_project_top}.check_design

check_timing > \
  ../reports/synthesis/${rm_project_top}.check_timing

report_qor > \
  ../reports/synthesis/${rm_project_top}.qor

report_timing -delay max \
              -max_paths 50 \
              -nosplit \
              -cap \
              -path full_clock_expanded \
              -nets \
              -transition_time \
              -input_pins > \
  ../reports/synthesis/${rm_project_top}.timing-max

# Create compacted version of the timing report showing only nets
set fr [open ../reports/synthesis/${rm_project_top}.timing-max r]
set fw [open ../reports/synthesis/${rm_project_top}.timing-max-nets w]

while {[gets $fr line] >= 0} {
    if {[regexp {delay} $line] ||
        [regexp { data } $line] ||
        [regexp {slack} $line] ||
        [regexp {\-\-\-\-} $line] ||
        [regexp {Group} $line] ||
        [regexp {Startpoint} $line] ||
        [regexp {Endpoint} $line] ||
        [regexp {Point} $line] ||
        [regexp { clock } $line] ||
        [regexp {(net)} $line] ||
        [regexp {^ *$} $line]
    } {
        if {![regexp {/n[0-9]+ } $line]} {
            puts $fw $line
        }
    }
}

close $fr
close $fw

report_timing -loops > \
  ../reports/synthesis/${rm_project_top}.loops

report_area -nosplit \
            -hierarchy \
            -physical > \
  ../reports/synthesis/${rm_project_top}.area

report_power -nosplit > \
  ../reports/synthesis/${rm_project_top}.power

report_constraint -all_violators \
                  -nosplit > \
  ../reports/synthesis/${rm_project_top}.constraint_violators

report_design > \
  ../reports/synthesis/${rm_project_top}.design_attributes

report_clocks -attributes \
              -skew > \
  ../reports/synthesis/${rm_project_top}.clocks

report_clock_gating -multi_stage \
                    -verbose \
                    -gated \
                    -ungated \
  > ../reports/synthesis/${rm_project_top}.clock_gating

report_clock_tree -summary \
                  -settings \
                  -structure > \
  ../reports/synthesis/${rm_project_top}.clock_tree

query_objects -truncate 0 [all_registers -level_sensitive ] \
  > ../reports/synthesis/${rm_project_top}.latches

report_isolate_ports -nosplit > \
  ../reports/synthesis/${rm_project_top}.isolate_ports

report_net_fanout -threshold 32 -nosplit > \
  ../reports/synthesis/${rm_project_top}.high_fanout_nets

report_port -verbose \
            -nosplit > \
  ../reports/synthesis/${rm_project_top}.port

report_hierarchy > \
  ../reports/synthesis/${rm_project_top}.hierarchy

report_resources -hierarchy > \
  ../reports/synthesis/${rm_project_top}.resources

report_compile_options > \
  ../reports/synthesis/${rm_project_top}.compile_options

report_congestion > \
  ../reports/synthesis/${rm_project_top}.congestion

# Zero interconnect delay mode to investigate potential design/floorplan problems
set_zero_interconnect_delay_mode true
report_timing -delay max \
              -max_paths 50 \
              -nosplit \
              -path full_clock_expanded \
              -nets \
              -transition_time \
              -input_pins > \
  ../reports/synthesis/${rm_project_top}_zero-interconnect.timing

report_qor > \
  ../reports/synthesis/${rm_project_top}_zero-interconnect.qor
set_zero_interconnect_delay_mode false


# ------------------------------------------------------------------------------
# End of File
# ------------------------------------------------------------------------------
