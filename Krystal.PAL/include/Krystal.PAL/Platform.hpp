#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/NativeHandle.hpp"

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
}