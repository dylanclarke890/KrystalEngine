/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion to `calc::Child`.

  KRYS_NODISCARD inline calc::Child CopyCalculation(Ref<calc::Value> value) noexcept
  {
    return value->CopyRoot();
  }

  KRYS_NODISCARD inline calc::Child CopyCalculation(Calc auto const &value) noexcept
  {
    return value.ProtectedCalculation()->CopyRoot();
  }

  template <auto R, typename V>
  KRYS_NODISCARD calc::Child CopyCalculation(const Number<R, V> &value) noexcept
  {
    return calc::CreateNumber(value.Value);
  }

  template <auto R, typename V>
  KRYS_NODISCARD calc::Child CopyCalculation(const Percentage<R, V> &value) noexcept
  {
    return calc::CreatePercentage(value.Value);
  }

  template <auto R, typename V>
  KRYS_NODISCARD calc::Child CopyCalculation(const Length<R, V> &value) noexcept
  {
    return calc::CreateDimension(value.UnresolvedValue());
  }

  KRYS_NODISCARD inline calc::Child CopyCalculation(Numeric auto const &value) noexcept
  {
    return calc::CreateDimension(value.Value);
  }

  KRYS_NODISCARD inline calc::Child CopyCalculation(DimensionPercentageNumeric auto const &value) noexcept
  {
    return krys::SwitchOn(value, [](const auto &alternative) { return CopyCalculation(alternative); });
  }
}
