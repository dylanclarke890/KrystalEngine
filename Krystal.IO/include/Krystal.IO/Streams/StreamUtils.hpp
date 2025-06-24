#pragma once

#include "Krystal.Core/ByteUtils.hpp"
#include "Krystal.Core/Core.hpp"
#include "Krystal.IO/IStream.hpp"

namespace Krys::IO
{
  struct StreamUtils final
  {
    STATIC_CLASS(StreamUtils)

    template <UnsignedIntegral T, DerivedFrom<IStreamReader> TReader>
    static Expected<List<T>> ReadAllAs(TReader &stream) noexcept
    {
      stream.Open();
      if (!stream.IsOpen())
      {
        return Unexpected("Failed to open stream.");
      }

      auto size = stream.Size();
      if (size == 0)
        return List<T>();

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
  };
}