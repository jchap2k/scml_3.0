/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_BITFIELD_H
#define SCML2_BITFIELD_H

#include <scml2/bitfield_base.h>
#include <scml2/reg.h>
#include <scml2/bitfield_read_callback_base.h>
#include <scml2/bitfield_write_callback_base.h>
#include <scml2/bitfield_post_write_callback_base.h>
#include <scml2/bitfield_debug_read_callback_base.h>
#include <scml2/bitfield_debug_write_callback_base.h>
#include <scml2/bitfield_callback_observers.h>
#include <scml2/bitfield_container.h>
#include <scml2/bitfield_access_restriction_base.h>
#include <scml2/restriction_owner.h>

#include <scml2/utils.h>
#include <scml2/scoped_reference.h>
#include <scml2/types.h>
#include <scml2_logging/snps_vp_dll.h>

#include <scml2/tagged_message_handler_provider.h>

#include <tlm.h>
#include <string>
#include <cassert>
#include <memory>

#if defined(__clang__) || defined(__GNUC__)
// ignore false positive warnings we started seeing with gcc 12.3
// Example:
// void operator delete(void*, std::size_t)' called on pointer '<unknown>' with nonzero offset 336
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif

namespace sc_core {
  class sc_time;
}

namespace scml2 {

void bitfield_count_created(); // temporary for evaluation purposes
void bitfield_count_initialized(); // temporary for evaluation purposes

template<typename DT>
class bitfield_restriction_api_owner
{
public:
  bitfield_restriction_api_owner() {}
  virtual ~bitfield_restriction_api_owner() {}

  virtual void set_restriction(bitfield_access_restriction_base<DT>* r) = 0;
  virtual void set_read_restriction(bitfield_access_restriction_base<DT>* r) = 0;
  virtual void set_write_restriction(bitfield_access_restriction_base<DT>* r) = 0;

  virtual bitfield_access_restriction_base<DT>* get_read_restriction() const = 0;
  virtual bitfield_access_restriction_base<DT>* get_write_restriction() const = 0;

  virtual void remove_restriction() = 0;
  virtual void remove_read_restriction() = 0;
  virtual void remove_write_restriction() = 0;
};

template<typename DT>
class bitfield_callback_observers_api_owner {
public:
  bitfield_callback_observers_api_owner() {}
  virtual ~bitfield_callback_observers_api_owner() {}

  virtual bool register_callback_observer(bitfield_callback_observer<DT>* o) = 0;
  virtual bool unregister_all_callback_observers() = 0;
  virtual bool unregister_callback_observer(bitfield_callback_observer<DT>* o) = 0;
};

template<typename DT>
class bitfield_callback_api_owner {
public:
  bitfield_callback_api_owner() {}
  virtual ~bitfield_callback_api_owner() {}

  virtual void set_read_callback(bitfield_read_callback_base<DT>* cb) = 0;
  virtual void set_write_callback(bitfield_write_callback_base<DT>* cb) = 0;
  virtual void set_post_write_callback(bitfield_post_write_callback_base<DT>* cb) = 0;
  virtual void set_post_write_callback(DT enum_val, bitfield_post_write_callback_base<DT>* cb) = 0;
  virtual void set_default_enum_write_callback(bitfield_post_write_callback_base<DT>* cb) = 0;
  virtual void remove_callback() = 0;
  virtual void remove_read_callback() = 0;
  virtual void remove_write_callback() = 0;
  virtual void remove_post_write_callback() = 0;
  virtual bitfield_read_callback_base<DT>* get_read_callback() const = 0;
  virtual bitfield_write_callback_base<DT>* get_write_callback() const = 0;
};

template<typename DT>
class bitfield_debug_callback_api_owner {
public:
  bitfield_debug_callback_api_owner() {}
  virtual ~bitfield_debug_callback_api_owner() {}

  virtual void set_debug_read_callback(bitfield_debug_read_callback_base<DT>* cb) = 0;
  virtual void set_debug_write_callback(bitfield_debug_write_callback_base<DT>* cb) = 0;
  virtual void remove_debug_callback() = 0;
  virtual void remove_debug_read_callback() = 0;
  virtual void remove_debug_write_callback() = 0;
  virtual bitfield_debug_read_callback_base<DT>* get_debug_read_callback() const = 0;
  virtual bitfield_debug_write_callback_base<DT>* get_debug_write_callback() const = 0;
};

template <typename DT> class bitfield_callback_observer;
template <typename DT> class bitfield_defaults;

template <typename DT>
class bitfield 
  : public bitfield_base
  , public restriction_owner
  , public tagged_message_handler_provider
  , public bitfield_restriction_api_owner<DT>
  , public bitfield_callback_observers_api_owner<DT>
  , public bitfield_callback_api_owner<DT>
  , public bitfield_debug_callback_api_owner<DT>
{
public:
  typedef DT data_type;

private:
  class bitfield_defaults
    : public bitfield_read_callback_base<DT>
    , public bitfield_write_callback_base<DT>
    , public bitfield_debug_read_callback_base<DT>
    , public bitfield_debug_write_callback_base<DT>
    , public bitfield_post_write_callback_base<DT>
    , public bitfield_access_restriction_base<DT>
  {

  public:
    bitfield_defaults(bitfield<DT> &bitfield) 
      : bitfield_read_callback_base<DT>()
      , bitfield_write_callback_base<DT>()
      , bitfield_debug_read_callback_base<DT>()
      , bitfield_debug_write_callback_base<DT>()
      , bitfield_post_write_callback_base<DT>()
      , bitfield_access_restriction_base<DT>()
      , mBitfield(bitfield)
      , mReadCallback(this)
      , mWriteCallback(this)
      , mDebugReadCallback(this)
      , mDebugWriteCallback(this)
      , mPostWriteCallback(this)
      , mReadRestriction(this)
      , mWriteRestriction(this)
    {
      static_cast<bitfield_read_callback_base<DT> *>(this)->ref();
      static_cast<bitfield_write_callback_base<DT> *>(this)->ref();
      static_cast<bitfield_debug_read_callback_base<DT> *>(this)->ref();
      static_cast<bitfield_debug_write_callback_base<DT> *>(this)->ref();
      static_cast<bitfield_post_write_callback_base<DT> *>(this)->ref();
      static_cast<bitfield_access_restriction_base<DT> *>(this)->ref();
      mReadCallback->ref();
      mWriteCallback->ref();
      mDebugReadCallback->ref();
      mDebugWriteCallback->ref();
      mPostWriteCallback->ref();
      mReadRestriction->ref();
      mWriteRestriction->ref();
    };

    virtual ~bitfield_defaults() {
      mReadCallback->unref();
      mWriteCallback->unref();
      mDebugReadCallback->unref();
      mDebugWriteCallback->unref();
      mPostWriteCallback->unref();
      mReadRestriction->unref();
      mWriteRestriction->unref();
      assert(static_cast<bitfield_read_callback_base<DT> *>(this)->getRefCount() == 1);
      assert(static_cast<bitfield_write_callback_base<DT> *>(this)->getRefCount() == 1);
      assert(static_cast<bitfield_debug_read_callback_base<DT> *>(this)->getRefCount() == 1);
      assert(static_cast<bitfield_debug_write_callback_base<DT> *>(this)->getRefCount() == 1);
      assert(static_cast<bitfield_post_write_callback_base<DT> *>(this)->getRefCount() == 1);
      assert(static_cast<bitfield_access_restriction_base<DT> *>(this)->getRefCount() == 1);
    }
  
    virtual bool read(DT& value, const DT& bitMask, sc_core::sc_time&) {
      value = mBitfield.get(bitMask);
      return true;
    }

    virtual bool read(DT& value, const DT& bitMask) {
      value = mBitfield.get_debug(bitMask);
      return true;
    }

    virtual bool write(const DT& value, const DT& bitMask, sc_core::sc_time&) {
      mBitfield.put(value, bitMask);
      return true;
    }

    virtual bool write(const DT& value, const DT& bitMask) {
      mBitfield.put_debug(value, bitMask);
      return true;
    }

    virtual bool post_write(const DT&, const DT&, sc_core::sc_time&) {
      return true;
    }

    virtual access_restriction_result restrict(DT&, DT&) {
      return RESTRICT_NO_ERROR;
    }

    virtual bool has_default_behavior() const {
      return true;
    }

    virtual bool has_never_syncing_behavior() const {
      return true;
    }

    virtual bool has_default_restriction() const {
      return true;
    }

    virtual const std::string& get_name() const {
      static std::string DEFAULT("<default>");
      return DEFAULT;
    }

    bitfield<DT>&                           mBitfield;
    bitfield_read_callback_base<DT>*        mReadCallback;
    bitfield_write_callback_base<DT>*       mWriteCallback;
    bitfield_debug_read_callback_base<DT>*  mDebugReadCallback;
    bitfield_debug_write_callback_base<DT>* mDebugWriteCallback;
    bitfield_post_write_callback_base<DT>*  mPostWriteCallback;
    bitfield_access_restriction_base<DT>*   mReadRestriction;
    bitfield_access_restriction_base<DT>*   mWriteRestriction;
  };

public:
  bitfield(const std::string& name,
           reg<DT>& r,
           unsigned int offset,
           unsigned int size);
  bitfield(const std::string& name,
           bitfield<DT>& bf,
           unsigned int offset,
           unsigned int size);
  ~bitfield();

  void put(const DT& value);
  void put(const DT& value, const DT& bitMask);
  DT get() const;
  DT get(const DT& bitMask) const;

  void put_debug(const DT& value);
  void put_debug(const DT& value, const DT& bitMask);
  DT get_debug() const;
  DT get_debug(const DT& bitMask) const;

  bool put_with_triggering_callbacks(const DT& value,
                                     sc_core::sc_time& t);
  bool put_with_triggering_callbacks(const DT& value,
                                     const DT& bitMask,
                                     sc_core::sc_time& t);
  bool get_with_triggering_callbacks(DT& value,
                                     sc_core::sc_time& t) const;
  bool get_with_triggering_callbacks(DT& value,
                                     const DT& bitMask,
                                     sc_core::sc_time& t) const;

  bool put_debug_with_triggering_callbacks(const DT& value);
  bool put_debug_with_triggering_callbacks(const DT& value, const DT& bitMask);
  bool get_debug_with_triggering_callbacks(DT& value) const;
  bool get_debug_with_triggering_callbacks(DT& value, const DT& bitMask) const;

  operator DT() const;

  bitfield& operator=(DT value);
  bitfield& operator =(const bitfield& b);

  bitfield& operator+=(DT value);
  bitfield& operator-=(DT value);
  bitfield& operator/=(DT value);
  bitfield& operator*=(DT value);
  bitfield& operator%=(DT value);
  bitfield& operator^=(DT value);
  bitfield& operator&=(DT value);
  bitfield& operator|=(DT value);
  bitfield& operator<<=(DT value);
  bitfield& operator>>=(DT value);

  bitfield& operator--();
  DT operator--(int);
  bitfield& operator++();
  DT operator++(int);

  virtual void set_read_callback(bitfield_read_callback_base<DT>* cb) override;
  virtual void set_write_callback(bitfield_write_callback_base<DT>* cb) override;
  virtual void set_post_write_callback(bitfield_post_write_callback_base<DT>* cb) override;
  virtual void set_post_write_callback(DT enum_val, bitfield_post_write_callback_base<DT>* cb) override;
  virtual void set_default_enum_write_callback(bitfield_post_write_callback_base<DT>* cb) override;
  virtual void remove_callback() override;
  virtual void remove_read_callback() override;
  virtual void remove_write_callback() override;
  virtual void remove_post_write_callback() override;
  bitfield_read_callback_base<DT>* get_read_callback() const override;
  bitfield_write_callback_base<DT>* get_write_callback() const override;

  virtual void set_debug_read_callback(bitfield_debug_read_callback_base<DT>* cb) override;
  virtual void set_debug_write_callback(bitfield_debug_write_callback_base<DT>* cb) override;

  virtual void remove_debug_callback() override;
  virtual void remove_debug_read_callback() override;
  virtual void remove_debug_write_callback() override;

  virtual bitfield_debug_read_callback_base<DT>* get_debug_read_callback() const override;
  virtual bitfield_debug_write_callback_base<DT>* get_debug_write_callback() const override;

  void set_restriction(bitfield_access_restriction_base<DT>* r) override;
  void set_read_restriction(bitfield_access_restriction_base<DT>* r) override;
  void set_write_restriction(bitfield_access_restriction_base<DT>* r) override;

  bitfield_access_restriction_base<DT>* get_read_restriction() const override;
  bitfield_access_restriction_base<DT>* get_write_restriction() const override;

  void remove_restriction() override;
  void remove_read_restriction() override;
  void remove_write_restriction() override;

  bool has_default_read_restriction() const;
  bool has_default_write_restriction() const;

  bool has_default_read_behavior() const;
  bool has_default_write_behavior() const;
  bool has_default_debug_read_behavior() const;
  bool has_default_debug_write_behavior() const;
  bool has_never_syncing_read_behavior() const;
  bool has_never_syncing_write_behavior() const;
  bool has_post_write_behavior() const;

  virtual bool register_callback_observer(bitfield_callback_observer<DT>* o) override;
  virtual bool unregister_all_callback_observers() override;
  virtual bool unregister_callback_observer(bitfield_callback_observer<DT>* o) override;

  // sc_object
  virtual const char* kind() const;

  using bitfield_base::set_symbolic_name;
  bool set_symbolic_name(const DT& value, const std::string& name);
  using bitfield_base::get_symbolic_name;
  std::string get_symbolic_name(const DT& value);

  const reg<DT>& get_register() const;
  reg<DT>& get_register();
  
  void store_old_value();
  DT get_old_value() const;

  bool filter(bool readNotWrite, DT& data, DT& bitMask) const;

  bool is_dmi_read_allowed() const;
  bool is_dmi_write_allowed() const;
private:
  virtual restriction_owner* get_restriction_parent() const;
  friend class bitfield_container<DT>;

  void register_bitfield(bitfield<DT>& bitfield);

  void do_set_read_callback(bitfield_read_callback_base<DT>* cb);
  void do_set_write_callback(bitfield_write_callback_base<DT>* cb);
  void do_set_post_write_callback(bitfield_post_write_callback_base<DT>* cb);
  void do_set_post_write_callback(DT enumval, bitfield_post_write_callback_base<DT>* cb);
  void do_set_default_enum_write_callback(bitfield_post_write_callback_base<DT>* cb);
  void do_set_debug_read_callback(bitfield_debug_read_callback_base<DT>* cb);
  void do_set_debug_write_callback(bitfield_debug_write_callback_base<DT>* cb);

  void do_set_read_restriction(bitfield_access_restriction_base<DT>* r);
  void do_set_write_restriction(bitfield_access_restriction_base<DT>* r);

  bool execute_write_behavior(const DT& value, const DT& bitMask, sc_core::sc_time& t);
  bool execute_read_behavior(DT& value, const DT& bitMask, sc_core::sc_time& t) const;
  bool execute_debug_write_behavior(const DT& value, const DT& bitMask);
  bool execute_debug_read_behavior(DT& value, const DT& bitMask) const;
  bool execute_post_write_behavior(const DT& value, const DT& oldValue, const DT& bitMask, sc_core::sc_time& t);
  

private:
  bitfield(const bitfield&);

private:
  reg<DT>& mRegister;
  bitfield<DT>* mParentBitfield;
  bitfield_defaults mCallbacks;
  bitfield_callback_observers<DT> mCallbackObservers;
  bitfield_container<DT> mBitfieldContainer;
};

template <typename DT>
inline
void
bitfield<DT>::
put(const DT& value)
{
  DT data = insert_bits(mRegister.get_debug(), value, mSize, mOffset);
  mRegister.put(data);
}

template <typename DT>
inline
void
bitfield<DT>::
put(const DT& value, const DT& bitMask)
{
  const DT maskedValue = (get_debug() & ~bitMask) | (value & bitMask);
  DT data = insert_bits(mRegister.get_debug(), maskedValue, mSize, mOffset);
  mRegister.put(data);
}

template <typename DT>
inline
DT
bitfield<DT>::
get() const
{
  return extract_bits(mRegister.get(), mSize, mOffset);
}

template <typename DT>
inline
DT
bitfield<DT>::
get(const DT& bitMask) const
{
  return extract_bits(mRegister.get(), mSize, mOffset) & bitMask;
}

template <typename DT>
inline
void
bitfield<DT>::
put_debug(const DT& value)
{
  DT data = insert_bits(mRegister.get_debug(), value, mSize, mOffset);
  mRegister.put_debug(data);
}

template <typename DT>
inline
void
bitfield<DT>::
put_debug(const DT& value, const DT& bitMask)
{
  const DT maskedValue = (get_debug() & ~bitMask) | (value & bitMask);
  DT data = insert_bits(mRegister.get_debug(), maskedValue, mSize, mOffset);
  mRegister.put_debug(data);
}

template <typename DT>
inline
DT
bitfield<DT>::
get_debug() const
{
  return extract_bits(mRegister.get_debug(), mSize, mOffset);
}

template <typename DT>
inline
DT
bitfield<DT>::
get_debug(const DT& bitMask) const
{
  return extract_bits(mRegister.get_debug(), mSize, mOffset) & bitMask;
}

template <typename DT>
inline
bool
bitfield<DT>::
put_with_triggering_callbacks(const DT& value, sc_core::sc_time& t)
{
  return put_with_triggering_callbacks(value, get_bit_mask<DT>(mSize), t);
}

template <typename DT>
inline
bool
bitfield<DT>::
put_with_triggering_callbacks(const DT& value, const DT& bitMask, sc_core::sc_time& t)
{
  return mRegister.put_with_triggering_bitfield_callbacks(
           insert_bits<DT>(0, value, mSize, mOffset),
           insert_bits<DT>(0, bitMask, mSize, mOffset),
           t);
}

template <typename DT>
inline
bool
bitfield<DT>::
get_with_triggering_callbacks(DT& value, sc_core::sc_time& t) const
{
  return get_with_triggering_callbacks(value, get_bit_mask<DT>(mSize), t);
}

template <typename DT>
inline
bool
bitfield<DT>::
get_with_triggering_callbacks(DT& value, const DT& bitMask, sc_core::sc_time& t) const
{
  DT readData = insert_bits<DT>(0, value, mSize, mOffset);
  bool result = mRegister.get_with_triggering_bitfield_callbacks(
                  readData,
                  insert_bits<DT>(0, bitMask, mSize, mOffset),
                  t);
  value = extract_bits(readData, mSize, mOffset);
  return result;
}

template <typename DT>
inline
bool
bitfield<DT>::
put_debug_with_triggering_callbacks(const DT& value)
{
  return put_debug_with_triggering_callbacks(value, get_bit_mask<DT>(mSize));
}

template <typename DT>
inline
bool
bitfield<DT>::
put_debug_with_triggering_callbacks(const DT& value, const DT& bitMask)
{
  return mRegister.put_debug_with_triggering_bitfield_callbacks(
           insert_bits<DT>(0, value, mSize, mOffset),
           insert_bits<DT>(0, bitMask, mSize, mOffset));
}

template <typename DT>
inline
bool
bitfield<DT>::
get_debug_with_triggering_callbacks(DT& value) const
{
  return get_debug_with_triggering_callbacks(value, get_bit_mask<DT>(mSize));
}

template <typename DT>
inline
bool
bitfield<DT>::
get_debug_with_triggering_callbacks(DT& value, const DT& bitMask) const
{
  DT readData = insert_bits<DT>(0, value, mSize, mOffset);
  bool result = mRegister.get_debug_with_triggering_bitfield_callbacks(
                  readData,
                  insert_bits<DT>(0, bitMask, mSize, mOffset));
  value = extract_bits(readData, mSize, mOffset);
  return result;
}

template <typename DT>
inline
bool
bitfield<DT>::
execute_write_behavior(const DT& value, const DT& bitMask, sc_core::sc_time& t)
{
  assert(mCallbacks.mWriteCallback);
  unsigned int id =
    mCallbackObservers.notify_begin(&value, &bitMask, ACCESS_TYPE_WRITE, ACCESS_MODE_REGULAR);
  scoped_reference<bitfield_write_callback_base<DT> > scopedRef(mCallbacks.mWriteCallback);
  bool result = mCallbacks.mWriteCallback->write(value, bitMask, t);
  mCallbackObservers.notify_end(0, id);
  return result;
}

template <typename DT>
inline
bool
bitfield<DT>::
execute_read_behavior(DT& value, const DT& bitMask, sc_core::sc_time& t) const
{
  assert(mCallbacks.mReadCallback);
  unsigned int id =
    mCallbackObservers.notify_begin(0, &bitMask, ACCESS_TYPE_READ, ACCESS_MODE_REGULAR);
  scoped_reference<bitfield_read_callback_base<DT> > scopedRef(mCallbacks.mReadCallback);
  bool result = mCallbacks.mReadCallback->read(value, bitMask, t);
  mCallbackObservers.notify_end(&value, id);
  return result;
}

template <typename DT>
inline
bool
bitfield<DT>::
execute_debug_write_behavior(const DT& value, const DT& bitMask)
{
  assert(mCallbacks.mDebugWriteCallback);
  unsigned int id = 
    mCallbackObservers.notify_begin(&value, &bitMask, ACCESS_TYPE_WRITE, ACCESS_MODE_DEBUG);
  scoped_reference<bitfield_debug_write_callback_base<DT> > scopedRef(mCallbacks.mDebugWriteCallback);
  bool result = mCallbacks.mDebugWriteCallback->write(value, bitMask);
  mCallbackObservers.notify_end(0, id);
  return result;
}

template <typename DT>
inline
bool
bitfield<DT>::
execute_debug_read_behavior(DT& value, const DT& bitMask) const
{
  assert(mCallbacks.mDebugReadCallback);
  unsigned int id =
    mCallbackObservers.notify_begin(0, &bitMask, ACCESS_TYPE_READ, ACCESS_MODE_DEBUG);
  scoped_reference<bitfield_debug_read_callback_base<DT> > scopedRef(mCallbacks.mDebugReadCallback);
  bool result = mCallbacks.mDebugReadCallback->read(value, bitMask);
  mCallbackObservers.notify_end(0, id);
  return result;
}

template <typename DT>
inline
bool
bitfield<DT>::
execute_post_write_behavior(const DT& oldValue, const DT& value, const DT& bitMask, sc_core::sc_time& t) 
{
  assert(mCallbacks.mPostWriteCallback);
  unsigned int id =
  mCallbackObservers.notify_begin(&value, &bitMask, ACCESS_TYPE_WRITE, ACCESS_MODE_REGULAR);
  scoped_reference<bitfield_post_write_callback_base<DT> > scopedRef(mCallbacks.mPostWriteCallback);
  bool result = mCallbacks.mPostWriteCallback->post_write(oldValue, value, t);
  mCallbackObservers.notify_end(0, id);
  return result;
}

template <typename DT>
inline
bitfield<DT>::
operator DT() const
{
  return get();
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator=(DT value)
{
  put(value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator =(const bitfield& b)
{
  put(b);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator+=(DT value)
{
  put(get() + value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator-=(DT value)
{
  put(get() - value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator/=(DT value)
{
  put(get() / value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator*=(DT value)
{
  put(get() * value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator%=(DT value)
{
  put(get() % value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator^=(DT value)
{
  put(get() ^ value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator&=(DT value)
{
  put(get() & value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator|=(DT value)
{
  put(get() | value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator<<=(DT value)
{
  put(get() << value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator>>=(DT value)
{
  put(get() >> value);
  return *this;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator--()
{
  put(get() - 1);
  return *this;
}

template <typename DT>
inline
DT
bitfield<DT>::
operator--(int)
{
  DT tmp = *this;
  --(*this);
  return tmp;
}

template <typename DT>
inline
bitfield<DT>&
bitfield<DT>::
operator++()
{
  put(get() + 1);
  return *this;
}

template <typename DT>
inline
DT
bitfield<DT>::
operator++(int)
{
  DT tmp = *this;
  ++(*this);
  return tmp;
}

template <typename DT>
inline
bool
bitfield<DT>::
has_default_read_behavior() const
{
  return mCallbacks.mReadCallback->has_default_behavior();
}

template <typename DT>
inline
bool
bitfield<DT>::
has_default_write_behavior() const
{
  return mCallbacks.mWriteCallback->has_default_behavior();
}

template <typename DT>
inline
bool
bitfield<DT>::
has_default_read_restriction() const
{
  return mCallbacks.mReadRestriction->has_default_restriction();
}

template <typename DT>
inline
bool
bitfield<DT>::
has_default_write_restriction() const
{
  return mCallbacks.mWriteRestriction->has_default_restriction();
}

template <typename DT>
inline
bool
bitfield<DT>::
has_default_debug_read_behavior() const
{
  return mCallbacks.mDebugReadCallback->has_default_behavior();
}

template <typename DT>
inline
bool
bitfield<DT>::
has_default_debug_write_behavior() const
{
  return mCallbacks.mDebugWriteCallback->has_default_behavior();
}

template <typename DT>
inline
bool
bitfield<DT>::has_post_write_behavior() const
{
  return !mCallbacks.mPostWriteCallback->has_default_behavior();
}

template <typename DT>
inline
bool
bitfield<DT>::
has_never_syncing_read_behavior() const
{
  return mCallbacks.mReadCallback->has_never_syncing_behavior();
}

template <typename DT>
inline
bool
bitfield<DT>::
has_never_syncing_write_behavior() const
{
  return mCallbacks.mWriteCallback->has_never_syncing_behavior();
}

#define SCML2_INSTANTIATE_TEMPLATE(type)	\
   SNPS_VP_API_TMPL_CLASS(bitfield<type >;)
SCML2_FOR_EACH_DT(SCML2_INSTANTIATE_TEMPLATE)
#undef SCML2_INSTANTIATE_TEMPLATE

}

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
