#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Codecs/ICodec.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Decode/Decode.hpp"
#include "Krystal.Text/Encode/Encode.hpp"
#include "Krystal.Text/EncodingName.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/Handlers/IncompleteHandler.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  template <typename TEncoding, typename TErrorHandler = ::Krys::Text::Handlers::DefaultHandler>
  class BasicCodec : public ICodec
  {
    static_assert(SameType<::Krys::Text::code_point_t<TEncoding>, ::Krys::Text::UnicodeCodePoint>,
                  "The encoding used in a Codec must use UnicodeCodePoint as its code point type");
    static_assert(SameType<::Krys::Text::code_unit_t<TEncoding>, byte>,
                  "The encoding used in a Codec must use byte as its code unit type");

    using ErrorHandler = ::Krys::Text::Handlers::BasicIncompleteHandler<TEncoding, TErrorHandler>;

  protected:
    TEncoding _encoding {};
    ErrorHandler _handler {};

  public:
    using Encoding = TEncoding;

    KRYS_NODISCARD virtual List<UnicodeCodePoint> Decode(Span<const byte> bytes) const override
    {
      return ::Krys::Text::Decode<List<UnicodeCodePoint>>(bytes, _encoding, _handler);
    }

    KRYS_NODISCARD virtual Span<const byte> IncompleteBytes() const noexcept
    {
      return std::as_bytes(_handler.CodeUnits());
    }

    KRYS_NODISCARD virtual List<byte> Encode(Span<const UnicodeCodePoint> codepoints) const override
    {
      return ::Krys::Text::Encode<List<byte>>(codepoints, _encoding, _handler);
    }

    KRYS_NODISCARD virtual Span<const byte> IncompleteCodepoints() const noexcept
    {
      return std::as_bytes(_handler.CodePoints());
    }

    KRYS_NODISCARD virtual ASCIILiteral Name() const noexcept override
    {
      return ::Krys::Text::GetEncodingName<TEncoding>();
    }
  };

  template <typename T>
  requires(IsSpecializationOf<T, BasicCodec>)
  using basic_codec_encoding_t = typename T::Encoding;
}