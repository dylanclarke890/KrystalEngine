/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Text/TextFlags.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <algorithm>

namespace krys::boo
{
  KRYS_NODISCARD inline Maybe<FontSelectionValue> FontWeightValue(css::ValueId value) noexcept
  {
    switch (value)
    {
      case css::ValueId::Normal:  return NormalWeightValue();
      case css::ValueId::Bold:
      case css::ValueId::Bolder:  return BoldWeightValue();
      case css::ValueId::Lighter: return LightWeightValue();
      default:                    return null;
    }
  }

  KRYS_NODISCARD inline Maybe<css::ValueId> FontWidthKeyword(FontSelectionValue width) noexcept
  {
    if (width == UltraCondensedWidthValue())
    {
      return css::ValueId::UltraCondensed;
    }
    if (width == ExtraCondensedWidthValue())
    {
      return css::ValueId::ExtraCondensed;
    }
    if (width == CondensedWidthValue())
    {
      return css::ValueId::Condensed;
    }
    if (width == SemiCondensedWidthValue())
    {
      return css::ValueId::SemiCondensed;
    }
    if (width == NormalWidthValue())
    {
      return css::ValueId::Normal;
    }
    if (width == SemiExpandedWidthValue())
    {
      return css::ValueId::SemiExpanded;
    }
    if (width == ExpandedWidthValue())
    {
      return css::ValueId::Expanded;
    }
    if (width == ExtraExpandedWidthValue())
    {
      return css::ValueId::ExtraExpanded;
    }
    if (width == UltraExpandedWidthValue())
    {
      return css::ValueId::UltraExpanded;
    }
    return null;
  }

  KRYS_NODISCARD inline Maybe<FontSelectionValue> FontWidthValue(css::ValueId value) noexcept
  {
    switch (value)
    {
      case css::ValueId::UltraCondensed: return UltraCondensedWidthValue();
      case css::ValueId::ExtraCondensed: return ExtraCondensedWidthValue();
      case css::ValueId::Condensed:      return CondensedWidthValue();
      case css::ValueId::SemiCondensed:  return SemiCondensedWidthValue();
      case css::ValueId::Normal:         return NormalWidthValue();
      case css::ValueId::SemiExpanded:   return SemiExpandedWidthValue();
      case css::ValueId::Expanded:       return ExpandedWidthValue();
      case css::ValueId::ExtraExpanded:  return ExtraExpandedWidthValue();
      case css::ValueId::UltraExpanded:  return UltraExpandedWidthValue();
      default:                           return null;
    }
  }

  KRYS_NODISCARD inline Maybe<css::ValueId> FontStyleKeyword(Maybe<FontSelectionValue> style,
                                                             FontStyleAxis axis) noexcept
  {
    if (!style)
    {
      return css::ValueId::Normal;
    }
    if (style.value() == ItalicValue())
    {
      return axis == FontStyleAxis::ital ? css::ValueId::Italic : css::ValueId::Oblique;
    }
    return null;
  }

  KRYS_NODISCARD inline FontSelectionValue NormalizedFontItalicValue(float inputValue) noexcept
  {
    return FontSelectionValue {std::clamp(inputValue, -90.0f, 90.0f)};
  }
}
