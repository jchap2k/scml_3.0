/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_UNMAPPED_RESTRICTION_ADAPTER_H
#define SCML2_UNMAPPED_RESTRICTION_ADAPTER_H

#include <scml2/types.h>

#include <cassert>

namespace tlm {
  class tlm_generic_payload;
}

namespace scml2 {

template <typename DT, template <typename> class MEM_OBJECT>
class unmapped_restriction_adapter
{
public:
  unmapped_restriction_adapter(MEM_OBJECT<DT>& mem, bool error, unsigned char value);

  access_restriction_result operator()(tlm::tlm_generic_payload& trans);

 private:
  MEM_OBJECT<DT>& m_mem;
  bool m_error;
  unsigned char m_value;
};

template <typename DT, template <typename> class MEM_OBJECT>
inline
unmapped_restriction_adapter<DT, MEM_OBJECT>
create_unmapped_restriction_adapter(MEM_OBJECT<DT>& mem, bool error, unsigned char value = 0)
{
  return unmapped_restriction_adapter<DT,MEM_OBJECT>(mem, error, value);
}

template <typename DT, template <typename> class MEM_OBJECT>
inline
unmapped_restriction_adapter<DT, MEM_OBJECT>::
unmapped_restriction_adapter(MEM_OBJECT<DT>& mem, bool error, unsigned char value) 
  : m_mem(mem)
  , m_error(error)
  , m_value(value)
{}

namespace {
  struct alias_to_index_comparator2 {
    bool operator()(const memory_base* lhs, unsigned long long idx) const {
      return lhs->get_offset() < idx;
    }
    bool operator()(unsigned long long idx, const memory_base* lhs) const {
      return idx < lhs->get_offset();
    }
  };
}

  
template <typename DT, template <typename> class MEM_OBJECT>
inline
access_restriction_result
unmapped_restriction_adapter<DT, MEM_OBJECT>::
operator()(tlm::tlm_generic_payload& trans)
{
  assert(trans.get_data_length() >= sizeOf<DT>());
  assert(trans.get_byte_enable_ptr() != 0);
  assert(trans.get_byte_enable_length() >= sizeOf<DT>());
  assert((trans.get_address() % sizeOf<DT>()) == 0);

  bool all_indices_mapped = true;
  size_t address = trans.get_address();
  size_t lowIndex = address_to_index<DT>(address)+m_mem.get_offset();
  size_t highIndex = address_to_index<DT>(address + trans.get_data_length()-1)+m_mem.get_offset();

  for (size_t idx = lowIndex; idx <=highIndex; ++idx) {
    size_t idxAddress = index_to_address<DT>(idx-m_mem.get_offset());
    if (array_to_dt<DT>(&trans.get_byte_enable_ptr()[idxAddress-address]) != 0) {
      memory_base::AliasesConstIterator child = std::upper_bound
	(m_mem.begin_alias(), m_mem.end_alias(), lowIndex, alias_to_index_comparator2());
      bool child_at_index = false;
      if (child != m_mem.begin_alias()) {
	child = std::prev(child);
	size_t child_end_index = (*child)->get_offset() + ((*child)->get_size()*(*child)->get_width())/m_mem.get_width();
	child_at_index = (child_end_index > lowIndex);
      } else {
	child_at_index = false;
      }
      if (!child_at_index) {
	for (size_t i = 0; i != sizeOf<DT>(); ++i) {
	    unsigned long long ptrIdx = (idxAddress-address)+i;
	    unsigned char byteEnable = trans.get_byte_enable_ptr()[ptrIdx];
	    if (byteEnable!=0) {
	      trans.get_data_ptr()[ptrIdx] = (~byteEnable&trans.get_data_ptr()[(ptrIdx)]) | (byteEnable&m_value);
	      trans.get_byte_enable_ptr()[ptrIdx] = 0;
	    }
	}
	all_indices_mapped = false;
      }
    }
  }

  if (m_error && !all_indices_mapped) {
    return RESTRICT_ERROR;
  } else {
    return RESTRICT_NO_ERROR;
  }
  
}

}

#endif
