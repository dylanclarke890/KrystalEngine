#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A state a theme draws a form control in.
  enum class ControlStyleState : uint8
  {
    Hovered,
    Pressed,
    Focused,
    Enabled,
    Checked,
    Default,
    WindowActive,
    Indeterminate,
    SpinUp, // Sub-state for HoverState and PressedState.
    Presenting,
    FormSemanticContext,
    DarkAppearance,
    InlineFlippedWritingMode,
    LargeControls,
    ReadOnly,
    ListButton,
    ListButtonPressed,
    VerticalWritingMode,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::ControlStyleState, 18uz);

namespace krys::boo
{
  /// @brief The states and style a theme draws a form control with.
  struct ControlStyle
  {
    using State = ControlStyleState;

    EnumSet<State> States;
    float FontSize {12};
    float ZoomFactor {1};
    Color AccentColor;
    Color TextColor;
    FloatBoxExtent BorderWidth;
  };

  TextStream &operator<<(TextStream &ts, ControlStyleState state) noexcept;
  TextStream &operator<<(TextStream &ts, const ControlStyle &style) noexcept;
}
