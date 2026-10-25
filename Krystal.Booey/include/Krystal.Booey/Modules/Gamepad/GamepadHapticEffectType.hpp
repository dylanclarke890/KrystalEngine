#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  /// @brief The kinds of haptic effect a gamepad can play.
  /// @see https://w3c.github.io/gamepad/#dom-gamepadhapticeffecttype
  enum class GamepadHapticEffectType : uint8
  {
    /// @brief Two eccentric rotating mass motors, one heavy (strong) and one light (weak).
    DualRumble
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::GamepadHapticEffectType, 1uz)

namespace krys::boo
{
  using GamepadHapticEffectTypeSet = EnumSet<GamepadHapticEffectType>;
}
