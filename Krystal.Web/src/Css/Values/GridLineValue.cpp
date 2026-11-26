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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Css/Values/GridLineValue.hpp"
#include "Krystal.Core/Memory/PointerComparison.hpp"

namespace krys::web::css
{
  CSSOMString GridLineValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;
    auto append = [&](const PrimitiveValue &value)
    {
      if (!result.empty())
      {
        result.push_back(u8' ');
      }
      result.append(value.CssText(context));
    };

    if (_spanValue)
    {
      append(*_spanValue);
    }
    // Only return the numeric value if not 1, or if it provided without a span value.
    // https://drafts.csswg.org/css-grid-2/#grid-placement-span-int
    if (_numericValue)
    {
      if (_numericValue->IsOne() != true || !_spanValue || !_gridLineName)
      {
        append(*_numericValue);
      }
    }
    if (_gridLineName)
    {
      append(*_gridLineName);
    }
    return result;
  }

  GridLineValue::GridLineValue(RefPtr<PrimitiveValue> spanValue, RefPtr<PrimitiveValue> numericValue,
                               RefPtr<PrimitiveValue> gridLineName) noexcept
      : Value(ValueType::GridLine), _spanValue(krys::move(spanValue)),
        _numericValue(krys::move(numericValue)), _gridLineName(krys::move(gridLineName))
  {
  }

  Ref<GridLineValue> GridLineValue::Create(RefPtr<PrimitiveValue> spanValue,
                                           RefPtr<PrimitiveValue> numericValue,
                                           RefPtr<PrimitiveValue> gridLineName) noexcept
  {
    return AdoptRef(
      *new GridLineValue(krys::move(spanValue), krys::move(numericValue), krys::move(gridLineName)));
  }

  bool GridLineValue::Equals(const GridLineValue &other) const noexcept
  {
    return ArePointingToEqualData(_spanValue, other._spanValue)
           && ArePointingToEqualData(_numericValue, other._numericValue)
           && ArePointingToEqualData(_gridLineName, other._gridLineName);
  }
}
