//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2015 ARM Limited or its affiliates.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : $Date: 2009-03-16 19:48:51 +0000 (Mon, 16 Mar 2009) $
//
//      Revision            : $Revision: 104227 $
//
//      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
//-----------------------------------------------------------------------------

// Purpose : Dummy Cortex-M0 Wake-Up Interrupt Controller
// -----------------------------------------------------------------------------

module cortexm0_wic
   (input         FCLK,
    input         nRESET,
    input         WICLOAD,     // WIC mask load from core
    input         WICCLEAR,    // WIC mask clear from core
    input  [33:0] WICINT,      // Interrupt request from system
    input  [33:0] WICMASK,     // Mask from core
    input         WICENREQ,    // WIC enable request from PMU
    input         WICDSACKn,   // WIC enable ack from core
    output        WAKEUP,      // Wake up request to PMU
    output [33:0] WICSENSE,    //
    output [33:0] WICPEND,     // Pended interrupt request
    output        WICDSREQn,   // WIC enable request to core
    output        WICENACK);   // WIC enable ack to PMU

    assign        WAKEUP     = 1'b0;
    assign        WICSENSE   = 34'h0;
    assign        WICPEND    = 34'h0;
    assign        WICDSREQn  = 1'b1;
    assign        WICENACK   = 1'b0;

endmodule
