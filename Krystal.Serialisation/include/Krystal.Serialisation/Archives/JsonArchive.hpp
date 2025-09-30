#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include <bit>
#include <memory>

namespace Krys::Serialisation
{
  class JsonArchiveWriter
  {
  private:
    IO::IStreamWriter &_stream;

  public:
    JsonArchiveWriter(IO::IStreamWriter &stream) noexcept : _stream(stream)
    {
    }

    template <ArchiveBuiltin T>
    JsonArchiveWriter &operator()(const T &value) noexcept
    {
      if constexpr (Arithmetic<T>)
      {
        auto *data = std::bit_cast<byte *>(std::addressof(value));
        _stream.Write(data, sizeof(T));
      }
      else if constexpr (SameType<T, byte>)
      {
        _stream.Write(std::addressof(value), sizeof(byte));
      }
      else if constexpr (SameType<T, string>)
      {
        size_t length = value.length();
        (*this)(length);
        if (length == 0)
          return *this;
        auto *data = std::bit_cast<const byte *>(value.data());
        _stream.Write(data, length);
      }

      return *this;
    }

    template <NonArchiveBuiltin T>
    JsonArchiveWriter &operator()(const T &obj) noexcept
    {
      Save(*this, obj);
      return *this;
    }
  };

  class JsonArchiveReader
  {
  private:
    IO::IStreamReader &_stream;

  public:
    JsonArchiveReader(IO::IStreamReader &stream) noexcept : _stream(stream)
    {
    }

    template <ArchiveBuiltin T>
    JsonArchiveReader &operator()(T &value) noexcept
    {
      if constexpr (Arithmetic<T>)
      {
        auto *data = std::bit_cast<byte *>(std::addressof(value));
        _stream.Read(data, sizeof(T));
      }
      else if constexpr (SameType<T, byte>)
      {
        _stream.Read(&value, 1);
      }
      else if constexpr (SameType<T, string>)
      {
        size_t length {};
        (*this)(length);

        if (length == 0)
          return *this;

        value.resize(length);
        auto *data = std::bit_cast<byte *>(value.data());
        _stream.Read(data, length);
      }

      return *this;
    }

    template <NonArchiveBuiltin T>
    JsonArchiveReader &operator()(T &obj) noexcept
    {
      Load(*this, obj);
      return *this;
    }
  };
}