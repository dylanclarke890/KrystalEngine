#pragma once

#include "Krystal.Core/Numeric.hpp"

/// @brief Windows virtual key codes: the legacy `keyCode` and `which` of keyboard events on every platform.
/// Digits and letters are their ASCII codes (`'0'` to `'9'`, `'A'` to `'Z'`).
namespace krys::boo::vk
{
  constexpr int32 Back = 0x08;
  constexpr int32 Tab = 0x09;
  constexpr int32 Clear = 0x0C;
  constexpr int32 Return = 0x0D;
  constexpr int32 Shift = 0x10;
  constexpr int32 Control = 0x11;
  /// @brief Alt.
  constexpr int32 Menu = 0x12;
  constexpr int32 Pause = 0x13;
  /// @brief Caps Lock.
  constexpr int32 Capital = 0x14;
  constexpr int32 Escape = 0x1B;
  constexpr int32 Space = 0x20;
  /// @brief Page Up.
  constexpr int32 Prior = 0x21;
  /// @brief Page Down.
  constexpr int32 Next = 0x22;
  constexpr int32 End = 0x23;
  constexpr int32 Home = 0x24;
  constexpr int32 Left = 0x25;
  constexpr int32 Up = 0x26;
  constexpr int32 Right = 0x27;
  constexpr int32 Down = 0x28;
  constexpr int32 Insert = 0x2D;
  constexpr int32 Delete = 0x2E;
  constexpr int32 Help = 0x2F;
  /// @brief The left Windows (or Command) key.
  constexpr int32 LWin = 0x5B;
  /// @brief The right Windows (or Command) key.
  constexpr int32 RWin = 0x5C;
  /// @brief The context menu key.
  constexpr int32 Apps = 0x5D;
  constexpr int32 Numpad0 = 0x60;
  constexpr int32 Numpad1 = 0x61;
  constexpr int32 Numpad2 = 0x62;
  constexpr int32 Numpad3 = 0x63;
  constexpr int32 Numpad4 = 0x64;
  constexpr int32 Numpad5 = 0x65;
  constexpr int32 Numpad6 = 0x66;
  constexpr int32 Numpad7 = 0x67;
  constexpr int32 Numpad8 = 0x68;
  constexpr int32 Numpad9 = 0x69;
  constexpr int32 Multiply = 0x6A;
  constexpr int32 Add = 0x6B;
  constexpr int32 Separator = 0x6C;
  constexpr int32 Subtract = 0x6D;
  constexpr int32 Decimal = 0x6E;
  constexpr int32 Divide = 0x6F;
  /// @brief F1; F2 to F24 follow it.
  constexpr int32 F1 = 0x70;
  constexpr int32 NumLock = 0x90;
  constexpr int32 Scroll = 0x91;
  constexpr int32 LShift = 0xA0;
  constexpr int32 RShift = 0xA1;
  constexpr int32 LControl = 0xA2;
  constexpr int32 RControl = 0xA3;
  constexpr int32 LMenu = 0xA4;
  constexpr int32 RMenu = 0xA5;
  constexpr int32 VolumeMute = 0xAD;
  constexpr int32 VolumeDown = 0xAE;
  constexpr int32 VolumeUp = 0xAF;
  /// @brief `;:` on a US keyboard.
  constexpr int32 Oem1 = 0xBA;
  constexpr int32 OemPlus = 0xBB;
  constexpr int32 OemComma = 0xBC;
  constexpr int32 OemMinus = 0xBD;
  constexpr int32 OemPeriod = 0xBE;
  /// @brief `/?` on a US keyboard.
  constexpr int32 Oem2 = 0xBF;
  /// @brief `` `~ `` on a US keyboard.
  constexpr int32 Oem3 = 0xC0;
  /// @brief `[{` on a US keyboard.
  constexpr int32 Oem4 = 0xDB;
  /// @brief `\|` on a US keyboard.
  constexpr int32 Oem5 = 0xDC;
  /// @brief `]}` on a US keyboard.
  constexpr int32 Oem6 = 0xDD;
  /// @brief `'"` on a US keyboard.
  constexpr int32 Oem7 = 0xDE;
  /// @brief The key code of a key an input method handles.
  constexpr int32 Process = 0xE5;
}
