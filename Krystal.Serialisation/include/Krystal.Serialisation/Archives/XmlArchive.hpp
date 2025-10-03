#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Serialisation/Archives/BaseArchive.hpp"
#include "Krystal.Serialisation/Builtins.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include "rapidxml/rapidxml.hpp"
namespace rapidxml
{
  namespace internal
  {
    template <class OutIt, class Ch>
    inline OutIt print_children(OutIt out, const xml_node<Ch> *node, int flags, int indent);

    template <class OutIt, class Ch>
    inline OutIt print_attributes(OutIt out, const xml_node<Ch> *node, int flags);

    template <class OutIt, class Ch>
    inline OutIt print_data_node(OutIt out, const xml_node<Ch> *node, int flags, int indent);

    template <class OutIt, class Ch>
    inline OutIt print_cdata_node(OutIt out, const xml_node<Ch> *node, int flags, int indent);

    template <class OutIt, class Ch>
    inline OutIt print_element_node(OutIt out, const xml_node<Ch> *node, int flags, int indent);

    template <class OutIt, class Ch>
    inline OutIt print_declaration_node(OutIt out, const xml_node<Ch> *node, int flags, int indent);

    template <class OutIt, class Ch>
    inline OutIt print_comment_node(OutIt out, const xml_node<Ch> *node, int flags, int indent);

    template <class OutIt, class Ch>
    inline OutIt print_doctype_node(OutIt out, const xml_node<Ch> *node, int flags, int indent);

    template <class OutIt, class Ch>
    inline OutIt print_pi_node(OutIt out, const xml_node<Ch> *node, int flags, int indent);
  }
}
#include "rapidxml/rapidxml_print.hpp"
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

    ~XmlArchiveWriter() noexcept
    {
      std::string output;
      const int flags = 0;
      rapidxml::print(std::back_inserter(output), _document, flags);

      _document.clear();
      _stream.Write(reinterpret_cast<const byte *>(output.data()), output.size());
    }

    template <ArchiveBuiltin T>
    XmlArchiveWriter &Write(const T &value) noexcept
    {
      if constexpr (SameType<T, byte>)
      {
        Write(static_cast<uint32>(value));
      }
      else
      {
        std::ostringstream oss;
        oss << std::boolalpha;
        oss.precision(std::numeric_limits<double>::digits10);

        oss << value;
        auto asString = oss.str();

        // If the first or last character is a whitespace, add xml:space attribute
        const auto len = asString.length();
        if (len > 0 && (IsWhitespace(asString[0]) || IsWhitespace(asString[len - 1])))
        {
          _nodes.top().Node->append_attribute(_document.allocate_attribute("xml:space", "preserve"));
        }

        // allocate strings for all of the data in the XML object
        auto dataPtr = _document.allocate_string(asString.c_str(), asString.length() + 1);

        // insert into the XML
        _nodes.top().Node->append_node(_document.allocate_node(rapidxml::node_data, nullptr, dataPtr));
      }

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

    void SetNextName(const string &name)
    {
      _nextFieldName = name;
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

    template <ArchiveBuiltin T>
    XmlArchiveReader &Read(T &value)
    {
      if constexpr (SameType<T, bool>)
      {
        std::istringstream is(_nodes.top().Node->value());
        is.setf(std::ios::boolalpha);
        is >> value;
      }
      else if constexpr (SameType<T, int>)
      {
        value = std::stoi(_nodes.top().Node->value());
      }
      else if constexpr (SameType<T, uint>)
      {
        value = static_cast<uint>(std::stoul(_nodes.top().Node->value()));
      }
      else if constexpr (SameType<T, int64>)
      {
        value = std::stoll(_nodes.top().Node->value());
      }
      else if constexpr (SameType<T, uint64>)
      {
        value = static_cast<uint64>(std::stoull(_nodes.top().Node->value()));
      }
      else if constexpr (SameType<T, float>)
      {
        try
        {
          value = std::stof(_nodes.top().Node->value());
        }
        catch (std::out_of_range const &)
        {
          // special case for denormalized values
          std::istringstream is(_nodes.top().Node->value());
          is >> value;
          if (std::fpclassify(value) != FP_SUBNORMAL)
            throw;
        }
      }
      else if constexpr (SameType<T, double>)
      {
        try
        {
          value = std::stod(_nodes.top().Node->value());
        }
        catch (std::out_of_range const &)
        {
          // special case for denormalized values
          std::istringstream is(_nodes.top().Node->value());
          is >> value;
          if (std::fpclassify(value) != FP_SUBNORMAL)
            throw;
        }
      }
      else if constexpr (SameType<T, byte>)
      {
        value = static_cast<byte>(std::stoul(_nodes.top().Node->value()));
      }
      else if constexpr (SameType<T, string>)
      {
        std::istringstream is(_nodes.top().Node->value());
        using Iter = std::istreambuf_iterator<char>;
        value.assign(Iter(is), Iter());
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

      _nodes.emplace(next);
    }

    void FinishNode()
    {
      // remove current
      _nodes.pop();

      // advance parent
      _nodes.top().Advance();

      // Reset name
      _nodes.top().Name.clear();
    }

    string GetNodeName() const noexcept
    {
      return _nodes.top().GetNextChildName();
    }

    void SetNextName(const string &name)
    {
      _nodes.top().Name = name;
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

  template <typename T>
  void Transfer(XmlArchiveWriter &, ContainerSize<T> &) noexcept
  {
    // No-op
  }

  template <typename T>
  void Transfer(XmlArchiveReader &archive, ContainerSize<T> &value) noexcept
  {
    archive.Read(value); // we get this value by counting the number of children in the current node.
  }

  template <typename T>
  void BeforeTransfer(XmlArchiveWriter &archive, const T &value) noexcept
  {
    if constexpr (ArchiveNamedField<T>)
    {
      archive.SetNextName(value.Name);
    }
    else if constexpr (!ArchiveContainerSize<T>)
    {
      archive.StartNode();
    }
  }

  template <typename T>
  void AfterTransfer(XmlArchiveWriter &archive, const T &) noexcept
  {
    if constexpr (!ArchiveNamedField<T> && !ArchiveContainerSize<T>)
    {
      archive.FinishNode(); // exits {} or [].
    }
  }

  template <typename T>
  void BeforeTransfer(XmlArchiveReader &archive, const T &value) noexcept
  {
    if constexpr (ArchiveNamedField<T>)
    {
      archive.SetNextName(value.Name);
    }
    else if constexpr (!ArchiveContainerSize<T>)
    {
      archive.StartNode(); // enter {} or [].
    }
  }

  template <typename T>
  void AfterTransfer(XmlArchiveReader &archive, const T &) noexcept
  {
    if constexpr (!ArchiveNamedField<T> && !ArchiveContainerSize<T>)
    {
      archive.FinishNode(); // exit {} or [].
    }
  }
}