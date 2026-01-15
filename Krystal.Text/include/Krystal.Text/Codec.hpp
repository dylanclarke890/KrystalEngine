#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Decode/Decode.hpp"
#include "Krystal.Text/Decode/DecodeOne.hpp"
#include "Krystal.Text/Encode/Encode.hpp"
#include "Krystal.Text/Encode/EncodeOne.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/Handlers/IncompleteHandler.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  // A codec class wraps a compile time encoding and provides methods for decoding to unicode code points and
  // encoding to bytes.
  class ICodec
  {
  public:
    virtual ~ICodec() = default;

    /// @brief Decodes a span of bytes to a list of unicode code points.
    /// @param bytes The input bytes to decode.
    /// @return A list of decoded unicode code points.
    KRYS_NODISCARD virtual List<UnicodeCodePoint> Decode(Span<const byte> bytes) const = 0;

    /// @brief Encodes a span of unicode code points to a list of bytes.
    /// @param codePoints The input unicode code points to encode.
    /// @return A list of encoded bytes.
    KRYS_NODISCARD virtual List<byte> Encode(Span<const UnicodeCodePoint> codePoints) const = 0;
  };

  template <typename TEncoding, typename TErrorHandler = ::Krys::Text::Handlers::DefaultHandler>
  class Codec : public ICodec
  {
    static_assert(::Krys::Text::code_point_t<TEncoding> == ::Krys::Text::UnicodeCodePoint,
                  "The encoding used in a Codec must use UnicodeCodePoint as its code point type");
    static_assert(::Krys::Text::code_unit_t<TEncoding> == byte,
                  "The encoding used in a Codec must use byte as its code unit type");

    using ErrorHandler = ::Krys::Text::Handlers::BasicIncompleteHandler<TEncoding, TErrorHandler>;

  protected:
    TEncoding _encoding {};
    ErrorHandler _handler {};

  public:
    KRYS_NODISCARD virtual List<UnicodeCodePoint> Decode(Span<const byte> bytes) const override
    {
      return ::Krys::Text::Decode<List<UnicodeCodePoint>>(bytes, _encoding, _handler);
    }

    KRYS_NODISCARD virtual Span<const byte> IncompleteBytes() const noexcept
    {
      return _handler.CodeUnits();
    }

    KRYS_NODISCARD virtual List<byte> Encode(Span<const UnicodeCodePoint> codepoints) const override
    {
      return ::Krys::Text::Encode<List<byte>>(codepoints, _encoding, _handler);
    }

    KRYS_NODISCARD virtual Span<const byte> IncompleteCodepoints() const noexcept
    {
      return _handler.CodePoints();
    }
  };
}