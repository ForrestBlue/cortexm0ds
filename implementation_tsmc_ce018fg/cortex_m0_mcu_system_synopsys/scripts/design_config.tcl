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
# Purpose : Synthesis Script - Configuration
#
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# THIS FILE DEFINES CONFIGURATION-SPECIFIC DATA
# ------------------------------------------------------------------------------


# Ultimate top-level of design
set rm_project_top        cmsdk_mcu_system

# ------------------------------------------------------------------------------
# configuration related
# ------------------------------------------------------------------------------
set rm_include_dbg            0     ;# Set to 0 if Debug Logic is not included in the RTL (parameter DBG = 0)
set rm_include_dma            0     ;# Set to 0 if DMA-230 module is not included in the RTL
set rm_include_f16            0     ;# Set to 1 for 16-bit Flash support (Cortex-M0+ only)
set rm_core_sel               CORTEX_M0 ;# Set to CORTEX_M0 or CORTEX_M0PLUS (default support Cortex-M0+)
set rm_design_start           1     ;# Set to 1 if Coretex-M0 DesignStart core is used


set rm_include_mtb            0     ;# set 1 to include CoreSight MTB M0+ (Cortex-M0+ only)
set rm_include_iop            0     ;# set 1 to include IO Port GPIO in place of AHB GPIO (Cortex-M0+ only)


# ------------------------------------------------------------------------------
# configuration checking
# ------------------------------------------------------------------------------

# Some checks to make sure invalid selections are caught before running the
# synthesis. Do not modify this subsection.

if {${rm_core_sel} == "CORTEX_M0PLUS"} {
set rm_design_start           0     ;# Set to 0 if using Cortex-M0+ core
}

if {${rm_core_sel} == "CORTEX_M0"} {
set rm_include_f16            0     ;# Set to 0 if using Cortex-M0 core
set rm_include_mtb            0     ;# Set to 0 if using Cortex-M0 core
set rm_include_iop            0     ;# Set to 0 if using Cortex-M0 core
}

if {${rm_design_start} } {
set rm_include_dbg            0     ;# Set to 0 if using Cortex-M0 DesignStart core
}


# ------------------------------------------------------------------------------
# Clock and Reset Definitions
# ------------------------------------------------------------------------------

set common_clock_ports        [list FCLK HCLK SCLK PCLKG PCLK]
set clock_ports               ${common_clock_ports}

set reset_ports               [list PORESETn HRESETn PRESETn]


if {${rm_include_dbg} } {
    set clock_ports          [concat ${clock_ports} DCLK]
    set clock_ports          [concat ${clock_ports} SWCLKTCK ]
    set reset_ports          [concat ${reset_ports} DBGRESETn]
    set reset_ports          [concat ${reset_ports} nTRST]
}

puts $clock_ports
puts $reset_ports

#-------------------------------------------------------------------------------
# Technology Variables
#-------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# DFT Flow Configuration Parameters
# ------------------------------------------------------------------------------

set num_scan_chains           3        ;# Number of scan chains to be inserted

# ------------------------------------------------------------------------------
# Define DFT Port Names
# ------------------------------------------------------------------------------

set scan_data_in             DFTSI     ;# Name of internal scan data in ports
set scan_data_out            DFTSO     ;# Name of internal scan data out ports
set scan_enable              DFTSE     ;# Name of scan shift enable port
if {${rm_core_sel} == "CORTEX_M0PLUS"} {
set dft_const                [list DFTRSTDISABLE] ;# Name of test control port
} else {
set dft_const                [list RSTBYPASS]     ;# Name of test control port
}

# ------------------------------------------------------------------------------
# End of File
# ------------------------------------------------------------------------------
