#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief Various flags for optimizing event target checks.
  enum class EventTargetFlags : uint8
  {
    None = 0,
    IsNode = 1 << 0,
    IsConnected = 1 << 1,
    IsInShadowTree = 1 << 2,
    IsWindow = 1 << 3,
    HasActivationBehavior = 1 << 4,
    HasLegacyPreActivationBehavior = 1 << 5,
    HasLegacyCanceledActivationBehavior = 1 << 6,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::EventTargetFlags, 8u)