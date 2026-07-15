#pragma once

#include "Krystal.HTML/DOM/Dicts/GetRootNodeOptions.hpp"
#include "Krystal.HTML/DOM/Enums/DocumentPosition.hpp"
#include "Krystal.HTML/DOM/Enums/NodeFlags.hpp"
#include "Krystal.HTML/DOM/Enums/NodeType.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"
#include "Krystal.HTML/DOM/Internals/RareData/NodeRareData.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Mixins/CanMakeCheckedPtr.hpp"
#include "Krystal.Lib/Pointers/CheckedPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Document;
  class Element;
  class NodeList;

  /// @see https://dom.spec.whatwg.org/#interface-node
  class Node : public EventTarget, public CanMakeCheckedPtr<Node>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Node);

    friend class Document;
    friend class DocumentAlgorithms;
    friend class ElementAlgorithms;
    friend class ExtensibilityHooks;
    friend class MutationAlgorithms;
    friend class MutationObserver;
    friend class MutationObserverAlgorithms;
    friend class TreeQueries;

  private:
    NodeFlags _flags : BitCount<NodeFlags>() {NodeFlags::None};
    NodeType _nodeType : BitCount<NodeType>() {NodeType::NONE};
    RefPtr<Document> _nodeDocument;
    CheckedPtr<ContainerNode> _parentNode;
    CheckedPtr<Node> _previousSibling;
    CheckedPtr<Node> _nextSibling;
    UniquePtr<NodeRareData> _nodeRareData;

  protected:
    Node(Document &document, NodeType type, NodeFlags flags) noexcept;

  public:
    virtual ~Node() noexcept = default;

#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodetype
    KRYS_NODISCARD NodeType NodeType() const noexcept
    {
      return _nodeType;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD virtual DOMString NodeName() const noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-node-baseuri
    KRYS_NODISCARD DOMString BaseURI() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-isconnected
    KRYS_NODISCARD bool IsConnected() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsConnected);
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-ownerdocument
    KRYS_NODISCARD RawPtr<Document> OwnerDocument() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-getrootnode
    KRYS_NODISCARD Node &GetRootNode(const GetRootNodeOptions &options) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-parentnode
    KRYS_NODISCARD RawPtr<ContainerNode> ParentNode() const noexcept
    {
      return _parentNode.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-parentelement
    KRYS_NODISCARD RawPtr<Element> ParentElement() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-haschildnodes
    KRYS_NODISCARD bool HasChildNodes() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-childnodes
    KRYS_NODISCARD Ref<NodeList> ChildNodes() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-firstchild
    KRYS_NODISCARD RawPtr<Node> FirstChild() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-lastchild
    KRYS_NODISCARD RawPtr<Node> LastChild() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-previoussibling
    KRYS_NODISCARD RawPtr<Node> PreviousSibling() const noexcept
    {
      return _previousSibling.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-nextsibling
    KRYS_NODISCARD RawPtr<Node> NextSibling() const noexcept
    {
      return _nextSibling.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-nodevalue
    KRYS_NODISCARD virtual Maybe<DOMString> NodeValue() const noexcept
    {
      return Null;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-nodevalue
    virtual ExceptionOr<void> NodeValue(DOMString &&value) noexcept
    {
      (void)value; // still want it in the signature
      return {};
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    KRYS_NODISCARD virtual Maybe<DOMString> TextContent() const noexcept
    {
      return Null;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    virtual ExceptionOr<void> TextContent(DOMString &&value) noexcept
    {
      (void)value; // still want it in the signature
      return {};
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-normalize
    ExceptionOr<void> Normalize() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-clonenode
    KRYS_NODISCARD ExceptionOr<Ref<Node>> CloneNode(bool subtree = false) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-isequalnode
    KRYS_NODISCARD bool IsEqualNode(RawPtr<const Node> otherNode) const noexcept;

    /// @note legacy alias of ===
    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool IsSameNode(RawPtr<const Node> otherNode) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool operator==(const Node &otherNode) const noexcept
    {
      return this == &otherNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool operator==(RawPtr<const Node> otherNode) const noexcept
    {
      return this == otherNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool operator!=(const Node &otherNode) const noexcept
    {
      return this != &otherNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool operator!=(RawPtr<const Node> otherNode) const noexcept
    {
      return this != otherNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-comparedocumentposition
    KRYS_NODISCARD DocumentPosition CompareDocumentPosition(const Node &other) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-contains
    KRYS_NODISCARD bool Contains(RawPtr<const Node> other) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-lookupprefix
    KRYS_NODISCARD DOMStringAtom LookupPrefix(DOMStringAtom namespaceURI) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-lookupnamespaceuri
    KRYS_NODISCARD DOMStringAtom LookupNamespaceURI(DOMStringAtom prefix) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-isdefaultnamespace
    KRYS_NODISCARD bool IsDefaultNamespace(DOMStringAtom namespaceURI) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-insertbefore
    ExceptionOr<Node &> InsertBefore(Node &newChild, RawPtr<Node> refChild) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-replacechild
    ExceptionOr<Node &> ReplaceChild(Node &newChild, Node &oldChild) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-removechild
    ExceptionOr<Node &> RemoveChild(Node &child) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-appendchild
    ExceptionOr<Node &> AppendChild(Node &newChild) noexcept;

#pragma endregion

    /// @brief Helper function to get the node document of a node. This is the same as OwnerDocument except it
    /// always returns the document, even for documents themselves.
    /// @see https://dom.spec.whatwg.org/#concept-node-document
    KRYS_NODISCARD Document &NodeDocument() const noexcept
    {
      return *_nodeDocument;
    }

  protected:
    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD RawPtr<EventTarget> GetParent(Event &event) const noexcept override;

#pragma region Tree Scope

    /// @see https://dom.spec.whatwg.org/#concept-shadow-tree
    KRYS_NODISCARD bool IsInShadowTree() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsInShadowTree);
    }

    /// @see https://dom.spec.whatwg.org/#concept-document-tree
    KRYS_NODISCARD bool IsInDocumentTree() const noexcept
    {
      return IsConnected() && !IsInShadowTree();
    }

#pragma endregion

#pragma region Registered Observers

    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    KRYS_NODISCARD List<Ref<RegisteredObserver>> &RegisteredObservers() noexcept;

    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    KRYS_NODISCARD List<Ref<TransientRegisteredObserver>> &TransientRegisteredObservers() noexcept;

#pragma endregion

#pragma region Extension Hooks

    virtual void OnInsert() noexcept
    {
    }

    virtual void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
    {
      (void)isSubtreeRoot;
      (void)oldAncestor;
    }

    virtual void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
    {
      (void)isSubtreeRoot;
      (void)oldAncestor;
    }

    virtual void OnClone(Node &copy, bool subtree) noexcept
    {
      (void)copy;
      (void)subtree;
    }

    virtual void OnAdopt(Document &oldDocument) noexcept
    {
      (void)oldDocument;
    }

    virtual void OnPostConnection() noexcept
    {
    }

#pragma endregion

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
      return HasNodeFlag(NodeFlags::IsContainerNode);
    }

    KRYS_NODISCARD bool IsElementNode() const noexcept
    {
      return _nodeType == NodeType::ELEMENT_NODE;
    }

    KRYS_NODISCARD bool IsCharacterDataNode() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsCharacterData);
    }

    KRYS_NODISCARD bool IsTextNode() const noexcept
    {
      return _nodeType == NodeType::TEXT_NODE || _nodeType == NodeType::CDATA_SECTION_NODE;
    }

    KRYS_NODISCARD bool IsCDATASectionNode() const noexcept
    {
      return _nodeType == NodeType::CDATA_SECTION_NODE;
    }

    KRYS_NODISCARD bool IsProcessingInstructionNode() const noexcept
    {
      return _nodeType == NodeType::PROCESSING_INSTRUCTION_NODE;
    }

    KRYS_NODISCARD bool IsCommentNode() const noexcept
    {
      return _nodeType == NodeType::COMMENT_NODE;
    }

    KRYS_NODISCARD bool IsShadowRootNode() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsShadowRoot);
    }

    KRYS_NODISCARD bool IsUnknownElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsUnknownElement);
    }

    KRYS_NODISCARD bool IsHTMLElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsHTMLElement);
    }

    KRYS_NODISCARD bool IsHTMLSlotElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsHTMLSlotElement);
    }

    KRYS_NODISCARD bool IsSVGElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsSVGElement);
    }

    KRYS_NODISCARD bool IsMathMLElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsMathMLElement);
    }

    KRYS_NODISCARD bool IsHTMLUnknownElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsHTMLElement) && HasNodeFlag(NodeFlags::IsUnknownElement);
    }

    KRYS_NODISCARD bool IsSVGUnknownElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsSVGElement) && HasNodeFlag(NodeFlags::IsUnknownElement);
    }

    KRYS_NODISCARD bool IsMathMLUnknownElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsMathMLElement) && HasNodeFlag(NodeFlags::IsUnknownElement);
    }

#pragma endregion

#pragma region Relationships

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetParentNode(RawPtr<ContainerNode> parent) noexcept
    {
      _parentNode = ShareCheckedPtr(parent);
    }

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetNextSibling(RawPtr<Node> sibling) noexcept
    {
      _nextSibling = ShareCheckedPtr(sibling);
    }

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetPreviousSibling(RawPtr<Node> sibling) noexcept
    {
      _previousSibling = ShareCheckedPtr(sibling);
    }

#pragma endregion

#pragma region Node Flags

    void SetNodeFlag(NodeFlags flag) noexcept
    {
      _flags = _flags | flag;
    }

    void ClearNodeFlag(NodeFlags flag) noexcept
    {
      _flags = _flags & ~flag;
    }

    KRYS_NODISCARD bool HasNodeFlag(NodeFlags flag) const noexcept
    {
      return HasFlag(_flags, flag);
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Node)
  static bool IsType(const Krys::HTML::EventTarget &target) noexcept
  {
    return target.IsNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();