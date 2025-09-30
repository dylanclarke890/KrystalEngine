#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include "Krystal.Serialisation/Dispatch.hpp"
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

    template <ArchiveBuiltin T>
    BinaryArchiveWriter &operator()(const T &value) noexcept
    {
      TransferGuard guard(*this, value);

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

    template <ArchiveCustom T>
    BinaryArchiveWriter &operator()(const T &value) noexcept
    {
      TransferGuard guard(*this, value);
      DispatchSave(*this, value);
      return *this;
    }

    template <class... Types>
    requires(sizeof...(Types) > 1)
    BinaryArchiveWriter &operator()(Types &&...types) noexcept
    {
      ((void)(*this)(std::forward<Types>(types)), ...);
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

    template <ArchiveBuiltin T>
    BinaryArchiveReader &operator()(T &value) noexcept
    {
      TransferGuard guard(*this, value);

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

    template <ArchiveCustom T>
    BinaryArchiveReader &operator()(T &value) noexcept
    {
      TransferGuard guard(*this, value);
      DispatchLoad(*this, value);
      return *this;
    }

    template <class... Types>
    requires(sizeof...(Types) > 1)
    BinaryArchiveReader &operator()(Types &...types) noexcept
    {
      ((void)(*this)(types), ...);
      return *this;
    }
  };
}