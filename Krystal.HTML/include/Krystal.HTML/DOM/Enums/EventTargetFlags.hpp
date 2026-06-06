#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief Various flags for optimizing event target checks.
  enum class EventTargetFlags : uint8
  {
    None = 0,
    IsNode = 1 << 0,
    IsWindow = 1 << 1,
    HasActivationBehavior = 1 << 2,
    HasLegacyPreActivationBehavior = 1 << 3,
    HasLegacyCanceledActivationBehavior = 1 << 4,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::EventTargetFlags, 6u)