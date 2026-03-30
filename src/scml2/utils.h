/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_UTILS_H
#define SCML2_UTILS_H

#include "sysc/kernel/sc_object.h"
#include <systemc>
#include <tlm.h>

#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <functional>
#include <scml2_logging/snps_vp_dll.h>

#define SCML2_FOR_EACH_DT(doit) \
  doit(unsigned char) \
  doit(unsigned short) \
  doit(unsigned int) \
  doit(unsigned long long) \
  doit(sc_dt::sc_biguint<128>) \
  doit(sc_dt::sc_biguint<256>) \
  doit(sc_dt::sc_biguint<512>) \
  doit(sc_dt::sc_biguint<1024>) \
  doit(sc_dt::sc_biguint<2048>) \
  doit(sc_dt::sc_biguint<4096>)

namespace scml2 {

template <typename T>
struct SizeOf {
  enum { result = sizeof(T) };
};

template <int N>
struct SizeOf<sc_dt::sc_biguint<N> > {
  //FIXME: assert(N % 8 );
  enum { result = N / 8 };
};

template <typename T>
inline
unsigned int sizeOf()
{
  return SizeOf<T>::result;
}

template< unsigned int DW > struct DataWidth2Type
{
  typedef
    typename std::conditional<DW <= 8, unsigned char,
    typename std::conditional<DW <= 16, unsigned short,
    typename std::conditional<DW <= 32, unsigned int,
    typename std::conditional<DW <= 64, unsigned long long,
    typename std::conditional<DW <= 128, sc_dt::sc_biguint<128>,
    typename std::conditional<DW <= 256, sc_dt::sc_biguint<256>,
    typename std::conditional<DW <= 512, sc_dt::sc_biguint<512>,
    typename std::conditional<DW <= 1024, sc_dt::sc_biguint<1024>,
    typename std::conditional<DW <= 2048, sc_dt::sc_biguint<2048>,
    typename std::conditional<DW <= 4096, sc_dt::sc_biguint<4096>,
    void  // not supported value
  >::type>::type>::type>::type>::type>::type>::type>::type>::type>::type result;
};

inline
void
copy_data(unsigned char* to,
          const unsigned char* from,
          unsigned int dataLength,
          const unsigned char* byteEnablePtr = 0,
          unsigned int byteEnableLength = 0)
{
  assert(to);
  assert(from);
  if (byteEnablePtr) {
    assert(byteEnableLength);
    for (unsigned int i = 0; i < dataLength; ++i) {
      unsigned char mask = byteEnablePtr[i % byteEnableLength];
      to[i] = (from[i] & mask) | (to[i] & ~mask);
    }
  } else {
    memcpy(to, from, dataLength);
  }
}

template <typename DT>
inline
void
to_char_array(DT from,
              unsigned char* to,
              size_t size,
              size_t offset)
{
  if (size == sizeOf<DT>()) {
    assert(!offset);
    *reinterpret_cast<DT*>(to) = from;

  } else {
    // coverity[ptr_arith]
    memcpy(to, reinterpret_cast<unsigned char*>(&from) + offset, size);
  }
}

template <typename DT>
inline
void
to_char_array(DT from,
              unsigned char* to,
	      unsigned char* be,
              size_t size,
              size_t offset)
{
  for (size_t i = 0; i != size; ++i) {
    if (be == 0) {
    // coverity[ptr_arith]
      to[i] = *(reinterpret_cast<unsigned char*>(&from) + offset + i);
    } else if (be[i] != 0) {
    // coverity[ptr_arith]
      to[i] = (to[i] & ~be[i]) | (*(reinterpret_cast<unsigned char*>(&from) + offset + i) & be[i]);
    }
  }
}

template <int N>
inline
void
to_char_array(const sc_dt::sc_biguint<N>& from,
              unsigned char* to,
              size_t size,
              size_t offset)
{
  // FIXME: only works on LE host
  for (unsigned int i = 0; i < size; ++i) {
    to[i] = ((from >> static_cast<int>(8 * (i + offset))).to_uint()) & 0xff;
  }
}

template <int N>
inline
void
to_char_array(const sc_dt::sc_biguint<N>& from,
              unsigned char* to,
	      unsigned char* be,
              size_t size,
              size_t offset)
{
  // FIXME: only works on LE host
  for (unsigned int i = 0; i < size; ++i) {
    if (be == 0) {
      to[i] = (from >> static_cast<int>(8 * (i + offset))).to_uint();
    } else if (be[i] != 0) {
      to[i] = (to[i] & ~be[i]) | (((from >> static_cast<int>(8 * (i + offset))).to_uint()) & be[i]);
    }
  }
}

template <typename DT>
inline
void
from_char_array(const unsigned char* from,
                DT& to,
                size_t size,
                size_t offset,
		const unsigned char* be = 0)
{
  if (size == sizeOf<DT>() && be == 0) {
    assert(!offset);
    to = *reinterpret_cast<const DT*>(from);

  } else if (be == 0) {
    memcpy(reinterpret_cast<unsigned char*>(&to) + offset, from, size);
  } else {
    for (unsigned i = 0; i != size; ++i) {
      if (be[i] != 0) {
	unsigned char& orig_value = reinterpret_cast<unsigned char*>(&to)[offset+i];
	orig_value=(orig_value&~be[i])|(from[i]&be[i]);
      }
    }
  }
}

template <int N>
inline
void
from_char_array(const unsigned char* from,
                sc_dt::sc_biguint<N>& to,
                size_t size,
                size_t offset,
		const unsigned char* be = 0)
{
  // FIXME: only works on LE host
  for (unsigned int i = 0; i < size; ++i) {
    int l = (int)(8 * (i + offset));
    int h = (int)(8 * ((i+1) + offset))-1;
    if (be==0) {
      to.range(h,l) = (unsigned)(from[i]);
    } else if (be[i] != 0) {
      unsigned char original_value = to.range(l,h).to_uint();
      to.range(h,l) = (unsigned char)(original_value&~be[i])|(from[i]&be[i]);
    }
  }
}

template <typename DT>
inline
DT
get_bit_mask(unsigned int size)
{
  DT mask = ~(DT)0;
  if(size >= 8*sizeOf<DT>()) {
    return mask;

  } else {
    mask <<= size;
    return ~mask;
  }
}

template <typename DT>
inline
DT
extract_bits(const DT& v,
             unsigned int sizeBits,
             unsigned int offsetBits)
{
  const DT mask = get_bit_mask<DT>(sizeBits);

  return (v >> offsetBits) & mask;
}

template <typename DT>
inline
DT
insert_bits(const DT& v,
            const DT& rhs,
            unsigned int sizeBits,
            unsigned int offsetBits)
{
  DT mask = get_bit_mask<DT>(sizeBits);

  DT new_data = (rhs & mask);
  mask <<= offsetBits;
  new_data <<= offsetBits;

  return (v & ~mask) | new_data;
}

inline
unsigned long long
address_to_index(unsigned long long address, unsigned int width)
{
  return address / width;
}

template <typename DT>
inline
unsigned long long
address_to_index(unsigned long long address)
{
  return address_to_index(address, sizeOf<DT>());
}

inline
unsigned long long
index_to_address(unsigned long long index, unsigned int width)
{
  return index * width;
}

template <typename DT>
inline
unsigned long long
index_to_address(unsigned long long index)
{
  return index_to_address(index, sizeOf<DT>());
}

//FIXME
inline
void
copy_extension_pointers(tlm::tlm_generic_payload& from,
                        tlm::tlm_generic_payload& to)
{
  for (unsigned int i = 0; i < tlm::max_num_extensions(); ++i) {
    to.set_extension(i, from.get_extension(i));
  }
}

inline
void
clear_extension_pointers(tlm::tlm_generic_payload& trans)
{
  for (unsigned int i = 0; i < tlm::max_num_extensions(); ++i) {
    // FIXME: clear_extension is private
    trans.set_extension(i, 0);
  }
}

template <typename DT>
class dt_to_array
{
public:
  // FIXME: add size/offset?
  dt_to_array(const DT& v) :
    mArray(reinterpret_cast<const unsigned char*>(&v))
  {
  }

  operator const unsigned char*() const
  {
    return mArray;
  }

private:
  const unsigned char* mArray;
};

template <int N>
class dt_to_array<sc_dt::sc_biguint<N> >
{
public:
  // FIXME: add size/offset?
  dt_to_array(const sc_dt::sc_biguint<N>& v)
  {
    to_char_array(v, mArray, N / 8, 0);
    // coverity[uninit_member]
  }

  operator const unsigned char*() const
  {
    return mArray;
  }

private:
  unsigned char mArray[N / 8];
};

template <typename DT>
class dt_from_array
{
public:
  // FIXME: add offset?
  dt_from_array(DT& v,
                unsigned int /* size */ = sizeof(DT),
                bool clearNotCopy = true) :
    mArray(reinterpret_cast<unsigned char*>(&v))
  {
    if (clearNotCopy) {
      v = 0;
    }
  }

  operator unsigned char*() const
  {
    return mArray;
  }

private:
  unsigned char* mArray;
};

template <typename DT>
class const_dt_from_array
{
public:
  // FIXME: add offset?
  const_dt_from_array(const DT& v,
		      unsigned int /* size */ = sizeof(DT)) :
    mArray(reinterpret_cast<const unsigned char*>(&v))
  {
  }

  operator const unsigned char*() const
  {
    return mArray;
  }

private:
  const unsigned char* mArray;
};

template <int N>
class dt_from_array<sc_dt::sc_biguint<N> >
{
public:
  // FIXME: add offset?
  dt_from_array(sc_dt::sc_biguint<N>& v,
                unsigned int size = N / 8,
                bool clearNotCopy = true) :
    mValue(v),
    mSize(size)
  {
    if (clearNotCopy) {
      memset(mArray, 0, N / 8);

    } else {
      to_char_array(mValue, mArray, size, 0);
    }
    // coverity[uninit_member]
  }

  ~dt_from_array()
  {
    from_char_array(mArray, mValue, mSize, 0);
  }

  operator unsigned char*()
  {
    return mArray;
  }

private:
  unsigned char mArray[N / 8];
  sc_dt::sc_biguint<N>& mValue;
  unsigned int mSize;
};

template <int N>
class const_dt_from_array<sc_dt::sc_biguint<N> >
{
public:
 const_dt_from_array(const sc_dt::sc_biguint<N>& v,
                unsigned int size = N / 8) :
    mValue(v),
    mSize(size)
  {
    to_char_array(mValue, mArray, size, 0);
    // coverity[uninit_member]
  }

  operator const unsigned char*()
  {
    return mArray;
  }

private:
  unsigned char mArray[N / 8];
  const sc_dt::sc_biguint<N>& mValue;
  unsigned int mSize;
};

template <typename DT>
class array_to_dt
{
public:
  explicit array_to_dt(const unsigned char* a,
                       unsigned int size = sizeof(DT),
                       unsigned int offset = 0) :
    mArray(a),
    mSize(size),
    mOffset(offset)
  {
  }

  operator DT() const
  {
    if (mSize == sizeof(DT)) {
      assert(!mOffset);
      return *reinterpret_cast<const DT*>(mArray);

    } else {
      DT value = 0;
      from_char_array(mArray, value, mSize, mOffset);
      return value;
    }
  }

private:
  const unsigned char* mArray;
  const unsigned int mSize;
  const unsigned int mOffset;
};

template <int N>
class array_to_dt<sc_dt::sc_biguint<N> >
{
public:
  array_to_dt(const unsigned char* a,
              unsigned int size = N / 8,
              unsigned int offset = 0)
  {
    from_char_array(a, mValue, size, offset);
  }

  operator const sc_dt::sc_biguint<N>&() const
  {
    return mValue;
  }

private:
  sc_dt::sc_biguint<N> mValue;
};

template <typename DT>
class array_from_dt
{
public:
  array_from_dt(unsigned char* a,
                unsigned int size = sizeof(DT),
                unsigned int offset = 0,
                bool clearNotCopy = true,
		unsigned char* be = 0) :
    mArray(a),
    mSize(size),
    mOffset(offset),
    mValue(0),
    mBe(be)  
  {
    if (!clearNotCopy) {
      from_char_array(a, mValue, mSize, mOffset, mBe);
    }
  }

  ~array_from_dt()
  {
    to_char_array(mValue, mArray, mBe, mSize, mOffset);
  }

  operator DT&()
  {
    return mValue;
  }

private:
  unsigned char* mArray;
  unsigned int mSize;
  unsigned int mOffset;
  DT mValue;
  unsigned char* mBe;
};

template <int N>
class array_from_dt<sc_dt::sc_biguint<N> >
{
public:
  array_from_dt(unsigned char* a,
                unsigned int size = N / 8,
                unsigned int offset = 0,
                bool clearNotCopy = true,
		unsigned char* be = 0) :
    mArray(a),
    mSize(size),
    mOffset(offset),
    mBe(be)  
  {
    if (clearNotCopy) {
      mValue = 0;

    } else {
      from_char_array(a, mValue, mSize, mOffset, mBe);
    }
  }

  ~array_from_dt()
  {
    to_char_array(mValue, mArray, mBe, mSize, mOffset);
  }

  operator sc_dt::sc_biguint<N>&()
  {
    return mValue;
  }

private:
  unsigned char* mArray;
  unsigned int mSize;
  unsigned int mOffset;
  sc_dt::sc_biguint<N> mValue;
  unsigned char* mBe;
};

//FIXME
inline
std::string
create_hierarchical_sc_name(const std::string& n)
{
  const sc_core::sc_simcontext* context = sc_core::sc_get_curr_simcontext();

  std::string name;
  if(context) {
#if (!defined CWR_SYSTEMC && defined SYSTEMC_VERSION && SYSTEMC_VERSION <= 20221128 /*2.3.4*/)
    const sc_core::sc_module* host = context->hierarchy_curr();
#elif (defined CWR_SYSTEMC)
    const sc_core::sc_object_host* host = context->hierarchy_curr();
#else
    const sc_core::sc_object* host = sc_core::sc_get_current_object();
#endif
    if(host) {
      name = host->name();
      if (!n.empty()) {
        name += '.';
      }
    }
  }
  name += n;
  return name;
}

inline
sc_core::sc_time::value_type value_in_ps(const sc_core::sc_time& t) {
	static sc_core::sc_time one_second(1, sc_core::SC_SEC);

	// prefer pico and nano
	if (one_second.value() == 1000ull * 1000 * 1000 * 1000) {  // pico
		return t.value();
	}
	if (one_second.value() == 1000ull * 1000 * 1000) {  // ns
		return t.value() * 1000;
	}

	if (one_second.value() == 1000ull * 1000 * 1000 * 1000 * 1000) {  // femto
		return t.value() / 1000;
	}
	if (one_second.value() == 1000ull * 1000) {  // us
		return t.value() * 1000 * 1000;
	}
	if (one_second.value() == 1000ull) {  // ms
		return t.value() * 1000 * 1000 * 1000;
	}
	if (one_second.value() == 1ull) {  // s
		return t.value() * 1000 * 1000 * 1000 * 1000;
	}

	// fallback
	return (sc_dt::uint64)(t.to_seconds() * 1000 * 1000 * 1000 * 1000);
}

inline
bool
are_bytes_enabled(const tlm::tlm_generic_payload& trans, unsigned long long count)
{
  if (!trans.get_byte_enable_ptr()) {
    return true;
  }
  for (unsigned i = 0; i != (std::min)((unsigned long long)trans.get_data_length(), count); ++i) {
    if (trans.get_byte_enable_ptr()[i%trans.get_byte_enable_length()] != TLM_BYTE_DISABLED) {
      return true;
    }
  }
  return false;
}
  
template<typename T>
struct trans_helper_base {
  trans_helper_base(tlm::tlm_generic_payload& trans)
    : m_trans(trans)
    , m_engaged(false)
  {}

  ~trans_helper_base() 
  {
    assert(!engaged());
  }

  bool engaged() const 
  {
    return m_engaged;
  }

  void engage(const T& v) 
  {
    assert(!engaged());
    m_engaged = true;
    do_engage(v);
  }
protected:
  void disengaged()
  {
    assert(engaged());
    m_engaged = false;
  }

  virtual void do_engage(const T& v) = 0;

  tlm::tlm_generic_payload& m_trans;
private:
  bool m_engaged;
};
 
struct trans_address_helper 
  : public trans_helper_base<unsigned long long>
{
  trans_address_helper(tlm::tlm_generic_payload& trans) 
    : trans_helper_base<unsigned long long>(trans)
    , m_offset(0) 
  {
  }

  trans_address_helper(tlm::tlm_generic_payload& trans, unsigned long long offset) 
    : trans_helper_base<unsigned long long>(trans)
    , m_offset(0) 
  {
    engage(offset);
  }
  
  ~trans_address_helper() 
  {
    if (engaged()) {
      m_trans.set_address(m_trans.get_address()+m_offset);
      disengaged();
    }
  }
private:
  virtual void do_engage(const unsigned long long& offset) 
  {
    m_offset = offset;
    m_trans.set_address(m_trans.get_address()-m_offset);
  }
  
  unsigned long long m_offset;
};

struct trans_byte_enable_helper 
  : public trans_helper_base<unsigned char*>
{
  trans_byte_enable_helper(tlm::tlm_generic_payload& trans)
    : trans_helper_base<unsigned char*>(trans)
    , m_old_byte_enable_ptr(trans.get_byte_enable_ptr())
    , m_old_byte_enable_length(trans.get_byte_enable_length()) 
  {}

  trans_byte_enable_helper(tlm::tlm_generic_payload& trans, unsigned char* byte_enable_ptr)
    : trans_helper_base<unsigned char*>(trans)
    , m_old_byte_enable_ptr(trans.get_byte_enable_ptr())
    , m_old_byte_enable_length(trans.get_byte_enable_length()) 
  {
    engage(byte_enable_ptr);
  }

  ~trans_byte_enable_helper() 
  {
    if (engaged()) {
      m_trans.set_byte_enable_ptr(m_old_byte_enable_ptr);
      m_trans.set_byte_enable_length(m_old_byte_enable_length);   
      disengaged();
    }
  }
private:
  virtual void do_engage(unsigned char* const & byte_enable_ptr) {
    m_trans.set_byte_enable_ptr(byte_enable_ptr);
    m_trans.set_byte_enable_length(m_trans.get_data_length());
  }

  unsigned char* m_old_byte_enable_ptr;
  unsigned m_old_byte_enable_length;
};

struct trans_data_helper 
  : public trans_helper_base<unsigned char*>
{
  trans_data_helper(tlm::tlm_generic_payload& trans)
    : trans_helper_base<unsigned char*>(trans)
    , m_old_data_ptr(trans.get_data_ptr())
  {}

  trans_data_helper(tlm::tlm_generic_payload& trans, unsigned char* data_ptr)
    : trans_helper_base<unsigned char*>(trans)
    , m_old_data_ptr(trans.get_data_ptr())
  {
    engage(data_ptr);
  }

  ~trans_data_helper() 
  {
    if (engaged()) {
      m_trans.set_data_ptr(m_old_data_ptr);
      disengaged();
    }
  }
private:
  virtual void do_engage(unsigned char* const& data_ptr) {
    m_old_data_ptr = m_trans.get_data_ptr();
    m_trans.set_data_ptr(data_ptr);
  }

  unsigned char* m_old_data_ptr;
};

template <typename T>
class obj_array
{
public:
  ~obj_array() {
    for(size_t i = 0; i < mObjects.size(); ++i) {
      delete mObjects[i];
    }
    mObjects.clear();
  }

  const T& operator[](size_t i) const {
    return *(mObjects[i]);
  }

  T& operator[](size_t i) {
    return *(mObjects[i]);
  }

  size_t size() const {
    return mObjects.size();
  }
  
  void append(T* obj) {
    assert(obj != 0);
    mObjects.push_back(obj);
  }

  class const_iterator;

  class iterator {
  private:
    friend class const_iterator;
    obj_array& m_oa;
    unsigned long long m_index;
  public:
    iterator(obj_array& oa, unsigned long long index) : m_oa(oa), m_index(index) {}
    T& operator*() const { return m_oa[m_index]; }
    iterator& operator++() {++m_index; return *this;}
    bool operator==(const iterator& rhs) const {return &m_oa==&(rhs.m_oa) && m_index==rhs.m_index;}
    bool operator!=(const iterator& rhs) const {return !((*this)==rhs);}
  };

  iterator begin() {return iterator(*this, 0);}
  iterator end() {return iterator(*this, mObjects.size());}

  class const_iterator {
  private:
    const obj_array& m_oa;
    unsigned long long m_index;
  public:
    const_iterator(const obj_array& oa, unsigned long long index) : m_oa(oa), m_index(index) {}
    const_iterator(const iterator& other) : m_oa(other.m_oa), m_index(other.m_index) {}
    const T& operator*() const { return m_oa[m_index]; }
    const_iterator& operator++() {++m_index; return *this;}
    bool operator==(const const_iterator& rhs) const {return &m_oa==&(rhs.m_oa) && m_index==rhs.m_index;}
    bool operator!=(const const_iterator& rhs) const {return !((*this)==rhs);}
  };

  const_iterator begin() const {return const_iterator(*this, 0);}
  const_iterator end() const {return const_iterator(*this, mObjects.size());}
  
private:
  std::vector<T*> mObjects;
};

inline bool to_bool(bool input) {
  return input;
}

inline bool to_bool(const std::function<bool()>& f) {
  return f();
}

}

#endif
