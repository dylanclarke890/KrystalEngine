#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <chrono>

namespace krys::boo
{
  class GraphicsContext;
  class FloatRect;
  class Path;
}

namespace krys::boo::adwaita
{
  constexpr uint32 FocusLineWidth = 2;
  constexpr double FocusRingOpacity = 0.8;
  constexpr double DisabledOpacity = 0.5;

  constexpr auto ArrowColorLight = SRGBA<uint8> {46, 52, 54};
  constexpr auto ArrowColorDark = SRGBA<uint8> {238, 238, 236};

  constexpr int32 TextFieldBorderSize = 1;
  constexpr auto TextFieldBorderColorLight = SRGBA<uint8> {0, 0, 0, 50};
  constexpr auto TextFieldBackgroundColorLight = Color::white;
  constexpr auto TextFieldBorderColorDark = SRGBA<uint8> {255, 255, 255, 50};
  constexpr auto TextFieldBackgroundColorDark = SRGBA<uint8> {45, 45, 45};

  constexpr uint32 ArrowSize = 16;
  constexpr uint32 MenuListButtonArrowSize = 16;
  constexpr int32 MenuListButtonFocusOffset = -2;
  constexpr uint32 MenuListButtonPadding = 5;
  constexpr int32 MenuListButtonBorderSize = 1;

  constexpr uint32 ProgressActivityBlocks = 5;
  constexpr MonotonicTime::duration ProgressAnimationDuration = std::chrono::milliseconds(2'475);
  constexpr int32 ProgressBarSize = 6;
  constexpr auto ProgressBarBackgroundColorLight = SRGBA<uint8> {0, 0, 0, 40};
  constexpr auto ProgressBarBackgroundColorDark = SRGBA<uint8> {255, 255, 255, 30};

  constexpr uint32 SliderTrackSize = 6;
  constexpr auto SliderTrackBackgroundColorLight = SRGBA<uint8> {0, 0, 0, 40};
  constexpr auto SliderTrackBackgroundColorDark = SRGBA<uint8> {255, 255, 255, 30};
  constexpr int32 SliderTrackFocusOffset = 2;
  constexpr int32 SliderThumbSize = 20;
  constexpr int32 SliderThumbBorderSize = 1;
  constexpr auto SliderThumbBorderColorLight = SRGBA<uint8> {0, 0, 0, 50};
  constexpr auto SliderThumbBackgroundColorLight = Color::white;
  constexpr auto SliderThumbBackgroundHoveredColorLight = SRGBA<uint8> {244, 244, 244};
  constexpr auto SliderThumbBackgroundDisabledColorLight = SRGBA<uint8> {244, 244, 244};
  constexpr auto SliderThumbBorderColorDark = SRGBA<uint8> {0, 0, 0, 50};
  constexpr auto SliderThumbBackgroundColorDark = SRGBA<uint8> {210, 210, 210};
  constexpr auto SliderThumbBackgroundHoveredColorDark = SRGBA<uint8> {230, 230, 230};
  constexpr auto SliderThumbBackgroundDisabledColorDark = SRGBA<uint8> {150, 150, 150};

  constexpr int32 ButtonFocusOffset = -2;
  constexpr int32 ButtonBorderSize = 1;
  constexpr auto ButtonTextColorLight = SRGBA<uint8> {0, 0, 0, 204};
  constexpr auto ButtonTextDisabledColorLight = SRGBA<uint8> {0, 0, 0, 102};
  constexpr auto ButtonTextColorDark = SRGBA<uint8> {255, 255, 255};
  constexpr auto ButtonTextDisabledColorDark = SRGBA<uint8> {255, 255, 255, 127};
  constexpr auto ButtonBorderColorLight = SRGBA<uint8> {0, 0, 0, 50};
  constexpr auto ButtonBackgroundColorLight = SRGBA<uint8> {244, 244, 244};
  constexpr auto ButtonBackgroundPressedColorLight = SRGBA<uint8> {214, 214, 214};
  constexpr auto ButtonBackgroundHoveredColorLight = SRGBA<uint8> {248, 248, 248};
  constexpr auto ToggleBorderColorLight = SRGBA<uint8> {0, 0, 0, 50};
  constexpr auto ToggleBorderHoveredColorLight = SRGBA<uint8> {0, 0, 0, 80};
  constexpr auto ButtonBorderColorDark = SRGBA<uint8> {255, 255, 255, 50};
  constexpr auto ButtonBackgroundColorDark = SRGBA<uint8> {52, 52, 52};
  constexpr auto ButtonBackgroundPressedColorDark = SRGBA<uint8> {30, 30, 30};
  constexpr auto ButtonBackgroundHoveredColorDark = SRGBA<uint8> {60, 60, 60};
  constexpr auto ToggleBorderColorDark = SRGBA<uint8> {255, 255, 255, 50};
  constexpr auto ToggleBorderHoveredColorDark = SRGBA<uint8> {255, 255, 255, 80};

  constexpr double ToggleSize = 14.;
  constexpr int32 ToggleBorderSize = 2;
  constexpr int32 ToggleFocusOffset = 1;

  constexpr auto SpinButtonBorderColorLight = SRGBA<uint8> {0, 0, 0, 25};
  constexpr auto SpinButtonBackgroundColorLight = Color::white;
  constexpr auto SpinButtonBackgroundHoveredColorLight = SRGBA<uint8> {0, 0, 0, 50};
  constexpr auto SpinButtonBackgroundPressedColorLight = SRGBA<uint8> {0, 0, 0, 70};
  constexpr auto SpinButtonBorderColorDark = SRGBA<uint8> {255, 255, 255, 25};
  constexpr auto SpinButtonBackgroundColorDark = SRGBA<uint8> {45, 45, 45};
  constexpr auto SpinButtonBackgroundHoveredColorDark = SRGBA<uint8> {255, 255, 255, 50};
  constexpr auto SpinButtonBackgroundPressedColorDark = SRGBA<uint8> {255, 255, 255, 70};

  enum class PaintRounded : bool
  {
    No,
    Yes
  };

  void PaintFocus(GraphicsContext &, const FloatRect &, int32 offset, const Color &,
                  PaintRounded = PaintRounded::No) noexcept;
  void PaintFocus(GraphicsContext &, const Path &, const Color &) noexcept;
  void PaintFocus(GraphicsContext &, Span<const FloatRect>, const Color &,
                  PaintRounded = PaintRounded::No) noexcept;

  enum class ArrowDirection : uint8
  {
    Up,
    Down
  };

  void PaintArrow(GraphicsContext &, const FloatRect &, ArrowDirection, bool useDarkAppearance) noexcept;

  KRYS_NODISCARD inline Color FocusColor(const Color &accentColor) noexcept
  {
    return accentColor.ColorWithAlphaMultipliedBy(static_cast<float>(FocusRingOpacity));
  }

  KRYS_NODISCARD Color SystemAccentColor() noexcept;
  KRYS_NODISCARD Color SystemFocusRingColor() noexcept;
}
