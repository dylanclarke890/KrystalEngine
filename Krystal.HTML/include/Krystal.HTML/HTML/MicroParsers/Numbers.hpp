#pragma once

#include "Krystal.HTML/HTML/MicroParsers/MicroParserResult.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML::MicroParsers
{
  class Numbers
  {
  public:
    KRYS_NODISCARD static MicroParserResult<int64> ParseInteger(DOMStringView input) noexcept;

    KRYS_NODISCARD static MicroParserResult<uint64> ParseNonNegativeInteger(DOMStringView input) noexcept;
    
    KRYS_NODISCARD static MicroParserResult<double> ParseFloatingPoint(DOMStringView input) noexcept;
  };
}