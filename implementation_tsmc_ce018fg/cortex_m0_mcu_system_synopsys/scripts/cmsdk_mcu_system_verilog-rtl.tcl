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
# Purpose :  Synthesis Script - Verilog
#
# ------------------------------------------------------------------------------

# ----------------------------------------------------------------------------------
# Add System RTL search path
# ----------------------------------------------------------------------------------

# path for verilog files
set proj_verilog    ../../..
# add you memory path here
set syn_mem_lib     ../../..


# Path for Cortex-M0
set cortexm0_verilog    ../../../cores/cortexm0_designstart_r1p0/logical

# Path for Cortex-M0+
set cortexm0p_logical_verilog  ../../../cores/at590_cortexm0p_r0p1/logical
set cortexm0p_ik_verilog       ../../../cores/at590_cortexm0p_r0p1/integration_kit/logical

set search_path [ concat $search_path $proj_verilog/logical/cmsdk_ahb_gpio/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_apb_timer/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_apb_dualtimers/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_apb_watchdog/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_apb_uart/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_ahb_default_slave/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_ahb_slave_mux/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_ahb_to_apb/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_apb_slave_mux/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_apb_subsystem/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/cmsdk_iop_gpio/verilog ]
set search_path [ concat $search_path $proj_verilog/logical/models/memories ]
set search_path [ concat $search_path $proj_verilog/systems/cortex_m0_mcu/verilog ]
set search_path [ concat $search_path $syn_mem_lib ]

# Replace or remove the following path which references the instantiated
# clock-gating cells, etc.

if {${rm_core_sel} == "CORTEX_M0PLUS"} {
set search_path [ concat $search_path $cortexm0p_logical_verilog/models/cells/generic ]
set search_path [ concat $search_path $cortexm0p_logical_verilog/models/wrappers ]
} else {
set search_path [ concat $search_path $cortexm0_verilog/models/cells ]
}

# Library clock gating cells
set search_path [ concat $search_path $proj_verilog/logical/models/clkgate ]

if {${rm_include_dma}} {
set search_path [ concat $search_path $proj_verilog/logical/pl230_udma/verilog ]
}

# Path for IO Port GPIO (for Cortex-M0+ only)
if {${rm_core_sel} == "CORTEX_M0PLUS"} {
set search_path [ concat $search_path $proj_verilog/logical/iop_gpio/verilog ]
}

# ----------------------------------------------------------------------------------
# Add Processor RTL search path
# ----------------------------------------------------------------------------------

if {${rm_core_sel} == "CORTEX_M0PLUS"} {
set search_path [ concat $search_path $cortexm0p_logical_verilog/cm0p_integration/verilog ]
set search_path [ concat $search_path $cortexm0p_logical_verilog/cortexm0plus/verilog ]
set search_path [ concat $search_path $cortexm0p_logical_verilog/cm0p_dap/verilog ]

set search_path [ concat $search_path $cortexm0p_ik_verilog/cm0p_ik_mcu/verilog ]
set search_path [ concat $search_path $cortexm0p_ik_verilog/tbench/verilog ]

if {${rm_include_mtb} } {
set search_path [ concat $search_path $cortexm0p_logical_verilog/cm0p_mtb/verilog ]
}

} else {

if {${rm_design_start}} {
set search_path [ concat $search_path $cortexm0_verilog/cortexm0ds/verilog ]
set search_path [ concat $search_path $cortexm0_verilog/cortexm0_integration/verilog ]
set search_path [ concat $search_path $cortexm0_verilog/cortexm0_dap/verilog ]
set search_path [ concat $search_path $cortexm0_verilog/models/cells ]
} else {
set search_path [ concat $search_path $cortexm0_verilog/cortexm0_integration/verilog ]
set search_path [ concat $search_path $cortexm0_verilog/cortexm0/verilog ]
set search_path [ concat $search_path $cortexm0_verilog/cortexm0_dap/verilog ]
}
}

# -----------------------------------------------------------------------------------
# RTL for Cortex-M0 processor
# -----------------------------------------------------------------------------------
#            cortexm0_rst_ctl.v \
#            cm0_dbg_reset_sync.v \ 

if {${rm_design_start}} {
set cortexm0_ds [ list \
            CORTEXM0DS.v \
            cortexm0ds_logic.v \
            ]

set cortexm0_integration [ list \
            CORTEXM0INTEGRATION.v \
            CORTEXM0DAP.v \
            cortexm0_wic.v ]

set cortexm0_cells [ list \
            cm0_dbg_reset_sync.v \
            ]

} else {
set cortexm0_cells [ list \
            cm0_acg.v \
            cm0_dap_cdc_capt_sync.v \
            cm0_dap_cdc_comb_and_addr.v \
            cm0_dap_cdc_comb_and_data.v \
            cm0_dap_cdc_comb_and.v \
            cm0_dap_cdc_send.v \
            cm0_dap_cdc_send_addr.v \
            cm0_dap_cdc_send_data.v \
            cm0_dap_cdc_send_reset.v \
            cm0_dap_jt_cdc_comb_and.v \
            cm0_dap_sw_cdc_capt_reset.v \
            cm0_dbg_reset_sync.v \
            cm0_pmu_acg.v \
            cm0_pmu_cdc_send_reset.v \
            cm0_pmu_cdc_send_set.v \
            cm0_pmu_sync_reset.v \
            cm0_pmu_sync_set.v \
            ]

set cortexm0_integration [ list \
            CORTEXM0INTEGRATION.v \
            CORTEXM0DAP.v \
            cortexm0_pmu.v \
            cortexm0_wic.v ]

set cortexm0 [ list \
            CORTEXM0.v \
            cm0_top.v \
            cm0_matrix.v \
            cm0_matrix_sel.v \
            cm0_top_sys.v \
            cm0_top_clk.v \
            cm0_core.v \
            cm0_core_alu.v \
            cm0_core_ctl.v \
            cm0_core_dec.v \
            cm0_core_gpr.v \
            cm0_core_mul.v \
            cm0_core_pfu.v \
            cm0_core_psr.v \
            cm0_core_spu.v \
            cm0_nvic.v \
            cm0_nvic_reg.v \
            cm0_nvic_main.v ]

set cortexm0_dbg [ list \
            cm0_top_dbg.v \
            cm0_dbg_bpu.v \
            cm0_dbg_ctl.v \
            cm0_dbg_dwt.v \
            cm0_dbg_if.v \
            cm0_dbg_sel.v ]

set cortexm0_dap [ list \
            CORTEXM0DAP.v \
            cm0_dap_dp_pwr.v \
            cm0_dap_dp_cdc.v \
            cm0_dap_dp.v \
            cm0_dap_ap_mast.v \
            cm0_dap_ap_cdc.v \
            cm0_dap_ap.v \
            cm0_dap_dp_jtag.v \
            cm0_dap_dp_sw.v ]
}

# -----------------------------------------------------------------------------------
# RTL for Cortex-M0 processor
# -----------------------------------------------------------------------------------

set cm0p_cells [ list \
                   cm0p_acg.v \
                   cm0p_dap_cdc_capt_sync.v \
                   cm0p_dap_cdc_comb_and.v \
                   cm0p_dap_cdc_comb_and_addr.v \
                   cm0p_dap_cdc_comb_and_data.v \
                   cm0p_dap_cdc_send.v \
                   cm0p_dap_cdc_send_addr.v \
                   cm0p_dap_cdc_send_data.v \
                   cm0p_dap_cdc_send_reset.v \
                   cm0p_dap_jt_cdc_comb_and.v \
                   cm0p_dap_sw_cdc_capt_reset.v \
                   cm0p_dap_sw_cdc_capt_sync.v \
                   cm0p_dbg_reset_sync.v ]

set cm0pmtb_integration [ list \
                   CM0PMTBINTEGRATION.v \
                   CM0PINTEGRATION.v  \
                   cm0p_wic.v ]

set cortexm0plus [ list \
                   CORTEXM0PLUS.v \
                   CORTEXM0PLUSIMP.v \
                   cm0p_core.v \
                   cm0p_matrix.v \
                   cm0p_matrix_sel.v \
                   cm0p_mpu.v \
                   cm0p_nvic.v \
                   cm0p_top.v \
                   cm0p_top_clk.v \
                   cm0p_top_sys.v ]

set cm0p_dbg [ list \
                   cm0p_top_dbg.v \
                   cm0p_dbg_bpu.v \
                   cm0p_dbg_ctl.v \
                   cm0p_dbg_dwt.v \
                   cm0p_dbg_if.v \
                   cm0p_dbg_sel.v ]

set cm0p_dap [ list \
                   CM0PDAP.v \
                   cm0p_dap_ap.v \
                   cm0p_dap_ap_cdc.v \
                   cm0p_dap_ap_mast.v \
                   cm0p_dap_dp.v \
                   cm0p_dap_dp_cdc.v \
                   cm0p_dap_dp_jtag.v \
                   cm0p_dap_dp_pwr.v \
                   cm0p_dap_dp_sw.v \
                   cm0p_dap_top.v ]

# -----------------------------------------------------------------------------------
#  RTL for CoreSight MTB M0+
# -----------------------------------------------------------------------------------

if {${rm_include_mtb}} {
set search_path [ concat $search_path $cortexm0p_logical_verilog/cm0p_mtb/verilog ]
set cm0p_mtb [list \
            CM0PMTB.v \
            cm0p_mtb_sram_bridge.v \
            cm0p_mtb_top.v \
            cm0p_mtb_trace.v \
            cmsdk_mtb_sync.v \
            ]
}

# -----------------------------------------------------------------------------------
#  RTL for DMA  PL230
# -----------------------------------------------------------------------------------

if {${rm_include_dma}} {
set pl230_udma_files [list \
            pl230_ahb_ctrl.v \
            pl230_apb_regs.v \
            pl230_dma_data.v \
            pl230_udma.v \
            ]
} else {
set pl230_udma_files [list ]
}

# -----------------------------------------------------------------------------------
#  RTL for system
# -----------------------------------------------------------------------------------

if {${rm_include_iop}} {
set cmsdk_sys [list \
            cmsdk_apb_dualtimers.v \
            cmsdk_apb_dualtimers_frc.v \
            cmsdk_apb_watchdog.v \
            cmsdk_apb_watchdog_frc.v \
            cmsdk_iop_gpio.v \
            cmsdk_iop_interconnect.v \
            cmsdk_ahb_slave_mux.v \
            cmsdk_ahb_to_apb.v \
            cmsdk_apb_slave_mux.v \
            cmsdk_apb_subsystem.v \
            cmsdk_apb_test_slave.v \
            cmsdk_irq_sync.v \
            cmsdk_apb_timer.v \
            cmsdk_apb_uart.v \
            cmsdk_ahb_default_slave.v \
            cmsdk_ahb_cs_rom_table.v \
            cmsdk_mcu_sysctrl.v \
            cmsdk_mcu_addr_decode.v \
            cmsdk_mcu_system.v \
            cmsdk_mcu_stclkctrl.v \
            ]
} else {
set cmsdk_sys [list \
            cmsdk_apb_dualtimers.v \
            cmsdk_apb_dualtimers_frc.v \
            cmsdk_apb_watchdog.v \
            cmsdk_apb_watchdog_frc.v \
            cmsdk_ahb_to_iop.v \
            cmsdk_iop_gpio.v \
            cmsdk_ahb_gpio.v \
            cmsdk_ahb_slave_mux.v \
            cmsdk_ahb_to_apb.v \
            cmsdk_apb_slave_mux.v \
            cmsdk_apb_subsystem.v \
            cmsdk_apb_test_slave.v \
            cmsdk_irq_sync.v \
            cmsdk_apb_timer.v \
            cmsdk_apb_uart.v \
            cmsdk_ahb_default_slave.v \
            cmsdk_ahb_cs_rom_table.v \
            cmsdk_mcu_sysctrl.v \
            cmsdk_mcu_addr_decode.v \
            cmsdk_mcu_system.v \
            cmsdk_mcu_stclkctrl.v \
            ]
}

# MCU level not synthesised here (memories, clock control, reset generator]
set cmsdk_mcu [list \
            cmsdk_ahb_rom.v \
            cmsdk_ahb_ram.v \
            cmsdk_mcu_clkctrl.v \
            cmsdk_mcu.v \
            cmsdk_mcu_pin_mux.v \
           ]


set rtl_image [ concat $pl230_udma_files $cmsdk_sys]

if {${rm_core_sel} == "CORTEX_M0PLUS"} {

set rtl_image [ concat $rtl_image $cm0pmtb_integration $cm0p_cells $cortexm0plus $cm0p_dbg $cm0p_dap]

if {${rm_include_mtb}} {
set rtl_image [ concat $rtl_image $cm0p_mtb]
}

} else {

if {${rm_design_start} } {
set rtl_image [ concat $rtl_image $cortexm0_ds $cortexm0_integration $cortexm0_cells ]
} else {
set rtl_image [ concat $rtl_image $cortexm0_integration $cortexm0_cells $cortexm0 $cortexm0_dbg $cortexm0_dap]
}

}


# ------------------------------------------------------------------------------
# End of File
# ------------------------------------------------------------------------------
