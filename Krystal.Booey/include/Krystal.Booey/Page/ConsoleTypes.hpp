#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief What a console message is about.
  enum class MessageSource : uint8
  {
    JS,
    Network,
    ConsoleAPI,
    Storage,
    Rendering,
    CSS,
    Security,
    Other,
  };

  /// @brief How serious a console message is.
  enum class MessageLevel : uint8
  {
    Log,
    Warning,
    Error,
    Debug,
    Info,
  };
}
