#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Builtins.hpp"
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
      FinishNode();
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

    template <ArchiveBuiltin T>
    JsonArchiveWriter &operator()(const T &value) noexcept
    {
      TransferGuard guard(*this, value);

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

    template <typename T>
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

    /// @brief This places an iterator for the next node to be parsed onto the iterator stack.  If the next
    /// node is an array, this will be a value iterator, otherwise it will be a member iterator.
    /// By default our strategy is to start with the document root node and then recursively iterate through
    /// all children in the order they show up in the document.
    /// We don't need to know NVPs to do this; we'll just blindly load in the order things appear in.
    /// If we were given an NVP, we will search for it if it does not match our the name of the next node
    /// that would normally be loaded.  This functionality is provided by search().
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

    /// @brief Finishes the most recently started node
    void FinishNode()
    {
      _iteratorStack.pop_back();
      Next();
    }

    /// @brief Searches for the expectedName node if it doesn't match the actualName
    /// @brief This needs to be called before every load or node start occurs.  This function will
    /// check to see if an NVP has been provided (with setNextName) and if so, see if that name matches the
    /// actual next name given.  If the names do not match, it will search in the current level of the JSON
    /// for that name. If the name is not found, an exception will be thrown. Resets the NVP name after
    /// called.
    /// @throws Exception if a match is not found for the provided NVP name.
    void Search()
    {
      // store pointer to itsNextName locally and reset to nullptr in case search() throws
      string localNextName = _nextName;
      _nextName = string();

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

    /// @brief Retrieves the current node name
    /// @return nullptr if no name exists
    const char *GetNodeName() const
    {
      return _iteratorStack.back().Name();
    }

    /// @brief Sets the name for the next node created with StartNode
    void SetNextName(const string &name)
    {
      _nextName = name;
    }

    void Next()
    {
      ++_iteratorStack.back();
    }

    template <ArchiveBuiltin T>
    JsonArchiveReader &operator()(T &value) noexcept
    {
      TransferGuard guard(*this, value);

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

    template <typename T>
    JsonArchiveReader &operator()(T &value) noexcept
    {
      TransferGuard guard(*this, value);
      DispatchLoad(*this, value);
      return *this;
    }

    template <class... Types>
    requires(sizeof...(Types) > 1)
    JsonArchiveReader &operator()(Types &&...types) noexcept
    {
      ((void)(*this)(types), ...);
      return *this;
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
    else
    {
      archive.StartNode(); // enter {} or []. also calls WriteName().
    }
  }

  template <typename T>
  void AfterTransfer(JsonArchiveWriter &archive, const T &) noexcept
  {
    if constexpr (!ArchiveBuiltin<T> && !ArchiveNamedField<T>)
    {
      archive.FinishNode(); // exit {} or [].
    }
  }

  template <typename T>
  void BeforeTransfer(JsonArchiveReader &archive, const T &value) noexcept
  {
    if constexpr (ArchiveNamedField<T>)
    {
      archive.SetNextName(value.Name);
    }
    else if constexpr (ArchiveBuiltin<T>)
    {
      archive.Search(); // consume _nextName if set, aligning the iterator to that member
    }
    else
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
    else if constexpr (!ArchiveNamedField<T>)
    {
      archive.FinishNode(); // exit {} or [].
    }
  }
}