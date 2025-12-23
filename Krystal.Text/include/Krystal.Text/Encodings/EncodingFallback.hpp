#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::Text
{
  class EncodingFallback
  {
  protected:
    utf8_stringview _replacementCharacter;

  public:
    EncodingFallback(utf8_stringview replacementCharacter) noexcept
        : _replacementCharacter(replacementCharacter)
    {
    }

    NO_DISCARD utf8_stringview GetReplacementCharacter() const noexcept
    {
      return _replacementCharacter;
    }
  };

  class EncoderFallback : public EncodingFallback
  {
  public:
    EncoderFallback(utf8_stringview replacementCharacter) noexcept : EncodingFallback(replacementCharacter)
    {
    }
  };

  class DecoderFallback : public EncodingFallback
  {
  public:
    DecoderFallback(utf8_stringview replacementCharacter) noexcept : EncodingFallback(replacementCharacter)
    {
    }
  };
}