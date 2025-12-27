#pragma once

#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Text
{
  enum class Base64EncodeFlags : uint8
  {
    URL = 1 << 0,
    OmitPadding = 1 << 1,
  };

  enum class Base64DecodeFlags : uint8
  {
    URL = 1 << 0,
    ValidatePadding = 1 << 1,
    IgnoreWhitespace = 1 << 2,
  };

  struct Base64Specification
  {
    Span<const byte> Input;
    Base64EncodeFlags Options;
  };
}