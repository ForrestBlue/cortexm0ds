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
# Purpose : Synthesis Script - Design Compiler DFT insertion script
#
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Job Diagnostics
# ------------------------------------------------------------------------------

# Log the time that this script starts executing
set start_time [clock seconds] ; echo [clock format ${start_time} -gmt false]


echo [pwd]

print_suppressed_messages

# ------------------------------------------------------------------------------
# Set-up Design Configuration Options
# ------------------------------------------------------------------------------

source -echo -verbose ../scripts/design_config.tcl

# ------------------------------------------------------------------------------
# Set-up Target Technology
# ------------------------------------------------------------------------------

source -echo -verbose ../scripts/${rm_project_top}_tech.tcl

# -----------------------------------------------------------------------------
# Set target libraries
# -----------------------------------------------------------------------------

set_app_var search_path [concat . $stdcell_search_path $search_path]

set_app_var synthetic_library dw_foundation.sldb
set_app_var target_library [concat $stdcell_library(db,$slow_corner_pvt)]
set_app_var link_library [concat * $target_library $synthetic_library]

set_app_var mw_reference_library [concat $stdcell_mw_library]

# -----------------------------------------------------------------------------
# Associate libraries with min libraries
# -----------------------------------------------------------------------------

foreach max_lib [concat $stdcell_library(db,$slow_corner_pvt)] \
        min_lib [concat $stdcell_library(db,$fast_corner_pvt)] \
    {
        echo "set_min_library $max_lib -min_version $min_lib"
        set_min_library $max_lib -min_version $min_lib
    }

# ------------------------------------------------------------------------------
# Open MW design library
# ------------------------------------------------------------------------------

set_app_var mw_design_library $rm_project_top

open_mw_lib $mw_design_library

check_library

set_tlu_plus_files -max_tluplus $tluplus_file($slow_corner_extraction) \
                   -min_tluplus $tluplus_file($fast_corner_extraction) \
                   -tech2itf_map $tf2itf_map_file

check_tlu_plus_files

# ------------------------------------------------------------------------------
# Library cell optimizations
# ------------------------------------------------------------------------------

# Tie cell optimizations
foreach libraryname [array names rm_tie_cells] {
  foreach rm_tiecelltype $rm_tie_cells($libraryname) {
    set rm_tie_cell [get_lib_cells  ${libraryname}/${rm_tiecelltype}]
    remove_attribute $rm_tie_cell dont_use
    remove_attribute $rm_tie_cell dont_touch
    foreach_in_collection rm_tiecellpin [get_lib_pins -of_objects $rm_tie_cell \
                                             -filter "pin_direction == out"] {
      set_attribute $rm_tiecellpin max_fanout 1.0 -type float
    }
  }
}

# Set any dont use lists
foreach libraryname [array names rm_dont_use] {
  foreach rm_dontusecelltype $rm_dont_use($libraryname) {
    set_dont_use -power [get_lib_cells ${libraryname}/${rm_dontusecelltype}]
    unset rm_dontusecelltype
  }
  unset libraryname
}

# ------------------------------------------------------------------------------
# Setup for Formality verification
# ------------------------------------------------------------------------------

set_svf ../data/${rm_project_top}.dft.svf

# -----------------------------------------------------------------------------
# Re-apply synthesis tool options
# -----------------------------------------------------------------------------

# These settings may not be stored in the DDC, but impact timing analysis

set_app_var enable_recovery_removal_arcs true

# Case analysis required to support EMA value setting for memories
set_app_var case_analysis_with_logic_constants true

# Allow identification of inserted logic
set_app_var compile_instance_name_prefix DFT_

#set_app_var timing_use_enhanced_capacitance_modeling true

set_app_var physopt_enable_via_res_support true

set_app_var write_name_nets_same_as_ports true
set_app_var report_default_significant_digits 3

# -----------------------------------------------------------------------------
# Read pre-scan insertion synthesis DDC
# -----------------------------------------------------------------------------

read_ddc ../data/${rm_project_top}.synthesis.ddc

# -----------------------------------------------------------------------------
# Link the design
# -----------------------------------------------------------------------------

current_design $rm_project_top

link

check_design -no_warnings
check_design -multiple_designs > \
  ../reports/dft/${rm_project_top}_initial.check_design

# Disable register merging
set_register_merging [all_registers] false

# ------------------------------------------------------------------------------
# DFT: Add test ports
# ------------------------------------------------------------------------------


# Scan chain I/O
set_dft_signal -type ScanDataIn  -port [get_ports ${scan_data_in}*]  -view spec
set_dft_signal -type ScanDataOut -port [get_ports ${scan_data_out}*] -view spec

# Associate pairs of scan ports together
for { set i 0 } {$i <= ${num_scan_chains} - 1 } {incr i} {
 set_scan_path $i -scan_data_in ${scan_data_in}$i -scan_data_out ${scan_data_out}$i -view spec
}

# Scan enable requires spec and existing_dft as connected to ICG SE pins and also
# needs to be connected to the DFTSE pins of flops by the insert_dft command
set_dft_signal -type ScanEnable -port [get_ports ${scan_enable}] -active_state 1 -view spec
set_dft_signal -type ScanEnable -port [get_ports ${scan_enable}] -active_state 1 -view existing_dft

# There is no debug present with Cortex-M0 Design Start
if {$rm_design_start == 0} {

# DFTRSTDISABLE/RSTBYPASS is used in lockstep with DFTSE to allow safe scan shift to occur
set_dft_signal -type ScanEnable -port [get_ports ${dft_const}] -active_state 1 -view existing_dft

}

# Resets
set_dft_signal -type Reset -port [get_ports $reset_ports] -active_state 0 -view existing_dft

# Test clock
set_dft_signal -type ScanClock -port [get_ports $clock_ports] -timing {45 55} -view existing_dft


# Mark balanced + synchronous clocks as DFT equivalent to share scan chains
set clock_balance_group_list [list FCLK HCLK SCLK PCLKG PCLK]

if { $rm_include_dbg } {
  lappend clock_balance_group_list DCLK
}
set_dft_equivalent_signals $clock_balance_group_list

# -----------------------------------------------------------------------------
# DFT: Configuration
# -----------------------------------------------------------------------------

# Design already has test-ready scan flops in place
set_scan_configuration -replace false
set_scan_state test_ready

set_dft_insertion_configuration -preserve_design_name true

# Do not run incremental compile as a part of insert_dft
set_dft_insertion_configuration -synthesis_optimization none

set_scan_configuration -create_dedicated_scan_out_ports true \
                       -chain_count ${num_scan_chains} \
                       -add_lockup true \
                       -lockup_type latch \

# DFT clock mixing specification
# For a hierarchical flow, don't mix clocks at the block-level:
#set_scan_configuration -clock_mixing no_mix
# For top-down methodology clock mixing is recommended, if possible:
set_scan_configuration -clock_mixing mix_clocks; #mix_edges

set_dft_drc_configuration -clock_gating_init_cycles 1
set_app_var test_setup_additional_clock_pulse true

# Internal DFT naming style to match top level ports
set_app_var test_scan_enable_port_naming_style ${scan_enable}%s
set_app_var test_scan_enable_inverted_port_naming_style ${scan_enable}n%s
set_app_var test_scan_in_port_naming_style ${scan_data_in}%s
set_app_var test_scan_out_port_naming_style ${scan_data_out}%s

# For verbose DFT DRC reporting:
#set_app_var test_disable_enhanced_dft_drc_reporting false
#set_app_var hdlin_enable_rtldrc_info true

# Specify that all constant flops are to be scan stitched (TEST-504 for constant 0 and TEST-505 for constant 1)
set_dft_drc_rules -ignore {TEST-504}
set_dft_drc_rules -ignore {TEST-505}
report_dft_drc_rules -all

set_dft_drc_configuration -allow_se_set_reset_fix true

create_test_protocol -capture_procedure multi_clock

# -----------------------------------------------------------------------------
# DFT: Scan chain insertion
# -----------------------------------------------------------------------------

# Use the -verbose option of dft_drc to assist in debugging if necessary
dft_drc > ../reports/dft/${rm_project_top}.initial_dft_drc

report_scan_configuration > ../reports/dft/${rm_project_top}.scan_configuration
report_dft_insertion_configuration > ../reports/dft/${rm_project_top}.dft_insertion_configuration

# Use the '-show all -test_points all' options to preview_dft for more detail
preview_dft > ../reports/dft/${rm_project_top}.preview_dft

insert_dft

# -----------------------------------------------------------------------------
# Additional optimization constraints
# -----------------------------------------------------------------------------

# Control DRC/Fanout for tie cells
# This allows a fanout of 1 on tie cells to be set:
set_auto_disable_drc_nets -constant false

# Prevent assignment statements resulting from insert_dft
set_fix_multiple_port_nets -all -buffer_constants [get_designs]

# Critical range for core
set_critical_range [expr 0.10 * ${clock_period} ] ${rm_project_top}

# Isolate the ports for accurate timing model creation
set clock_ports [filter_collection [get_attribute [get_clocks] sources] object_class==port]
set isolated_inputs [remove_from_collection [all_inputs] $clock_ports ]

set_isolate_ports -type buffer -force [get_ports ${isolated_inputs} ]
set_isolate_ports -type buffer -force [all_outputs]

# Set to enable full range of flops for synthesis consideration
set compile_filter_prune_seq_cells false

# -----------------------------------------------------------------------------
# DFT: Post DFT incremental optimization
# -----------------------------------------------------------------------------

# Incremental compile required after scan chain insertion
compile_ultra -incremental -scan -no_autoungroup

# -----------------------------------------------------------------------------
# DFT: Write out test protocols and reports
# -----------------------------------------------------------------------------

# write_scan_def adds SCANDEF info to the design database in memory so this
# must be performed prior to writing out the design
write_scan_def -output ../data/${rm_project_top}.dft_scandef

check_scan_def > ../reports/dft/${rm_project_top}.check_scan_def

write_test_model -format ctl -output ../data/${rm_project_top}.dft_ctl

report_dft_signal > ../reports/dft/${rm_project_top}.dft_signals

write_test_protocol -names verilog -output ../data/${rm_project_top}.dft_scan_spf
dft_drc -verbose > ../reports/dft/${rm_project_top}.dft_drc

report_scan_path > ../reports/dft/${rm_project_top}.scanpath
report_scan_path -chain all > ../reports/dft/${rm_project_top}.scanpath_chain
report_scan_path -cell  all > ../reports/dft/${rm_project_top}.scanpath_cell


# -----------------------------------------------------------------------------
# Change names before output
# -----------------------------------------------------------------------------

# If this will be a sub-block in a hierarchical design, uniquify with block
# unique names to avoid name collisions when integrating the design at the top
# level
set_app_var uniquify_naming_style ${rm_project_top}_%s_%d
uniquify -force

define_name_rules verilog -case_insensitive
change_names -rules verilog -hierarchy -verbose > ../reports/dft/${rm_project_top}.change_names



# ------------------------------------------------------------------------------
# Write out design data
# ------------------------------------------------------------------------------

set_app_var verilogout_higher_designs_first true
set_app_var verilogout_no_tri true

write -format ddc -hierarchy -output ../data/${rm_project_top}.dft.ddc
write -f verilog  -hierarchy -output ../data/${rm_project_top}.dft.v

# ------------------------------------------------------------------------------
# Write out design data
# ------------------------------------------------------------------------------

# Write and close SVF file, make it available for immediate use
set_svf -off

# Write SDF backannotation data from DCT placement for static timing analysis
write_sdf ../data/${rm_project_top}.dft.sdf

# Do not write out net RC info into SDC
set_app_var write_sdc_output_lumped_net_capacitance false
set_app_var write_sdc_output_net_resistance false

# Write out SDC version 1.7 to omit set_voltage for backwards compatibility
write_sdc -version 1.7 -nosplit ../data/${rm_project_top}.dft.sdc


# ------------------------------------------------------------------------------
# Write final reports
# ------------------------------------------------------------------------------

printvar > ../reports/dft/${rm_project_top}.vars

check_design -multiple_designs > \
  ../reports/dft/${rm_project_top}.check_design

check_timing > \
  ../reports/dft/${rm_project_top}.check_timing

report_qor > \
  ../reports/dft/${rm_project_top}.qor

report_timing -delay max \
              -max_paths 10 \
              -nosplit \
              -path full_clock_expanded \
              -nets \
              -transition_time \
              -input_pins > \
  ../reports/dft/${rm_project_top}.timing-max

# Create compacted version of the timing report showing only nets
set fr [open ../reports/dft/${rm_project_top}.timing-max r]
set fw [open ../reports/dft/${rm_project_top}.timing-max-nets w]

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
  ../reports/dft/${rm_project_top}.loops

report_area -hierarchy \
            -physical > \
  ../reports/dft/${rm_project_top}.area

report_power -nosplit > \
  ../reports/dft/${rm_project_top}.power

report_constraint -all_violators \
                  -nosplit > \
  ../reports/dft/${rm_project_top}.constraint_violators

report_design > \
  ../reports/dft/${rm_project_top}.design_attributes

report_clocks -attributes \
              -skew > \
  ../reports/dft/${rm_project_top}.clocks

report_clock_gating -multi_stage \
                    -verbose \
                    -gated \
                    -ungated \
  > ../reports/dft/${rm_project_top}.clock_gating

report_clock_tree -summary \
                  -settings \
                  -structure > \
  ../reports/dft/${rm_project_top}.clock_tree

query_objects -truncate 0 [all_registers -level_sensitive ] \
  > ../reports/dft/${rm_project_top}.latches

report_isolate_ports -nosplit > \
  ../reports/dft/${rm_project_top}.isolate_ports

report_net_fanout -threshold 32 > \
  ../reports/dft/${rm_project_top}.high_fanout_nets

report_port -verbose \
            -nosplit > \
  ../reports/dft/${rm_project_top}.port

report_hierarchy > \
  ../reports/dft/${rm_project_top}.hierarchy

report_resources -hierarchy > \
  ../reports/dft/${rm_project_top}.resources

report_compile_options > \
  ../reports/dft/${rm_project_top}.compile_options

report_congestion > \
  ../reports/dft/${rm_project_top}.congestion

# Zero interconnect delay mode to investigate potential design/floorplan problems
set_zero_interconnect_delay_mode true
report_timing -delay max \
              -max_paths 10 \
              -nosplit \
              -path full_clock_expanded \
              -nets \
              -transition_time \
              -input_pins > \
  ../reports/dft/${rm_project_top}.zero_interconnect_timing

report_qor > \
  ../reports/dft/${rm_project_top}.zero_interconnect_qor
set_zero_interconnect_delay_mode false

# ------------------------------------------------------------------------------
# Insert scan chains and report estimated scan coverage
# ------------------------------------------------------------------------------

dft_drc -verbose -coverage_estimate > \
  ../reports/dft/${rm_project_top}.scan_estimate

# ------------------------------------------------------------------------------
# Report message summary and quit
# ------------------------------------------------------------------------------

print_message_info

set end_time [clock seconds]; echo [string toupper inform:] End time [clock format ${end_time} -gmt false]

# Total script wall clock run time
echo "[string toupper inform:] Time elapsed: [format %02d \
                     [expr ($end_time - $start_time)/86400]]d \
                    [clock format [expr ($end_time - $start_time)] \
                    -format %Hh%Mm%Ss -gmt true]"

exit

# ------------------------------------------------------------------------------
# End of File
# ------------------------------------------------------------------------------
