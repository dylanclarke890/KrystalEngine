#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
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
