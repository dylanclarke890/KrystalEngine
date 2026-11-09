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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareInheritedData.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Interpolation/ChangedAnimatablePropertiesGenerated.hpp"

namespace krys::boo::style
{
  /// @brief The changed animatable properties whose storage the generated collection can't compare.
  class ChangedAnimatablePropertiesCustom final
  {
  public:
    static void ConservativelyCollectChangedAnimatablePropertiesForCursor(
      const ComputedStyle::InheritedFlags &a, const ComputedStyle::InheritedFlags &b,
      CSSPropertiesBitSet &changingProperties) noexcept
    {
      if (a.CursorType != b.CursorType)
      {
        changingProperties.Set(css::PropertyId::Cursor);
      }
    }

    static void ConservativelyCollectChangedAnimatablePropertiesForZIndex(
      const StyleBoxData &a, const StyleBoxData &b, CSSPropertiesBitSet &changingProperties) noexcept
    {
      if (a.SpecifiedZIndex() != b.SpecifiedZIndex())
      {
        changingProperties.Set(css::PropertyId::ZIndex);
      }
    }

    static void ConservativelyCollectChangedAnimatablePropertiesForCaretColor(
      const StyleRareInheritedData &a, const StyleRareInheritedData &b,
      CSSPropertiesBitSet &changingProperties) noexcept
    {
      if (a.CaretColor != b.CaretColor || a.HasAutoCaretColor != b.HasAutoCaretColor)
      {
        changingProperties.Set(css::PropertyId::CaretColor);
      }
    }
  };
}
