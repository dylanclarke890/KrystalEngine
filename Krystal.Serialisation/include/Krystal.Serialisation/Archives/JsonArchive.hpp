#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Serialisation/IArchive.hpp"
#include <bit>
#include <memory>

namespace Krys::Serialisation
{
  class JsonArchiveWriter : public IArchiveWriter<JsonArchiveWriter>
  {
    using Base = IArchiveWriter<JsonArchiveWriter>;

  public:
    JsonArchiveWriter(IO::IStreamWriter &stream) noexcept : Base(stream)
    {
    }

    template <Arithmetic T>
    JsonArchiveWriter &operator()(T &value) noexcept
    {
      auto *data = std::bit_cast<byte *>(std::addressof(value));
      _stream.Write(data, sizeof(T));

      return *this;
    }

    JsonArchiveWriter &operator()(byte &value) noexcept
    {
      _stream.Write(std::addressof(value), sizeof(byte));

      return *this;
    }

    JsonArchiveWriter &operator()(const string &value) noexcept
    {
      size_t length = value.length();
      (*this)(length);

      if (length == 0)
        return *this;

      auto *data = std::bit_cast<const byte *>(value.data());
      _stream.Write(data, length);

      return *this;
    }
  };

  class JsonArchiveReader : public IArchiveReader<JsonArchiveReader>
  {
    using Base = IArchiveReader<JsonArchiveReader>;

  public:
    JsonArchiveReader(IO::IStreamReader &stream) noexcept : Base(stream)
    {
    }

    template <Arithmetic T>
    JsonArchiveReader &operator()(T &value) noexcept
    {
      auto *data = std::bit_cast<byte *>(std::addressof(value));
      _stream.Read(data, sizeof(T));
      return *this;
    }

    JsonArchiveReader &operator()(byte &value) noexcept
    {
      _stream.Read(&value, 1);
      return *this;
    }

    JsonArchiveReader &operator()(string &value) noexcept
    {
      size_t length;
      (*this)(length);

      if (length == 0)
        return *this;

      value.resize(length);
      auto *data = std::bit_cast<byte *>(value.data());
      _stream.Read(data, length);

      return *this;
    }
  };
}