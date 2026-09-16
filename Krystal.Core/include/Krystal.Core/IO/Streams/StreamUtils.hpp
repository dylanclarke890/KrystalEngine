#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Endian.hpp"
#include "Krystal.Core/IO/Streams/Stream.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/ByteUtils.hpp"

namespace krys::io
{
  class StreamUtils
  {
    KRYS_NON_COPY_MOVABLE(StreamUtils);

    StreamUtils() = delete;
    ~StreamUtils() = delete;

  public:
    template <UnsignedIntegral T, DerivedFrom<IStreamReader> TReader>
    KRYS_NODISCARD static Expected<List<T>> ReadAllAs(TReader &stream) noexcept
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
      auto bytesRead = stream.Read(buffer);

      if (bytesRead != size)
      {
        return Unexpected("Failed to read all bytes from the stream.");
      }

      stream.Close();

      return ByteUtils::AsNumericArray<Endian::System, Endian::System, T>(buffer);
    }

    template <DerivedFrom<IStreamReader> TReader>
    KRYS_NODISCARD static Expected<List<byte>> ReadAll(TReader &stream) noexcept
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
      auto bytesRead = stream.Read(buffer);

      if (bytesRead != size)
      {
        return Unexpected("Failed to read all bytes from the stream.");
      }

      stream.Close();

      return buffer;
    }

    template <DerivedFrom<IStreamReader> TReader>
    KRYS_NODISCARD static Expected<string> ReadAllText(TReader &stream) noexcept
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