#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include "Krystal.Serialisation/Dispatch.hpp"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include <bit>
#include <memory>

namespace Krys::Serialisation
{
  class JsonArchiveWriter
  {
    using rapidjsonStream = rapidjson::OStreamWrapper;
    using rapidjsonWriter = rapidjson::PrettyWriter<rapidjsonStream>;

    struct Settings
    {
      char IndentChar = ' ';
      uint IndentLength = 4;
      int DecimalPrecision = rapidjsonWriter::kDefaultMaxDecimalPlaces;
    };

    enum class NodeType
    {
      StartObject,
      InObject,
      StartArray,
      InArray
    };

  private:
    IO::IStreamWriter &_stream;
    Settings _settings;
    rapidjsonStream _rapidjsonStream;
    rapidjsonWriter _rapidJsonWriter;
    char const *itsNextName;        // The next name
    Stack<uint32_t> itsNameCounter; // Counter for creating unique names for unnamed nodes
    Stack<NodeType> itsNodeStack;

  public:
    JsonArchiveWriter(IO::IStreamWriter &stream, const Settings &settings = {}) noexcept
        : _stream(stream), _settings(settings)
    {
    }

    template <ArchiveBuiltin T>
    JsonArchiveWriter &operator()(const T &value) noexcept
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
    JsonArchiveWriter &operator()(const T &value) noexcept
    {
      TransferGuard guard(*this, value);
      DispatchSave(*this, value);
      return *this;
    }

    template <class... Types>
    requires(sizeof...(Types) > 1)
    JsonArchiveWriter &operator()(Types &&...types) noexcept
    {
      ((void)(*this)(std::forward<Types>(types)), ...);
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
    JsonArchiveReader &operator()(T &value) noexcept
    {
      TransferGuard guard(*this, value);
      DispatchLoad(*this, value);
      return *this;
    }

    template <class... Types>
    requires(sizeof...(Types) > 1)
    JsonArchiveReader &operator()(Types &...types) noexcept
    {
      ((void)(*this)(types), ...);
      return *this;
    }
  };
}