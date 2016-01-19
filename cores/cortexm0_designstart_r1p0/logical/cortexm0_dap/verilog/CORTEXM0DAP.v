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
//      Checked In          : $Date: 2009-03-19 14:11:42 +0000 (Thu, 19 Mar 2009) $
//
//      Revision            : $Revision: 104573 $
//
//      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
//-----------------------------------------------------------------------------

module CORTEXM0DAP
  ( //DP Signals
    SWCLKTCK, DPRESETn, nTRST,
    TDI, TDO, nTDOEN, SWDITMS, SWDO, SWDOEN,
    CDBGPWRUPREQ, CDBGPWRUPACK,
    //AP Signals
    DCLK, APRESETn, DEVICEEN,
    SLVADDR, SLVWDATA, SLVTRANS, SLVWRITE, SLVRDATA, SLVREADY, SLVRESP,
    SLVSIZE, BASEADDR,
    //Configuration Pins
    ECOREVNUM,
    // DFT
    SE
  );

// ----------------------------------------------------------------------------
// Port Definitions
// ----------------------------------------------------------------------------
  //DP I/O
  input         SWCLKTCK;     // SW/JTAG clock
  input         DPRESETn;     // Negative sense power-on reset for DP
  input         nTRST;        // JTAG test logic reset signal
  input         TDI;          // JTAG data in
  output        TDO;          // JTAG data out
  output        nTDOEN;       // JTAG TDO Output Enable
  input         SWDITMS;      // SW data in/JTAG TMS
  output        SWDO;         // SW data out
  output        SWDOEN;       // SW data out enable
  output        CDBGPWRUPREQ; // System Power Up & Reset Request/Acknowledge
  input         CDBGPWRUPACK; //            "                   "

  //AP I/O
  input         DCLK;         // AP clock
  input         APRESETn;     // Negative sense power-on reset for AP
  input         DEVICEEN;     // Debug enabled by system
  output [31:0] SLVADDR;      // Bus address
  output [31:0] SLVWDATA;     // Bus write data
  output  [1:0] SLVTRANS;     // Bus transfer valid
  output        SLVWRITE;     // Bus write/not read
  output  [1:0] SLVSIZE;      // Bus Access Size
  input  [31:0] SLVRDATA;     // Bus read data
  input         SLVREADY;     // Bus Ready from bus
  input         SLVRESP;      // Bus Response from bus
  input  [31:0] BASEADDR;     // AP ROM Table Base Value (to be tied externally)

  //Configuration IO
  input   [7:0] ECOREVNUM;    // Top 4 bits = DP Revision, Bottom 4 = AP Revision

  // DFT
  input         SE;           // Scan enable for DFT
  

  assign        TDO           = 1'b0;
  assign        nTDOEN        = 1'b1;
  assign        SWDO          = 1'b0;
  assign        SWDOEN        = 1'b0;
  assign        CDBGPWRUPREQ  = 1'b0;
  assign        SLVADDR       = 32'h0;
  assign        SLVWDATA      = 32'h0;
  assign        SLVTRANS      = 2'b00;
  assign        SLVWRITE      = 1'b0;
  assign        SLVSIZE       = 2'b00;


endmodule
