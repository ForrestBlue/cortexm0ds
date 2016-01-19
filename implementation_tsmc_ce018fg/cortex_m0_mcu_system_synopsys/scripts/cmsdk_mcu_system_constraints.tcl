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
# Purpose :  Synthesis Script - Constraints
#
# ------------------------------------------------------------------------------

# -----------------------------------------------------------------------------------
# Define Cycle Percentage Expressions
# -----------------------------------------------------------------------------------

set cycle90    [expr 0.90 * ${clock_period}]
set cycle80    [expr 0.80 * ${clock_period}]
set cycle70    [expr 0.70 * ${clock_period}]
set cycle60    [expr 0.60 * ${clock_period}]
set cycle50    [expr 0.50 * ${clock_period}]
set cycle40    [expr 0.40 * ${clock_period}]
set cycle30    [expr 0.30 * ${clock_period}]
set cycle20    [expr 0.20 * ${clock_period}]
set cycle10    [expr 0.10 * ${clock_period}]

# ------------------------------------------------------------------------------
# DFT Ports
# ------------------------------------------------------------------------------

set_input_delay  -clock VCLK  -max $cycle20   [get_ports ${scan_enable}]
set_input_delay  -clock VCLK  -min 0.0        [get_ports ${scan_enable}]

# There is no debug present with Cortex-M0 Design Start
if {${rm_design_start} == 0} {
  set_input_delay  -clock VCLK  -max $cycle20 [get_ports ${dft_const}]
  set_input_delay  -clock VCLK  -min 0.0      [get_ports ${dft_const}]
}

for { set i 0 } {$i <=  $num_scan_chains -1  } {incr i} {
  set_input_delay  -clock VCLK  -max $cycle20 [get_ports ${scan_data_in}$i]
  set_input_delay  -clock VCLK  -min 0.0      [get_ports ${scan_data_in}$i]

  set_output_delay -clock VCLK  -max $cycle20 [get_ports ${scan_data_out}$i]
  set_output_delay -clock VCLK  -min 0.0      [get_ports ${scan_data_out}$i]
}

# Setting a multicycle path on Scan Enable is standard procedure as ATPG
# is typically performed at a lower clock frequency
set_multicycle_path 2 -setup -end -from       [get_ports ${scan_enable}]
set_multicycle_path 1 -hold  -end -from       [get_ports ${scan_enable}]

# There is no debug present with Cortex-M0 Design Start
if {${rm_design_start} == 0} {
  set_multicycle_path 2 -setup -end -from     [get_ports ${dft_const}]
  set_multicycle_path 1 -hold  -end -from     [get_ports ${dft_const}]
}

for { set i 0 } {$i <=  $num_scan_chains -1  } {incr i} {
  set_multicycle_path 2 -setup -end -from     [get_ports ${scan_data_in}$i]
  set_multicycle_path 1 -hold  -end -from     [get_ports ${scan_data_in}$i]

  set_multicycle_path 2 -setup -end -to       [get_ports ${scan_data_out}$i]
  set_multicycle_path 1 -hold  -end -to       [get_ports ${scan_data_out}$i]
}

puts "Constrained DFT"

if { ${rm_include_dbg} } {

# -----------------------------------------------------------------------------
# Model asynchronous clock domains crossings
# -----------------------------------------------------------------------------

# Maximum of one core clock cycle delay for asynchronous clock domains crossings

  set_clock_groups -asynchronous -allow_paths -name clk_groups -group [list HCLK SCLK FCLK DCLK VCLK PCLK PCLKG] -group [list SWCLKTCK SVCLK]
  set_max_delay ${clock_period} -from [get_clocks [list HCLK SCLK FCLK DCLK VCLK PCLK PCLKG]] -to   [get_clocks [list SWCLKTCK SVCLK]]
  set_max_delay ${clock_period} -to   [get_clocks [list HCLK SCLK FCLK DCLK VCLK PCLK PCLKG]] -from [get_clocks [list SWCLKTCK SVCLK]]

}

# -----------------------------------------------------------------------------------
# Resets
# -----------------------------------------------------------------------------------

if { ${rm_design_start} } {
  set_input_delay  -clock VCLK -max $cycle60 [get_ports PORESETn]
  set_input_delay  -clock VCLK -min 0.0      [get_ports PORESETn]
} else {
  if { ${rm_include_dbg} } {
    set_input_delay  -clock SVCLK -max $cycle60 [get_ports PORESETn]
    set_input_delay  -clock SVCLK -min 0.0      [get_ports PORESETn]
  } else {
    set_input_delay  -clock VCLK  -max $cycle60 [get_ports PORESETn]
    set_input_delay  -clock VCLK  -min 0.0      [get_ports PORESETn]
  }
}

set_input_delay  -clock VCLK  -max $cycle60 [get_ports HRESETn]
set_input_delay  -clock VCLK  -min 0.0      [get_ports HRESETn]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports PRESETn]
set_input_delay  -clock VCLK  -min 0.0      [get_ports PRESETn]

if { ${rm_include_dbg} } {
  set_input_delay  -clock VCLK  -max $cycle60 [get_ports DBGRESETn]
  set_input_delay  -clock VCLK  -min 0.0      [get_ports DBGRESETn]
  set_input_delay  -clock SVCLK -max $cycle60 [get_ports nTRST]
  set_input_delay  -clock SVCLK -min 0.0      [get_ports nTRST]
}

puts "Constrained Resets"

# -----------------------------------------------------------------------------------
# AHB-lite Ports
# -----------------------------------------------------------------------------------

# AHB-LITE Master Port
set_output_delay -clock VCLK  -max $cycle50 [get_ports {HADDR[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {HADDR[*]}]
set_output_delay -clock VCLK  -max $cycle50 [get_ports {HTRANS[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {HTRANS[*]}]
set_output_delay -clock VCLK  -max $cycle50 [get_ports {HSIZE[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {HSIZE[*]}]
set_output_delay -clock VCLK  -max $cycle50 [get_ports HWRITE]
set_output_delay -clock VCLK  -min 0.0      [get_ports HWRITE]
set_output_delay -clock VCLK  -max $cycle50 [get_ports {HWDATA[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {HWDATA[*]}]


# Combinatorial path from flash_hreadyout, sram_hreadyout, boot_hreadyout
# to HREADY
set_max_delay [expr $cycle80 + $clock_uncertainty + 2.0] -from [get_ports flash_hreadyout] -to [get_ports HREADY]
set_max_delay [expr $cycle80 + $clock_uncertainty + 2.0] -from [get_ports sram_hreadyout]  -to [get_ports HREADY]
set_max_delay [expr $cycle80 + $clock_uncertainty + 2.0] -from [get_ports boot_hreadyout]  -to [get_ports HREADY]

set_output_delay -clock VCLK  -max $cycle30 [get_ports HREADY]
set_output_delay -clock VCLK  -min 0.0      [get_ports HREADY]

set_output_delay -clock VCLK  -max $cycle50 [get_ports flash_hsel]
set_output_delay -clock VCLK  -min 0.0      [get_ports flash_hsel]
set_input_delay  -clock VCLK  -max $cycle40 [get_ports flash_hreadyout]
set_input_delay  -clock VCLK  -min 0.0      [get_ports flash_hreadyout]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports {flash_hrdata[*]}]
set_input_delay  -clock VCLK  -min 0.0      [get_ports {flash_hrdata[*]}]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports flash_hresp]
set_input_delay  -clock VCLK  -min 0.0      [get_ports flash_hresp]

set_output_delay -clock VCLK  -max $cycle50 [get_ports sram_hsel]
set_output_delay -clock VCLK  -min 0.0      [get_ports sram_hsel]
set_input_delay  -clock VCLK  -max $cycle40 [get_ports sram_hreadyout]
set_input_delay  -clock VCLK  -min 0.0      [get_ports sram_hreadyout]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports {sram_hrdata[*]}]
set_input_delay  -clock VCLK  -min 0.0      [get_ports {sram_hrdata[*]}]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports sram_hresp]
set_input_delay  -clock VCLK  -min 0.0      [get_ports sram_hresp]

set_output_delay -clock VCLK  -max $cycle50 [get_ports boot_hsel]
set_output_delay -clock VCLK  -min 0.0      [get_ports boot_hsel]
set_input_delay  -clock VCLK  -max $cycle40 [get_ports boot_hreadyout]
set_input_delay  -clock VCLK  -min 0.0      [get_ports boot_hreadyout]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports {boot_hrdata[*]}]
set_input_delay  -clock VCLK  -min 0.0      [get_ports {boot_hrdata[*]}]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports boot_hresp]
set_input_delay  -clock VCLK  -min 0.0      [get_ports boot_hresp]

puts "Constrained AHBLite Interface Generic"

# -----------------------------------------------------------------------------------
# DEBUG
# -----------------------------------------------------------------------------------

if { ${rm_include_dbg} } {

set_input_delay  -clock SVCLK -max $cycle60 [get_ports SWDITMS]
set_input_delay  -clock SVCLK -min 0.0      [get_ports SWDITMS]
set_input_delay  -clock SVCLK -max $cycle60 [get_ports TDI]
set_input_delay  -clock SVCLK -min 0.0      [get_ports TDI]
set_output_delay -clock SVCLK -max $cycle60 [get_ports SWDO]
set_output_delay -clock SVCLK -min 0.0      [get_ports SWDO]
set_output_delay -clock SVCLK -max $cycle60 [get_ports SWDOEN]
set_output_delay -clock SVCLK -min 0.0      [get_ports SWDOEN]
set_output_delay -clock SVCLK -max $cycle60 [get_ports nTDOEN]
set_output_delay -clock SVCLK -min 0.0      [get_ports nTDOEN]
set_output_delay -clock SVCLK -max $cycle60 [get_ports TDO]
set_output_delay -clock SVCLK -min 0.0      [get_ports TDO]

puts "Constrained Debug Interface Generic"
}

# -----------------------------------------------------------------------------------
# MISC
# -----------------------------------------------------------------------------------
set_input_delay  -clock VCLK  -max $cycle50 [get_ports PCLKEN]
set_input_delay  -clock VCLK  -min 0.0      [get_ports PCLKEN]

set_input_delay  -clock VCLK  -max $cycle50 [get_ports DFTSE]
set_input_delay  -clock VCLK  -min 0.0      [get_ports DFTSE]

set_output_delay -clock VCLK  -max $cycle40 [get_ports APBACTIVE]
set_output_delay -clock VCLK  -min 0.0      [get_ports APBACTIVE]
set_output_delay -clock VCLK  -max $cycle50 [get_ports SLEEPING]
set_output_delay -clock VCLK  -min 0.0      [get_ports SLEEPING]
set_output_delay -clock VCLK  -max $cycle50 [get_ports SLEEPDEEP]
set_output_delay -clock VCLK  -min 0.0      [get_ports SLEEPDEEP]
set_output_delay -clock VCLK  -max $cycle50 [get_ports SYSRESETREQ]
set_output_delay -clock VCLK  -min 0.0      [get_ports SYSRESETREQ]
set_output_delay -clock VCLK  -max $cycle50 [get_ports WDOGRESETREQ]
set_output_delay -clock VCLK  -min 0.0      [get_ports WDOGRESETREQ]
set_output_delay -clock VCLK  -max $cycle50 [get_ports LOCKUP]
set_output_delay -clock VCLK  -min 0.0      [get_ports LOCKUP]
set_output_delay -clock VCLK  -max $cycle50 [get_ports LOCKUPRESET]
set_output_delay -clock VCLK  -min 0.0      [get_ports LOCKUPRESET]
set_output_delay -clock VCLK  -max $cycle50 [get_ports PMUENABLE]
set_output_delay -clock VCLK  -min 0.0      [get_ports PMUENABLE]

# Peripherals I/O
set_input_delay  -clock VCLK  -max $cycle60 [get_ports uart0_rxd]
set_input_delay  -clock VCLK  -min 0.0      [get_ports uart0_rxd]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports uart1_rxd]
set_input_delay  -clock VCLK  -min 0.0      [get_ports uart1_rxd]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports uart2_rxd]
set_input_delay  -clock VCLK  -min 0.0      [get_ports uart2_rxd]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports timer0_extin]
set_input_delay  -clock VCLK  -min 0.0      [get_ports timer0_extin]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports timer1_extin]
set_input_delay  -clock VCLK  -min 0.0      [get_ports timer1_extin]

set_output_delay -clock VCLK  -max $cycle60 [get_ports uart0_txd]
set_output_delay -clock VCLK  -min 0.0      [get_ports uart0_txd]
set_output_delay -clock VCLK  -max $cycle60 [get_ports uart0_txen]
set_output_delay -clock VCLK  -min 0.0      [get_ports uart0_txen]
set_output_delay -clock VCLK  -max $cycle60 [get_ports uart1_txd]
set_output_delay -clock VCLK  -min 0.0      [get_ports uart1_txd]
set_output_delay -clock VCLK  -max $cycle60 [get_ports uart1_txen]
set_output_delay -clock VCLK  -min 0.0      [get_ports uart1_txen]
set_output_delay -clock VCLK  -max $cycle60 [get_ports uart2_txd]
set_output_delay -clock VCLK  -min 0.0      [get_ports uart2_txd]
set_output_delay -clock VCLK  -max $cycle60 [get_ports uart2_txen]
set_output_delay -clock VCLK  -min 0.0      [get_ports uart2_txen]

set_input_delay  -clock VCLK  -max $cycle60 [get_ports timer0_extin]
set_input_delay  -clock VCLK  -min 0.0      [get_ports timer0_extin]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports timer1_extin]
set_input_delay  -clock VCLK  -min 0.0      [get_ports timer1_extin]

set_input_delay  -clock VCLK  -max $cycle60 [get_ports {p0_in[*]}]
set_input_delay  -clock VCLK  -min 0.0      [get_ports {p0_in[*]}]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {p0_out[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {p0_out[*]}]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {p0_outen[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {p0_outen[*]}]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {p0_altfunc[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {p0_altfunc[*]}]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports {p1_in[*]}]
set_input_delay  -clock VCLK  -min 0.0      [get_ports {p1_in[*]}]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {p1_out[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {p1_out[*]}]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {p1_outen[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {p1_outen[*]}]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {p1_altfunc[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {p1_altfunc[*]}]

puts "Constrained Misellaneous Generic"

if {${rm_core_sel} == "CORTEX_M0PLUS"} {

if {${rm_include_mtb}} {
set_output_delay -clock VCLK  -max $cycle10 [get_ports RAMHCLK]
set_output_delay -clock VCLK  -min 0.0      [get_ports RAMHCLK]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {RAMAD[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {RAMAD[*]}]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {RAMWD[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {RAMWD[*]}]
set_output_delay -clock VCLK  -max $cycle60 [get_ports RAMCS]
set_output_delay -clock VCLK  -min 0.0      [get_ports RAMCS]
set_output_delay -clock VCLK  -max $cycle60 [get_ports {RAMWE[*]}]
set_output_delay -clock VCLK  -min 0.0      [get_ports {RAMWE[*]}]

set_input_delay  -clock VCLK  -max $cycle60 [get_ports TSTART]
set_input_delay  -clock VCLK  -min 0.0      [get_ports TSTART]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports TSTOP]
set_input_delay  -clock VCLK  -min 0.0      [get_ports TSTOP]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports {SRAMBASEADDR[*]}]
set_input_delay  -clock VCLK  -min 0.0      [get_ports {SRAMBASEADDR[*]}]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports {RAMRD[*]}]
set_input_delay  -clock VCLK  -min 0.0      [get_ports {RAMRD[*]}]

puts "Constrained Misellaneous Cortex-M0+"
}
}

# -----------------------------------------------------------------------------------
# POWER MANAGEMENT
# -----------------------------------------------------------------------------------

# These signals are not available for DesignStart

if { !${rm_design_start} } {
set_output_delay -clock VCLK  -max $cycle60 [get_ports GATEHCLK]
set_output_delay -clock VCLK  -min 0.0      [get_ports GATEHCLK]
set_output_delay -clock VCLK  -max $cycle60 [get_ports WAKEUP]
set_output_delay -clock VCLK  -min 0.0      [get_ports WAKEUP]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports WICENREQ]
set_input_delay  -clock VCLK  -min 0.0      [get_ports WICENREQ]
set_output_delay -clock VCLK  -max $cycle60 [get_ports WICENACK]
set_output_delay -clock VCLK  -min 0.0      [get_ports WICENACK]
set_input_delay  -clock VCLK  -max $cycle60 [get_ports SLEEPHOLDREQn]
set_input_delay  -clock VCLK  -min 0.0      [get_ports SLEEPHOLDREQn]
set_output_delay -clock VCLK  -max $cycle60 [get_ports SLEEPHOLDACKn]
set_output_delay -clock VCLK  -min 0.0      [get_ports SLEEPHOLDACKn]

puts "Constrained Power Management Generic"

if {${rm_core_sel} == "CORTEX_M0PLUS"} {

set_input_delay  -clock VCLK  -max $cycle50 [get_ports SYSRETAINn]
set_input_delay  -clock VCLK  -min 0.0      [get_ports SYSRETAINn]
set_input_delay  -clock VCLK  -max $cycle50 [get_ports SYSISOLATEn]
set_input_delay  -clock VCLK  -min 0.0      [get_ports SYSISOLATEn]
set_input_delay  -clock VCLK  -max $cycle50 [get_ports SYSPWRDOWN]
set_input_delay  -clock VCLK  -min 0.0      [get_ports SYSPWRDOWN]
set_output_delay -clock VCLK  -max $cycle50 [get_ports SYSPWRDOWNACK]
set_output_delay -clock VCLK  -min 0.0      [get_ports SYSPWRDOWNACK]
set_input_delay  -clock VCLK  -max $cycle50 [get_ports DBGISOLATEn]
set_input_delay  -clock VCLK  -min 0.0      [get_ports DBGISOLATEn]
set_input_delay  -clock VCLK  -max $cycle50 [get_ports DBGPWRDOWN]
set_input_delay  -clock VCLK  -min 0.0      [get_ports DBGPWRDOWN]
set_output_delay -clock VCLK  -max $cycle50 [get_ports DBGPWRDOWNACK]
set_output_delay -clock VCLK  -min 0.0      [get_ports DBGPWRDOWNACK]

puts "Constrained Power Management Cortex-M0+"

# -----------------------------------------------------------------------------------
# Set Multicycle Paths On Static Signals
# -----------------------------------------------------------------------------------

set_multicycle_path 2 -setup -from [get_pins u_cm0pmtbintegration/STCALIB] -end
set_multicycle_path 1 -hold  -from [get_pins u_cm0pmtbintegration/STCALIB] -end

set_multicycle_path 2 -setup -from [get_pins u_cm0pmtbintegration/ECOREVNUM] -end
set_multicycle_path 1 -hold  -from [get_pins u_cm0pmtbintegration/ECOREVNUM] -end

puts "Constrained Multi-cycle paths Cortex-M0+"
} else {
set_multicycle_path 2 -setup -from [get_pins u_cortex_m0_integration/STCALIB] -end
set_multicycle_path 1 -hold  -from [get_pins u_cortex_m0_integration/STCALIB] -end

set_multicycle_path 2 -setup -from [get_pins u_cortex_m0_integration/ECOREVNUM] -end
set_multicycle_path 1 -hold  -from [get_pins u_cortex_m0_integration/ECOREVNUM] -end

puts "Constrained Multi-cycle paths Cortex-M0"
}
}

if { ${rm_include_dbg} } {

set_output_delay -clock SVCLK -max $cycle60 [get_ports CDBGPWRUPREQ]
set_output_delay -clock SVCLK -min 0.0      [get_ports CDBGPWRUPREQ]
set_input_delay  -clock VCLK  -max $cycle30 [get_ports CDBGPWRUPACK]
set_input_delay  -clock VCLK  -min 0.0      [get_ports CDBGPWRUPACK]

}

# Source any timing exceptions
#source ../scripts/your_design_exceptions.tcl

# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Define Signal Constraints
# ------------------------------------------------------------------------------
