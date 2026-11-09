/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief The value of a `view()` function, as used in `animation-timeline`.
  /// @note https://drafts.csswg.org/scroll-animations-1/#funcdef-view
  class ViewValue final : public Value
  {
  private:
    RefPtr<Value> _axis;
    RefPtr<Value> _startInset;
    RefPtr<Value> _endInset;

    ViewValue(RefPtr<Value> &&axis, RefPtr<Value> &&startInset, RefPtr<Value> &&endInset) noexcept;

  public:
    KRYS_NODISCARD static Ref<ViewValue> Create() noexcept;
    KRYS_NODISCARD static Ref<ViewValue> Create(RefPtr<Value> &&axis, RefPtr<Value> &&startInset,
                                                RefPtr<Value> &&endInset) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD const RefPtr<Value> &Axis() const noexcept
    {
      return _axis;
    }

    KRYS_NODISCARD const RefPtr<Value> &StartInset() const noexcept
    {
      return _startInset;
    }

    KRYS_NODISCARD const RefPtr<Value> &EndInset() const noexcept
    {
      return _endInset;
    }

    KRYS_NODISCARD bool Equals(const ViewValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (_axis)
      {
        if (func(*_axis) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      if (_startInset)
      {
        if (func(*_startInset) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      if (_endInset)
      {
        if (func(*_endInset) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ViewValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsViewValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
