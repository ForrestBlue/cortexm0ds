# -----------------------------------------------------------------------------
#  The confidential and proprietary information contained in this file may
#  only be used by a person authorised under and to the extent permitted
#  by a subsisting licensing agreement from ARM Limited.
#
#             (C) COPYRIGHT 2012-2015  ARM Limited or its affiliates.
#                 ALL RIGHTS RESERVED
#
#  This entire notice must be reproduced on all copies of this file
#  and copies of this file may only be made by a person if such person is
#  permitted to do so under the terms of a subsisting license agreement
#  from ARM Limited.
#
#       SVN Information
#
#       Checked In          : $Date: 2013-04-10 14:47:58 +0100 (Wed, 10 Apr 2013) $
#
#       Revision            : $Revision: 243487 $
#
#       Release Information : Cortex-M0 DesignStart-r1p0-00rel0
#
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Abstract: GNU Makefile for reference implementation scripts
# -----------------------------------------------------------------------------
#
# Notes:
# Ensure appropriate EDA tools are on search path before using make targets
# Refer to supplied documentation for required EDA tools and versions
#
# -----------------------------------------------------------------------------

# Dummy target to prevent 'make' without options running other targets:
intro:
	@echo "Refer to supplied documentation for reference implementation"
	@echo
	@echo "Make targets are available for:"
	@echo "synthesis, dft"
	@echo
	@echo "Make targets included combining the above: front"
	@echo


.PHONY: clean clean_all dist_clean

#####################################
# CLEAN                             #
#####################################

clean clean_all dist_clean:
	@echo "Cleaning implementation build directory ....."
	@/bin/rm -rf work/*
	@/bin/rm -rf logs/*
	@/bin/rm -rf data/*
	@/bin/rm -rf lec/*
	@/bin/rm -rf reports/synthesis/*
	@/bin/rm -rf reports/dft/*
	@/bin/rm -rf reports/lec/*
	@/bin/rm -rf reports/*/* # Catch any remaining reports

#####################################
# FRONT                             #
#####################################

synthesis:
	@mkdir -p work
	@mkdir -p logs  
	@mkdir -p reports  
	@mkdir -p reports/dft  
	@mkdir -p reports/lec  
	@mkdir -p reports/synthesis
	@mkdir -p data
	cd work; (time dc_shell-xg-t -64bit -topo -f ../scripts/cmsdk_mcu_system_syn.tcl ) 2>&1 | tee ../logs/$@.log

dft: #dependency is: synthesis
	cd work; (time dc_shell-xg-t -64bit -topo -f ../scripts/cmsdk_mcu_system_dft.tcl ) 2>&1 | tee ../logs/$@.log

#####################################
# LOGICAL EQUIVALENCE               #
#####################################

lec_synthesis: #dependency is: synthesis
	cd work; (time fm_shell -64bit -x "set netlist cmsdk_mcu_system.synthesis" -overwrite -name_suffix "cmsdk_mcu_system.synthesis" -f ../scripts/cmsdk_mcu_system_fm.tcl ) 2>&1 | tee ../logs/$@.log
	cd work; mv formality_cmsdk_mcu_system.synthesis.log ../reports/lec/cmsdk_mcu_system.synthesis.log

lec_dft: #dependency is: dft
	cd work; (time fm_shell -64bit -x "set netlist cmsdk_mcu_system.dft" -overwrite -name_suffix "cmsdk_mcu_system.dft" -f ../scripts/cmsdk_mcu_system_fm.tcl ) 2>&1 | tee ../logs/$@.log
	cd work; mv formality_cmsdk_mcu_system.dft.log ../reports/lec/cmsdk_mcu_system.dft.log

#####################################
# COMBINED TARGETS                  #
#####################################

front: synthesis dft
analysis: lec_synthesis lec_dft

all: front analysis
