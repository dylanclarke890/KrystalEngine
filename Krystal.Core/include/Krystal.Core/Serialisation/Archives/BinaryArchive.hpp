#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/IO/Streams/Stream.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Serialisation/Archives/BaseArchive.hpp"
#include "Krystal.Core/Serialisation/Builtins.hpp"
#include "Krystal.Core/Serialisation/Concepts.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <bit>
#include <memory>

namespace krys::Serialisation
{
  class BinaryArchiveWriter : public BaseArchiveWriter<BinaryArchiveWriter>
  {
  private:
    io::IStreamWriter &_stream;

  public:
    BinaryArchiveWriter(io::IStreamWriter &stream) noexcept : _stream(stream)
    {
    }

    ~BinaryArchiveWriter() noexcept override = default;

    template <ArchiveBuiltin T>
    BinaryArchiveWriter &Write(const T &value) noexcept
    {
      if constexpr (SameType<T, byte>)
      {
        _stream.Write(FixedSpan(std::addressof(value), 1u));
      }
      else if constexpr (SameType<T, string>)
      {
        size_t length = value.length();
        (*this)(length);
        if (length == 0)
          return *this;
        auto *data = std::bit_cast<const byte *>(value.data());
        _stream.Write(Span(data, length));
      }
      else
      {
        auto *data = std::bit_cast<byte *>(std::addressof(value));
        _stream.Write(FixedSpan(data, sizeof(T)));
      }

      return *this;
    }
  };

  class BinaryArchiveReader : public BaseArchiveReader<BinaryArchiveReader>
  {
  private:
    io::IStreamReader &_stream;

  public:
    BinaryArchiveReader(io::IStreamReader &stream) noexcept : _stream(stream)
    {
    }

    ~BinaryArchiveReader() noexcept override = default;

    template <ArchiveBuiltin T>
    BinaryArchiveReader &Read(T &value) noexcept
    {
      if constexpr (SameType<T, byte>)
      {
        _stream.Read(FixedSpan(std::addressof(value), 1u));
      }
      else if constexpr (SameType<T, string>)
      {
        size_t length {};
        (*this)(length);

        if (length == 0)
          return *this;

        value.resize(length);
        auto *data = std::bit_cast<byte *>(value.data());
        _stream.Read(Span(data, length));
      }
      else
      {
        auto *data = std::bit_cast<byte *>(std::addressof(value));
        _stream.Read(FixedSpan(data, sizeof(T)));
      }

      return *this;
    }
  };

  template <>
  struct ArchiveTraits<BinaryArchiveWriter>
  {
    constexpr static bool IsWriter = true;
    constexpr static bool IsReader = false;
    constexpr static bool IsBinary = true;
    constexpr static bool IsText = false;
  };

  template <>
  struct ArchiveTraits<BinaryArchiveReader>
  {
    constexpr static bool IsWriter = false;
    constexpr static bool IsReader = true;
    constexpr static bool IsBinary = true;
    constexpr static bool IsText = false;
  };

  template <IsBinaryArchive Archive>
  void Transfer(Archive &archive, Version &version) noexcept
  {
    archive(version.Value);
  }
}