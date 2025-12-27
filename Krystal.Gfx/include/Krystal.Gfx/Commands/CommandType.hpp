#pragma once

#include "Krystal.Lib/Core/StringId.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx
{
  /// @brief Creates a command type from a string. Can be used in switch statements similar to an enum.
#define KRYS_CMD_TYPE(name) KRYS_SID(name)

  using CommandType = uint32;
}