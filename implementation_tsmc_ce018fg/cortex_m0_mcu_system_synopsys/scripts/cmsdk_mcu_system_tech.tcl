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
# Purpose : Synthesis Script - Tech library setup
#
# ------------------------------------------------------------------------------

puts "Technology setup file for CMSDK with 7-Track RVT cells on TSMC CE018FG process"

# Usage:
# This script creates Tcl arrays to describe technology constants.
# These are keyed around 'extract corner' and 'pvt corner'. However,
# the key could be any required naming or combination of design variables.

# For example:
# 1) "$tluplus_file(key)" could use 'extract' as the key name:
#    setting e.g. "$tluplus_file(typical)" or "$tluplus_file(typical)" etc.
# 2) "$stdcell_library(key)" could use 'libtype,transistor_temperature_voltage' as the
#    key name: setting e.g. "$stdcell_library(db,ff_1p98v_m40c)"
#

# They key can be any required name, such as "$stdcell_library(best)", but
# building up a key name from other independent variable will give consistent
# naming.
# The following variables are used in this flow to set key names:

# Variable:     Description:                     Examples:
#
# $transistor   Transistor n+p process corner    ss, tt, ff
# $voltage      Primary voltage                  1p62v, 1p80v, 1p98v
# $temperature  Operating temperature            m40c, 25c, 125c
# $extraction   RC parasitic extraction corner   typical
# $mode         Run-time mode of design          unconstrained, functional, scanshift

# These allow simpler iteration around scenarios for multi-scenario flows.

# A technology setting may not require a per corner key
# E.g. "$stdcell_search_path" may not change between corners.
# If this did need to change between corners, it could be replaced with
# different verisions, such as "$stdcell_search_path(ss_1p62v_m40c)",
# "$stdcell_search_path(ff_1p98v_m40c)" etc, but all script references must be
# updated to accept data in array from e.g. "$stdcell_search_path($pvt)"

# Important: for variables that include lists, ordering within lists may be
# important. Ensure list ordering is consistent between differently keyed lists
# in the same array.

#####################################################################################
# 1. Main design performance targets
#####################################################################################

# -----------------------------------------------------------------------------
# Default corners:
# -----------------------------------------------------------------------------

# The default corner names are 'keys' defining the min/max PVT used throughout
# implementation. These can be changed if matching libraries/pvt corners are
# available

set slow_corner_pvt ss_1p62v_125c
set typ_corner_pvt  tt_1p80v_25c
set fast_corner_pvt ff_1p98v_m40c

# Equivalent default RC extraction corners 'keys' are also used

set slow_corner_extraction typical
set typ_corner_extraction  typical
set fast_corner_extraction typical

# -----------------------------------------------------------------------------
# Clock timing
# -----------------------------------------------------------------------------

set clock_period            20.000              ;# Target clock period for the system clock
set swclock_period          100.000             ;# Target clock period for the SWCLKTCK clock
set clock_period_jitter     0.000               ;# Cycle jitter (rise-to-rise) +/- N ns
                                                ;# of the whole cycle
set clock_dutycycle_jitter [expr $clock_period * 0.05 ]
                                                ;# Duty cycle distortion as a percentage of the whole
                                                ;# cycle - +/- N%. Affects the falling edge
                                                ;# of the clock
                                                ;# Adjusts clock source falling edge timing
set swclock_dutycycle_jitter [expr $swclock_period * 0.05 ]
                                                ;# Adjusts clock source falling edge
                                                ;# timing

set setup_margin            0.050               ;# in ns. Setup margin
set hold_margin             0.050               ;# in ns. Hold margin
set clock_uncertainty       0.250               ;# in ns. Pre-CTS clock skew estimate

# -----------------------------------------------------------------------------
# Pre-CTS clock skew and latency estimates
# -----------------------------------------------------------------------------

set pre_cts_clock_skew_estimate    0.150
set pre_cts_clock_latency_estimate 2.000

#####################################################################################
# 2. Design environment
#####################################################################################

# -----------------------------------------------------------------------------
# Input driving cell models
# -----------------------------------------------------------------------------

set driving_cell            BUF_X4_A7TULL       ;# The driving cell for all inputs
set driving_from_pin        A
set driving_pin             Y                   ;# The output pin of the driving cell

set clock_driving_cell      CLKBUF_X8_A7TULL    ;# The driving cell for clock ports
set clock_driving_from_pin  A
set clock_driving_pin       Y                   ;# The output pin of the clock driving cell

set icg_name   {integrated:TLATNTSCA_X8_A7TULL} ;# Name of ICG cell

# -----------------------------------------------------------------------------
# Output loading models
# -----------------------------------------------------------------------------

set output_load             0.150               ;# Capacitive load placed on all outputs

# -----------------------------------------------------------------------------
# Max capacitance
# -----------------------------------------------------------------------------

# Keyed from "$transistor_$voltage_$temperature"

# This is used to set the upper limits for tables during timing model creation
# These values have based on the largest max_capacitance in target library
# Smaller values may be preferable for increased accuracy over a smaller range

set max_capacitance(ff_1p98v_125c) 9.999
set max_capacitance(ff_1p98v_m40c) 9.999
set max_capacitance(ss_1p62v_125c) 9.999
set max_capacitance(tt_1p80v_25c)  9.999

# -----------------------------------------------------------------------------
# Transition time targets
# -----------------------------------------------------------------------------

# Keyed from "$transistor_$voltage_$temperature"

# Only max_transition($slow_corner_pvt) is required during implementation
# Others are used in analysis steps such as sta and model creation

set max_transition(ff_1p98v_125c) 2.000
set max_transition(ff_1p98v_m40c) 2.000
set max_transition(ss_1p62v_125c) 2.000
set max_transition(tt_1p80v_25c)  2.000

# Clock transition requirement
set max_clock_transition(ff_1p98v_125c) [expr $max_transition(ff_1p98v_125c)/2.0 ]
set max_clock_transition(ff_1p98v_m40c) [expr $max_transition(ff_1p98v_m40c)/2.0 ]
set max_clock_transition(ss_1p62v_125c) [expr $max_transition(ss_1p62v_125c)/2.0 ]
set max_clock_transition(tt_1p80v_25c)  [expr $max_transition(tt_1p80v_25c)/2.0  ]

set max_fanout                    32            ;# Maximum fanout threshold

#####################################################################################
# 3. Design libraries
#####################################################################################

# -----------------------------------------------------------------------------------
# Path to libraries
# -----------------------------------------------------------------------------------

if {[info exists sh_launch_dir] == 0} {
  set sh_launch_dir "."
}

set libs                  "/arm/scratch/geoflo01/gate_libs/libs"

# -----------------------------------------------------------------------------
# Techfile and metal stack extract models
# -----------------------------------------------------------------------------

set tech_file             ${libs}/arm/tsmc/ce018fg/arm_tech/r5p1/milkyway/6lm/sc7_tech.tf
set tf2itf_map_file       ${libs}/arm/tsmc/ce018fg/arm_tech/r5p1/synopsys_tluplus/6lm/tluplus.map

# Keyed from '$extraction'

set tluplus_file(typical) ${libs}/arm/tsmc/ce018fg/arm_tech/r5p1/synopsys_tluplus/6lm/typical.tluplus

# -----------------------------------------------------------------------------
# Library search path and Milkyway locations
# -----------------------------------------------------------------------------

set stdcell_search_path [ list ${libs}/arm/tsmc/ce018fg/sc7_base_rvt/r9p0-01eac0/db ]

set stdcell_mw_library  [ list ${libs}/arm/tsmc/ce018fg/sc7_base_rvt/r9p0-01eac0/milkyway/6lm/sc7_ce018fg_base_rvt ]

# -----------------------------------------------------------------------------
# NLDM .db filenames
# -----------------------------------------------------------------------------

# Keyed from "db,$transistor_$voltage_$temperature"

# Order within lists must be consistent between corners to allow min/max
# relationship linking.

# Standard Cells
set stdcell_library(db,ff_1p98v_125c) [ list \
                                        sc7_ce018fg_base_rvt_ff_typical_min_1p98v_125c.db \
                                        ]

set stdcell_library(db,ff_1p98v_m40c) [ list \
                                        sc7_ce018fg_base_rvt_ff_typical_min_1p98v_m40c.db \
                                        ]

set stdcell_library(db,ss_1p62v_125c) [ list \
                                        sc7_ce018fg_base_rvt_ss_typical_max_1p62v_125c.db \
                                        ]

set stdcell_library(db,tt_1p80v_25c)  [ list \
                                        sc7_ce018fg_base_rvt_tt_typical_max_1p80v_25c.db \
                                        ]

# -----------------------------------------------------------------------------
# Operating conditions
# -----------------------------------------------------------------------------

# Keyed from "$transistor_$voltage_$temperature"

set operating_condition_name(ff_1p98v_125c) ff_typical_min_1p98v_125c
set target_library_name(ff_1p98v_125c)      sc7_ce018fg_base_rvt_ff_typical_min_1p98v_125c

set operating_condition_name(ff_1p98v_m40c) ff_typical_min_1p98v_m40c
set target_library_name(ff_1p98v_m40c)      sc7_ce018fg_base_rvt_ff_typical_min_1p98v_m40c

set operating_condition_name(ss_1p62v_125c) ss_typical_max_1p62v_125c
set target_library_name(ss_1p62v_125c)      sc7_ce018fg_base_rvt_ss_typical_max_1p62v_125c

set operating_condition_name(tt_1p80v_25c)  tt_typical_max_1p80v_25c
set target_library_name(tt_1p80v_25c)       sc7_ce018fg_base_rvt_tt_typical_max_1p80v_25c

# -----------------------------------------------------------------------------
# Tetramax ATPG cell views
# -----------------------------------------------------------------------------

set tmax_library [ list \
                        ${libs}/arm/tsmc/ce018fg/sc7_base_rvt/r9p0-01eac0/tetramax/sc7_ce018fg_base_rvt.tv \
                       ]

# -----------------------------------------------------------------------------
# Tie cells
# -----------------------------------------------------------------------------

# Keyed from a target libary name,
# Note: Use of wildcards permitted in Tcl for library names and cell names:

set tie_cells(sc7_ce018fg_base_rvt_*) [list TIE* ]

# Example of use within tool:
# foreach libraryname [array names tie_cells] { #foreach group of libraries
#     foreach tiecelltype $tie_cells($libraryname) {#foreach group of tie cells in libraries
#         set tie_cell [get_lib_cells ${libraryname}/${tiecelltype}]
#         remove_attribute $tie_cell dont_use
#         remove_attribute $tie_cell dont_touch
#         foreach_in_collection tiecellpin [get_lib_pins -of_objects $tie_cell -filter "pin_direction == out"] { #foreach tie cells output
#             #set_attribute $tiecellpin max_capacitance 0.1 -type float
#             #set_attribute $tiecellpin max_fanout 1.0 -type float
#         }
#     }
# }

# -----------------------------------------------------------------------------
# Don't use lists
# -----------------------------------------------------------------------------

# Keyed from a target libary name,
# Note: Use of wildcards permitted in Tcl for library names and cell names:
# e.g. set dont_use(sc7_ce018fg_base_rvt*) [list *_XL_* ]

set dont_use(sc7_ce018fg_base_rvt_*) {}

# Basic dont_use for specific cell types
lappend dont_use(sc7_ce018fg_base_rvt_*) *CLK*
lappend dont_use(sc7_ce018fg_base_rvt_*) *EDFF*
lappend dont_use(sc7_ce018fg_base_rvt_*) DLY*
lappend dont_use(sc7_ce018fg_base_rvt_*) TBUF*
lappend dont_use(sc7_ce018fg_base_rvt_*) SDFFTR*

# Banning low drive strength cells may improve speed, but with area/power impact
lappend dont_use(sc7_ce018fg_base_rvt_*) *_XL_*

# Example of use within tool:
# foreach libraryname [array names dont_use] {
#     foreach dontusecelltype $dont_use($libraryname) {
#         echo "set_dont_use -power [get_object_name [get_lib_cells ${libraryname}/${dontusecelltype}]]"
#         set_dont_use -power [get_lib_cells ${libraryname}/${dontusecelltype}]
#     }
# }

# -----------------------------------------------------------------------------------
# Tool reporting defaults
# -----------------------------------------------------------------------------------

# Increase the precision of timing reports to 3 significant digits
# Note: *decreases* precision of area reports to 3 from 6 significant digits
set report_default_significant_digits 3

# End of File
