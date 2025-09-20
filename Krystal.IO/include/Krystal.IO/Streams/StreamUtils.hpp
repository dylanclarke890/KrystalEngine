#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::IO
{
  struct StreamUtils final
  {
    STATIC_CLASS(StreamUtils)

    template <UnsignedIntegral T, DerivedFrom<IStreamReader> TReader>
    NO_DISCARD static Expected<List<T>> ReadAllAs(TReader &stream) noexcept
    {
      stream.Open();
      if (!stream.IsOpen())
      {
        return Unexpected("Failed to open stream.");
      }

      auto size = stream.Size();
      if (size == 0)
      {
        return List<T>();
      }

      if (size % sizeof(T) != 0)
      {
        return Unexpected("Stream size is not a multiple of the type size.");
      }

      List<byte> buffer(size);
      auto bytesRead = stream.Read(buffer.data(), size);

      if (bytesRead != size)
      {
        return Unexpected("Failed to read all bytes from the stream.");
      }

      stream.Close();

      return ByteUtils::AsNumericArray<T>(buffer);
    }

    template <DerivedFrom<IStreamReader> TReader>
    NO_DISCARD static Expected<List<byte>> ReadAll(TReader &stream) noexcept
    {
      stream.Open();
      if (!stream.IsOpen())
      {
        return Unexpected("Failed to open stream.");
      }

      auto size = stream.Size();
      if (size == 0)
      {
        return List<byte>();
      }

      List<byte> buffer(size);
      auto bytesRead = stream.Read(buffer.data(), size);

      if (bytesRead != size)
      {
        return Unexpected("Failed to read all bytes from the stream.");
      }

      stream.Close();

      return buffer;
    }

    template <DerivedFrom<IStreamReader> TReader>
    NO_DISCARD static Expected<string> ReadAllText(TReader &stream) noexcept
    {
      return ReadAll(stream).and_then(
        [](List<byte> &&bytes) -> Expected<string>
        {
          auto size = bytes.size();
          if (size == 0)
          {
            return string();
          }
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          return string(reinterpret_cast<const char *>(bytes.data()), size);
        });
    }
  };
}