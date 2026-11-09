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

#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyleProperties+GettersCustomInlines.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  const boo::Color &ComputedStyleProperties::GetColor() const noexcept
  {
    return _inheritedData->Color;
  }

  LineWidth ComputedStyleProperties::OutlineWidth() const noexcept
  {
    auto &outline = _nonInheritedData->BackgroundData->Outline;
    if (static_cast<OutlineStyle>(outline.OutlineStyle) == OutlineStyle::None)
    {
      return 0_css_px;
    }
    if (static_cast<OutlineStyle>(outline.OutlineStyle) == OutlineStyle::Auto)
    {
      return LineWidth {std::max(Evaluate<float>(outline.OutlineWidth, ZoomNeeded {}),
                                 RenderTheme::PlatformFocusRingWidth())};
    }
    return outline.OutlineWidth;
  }

  Length<> ComputedStyleProperties::OutlineOffset() const noexcept
  {
    auto &outline = _nonInheritedData->BackgroundData->Outline;
    if (static_cast<OutlineStyle>(outline.OutlineStyle) == OutlineStyle::Auto)
    {
      return Length<> {
        Evaluate<float>(outline.OutlineOffset, ZoomNeeded {})
        + RenderTheme::PlatformFocusRingOffset(Evaluate<float>(outline.OutlineWidth, ZoomNeeded {}))};
    }
    return outline.OutlineOffset;
  }
}
