/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"

namespace krys::boo::style
{
  UnevaluatedCalcBase::UnevaluatedCalcBase(calc::Value &value) noexcept : _calc {ShareRef(value)}
  {
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(Ref<calc::Value> &&value) noexcept : _calc {krys::move(value)}
  {
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(calc::Child &&root, css::Category category,
                                           css::Range range) noexcept
      : _calc {
          calc::Value::Create(category, css::Range {range.Min, range.Max}, calc::Tree {krys::move(root)})}
  {
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(const UnevaluatedCalcBase &) noexcept = default;
  UnevaluatedCalcBase::UnevaluatedCalcBase(UnevaluatedCalcBase &&) noexcept = default;
  UnevaluatedCalcBase &UnevaluatedCalcBase::operator=(const UnevaluatedCalcBase &) noexcept = default;
  UnevaluatedCalcBase &UnevaluatedCalcBase::operator=(UnevaluatedCalcBase &&) noexcept = default;

  UnevaluatedCalcBase::~UnevaluatedCalcBase() noexcept = default;

  Ref<calc::Value> UnevaluatedCalcBase::ProtectedCalculation() const noexcept
  {
    return _calc;
  }

  bool UnevaluatedCalcBase::Equal(const UnevaluatedCalcBase &other) const noexcept
  {
    return *_calc == *other._calc;
  }
}
