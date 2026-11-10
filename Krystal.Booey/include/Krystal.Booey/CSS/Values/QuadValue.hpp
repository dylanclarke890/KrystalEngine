/**
 * Copyright (C) 2023 Apple Inc. All right reserved.
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

#include "Krystal.Booey/CSS/Values/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A value with four sides, e.g. `margin: 1px 2px`.
  class QuadValue final : public Value
  {
  private:
    Quad _quad;

    explicit QuadValue(Quad quad) noexcept;

  public:
    KRYS_NODISCARD static Ref<QuadValue> Create(Quad quad) noexcept;

    /// @brief All four sides are `value`.
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> value) noexcept
    {
      return Create(Quad {value, value, value, value});
    }

    /// @brief `first` for top and bottom, `second` for right and left.
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second) noexcept
    {
      return Create(Quad {first, second, first, second});
    }

    /// @brief `first` for top, `second` for right and left, `third` for bottom.
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second,
                                                Ref<Value> third) noexcept
    {
      return Create(Quad {first, second, third, second});
    }

    /// @brief Top, right, bottom and left.
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second, Ref<Value> third,
                                                Ref<Value> fourth) noexcept
    {
      return Create(Quad {krys::move(first), krys::move(second), krys::move(third), krys::move(fourth)});
    }

    KRYS_NODISCARD const Quad &GetQuad() const noexcept
    {
      return _quad;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const QuadValue &other) const noexcept;

    /// @brief Whether the value coalesces identical values and all four sides are equal.
    KRYS_NODISCARD bool CanBeCoalesced() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::QuadValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsQuadValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

namespace krys::boo::css
{
  inline const Quad &Value::GetQuad() const noexcept
  {
    return Downcast<QuadValue>(*this).GetQuad();
  }
}
