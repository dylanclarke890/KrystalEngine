#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Utils/StringId.hpp"

namespace krys
{
  /// @brief Creates a command type from a string. Can be used in switch statements similar to an enum.
#define KRYS_CMD_TYPE(name) KRYS_SID(name)

  using CommandType = uint32;
}