#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/NativeHandle.hpp"
#include "Krystal.PAL/Pointer.hpp"

namespace krys::pal
{
  /// @brief Initialises platform-specific functionality.
  void Initialise() noexcept;

  /// @brief Shuts down platform-specific functionality.
  void Shutdown() noexcept;

  /// @brief Gets dpi of the given window.
  KRYS_NODISCARD int GetDPIForWindow(NativeHandle windowHandle = {nullptr}) noexcept;

  KRYS_NODISCARD NativeHandle GetActiveWindow() noexcept;

  /// @brief Sets the timer precision for the application.
  /// @param min The requested precision. nullopt can be passed to request the lowest possible
  /// precision.
  /// @returns The actual timer precision set, in milliseconds.
  uint SetTimerPrecision(Maybe<uint> min = null) noexcept;

  /// @brief Sleep for the specified number of milliseconds.
  void Sleep(milliseconds duration) noexcept;

  /// @brief How far the user's system settings say a notch of the mouse wheel scrolls.
  struct WheelScrollSettings
  {
    /// @brief Lines per notch of the wheel, or none to scroll a page per notch.
    Maybe<uint32> Lines {3};
    /// @brief Characters per notch of the horizontal wheel.
    uint32 Chars {3};
  };

  /// @brief Gets the system's wheel scroll settings, read when the user last changed them.
  KRYS_NODISCARD WheelScrollSettings GetWheelScrollSettings() noexcept;

  /// @brief Gets the pointing devices the system has: a mouse, a pen, a touch screen and how many touches it
  /// takes.
  KRYS_NODISCARD PointingDevices GetPointingDevices() noexcept;

  /// @brief Puts @p text (UTF-8, with LF line breaks) on the system clipboard, replacing what's there.
  /// @param windowHandle The window that owns the clipboard's contents; the active window if null.
  /// @return Whether the clipboard took the text.
  bool SetClipboardText(utf8_stringview text, NativeHandle windowHandle = {nullptr}) noexcept;

  /// @brief Gets the system clipboard's text, as UTF-8 with LF line breaks.
  /// @return The text, or none if the clipboard holds no text or can't be opened.
  KRYS_NODISCARD Maybe<utf8_string> GetClipboardText() noexcept;
}
