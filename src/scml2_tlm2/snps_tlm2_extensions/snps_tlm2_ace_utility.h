/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
/*******************************************************************************
 * Name : snoop_trans.h
 *
 * Description : Provides common defines for ACE. Currently, it provides mapping
 *               from various transaction attributes to their corresponding ACE
 *               transaction types. 
 *               GET_DECLARE_FTAXI_SNOOP_TRANS_TYPE(cmd, domain, snoop_attr) returns the snoop
 *               transaction type for it.
 *
 *               NOTE: Combination of all three fields is needed to determine
 *               the correct value because there are overlapping bit encodings
 ******************************************************************************/
#ifndef __SNPS_TLM2_ACE_UTILITY_H__
#define __SNPS_TLM2_ACE_UTILITY_H__

namespace scml2 {


//returns snoop transaction corresponding to signals    
// Barrier is anded with 0x1 because BAR[0] act as dont care bit in snoop
// type determination
#define GET_FT_SNOOP_TRANS_FRM_SIG(_cmd, _barrier, _domain, _snoop_attr)       \
    scml2::rtl_2_ftaxi_snoop_map [ ((_cmd)         << scml2::COMMAND_START_BIT)  |  \
                        ((_barrier & 0x1)     << scml2::BARRIER_START_BIT)  |  \
                        ((_domain)      << scml2::DOMAIN_START_BIT)   |  \
                        ((_snoop_attr)  << scml2::SNOOP_START_BIT)];    


//declares a mapping from snoop signals to a snoop transaction    
// Barrier is anded with 0x1 because BAR[0] act as dont care bit in snoop
// type determination
#define MAP_SIG_2_FT_SNOOP_TRANS(_cmd, _barrier, _domain, _snoop_val, _snoop_trans)       \
{                                                                            \
   int _idx = ((_cmd)         << scml2::COMMAND_START_BIT)   |                      \
              ((_barrier & 0x1)     << scml2::BARRIER_START_BIT)   |                      \
              ((_domain)      << scml2::DOMAIN_START_BIT)    |                      \
              ((_snoop_val)   << scml2::SNOOP_START_BIT);                           \
                                                                             \
   scml2::rtl_2_ftaxi_snoop_map[_idx] = _snoop_trans;                          \
}



//returns snoop signal corresponding to snoop transaction
#define GET_SIG_FRM_FT_SNOOP_TRANS(_snoop_trans)  \
    scml2::ftaxi_2_rtl_snoop_map[(_snoop_trans)];                

//declares a mapping from snoop transaction to snoop signal
#define MAP_FT_SNOOP_TRANS_TO_SIG(_snoop_trans, _signal_val) \
    scml2::ftaxi_2_rtl_snoop_map [(_snoop_trans)] = (_signal_val);

typedef enum {
    SNOOP_START_BIT   = 0,
    SNOOP_END_BIT     = SNOOP_START_BIT + 3,
    DOMAIN_START_BIT  = SNOOP_END_BIT + 1,
    DOMAIN_END_BIT    = DOMAIN_START_BIT + 1,
    BARRIER_START_BIT = DOMAIN_END_BIT + 1,
    BARRIER_END_BIT   = BARRIER_START_BIT + 1,
    COMMAND_START_BIT = BARRIER_END_BIT + 1,
    COMMAND_END_BIT   = COMMAND_START_BIT,

    NUM_SNOOP_TRANS   = (1 << (COMMAND_END_BIT + 1)) 
} snoop_bitmap_desc_e;


//Global definition for converting trans attribute bitmap to symbolic trans type
static axi_snoop_enum rtl_2_ftaxi_snoop_map[NUM_SNOOP_TRANS];
static unsigned int   ftaxi_2_rtl_snoop_map[axiMaxSnoopTypes];

/*******************************************************************************
 * Name : init_signal_2_snoop_trans_map
 *
 * Description: Global function to populate lookup table to convert trans
 *              attribute bit map to symbolic name.
 ******************************************************************************/
static bool init_signal_2_snoop_trans_map() 
{

    for (unsigned int i = 0; i < NUM_SNOOP_TRANS; i++) {
        rtl_2_ftaxi_snoop_map[i] = axiInvalidSnoop;
    }
    
    //ARSNOOP encodings (CMD, BAR, ARDOMAIN(1:0), ARSNOOP (3:0), VALUE) 
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiNon_Shareable,   0, axiReadNoSnoop);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiSystem,          0, axiReadNoSnoop);
        
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 0, axiReadOnce);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 0, axiReadOnce);
        
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 1, axiReadShared);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 1, axiReadShared);

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 2, axiReadClean);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 2, axiReadClean);

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 3, axiReadNotSharedDirty);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 3, axiReadNotSharedDirty);

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 7, axiReadUnique);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 7, axiReadUnique);

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 11, axiCleanUnique);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 11, axiCleanUnique);

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 12, axiMakeUnique);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 12, axiMakeUnique);

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiNon_Shareable,   8, axiCleanShared);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 8, axiCleanShared);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 8, axiCleanShared);
         
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiNon_Shareable,   9, axiCleanInvalid);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 9, axiCleanInvalid);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 9, axiCleanInvalid);
       
       
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiNon_Shareable,   13, axiMakeInvalid);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiInner_Shareable, 13, axiMakeInvalid);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiNormal, axiOuter_Shareable, 13, axiMakeInvalid);
    
#if 0
        //no encoding for barrier
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiMemory, axiNon_Shareable,   0, BARRIER);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiMemory, axiInner_Shareable, 0, BARRIER);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiMemory, axiOuter_Shareable, 0, BARRIER);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_READ_COMMAND, axiMemory, axiSystem,          0, BARRIER);
#endif
        
        //AWSNOOP encodings (AWDOMAIN(1:0), AWSNOOP (2:0)) 
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiNon_Shareable,   0, axiWriteNoSnoop);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiSystem,          0, axiWriteNoSnoop);
     
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiInner_Shareable, 0, axiWriteUnique);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiOuter_Shareable, 0, axiWriteUnique);
       
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiInner_Shareable, 1, axiWriteLineUnique);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiOuter_Shareable, 1, axiWriteLineUnique); 
    
    
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiNon_Shareable, 2, axiWriteClean);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiInner_Shareable, 2, axiWriteClean);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiOuter_Shareable, 2, axiWriteClean); 

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiNon_Shareable, 3, axiWriteBack);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiInner_Shareable, 3, axiWriteBack);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiOuter_Shareable, 3, axiWriteBack); 

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiInner_Shareable, 4, axiEvict);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiOuter_Shareable, 4, axiEvict); 

    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiNon_Shareable, 5, axiWriteEvict);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiInner_Shareable, 5, axiWriteEvict);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiNormal, axiOuter_Shareable, 5, axiWriteEvict); 
#if 0
        //no encoding for barrier
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiMemory, axiNon_Shareable,   0, BARRIER);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiMemory, axiInner_Shareable, 0, BARRIER);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiMemory, axiOuter_Shareable, 0, BARRIER);
    MAP_SIG_2_FT_SNOOP_TRANS(tlm::TLM_WRITE_COMMAND, axiMemory, axiSystem,          0, BARRIER);
#endif

    //TODO complete for rest of the transactions

    return true;
}  


static bool init_snoop_trans_2_sig_map () 
{
    for (unsigned int i = 0; i < axiMaxSnoopTypes; i++) {
        ftaxi_2_rtl_snoop_map[i] = -1;
    }

    
    MAP_FT_SNOOP_TRANS_TO_SIG(axiInvalidSnoop,        0);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiReadNoSnoop,        0);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiReadOnce,           0);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiReadShared,         1);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiReadClean,          2);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiReadNotSharedDirty, 3);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiReadUnique,         7);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiCleanUnique,       11);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiMakeUnique,        12);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiCleanShared,        8);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiCleanInvalid,       9);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiMakeInvalid,       13);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiDVM_Complete,      14);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiDVM_Message,        15);
    
    MAP_FT_SNOOP_TRANS_TO_SIG(axiWriteNoSnoop,       0);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiWriteUnique,        0);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiWriteLineUnique,    1);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiWriteClean,         2);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiWriteBack,          3);
    MAP_FT_SNOOP_TRANS_TO_SIG(axiEvict,              4);
    
    //TODO add barrier signals
    return false;
}
    
static volatile bool signal_2_trans_def  = init_signal_2_snoop_trans_map();
static volatile bool trans_2_sig_def     = init_snoop_trans_2_sig_map();
}//namespace scml2

#endif //__SNPS_TLM2_ACE_UTILITY_H__
