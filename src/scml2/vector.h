/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_VECTOR_H
#define SCML2_VECTOR_H

#include <vector>
#include <sstream>
#include <systemc>

namespace scml2 {

/*
 * This class is an enhancement of sc_core::sc_vector in the sense that the vector itself is not
 *  named with the same basename as the vector elements. This avoids the off-by-one element name 
 *  numbering, which sc_vector currently honors.
 */
template <typename T>
class vector : public sc_core::sc_object
{
public:
 explicit vector(const char* name) : sc_core::sc_object(sc_core::sc_gen_unique_name((name + std::string("_vector")).c_str(), true)), mElementName(name) {
       static sc_core::sc_attribute<bool> s_vector_attribute{"scml2::vector", true};
       this->add_attribute(s_vector_attribute);
  }

 vector(const char* name, size_t size) : sc_core::sc_object(sc_core::sc_gen_unique_name((name + std::string("_vector")).c_str(), true)), mElementName(name) {
    static sc_core::sc_attribute<bool> s_vector_attribute{"scml2::vector", true};
    this->add_attribute(s_vector_attribute);
    init(size);
  }

  template<typename Creator>
    vector(const char* name, size_t size, Creator creator)
      : sc_core::sc_object(sc_core::sc_gen_unique_name((name + std::string("_vector")).c_str(), true)), mElementName(name) {
    static sc_core::sc_attribute<bool> s_vector_attribute{"scml2::vector", true};
    this->add_attribute(s_vector_attribute);
    init(size, creator);
  }

  ~vector() {
    size_t i = mElements.size();
    while (i-- > 0) {
      delete mElements[i];
    }
    mElements.clear();
  }

  class const_iterator;

  class iterator {
  private:
    friend class const_iterator;
    scml2::vector<T>& m_oa;
    unsigned long long m_index;
  public:
    iterator(scml2::vector<T>& oa, unsigned long long index) : m_oa(oa), m_index(index) {}
    T& operator*() const { return m_oa[m_index]; }
    iterator& operator++() {++m_index; return *this;}
    bool operator==(const iterator& rhs) const {return &m_oa==&(rhs.m_oa) && m_index==rhs.m_index;}
    bool operator!=(const iterator& rhs) const {return !((*this)==rhs);}
  };

  iterator begin() {return iterator(*this, 0);}
  iterator end() {return iterator(*this, mElements.size());}

  class const_iterator {
  private:
    const scml2::vector<T>& m_oa;
    unsigned long long m_index;
  public:
    const_iterator(const scml2::vector<T>& oa, unsigned long long index) : m_oa(oa), m_index(index) {}
    const_iterator(const iterator& other) : m_oa(other.m_oa), m_index(other.m_index) {}
    const T& operator*() const { return m_oa[m_index]; }
    const_iterator& operator++() {++m_index; return *this;}
    bool operator==(const const_iterator& rhs) const {return &m_oa==&(rhs.m_oa) && m_index==rhs.m_index;}
    bool operator!=(const const_iterator& rhs) const {return !((*this)==rhs);}
  };

  const_iterator begin() const {return const_iterator(*this, 0);}
  const_iterator end() const {return const_iterator(*this, mElements.size());}

  const T& operator[](size_t i) const {
    return *(mElements[i]);
  }

  T& operator[](size_t i) {
    return *(mElements[i]);
  }

  size_t size() const {
    return mElements.size();
  }

  void init(size_t size) {
    init(size, &vector<T>::create_element);
  }

  template<typename Creator>
  void init(size_t size, Creator creator) {
    mElements.resize(size);
    for (size_t i = 0; i < size; ++i) {
      mElements[i] = creator(sc_core::sc_gen_unique_name(mElementName.c_str()), i);
    }
  }


private:
  static T* create_element(const char* prefix, size_t /*i*/) {
    return new T(prefix);
  }

private:
  std::string mElementName;
  std::vector<T*> mElements;
};

}

#endif
