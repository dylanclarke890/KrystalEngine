/*
 * Copyright (C) 2007 Nicholas Shanks <contact@nickshanks.com>
 * Copyright (C) 2008-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Fonts/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <functional>

namespace krys::boo
{
  FontSelectionValue FontCascadeDescription::LighterWeight(FontSelectionValue weight) noexcept
  {
    if (weight < FontSelectionValue(100))
    {
      return weight;
    }
    if (weight < FontSelectionValue(550))
    {
      return FontSelectionValue(100);
    }
    if (weight < FontSelectionValue(750))
    {
      return FontSelectionValue(400);
    }
    return FontSelectionValue(700);
  }

  FontSelectionValue FontCascadeDescription::BolderWeight(FontSelectionValue weight) noexcept
  {
    if (weight < FontSelectionValue(350))
    {
      return FontSelectionValue(400);
    }
    if (weight < FontSelectionValue(550))
    {
      return FontSelectionValue(700);
    }
    if (weight < FontSelectionValue(900))
    {
      return FontSelectionValue(900);
    }
    return weight;
  }

  bool FontCascadeDescription::FamilyNamesAreEqual(const StringAtom &family1,
                                                   const StringAtom &family2) noexcept
  {
    return text::IsASCIICaselessEqual(family1.View(), family2.View());
  }

  size_t FontCascadeDescription::FamilyNameHash(const StringAtom &family) noexcept
  {
    return std::hash<utf8_string> {}(FoldedFamilyName(family.View()));
  }

  utf8_string FontCascadeDescription::FoldedFamilyName(utf8_stringview family) noexcept
  {
    return text::ToASCIILower(family);
  }

  uint32 FontCascadeDescription::EffectiveFamilyCount() const noexcept
  {
    return FamilyCount();
  }

  FontFamilySpecification FontCascadeDescription::EffectiveFamilyAt(uint32 index) const noexcept
  {
    return FamilyAt(index);
  }

  FontSmoothingMode FontCascadeDescription::UsedFontSmoothing() const noexcept
  {
    return FontSmoothing();
  }

  void FontCascadeDescription::ResolveFontSizeAdjustFromFontIfNeeded(const Font &font) noexcept
  {
    auto fontSizeAdjust = GetFontSizeAdjust();
    if (!fontSizeAdjust.ShouldResolveFromFont())
    {
      return;
    }

    // The font's metrics are at its own size, which @font-face's size-adjust can make differ from the
    // computed size.
    auto aspectValue = fontSizeAdjust.Resolve(font.PlatformData().Size(), font.GetFontMetrics());
    if (!aspectValue)
    {
      return;
    }
    fontSizeAdjust.Value = *aspectValue;
    SetFontSizeAdjust(fontSizeAdjust);
  }
}
