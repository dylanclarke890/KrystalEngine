#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/EncodingConstants.hpp"
#include "Krystal.Text/Encodings/EncodingFallback.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys::Text
{
  struct EncodingInfo
  {
    utf8_stringview IANAName;
  };

  /// @brief Represents a character encoding.
  class Encoding
  {
  protected:
    EncodingInfo _encodingInfo;
    EncoderFallback _encoderFallback;
    DecoderFallback _decoderFallback;

    constexpr explicit Encoding(const EncodingInfo &encodingInfo) noexcept
        : Encoding(encodingInfo, EncodingReplacement_UTF, EncodingReplacement_UTF)
    {
    }

    constexpr Encoding(const EncodingInfo &encodingInfo, EncoderFallback encoderFallback,
                       DecoderFallback decoderFallback) noexcept
        : _encodingInfo(encodingInfo), _encoderFallback(encoderFallback), _decoderFallback(decoderFallback)
    {
    }

  public:
    constexpr virtual ~Encoding() noexcept = default;

    /// @brief Get information about the current encoding.
    NO_DISCARD constexpr const EncodingInfo &GetEncodingInfo() const noexcept
    {
      return _encodingInfo;
    }

    /// @brief Returns the encoder fallback that should be used to replace invalid byte sequences.
    NO_DISCARD constexpr EncoderFallback GetEncoderReplacementFallback() const noexcept
    {
      return _encoderFallback;
    }

    /// @brief Returns a span of bytes that should be used to replace invalid character sequences.
    NO_DISCARD constexpr DecoderFallback GetDecoderReplacementFallback() const noexcept
    {
      return _decoderFallback;
    }

    /// @brief Returns a sequence of bytes that specifies the encoding used.
    NO_DISCARD constexpr virtual List<byte> GetPreamble() const noexcept
    {
      return {};
    }

    /// @brief Indicates whether the encoding uses single-byte characters.
    NO_DISCARD constexpr virtual bool IsSingleByte() const noexcept
    {
      return false;
    }

    /// @brief Encodes a UTF-8 string into a sequence of bytes in the target encoding.
    NO_DISCARD constexpr virtual List<byte> Encode(utf8_stringview characters) const noexcept = 0;

    /// @brief Encodes a UTF-8 string into a sequence of bytes in the target encoding.
    constexpr virtual void Encode(utf8_stringview characters, List<byte> &out) const noexcept = 0;

    /// @brief Decodes a sequence of bytes into a UTF-8 string.
    NO_DISCARD constexpr virtual utf8_string Decode(Span<const byte> bytes) const noexcept = 0;

    /// @brief Decodes a sequence of bytes into a UTF-8 string.
    constexpr virtual void Decode(Span<const byte> bytes, utf8_string &out) const noexcept = 0;

  protected:
    constexpr void Reserve(List<byte> &container, size_t minimumRequiredBytes) const noexcept
    {
      if (container.capacity() < container.size() + minimumRequiredBytes)
      {
        container.reserve(container.size() + minimumRequiredBytes);
      }
    }

    constexpr void Reserve(utf8_string &container, size_t minimumRequiredBytes) const noexcept
    {
      if (container.capacity() < container.size() + minimumRequiredBytes)
      {
        container.reserve(container.size() + minimumRequiredBytes);
      }
    }
  };
}