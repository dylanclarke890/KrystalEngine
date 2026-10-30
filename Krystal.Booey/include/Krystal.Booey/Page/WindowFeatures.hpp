#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief The features `window.open()` asks a new window to have: its position and size, its bars, and
  /// whether it's a popup.
  /// @see https://html.spec.whatwg.org/#concept-window-open-features
  struct WindowFeatures
  {
    bool HasAdditionalFeatures {false};

    Maybe<float> X;
    Maybe<float> Y;
    Maybe<float> Width;
    Maybe<float> Height;

    Maybe<bool> Popup;
    Maybe<bool> MenuBarVisible;
    Maybe<bool> StatusBarVisible;
    Maybe<bool> ToolBarVisible;
    Maybe<bool> LocationBarVisible;
    Maybe<bool> ScrollbarsVisible;
    Maybe<bool> Resizable;

    Maybe<bool> Fullscreen;
    Maybe<bool> NoOpener;
    Maybe<bool> NoReferrer;

    List<dom::DOMString> AdditionalFeatures;

    KRYS_NODISCARD bool WantsNoOpener() const noexcept
    {
      return (NoOpener && *NoOpener) || (NoReferrer && *NoReferrer);
    }

    KRYS_NODISCARD bool WantsNoReferrer() const noexcept
    {
      return NoReferrer && *NoReferrer;
    }

    /// @brief Whether the features ask for a popup (a window with minimal chrome).
    /// @see https://html.spec.whatwg.org/#popup-window-is-requested
    KRYS_NODISCARD bool WantsPopup() const noexcept;
  };

  /// @brief The features of @p windowFeaturesString, as `window.open()`'s third argument.
  /// @see https://html.spec.whatwg.org/#concept-window-open-features-tokenize
  KRYS_NODISCARD WindowFeatures ParseWindowFeatures(dom::DOMStringView windowFeaturesString) noexcept;
}
