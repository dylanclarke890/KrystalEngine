#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Serialisation/Archives/BaseArchive.hpp"
#include "Krystal.Serialisation/Builtins.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include "rapidxml/include.hpp"
#include <format>
#include <sstream>

namespace Krys::Serialisation
{
  class XmlArchiveWriter : public BaseArchiveWriter<XmlArchiveWriter>
  {
    NO_COPY_MOVE(XmlArchiveWriter)

    struct NodeMetadata
    {
      rapidxml::xml_node<> *Node;
      size_t NameCounter;

      NodeMetadata(rapidxml::xml_node<> *node) noexcept : Node(node), NameCounter(0u)
      {
      }

      NO_DISCARD string GetNextFieldName() noexcept
      {
        return std::format("Field{}", NameCounter++);
      }
    };

  private:
    IO::IStreamWriter &_stream;
    rapidxml::xml_document<> _document;
    Stack<NodeMetadata> _nodes {};
    string _nextFieldName {};

  public:
    XmlArchiveWriter(IO::IStreamWriter &stream) noexcept : _stream(stream)
    {
      // rapidxml will delete all allocations when xml_document is cleared
      auto *node = _document.allocate_node(rapidxml::node_declaration);
      node->append_attribute(_document.allocate_attribute("version", "1.0"));
      node->append_attribute(_document.allocate_attribute("encoding", "utf-8"));
      _document.append_node(node);

      // allocate root node
      auto *root = _document.allocate_node(rapidxml::node_element, "Krystal");
      _document.append_node(root);
      _nodes.emplace(root);
    }

    ~XmlArchiveWriter() noexcept override
    {
      struct StreamOutIt
      {
        IO::IStreamWriter *s;
        using iterator_category = std::output_iterator_tag;
        StreamOutIt &operator=(char c)
        {
          s->Write(reinterpret_cast<const byte *>(&c), 1);
          return *this;
        }
        StreamOutIt &operator*()
        {
          return *this;
        }
        StreamOutIt &operator++()
        {
          return *this;
        }
        StreamOutIt operator++(int)
        {
          return *this;
        }
      };
      rapidxml::print(StreamOutIt {&_stream}, _document, 0);
      _document.clear();
    }

    void SetNextFieldName(const string &name) noexcept
    {
      _nextFieldName = name;
    }

    void SetCurrentNodeVersion(const Version &version) noexcept
    {
      // Allocate both name and value in the document's memory pool
      const auto verStr = std::to_string(version.Value);
      char *name = _document.allocate_string(Version::FieldName); // or "k_version"
      char *value = _document.allocate_string(verStr.c_str());

      _nodes.top().Node->append_attribute(_document.allocate_attribute(name, value));
    }

    template <ArchiveBuiltin T>
    XmlArchiveWriter &Write(const T &value)
    {
      string data {};
      if constexpr (SameType<T, bool>)
      {
        data = value ? "true" : "false";
      }
      else if constexpr (OneOf<T, int64, uint64, ulong, long, long long, unsigned long long>)
      {
        data = std::to_string(value);
      }
      else if constexpr (OneOf<T, float, double, long double>)
      {
        Array<char, 128> buffer {};
        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value,
                                       std::chars_format::general, std::numeric_limits<T>::max_digits10);
        if (ec != std::errc {})
        {
          throw std::runtime_error("Invalid numeric in XML");
        }
        data = string {buffer.data(), ptr};
      }
      else if constexpr (SameType<T, string>)
      {
        data = value;
      }
      else if constexpr (OneOf<T, byte, uchar, ushort, uint8, uint16, uint32>)
      {
        data = std::to_string(static_cast<uint32>(value));
      }
      else if constexpr (OneOf<T, char, short, int8, int16, int32>)
      {
        data = std::to_string(static_cast<int32>(value));
      }

      // If the first or last character is a whitespace, add xml:space attribute
      const auto len = data.length();
      if (len > 0 && (IsWhitespace(data[0]) || IsWhitespace(data[len - 1])))
      {
        _nodes.top().Node->append_attribute(_document.allocate_attribute("xml:space", "preserve"));
      }

      // allocate strings for all of the data in the XML object
      auto dataPtr = _document.allocate_string(data.c_str(), data.length() + 1);

      // insert into the XML
      _nodes.top().Node->append_node(_document.allocate_node(rapidxml::node_data, nullptr, dataPtr));

      return *this;
    }

    void StartNode()
    {
      // allocate strings for all of the data in the XML object
      const string name = _nextFieldName.empty() ? _nodes.top().GetNextFieldName() : _nextFieldName;
      auto *namePtr = _document.allocate_string(name.data(), name.length() + 1);
      _nextFieldName.clear();

      // insert into the XML
      auto node = _document.allocate_node(rapidxml::node_element, namePtr, nullptr, name.size());
      _nodes.top().Node->append_node(node);
      _nodes.emplace(node);
    }

    void FinishNode()
    {
      _nodes.pop();
    }

  private:
    NO_DISCARD bool IsWhitespace(char c) noexcept
    {
      return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }
  };

  class XmlArchiveReader : public BaseArchiveReader<XmlArchiveReader>
  {
    NO_COPY_MOVE(XmlArchiveReader)

    using NodeType = rapidxml::xml_node<>;

    struct NodeMetadata
    {
      NodeType *Node;   /// @brief A pointer to this node
      NodeType *Child;  /// @brief A pointer to its current child
      size_t Size {0u}; /// @brief The remaining number of children for this node
      string Name {};   /// @brief The NVP name for next child node

      NodeMetadata(NodeType *node) noexcept
          : Node(node), Child(node->first_node()), Size(GetNumberOfChildren(node))
      {
      }

      void Advance() noexcept
      {
        if (Size > 0)
        {
          --Size;
          Child = Child->next_sibling();
        }
      }

      NodeType *Search(const string &searchName) noexcept
      {
        if (searchName.empty())
        {
          return nullptr;
        }

        using namespace rapidxml::internal;
        size_t size = GetNumberOfChildren(Node);
        const size_t len = measure(searchName.c_str());

        for (auto child = Node->first_node(); child != nullptr; child = child->next_sibling())
        {
          if (compare(child->name(), child->name_size(), searchName.c_str(), len, true))
          {
            Size = size;
            Child = child;

            return child;
          }
          --size;
        }

        return nullptr;
      }

      string GetNextChildName() const noexcept
      {
        return Child ? Child->name() : "";
      }
    };

  private:
    IO::IStreamReader &_stream;
    List<char> _data;
    rapidxml::xml_document<> _document;
    Stack<NodeMetadata> _nodes {};

  public:
    XmlArchiveReader(IO::IStreamReader &stream) : _stream(stream)
    {
      // Read the entire stream into memory
      const auto size = static_cast<size_t>(_stream.Size());
      _data.resize(size + 1); // +1 for null terminator
      _stream.Read(reinterpret_cast<byte *>(_data.data()), size);
      _data[size] = '\0'; // Null-terminate the data

      // Parse the XML data
      _document.parse<0>(_data.data());
      // Move to the root node
      auto *root = _document.first_node("Krystal");
      if (root == nullptr)
      {
        throw std::exception("Parsing failed: No root 'Krystal' node found.");
      }
      _nodes.emplace(root);
    }

    ~XmlArchiveReader() noexcept override
    {
      _document.clear();
    }

    void SetNextFieldName(const string &name) noexcept
    {
      _nodes.top().Name = name;
    }

    void GetCurrentNodeVersion(Version &version) noexcept
    {
      if (auto *attr = _nodes.top().Node->first_attribute(Version::FieldName))
      {
        int32 ver = 0;
        auto [_, ec] = std::from_chars(attr->value(), attr->value() + attr->value_size(), ver);
        if (ec == std::errc {})
        {
          version.Value = static_cast<uint32>(ver);
        }
      }
    }

    template <ArchiveBuiltin T>
    XmlArchiveReader &Read(T &value)
    {
      NodeType *node = _nodes.top().Node;
      if constexpr (SameType<T, bool>)
      {
        stringview text(_nodes.top().Node->value(), _nodes.top().Node->value_size());
        value = (text == "true" || text == "1");
      }
      else if constexpr (OneOf<T, int32, int64, uint64, char, short, ulong, long, long long,
                               unsigned long long, float, double, long double>)
      {
        auto [_, ec] = std::from_chars(node->value(), node->value() + node->value_size(), value);
        if (ec != std::errc {})
        {
          throw std::runtime_error("Invalid numeric in XML");
        }
      }
      else if constexpr (OneOf<T, byte, uchar, ushort, uint8, uint16, uint32>)
      {
        uint32 temp = 0;
        auto [_, ec] = std::from_chars(node->value(), node->value() + node->value_size(), temp);
        if (ec != std::errc {})
        {
          throw std::runtime_error("Invalid numeric in XML");
        }
        value = static_cast<T>(temp);
      }
      else if constexpr (OneOf<T, char, short, int8, int16, int32>)
      {
        int32 temp = 0;
        auto [_, ec] = std::from_chars(node->value(), node->value() + node->value_size(), temp);
        if (ec != std::errc {})
        {
          throw std::runtime_error("Invalid numeric in XML");
        }
        value = static_cast<T>(temp);
      }
      else if constexpr (SameType<T, string>)
      {
        value.assign(node->value(), node->value_size());
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
      Size size = GetNumberOfChildren(_nodes.top().Node);
      value.Size = size;
    }

    void StartNode()
    {
      auto next = _nodes.top().Child;                // By default we would move to the next child node
      const string expectedName = _nodes.top().Name; // expected name from the named field, if provided

      // If we were given an NVP name, look for it in the current level of the document.
      //    We only need to do this if either we have exhausted the siblings of the current level or
      //    the NVP name does not match the name of the node we would normally read next
      if (!expectedName.empty() && (next == nullptr || std::strcmp(next->name(), expectedName.c_str()) != 0))
      {
        next = _nodes.top().Search(expectedName);

        if (next == nullptr)
          throw std::exception(
            ("XML Parsing failed - provided NVP (" + std::string(expectedName) + ") not found").c_str());
      }

      if (next != nullptr)
      {
        _nodes.emplace(next);
      }
      else
      {
        throw std::exception("XML Parsing failed - no more nodes to read.");
      }
    }

    void FinishNode()
    {
      // remove current, advance parent and clear its name
      _nodes.pop();
      _nodes.top().Advance();
      _nodes.top().Name.clear();
    }

    string GetNodeName() const noexcept
    {
      return _nodes.top().GetNextChildName();
    }

    NO_DISCARD static size_t GetNumberOfChildren(rapidxml::xml_node<> *node)
    {
      size_t size = 0;
      node = node->first_node(); // get first child

      while (node != nullptr)
      {
        size++;
        node = node->next_sibling();
      }

      return size;
    }
  };

  template <>
  struct ArchiveTraits<XmlArchiveWriter>
  {
    static constexpr bool IsWriter = true;
    static constexpr bool IsReader = false;
    static constexpr bool IsBinary = false;
    static constexpr bool IsText = true;
  };

  template <>
  struct ArchiveTraits<XmlArchiveReader>
  {
    static constexpr bool IsWriter = false;
    static constexpr bool IsReader = true;
    static constexpr bool IsBinary = false;
    static constexpr bool IsText = true;
  };

  template <typename T>
  concept IsXmlArchive = OneOf<T, XmlArchiveWriter, XmlArchiveReader>;

  template <typename T>
  void Transfer(XmlArchiveReader &archive, ContainerSize<T> &value) noexcept
  {
    archive.Read(value); // we get this value by counting the number of children in the current node.
  }

  template <typename T>
  requires(!ArchiveNamedField<T> && !ArchiveContainerSize<T> && !ArchiveVersion<T>)
  void BeforeTransfer(XmlArchiveWriter &archive, const T &) noexcept
  {
    archive.StartNode();
  }

  template <typename T>
  requires(!ArchiveNamedField<T> && !ArchiveContainerSize<T> && !ArchiveVersion<T>)
  void AfterTransfer(XmlArchiveWriter &archive, const T &) noexcept
  {
    archive.FinishNode();
  }

  template <typename T>
  requires(!ArchiveNamedField<T> && !ArchiveContainerSize<T> && !ArchiveVersion<T>)
  void BeforeTransfer(XmlArchiveReader &archive, const T &) noexcept
  {
    archive.StartNode();
  }

  template <typename T>
  requires(!ArchiveNamedField<T> && !ArchiveContainerSize<T> && !ArchiveVersion<T>)
  void AfterTransfer(XmlArchiveReader &archive, const T &) noexcept
  {
    archive.FinishNode();
  }

  template <IsXmlArchive Archive>
  void Transfer(Archive &archive, Version &version) noexcept
  {
    if constexpr (IsArchiveWriter<Archive>)
    {
      archive.SetCurrentNodeVersion(version);
    }
    else
    {
      archive.GetCurrentNodeVersion(version);
    }
  }
}