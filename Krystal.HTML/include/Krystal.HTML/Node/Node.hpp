#pragma once

#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakRef.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  enum class NodeType : uint8
  {
    ELEMENT_NODE = 1,
    ATTRIBUTE_NODE = 2,
    TEXT_NODE = 3,
    CDATA_SECTION_NODE = 4,
    ENTITY_REFERENCE_NODE = 5, // legacy
    ENTITY_NODE = 6,           // legacy
    PROCESSING_INSTRUCTION_NODE = 7,
    COMMENT_NODE = 8,
    DOCUMENT_NODE = 9,
    DOCUMENT_TYPE_NODE = 10,
    DOCUMENT_FRAGMENT_NODE = 11,
    NOTATION_NODE = 12, // legacy
  };

  enum class DocumentPosition : uint8
  {
    DOCUMENT_POSITION_EQUIVALENT = 0x00,
    DOCUMENT_POSITION_DISCONNECTED = 0x01,
    DOCUMENT_POSITION_PRECEDING = 0x02,
    DOCUMENT_POSITION_FOLLOWING = 0x04,
    DOCUMENT_POSITION_CONTAINS = 0x08,
    DOCUMENT_POSITION_CONTAINED_BY = 0x10,
    DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20,
  };

  enum class NodeFlag : uint8
  {
    None = 0,
    IsContainerNode = 1 << 0,
    IsTextNode = 1 << 1,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::NodeType, 12u)
KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::DocumentPosition, 7u)
KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::NodeFlag, 3u)

namespace Krys::HTML
{
  class NodeList;
  class ContainerNode;
  class Document;

  /// @see https://dom.spec.whatwg.org/#dictdef-getrootnodeoptions
  struct GetRootNodeOptions
  {
    bool Composed = false;
  };

  struct URL
  {
  };

  class Node : public EventTarget
  {
    friend class Document;

  private:
    // @internal
    NodeFlag _flags : BitCount<NodeFlag>() {NodeFlag::None};
    NodeType _nodeType;
    RefPtr<Document> _ownerDocument;
    RawPtr<ContainerNode> _parentNode;
    RawPtr<Node> _previousSibling;
    RefPtr<Node> _nextSibling;

  public:
    virtual ~Node() = default;

    KRYS_NODISCARD virtual utf8_string NodeName() const noexcept = 0;

    KRYS_NODISCARD NodeType GetNodeType() const noexcept
    {
      return _nodeType;
    }

    KRYS_NODISCARD URL BaseURI() const noexcept
    {
      return {};
    }

    /// @see https://dom.spec.whatwg.org/#connected
    KRYS_NODISCARD bool IsConnected() const noexcept;

    KRYS_NODISCARD RawPtr<Document> OwnerDocument() const noexcept
    {
      return _ownerDocument.get();
    }
    KRYS_NODISCARD Node &GetRootNode(const GetRootNodeOptions &options) const noexcept;
    KRYS_NODISCARD RawPtr<ContainerNode> ParentNode() const noexcept
    {
      return _parentNode;
    }
    KRYS_NODISCARD RawPtr<Node> ParentElement() const noexcept;
    KRYS_NODISCARD bool HasChildNodes() const noexcept;
    KRYS_NODISCARD RefPtr<NodeList> ChildNodes() noexcept;
    KRYS_NODISCARD RawPtr<Node> FirstChild() const noexcept;
    KRYS_NODISCARD RawPtr<Node> LastChild() const noexcept;
    KRYS_NODISCARD RawPtr<Node> PreviousSibling() const noexcept;
    KRYS_NODISCARD RawPtr<Node> NextSibling() const noexcept;

    KRYS_NODISCARD virtual utf8_string NodeValue() const noexcept;
    KRYS_NODISCARD virtual ExceptionOr<void> SetNodeValue(utf8_stringview value) noexcept;
    KRYS_NODISCARD utf8_string TextContent(bool convertBRsToNewlines = false) const noexcept;
    KRYS_NODISCARD ExceptionOr<void> SetTextContent(utf8_string &&text) noexcept;
    KRYS_NODISCARD ExceptionOr<void> Normalize() noexcept;

    KRYS_NODISCARD Ref<Node> CloneNode(bool deep) const noexcept;
    KRYS_NODISCARD bool IsEqualNode(RawPtr<const Node> otherNode) const noexcept;
    KRYS_NODISCARD bool IsSameNode(RawPtr<const Node> otherNode) const noexcept; // legacy alias of ===

    KRYS_NODISCARD DocumentPosition CompareDocumentPosition(Node &other) const noexcept;
    KRYS_NODISCARD bool Contains(RawPtr<const Node> other) const noexcept;

    KRYS_NODISCARD const StringAtom &LookupPrefix(const StringAtom &namespaceURI) const noexcept;
    KRYS_NODISCARD const StringAtom &LookupNamespaceURI(const StringAtom &prefix) const noexcept;
    KRYS_NODISCARD bool IsDefaultNamespace(const StringAtom &namespaceURI) const noexcept;

    KRYS_NODISCARD ExceptionOr<void> InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept;
    KRYS_NODISCARD ExceptionOr<void> ReplaceChild(Node &newChild, Node &oldChild) noexcept;
    KRYS_NODISCARD ExceptionOr<void> RemoveChild(Node &child) noexcept;
    KRYS_NODISCARD ExceptionOr<void> AppendChild(Node &newChild) noexcept;

    KRYS_NODISCARD bool IsContainerNode() const noexcept
    {
      return HasFlag(_flags, NodeFlag::IsContainerNode);
    }
    KRYS_NODISCARD bool IsDocumentNode() const noexcept
    {
      return _nodeType == NodeType::DOCUMENT_NODE;
    }
    KRYS_NODISCARD bool IsDocumentFragmentNode() const noexcept
    {
      return _nodeType == NodeType::DOCUMENT_FRAGMENT_NODE;
    }
    KRYS_NODISCARD bool IsElementNode() const noexcept
    {
      return _nodeType == NodeType::ELEMENT_NODE;
    }
    KRYS_NODISCARD bool IsTextNode() const noexcept
    {
      return HasFlag(_flags, NodeFlag::IsTextNode);
    }

  protected:
    Node(Document &document, NodeType type, NodeFlag flags) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Node)
  static bool IsType(const Krys::HTML::EventTarget &target) noexcept
  {
    return target.IsNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()