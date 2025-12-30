#pragma once

#include "Krystal.IO/Streams/Stream.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::HTML::DOM
{
  class ByteStreamDecoder : NonCopyMovable<ByteStreamDecoder>
  {
  private:
    IO::IStreamReader *_byteStream;
    Encoding *_encoding;

  public:
    ByteStreamDecoder(Encoding *encoding, IO::IStreamReader *byteStream) noexcept
        : _encoding(encoding), _byteStream(byteStream)
    {
    }

    ~ByteStreamDecoder() noexcept = default;

    /// @brief Reads up to 'maxChars' decoded characters from the byte stream.
    KRYS_NODISCARD utf8_string Read(size_t maxChars) noexcept
    {
      utf8_string result;
      result.reserve(maxChars);
      size_t charsRead = 0;
      while (charsRead < maxChars && !_byteStream->EndOfStream())
      {
        // Read a chunk of bytes from the stream
        Array<byte, 4u> buffer {};
        size_t bytesRead = _byteStream->Read(buffer.data(), buffer.size());
        if (bytesRead == 0)
        {
          break; // End of stream reached
        }
        // Decode the bytes into characters
        Span<const byte> byteSpan(buffer.data(), bytesRead);
        utf8_string decodedChars = _encoding->Decode(byteSpan);
        result.append(decodedChars);
        charsRead += decodedChars.size();
      }
      return result;
    }
  };
}