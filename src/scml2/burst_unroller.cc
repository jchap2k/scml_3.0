/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2/burst_unroller.h>
#include <scml2/utils.h>

#include <tlm.h>
#include <algorithm>
#include <cassert>

namespace scml2 {

burst_unroller::
burst_unroller(unsigned int width) :
  mWordSize(width)
{
}

burst_unroller::
~burst_unroller()
{
}

unsigned int 
burst_unroller::
get_access_size_at_address(unsigned long long /*address*/) const
{
  return mWordSize;
}

void
burst_unroller::
unroll_transaction(tlm::tlm_generic_payload& trans, sc_core::sc_time& t)
{
  assert(trans.get_streaming_width());
  assert(!trans.get_byte_enable_ptr() || trans.get_byte_enable_length());

  const unsigned long long address = trans.get_address();
  unsigned char* const dataPtr = trans.get_data_ptr();
  const unsigned int dataLength = trans.get_data_length();
  unsigned char* const byteEnablePtr = trans.get_byte_enable_ptr();
  const unsigned int byteEnableLength = trans.get_byte_enable_length();
  const unsigned int origStreamingWidth = trans.get_streaming_width();
  const unsigned int streamingWidth = std::min(dataLength, origStreamingWidth);

  {
    // TODO: we could also consider reusing the byte_enable and data_ptr from the original
    // transaction and simply adjusting the pointers as we unroll the transaction
    // in that case we would save on the extra allocations and copying of data 
    std::vector<unsigned char> data;
    std::vector<unsigned char> byteEnables;

    const unsigned int offset = address % get_access_size_at_address(address);
    const unsigned long long baseAddress = address - offset;
    unsigned long long currentAddress = baseAddress;
    unsigned int bytesLeftInStreamingBeat =
      std::min(streamingWidth, dataLength);
    for (unsigned int currentIndex = 0; currentIndex < dataLength; ) {
      unsigned int access_size = get_access_size_at_address(currentAddress);
      trans.set_streaming_width(access_size);
      trans.set_data_length(access_size);
      unsigned int currentDataLength;
      unsigned int currentOffset;
      if (currentIndex % streamingWidth == 0) {
        currentOffset = offset;
        currentDataLength =
          std::min(access_size - offset, bytesLeftInStreamingBeat);

      } else {
        currentOffset = 0;
        currentDataLength = std::min(access_size, bytesLeftInStreamingBeat);
      }
      assert(currentDataLength);

      bool needDataCopy = (currentIndex + currentDataLength > dataLength);
      if (needDataCopy) {
	// copy data array
	data.resize(access_size);
	trans.set_data_ptr(&data[0]);
	memset(&data[0], 0, access_size);
	memcpy(&data[0] + currentOffset,
	       dataPtr + currentIndex,
	       currentDataLength);
      } else {
	trans.set_data_ptr(&dataPtr[0]+currentIndex-currentOffset);
      }

      // copy byte enables
      if (byteEnablePtr && !currentOffset && (byteEnableLength >= currentIndex + std::max(access_size,dataLength))) {
	trans.set_byte_enable_length(byteEnableLength-currentIndex);
	trans.set_byte_enable_ptr(&byteEnablePtr[0]+currentIndex);
      } else if (!byteEnablePtr && currentDataLength == access_size) {
	trans.set_byte_enable_ptr(0);
	trans.set_byte_enable_length(0);
      } else { 
	byteEnables.resize(access_size);
	trans.set_byte_enable_ptr(&byteEnables[0]);
	trans.set_byte_enable_length(access_size);
	memset  (&byteEnables[0],                                     TLM_BYTE_DISABLED,               currentOffset);
	if (byteEnablePtr) {
	  for (unsigned int i = 0; i < currentDataLength; ++i) {
	    byteEnables[currentOffset + i] =
	      byteEnablePtr[(currentIndex + i) % byteEnableLength];
	  }
	} else {
	  memset(&byteEnables[0] + currentOffset,                     TLM_BYTE_ENABLED,                                currentDataLength);	  
	}
	memset  (&byteEnables[0] + currentOffset + currentDataLength, TLM_BYTE_DISABLED, access_size - currentOffset - currentDataLength);
      }

      trans.set_address(currentAddress);
      trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

      process_unrolled_transaction(trans, t);

      if (trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
        goto restore_and_exit;
      }
  
      if (needDataCopy && trans.is_read()) {
        // copy back read data
        memcpy(dataPtr + currentIndex,
               &data[0] + currentOffset,
               currentDataLength);
      }

      currentIndex += currentDataLength;
      bytesLeftInStreamingBeat -= currentDataLength;
      if (currentIndex % streamingWidth == 0) {
        bytesLeftInStreamingBeat =
          std::min(streamingWidth, dataLength - currentIndex);
        currentAddress = baseAddress;

      } else {
        currentAddress += access_size;
      }
    }
  }

  trans.set_response_status(tlm::TLM_OK_RESPONSE);

restore_and_exit:
  trans.set_address(address);
  trans.set_data_ptr(dataPtr);
  trans.set_data_length(dataLength);
  trans.set_byte_enable_ptr(byteEnablePtr);
  trans.set_byte_enable_length(byteEnableLength);
  trans.set_streaming_width(origStreamingWidth);
}

}
