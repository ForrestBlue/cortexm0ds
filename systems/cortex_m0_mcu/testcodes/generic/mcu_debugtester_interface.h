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

//
// Definition of CMSDK example MCU to the Debug Tester
//
#include "CMSDK_CM0.h"
#include <stdio.h>
#include "uart_stdout.h"   // for stdout
#include "CMSDK_driver.h"
#include "config_id.h" // general defines such as test_pass


// Functions used by tests that communicate with the Debug Tester
extern void EnableDebugTester(void);
extern void DisableDebugTester(void);
extern uint32_t CallDebugTester(uint32_t);
extern void StartDebugTester(uint32_t);
extern uint32_t CheckDebugTester(void);


//Test command sequence definition
#define  DBG_ESCAPE          0x1B
#define  DBG_CONNECT_ENABLE  0x11
#define  DBG_CONNECT_DISABLE 0x12
#define  DBG_SIM_STOP        0x4


// GPIO0 bit allocation
//
// CM0_MCU GPIO0 --------------------------------------  Debug Tester
//
// GPIO[15] 7 <----------------------------------------< Running
// GPIO[14] 6 <----------------------------------------< Error
// GPIO[13] 5 >----------------------------------------> Function Strobe
// GPIO[12] 4 >----------------------------------------> Function Select bit 4
// GPIO[11] 3 >----------------------------------------> Function Select bit 3
// GPIO[10] 2 >----------------------------------------> Function Select bit 2
// GPIO[9]  1 >----------------------------------------> Function Select bit 1
// GPIO[8]  0 >----------------------------------------> Function Select bit 0
//
#define   DEBUG_BIT_LOC            8  //GPIO[8] is the least bit of Function Select
#define   DEBUG_CMD                0x3f<<8            //GPIO [13:8]
#define   DEBUG_STROBE             0x00002000         //GPIO [13]
#define   DEBUG_ERROR              0x00004000         //GPIO [14]
#define   DEBUG_RUNNING            0x00008000         //GPIO [15]

// GPIO1[7] 7
// GPIO1[6] 6
// GPIO1[5] 5
// GPIO1[4] 4
// GPIO1[3] 3
// GPIO1[2] 2
// GPIO1[1] 1
// GPIO1[0] 0


// CMSDK example MCU's view of the memory shared with the debugtester
// (4 words above stack top)
// This macro uses the SP value from the vector table as stacktop
// The stacktop cannot be set to the top of the memory.
#define DEBUGTESTERDATA ((volatile uint32_t *) *((uint32_t *) 0x0))

