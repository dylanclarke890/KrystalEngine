#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::Text
{
  class EncodingFallback
  {
  protected:
    utf8_stringview _replacementCharacter;

  public:
    constexpr EncodingFallback(utf8_stringview replacementCharacter) noexcept
        : _replacementCharacter(replacementCharacter)
    {
    }

    KRYS_NODISCARD constexpr utf8_stringview GetReplacementCharacter() const noexcept
    {
      return _replacementCharacter;
    }
  };

  class EncoderFallback : public EncodingFallback
  {
  public:
    constexpr EncoderFallback(utf8_stringview replacementCharacter) noexcept
        : EncodingFallback(replacementCharacter)
    {
    }
  };

  class DecoderFallback : public EncodingFallback
  {
  public:
    constexpr DecoderFallback(utf8_stringview replacementCharacter) noexcept
        : EncodingFallback(replacementCharacter)
    {
    }
  };
}