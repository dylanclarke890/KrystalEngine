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
#include "Krystal.Text/Handlers/ReplacementHandler.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  template <typename TEncoding>
  class BasicCodec : public ICodec
  {
    static_assert(SameType<::Krys::Text::code_point_t<TEncoding>, ::Krys::Text::UnicodeCodePoint>,
                  "The encoding used in a Codec must use UnicodeCodePoint as its code point type");
    static_assert(SameType<::Krys::Text::code_unit_t<TEncoding>, byte>,
                  "The encoding used in a Codec must use byte as its code unit type");

    using ErrorHandler = Handlers::ReplacementHandler<Handlers::ReplacementPolicy::Invalid>;

  protected:
    TEncoding _encoding {};
    ErrorHandler _handler {};

  public:
    using Encoding = TEncoding;

    KRYS_NODISCARD virtual ASCIILiteral Name() const noexcept override
    {
      return ::Krys::Text::GetEncodingName<TEncoding>();
    }

    KRYS_NODISCARD virtual CodecDecodeResult Decode(Span<const byte> bytes) const override
    {
      return ::Krys::Text::DecodeTo<utf32_string>(bytes, _encoding, _handler);
    }

    KRYS_NODISCARD virtual CodecEncodeResult Encode(Span<const UnicodeCodePoint> codepoints) const override
    {
      return ::Krys::Text::EncodeTo<List<byte>>(codepoints, _encoding, _handler);
    }
  };

  template <typename T>
  requires(IsSpecializationOf<T, BasicCodec>)
  using basic_codec_encoding_t = typename T::Encoding;
}