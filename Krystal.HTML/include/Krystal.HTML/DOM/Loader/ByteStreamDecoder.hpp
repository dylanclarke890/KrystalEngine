#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Variant.hpp"
#include "Krystal.Text/Decode/Decode.hpp"
#include "Krystal.Text/Encodings/EncodingScheme.hpp"
#include "Krystal.Text/Encodings/Windows1251.hpp"
#include "Krystal.Text/Handlers/ReplacementHandler.hpp"
#include "Krystal.Text/Handlers/IncompleteHandler.hpp"

namespace Krys::HTML::DOM
{
  using ByteDecoderVariant =
    Variant<Text::basic_utf8<byte>, Text::utf16_be_t, Text::utf16_le_t, Text::basic_windows_1251<byte>>;

  class HTMLEncoding
  {

  };

  class ByteStreamDecoder
  {
    ByteDecoderVariant _decoder;

  public:
    explicit ByteStreamDecoder(ByteDecoderVariant encoding) noexcept : _decoder(std::move(encoding))
    {
    }

    KRYS_NODISCARD constexpr utf32_string Decode(Span<const byte> bytes) noexcept
    {
      auto decode = [&](auto &encoding) -> utf32_string
      {
        return Text::Decode(bytes, encoding);
      };

      return std::visit(decode, _decoder);
    }
  };
}