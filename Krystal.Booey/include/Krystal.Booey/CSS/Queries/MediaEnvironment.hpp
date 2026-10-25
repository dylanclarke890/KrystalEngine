#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css::mq
{
  /// @brief What a pointing device can do.
  enum class PointerCharacteristics : uint8
  {
    None = 0,
    Coarse = 1 << 0,
    Fine = 1 << 1,
  };

  /// @brief How the document is presented, as an application manifest's `display` member.
  enum class DisplayMode : uint8
  {
    Fullscreen,
    Standalone,
    MinimalUI,
    Browser
  };

  /// @brief The device and user state that media features are evaluated against: the screen, the pointing
  /// devices, the user's preferences and how the document is presented. The host fills it in, on the frame
  /// (`LocalFrame::SetMediaEnvironment`); the view's size and media type are the view's.
  // FIXME: Stands in for the page (device scale factor, appearance, application manifest), its chrome client
  // (pointing devices), the platform's screen and accessibility settings, and the frame's script controller.
  struct MediaEnvironment
  {
    /// @brief Device pixels per CSS pixel.
    float DeviceScaleFactor {1.f};

    /// @brief The screen's size, in CSS pixels.
    FloatSize ScreenSize {};
    /// @brief The part of the screen windows can use (less the task bar), in CSS pixels; empty for the whole
    /// screen.
    FloatRect ScreenAvailableRect {};

    /// @brief Bits per colour component.
    int32 ScreenDepthPerComponent {8};

    /// @brief Whether the screen shows colours beyond sRGB (Display P3).
    bool ScreenSupportsExtendedColor {false};

    /// @brief Whether the screen supports high dynamic range.
    bool ScreenSupportsHighDynamicRange {false};

    /// @brief Whether the screen shows only shades of one colour.
    bool ScreenIsMonochrome {false};

    /// @brief Whether the system inverts colours.
    bool ColorsAreInverted {false};

    /// @brief Whether the user asked for more contrast.
    bool UserPrefersContrast {false};

    /// @brief Whether the user asked for less motion.
    bool UserPrefersReducedMotion {false};

    /// @brief Whether the user asked for a dark appearance.
    bool UseDarkAppearance {false};

    /// @brief Whether the primary pointing device can hover.
    bool HoverSupportedByPrimaryPointingDevice {true};

    /// @brief Whether any available pointing device can hover.
    bool HoverSupportedByAnyAvailablePointingDevice {true};

    /// @brief What the primary pointing device can do.
    PointerCharacteristics PrimaryPointingDevice {PointerCharacteristics::Fine};

    /// @brief What the available pointing devices can do, together.
    PointerCharacteristics AllAvailablePointingDevices {PointerCharacteristics::Fine};

    /// @brief How many touches the touch screens take at once; 0 without one (`navigator.maxTouchPoints`).
    uint32 MaxTouchPoints {0};

    /// @brief Whether scripts can run.
    bool CanExecuteScripts {true};

    boo::css::mq::DisplayMode DisplayMode {DisplayMode::Browser};

    friend bool operator==(const MediaEnvironment &, const MediaEnvironment &) = default;
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::mq::PointerCharacteristics, 2uz);
