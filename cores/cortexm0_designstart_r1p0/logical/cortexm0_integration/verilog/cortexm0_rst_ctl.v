//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2009-2015 ARM Limited or its affiliates.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : $Date: 2009-03-21 16:43:18 +0000 (Sat, 21 Mar 2009) $
//
//      Revision            : $Revision: 104871 $
//
//      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CORTEX-M0 EXAMPLE RESET CONTROLLER
// This module is designed as an example reset controller for the Cortex-M0 
// // processor It takes a global reset that can be asynchronously asserted 
// and generates from it synchronously asserted and deasserted resets based 
// on synchronous reset requests
// This module is intended to interface to the example PMU provided (cortexm0_pmu.v)
// You can modify this module to suit your requirements
//-----------------------------------------------------------------------------

module cortexm0_rst_ctl
  (/*AUTOARG*/
  // Outputs
  PORESETn, HRESETn, DBGRESETn, HRESETREQ, 
  // Inputs
  GLOBALRESETn, FCLK, HCLK, DCLK, SYSRESETREQ, PMUHRESETREQ, 
  PMUDBGRESETREQ, RSTBYPASS, SE
  );

  input  GLOBALRESETn;   // Global asynchronous reset
  input  FCLK;           // Free running clock (connect to FCLK of CORTEXM0INTEGRATION)
  input  HCLK;           // AHB clock (connect to HCLK of CORTEXM0INTEGRATION)
  input  DCLK;           // Debug clock (connect to DCLK of CORTEXM0INTEGRATION)
  input  SYSRESETREQ;    // Synchronous (to HCLK) request for HRESETn from system
  input  PMUHRESETREQ;   // Synchronous (to HCLK) request for HRESETn from PMU
  input  PMUDBGRESETREQ; // Synchronous (to DCLK) request for DBGRESETn from PMU
  input  RSTBYPASS;      // Reset synchroniser bypass (for DFT)
  input  SE;             // Scan Enable (for DFT)

  output PORESETn;       // Connect to PORESETn of CORTEXM0INTEGRATION
  output HRESETn;        // Connect to HRESETn of CORTEXM0INTEGRATION
  output DBGRESETn;      // Connect to DBGRESETn of CORTEXM0INTEGRATION
  output HRESETREQ;      // Synchronous (to FCLK) indication of HRESET request

  // Sample synchronous requests to assert HRESETn
  // Sources:
  // 1 - System (SYSRESETREQ)
  // 2 - PMU    (PMUHRESETREQ)
  wire   h_reset_req_in = SYSRESETREQ | PMUHRESETREQ;
  
  cm0_rst_send_set u_hreset_req
    (.RSTn      (PORESETn),
     .CLK       (FCLK),
     .RSTREQIN  (h_reset_req_in),
     .RSTREQOUT (HRESETREQ)
     );

  // Sample synchronous requests to assert DBGRESETn
  wire   dbg_reset_req_sync;
  
  cm0_rst_send_set u_dbgreset_req
    (.RSTn      (PORESETn),
     .CLK       (FCLK),
     .RSTREQIN  (PMUDBGRESETREQ),
     .RSTREQOUT (dbg_reset_req_sync)
     );
  
  // --------------------
  // Reset synchronisers
  // --------------------
  
  cm0_rst_sync u_poresetn_sync
    (.RSTINn    (GLOBALRESETn),
     .RSTREQ    (1'b0),
     .CLK       (FCLK),
     .SE        (SE),
     .RSTBYPASS (RSTBYPASS),
     .RSTOUTn   (PORESETn)
     );

  cm0_rst_sync u_hresetn_sync
    (.RSTINn    (GLOBALRESETn),
     .RSTREQ    (HRESETREQ),
     .CLK       (HCLK),
     .SE        (SE),
     .RSTBYPASS (RSTBYPASS),
     .RSTOUTn   (HRESETn)
     );

  cm0_rst_sync u_dbgresetn_sync
    (.RSTINn    (GLOBALRESETn),
     .RSTREQ    (dbg_reset_req_sync),
     .CLK       (DCLK),
     .SE        (SE),
     .RSTBYPASS (RSTBYPASS),
     .RSTOUTn   (DBGRESETn)
     );
  
endmodule // cortexm0_rst_ctl



