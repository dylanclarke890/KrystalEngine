#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  using CodecDecodeResult = StatelessDecodeResult<Span<const byte>, utf32_string>;
  using CodecEncodeResult = StatelessEncodeResult<Span<const UnicodeCodePoint>, List<byte>>;

  // A codec class wraps a compile time encoding and provides methods for decoding to unicode code points and
  // encoding to bytes.
  class ICodec : NonCopyMovable<ICodec>
  {
  public:
    virtual ~ICodec() = default;

    /// @brief Gets the name of the encoding used by this codec.
    KRYS_NODISCARD virtual ASCIILiteral Name() const noexcept = 0;

    /// @brief Decodes a span of bytes to a list of unicode code points.
    /// @param bytes The input bytes to decode.
    /// @return A list of decoded unicode code points.
    KRYS_NODISCARD virtual CodecDecodeResult Decode(Span<const byte> bytes) const = 0;

    /// @brief Encodes a span of unicode code points to a list of bytes.
    /// @param codePoints The input unicode code points to encode.
    /// @return A list of encoded bytes.
    KRYS_NODISCARD virtual CodecEncodeResult Encode(Span<const UnicodeCodePoint> codePoints) const = 0;
  };
}