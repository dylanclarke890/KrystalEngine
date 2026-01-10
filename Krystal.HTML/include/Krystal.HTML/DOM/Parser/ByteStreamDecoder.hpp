#pragma once

#include "Krystal.IO/Streams/Stream.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Array.hpp"

namespace Krys::HTML::DOM
{
  class ByteStreamDecoder : NonCopyMovable<ByteStreamDecoder>
  {
  private:
    IO::IStreamReader *_byteStream;

  public:
    ByteStreamDecoder(IO::IStreamReader *byteStream) noexcept : _byteStream(byteStream)
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

      }
      return result;
    }
  };
}