#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Archives/BaseArchive.hpp"
#include "Krystal.Serialisation/Builtins.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include <bit>
#include <memory>

namespace Krys::Serialisation
{
  class BinaryArchiveWriter : public BaseArchiveWriter<BinaryArchiveWriter>
  {
  private:
    IO::IStreamWriter &_stream;

  public:
    BinaryArchiveWriter(IO::IStreamWriter &stream) noexcept : _stream(stream)
    {
    }

    ~BinaryArchiveWriter() noexcept override = default;

    template <ArchiveBuiltin T>
    BinaryArchiveWriter &Write(const T &value) noexcept
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
  };

  class BinaryArchiveReader : public BaseArchiveReader<BinaryArchiveReader>
  {
  private:
    IO::IStreamReader &_stream;

  public:
    BinaryArchiveReader(IO::IStreamReader &stream) noexcept : _stream(stream)
    {
    }

    ~BinaryArchiveReader() noexcept override = default;

    template <ArchiveBuiltin T>
    BinaryArchiveReader &Read(T &value) noexcept
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
  };

  template <typename T>
  void Transfer(BinaryArchiveWriter &archive, NamedField<T> &field) noexcept
  {
    archive(field.Value);
  }

  template <typename T>
  void Transfer(BinaryArchiveReader &archive, NamedField<T> &field) noexcept
  {
    archive(field.Value);
  }
}