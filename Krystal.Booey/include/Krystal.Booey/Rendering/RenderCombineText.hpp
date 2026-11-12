/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief The text of an element with `text-combine-upright: all`: in vertical typographic writing modes,
  /// it's drawn horizontally in the space of one character (1em), with narrower glyphs or compressed to fit.
  /// @see https://drafts.csswg.org/css-writing-modes/#text-combine-upright
  class RenderCombineText final : public RenderText
  {
    KRYS_NON_COPY_MOVABLE(RenderCombineText);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderCombineText);

  private:
    UniquePtr<RenderStyle> _combineFontStyle;
    float _combinedTextWidth {0};
    float _combinedTextAscent {0};
    float _combinedTextDescent {0};
    float _horizontalScale {1};
    bool _isCombined : 1 {false};
    bool _needsFontUpdate : 1 {false};

  public:
    RenderCombineText(dom::Text &textNode, utf8_string text) noexcept;
    ~RenderCombineText() noexcept override;

    /// @brief Combines the text again if its font or text changed since it last did.
    void CombineTextIfNeeded() noexcept;
    /// @brief Where to draw the combined text so it's centred in @p boxRect, if it's combined.
    KRYS_NODISCARD Maybe<FloatPoint> ComputeTextOrigin(const FloatRect &boxRect) const noexcept;

    KRYS_NODISCARD bool IsCombined() const noexcept
    {
      return _isCombined;
    }
    /// @brief How much the combined text is compressed horizontally to fit one em (1 if it isn't).
    KRYS_NODISCARD float HorizontalScale() const noexcept
    {
      return _horizontalScale;
    }
    /// @brief Combined text takes one em.
    KRYS_NODISCARD float CombinedTextWidth(const FontCascade &font) const noexcept
    {
      return font.Size();
    }
    KRYS_NODISCARD const FontCascade &OriginalFont() const noexcept
    {
      return Parent()->Style().GetFontCascade();
    }
    /// @brief The font the combined text is drawn with: horizontal, with narrower glyphs if they fit.
    KRYS_NODISCARD const FontCascade &TextCombineFont() const noexcept
    {
      return _combineFontStyle->GetFontCascade();
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderCombineText";
    }

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

  private:
    void SetRenderedText(utf8_string text) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderCombineText)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderCombineText();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
