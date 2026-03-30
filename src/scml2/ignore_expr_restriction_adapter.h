/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_IGNORE_EXPR_RESTRICTION_ADAPTER_H
#define SCML2_IGNORE_EXPR_RESTRICTION_ADAPTER_H

#include <scml2/types.h>

#include <functional>
#include <cassert>

namespace tlm {
  class tlm_generic_payload;
}

namespace scml2 {

template <typename DT>
class ignore_expr_restriction_adapter
{
public:
  typedef std::function<bool()> RestrictionFunction;

public:
  ignore_expr_restriction_adapter(const RestrictionFunction& f, DT value);

  access_restriction_result operator()(tlm::tlm_generic_payload& trans);
  access_restriction_result operator()(DT& data, DT& bit_enables);
private:
  RestrictionFunction mRestrictionFunction;
  DT m_value;
};

template <typename DT>
inline
ignore_expr_restriction_adapter<DT>
create_ignore_expr_restriction_adapter(const typename ignore_expr_restriction_adapter<DT>::RestrictionFunction& f, DT value = DT())
{
  return ignore_expr_restriction_adapter<DT>(f, value);
}

template <typename DT>
inline
ignore_expr_restriction_adapter<DT>::
ignore_expr_restriction_adapter(const RestrictionFunction& f, DT value) 
  : mRestrictionFunction(f)
  , m_value(value)
{}

template <typename DT>
inline
access_restriction_result
ignore_expr_restriction_adapter<DT>::
operator()(tlm::tlm_generic_payload& trans)
{
  return mRestrictionFunction() ?
    restrict_all(trans, m_value) : scml2::RESTRICT_NO_ERROR;
}

template <typename DT>
inline
access_restriction_result
ignore_expr_restriction_adapter<DT>::
operator()(DT& data, DT& bit_enables)
{
  return mRestrictionFunction() ?
    restrict_all(data, bit_enables, m_value) : scml2::RESTRICT_NO_ERROR;
}

}

#endif
