#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/NativeHandle.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Time/Duration.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Platform
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
  uint SetTimerPrecision(Maybe<uint> min = std::nullopt) noexcept;

  /// @brief Sleep for the specified number of milliseconds.
  void Sleep(Milliseconds duration) noexcept;
}