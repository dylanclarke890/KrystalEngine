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
      if constexpr (SameType<T, byte>)
      {
        _stream.Write(std::addressof(value), 1);
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
      else
      {
        auto *data = std::bit_cast<byte *>(std::addressof(value));
        _stream.Write(data, sizeof(T));
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
      if constexpr (SameType<T, byte>)
      {
        _stream.Read(std::addressof(value), 1);
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
      else
      {
        auto *data = std::bit_cast<byte *>(std::addressof(value));
        _stream.Read(data, sizeof(T));
      }

      return *this;
    }
  };

  template <>
  struct ArchiveTraits<BinaryArchiveWriter>
  {
    static constexpr bool IsWriter = true;
    static constexpr bool IsReader = false;
    static constexpr bool IsBinary = true;
    static constexpr bool IsText = false;
  };

  template <>
  struct ArchiveTraits<BinaryArchiveReader>
  {
    static constexpr bool IsWriter = false;
    static constexpr bool IsReader = true;
    static constexpr bool IsBinary = true;
    static constexpr bool IsText = false;
  };
}