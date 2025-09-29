#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include <bit>
#include <memory>

namespace Krys::Serialisation
{
  class BinaryArchiveWriter
  {
  private:
    IO::IStreamWriter &_stream;
    
  public:
    BinaryArchiveWriter(IO::IStreamWriter &stream) noexcept : _stream(stream)
    {
    }

    template <Arithmetic T>
    BinaryArchiveWriter &operator()(const T &value) noexcept
    {
      auto *data = std::bit_cast<byte *>(std::addressof(value));
      _stream.Write(data, sizeof(T));

      return *this;
    }

    BinaryArchiveWriter &operator()(const byte &value) noexcept
    {
      _stream.Write(std::addressof(value), sizeof(byte));

      return *this;
    }

    BinaryArchiveWriter &operator()(const string &value) noexcept
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

  class BinaryArchiveReader
  {
  private:
    IO::IStreamReader &_stream;

  public:
    BinaryArchiveReader(IO::IStreamReader &stream) noexcept : _stream(stream)
    {
    }

    template <Arithmetic T>
    BinaryArchiveReader &operator()(T &value) noexcept
    {
      auto *data = std::bit_cast<byte *>(std::addressof(value));
      _stream.Read(data, sizeof(T));
      return *this;
    }

    BinaryArchiveReader &operator()(byte &value) noexcept
    {
      _stream.Read(&value, 1);
      return *this;
    }

    BinaryArchiveReader &operator()(string &value) noexcept
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