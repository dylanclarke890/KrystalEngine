#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Archives/BaseArchive.hpp"
#include "Krystal.Serialisation/Builtins.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include "Krystal.Serialisation/Helpers/RapidJsonStreamAdapters.hpp"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <bit>
#include <format>
#include <memory>

namespace Krys::Serialisation
{
  class JsonArchiveWriter : public BaseArchiveWriter<JsonArchiveWriter>
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

    ~JsonArchiveWriter() noexcept override
    {
      FinishNode();
    }

    template <ArchiveBuiltin T>
    JsonArchiveWriter &Write(const T &value) noexcept
    {
      if constexpr (SameType<T, bool>)
      {
        _writer.Bool(value);
      }
      else if constexpr (SameType<T, int>)
      {
        _writer.Int(value);
      }
      else if constexpr (SameType<T, uint>)
      {
        _writer.Uint(value);
      }
      else if constexpr (SameType<T, int64>)
      {
        _writer.Int64(value);
      }
      else if constexpr (SameType<T, uint64>)
      {
        _writer.Uint64(value);
      }
      else if constexpr (OneOf<T, float, double>) // rapidjson does not have a Float() method
      {
        _writer.Double(value);
      }
      else if constexpr (SameType<T, byte>)
      {
        _writer.Uint(static_cast<uint>(value));
      }
      else if constexpr (SameType<T, string>)
      {
        _writer.String(value.c_str(), static_cast<rapidjson::SizeType>(value.size()));
      }
      else
      {
        static_assert(DependentFalse<T>, "Unsupported type");
      }

      return *this;
    }

    void SetNextName(const string &name) noexcept
    {
      _nextName = name;
    }

    void StartArray() noexcept
    {
      _nodeStack.top() = NodeType::StartArray;
    }

    void WriteName()
    {
      const NodeType &nodeType = _nodeStack.top();

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
        _nextName = std::format("value{}", _counter.top()++);
      }

      _writer.Key(_nextName.c_str(), static_cast<rapidjson::SizeType>(_nextName.size()));
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
      // if we ended up serializing an empty object or array, WriteName
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
  };

  class JsonArchiveReader : public BaseArchiveReader<JsonArchiveReader>
  {
    using rapidjsonStream = RapidJsonStreamReaderAdapter;
    using rapidjsonReader = rapidjson::Reader;
    using JSONValue = rapidjson::GenericValue<rapidjson::UTF8<>>;
    using MemberIterator = JSONValue::ConstMemberIterator;
    using ValueIterator = JSONValue::ConstValueIterator;
    using GenericValue = rapidjson::Document::GenericValue;

    // @brief An internal iterator that handles both array and object types.
    class Iterator
    {
    public:
      Iterator() noexcept = default;

      Iterator(MemberIterator begin, MemberIterator end) noexcept
          : _memberIteratorBegin(begin), _memberIteratorEnd(end), _index(0u),
            _size(std::distance(begin, end)), _type(_size == 0 ? Type::Null : Type::Member)
      {
      }

      Iterator(ValueIterator begin, ValueIterator end) noexcept
          : _valueIteratorBegin(begin), _index(0u), _size(std::distance(begin, end)),
            _type(_size == 0 ? Type::Null : Type::Value)
      {
      }

      /// @brief Advance to the next node.
      Iterator &operator++()
      {
        _index++;
        return *this;
      }

      /// @brief Get the value of the current node
      GenericValue const &Value()
      {
        if (_index >= _size)
        {
          throw std::exception("No more objects in input");
        }

        switch (_type)
        {
          case Type::Value:  return _valueIteratorBegin[_index];
          case Type::Member: return _memberIteratorBegin[_index].value;
          default:
          {
            throw std::exception(
              "JSONInputArchive internal error: null or empty iterator to object or array!");
          }
        }
      }

      /// @brief Get the name of the current node, or nullptr if it has no name
      const char *Name() const noexcept
      {
        if (_type == Type::Member && (_memberIteratorBegin + _index) != _memberIteratorEnd)
        {
          return _memberIteratorBegin[_index].name.GetString();
        }
        else
        {
          return nullptr;
        }
      }

      /// @brief Adjust our position such that we are at the node with the given name.
      /// @throws Exception if no such named node exists.
      inline void Search(const char *searchName)
      {
        const auto len = std::strlen(searchName);
        size_t index = 0;
        for (auto it = _memberIteratorBegin; it != _memberIteratorEnd; ++it, ++index)
        {
          const auto currentName = it->name.GetString();
          if ((std::strncmp(searchName, currentName, len) == 0) && (std::strlen(currentName) == len))
          {
            _index = index;
            return;
          }
        }

        throw std::exception(
          (string("JSON Parsing failed - provided NVP not found.") + string(searchName)).c_str());
      }

    private:
      MemberIterator _memberIteratorBegin {}, _memberIteratorEnd {}; /// @brief The member iterator (object)
      ValueIterator _valueIteratorBegin {};                          /// @brief The value iterator (array)
      size_t _index {0u}, _size {0u}; /// @brief The current index of this iterator
      enum class Type
      {
        Value,
        Member,
        Null
      } _type {Type::Null}; /// @brief Whether this holds values (array) or members (objects) or nothing
    };

  private:
    string _nextName {};              /// @brief Next name set by NVP
    rapidjsonStream _stream;          /// @brief Rapidjson read stream
    List<Iterator> _iteratorStack {}; /// @brief 'Stack' of rapidJSON iterators
    rapidjson::Document _document;    /// @brief Rapidjson document

  public:
    JsonArchiveReader(IO::IStreamReader &stream) noexcept : _stream(stream)
    {
      _document.ParseStream(_stream);
      assert(!_document.HasParseError() && "Document parsing failed");

      if (_document.IsArray())
      {
        _iteratorStack.emplace_back(_document.Begin(), _document.End());
      }
      else if (_document.IsObject())
      {
        _iteratorStack.emplace_back(_document.MemberBegin(), _document.MemberEnd());
      }
    }

    ~JsonArchiveReader() noexcept override
    {
    }

    template <ArchiveBuiltin T>
    JsonArchiveReader &Read(T &value) noexcept
    {
      const GenericValue &node = _iteratorStack.back().Value();

      if constexpr (SameType<T, bool>)
      {
        value = node.GetBool();
      }
      else if constexpr (SameType<T, int>)
      {
        value = node.GetInt();
      }
      else if constexpr (SameType<T, uint>)
      {
        value = node.GetUint();
      }
      else if constexpr (SameType<T, int64>)
      {
        value = node.GetInt64();
      }
      else if constexpr (SameType<T, uint64>)
      {
        value = node.GetUint64();
      }
      else if constexpr (SameType<T, float>)
      {
        value = node.GetFloat();
      }
      else if constexpr (SameType<T, double>)
      {
        value = node.GetDouble();
      }
      else if constexpr (SameType<T, byte>)
      {
        value = static_cast<byte>(node.GetUint());
      }
      else if constexpr (SameType<T, string>)
      {
        value = node.GetString();
      }
      else
      {
        static_assert(DependentFalse<T>, "Unsupported type");
      }

      return *this;
    }

    /// @brief This places an iterator for the next node to be parsed onto the iterator stack. Depending on
    /// the node's type it will be a member or value iterator to allow arrays and objects to be iterated
    /// recursively.
    void StartNode()
    {
      Search();

      if (_iteratorStack.back().Value().IsArray())
      {
        _iteratorStack.emplace_back(_iteratorStack.back().Value().Begin(),
                                    _iteratorStack.back().Value().End());
      }
      else
      {
        _iteratorStack.emplace_back(_iteratorStack.back().Value().MemberBegin(),
                                    _iteratorStack.back().Value().MemberEnd());
      }
    }

    /// @brief Finishes the most recently started node.
    void FinishNode()
    {
      _iteratorStack.pop_back();
      Next();
    }

    /// @brief Must be called on every load/node start. Ensures the iterator is aligned with the named node.
    void Search()
    {
      if (_nextName.empty())
      {
        return;
      }

      string localNextName = _nextName;
      _nextName.clear();

      // The name an NVP provided with setNextName()
      if (!localNextName.empty())
      {
        // The actual name of the current node
        string actualName = _iteratorStack.back().Name();

        // Do a search if we don't see a name coming up, or if the names don't match
        if (localNextName != actualName)
        {
          _iteratorStack.back().Search(localNextName.c_str());
        }
      }
    }

    /// @brief Sets the name for the next node created with StartNode
    void SetNextName(const string &name)
    {
      _nextName = name;
    }

    /// @brief Advance to the next node in the current object or array.
    void Next()
    {
      ++_iteratorStack.back();
    }
  };

  template <typename T>
  void BeforeTransfer(JsonArchiveWriter &archive, const T &value) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      archive.WriteName(); // we're still inside a {} or [].
    }
    else if constexpr (ArchiveNamedField<T>)
    {
      archive.SetNextName(value.Name); // Set the name for the next node.
    }
    else if constexpr (ArchiveCustom<T>)
    {
      archive.StartNode(); // enter {} or []. also calls WriteName().
    }
  }

  template <typename T>
  void AfterTransfer(JsonArchiveWriter &archive, const T &) noexcept
  {
    if constexpr (ArchiveCustom<T>)
    {
      archive.FinishNode(); // exit {} or [].
    }
  }

  template <typename T>
  void BeforeTransfer(JsonArchiveReader &archive, const T &value) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      archive.Search(); // consume _nextName if set, aligning the iterator to that member
    }
    else if constexpr (ArchiveNamedField<T>)
    {
      archive.SetNextName(value.Name);
    }
    else if constexpr (ArchiveCustom<T>)
    {
      archive.StartNode(); // enter {} or [].
    }
  }

  template <typename T>
  void AfterTransfer(JsonArchiveReader &archive, const T &) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      archive.Next(); // move to next value in {} or [].
    }
    else if constexpr (ArchiveCustom<T>)
    {
      archive.FinishNode(); // exit {} or [].
    }
  }
}