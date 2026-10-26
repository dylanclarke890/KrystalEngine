#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::pal
{
  /// @brief A modifier key held (or, for Caps Lock, toggled on) during an interaction.
  enum class KeyModifier : uint8
  {
    Shift,
    Control,
    Alt,
    /// @brief The Windows key, or Command.
    Meta,
    CapsLock,
  };

  using KeyModifiers = EnumSet<KeyModifier>;

  /// @brief The text a key press types with the keyboard layout.
  struct KeyText
  {
    /// @brief The characters the key types with the modifiers held, without control characters (Enter
    /// types a carriage return): a dead key's accent joins the next key's. Empty for releases and keys
    /// that type nothing.
    utf8_string Text;
    /// @brief The characters the key would type with only Shift and Caps Lock applied (for shortcuts such
    /// as Ctrl+A, which types nothing).
    utf8_string UnmodifiedText;
    /// @brief Whether the key is a dead key: it types nothing itself, but changes the next key's
    /// characters.
    bool DeadKey {false};
    /// @brief Whether the input method took the key, to compose with: it types nothing itself, and the
    /// composition changes as the window reports.
    bool HandledByInputMethod {false};
  };
}
