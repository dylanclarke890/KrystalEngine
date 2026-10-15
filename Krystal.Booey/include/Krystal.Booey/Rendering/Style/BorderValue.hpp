#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/LineWidth.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"

namespace krys::boo
{
  /// @brief One edge of an element's border: its colour, width and style.
  struct BorderValue
  {
    style::Color Color {style::Color::GetCurrentColor()};
    style::LineWidth Width {style::LineWidth::Length {3.0f}};
    uint8 Style : 4 {static_cast<uint8>(BorderStyle::None)};

    KRYS_NODISCARD bool IsVisible() const noexcept;
    KRYS_NODISCARD bool NonZero() const noexcept;
    KRYS_NODISCARD bool IsTransparent() const noexcept;

    KRYS_NODISCARD bool HasHiddenStyle() const noexcept
    {
      return static_cast<BorderStyle>(Style) == BorderStyle::Hidden;
    }
    KRYS_NODISCARD bool HasVisibleStyle() const noexcept
    {
      return static_cast<BorderStyle>(Style) > BorderStyle::Hidden;
    }

    KRYS_NODISCARD bool operator==(const BorderValue &) const noexcept = default;
  };

  inline bool BorderValue::NonZero() const noexcept
  {
    return Width && static_cast<BorderStyle>(Style) != BorderStyle::None;
  }
}
