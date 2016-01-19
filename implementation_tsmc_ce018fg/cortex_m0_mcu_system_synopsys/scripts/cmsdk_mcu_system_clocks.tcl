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
# Purpose : Synthesis Script - Clocks
#
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Define the clocks in the $rm_project_top
# ------------------------------------------------------------------------------

# Declares the clocks present in the design with period, uncertainty and
# latency information for synthesis:
#
#   Period      - Describes the frequency to be acheieved by synthesis.
#
#   Uncertainty - Describes all parameters that could influence the difference
#                 in clock timing between two related flops. Since jitter is
#                 explicitly mentioned this will include OCV, skew and margin.
#
#   Latency     - Describes the delay in the clock tree from the core clock pin
#                 to the flop clock pin; at this point it is an estimate.
#
foreach clock_name ${common_clock_ports} {
  create_clock -name ${clock_name} -period [expr ${clock_period} - ${clock_period_jitter}] [get_ports ${clock_name} ]
  set_clock_uncertainty -setup [expr ${setup_margin} + $pre_cts_clock_skew_estimate] [get_clocks ${clock_name} ]
  set_clock_uncertainty -hold [expr ${hold_margin} + $pre_cts_clock_skew_estimate] [get_clocks ${clock_name} ]
  set_clock_latency -source -fall -early [expr 0.0 - $clock_dutycycle_jitter] [get_clocks ${clock_name} ]
  set_clock_latency -source -fall -late  [expr 0.0 + $clock_dutycycle_jitter] [get_clocks ${clock_name} ]
  set_clock_latency $pre_cts_clock_latency_estimate [get_clocks ${clock_name} ]

  echo "Defined clock $clock_name"
}

# ------------------------------------------------------------------------------
# Virtual clocks
# ------------------------------------------------------------------------------

create_clock -name VCLK -period [expr ${clock_period} - ${clock_period_jitter}]
set_clock_uncertainty -setup [expr ${setup_margin} + $pre_cts_clock_skew_estimate] [get_clocks {VCLK} ]
set_clock_uncertainty -hold  [expr ${hold_margin} + $pre_cts_clock_skew_estimate] [get_clocks {VCLK} ]
set_clock_latency -source -fall -early [expr 0.0 - $clock_dutycycle_jitter] [get_clocks {VCLK} ]
set_clock_latency -source -fall -late  [expr 0.0 + $clock_dutycycle_jitter] [get_clocks {VCLK} ]
set_clock_latency $pre_cts_clock_latency_estimate [get_clocks {VCLK} ]

echo "Defined clock VCLK"

# ------------------------------------------------------------------------------
# Debug logic related clock
# ------------------------------------------------------------------------------
if {${rm_include_dbg} } {

# Create debug clock
create_clock -name DCLK -period [expr ${clock_period} - ${clock_period_jitter}] [get_ports {DCLK} ]
set_clock_uncertainty -setup [expr ${setup_margin} + $pre_cts_clock_skew_estimate] [get_clocks {DCLK} ]
set_clock_uncertainty -hold [expr ${hold_margin} + $pre_cts_clock_skew_estimate] [get_clocks {DCLK} ]
set_clock_latency -source -fall -early [expr 0.0 - $clock_dutycycle_jitter] [get_clocks {DCLK} ]
set_clock_latency -source -fall -late  [expr 0.0 + $clock_dutycycle_jitter] [get_clocks {DCLK} ]
set_clock_latency $pre_cts_clock_latency_estimate [get_clocks {DCLK} ]
echo "Defined clock DCLK"

# Create JTAG/Serial clock
create_clock -name SWCLKTCK -period [expr (${swclock_period} - ${clock_period_jitter})] [get_ports {SWCLKTCK} ]
set_clock_uncertainty -setup [expr ${setup_margin} + $pre_cts_clock_skew_estimate] [get_clocks {SWCLKTCK} ]
set_clock_uncertainty -hold [expr ${hold_margin} + $pre_cts_clock_skew_estimate] [get_clocks {SWCLKTCK} ]
set_clock_latency -source -fall -early [expr 0.0 - $clock_dutycycle_jitter] [get_clocks {SWCLKTCK} ]
set_clock_latency -source -fall -late  [expr 0.0 + $clock_dutycycle_jitter] [get_clocks {SWCLKTCK} ]
set_clock_latency $pre_cts_clock_latency_estimate [get_clocks {SWCLKTCK} ]
echo "Defined clock SWCLKTCK"

# Create virtual clock for debug interface
create_clock -name SVCLK -period [expr (${swclock_period} - ${clock_period_jitter})]
set_clock_uncertainty -setup [expr ${setup_margin} + $pre_cts_clock_skew_estimate] [get_clocks {SVCLK} ]
set_clock_uncertainty -hold [expr ${hold_margin} + $pre_cts_clock_skew_estimate] [get_clocks {SVCLK} ]
set_clock_latency -source -fall -early [expr 0.0 - $swclock_dutycycle_jitter] [get_clocks {SVCLK} ]
set_clock_latency -source -fall -late  [expr 0.0 + $swclock_dutycycle_jitter] [get_clocks {SVCLK} ]
set_clock_latency $pre_cts_clock_latency_estimate [get_clocks {SVCLK} ]
echo "Defined clock SVCLK"

}


# ------------------------------------------------------------------------------
# End of File
# ------------------------------------------------------------------------------
