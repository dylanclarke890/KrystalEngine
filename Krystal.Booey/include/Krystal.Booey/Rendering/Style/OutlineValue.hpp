#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/LineWidth.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"

namespace krys::boo
{
  /// @brief An element's outline: its colour, width, offset and style.
  struct OutlineValue
  {
    style::Color OutlineColor {style::Color::GetCurrentColor()};
    style::LineWidth OutlineWidth {style::LineWidth::Length {3.0f}};
    style::Length<> OutlineOffset {0};
    uint8 OutlineStyle : 4 {static_cast<uint8>(boo::OutlineStyle::None)};

    KRYS_NODISCARD bool IsVisible() const noexcept;
    KRYS_NODISCARD bool NonZero() const noexcept;
    KRYS_NODISCARD bool IsTransparent() const noexcept;

    KRYS_NODISCARD bool operator==(const OutlineValue &) const noexcept = default;
  };

  KRYS_NODISCARD inline Maybe<BorderStyle> ToBorderStyle(OutlineStyle outlineStyle) noexcept
  {
    switch (outlineStyle)
    {
      case OutlineStyle::Auto:   break;
      case OutlineStyle::None:   return BorderStyle::None;
      case OutlineStyle::Inset:  return BorderStyle::Inset;
      case OutlineStyle::Groove: return BorderStyle::Groove;
      case OutlineStyle::Outset: return BorderStyle::Outset;
      case OutlineStyle::Ridge:  return BorderStyle::Ridge;
      case OutlineStyle::Dotted: return BorderStyle::Dotted;
      case OutlineStyle::Dashed: return BorderStyle::Dashed;
      case OutlineStyle::Solid:  return BorderStyle::Solid;
      case OutlineStyle::Double: return BorderStyle::Double;
    }
    return std::nullopt;
  }
}
