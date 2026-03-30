/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2/bitfield.h>

#include <scml2/bitfield_read_callback_base.h>
#include <scml2/bitfield_write_callback_base.h>
#include <scml2/bitfield_debug_write_callback_base.h>
#include <scml2/bitfield_debug_read_callback_base.h>
#include <scml2/bitfield_enum_post_write_decoder.h>
#include <scml2/bitfield_default_restriction.h>
#include <scml2/reg.h>

#include <cassert>
#include <iostream>
#include <cstdlib>

namespace scml2 {

template <typename DT>
bitfield<DT>::
bitfield(const std::string& name,
         reg<DT>& r,
         unsigned int offset,
         unsigned int size) :
  bitfield_base(name, offset, size, r.get_streams()),
  tagged_message_handler_provider(this),
  mRegister(r),
  mParentBitfield(0),
  mCallbacks(*this)
{
  if (size > 8*sizeOf<DT>()) {
    SCML2_ERROR(GENERIC_ERROR) << " size " << size << " of bitfield '" << this->name() << "' in register '" << r.name() << "' is larger than datatype width " << sizeOf<DT>() << std::endl;
  }
  mRegister.register_bitfield(*this);
}

template <typename DT>
restriction_owner* 
bitfield<DT>::
get_restriction_parent() const
{
  if (mParentBitfield != 0) {
    return mParentBitfield;
  } else {
    return &mRegister;
  }
}

template <typename DT>
const reg<DT>& 
bitfield<DT>::get_register() const
{
  return mRegister;
}

template <typename DT>
reg<DT>& 
bitfield<DT>::get_register()
{
  return mRegister;
}

template <typename DT>
void 
bitfield<DT>::store_old_value() {
  std::cerr << "illegal use of undocumented API bitfield<DT>::store_old_value()" << std::endl;
  abort();
}

template <typename DT>
DT 
bitfield<DT>::get_old_value() const{
  std::cerr << "illegal use of undocumented API bitfield<DT>::get_old_value()" << std::endl;
  abort();
  return (DT)0;
}

template <typename DT>
bitfield<DT>::
bitfield(const std::string& name,
         bitfield<DT>& b,
         unsigned int offset,
         unsigned int size) :
  bitfield_base(name, b.get_offset() + offset, size, b.get_streams()),
  tagged_message_handler_provider(this),
  mRegister(b.mRegister),
  mParentBitfield(&b),
  mCallbacks(*this)
{
  mParentBitfield->register_bitfield(*this);
}
  
template <typename DT>
bitfield<DT>::
~bitfield()
{
}

template <typename DT>
void
bitfield<DT>::
register_bitfield(bitfield<DT>& bitfield)
{
  if ((bitfield.get_offset() + bitfield.get_size()) > (get_offset() + get_size())) {
    SCML2_LOG_ERROR(this, get_streams().error(), CONFIGURATION_ERROR)
      << "The bitfield '" << bitfield.get_name()
      << "' addresses bits outside the range of the parent bitfield '"
      << get_name() << "'"
      << std::endl;
  }

  mBitfieldContainer.register_bitfield(bitfield);
}

template <typename DT>
bool
bitfield<DT>::
set_symbolic_name(const DT& value, const std::string& name)
{
  std::vector<unsigned char> tmp(sizeOf<DT>());
  to_char_array(value, &tmp[0], sizeOf<DT>(), 0);
  return set_symbolic_name(tmp, name);
}

template <typename DT>
std::string
bitfield<DT>::
get_symbolic_name(const DT& value)
{
  std::vector<unsigned char> tmp(sizeOf<DT>());
  to_char_array(value, &tmp[0], sizeOf<DT>(), 0);
  return get_symbolic_name(tmp);
}

template <typename DT>
void
bitfield<DT>::
do_set_read_callback(bitfield_read_callback_base<DT>* cb)
{
  assert(mCallbacks.mReadCallback);
  assert(cb);
  mCallbacks.mReadCallback->unref();
  mCallbacks.mReadCallback = cb;
  mCallbacks.mReadCallback->ref();
}

template <typename DT>
void
bitfield<DT>::
do_set_write_callback(bitfield_write_callback_base<DT>* cb)
{
  assert(mCallbacks.mWriteCallback);
  assert(cb);
  mCallbacks.mWriteCallback->unref();
  mCallbacks.mWriteCallback = cb;
  mCallbacks.mWriteCallback->ref();
}

template <typename DT>
void
bitfield<DT>::
do_set_read_restriction(bitfield_access_restriction_base<DT>* r)
{
  assert(mCallbacks.mReadRestriction);
  assert(r);
  if (!mCallbacks.mReadRestriction->has_default_restriction()) {
    read_restriction_removed();
  }
  mCallbacks.mReadRestriction->unref();
  mCallbacks.mReadRestriction = r;
  if (!mCallbacks.mReadRestriction->has_default_restriction()) {
    read_restriction_added();
  }
  mCallbacks.mReadRestriction->ref();
}

template <typename DT>
void
bitfield<DT>::
do_set_write_restriction(bitfield_access_restriction_base<DT>* r)
{
  assert(mCallbacks.mWriteRestriction);
  assert(r);
  if (!mCallbacks.mWriteRestriction->has_default_restriction()) {
    write_restriction_removed();
  }
  mCallbacks.mWriteRestriction->unref();
  mCallbacks.mWriteRestriction = r;
  if (!mCallbacks.mWriteRestriction->has_default_restriction()) {
    write_restriction_added();
  }
  mCallbacks.mWriteRestriction->ref();
}

template <typename DT>
void
bitfield<DT>::
do_set_post_write_callback(bitfield_post_write_callback_base<DT>* cb)
{
  assert(mCallbacks.mPostWriteCallback);
  assert(cb);
  mCallbacks.mPostWriteCallback->unref();
  mCallbacks.mPostWriteCallback = cb;
  mCallbacks.mPostWriteCallback->ref();
}

template <typename DT>
void
bitfield<DT>::
// coverity[pass_by_value]
do_set_post_write_callback(DT enumval, bitfield_post_write_callback_base<DT>* cb){
  //check if we the current callback is an enum decoder callback. 
  assert(mCallbacks.mPostWriteCallback);
  assert(cb);
  if (! dynamic_cast< bitfield_enum_post_write_decoder<DT> * > (mCallbacks.mPostWriteCallback)) {
    //we don't have a decoder yet. instantiate one.
    mCallbacks.mPostWriteCallback->unref();
    mCallbacks.mPostWriteCallback = new bitfield_enum_post_write_decoder<DT>("");
    mCallbacks.mPostWriteCallback->ref();
  } 
  bitfield_enum_post_write_decoder<DT> * decoder =  dynamic_cast< bitfield_enum_post_write_decoder<DT> *> (mCallbacks.mPostWriteCallback);
  if (decoder) {
    cb->ref();
    decoder->register_decode_cb(enumval, cb);
  } else {
    //this is utterly inexplicable.
    assert(0);
  }
}

template <typename DT>
void
bitfield<DT>::
do_set_default_enum_write_callback(bitfield_post_write_callback_base<DT>* cb){
  //check if we the current callback is an enum decoder callback. 
  assert(mCallbacks.mPostWriteCallback);
  assert(cb);
  if (! dynamic_cast< bitfield_enum_post_write_decoder<DT> * > (mCallbacks.mPostWriteCallback)) {
    //we don't have a decoder yet. instantiate one.
    mCallbacks.mPostWriteCallback->unref();
    mCallbacks.mPostWriteCallback = new bitfield_enum_post_write_decoder<DT>("");
    mCallbacks.mPostWriteCallback->ref();
  } else {
  }
  bitfield_enum_post_write_decoder<DT> * decoder =  dynamic_cast< bitfield_enum_post_write_decoder<DT> *> (mCallbacks.mPostWriteCallback);
  if (decoder) {
    cb->ref();
    decoder->register_default_cb(cb);
  } else {
    //this is utterly inexplicable.
    assert(0);
  }
}


template <typename DT>
void
bitfield<DT>::
do_set_debug_read_callback(bitfield_debug_read_callback_base<DT>* cb)
{
  assert(mCallbacks.mDebugReadCallback);
  assert(cb);
  mCallbacks.mDebugReadCallback->unref();
  mCallbacks.mDebugReadCallback = cb;
  mCallbacks.mDebugReadCallback->ref();
}

template <typename DT>
void
bitfield<DT>::
do_set_debug_write_callback(bitfield_debug_write_callback_base<DT>* cb)
{
  assert(mCallbacks.mDebugWriteCallback);
  assert(cb);
  mCallbacks.mDebugWriteCallback->unref();
  mCallbacks.mDebugWriteCallback = cb;
  mCallbacks.mDebugWriteCallback->ref();
}

template <typename DT>
void
bitfield<DT>::
set_read_callback(bitfield_read_callback_base<DT>* cb)
{
  assert(cb);
  scoped_reference<bitfield_read_callback_base<DT> > scopedRef(cb);
  if (has_default_read_behavior()) {
    mRegister.bitfield_read_callback_registered();
  }
  do_set_read_callback(cb);
  mCallbackObservers.notify_callback_changed(ACCESS_TYPE_READ, ACCESS_MODE_REGULAR);
}

template <typename DT>
void
bitfield<DT>::
set_write_callback(bitfield_write_callback_base<DT>* cb)
{
  assert(cb);
  scoped_reference<bitfield_write_callback_base<DT> > scopedRef(cb);
  if (has_default_write_behavior()) {
    mRegister.bitfield_write_callback_registered();
  }
  do_set_write_callback(cb);
  mCallbackObservers.notify_callback_changed(ACCESS_TYPE_WRITE, ACCESS_MODE_REGULAR);
}

template <typename DT>
void
bitfield<DT>::
set_restriction(bitfield_access_restriction_base<DT>* r)
{
  assert(r);
  scoped_reference<bitfield_access_restriction_base<DT> > scopedRef(r);
  if (has_default_read_restriction() || has_default_write_restriction()) {
    get_register().invalidate_dmi();
  }
  do_set_read_restriction(r);
  do_set_write_restriction(r);
}

template <typename DT>
void
bitfield<DT>::
set_read_restriction(bitfield_access_restriction_base<DT>* r)
{
  assert(r);
  scoped_reference<bitfield_access_restriction_base<DT> > scopedRef(r);
  if (has_default_read_restriction()) {
    get_register().invalidate_dmi();
  }
  do_set_read_restriction(r);
}

template <typename DT>
void
bitfield<DT>::
set_write_restriction(bitfield_access_restriction_base<DT>* r)
{
  assert(r);
  scoped_reference<bitfield_access_restriction_base<DT> > scopedRef(r);
  if (has_default_write_restriction()) {
    get_register().invalidate_dmi();
  }
  do_set_write_restriction(r);
}

template <typename DT>
void
bitfield<DT>::
set_post_write_callback(bitfield_post_write_callback_base<DT>* cb)
{
  assert(cb);
  scoped_reference<bitfield_post_write_callback_base<DT> > scopedRef(cb);
  
  mRegister.bitfield_post_write_callback_registered();

  do_set_post_write_callback(cb);
  
  mCallbackObservers.notify_callback_changed(ACCESS_TYPE_WRITE, ACCESS_MODE_REGULAR);
}

template <typename DT>
void
bitfield<DT>::
// coverity[pass_by_value]
set_post_write_callback(DT enum_val, bitfield_post_write_callback_base<DT>* cb){
  assert(cb);
  scoped_reference<bitfield_post_write_callback_base<DT> > scopedRef(cb);
  mRegister.bitfield_post_write_callback_registered();

  do_set_post_write_callback(enum_val, cb);
  
  mCallbackObservers.notify_callback_changed(ACCESS_TYPE_WRITE, ACCESS_MODE_REGULAR);
}

template <typename DT>
void
bitfield<DT>::
set_default_enum_write_callback(bitfield_post_write_callback_base<DT>* cb){
  assert(cb);
  scoped_reference<bitfield_post_write_callback_base<DT> > scopedRef(cb);
  mRegister.bitfield_post_write_callback_registered();

  do_set_default_enum_write_callback(cb);
  
  mCallbackObservers.notify_callback_changed(ACCESS_TYPE_WRITE, ACCESS_MODE_REGULAR);

}

template <typename DT>
void
bitfield<DT>::
remove_callback()
{
  remove_read_callback();
  remove_write_callback();
}

template <typename DT>
void
bitfield<DT>::
remove_read_callback()
{
  if (!has_default_read_behavior()) {
    scoped_reference<bitfield_read_callback_base<DT> > scopedRef(&mCallbacks);
    do_set_read_callback(&mCallbacks);
    mRegister.bitfield_read_callback_unregistered();
    mCallbackObservers.notify_callback_changed(ACCESS_TYPE_READ, ACCESS_MODE_REGULAR);
  }
}

template <typename DT>
void
bitfield<DT>::
remove_write_callback()
{
  if (!has_default_write_behavior()) {
    scoped_reference<bitfield_write_callback_base<DT> > scopedRef(&mCallbacks);
    do_set_write_callback(&mCallbacks);
    mRegister.bitfield_write_callback_unregistered();
    mCallbackObservers.notify_callback_changed(ACCESS_TYPE_WRITE, ACCESS_MODE_REGULAR);
  }
}

template <typename DT>
void 
bitfield<DT>::
remove_post_write_callback()
{
  scoped_reference<bitfield_post_write_callback_base<DT> > scopedRef(&mCallbacks);
  mCallbacks.mPostWriteCallback->unref();
  mCallbacks.mPostWriteCallback = &mCallbacks;
  mCallbacks.mPostWriteCallback->ref();
  mRegister.bitfield_post_write_callback_unregistered();
  mCallbackObservers.notify_callback_changed(ACCESS_TYPE_WRITE, ACCESS_MODE_REGULAR);
}

template <typename DT>
void
bitfield<DT>::
remove_restriction()
{
  remove_read_restriction();
  remove_write_restriction();
}

template <typename DT>
void
bitfield<DT>::
remove_read_restriction()
{
  if (!has_default_read_restriction()) {
    scoped_reference<bitfield_access_restriction_base<DT> > scopedRef(&mCallbacks);
    do_set_read_restriction(&mCallbacks);
    get_register().invalidate_dmi();
  }
}

template <typename DT>
void
bitfield<DT>::
remove_write_restriction()
{
  if (!has_default_write_restriction()) {
    scoped_reference<bitfield_access_restriction_base<DT> > scopedRef(&mCallbacks);
    do_set_write_restriction(&mCallbacks);
    get_register().invalidate_dmi();
  }
}

template <typename DT>
bitfield_read_callback_base<DT>*
bitfield<DT>::
get_read_callback() const
{
  return mCallbacks.mReadCallback;
}

template <typename DT>
bitfield_write_callback_base<DT>*
bitfield<DT>::
get_write_callback() const
{
  return mCallbacks.mWriteCallback;
}

template <typename DT>
bitfield_access_restriction_base<DT>*
bitfield<DT>::
get_read_restriction() const
{
  return mCallbacks.mReadRestriction;
}

template <typename DT>
bitfield_access_restriction_base<DT>*
bitfield<DT>::
get_write_restriction() const
{
  return mCallbacks.mWriteRestriction;
}

template <typename DT>
void
bitfield<DT>::
set_debug_read_callback(bitfield_debug_read_callback_base<DT>* cb)
{
  assert(cb);
  scoped_reference<bitfield_debug_read_callback_base<DT> > scopedRef(cb);
  if (has_default_debug_read_behavior()) {
    mRegister.bitfield_debug_read_callback_registered();
  }
  do_set_debug_read_callback(cb);
  mCallbackObservers.notify_callback_changed(ACCESS_TYPE_READ, ACCESS_MODE_DEBUG);
}

template <typename DT>
void
bitfield<DT>::
set_debug_write_callback(bitfield_debug_write_callback_base<DT>* cb)
{
  assert(cb);
  scoped_reference<bitfield_debug_write_callback_base<DT> > scopedRef(cb);
  if (has_default_debug_write_behavior()) {
    mRegister.bitfield_debug_write_callback_registered();
  }
  do_set_debug_write_callback(cb);
  mCallbackObservers.notify_callback_changed(ACCESS_TYPE_WRITE, ACCESS_MODE_DEBUG);
}

template <typename DT>
void
bitfield<DT>::
remove_debug_callback()
{
  remove_debug_read_callback();
  remove_debug_write_callback();
}

template <typename DT>
void
bitfield<DT>::
remove_debug_read_callback()
{
  if (!has_default_debug_read_behavior()) {
    scoped_reference<bitfield_debug_read_callback_base<DT> > scopedRef(&mCallbacks);
    do_set_debug_read_callback(&mCallbacks);
    mRegister.bitfield_debug_read_callback_unregistered();
    mCallbackObservers.notify_callback_changed(ACCESS_TYPE_READ, ACCESS_MODE_DEBUG);
  }
}

template <typename DT>
void
bitfield<DT>::
remove_debug_write_callback()
{
  if (!has_default_debug_write_behavior()) {
    scoped_reference<bitfield_debug_write_callback_base<DT> > scopedRef(&mCallbacks);
    do_set_debug_write_callback(&mCallbacks);
    mRegister.bitfield_debug_write_callback_unregistered();
    mCallbackObservers.notify_callback_changed(ACCESS_TYPE_WRITE, ACCESS_MODE_DEBUG);
  }
}

template <typename DT>
bitfield_debug_read_callback_base<DT>*
bitfield<DT>::
get_debug_read_callback() const
{
  return mCallbacks.mDebugReadCallback;
}

template <typename DT>
bitfield_debug_write_callback_base<DT>*
bitfield<DT>::
get_debug_write_callback() const
{
  return mCallbacks.mDebugWriteCallback;
}

template <typename DT>
bool
bitfield<DT>::
register_callback_observer(bitfield_callback_observer<DT>* o)
{
  return mCallbackObservers.register_observer(o);
}

template <typename DT>
bool
bitfield<DT>::
unregister_all_callback_observers()
{
  return mCallbackObservers.unregister_all_observers();
}

template <typename DT>
bool
bitfield<DT>::
unregister_callback_observer(bitfield_callback_observer<DT>* o)
{
  return mCallbackObservers.unregister_observer(o);
}

template <typename DT>
const char*
bitfield<DT>::
kind() const
{
  return "scml2::bitfield";
}

template <typename DT>
bool
bitfield<DT>::
filter(bool readNotWrite, DT& data, DT& bitMask) const
{
  bitfield_access_restriction_base<DT>* restriction = readNotWrite ? 
    get_read_restriction() : get_write_restriction();
  DT bfBitMask = extract_bits<DT>(bitMask, get_size(),get_offset());
  if ((get_bit_mask<DT>(get_size()) & bfBitMask) != 0) {
    if (!restriction->has_default_restriction()) {
      DT bfData = extract_bits<DT>(data, get_size(),get_offset());
      DT orig_data = bfData;
      DT orig_bit_enables = bfBitMask;
      bool error = restriction->restrict(bfData, bfBitMask);
      DT changed_bit_enables = orig_bit_enables & (~bfBitMask);
      bfData = (orig_data & ~changed_bit_enables) | (bfData & changed_bit_enables);
      data = insert_bits<DT>(data, bfData, get_size(), get_offset());
      bitMask = insert_bits<DT>(bitMask, bfBitMask, get_size(), get_offset());
      if (error) {
	return true;
      }
    }
    if (mBitfieldContainer.filter(readNotWrite, data, bitMask)) {
      return true;
    }
  }

  return false;
}

template <typename DT>
bool 
bitfield<DT>::
is_dmi_read_allowed() const
{
  return has_default_read_restriction() && mBitfieldContainer.is_dmi_read_allowed();
}

template <typename DT>
bool 
bitfield<DT>::
is_dmi_write_allowed() const
{
  return has_default_write_restriction() && mBitfieldContainer.is_dmi_write_allowed();
}


#ifdef _WIN32
#define SCML2_INSTANTIATE_TEMPLATE(type) template class SNPS_VP_API bitfield<type >;
#else
#define SCML2_INSTANTIATE_TEMPLATE(type) template class bitfield<type >;
#endif  
SCML2_FOR_EACH_DT(SCML2_INSTANTIATE_TEMPLATE)
#undef SCML2_INSTANTIATE_TEMPLATE

}
