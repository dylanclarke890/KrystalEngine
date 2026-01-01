#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/Old/Encodings/EncodingInfo.hpp"
#include "Krystal.Text/Old/Rune.hpp"

namespace Krys
{
  enum class EncodingStatus
  {
    Ok,
    NeedMoreInput,
  };

  enum class EncodingErrorAction
  {
    /// @brief Replace the invalid input with a replacement character.
    Replace,

    /// @brief Skip the invalid input without producing any output.
    Skip,
  };

  struct DecoderSettings
  {
    /// @brief Action to take when an invalid byte sequence is encountered.
    EncodingErrorAction InvalidByteSequence = EncodingErrorAction::Replace;

    /// @brief Action to take when a surrogate codepoint is encountered.
    EncodingErrorAction SurrogateCodepoint = EncodingErrorAction::Replace;

    /// @brief Whether the input byte sequence may be prefixed with a byte order mark (BOM).
    bool AllowByteOrderMark = true;
  };

  struct EncoderSettings
  {
    /// @brief Whether to emit a byte order mark (BOM) at the start of the encoded byte sequence.
    bool EmitByteOrderMark = false;

    /// @brief Action to take when a character cannot be represented in the target encoding.
    EncodingErrorAction UnrepresentableCharacter = EncodingErrorAction::Replace;
  };

  /// @brief Represents a character encoding. Each encoding must implement methods for encoding and decoding
  /// runes (Unicode codepoints) to and from byte sequences, as well as methods for determining the maximum
  /// byte and character counts for encoding and decoding operations.
  class Encoding
  {
  protected:
    EncodingInfo _encodingInfo;

    Encoding(const EncodingInfo &encodingInfo) noexcept : _encodingInfo(encodingInfo)
    {
    }

  public:
    virtual ~Encoding() noexcept = default;

    /// @brief Get information about the current encoding.
    KRYS_NODISCARD const EncodingInfo &GetInfo() const noexcept
    {
      return _encodingInfo;
    }

    /// @brief Encodes a series of characters into a sequence of bytes in the target character encoding.
    virtual void Encode(Span<const Rune> characters, List<byte> &out) const noexcept = 0;

    /// @brief Returns the maximum number of bytes required to encode a series of characters.
    KRYS_NODISCARD virtual size_t GetMaxByteCount(size_t charCount) const noexcept = 0;

    /// @brief Decodes a sequence of bytes in the target character encoding into a series of characters.
    virtual void Decode(Span<const byte> bytes, List<Rune> &out) const noexcept = 0;

    /// @brief Returns the maximum number of characters that can result from decoding a sequence of bytes.
    KRYS_NODISCARD virtual size_t GetMaxCharCount(size_t byteCount) const noexcept = 0;
  };
}