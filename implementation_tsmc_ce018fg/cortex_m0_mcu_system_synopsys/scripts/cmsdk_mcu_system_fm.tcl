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
# Purpose       : Formality script for equivalence checking a gate level
#                 netlist to the RTL
#
#------------------------------------------------------------------------------
# The variable $netlist is passed to this script to check ../data/${netlist}.v
# This is to allow LEC analysis after any key implementation stage
# -----------------------------------------------------------------------------
# Job diagnostics
# -----------------------------------------------------------------------------

set start_time [clock seconds] ; echo [clock format ${start_time} -gmt false]


echo [pwd]

print_suppressed_messages

set rm_create_test_wrapper 0

# -----------------------------------------------------------------------------------
# Setup the configuration
# -----------------------------------------------------------------------------------

source -echo -verbose ../scripts/design_config.tcl

# -----------------------------------------------------------------------------------
# Setup the Target Technology
# -----------------------------------------------------------------------------------

source -echo -verbose ../scripts/${rm_project_top}_tech.tcl

# -----------------------------------------------------------------------------------
# Set Formality Variables
# -----------------------------------------------------------------------------------

# Pessimistic analysis mode on constant registers
set verification_assume_reg_init none

# Enable identification of clock gating in the design
set_app_var verification_clock_gate_edge_analysis true

# Account for inversions across register boundaries
set_app_var verification_inversion_push true

# This variable should be set to the top of the Synopsys installation tree
# containing Designware
# If left blank, DesignWare instances are treated as black boxes
set_app_var hdlin_dwroot ""

# -----------------------------------------------------------------------------
# Example variables for investigating equivalence fails
# -----------------------------------------------------------------------------

# Enable auto setup mode using:
#set_app_var synopsys_auto_setup true

# Synopsys auto setup mode changes basic settings and includes more SVF info:
# * hdlin_error_on_mismatch_message = false
# * hdlin_ignore_embedded_configuration = true (VHDL only)
# * hdlin_ignore_full_case = false
# * hdlin_ignore_parallel_case = false
# * signature_analysis_allow_subset_match = false
# * svf_ignore_unqualified_fsm_information = false (dependent on SVF)
# * verification_set_undriven_signals = synthesis
# * verification_verify_directly_undriven_output = false

# Restore these values to defaults if using synopsys_auto_setup mode
set_app_var verification_set_undriven_signals "BINARY:X"
set_app_var verification_verify_directly_undriven_output true

# Switch off the signature analysis
#set_app_var signature_analysis_match_compare_points false
#set_app_var signature_analysis_match_datapath false
#set_app_var signature_analysis_match_hierarchy false

# Increase number of failing points before halting verification (0 = unlimited)
#set_app_var verification_failing_point_limit 0

# -----------------------------------------------------------------------------------
# Read the SVF file created during implementation
# -----------------------------------------------------------------------------------

set_svf ../data/${rm_project_top}.synthesis.svf

if { ![regexp {synthesis} $netlist] } {
  set_svf -append ../data/${rm_project_top}.dft.svf
}

# -----------------------------------------------------------------------------
# Read in the libraries to determine cell functionality
# -----------------------------------------------------------------------------

set_app_var search_path [concat . $stdcell_search_path $search_path]

read_db $stdcell_library(db,$slow_corner_pvt)

# -----------------------------------------------------------------------------------
# Read in the Reference Design ( -> r )
# -----------------------------------------------------------------------------------

source -echo -verbose ../scripts/${rm_project_top}_verilog-rtl.tcl
read_verilog -r -work_library WORK -01 $rtl_image

set_top r:/WORK/${rm_project_top}

# -----------------------------------------------------------------------------------
# Read in the Implementation Design ( -> i )
# -----------------------------------------------------------------------------------

# Netlist file name ${netlist}.v - is passed through from fm_shell invocation

read_verilog -i -work_library WORK -netlist ../data/${netlist}.v
puts "Verifying netlist"

set_top i:/WORK/${rm_project_top}

# -----------------------------------------------------------------------------
# Compare rules to avoid naming concordance differences around generate blocks
# -----------------------------------------------------------------------------

# Mismatching compare points by signature analysis may occur around generate
# blocks. These rules may help the generic and HANDINST_ cells to match

set_compare_rule r:/WORK/${rm_project_top} -from {gen_rar.} -to {}
set_compare_rule i:/WORK/${rm_project_top} -from {gen_rar.} -to {}

set_compare_rule r:/WORK/${rm_project_top} -from {gen_non_rar.} -to {}
set_compare_rule i:/WORK/${rm_project_top} -from {gen_non_rar.} -to {}

# -----------------------------------------------------------------------------------
# Configure constant ports (disable scan shift, including test wrapper if applicable)
# -----------------------------------------------------------------------------------

set_constant  r:/WORK/${rm_project_top}/${scan_enable} 0 -type port
set_constant  i:/WORK/${rm_project_top}/${scan_enable} 0 -type port

for { set x 0 } {$x <=  ${num_scan_chains} -1  } {incr x} {
  set_dont_verify_point i:/WORK/${rm_project_top}/${scan_data_out}$x -type port
  set_dont_verify_point i:/WORK/${rm_project_top}/${scan_data_in}$x -type port
}

if { $rm_create_test_wrapper } {
  set_constant i:/WORK/${rm_project_top}/${rm_wrp_mode1}  0 -type port
  set_constant i:/WORK/${rm_project_top}/${rm_wrp_mode2}  0 -type port
  set_constant i:/WORK/${rm_project_top}/${rm_wrp_ishift} 0 -type port
  set_constant i:/WORK/${rm_project_top}/${rm_wrp_oshift} 0 -type port
}

# -----------------------------------------------------------------------------
# Ignoring SRPG ports on wrapper (Not implemented, no RTL function)
# -----------------------------------------------------------------------------

if {${rm_core_sel} == "CORTEX_M0PLUS"} {
  set_dont_verify_point r:/WORK/${rm_project_top}/SYSPWRDOWNACK -type port
  set_dont_verify_point r:/WORK/${rm_project_top}/DBGPWRDOWNACK -type port
  puts "Don't verify SYSPWRDOWNACK and DBGPWRDOWNACK"
}

# -----------------------------------------------------------------------------------
# Identify the mode of clock gating if used in the design
# -----------------------------------------------------------------------------------

set_app_var verification_clock_gate_hold_mode low

# -----------------------------------------------------------------------------------
# Set reference and implementation designs
# -----------------------------------------------------------------------------------

set_reference_design  r:/WORK/${rm_project_top}
set_implementation_design  i:/WORK/${rm_project_top}

# -----------------------------------------------------------------------------------
# Perform matching of compare points
# -----------------------------------------------------------------------------------

## User match for wrapper cells
if { $rm_create_test_wrapper } {
  set_compare_rule -from {/temp_cto_reg} -to {} $impl
}

match

report_matched_points         > ../reports/lec/${netlist}.matched.fm
report_unmatched_points -status unread > ../reports/lec/${netlist}.unread.fm
report_unmatched_points       > ../reports/lec/${netlist}.unmatched.fm

# Report setup status after matching
report_setup_status

# -----------------------------------------------------------------------------------
# Verify the design
# -----------------------------------------------------------------------------------

set status [ verify r:/WORK/${rm_project_top} i:/WORK/${rm_project_top} ]

report_passing_points         > ../reports/lec/${netlist}.passed.fm
report_failing_points         > ../reports/lec/${netlist}.failed.fm
report_aborted_points         > ../reports/lec/${netlist}.aborted.fm
report_constants              > ../reports/lec/${netlist}.constants.fm
report_loops                  > ../reports/lec/${netlist}.loops.fm
report_undriven_nets          > ../reports/lec/${netlist}.undriven_nets.fm
report_multidriven_nets       > ../reports/lec/${netlist}.multidriven_nets.fm
report_guidance -summary      > ../reports/lec/${netlist}.svf_guidance.summary
report_guidance -to             ../reports/lec/${netlist}.svf_guidance.txt
report_libraries -defects all > ../reports/lec/${netlist}.defects.fm

# Analyze points and save session if verification unsuccessful
if {$status == 0} {
  analyze_points -all > ../reports/lec/${netlist}.analysis_results
  save_session -replace ../data/$netlist.lec ;# Save session as a .fss file
} else {
  echo "No points analyzed" > ../reports/lec/${netlist}.analysis_results
}

# -----------------------------------------------------------------------------
# Report logical equivalence status
# -----------------------------------------------------------------------------

report_status

# -----------------------------------------------------------------------------
# Report message summary and quit
# -----------------------------------------------------------------------------

print_message_info

set end_time [clock seconds]; echo [clock format ${end_time} -gmt false]

# Total script wall clock run time
echo "Time elapsed: [format %02d [expr ( $end_time - $start_time ) / 86400 ]]d\
[clock format [expr ( $end_time - $start_time ) ] -format %Hh%Mm%Ss -gmt true]"

exit
