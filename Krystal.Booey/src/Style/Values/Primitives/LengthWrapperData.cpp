/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperData.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Calc/ValueMap.hpp"
#include <cmath>

namespace krys::boo::style
{
  LengthWrapperData::LengthWrapperData(uint8 opaqueType, Ref<calc::Value> &&value) noexcept
      : _opaqueType {opaqueType}, _kind {LengthWrapperDataKind::Calculation}
  {
    _calculationValueHandle = calc::ValueMap::CalculationValues().Insert(krys::move(value));
  }

  calc::Value &LengthWrapperData::CalculationValue() const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    return calc::ValueMap::CalculationValues().Get(_calculationValueHandle);
  }

  Ref<calc::Value> LengthWrapperData::ProtectedCalculationValue() const noexcept
  {
    return ShareRef(CalculationValue());
  }

  void LengthWrapperData::AddRef() const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    calc::ValueMap::CalculationValues().AddRef(_calculationValueHandle);
  }

  void LengthWrapperData::SubRef() const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    calc::ValueMap::CalculationValues().SubRef(_calculationValueHandle);
  }

  float LengthWrapperData::NonNanCalculatedValue(float maxValue, const ZoomFactor &usedZoom) const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    float result = static_cast<float>(ProtectedCalculationValue()->Evaluate(maxValue, usedZoom));
    if (std::isnan(result))
    {
      return 0;
    }

    return result;
  }

  float LengthWrapperData::NonNanCalculatedValue(float maxValue, const ZoomNeeded &token) const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    float result = static_cast<float>(ProtectedCalculationValue()->Evaluate(maxValue, token));
    if (std::isnan(result))
    {
      return 0;
    }

    return result;
  }

  bool LengthWrapperData::IsCalculatedEqual(const LengthWrapperData &other) const noexcept
  {
    return CalculationValue() == other.CalculationValue();
  }
}
