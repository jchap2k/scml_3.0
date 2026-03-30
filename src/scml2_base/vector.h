/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef SCML2_BASE_VECTOR_H_INCLUDED
#define SCML2_BASE_VECTOR_H_INCLUDED

#include "scml2_base/utils.h"

#ifdef SNPS_SLS_VP_BASE

#include "scml2_base/object.h"
#include "scml2_base/value_attribute.h"
#include "scml2_base/callback_macros.h"

namespace scml2 { namespace base {

    template<typename T>
    class vector
      : public atom
    {
    public:
      vector(const std::string& name, std::size_t size = 0)
	: atom(name)
	, m_name(name)
	, m_size(name + "_size")
      {
	resize_impl(size);
      }

      ~vector() {
	resize(0);
      }

      void resize(size_t size)
      {
	size_t old_size = m_size;
	get_parent()->in_construction_scope([&]() {resize_impl(size);});
	resized(old_size, size);
      }

      size_t size() const {
	return m_size;
      }

      class const_iterator;
      
      class iterator {
      private:
	friend class const_iterator;
	scml2::base::vector<T>& m_oa;
	unsigned long long m_index;
      public:
      iterator(scml2::base::vector<T>& oa, unsigned long long index) : m_oa(oa), m_index(index) {}
	T& operator*() const { return m_oa[m_index]; }
	iterator& operator++() {++m_index; return *this;}
	bool operator==(const iterator& rhs) const {return &m_oa==&(rhs.m_oa) && m_index==rhs.m_index;}
	bool operator!=(const iterator& rhs) const {return !((*this)==rhs);}
      };
      
      iterator begin() {return iterator(*this, 0);}
      iterator end() {return iterator(*this, m_elements.size());}
      
      class const_iterator {
      private:
	const scml2::base::vector<T>& m_oa;
	unsigned long long m_index;
      public:
        const_iterator(const scml2::base::vector<T>& oa, unsigned long long index) : m_oa(oa), m_index(index) {}
        const_iterator(const iterator& other) : m_oa(other.m_oa), m_index(other.m_index) {}
	const T& operator*() const { return m_oa[m_index]; }
	const_iterator& operator++() {++m_index; return *this;}
	bool operator==(const const_iterator& rhs) const {return &m_oa==&(rhs.m_oa) && m_index==rhs.m_index;}
	bool operator!=(const const_iterator& rhs) const {return !((*this)==rhs);}
      };
      
      const_iterator begin() const {return const_iterator(*this, 0);}
      const_iterator end() const {return const_iterator(*this, m_elements.size());}

      T& operator[](std::size_t idx) {
	return *m_elements[idx];
      }
      const T& operator[](std::size_t idx) const {
	return *m_elements[idx];
      }
      
      SCML2_BASE_CALLBACK_IGNORE(resized, void, size_t, size_t);
    private:
      void resize_impl(size_t size) {
	// coverity[fun_call_w_exception]
        m_size = size;
	if (size > m_elements.size()) {
	  for (size_t i = m_elements.size(); i != size; ++i) {
	    std::stringstream ss;
	    ss << m_name << "_" << i;
	    m_elements.push_back(new T(ss.str()));
	  }
	} else if (size < m_elements.size()) {
	  for (size_t i = m_elements.size()-1; i != size-1; --i) {
	    delete m_elements[i];
	  }
	  m_elements.resize(size);
	}
      }

      std::string m_name;
      value_attribute<size_t> m_size;
      std::vector<T*> m_elements;
    };

  }}

#endif // SNPS_SLS_VP_BASE

#endif // SCML2_BASE_OBJECT_H_INCLUDED
