/*
 *-----------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from ARM Limited.
 *
 *            (C) COPYRIGHT 2010-2015  ARM Limited or its affiliates.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from ARM Limited.
 *
 *      SVN Information
 *
 *      Checked In          : $Date: 2013-03-27 23:58:01 +0000 (Wed, 27 Mar 2013) $
 *
 *      Revision            : $Revision: 242484 $
 *
 *      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
 *-----------------------------------------------------------------------------
 */

#define TEST_PASS 0
#define TEST_FAIL 1
//==============================================================================
// Cortex-M0+ & Cortex-M0 IDs header File
//==============================================================================

// Values used by generic tests, such as debug_tests, romtable_tests

#define MCU_CPU_NAME "Cortex-M0"
#define MCU_CPU_ID_VALUE  0x410cc200
#define MCU_DP_IDR_VALUE  0x0BB11477
#define MCU_AP_IDR_VALUE  0x04770021
#define MCU_AP_BASE_VALUE 0xE00FF003

//==============================================================================
//
// GenericID values - DO NOT MODIFY
//

// CoreSight Component Identifier for Peripheral classes
#define CORESIGHT_CID0          0x0D
#define CORESIGHT_CID1_tbl      0x10
#define CORESIGHT_CID1_dbg      0x90
#define CORESIGHT_CID1_mem      0xE0
#define CORESIGHT_CID1_prm      0xF0
#define CORESIGHT_CID2          0x05
#define CORESIGHT_CID3          0xB1

// Component Part Numbers
#define ARM_JEP_ID              0x3B
#define ARM_JEP_CONT            0x4

//==============================================================================
//
// Cortex-M0 ID values
#define CORTEXM0_CPUID          0x410cc200

#define CM0_CPU_PART            0x471
#define CM0_SCS_PART            0x008
#define CM0_DWT_PART            0x00A
#define CM0_BPU_PART            0x00B
#define CM0_CPU_PART            0x471
#define CM0_CSI_PART            0x4C2
#define CS_CTI_PART             0x906
#define CM0_DAP_DP_PART         0xBB

// Revision Codes
#define CM0_ROM_REV             0x0
#define CM0_SCS_REV             0x0
#define CM0_DWT_REV             0x0
#define CM0_BPU_REV             0x0

// Revand
#define CM0_ROM_ECO             0x0
#define CM0_SCS_ECO             0x0
#define CM0_DWT_ECO             0x0
#define CM0_BPU_ECO             0x0

#define CORTEXM0DAP_JTAG_TAPID  0x0BA01477
#define CORTEXM0DAP_JTAG_DPIDR  0x0BB11477
#define CORTEXM0DAP_SW_DPIDR    0x0BB11477
#define CORTEXM0DAP_AP_IDR      0x04770021

#define CORTEXM0_ROM_PID3       0x00
#define CORTEXM0_SCS_PID3       0x00
#define CORTEXM0_DWT_PID3       0x00
#define CORTEXM0_BPU_PID3       0x00

//==============================================================================
//
// Cortex-M0+ ID values
#define CORTEXM0PLUS_CPUID      (0x410cc601 ^ (EXPECTED_ECOREVNUM & 0x0000000F))

#define CM0P_CPU_PART           0x4C0
#define CM0P_SCS_PART           0x008
#define CM0P_DWT_PART           0x00A
#define CM0P_BPU_PART           0x00B
#define CM0P_MTB_PART           0x932
#define CM0P_CSI_PART           0x4C1
#define CM0P_CTI_PART           0x9A6
#define CM0P_DAP_DP_PART        0xBC

// Revision Codes
#define CM0P_ROM_REV            0x0
#define CM0P_SCS_REV            0x0
#define CM0P_DWT_REV            0x0
#define CM0P_BPU_REV            0x0
#define CM0P_MTB_REV            0x1
#define CM0P_DAP_DP_REV         (0x0 ^ ((EXPECTED_ECOREVNUM & 0x0F000000) >> 24))
#define CM0P_DAP_AP_REV         (0x0 ^ ((EXPECTED_ECOREVNUM & 0x00F00000) >> 20))

// Revand
#define CM0P_ROM_ECO            ((EXPECTED_ECOREVNUM & 0x000F0000) >> 16)
#define CM0P_SCS_ECO            ((EXPECTED_ECOREVNUM & 0x0000F000) >> 12)
#define CM0P_DWT_ECO            ((EXPECTED_ECOREVNUM & 0x00000F00) >>  8)
#define CM0P_BPU_ECO            ((EXPECTED_ECOREVNUM & 0x000000F0) >>  4)
#define CM0P_MTB_ECO            ((EXPECTED_ECOREVNUM & 0xF0000000) >> 28)

// DAP Registers
#define CM0P_DAP_JTAG_TAPID     (0x0BA01477 | (CM0P_DAP_DP_REV << 28))
#define CM0P_DAP_JTAG_DPIDR     (0x0BC11477 | (CM0P_DAP_DP_REV << 28))
#define CM0P_DAP_SW_DPIDR       (0x0BC11477 | (CM0P_DAP_DP_REV << 28))
#define CM0P_DAP_SWMD_DPIDR     (0x0BC12477 | (CM0P_DAP_DP_REV << 28))
#define CM0P_DAP_APIDR          (0x04770031 | (CM0P_DAP_AP_REV << 28))

//==============================================================================

//=====================================================================
// Cortex-M0 & Cortex-M0+ Configuration
//=====================================================================

////////////////////////////////////////////////////////////////////////////////
//
//  Processor configuration options.
//  These must match the expected hardware configuration of the processor.
//
//  - EXPECTED_BE            : Expected Endianness (0-1)
//  - EXPECTED_BKPT          : Expected number of Breakpoint Comparators (0-4)
//  - EXPECTED_DBG           : Expected Debug config (0-1)
//  - EXPECTED_NUMIRQ        : Expected number of IRQ lines (0-32)
//  - EXPECTED_SMUL          : Expected Multiplier config (0-1)
//  - EXPECTED_SYST          : Expected SysTick extension (0-1)
//  - EXPECTED_WPT           : Expected number of Watchpoint Comparators (0-2)
//
//  Processor Tie-offs
//
//  - EXPECTED_STCALIB       : Expected value of STCALIB (0-0x3FFFFFF)
//
////////////////////////////////////////////////////////////////////////////////
//
//  System ROM Table options.
//  These must match the values in the System ROM Table.
//
//  - EXPECTED_CUST_JEP_ID   : Expected JEDEC JEP-106 identity code (0-0x7F)
//  - EXPECTED_CUST_JEP_CONT : Expected JEDEC JEP-106 continuation code (0-0xF)
//  - EXPECTED_CUST_PART     : Expected Part number (0-0xFFF)
//  - EXPECTED_CUST_REV      : Expected Revision number (0-0xF)
//  - EXPECTED_CUST_REVAND   : Expected Manufacturer Revision number (0-0xF)
//
////////////////////////////////////////////////////////////////////////////////
//
//  Engineering Change Order and Revision Number Tie-off.
//
//  - EXPECTED_ECOREVNUM     : Expected Engineering Change Order and Revision number (0-0xFFFFFFFF)
//
////////////////////////////////////////////////////////////////////////////////

// <h> Processor configuration options
// <o> EXPECTED_BE: Expected Endianness <0=> Little Endian <1=> Big Endian
#define EXPECTED_BE             0


// <o> EXPECTED_NUMIRQ: Expected number of IRQ lines <0-32>
#define EXPECTED_NUMIRQ         32


// <o> EXPECTED_SMUL: Expected Small Multiplier config <0=> Absent (Fast Multiplier) <1=> Present (Small Multiplier)
#define EXPECTED_SMUL           0


// <o> EXPECTED_SYST: Expected SysTick extension <0=> Absent <1=> Present
#define EXPECTED_SYST           1


// <o> EXPECTED_SIMPLE_CHECK:  <1=> only run the simple CPUID check <0=> more complex check included
// like Lockup, Sleep, other ID check etc
#define EXPECTED_SIMPLE_CHECK   0


// <h> Processor port Tie-Offs
// <o> EXPECTED_STCALIB: Expected value of STCALIB[25:0] at CORTEXM0INTEGRATION or CORTEXM0 <0x0-0x3FFFFFF>
#define EXPECTED_STCALIB        0x1028B0A


// <o> EXPECTED_BASEADDR: Expected value of BASEADDR[31:0] at CORTEXM0DAP <0x0-0xFFFFFFFF>
#define EXPECTED_BASEADDR       0xE00FF003

// <o> EXPECTED_JTAGnSW: Expected Cortex M0(+) DAP Protocol <0=> Serial Wire <1=> JTAG
#define EXPECTED_JTAGnSW        1

// <h> System ROM Table ID values
// <o> EXPECTED_CUST_JEP_ID: Expected JEDEC JEP-106 identity code (0-0x7F)
#define EXPECTED_CUST_JEP_ID    0x00


// <o> EXPECTED_CUST_JEP_CONT: Expected JEDEC JEP-106 continuation code (0-0xF)
#define EXPECTED_CUST_JEP_CONT  0x0


// <o> EXPECTED_CUST_PART: Expected Part number (0-0xFFF)
#define EXPECTED_CUST_PART      0x000


// <o> EXPECTED_CUST_REV: Expected Revision number (0-0xF)
#define EXPECTED_CUST_REV       0x0


// <o> EXPECTED_CUST_REVAND: Expected Manufacturer Revision number (0-0xF)
#define EXPECTED_CUST_REVAND    0x0

// </h>

// <h> EXPECTED_ECOREVNUM
// <o> EXPECTED_ECOREVNUM: Expected Engineering Change Order and Revision number (0-0xFFFFFFFF)
#define EXPECTED_ECOREVNUM      0x00000000

// </h>

//=====================================================================
// Cortex-M0+ Configuration
//=====================================================================

////////////////////////////////////////////////////////////////////////////////
//
//  Processor configuration options.
//  These must match the expected hardware configuration of the processor.
//
//  - EXPECTED_BE            : Expected Endianness (0-1)
//  - EXPECTED_BKPT          : Expected number of Breakpoint Comparators (0-4)
//  - EXPECTED_DBG           : Expected Debug config (0-1)
//  - EXPECTED_IOP           : Expected IOP config (0-1)
//  - EXPECTED_IRQDIS        : Expected IRQ disable (0-0xFFFFFFFF)
//  - EXPECTED_MPU           : Expected MPU config (0,8)
//  - EXPECTED_NUMIRQ        : Expected number of IRQ lines (0-32)
//  - EXPECTED_SMUL          : Expected Multiplier config (0-1)
//  - EXPECTED_SYST          : Expected SysTick extension (0-1)
//  - EXPECTED_USER          : Expected User config (0-1)
//  - EXPECTED_VTOR          : Expected VTOR config (0-1)
//  - EXPECTED_WIC           : Expected Wake-up interrupt controller support <0-1>
//  - EXPECTED_WICLINES      : Expected number of supported WIC lines <2-34>
//  - EXPECTED_WPT           : Expected number of Watchpoint Comparators (0-2)
//
//  Processor Tie-offs
//
//  - EXPECTED_STCALIB       : Expected value of STCALIB (0-0x3FFFFFF)
//
////////////////////////////////////////////////////////////////////////////////
//
//  DAP configuration options.
//  These must match the expected hardware configuration of the DAP.
//
//  - EXPECTED_BASEADDR      : Expected BASEADDR at Cortex M0+ DAP (0-0xFFFFFFFF)
//  - EXPECTED_JTAGnSW       : Expected Cortex M0+ DAP Protocol (0,1)
//  - EXPECTED_SWMD          : Expected SW Multi-Drop config (0-1)
//
//  DAP Tie-offs
//
//  - EXPECTED_INSTANCEID    : Expected DAP instance number (0-0xF)
//  - EXPECTED_TREVISION     : Expected Revision number (0-0xF)
//  - EXPECTED_TPARTNO       : Expected Part number (0-0xFFFF)
//  - EXPECTED_TDESIGNER     : Expected Designer code (0-0x7FF)
//
////////////////////////////////////////////////////////////////////////////////
//
//  CoreSight MTB-M0+ configuration options.
//
//  - EXPECTED_MTB           : Expected CoreSight MTB-M0+ config (0-1)
//  - EXPECTED_MTB_BASEADDR  : Expected CoreSight MTB-M0+ BASEADDR (0-0xFFFFFFFF)
//  - EXPECTED_MTB_AWIDTH    : Expected CoreSight MTB-M0+ Address width (5-32)
//
// NOTE : When the MTB (EXPECTED_MTB = 1) is included, the RAM that is used for
//        the data accesses is shared with the MTB, so if you set the address
//        width to be smaller than the default (EXPECTED_MTB_AWIDTH = 16), the
//        addresses that some tests are using to write data to will alias to
//        lower addresses,so some tests will not work with smaller memory.
//
////////////////////////////////////////////////////////////////////////////////
//
//  System ROM Table options.
//  These must match the values in the System ROM Table.
//
//  - EXPECTED_CUST_JEP_ID   : Expected JEDEC JEP-106 identity code (0-0x7F)
//  - EXPECTED_CUST_JEP_CONT : Expected JEDEC JEP-106 continuation code (0-0xF)
//  - EXPECTED_CUST_PART     : Expected Part number (0-0xFFF)
//  - EXPECTED_CUST_REV      : Expected Revision number (0-0xF)
//  - EXPECTED_CUST_REVAND   : Expected Manufacturer Revision number (0-0xF)
//
////////////////////////////////////////////////////////////////////////////////
//
//  Engineering Change Order and Revision Number Tie-off.
//
//  - EXPECTED_ECOREVNUM     : Expected Engineering Change Order and Revision number (0-0xFFFFFFFF)
//
////////////////////////////////////////////////////////////////////////////////

// <h> Processor configuration options
// <o> EXPECTED_IRQDIS: Expected interrupt disable <0x0-0xFFFFFFFF>
#define EXPECTED_IRQDIS         0x00000000


// <o> EXPECTED_WIC: Expected Wake-up interrupt controller support <0-1>
#define EXPECTED_WIC            0


// <o> EXPECTED_WICLINES : Expected number of supported WIC lines <2-34>
#define EXPECTED_WICLINES       34


// <h> Processor port Tie-Offs
// <o> EXPECTED_SWMD: Expected SW Multi-Drop config <0=> Absent <1=> Present (Serial Wire Protocol 2)
#define EXPECTED_SWMD           0

// </h>

// <h> DAP Tie-offs
// <o> EXPECTED_INSTANCEID: Expected DAP instance number (0-0xF)
#define EXPECTED_INSTANCEID     0x0


// <o> EXPECTED_TREVISION: Expected Revision number (0-0xF)
#define EXPECTED_TREVISION      0x0


// <o> EXPECTED_TPARTNO: Expected Part number (0-0xFFFF)
#define EXPECTED_TPARTNO        0x0000


// <o> EXPECTED_TDESIGNER: Expected Designer code (0-0x7FF)
#define EXPECTED_TDESIGNER      0x000

// </h>

// <<< end of configuration section >>>
