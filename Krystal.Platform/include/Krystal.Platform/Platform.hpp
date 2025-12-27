#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Nullable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/NativeHandle.hpp"

namespace Krys::Platform
{
  /// @brief Initialises platform-specific functionality.
  void Initialise() noexcept;

  /// @brief Shuts down platform-specific functionality.
  void Shutdown() noexcept;

  /// @brief Gets the time in seconds since the application started.
  KRYS_NODISCARD double GetTime() noexcept;

  /// @brief Gets the time in milliseconds since the application started.
  KRYS_NODISCARD double GetTimeMilliseconds() noexcept;

  /// @brief Gets dpi of the given window.
  KRYS_NODISCARD int GetDPIForWindow(NativeHandle windowHandle = {nullptr}) noexcept;

  KRYS_NODISCARD NativeHandle GetActiveWindow() noexcept;

  /// @brief Sets the timer precision for the application.
  /// @param min The requested precision. nullopt can be passed to request the lowest possible
  /// precision.
  /// @returns The actual timer precision set, in milliseconds.
  uint SetTimerPrecision(Nullable<uint> min = std::nullopt) noexcept;

  /// @brief Sleep for the specified number of milliseconds.
  void Sleep(uint32 milliseconds) noexcept;

  /// @brief Converts a UTF-8 string to a wide string.
  KRYS_NODISCARD wstring ToWideString(const string &utf8String) noexcept;
}