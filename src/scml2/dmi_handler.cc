/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2/dmi_handler.h>
#include <scml2/abstraction_level_switch.h>

#include <scml2_logging/severity.h>

namespace scml2 {

bool 
dmi_handler::sPreVReleaseTlmIgnoreBehavior = 
  (::getenv("SNPS_ENABLE_TLM_IGNORE_COMMAND") == nullptr);

logging::stream
dmi_handler::mError("dmi_handler", logging::severity::error());

dmi_handler::
dmi_handler() :
  mFwDirectMemIf(0),
  mDmiEnabled(true)
{
  abstraction_level_switch::get_instance().register_observer(this);
  invalidate(mReadRanges);
  invalidate(mWriteRanges);
}

dmi_handler::
~dmi_handler()
{
  abstraction_level_switch::get_instance().unregister_observer(this);
}

void
dmi_handler::
set_interface(tlm::tlm_fw_direct_mem_if<tlm::tlm_generic_payload>* ifs)
{
  mFwDirectMemIf = ifs;
}

void
dmi_handler::
invalidate(DmiRanges& ranges)
{
  tlm::tlm_dmi dmiData;
  if (!is_dmi_enabled() ||
      !abstraction_level_switch::get_instance().is_dmi_allowed()) {
    dmiData.set_start_address(0);
    dmiData.set_end_address((unsigned long long)-1);
    dmiData.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_NONE);

  } else {
    dmiData.set_start_address(1);
    dmiData.set_end_address(0);
  }

  if (ranges.empty()) {
    ranges.push_front(dmiData);
  }
  else {
    // remove all but first element
    ranges.erase(++ranges.begin(), ranges.end());
    ranges.front() = dmiData;
  }
}

bool
dmi_handler::
get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmiData)
{
  mIncomingTrans = &trans;
  bool result;
  if (trans.is_read()) {
    dmiData = *get_read_range(trans.get_address());
    result = dmiData.is_read_allowed();
    dmiData.allow_read();

  } else {
    assert(trans.is_write());
    dmiData = *get_write_range(trans.get_address());
    result = dmiData.is_write_allowed();
    dmiData.allow_write();
  }

  mIncomingTrans = nullptr;
  return result;
}

void
dmi_handler::
invalidate_direct_mem_ptr(sc_dt::uint64, sc_dt::uint64)
{
  invalidate(mReadRanges);
  invalidate(mWriteRanges);
}

void
dmi_handler::
enable_dmi()
{
  if (!is_dmi_enabled()) {
    mDmiEnabled = true;
    invalidate_direct_mem_ptr(0, (unsigned long long)-1);
  }
}

void
dmi_handler::
disable_dmi()
{
  if (is_dmi_enabled()) {
    mDmiEnabled = false;
    invalidate_direct_mem_ptr(0, (unsigned long long)-1);
  }
}

tlm::tlm_dmi*
dmi_handler::
get_read_range_slow(unsigned long long address)
{
  DmiRangesIter it = mReadRanges.begin();
  DmiRangesConstIter end = mReadRanges.end();
  for (; it != end; ++it) {
    if (contains(*it, address)) {
      if (it != mReadRanges.begin()) {
        // move entry to the front
        mReadRanges.splice(mReadRanges.begin(), mReadRanges, it);
      }
      return &mReadRanges.front();
    }
  }

  return load_range(address, true);
}

tlm::tlm_dmi*
dmi_handler::
get_write_range_slow(unsigned long long address)
{
  DmiRangesIter it = mWriteRanges.begin();
  DmiRangesConstIter end = mWriteRanges.end();
  for (; it != end; ++it) {
    if (contains(*it, address)) {
      if (it != mWriteRanges.begin()) {
        // move entry to the front
        mWriteRanges.splice(mWriteRanges.begin(), mWriteRanges, it);
      }
      return &mWriteRanges.front();
    }
  }

  return load_range(address, false);
}

tlm::tlm_dmi*
dmi_handler::
load_range(unsigned long long address, bool readNotWrite)
{
  tlm::tlm_generic_payload& trans = mIncomingTrans != nullptr ? *mIncomingTrans : mTrans;

  if (mIncomingTrans==nullptr) {
    mTrans.set_address(address);
    mTrans.set_command(readNotWrite ? tlm::TLM_READ_COMMAND : tlm::TLM_WRITE_COMMAND);
  }
    
  tlm::tlm_dmi dmiData;

  if (!mFwDirectMemIf) {
    SCML2_LOG(mError) << "no 'tlm::tlm_fw_direct_mem_if' set" << std::endl;
    exit(-1);
  }

  if (!mFwDirectMemIf->get_direct_mem_ptr(trans, dmiData)) {
    dmiData.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_NONE);
  }

  // IEEE 1666-2011
  // 11.2.3 get_direct_mem_ptr method
  // h) If the target is not able to support DMI access to the given address, it need set only the address range
  // and type members of the DMI descriptor as described below and set the return value of the function
  // to false. When a target denies DMI access, the DMI descriptor is used to indicate the details of the
  // access being denied.
  if (!contains(dmiData, address)) {
    SCML2_LOG(mError) << "dmi request for address " << address << " returned true while the returned range [" << dmiData.get_start_address() << ", " << dmiData.get_end_address() 
		      << "] doesn't include the requested address" << std::endl;
    return 0;
  }

  if (readNotWrite) {
    mReadRanges.push_front(dmiData);
    return &mReadRanges.front();

  } else {
    mWriteRanges.push_front(dmiData);
    return &mWriteRanges.front();
  }
}

void
dmi_handler::
handle_abstraction_level_switched()
{
  invalidate_direct_mem_ptr(0, (unsigned long long)-1);
}

}
