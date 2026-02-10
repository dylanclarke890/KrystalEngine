#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Node/NodeMutationContexts.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CheckedPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  enum class NodeType : uint8
  {
    NONE = 0,
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

  enum class NodeFlag : uint16
  {
    None = 0,
    IsCharacterData = 1 << 0,
    IsContainerNode = 1 << 1,
    IsTextNode = 1 << 2,
    IsElement = 1 << 3,
    IsHTMLElement = 1 << 4,
    IsShadowRoot = 1 << 5,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::NodeType, 13u)
KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::DocumentPosition, 7u)
KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::NodeFlag, 7u)

namespace Krys::HTML
{
  class ContainerNode;
  class Document;
  class Element;
  class NodeList;
  class NodeMutationNotifier;
  class ShadowRoot;
  class TreeScope;

  /// @see https://dom.spec.whatwg.org/#dictdef-getrootnodeoptions
  struct GetRootNodeOptions
  {
    bool Composed = false;
  };

  struct URL
  {
    // TODO(IMPL): Implement URL class.
    DOMString Href;
  };

  class Node : public EventTarget, public CanMakeCheckedPtr<Node>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Node);

    friend class Document;
    friend class NodeMutationNotifier;

  private:
    NodeFlag _flags : BitCount<NodeFlag>() {NodeFlag::None};
    NodeType _nodeType;
    RefPtr<Document> _ownerDocument;
    RawPtr<ContainerNode> _parentNode;
    RawPtr<Node> _previousSibling;
    RawPtr<Node> _nextSibling;
    RawPtr<TreeScope> _treeScope;

  public:
    virtual ~Node() noexcept = default;

    void AddRef() const noexcept;
    void SubRef() const noexcept;

  protected:
    void AddRef(EventTargetTag) const noexcept final
    {
      AddRef();
    }

    void SubRef(EventTargetTag) const noexcept final
    {
      SubRef();
    }

  public:
    KRYS_NODISCARD virtual utf8_string NodeName() const noexcept = 0;

    KRYS_NODISCARD NodeType GetNodeType() const noexcept
    {
      return _nodeType;
    }

    KRYS_NODISCARD URL BaseURI() const noexcept
    {
      return {u8"about:blank"};
    }

    /// @see https://dom.spec.whatwg.org/#connected
    KRYS_NODISCARD bool IsConnected() const noexcept
    {
      return HasEventTargetFlag(EventTargetFlag::IsConnected);
    }

    KRYS_NODISCARD RawPtr<Document> OwnerDocument() const noexcept
    {
      return _ownerDocument.get();
    }

    KRYS_NODISCARD Node &GetRootNode(const GetRootNodeOptions &options) noexcept;
    KRYS_NODISCARD const Node &GetRootNode(const GetRootNodeOptions &options) const noexcept;

    KRYS_NODISCARD Node &Root() noexcept;
    KRYS_NODISCARD const Node &Root() const noexcept;

    KRYS_NODISCARD Node &ShadowIncludingRoot() noexcept;
    KRYS_NODISCARD const Node &ShadowIncludingRoot() const noexcept;

    KRYS_NODISCARD RawPtr<ContainerNode> ParentNode() const noexcept
    {
      return _parentNode;
    }
    KRYS_NODISCARD RawPtr<Element> ParentElement() const noexcept;
    KRYS_NODISCARD bool HasChildNodes() const noexcept;
    KRYS_NODISCARD RefPtr<NodeList> ChildNodes() noexcept;
    KRYS_NODISCARD RawPtr<Node> FirstChild() const noexcept;
    KRYS_NODISCARD RawPtr<Node> LastChild() const noexcept;
    KRYS_NODISCARD RawPtr<Node> PreviousSibling() const noexcept
    {
      return _previousSibling;
    }
    KRYS_NODISCARD RawPtr<Node> NextSibling() const noexcept
    {
      return _nextSibling;
    }

    KRYS_NODISCARD virtual DOMString NodeValue() const noexcept;
    KRYS_NODISCARD virtual ExceptionOr<void> SetNodeValue(DOMStringView value) noexcept;
    KRYS_NODISCARD DOMString TextContent(bool convertBRsToNewlines = false) const noexcept;
    KRYS_NODISCARD ExceptionOr<void> SetTextContent(DOMString &&text) noexcept;
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

    KRYS_NODISCARD size_t Length() const noexcept;

    KRYS_NODISCARD size_t CountChildNodes() const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-tree
    KRYS_NODISCARD bool IsInShadowTree() const noexcept
    {
      return HasEventTargetFlag(EventTargetFlag::IsInShadowTree);
    }

    /// @see https://dom.spec.whatwg.org/#concept-document-tree
    KRYS_NODISCARD bool IsInDocumentTree() const noexcept
    {
      return IsConnected() && !IsInShadowTree();
    }

    KRYS_NODISCARD bool IsInTreeScope() const noexcept
    {
      return IsConnected() || IsInShadowTree();
    }

    void SetTreeScope(TreeScope &treeScope) noexcept
    {
      _treeScope = &treeScope;
    }

    KRYS_NODISCARD TreeScope &GetTreeScope() const noexcept
    {
      assert(_treeScope != nullptr);
      return *_treeScope;
    }

    KRYS_NODISCARD RawPtr<ShadowRoot> GetShadowRoot() const noexcept;

#pragma region Type Checks

    KRYS_NODISCARD bool IsAttributeNode() const noexcept
    {
      return _nodeType == NodeType::ATTRIBUTE_NODE;
    }

    KRYS_NODISCARD bool IsDocumentNode() const noexcept
    {
      return _nodeType == NodeType::DOCUMENT_NODE;
    }

    KRYS_NODISCARD bool IsDocumentTypeNode() const noexcept
    {
      return _nodeType == NodeType::DOCUMENT_TYPE_NODE;
    }

    KRYS_NODISCARD bool IsDocumentFragmentNode() const noexcept
    {
      return _nodeType == NodeType::DOCUMENT_FRAGMENT_NODE;
    }

    KRYS_NODISCARD bool IsContainerNode() const noexcept
    {
      return HasNodeFlag(NodeFlag::IsContainerNode);
    }

    KRYS_NODISCARD bool IsElementNode() const noexcept
    {
      return HasNodeFlag(NodeFlag::IsElement);
    }

    KRYS_NODISCARD bool IsHTMLElementNode() const noexcept
    {
      return HasNodeFlag(NodeFlag::IsHTMLElement);
    }

    KRYS_NODISCARD bool IsCharacterDataNode() const noexcept
    {
      return HasNodeFlag(NodeFlag::IsCharacterData);
    }

    KRYS_NODISCARD bool IsTextNode() const noexcept
    {
      return HasNodeFlag(NodeFlag::IsTextNode);
    }

    KRYS_NODISCARD bool IsShadowRootNode() const noexcept
    {
      return HasNodeFlag(NodeFlag::IsShadowRoot);
    }

    KRYS_NODISCARD bool IsTreeScope() const noexcept
    {
      return IsDocumentNode() || IsShadowRootNode();
    }

#pragma endregion

  protected:
    Node(Document &document, NodeType type, NodeFlag flags) noexcept;

    KRYS_NODISCARD bool HasNodeFlag(NodeFlag flag) const noexcept
    {
      return HasFlag(_flags, flag);
    }

    void SetNodeFlag(NodeFlag flag) noexcept
    {
      _flags = _flags | flag;
    }

    void ClearNodeFlag(NodeFlag flag) noexcept
    {
      _flags = _flags & ~flag;
    }

    virtual void InsertedIntoAncestor(const NodeInsertedContext &context) noexcept;
    virtual void RemovedFromAncestor(const NodeRemovedContext &context) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Node)
  static bool IsType(const Krys::HTML::EventTarget &target) noexcept
  {
    return target.IsNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()