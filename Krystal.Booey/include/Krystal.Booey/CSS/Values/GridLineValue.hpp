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

#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<grid-line>` value: an optional `span`, an optional integer and an optional line name, e.g.
  /// `span 2 header`.
  class GridLineValue final : public Value
  {
  private:
    RefPtr<PrimitiveValue> _spanValue;
    RefPtr<PrimitiveValue> _numericValue;
    RefPtr<PrimitiveValue> _gridLineName;

    GridLineValue(RefPtr<PrimitiveValue> spanValue, RefPtr<PrimitiveValue> numericValue,
                  RefPtr<PrimitiveValue> gridLineName) noexcept;

  public:
    KRYS_NODISCARD static Ref<GridLineValue> Create(RefPtr<PrimitiveValue> spanValue,
                                                    RefPtr<PrimitiveValue> numericValue,
                                                    RefPtr<PrimitiveValue> gridLineName) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const GridLineValue &other) const noexcept;

    KRYS_NODISCARD PrimitiveValue *SpanValue() const noexcept
    {
      return _spanValue.get();
    }

    KRYS_NODISCARD RefPtr<PrimitiveValue> ProtectedSpanValue() const noexcept
    {
      return _spanValue;
    }

    KRYS_NODISCARD PrimitiveValue *NumericValue() const noexcept
    {
      return _numericValue.get();
    }

    KRYS_NODISCARD RefPtr<PrimitiveValue> ProtectedNumericValue() const noexcept
    {
      return _numericValue;
    }

    KRYS_NODISCARD PrimitiveValue *GridLineName() const noexcept
    {
      return _gridLineName.get();
    }

    KRYS_NODISCARD RefPtr<PrimitiveValue> ProtectedGridLineName() const noexcept
    {
      return _gridLineName;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridLineValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridLineValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
