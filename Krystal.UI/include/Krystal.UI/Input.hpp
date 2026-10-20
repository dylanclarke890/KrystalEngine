#pragma once

#include "Krystal.Booey/Page/MouseEventTypes.hpp"
#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.PAL/CursorShapes.hpp"
#include "Krystal.PAL/KeyModifiers.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.PAL/MouseButtons.hpp"
#include "Krystal.PAL/Platform.hpp"

/// @brief Conversions between the platform layer's input and cursors and Booey's.
namespace krys::ui::input
{
  /// @brief The DOM's description of a key, independent of the keyboard layout.
  struct KeyDescription
  {
    /// @brief The physical key: `KeyA`, `Digit1`, `ShiftLeft`; empty if unknown.
    /// @see https://w3c.github.io/uievents-code/
    utf8_stringview Code;
    /// @brief The key's value if it doesn't type text (`Enter`, `Shift`, `ArrowLeft`); empty for keys whose
    /// value is the text they type.
    /// @see https://w3c.github.io/uievents-key/
    utf8_stringview NamedKey;
    /// @brief The legacy key identifier of a key that doesn't type text (`Enter`, `U+0009`); empty for keys
    /// identified by their text.
    utf8_stringview KeyIdentifier;
    /// @brief The Windows virtual key code, with the side for modifiers (`LShift`, `RControl`).
    int32 WindowsVirtualKeyCode {0};
    bool IsKeypad {false};
  };

  KRYS_NODISCARD KeyDescription Describe(pal::Key key) noexcept;

  KRYS_NODISCARD boo::MouseButton ToBooey(pal::MouseButton button) noexcept;
  KRYS_NODISCARD EnumSet<boo::PlatformEvent::Modifier> ToBooey(pal::KeyModifiers modifiers) noexcept;

  /// @brief A key press (`Type::KeyDown`, to be split into `keydown` and `keypress`) or release
  /// (`Type::KeyUp`) of @p key. A press's `key` is the key's name, or else the text it types (with the
  /// modifiers, or only Shift if they type nothing, as for Ctrl+A); a release's is @p releasedKeyValue.
  KRYS_NODISCARD boo::PlatformKeyboardEvent KeyboardEvent(boo::PlatformEvent::Type type, pal::Key key,
                                                          pal::KeyModifiers modifiers,
                                                          const pal::KeyText &text, bool isAutoRepeat,
                                                          utf8_stringview releasedKeyValue = {}) noexcept;

  /// @brief A turn of the wheel at @p point by @p notchesX and @p notchesY (positive is away from the user,
  /// or to the right), scrolling @p settings' lines (or a page) per notch, at 100 pixels per three lines;
  /// with Shift, the vertical wheel scrolls sideways.
  KRYS_NODISCARD boo::PlatformWheelEvent WheelEvent(const boo::IntPoint &point, float notchesX,
                                                    float notchesY, pal::KeyModifiers modifiers,
                                                    const pal::WheelScrollSettings &settings) noexcept;

  /// @brief The system cursor nearest to a Booey cursor.
  KRYS_NODISCARD pal::CursorShape ToCursorShape(boo::Cursor cursor) noexcept;
}
