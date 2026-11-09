/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief An `offset-rotate` value: `auto`/`reverse`, an angle, or both, e.g. `auto 45deg`.
  class OffsetRotateValue final : public Value
  {
  private:
    RefPtr<PrimitiveValue> _modifier;
    RefPtr<PrimitiveValue> _angle;

    OffsetRotateValue(RefPtr<PrimitiveValue> &&modifier, RefPtr<PrimitiveValue> &&angle) noexcept;

  public:
    /// @param modifier A keyword primitive (`auto` or `reverse`), or null.
    /// @param angle An angle primitive, or null. At least one of the two must be set.
    KRYS_NODISCARD static Ref<OffsetRotateValue> Create(RefPtr<Value> modifier, RefPtr<Value> angle) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD PrimitiveValue *Modifier() const noexcept
    {
      return _modifier.get();
    }

    KRYS_NODISCARD PrimitiveValue *GetAngle() const noexcept
    {
      return _angle.get();
    }

    /// @brief Whether this is `auto` (optionally with a zero angle), the initial value.
    KRYS_NODISCARD bool IsInitialValue() const noexcept;

    KRYS_NODISCARD bool Equals(const OffsetRotateValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (_modifier)
      {
        if (func(*_modifier) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      if (_angle)
      {
        if (func(*_angle) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::OffsetRotateValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsOffsetRotateValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
