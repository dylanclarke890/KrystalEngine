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

#include "Krystal.Booey/Rendering/RenderCombineText.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontSelector.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo
{
  namespace
  {
    // Allow an em and a 15% margin.
    constexpr float TextCombineMargin = 1.15f;
  }

  RenderCombineText::RenderCombineText(dom::Text &textNode, utf8_string text) noexcept
      : RenderText(Type::CombineText, textNode, krys::move(text))
  {
    krys_debug_assert(IsRenderCombineText());
  }

  RenderCombineText::~RenderCombineText() noexcept = default;

  void RenderCombineText::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    // Only cache a new font style if the old one actually changed, so width variants and shrinking to fit
    // aren't lost: the text isn't combined again unless the font changes.
    if (!oldStyle || !oldStyle->FontCascadeEqual(Style()))
    {
      _combineFontStyle = RenderStyle::ClonePtr(Style());
    }

    RenderText::StyleDidChange(diff, oldStyle);

    if (_isCombined && SelfNeedsLayout())
    {
      // Layout combines the text again, so it's only uncombined when the style change needs a layout.
      RenderText::SetRenderedText(OriginalText());
      _isCombined = false;
    }

    _needsFontUpdate = true;
    CombineTextIfNeeded();
  }

  void RenderCombineText::SetRenderedText(utf8_string text) noexcept
  {
    RenderText::SetRenderedText(krys::move(text));

    _needsFontUpdate = true;
    CombineTextIfNeeded();
  }

  Maybe<FloatPoint> RenderCombineText::ComputeTextOrigin(const FloatRect &boxRect) const noexcept
  {
    if (!_isCombined)
    {
      return null;
    }

    // Visually centre the combined text's width, ascent and descent within the box.
    FloatPoint result = boxRect.MinXMaxYCorner();
    FloatSize combinedTextSize(_combinedTextWidth, _combinedTextAscent + _combinedTextDescent);
    result.Move((boxRect.Size().TransposedSize() - combinedTextSize) / 2);
    result.Move(0, _combinedTextAscent);
    return result;
  }

  void RenderCombineText::CombineTextIfNeeded() noexcept
  {
    if (!_needsFontUpdate)
    {
      return;
    }

    // An ancestor may lay this out again when it's already combined.
    if (_isCombined)
    {
      RenderText::SetRenderedText(OriginalText());
    }

    _isCombined = false;
    _needsFontUpdate = false;

    // text-combine-upright only applies in vertical typographic modes.
    if (!GetWritingMode().IsVerticalTypographic())
    {
      return;
    }

    auto description = OriginalFont().GetFontDescription();
    float emWidth = description.ComputedSize() * TextCombineMargin;

    auto fontSelector = ShareRefPtr(Style().GetFontCascade().GetFontSelector());

    // The combined text is drawn horizontally.
    description.SetOrientation(FontOrientation::Horizontal);

    auto textLength = Text().size();
    auto widthWith = [&](const FontCascadeDescription &fontDescription, GlyphOverflow &glyphOverflow)
    {
      FontCascade font(FontCascadeDescription {fontDescription}, Style().GetFontCascade());
      font.Update(RefPtr {fontSelector});
      glyphOverflow = GlyphOverflow {.ComputeBounds = true};
      return Width(0, textLength, font, 0, nullptr, &glyphOverflow);
    };

    GlyphOverflow glyphOverflow;
    float combinedTextWidth = widthWith(description, glyphOverflow);
    _horizontalScale = 1;
    _isCombined = combinedTextWidth <= emWidth;

    if (_isCombined)
    {
      _combineFontStyle->SetFontDescription(FontCascadeDescription {description});
    }
    else
    {
      // Try the compressed glyphs of the half, third and quarter width variants.
      auto variantDescription = description;
      constexpr Array<FontWidthVariant, 3> widthVariants {
        FontWidthVariant::HalfWidth, FontWidthVariant::ThirdWidth, FontWidthVariant::QuarterWidth};
      for (auto widthVariant : widthVariants)
      {
        // Keep in sync with FontPlatformData::IsForTextCombine.
        variantDescription.SetWidthVariant(widthVariant);

        GlyphOverflow variantGlyphOverflow;
        float runWidth = widthWith(variantDescription, variantGlyphOverflow);
        if (runWidth <= emWidth)
        {
          combinedTextWidth = runWidth;
          glyphOverflow = variantGlyphOverflow;
          _isCombined = true;
          _combineFontStyle->SetFontDescription(FontCascadeDescription {variantDescription});
          break;
        }
      }
    }

    if (!_isCombined)
    {
      // As Chrome, the text is compressed horizontally to one em, where upstream shrinks the font.
      _horizontalScale = description.ComputedSize() / combinedTextWidth;
      combinedTextWidth = description.ComputedSize();
      _isCombined = true;
      _combineFontStyle->SetFontDescription(FontCascadeDescription {description});
    }

    if (_isCombined)
    {
      StringBuilder objectReplacementCharacter;
      objectReplacementCharacter.Append(text::ObjectReplacementCharacter);
      RenderText::SetRenderedText(krys::move(objectReplacementCharacter).ToString());
      _combinedTextWidth = combinedTextWidth;
      _combinedTextAscent = glyphOverflow.Top;
      _combinedTextDescent = glyphOverflow.Bottom;
      SetNeedsLayout();
    }
  }
}
