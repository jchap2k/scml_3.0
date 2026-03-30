/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_REG_VECTOR_CREATOR_H
#define SCML2_REG_VECTOR_CREATOR_H

#include <scml2/reg.h>

#include <tuple>
#include <vector>
#include <type_traits>

namespace scml2 {

class toplevel_memory_base;
template <typename DT> class memory;
template <typename DT> class memory_alias;

class toplevel_memory_base_proxy;
class memory_alias_base_proxy;

namespace {
  template<bool, typename T> struct tuple_type_helper {
    typedef T& type;
  };

  template<typename T> struct tuple_type_helper<true, T> {
    typedef T type;
  };
}

class vector_creator_index_registry {
public:
  typedef size_t index_t;
private:
  
  std::vector<index_t> m_indices;
  vector_creator_index_registry() {}
public:
  static vector_creator_index_registry& get_instance() {static vector_creator_index_registry s_instance; return s_instance;}

  void push_index(index_t idx) {m_indices.push_back(idx);}
  void pop_index() {m_indices.pop_back();}
  index_t get_index(size_t level = 0) const {return m_indices[m_indices.size()-1-level];}
};

class vector_creator_index {
public:
  vector_creator_index(vector_creator_index_registry::index_t idx) {
    vector_creator_index_registry::get_instance().push_index(idx);
  }
  ~vector_creator_index() {
    vector_creator_index_registry::get_instance().pop_index();
  }
  vector_creator_index(const vector_creator_index&) = delete;
  void operator=(const vector_creator_index&) = delete;
};
  
template <typename DT, typename PARENT_TYPE, typename T, typename... Args>
class reg_vector_creator_with_extra_args
{
  template<int ...>
  struct seq { };
  
  template<int N, int ...S>
  struct gens : gens<N-1, N-1, S...> { };
  
  template<int ...S>
  struct gens<0, S...> {
    typedef seq<S...> type;
  };
public:
  reg_vector_creator_with_extra_args(PARENT_TYPE& parent, size_t offset, size_t size, Args&&... args) : mParent(parent), mOffset(offset), mSize(size), mArgs(args...) {
  }

  T* operator()(const char* prefix, size_t i) {
    return callFunc(prefix, i, typename gens<sizeof...(Args)>::type());
  }

private:
  template<int... S>
  T* callFunc(const char* prefix, size_t i, seq<S...>) {
    vector_creator_index idx(i);
    return new T(prefix, mParent, mOffset + i*mSize, std::get<S>(mArgs)...);    
  }

  PARENT_TYPE& mParent;
  size_t mOffset;
  size_t mSize;
  std::tuple<typename tuple_type_helper<std::is_trivially_copyable<Args>::value, Args>::type...> mArgs;
};


template <typename DT, typename T, typename... Args>
inline reg_vector_creator_with_extra_args<DT, scml2::toplevel_memory_base, T, Args...>
create_reg_vector_creator(scml2::toplevel_memory_base& parent, size_t offset, size_t size, Args&&... args) {
  return reg_vector_creator_with_extra_args<DT, scml2::toplevel_memory_base, T, Args...>(parent, offset, size, std::forward<Args>(args)...);
}

template <typename DT, typename T, typename... Args>
inline reg_vector_creator_with_extra_args<DT, scml2::memory_alias_base, T, Args...>
create_reg_vector_creator(scml2::memory_alias_base& parent, size_t offset, size_t size, Args&&... args) {
  return reg_vector_creator_with_extra_args<DT, scml2::memory_alias_base, T, Args...>(parent, offset, size, std::forward<Args>(args)...);
}

// proxies
template <typename DT, typename T, typename... Args>
inline reg_vector_creator_with_extra_args<DT, scml2::toplevel_memory_base_proxy, T, Args...>
create_reg_vector_creator(scml2::toplevel_memory_base_proxy& parent, size_t offset, size_t size, Args&&... args) {
  return reg_vector_creator_with_extra_args<DT, scml2::toplevel_memory_base_proxy, T, Args...>(parent, offset, size, std::forward<Args>(args)...);
}

template <typename DT, typename T, typename... Args>
inline reg_vector_creator_with_extra_args<DT, scml2::memory_alias_base_proxy, T, Args...>
create_reg_vector_creator(scml2::memory_alias_base_proxy& parent, size_t offset, size_t size, Args&&... args) {
  return reg_vector_creator_with_extra_args<DT, scml2::memory_alias_base_proxy, T, Args...>(parent, offset, size, std::forward<Args>(args)...);
}

// only for backward compatibility with old generated code where TLMC is emitting reg derived classes that accept the DT-variants of the parents instead of the base classes
template <typename DT, typename T, typename... Args>
inline reg_vector_creator_with_extra_args<DT, scml2::memory<DT>, T, Args...>
create_reg_vector_creator(scml2::memory<DT>& parent, size_t offset, size_t size, Args&&... args) {
  return reg_vector_creator_with_extra_args<DT, scml2::memory<DT>, T, Args...>(parent, offset, size, std::forward<Args>(args)...);
}

template <typename DT, typename T, typename... Args>
  inline reg_vector_creator_with_extra_args<DT, scml2::memory_alias<DT>, T, Args...>
create_reg_vector_creator(scml2::memory_alias<DT>& parent, size_t offset, size_t size, Args&&... args) {
  return reg_vector_creator_with_extra_args<DT, scml2::memory_alias<DT>, T, Args...>(parent, offset, size, std::forward<Args>(args)...);
}

// only for backward compatibility with old generated code where TLMC is emitting different function name
template <typename DT, typename T, typename... Args>
inline reg_vector_creator_with_extra_args<DT, scml2::memory_alias_base, T, Args...>
create_reg_alias_vector_creator(scml2::memory_alias_base& parent, size_t offset, size_t size, Args&&... args) {
  return create_reg_vector_creator<DT, T, Args...> (parent, offset, size, std::forward<Args>(args)...);
}

template <typename DT, typename T, typename... Args>
inline reg_vector_creator_with_extra_args<DT, scml2::memory_alias<DT>, T, Args...>
create_reg_alias_vector_creator(scml2::memory_alias<DT>& parent, size_t offset, size_t size, Args&&... args) {
  return create_reg_vector_creator<DT, T, Args...> (parent, offset, size, std::forward<Args>(args)...);
}

}

#endif
