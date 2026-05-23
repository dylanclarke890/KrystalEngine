#pragma once

#include "Krystal.IO/Streams/Stream.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::IO
{
  struct StreamUtils : NonCopyMovable<StreamUtils>
  {
    StreamUtils() = delete;
    ~StreamUtils() = delete;

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