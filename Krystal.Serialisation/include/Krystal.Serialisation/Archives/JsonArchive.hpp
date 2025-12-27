#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Stack.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Archives/Adapters/RapidJsonStreamAdapters.hpp"
#include "Krystal.Serialisation/Archives/BaseArchive.hpp"
#include "Krystal.Serialisation/Builtins.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <bit>
#include <format>
#include <memory>

namespace Krys::Serialisation
{
  class JsonArchiveWriter : public BaseArchiveWriter<JsonArchiveWriter>
  {
  public:
    struct Node
    {
      enum class Type
      {
        StartObject,
        InObject,
        StartArray,
        InArray
      } NodeType {Type::StartObject};

      uint32 NameCounter {0u};
    };

  private:
    using Stream = RapidJsonStreamWriterAdapter;
    using Writer = rapidjson::PrettyWriter<Stream>;

  private:
    Stream _stream;
    Writer _writer;
    stringview _nextName {}; // The next name, can be specified using a NamedField or generated automatically.
    Stack<Node> _nodes {};

  public:
    JsonArchiveWriter(IO::IStreamWriter &stream) noexcept : _stream(stream), _writer(_stream)
    {
      _writer.SetMaxDecimalPlaces(Writer::kDefaultMaxDecimalPlaces);
      _writer.SetIndent(' ', 2);
      _nodes.push({Node::Type::StartObject, 0u});
    }

    ~JsonArchiveWriter() noexcept override
    {
      FinishNode();
    }

    void SetNextFieldName(stringview name) noexcept
    {
      _nextName = name;
    }

    template <ArchiveBuiltin T>
    JsonArchiveWriter &Write(const T &value) noexcept
    {
      if constexpr (SameType<T, bool>)
      {
        _writer.Bool(value);
      }
      else if constexpr (OneOf<T, char, short, int8, int16, int32, long>)
      {
        _writer.Int(static_cast<int32>(value));
      }
      else if constexpr (OneOf<T, byte, uchar, ushort, uint8, uint16, uint32, ulong>)
      {
        _writer.Uint(static_cast<uint32>(value));
      }
      else if constexpr (OneOf<T, int64, long long>)
      {
        _writer.Int64(static_cast<int64>(value));
      }
      else if constexpr (OneOf<T, uint64, unsigned long long>)
      {
        _writer.Uint64(static_cast<uint64>(value));
      }
      else if constexpr (OneOf<T, float, double, long double>) // rapidjson does not have a Float() method
      {
        _writer.Double(static_cast<double>(value));
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

    void StartNode(Node::Type type)
    {
      WriteName();
      _nodes.push({type, 0u});
    }

    void FinishNode()
    {
      // Handles serialising empty objects (WriteName is never called)
      if (_nodes.top().NodeType == Node::Type::StartArray)
      {
        _writer.StartArray();
        _writer.EndArray();
      }
      else if (_nodes.top().NodeType == Node::Type::StartObject)
      {
        _writer.StartObject();
        _writer.EndObject();
      }
      // Exiting an object or array
      else if (_nodes.top().NodeType == Node::Type::InObject)
      {
        _writer.EndObject();
      }
      else if (_nodes.top().NodeType == Node::Type::InArray)
      {
        _writer.EndArray();
      }

      _nodes.pop();
    }

    void SetCurrentNodeAsArray() noexcept
    {
      _nodes.top().NodeType = Node::Type::StartArray;
    }

    void WriteName()
    {
      const auto &nodeType = _nodes.top().NodeType;

      // Start up either an object or an array, depending on state
      if (nodeType == Node::Type::StartArray)
      {
        _writer.StartArray();
        _nodes.top().NodeType = Node::Type::InArray;
      }
      else if (nodeType == Node::Type::StartObject)
      {
        _writer.StartObject();
        _nodes.top().NodeType = Node::Type::InObject;
      }

      // Array types do not output names
      if (nodeType == Node::Type::InArray)
      {
        return;
      }

      if (_nextName.empty())
      {
        auto nextName = std::format("value{}", _nodes.top().NameCounter++);
        _writer.Key(nextName.data(), static_cast<rapidjson::SizeType>(nextName.size()));
      }
      else
      {
        _writer.Key(_nextName.data(), static_cast<rapidjson::SizeType>(_nextName.size()));
        _nextName = {};
      }
    }
  };

  class JsonArchiveReader : public BaseArchiveReader<JsonArchiveReader>
  {
    using Stream = RapidJsonStreamReaderAdapter;
    using Reader = rapidjson::Reader;
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
        else if (_type == Type::Null)
        {
          throw std::exception("Null or empty iterator");
        }

        return _type == Type::Value ? _valueIteratorBegin[_index] : _memberIteratorBegin[_index].value;
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
      void Search(const char *searchName)
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
          (string("Parsing failed: provided NamedField not found.") + string(searchName)).c_str());
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
    stringview _nextName {};              /// @brief Next name set by NamedField
    Stream _stream;                   /// @brief Rapidjson read stream
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

    /// @brief Sets the name for the next node created with StartNode
    void SetNextFieldName(stringview name)
    {
      _nextName = name;
    }

    template <ArchiveBuiltin T>
    JsonArchiveReader &Read(T &value) noexcept
    {
      const GenericValue &node = _iteratorStack.back().Value();

      if constexpr (SameType<T, bool>)
      {
        value = node.GetBool();
      }
      else if constexpr (OneOf<T, char, short, int8, int16, int32, long>)
      {
        value = static_cast<T>(node.GetInt());
      }
      else if constexpr (OneOf<T, byte, uchar, ushort, uint8, uint16, uint32, ulong>)
      {
        value = static_cast<T>(node.GetUint());
      }
      else if constexpr (OneOf<T, int64, long long>)
      {
        value = node.GetInt64();
      }
      else if constexpr (OneOf<T, uint64, unsigned long long>)
      {
        value = node.GetUint64();
      }
      else if constexpr (SameType<T, float>)
      {
        value = node.GetFloat();
      }
      else if constexpr (OneOf<T, double, long double>)
      {
        value = node.GetDouble();
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

    template <typename T>
    void Read(ContainerSize<T> &value) noexcept
    {
      using Size = RemoveRef<typename ContainerSize<T>::SizeType>;
      Size size = _iteratorStack.size() == 1
                    ? static_cast<Size>(_document.Size())
                    : static_cast<Size>((_iteratorStack.rbegin() + 1)->Value().Size());
      value.Size = size;
    }

    /// @brief This places an iterator for the next node to be parsed onto the iterator stack. Depending on
    /// the node's type it will be a member or value iterator to allow arrays and objects to be iterated
    /// recursively.
    void StartNode()
    {
      Search();

      const GenericValue &value = _iteratorStack.back().Value();
      if (value.IsArray())
      {
        _iteratorStack.emplace_back(value.Begin(), value.End());
      }
      else
      {
        _iteratorStack.emplace_back(value.MemberBegin(), value.MemberEnd());
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

      stringview requiredFieldName = _nextName;
      _nextName = {};

      string currentFieldName = _iteratorStack.back().Name();
      if (currentFieldName != requiredFieldName)
      {
        _iteratorStack.back().Search(requiredFieldName.data());
      }
    }

    /// @brief Advance to the next node in the current object or array.
    void Next()
    {
      ++_iteratorStack.back();
    }
  };

  template <>
  struct ArchiveTraits<JsonArchiveWriter>
  {
    static constexpr bool IsWriter = true;
    static constexpr bool IsReader = false;
    static constexpr bool IsBinary = false;
    static constexpr bool IsText = true;
  };

  template <>
  struct ArchiveTraits<JsonArchiveReader>
  {
    static constexpr bool IsWriter = false;
    static constexpr bool IsReader = true;
    static constexpr bool IsBinary = false;
    static constexpr bool IsText = true;
  };

  template <typename T>
  concept IsJsonArchive = OneOf<T, JsonArchiveWriter, JsonArchiveReader>;

  template <typename T>
  void Transfer(JsonArchiveReader &archive, ContainerSize<T> &value) noexcept
  {
    archive.Read(value); // we get this value by counting the number of elements in the array/object.
  }

  template <typename T>
  requires(!ArchiveNamedField<T>)
  void BeforeTransfer(JsonArchiveWriter &archive, const T &) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      archive.WriteName(); // we're still inside a {} or [].
    }
    else if constexpr (ArchiveContainerSize<T>)
    {
      archive.SetCurrentNodeAsArray(); // we don't need to store size, just start the array.
    }
    else if constexpr (ArchiveCustom<T> || ArchiveKeyValuePair<T>)
    {
      archive.StartNode(JsonArchiveWriter::Node::Type::StartObject); // enters {}. also calls WriteName().
    }
    else if constexpr (IsArray<T>)
    {
      archive.StartNode(JsonArchiveWriter::Node::Type::StartArray); // enters []
    }
    else if constexpr (ArchiveVersion<T>)
    {
      archive.SetNextFieldName(Version::FieldName);
    }
  }

  template <typename T>
  requires(ArchiveCustom<T> || ArchiveKeyValuePair<T>)
  void AfterTransfer(JsonArchiveWriter &archive, const T &) noexcept
  {
    archive.FinishNode(); // exits {} or [].
  }

  template <typename T>
  requires(!ArchiveNamedField<T>)
  void BeforeTransfer(JsonArchiveReader &archive, const T &) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      archive.Search(); // Ensure we're at the right node before reading a value.
    }
    else if constexpr (ArchiveCustom<T> || ArchiveKeyValuePair<T>)
    {
      archive.StartNode(); // enter {} or [].
    }
    else if constexpr (ArchiveVersion<T>)
    {
      archive.SetNextFieldName(Version::FieldName);
    }
  }

  template <typename T>
  void AfterTransfer(JsonArchiveReader &archive, const T &) noexcept
  {
    if constexpr (ArchiveBuiltin<T>)
    {
      archive.Next(); // move to next value in {} or [].
    }
    else if constexpr (ArchiveCustom<T> || ArchiveKeyValuePair<T>)
    {
      archive.FinishNode(); // exit {} or [].
    }
  }

  template <IsJsonArchive Archive>
  void Transfer(Archive &archive, Version &version) noexcept
  {
    archive(version.Value);
  }
}