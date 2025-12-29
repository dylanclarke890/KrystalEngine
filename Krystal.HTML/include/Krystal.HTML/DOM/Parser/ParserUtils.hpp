#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Rune.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  struct ParserUtils : NonCopyMovable<ParserUtils>
  {
    ParserUtils() = delete;
    ~ParserUtils() = delete;
  };
}
