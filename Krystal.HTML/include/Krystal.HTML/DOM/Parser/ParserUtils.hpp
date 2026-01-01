#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Old/ASCII.hpp"
#include "Krystal.Text/Old/Concepts.hpp"
#include "Krystal.Text/Old/Rune.hpp"
#include "Krystal.Text/Old/Unicode.hpp"

namespace Krys
{
  struct ParserUtils : NonCopyMovable<ParserUtils>
  {
    ParserUtils() = delete;
    ~ParserUtils() = delete;
  };
}
