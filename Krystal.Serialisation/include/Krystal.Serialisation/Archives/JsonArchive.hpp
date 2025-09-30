#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include "Krystal.Serialisation/Dispatch.hpp"
#include "Krystal.Serialisation/Helpers/RapidJsonStreamAdapters.hpp"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <bit>
#include <format>
#include <memory>

namespace Krys::Serialisation
{
  class JsonArchiveWriter
  {
    using rapidjsonStream = RapidJsonStreamWriterAdapter;
    using rapidjsonWriter = rapidjson::PrettyWriter<rapidjsonStream>;

    struct Settings
    {
      char IndentChar = ' '; // Must be ' ', '\t', '\n' or '\r'.
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
    Settings _settings;
    rapidjsonStream _stream;
    rapidjsonWriter _writer;
    string _nextName {}; // The next name, can be specified using a NamedField or generated automatically.
    Stack<uint32_t> _counter {}; // Counter for creating unique names for unnamed nodes
    Stack<NodeType> _nodeStack {};

  public:
    JsonArchiveWriter(IO::IStreamWriter &stream, const Settings &settings = {}) noexcept
        : _settings(settings), _stream(stream), _writer(_stream)
    {
      _writer.SetMaxDecimalPlaces(_settings.DecimalPrecision);
      _writer.SetIndent(_settings.IndentChar, _settings.IndentLength);
      _counter.push(0);
      _nodeStack.push(NodeType::StartObject);
    }

    ~JsonArchiveWriter() noexcept
    {
      if (_nodeStack.top() == NodeType::InObject)
      {
        _writer.EndObject();
      }
      else if (_nodeStack.top() == NodeType::InArray)
      {
        _writer.EndArray();
      }
    }

    void WriteName()
    {
      NodeType const &nodeType = _nodeStack.top();

      // Start up either an object or an array, depending on state
      if (nodeType == NodeType::StartArray)
      {
        _writer.StartArray();
        _nodeStack.top() = NodeType::InArray;
      }
      else if (nodeType == NodeType::StartObject)
      {
        _writer.StartObject();
        _nodeStack.top() = NodeType::InObject;
      }

      // Array types do not output names
      if (nodeType == NodeType::InArray)
      {
        return;
      }

      if (_nextName.empty())
      {
        _nextName = std::format("value{}\0", _counter.top()++);
      }

      (*this)(_nextName);
      _nextName.clear();
    }

    void StartNode()
    {
      WriteName();
      _nodeStack.push(NodeType::StartObject);
      _counter.push(0);
    }

    void FinishNode()
    {
      // if we ended up serializing an empty object or array, writeName
      // will never have been called - so start and then immediately end
      // the object/array.
      if (_nodeStack.top() == NodeType::StartArray)
      {
        _writer.StartArray();
        _writer.EndArray();
      }
      else if (_nodeStack.top() == NodeType::StartObject)
      {
        _writer.StartObject();
        _writer.EndObject();
      }
      else if (_nodeStack.top() == NodeType::InObject)
      {
        _writer.EndObject();
      }
      else if (_nodeStack.top() == NodeType::InArray)
      {
        _writer.EndArray();
      }

      _nodeStack.pop();
      _counter.pop();
    }

    void SetNextName(const char *name) noexcept
    {
      _nextName = name;
    }

    template <ArchiveBuiltin T>
    JsonArchiveWriter &operator()(const T &value) noexcept
    {
      TransferGuard guard(*this, value);

      if constexpr (SameType<T, bool>)
      {
        _writer.Bool(b);
      }
      else if constexpr (SameType<T, int>)
      {
        _writer.Int(i);
      }
      else if constexpr (SameType<T, uint>)
      {
        _writer.Uint(u);
      }
      else if constexpr (SameType<T, int64>)
      {
        _writer.Int64(i64);
      }
      else if constexpr (SameType<T, uint64>)
      {
        _writer.Uint64(u64);
      }
      else if constexpr (OneOf<T, double, float>) // rapidjson does not have a Float() method
      {
        _writer.Double(d);
      }
      else if constexpr (SameType<T, byte>)
      {
        _writer.Uint(static_cast<uint>(b));
      }
      else if constexpr (SameType<T, string>)
      {
        _writer.String(s.c_str(), static_cast<rapidjson::SizeType>(s.size()));
      }
      else
      {
        static_assert(DependentFalse<T>, "Unsupported arithmetic type");
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