#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief Why a frame is loading.
  enum class FrameLoadType : uint8
  {
    Standard,
    /// @brief The host loads the document again.
    Reload,
  };

  /// @brief Whether a new browsing context opened by a navigation gets its opener.
  enum class NewFrameOpenerPolicy : uint8
  {
    Suppress,
    Allow
  };

  /// @brief What started a navigation.
  enum class NavigationType : uint8
  {
    LinkClicked,
    FormSubmitted,
    Other
  };
}
