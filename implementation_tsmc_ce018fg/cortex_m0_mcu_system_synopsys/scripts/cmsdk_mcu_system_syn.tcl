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
# Purpose : Synthesis Script - Synthesis
#
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Job Diagnostics
# ------------------------------------------------------------------------------

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

# ------------------------------------------------------------------------------
# Set-up Target Libraries
# ------------------------------------------------------------------------------

set_app_var search_path [concat . $stdcell_search_path $search_path]

set_app_var synthetic_library dw_foundation.sldb
set_app_var target_library [concat $stdcell_library(db,$slow_corner_pvt)]
set_app_var link_library [concat * $target_library $synthetic_library]

set_app_var mw_reference_library [concat $stdcell_mw_library]

# ------------------------------------------------------------------------------
# Associate libraries with min libraries
# ------------------------------------------------------------------------------

foreach max_lib [concat $stdcell_library(db,$slow_corner_pvt)] \
        min_lib [concat $stdcell_library(db,$fast_corner_pvt)] \
    {
        echo "set_min_library $max_lib -min_version $min_lib"
        set_min_library $max_lib -min_version $min_lib
    }

# ------------------------------------------------------------------------------
# Create MW design library
# ------------------------------------------------------------------------------

set_app_var mw_design_library $rm_project_top

create_mw_lib -technology $tech_file \
              -mw_reference_library $mw_reference_library \
                                    $mw_design_library

open_mw_lib $mw_design_library

# Check consistency of logical vs. physical libraries
check_library

set_tlu_plus_files -max_tluplus $tluplus_file($slow_corner_extraction) \
                   -min_tluplus $tluplus_file($fast_corner_extraction) \
                   -tech2itf_map $tf2itf_map_file

check_tlu_plus_files

# -----------------------------------------------------------------------------
# Library cell optimizations
# -----------------------------------------------------------------------------

# Tie cell optimizations
foreach libraryname [array names tie_cells] {
    foreach tiecelltype $tie_cells($libraryname) {
        set tie_cell [get_lib_cells ${libraryname}/${tiecelltype}]
        remove_attribute $tie_cell dont_use
        remove_attribute $tie_cell dont_touch
        foreach_in_collection tiecellpin [get_lib_pins -of_objects $tie_cell -filter "pin_direction == out"] {
            #set_attribute $tiecellpin max_capacitance 0.1 -type float
            #set_attribute $tiecellpin max_fanout 1.0 -type float
        }
    }
}

# Set any dont use lists
foreach libraryname [array names dont_use] {
  foreach dontusecelltype $dont_use($libraryname) {
      echo "set_dont_use -power [get_object_name [get_lib_cells ${libraryname}/${dontusecelltype}]]"
      set_dont_use -power [get_lib_cells ${libraryname}/${dontusecelltype}]
      unset dontusecelltype
  }
  unset libraryname
}

# ------------------------------------------------------------------------------
# Setup for Formality verification
# ------------------------------------------------------------------------------

set_svf ../data/${rm_project_top}.synthesis.svf

# ------------------------------------------------------------------------------
# Setup for SAIF name mapping database
# ------------------------------------------------------------------------------

saif_map -start

# ------------------------------------------------------------------------------
# Read in the design verilog RTL
# ------------------------------------------------------------------------------

# Default to read Verilog as standard version 2001 (not 2005)
set_app_var hdlin_vrlg_std 2001

# Don't optimize constants for Formality and ID registers.
set_app_var compile_seqmap_propagate_constants false

# Identify architecturally instantiated clock gates
# Note: This application variable must be set BEFORE the RTL is read in.
set_app_var power_cg_auto_identify true

# Check for latches in RTL
set_app_var hdlin_check_no_latch true

# Local directory for intermediate elaboration files
define_design_lib work -path elab

# Setup RTL files and paths
source -echo -verbose ../scripts/${rm_project_top}_verilog.tcl

# Tee analyze output to separate log file
redirect -tee ../reports/synthesis/${rm_project_top}.analyze { \
  analyze -format verilog $rtl_image }

# Tee elaboration output to separate log file
redirect -tee ../reports/synthesis/${rm_project_top}.elaborate { \
  elaborate -architecture verilog ${rm_project_top}}


write -hierarchy -format ddc \
      -output ../data/${rm_project_top}.synthesis-unmapped.ddc

# ------------------------------------------------------------------------------
# Link the design
# ------------------------------------------------------------------------------

current_design $rm_project_top

link

check_design -no_warnings
check_design -multiple_designs > \
  ../reports/synthesis/${rm_project_top}_initial.check_design

# Disable register merging
set_register_merging [all_registers] false

# ------------------------------------------------------------------------------
# DFT: Add test ports
# ------------------------------------------------------------------------------

# DFT ports can be created here to ease use of pin physical constraints data
# This also ensures any path groups and constraints cover all required ports

for { set i 0 } {$i <= ${num_scan_chains} - 1 } {incr i} {
  create_port ${scan_data_in}$i  -direction "in"
  create_port ${scan_data_out}$i -direction "out"
}

if {[get_ports ${scan_enable}] == ""} {
  create_port ${scan_enable} -direction in
}

set_dft_signal -view spec              \
               -port ${scan_enable} \
               -type ScanEnable        \
               -active_state 1

set_dft_signal -view existing_dft      \
               -port ${scan_enable} \
               -type ScanEnable        \
               -active_state 1

# ------------------------------------------------------------------------------
# Clock and constraints
# ------------------------------------------------------------------------------

source -echo -verbose ../scripts/${rm_project_top}_clocks.tcl

source -echo -verbose ../scripts/${rm_project_top}_constraints.tcl

# -----------------------------------------------------------------------------------
# stop synthesis optimization for the following static signals, add more other
# appropriate signals here for your design
# -----------------------------------------------------------------------------------

if {${rm_core_sel} == "CORTEX_M0PLUS"} {
set_dont_touch [list  u_cm0pmtbintegration/ECOREVNUM]
} else {
if {!${rm_design_start}} {
set_dont_touch [list  u_cortex_m0_integration/ECOREVNUM]
}
}

# ------------------------------------------------------------------------------
# Set design context
# ------------------------------------------------------------------------------

# Set the maximum fanout value on the design
set_max_fanout ${max_fanout} $rm_project_top

# Set the maximum transition value on the design
set_max_transition $max_transition($slow_corner_pvt)  $rm_project_top

# Load all outputs with suitable capacitance
set_load $output_load [all_outputs]

# Derive list of clock ports
set clock_ports [filter_collection [get_attribute [get_clocks] sources] object_class==port]

# Drive input ports with a standard driving cell and input transition
set_driving_cell -library $target_library_name($slow_corner_pvt) \
                 -from_pin ${driving_from_pin} \
                 -input_transition_rise $max_transition($slow_corner_pvt) \
                 -input_transition_fall $max_transition($slow_corner_pvt) \
                 -lib_cell ${driving_cell} \
                 -pin ${driving_pin} \
                 [remove_from_collection [all_inputs] ${clock_ports} ]

set_driving_cell -library $target_library_name($slow_corner_pvt) \
                 -from_pin ${clock_driving_from_pin} \
                 -input_transition_rise $max_transition($slow_corner_pvt) \
                 -input_transition_fall $max_transition($slow_corner_pvt) \
                 -lib_cell ${clock_driving_cell} \
                 -pin ${clock_driving_pin} \
                 ${clock_ports}

# ------------------------------------------------------------------------------
# Set Operating conditions (Synthesis uses best case / worst case)
# ------------------------------------------------------------------------------

set_operating_conditions \
-max $operating_condition_name($slow_corner_pvt) -max_lib [get_libs $target_library_name($slow_corner_pvt)] \
-min $operating_condition_name($fast_corner_pvt) -min_lib [get_libs $target_library_name($fast_corner_pvt)] \
-analysis_type bc_wc

# ------------------------------------------------------------------------------
# Create default path groups
# ------------------------------------------------------------------------------

# Separating paths can help improve optimization.

set ports_clock_root [get_ports [all_fanout -flat -clock_tree -level 0]]

group_path -name Inputs  -from [remove_from_collection [all_inputs] \
                                                       $ports_clock_root]
group_path -name Outputs -to   [all_outputs]

# Group internal paths between registers
group_path -name Regs_to_Regs -from [all_registers] -to [all_registers]

# ------------------------------------------------------------------------------
# Clock gating setup
# ------------------------------------------------------------------------------

set_app_var compile_clock_gating_through_hierarchy true
set_app_var power_cg_balance_stages true

set_clock_gating_style -sequential_cell latch \
                       -positive_edge_logic $icg_name \
                       -control_point before \
                       -control_signal scan_enable \
                       -num_stages 2 \
                       -max_fanout 40

# Register latency
set_clock_gate_latency -clock HCLK -stage 0 -fanout_latency [list 1-inf $pre_cts_clock_latency_estimate]
# ICG clock latency (latency=bottom of clock tree less ICG CK-ECK delay)
set_clock_gate_latency -clock HCLK -stage 1 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 0.6]]
# ICG clock latency (latency=bottom of clock tree less 2*ICG CK-ECK delay)
set_clock_gate_latency -clock HCLK -stage 2 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 1.2]]

set_clock_gate_latency -clock SCLK -stage 0 -fanout_latency [list 1-inf $pre_cts_clock_latency_estimate]
# ICG clock latency (latency=bottom of clock tree less ICG CK-ECK delay)
set_clock_gate_latency -clock SCLK -stage 1 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 0.6]]
# ICG clock latency (latency=bottom of clock tree less 2*ICG CK-ECK delay)
set_clock_gate_latency -clock SCLK -stage 2 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 1.2]]

set_clock_gate_latency -clock FCLK -stage 0 -fanout_latency [list 1-inf $pre_cts_clock_latency_estimate]
# ICG clock latency (latency=bottom of clock tree less ICG CK-ECK delay)
set_clock_gate_latency -clock FCLK -stage 1 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 0.6]]
# ICG clock latency (latency=bottom of clock tree less 2*ICG CK-ECK delay)
set_clock_gate_latency -clock FCLK -stage 2 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 1.2]]

set_clock_gate_latency -clock PCLK -stage 0 -fanout_latency [list 1-inf $pre_cts_clock_latency_estimate]
# ICG clock latency (latency=bottom of clock tree less ICG CK-ECK delay)
set_clock_gate_latency -clock PCLK -stage 1 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 0.6]]
# ICG clock latency (latency=bottom of clock tree less 2*ICG CK-ECK delay)
set_clock_gate_latency -clock PCLK -stage 2 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 1.2]]

set_clock_gate_latency -clock PCLKG -stage 0 -fanout_latency [list 1-inf $pre_cts_clock_latency_estimate]
# ICG clock latency (latency=bottom of clock tree less ICG CK-ECK delay)
set_clock_gate_latency -clock PCLKG -stage 1 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 0.6]]
# ICG clock latency (latency=bottom of clock tree less 2*ICG CK-ECK delay)
set_clock_gate_latency -clock PCLKG -stage 2 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 1.2]]

if { ${rm_include_dbg} } {
  set_clock_gate_latency -clock DCLK -stage 0 -fanout_latency [list 1-inf $pre_cts_clock_latency_estimate]
  # ICG clock latency (latency=bottom of clock tree less ICG CK-ECK delay)
  set_clock_gate_latency -clock DCLK -stage 1 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 0.6]]
  # ICG clock latency (latency=bottom of clock tree less 2*ICG CK-ECK delay)
  set_clock_gate_latency -clock DCLK -stage 2 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 1.2]]

  set_clock_gate_latency -clock SWCLKTCK -stage 0 -fanout_latency [list 1-inf $pre_cts_clock_latency_estimate]
  # ICG clock latency (latency=bottom of clock tree less ICG CK-ECK delay)
  set_clock_gate_latency -clock SWCLKTCK -stage 1 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 0.6]]
  # ICG clock latency (latency=bottom of clock tree less 2*ICG CK-ECK delay)
  set_clock_gate_latency -clock SWCLKTCK -stage 2 -fanout_latency [list 1-inf [expr $pre_cts_clock_latency_estimate - 1.2]]
}

# Apply architectural clock gating latency:
# Note: Small latencies as intended to be nearer top of clock tree
set architectural_cg [get_cells -hierarchical -filter "full_name=~*ICGCell*"]

foreach_in_collection cgname $architectural_cg {
  set cgname [get_object_name $cgname]
  echo "Setting clock latency for $cgname"
  set_clock_latency 0.05                            [get_pins $cgname/CK]
  set_clock_latency $pre_cts_clock_latency_estimate [get_pins $cgname/ECK]
}

# ------------------------------------------------------------------------------
# Apply power optimization constraints
# ------------------------------------------------------------------------------

# A SAIF file can be used for power optimization. Without this a default toggle
# rate of 0.1 will be used for propagating switching activity
# read_saif -auto_map_names -input ../data/${rm_project_top}.saif -instance ${rm_project_top} -verbose

# Setting power constraints will automatically enable power prediction using clock tree estimation.
set_power_prediction true

# -----------------------------------------------------------------------------
# Physical constraints
# -----------------------------------------------------------------------------

# Specify ignored layers for routing to improve correlation
set_preferred_routing_direction -layers {METAL1 METAL3 METAL5} -direction horizontal
set_preferred_routing_direction -layers {METAL2 METAL4 METAL6} -direction vertical

# Target five routing layers (power on METAL6)
set_ignored_layers -min_routing_layer METAL1
set_ignored_layers -max_routing_layer METAL5

report_ignored_layers

report_preferred_routing_direction

# ------------------------------------------------------------------------------
# Apply synthesis tool options
# ------------------------------------------------------------------------------

set_app_var enable_recovery_removal_arcs true

# Case analysis required to support EMA value setting for memories
set_app_var case_analysis_with_logic_constants true

set_app_var physopt_enable_via_res_support true

set_app_var write_name_nets_same_as_ports true
set_app_var report_default_significant_digits 3

# ------------------------------------------------------------------------------
# Additional optimization constraints
# ------------------------------------------------------------------------------

# Control DRC/Fanout for tie cells
# This allows a fanout of 1 on tie cells to be set:
set_auto_disable_drc_nets -constant false

# Prevent assignment statements in the Verilog netlist.
set_fix_multiple_port_nets -all -buffer_constants [get_designs]

# Critical range for core
set_critical_range [expr 0.10 * ${clock_period} ] ${rm_project_top}

# Isolate the ports for accurate timing model creation
set clock_ports [filter_collection [get_attribute [get_clocks] sources] object_class==port]
set isolated_inputs [remove_from_collection [all_inputs] $clock_ports ]

set_isolate_ports -type buffer -force [get_ports ${isolated_inputs}]
set_isolate_ports -type buffer -force [all_outputs]

# Set to enable full range of flops for synthesis consideration
set compile_filter_prune_seq_cells false

# ------------------------------------------------------------------------------
# Compile the design
# ------------------------------------------------------------------------------

compile_ultra -scan -gate_clock -no_autoungroup

# ------------------------------------------------------------------------------
# Change names before output
# ------------------------------------------------------------------------------

# If this will be a sub-block in a hierarchical design, uniquify with block
# unique names to avoid name collisions when integrating the design at the top
# level
set_app_var uniquify_naming_style ${rm_project_top}_%s_%d
uniquify -force

define_name_rules verilog -case_insensitive
change_names -rules verilog -hierarchy -verbose > \
  ../reports/synthesis/${rm_project_top}.change_names

# ------------------------------------------------------------------------------
# Write out design data
# ------------------------------------------------------------------------------

set_app_var verilogout_higher_designs_first true
set_app_var verilogout_no_tri true

write -format ddc -hierarchy -output ../data/${rm_project_top}.synthesis.ddc
write -f verilog  -hierarchy -output ../data/${rm_project_top}.synthesis.v

# ------------------------------------------------------------------------------
# Write out design data
# ------------------------------------------------------------------------------

# Write and close SVF file, make it available for immediate use
set_svf -off


# Write parasitics data from DCT placement for static timing analysis
write_parasitics -output ../data/${rm_project_top}.synthesis.spef

# Write SDF backannotation data from DCT placement for static timing analysis
write_sdf ../data/${rm_project_top}.synthesis.sdf

# Do not write out net RC info into SDC
set_app_var write_sdc_output_lumped_net_capacitance false
set_app_var write_sdc_output_net_resistance false

# Write out SDC version 1.7 to omit set_voltage for backwards compatibility
write_sdc -version 1.7 -nosplit ../data/${rm_project_top}.synthesis.sdc

# If SAIF is used, write out SAIF name mapping file for PrimeTime-PX
saif_map -type ptpx -write_map ../reports/synthesis/${rm_project_top}_SAIF.namemap

# ------------------------------------------------------------------------------
# Write final reports
# ------------------------------------------------------------------------------

source -echo -verbose ../scripts/${rm_project_top}_reports.tcl

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
