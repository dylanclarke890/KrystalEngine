#pragma once

#include "Krystal.Booey/Core/StyleAppearance.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  enum class SelectionPart : bool
  {
    Background,
    Foreground
  };

  enum class ThemeFont : uint8
  {
    CaptionFont,
    IconFont,
    MenuFont,
    MessageBoxFont,
    SmallCaptionFont,
    StatusBarFont,
    MiniControlFont,
    SmallControlFont,
    ControlFont
  };

  enum class ThemeColor : uint8
  {
    ActiveBorderColor,
    ActiveCaptionColor,
    ActiveTextColor,
    AppWorkspaceColor,
    BackgroundColor,
    ButtonFaceColor,
    ButtonHighlightColor,
    ButtonShadowColor,
    ButtonTextColor,
    CanvasColor,
    CanvasTextColor,
    CaptionTextColor,
    FieldColor,
    FieldTextColor,
    GrayTextColor,
    HighlightColor,
    HighlightTextColor,
    InactiveBorderColor,
    InactiveCaptionColor,
    InactiveCaptionTextColor,
    InfoBackgroundColor,
    InfoTextColor,
    LinkTextColor,
    MatchColor,
    MenuTextColor,
    ScrollbarColor,
    ThreeDDarkShadowColor,
    ThreeDFaceColor,
    ThreeDHighlightColor,
    ThreeDLightShadowColor,
    ThreeDShadowColor,
    VisitedTextColor,
    WindowColor,
    WindowFrameColor,
    WindowTextColor,
    FocusRingColor
  };
}
