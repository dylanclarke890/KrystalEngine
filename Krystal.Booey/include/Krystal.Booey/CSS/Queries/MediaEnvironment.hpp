#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

// FIXME: The environment is process-wide. Give each document its own once documents have a frame (the frame's
// view, page and settings are what it stands in for).

namespace krys::boo::dom
{
  class Document;
}

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

  /// @brief The device and user state that media features are evaluated against: the view's size and media
  /// type, the screen, the pointing devices and the user's preferences. The host fills it in.
  struct MediaEnvironment
  {
    /// @brief The view's media type (`screen`, `print`, ...).
    CSSOMStringAtom MediaType {u8"screen"};

    /// @brief The view's layout size, in CSS pixels.
    IntSize LayoutSize {};

    /// @brief Whether the view lays content out in pages.
    bool IsPaginated {false};

    /// @brief Device pixels per CSS pixel.
    float DeviceScaleFactor {1.f};

    /// @brief The screen's size, in CSS pixels.
    FloatSize ScreenSize {};

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

    /// @brief Whether scripts can run.
    bool CanExecuteScripts {true};

    boo::css::mq::DisplayMode DisplayMode {DisplayMode::Browser};
  };

  /// @brief The environment @p document is evaluated against, or null if it has no view (no environment has
  /// been set).
  KRYS_NODISCARD const MediaEnvironment *MediaEnvironmentFor(const dom::Document &document) noexcept;

  /// @brief Sets the environment every document is evaluated against; `NullOpt` means documents have no view.
  void SetMediaEnvironment(Maybe<MediaEnvironment> environment) noexcept;
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::mq::PointerCharacteristics, 2uz);
