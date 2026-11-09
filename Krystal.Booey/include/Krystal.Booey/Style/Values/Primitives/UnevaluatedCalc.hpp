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

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/Concepts.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/IntrusivePtr.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style::calc
{
  class Value;
  struct Child;
}

namespace krys::boo::style
{
  // Non-generic base type to allow code sharing and out-of-line definitions.
  struct UnevaluatedCalcBase
  {
  private:
    Ref<calc::Value> _calc;

  public:
    explicit UnevaluatedCalcBase(calc::Value &) noexcept;
    explicit UnevaluatedCalcBase(Ref<calc::Value> &&) noexcept;
    explicit UnevaluatedCalcBase(calc::Child &&, css::Category, css::Range) noexcept;

    UnevaluatedCalcBase(const UnevaluatedCalcBase &) noexcept;
    UnevaluatedCalcBase(UnevaluatedCalcBase &&) noexcept;
    UnevaluatedCalcBase &operator=(const UnevaluatedCalcBase &) noexcept;
    UnevaluatedCalcBase &operator=(UnevaluatedCalcBase &&) noexcept;

    ~UnevaluatedCalcBase() noexcept;

    KRYS_NODISCARD Ref<calc::Value> ProtectedCalculation() const noexcept;

    KRYS_NODISCARD bool Equal(const UnevaluatedCalcBase &) const noexcept;
  };

  // Wrapper for `Ref<calc::Value>` that includes range and category as part of the type.
  template <css::Numeric CSSType>
  struct UnevaluatedCalc : UnevaluatedCalcBase
  {
    using UnevaluatedCalcBase::UnevaluatedCalcBase;
    using UnevaluatedCalcBase::operator=;

    using CSS = CSSType;
    constexpr static auto Range = CSS::Range;
    constexpr static auto Category = CSS::Category;

    explicit UnevaluatedCalc(calc::Value &calculationValue) noexcept : UnevaluatedCalcBase(calculationValue)
    {
    }

    explicit UnevaluatedCalc(calc::Child &&child) noexcept
        : UnevaluatedCalcBase(krys::move(child), Category, Range)
    {
    }

    KRYS_NODISCARD bool operator==(const UnevaluatedCalc &other) const noexcept
    {
      return UnevaluatedCalcBase::Equal(static_cast<const UnevaluatedCalcBase &>(other));
    }
  };
}

namespace krys
{
  template <boo::style::Calc T>
  constexpr bool IsSmartPtr<T> = true;
}
