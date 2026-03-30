/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML_PROPERTY_VECTOR_CREATOR_H_INCLUDED__
#define SCML_PROPERTY_VECTOR_CREATOR_H_INCLUDED__

#include "scml2/reg_vector_creator.h"
#include <functional>

namespace scml2 {

template <typename T, typename DT>
class scml_property_vector_creator {
public:
  scml_property_vector_creator(std::function<DT()> f) : valueGetter(f) {}

  T* operator()(const char* prefix, size_t i) {
    return create(prefix, i);
  }
private:
  T* create(const char* prefix, size_t i) {
    scml2::vector_creator_index idx(i);
    std::string name(std::string(prefix).substr(0, std::string(prefix).find_last_of('_')) + '[' + std::to_string(i) + ']');
    return new T(name.c_str(), valueGetter());
  }
  std::function<DT()> valueGetter;
};

template <typename T, typename DT>
inline scml_property_vector_creator<T, DT> create_scml_property_vector_creator(std::function<DT()> valueGetter) {
  return scml_property_vector_creator<T, DT>(valueGetter);
}

template<typename T, typename C>
class vector_creator {
public:
  vector_creator(size_t size, C elementCreator)
    : mSize(size)
    , mElementCreator(elementCreator)
  {
  }

  T* operator()(const char* prefix, size_t i) {
    return create(prefix, i);
  }
private:
  T* create(const char* prefix, size_t i) {
    scml2::vector_creator_index idx(i);
    std::string name(std::string(prefix).substr(0, std::string(prefix).find_last_of('_')) + '[' + std::to_string(i) + ']');
    return new T(name.c_str(), mSize, mElementCreator);
  }
  size_t mSize;
  C mElementCreator;
};

template <typename T, typename C>
inline vector_creator<T, C> create_vector_creator(size_t size, C creator) {
  return vector_creator<T, C>(size, creator);
}

} /* namespace scml2 */

#endif /* SCML_PROPERTY_VECTOR_CREATOR_H_INCLUDED__ */
