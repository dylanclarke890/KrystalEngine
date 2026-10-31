#pragma once

#include "Krystal.Booey/DOM/Dicts/GetRootNodeOptions.hpp"
#include "Krystal.Booey/DOM/Enums/DocumentPosition.hpp"
#include "Krystal.Booey/DOM/Enums/NodeFlags.hpp"
#include "Krystal.Booey/DOM/Enums/NodeStyleFlags.hpp"
#include "Krystal.Booey/DOM/Enums/NodeType.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/NodeRareData.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/Validity.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <compare>

namespace krys::boo
{
  class RenderBox;
  class RenderObject;
  class RenderStyle;
  enum class TextDirection : bool;
}

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class DocumentOrShadowRoot;
  class Element;
  class NodeList;

  /// @see https://dom.spec.whatwg.org/#interface-node
  /// @see https://dom.spec.whatwg.org/#concept-node
  /// @see https://dom.spec.whatwg.org/#concept-node-tree
  class Node : public EventTarget, public CanMakeCheckedPtr<Node>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Node);

    friend class ContainerNode;
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
    style::Validity _styleValidity : BitCount<style::Validity>() {style::Validity::Valid};
    NodeStyleFlags _styleFlags : BitCount<NodeStyleFlags>() {NodeStyleFlags::None};
    /// @brief Not owned: the document counts the nodes that refer to it (`IncrementReferencingNodeCount`)
    /// and lives while any do. A document's is itself, which it doesn't count.
    Document *_nodeDocument;
    CheckedPtr<ContainerNode> _parentNode;
    CheckedPtr<Node> _previousSibling;
    CheckedPtr<Node> _nextSibling;
    UniquePtr<NodeRareData> _nodeRareData;
    RenderObject *_renderer {nullptr};

  protected:
    Node(Document &document, NodeType type, NodeFlags flags) noexcept;

    /// @brief Moves the node to @p document, which then counts it rather than its old document.
    void SetNodeDocument(Document &document) noexcept;

  public:
    virtual ~Node() noexcept;

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
    KRYS_NODISCARD Document *OwnerDocument() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-getrootnode
    KRYS_NODISCARD Node &GetRootNode(const GetRootNodeOptions &options) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-parent
    /// @see https://dom.spec.whatwg.org/#dom-node-parentnode
    KRYS_NODISCARD ContainerNode *ParentNode() const noexcept
    {
      return _parentNode.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-parentelement
    KRYS_NODISCARD Element *ParentElement() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-haschildnodes
    KRYS_NODISCARD bool HasChildNodes() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-childnodes
    KRYS_NODISCARD Ref<NodeList> ChildNodes() noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-first-child
    /// @see https://dom.spec.whatwg.org/#dom-node-firstchild
    KRYS_NODISCARD Node *FirstChild() const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-last-child
    /// @see https://dom.spec.whatwg.org/#dom-node-lastchild
    KRYS_NODISCARD Node *LastChild() const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-previous-sibling
    /// @see https://dom.spec.whatwg.org/#dom-node-previoussibling
    KRYS_NODISCARD Node *PreviousSibling() const noexcept
    {
      return _previousSibling.get();
    }

    /// @see https://dom.spec.whatwg.org/#concept-tree-next-sibling
    /// @see https://dom.spec.whatwg.org/#dom-node-nextsibling
    KRYS_NODISCARD Node *NextSibling() const noexcept
    {
      return _nextSibling.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-nodevalue
    KRYS_NODISCARD virtual Maybe<DOMString> NodeValue() const noexcept
    {
      return null;
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
      return null;
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
    KRYS_NODISCARD bool IsEqualNode(const Node *otherNode) const noexcept;

    /// @note legacy alias of ===
    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool IsSameNode(const Node *otherNode) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool operator==(const Node &otherNode) const noexcept
    {
      return this == &otherNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool operator==(const Node *otherNode) const noexcept
    {
      return this == otherNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool operator!=(const Node &otherNode) const noexcept
    {
      return this != &otherNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-issamenode
    KRYS_NODISCARD bool operator!=(const Node *otherNode) const noexcept
    {
      return this != otherNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-comparedocumentposition
    KRYS_NODISCARD DocumentPosition CompareDocumentPosition(const Node &other) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-contains
    KRYS_NODISCARD bool Contains(const Node *other) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-lookupprefix
    KRYS_NODISCARD DOMStringAtom LookupPrefix(DOMStringAtom namespaceURI) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-lookupnamespaceuri
    KRYS_NODISCARD DOMStringAtom LookupNamespaceURI(DOMStringAtom prefix) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-isdefaultnamespace
    KRYS_NODISCARD bool IsDefaultNamespace(DOMStringAtom namespaceURI) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-node-insertbefore
    ExceptionOr<Node &> InsertBefore(Node &newChild, Node *refChild) noexcept;

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

#pragma region Style

    /// @brief Whether the node's style needs resolving again.
    KRYS_NODISCARD bool NeedsStyleRecalc() const noexcept
    {
      return StyleValidity() != style::Validity::Valid || HasInvalidRenderer();
    }

    KRYS_NODISCARD style::Validity StyleValidity() const noexcept
    {
      return _styleValidity;
    }

    KRYS_NODISCARD bool HasInvalidRenderer() const noexcept
    {
      return HasNodeFlag(NodeFlags::HasInvalidRenderer);
    }

    KRYS_NODISCARD bool StyleResolutionShouldRecompositeLayer() const noexcept
    {
      return HasNodeFlag(NodeFlags::StyleResolutionShouldRecompositeLayer);
    }

    /// @brief Whether an end point of the frame's selection is anchored in the node or a descendant.
    KRYS_NODISCARD bool ContainsSelectionEndPoint() const noexcept
    {
      return HasNodeFlag(NodeFlags::ContainsSelectionEndPoint);
    }
    void SetContainsSelectionEndPoint(bool value) noexcept
    {
      if (value)
      {
        SetNodeFlag(NodeFlags::ContainsSelectionEndPoint);
      }
      else
      {
        ClearNodeFlag(NodeFlags::ContainsSelectionEndPoint);
      }
    }

    /// @brief Whether the node's text may decide the auto directionality of an ancestor.
    KRYS_NODISCARD bool SelfOrPrecedingNodesAffectDirAuto() const noexcept
    {
      return HasNodeFlag(NodeFlags::SelfOrPrecedingNodesAffectDirAuto);
    }
    void SetSelfOrPrecedingNodesAffectDirAuto(bool flag) noexcept
    {
      if (flag)
      {
        SetNodeFlag(NodeFlags::SelfOrPrecedingNodesAffectDirAuto);
      }
      else
      {
        ClearNodeFlag(NodeFlags::SelfOrPrecedingNodesAffectDirAuto);
      }
    }

    /// @brief The node's directionality: its own if it keeps one, otherwise the document element's.
    /// @see https://html.spec.whatwg.org/#the-directionality
    KRYS_NODISCARD TextDirection EffectiveTextDirection() const noexcept;
    void SetEffectiveTextDirection(TextDirection direction) noexcept;

    /// @brief Whether the node keeps a directionality of its own.
    KRYS_NODISCARD bool UsesEffectiveTextDirection() const noexcept
    {
      return HasNodeFlag(NodeFlags::UsesEffectiveTextDirection);
    }
    void SetUsesEffectiveTextDirection(bool value) noexcept
    {
      if (value)
      {
        SetNodeFlag(NodeFlags::UsesEffectiveTextDirection);
      }
      else
      {
        ClearNodeFlag(NodeFlags::UsesEffectiveTextDirection);
      }
    }

    KRYS_NODISCARD bool NeedsSVGRendererUpdate() const noexcept
    {
      return HasNodeFlag(NodeFlags::NeedsSVGRendererUpdate);
    }
    void SetNeedsSVGRendererUpdate(bool flag) noexcept
    {
      if (flag)
      {
        SetNodeFlag(NodeFlags::NeedsSVGRendererUpdate);
      }
      else
      {
        ClearNodeFlag(NodeFlags::NeedsSVGRendererUpdate);
      }
    }

    /// @brief Whether a descendant's style needs resolving again.
    KRYS_NODISCARD bool ChildNeedsStyleRecalc() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::DescendantNeedsStyleResolution);
    }

    /// @brief Whether a child's style needs resolving again.
    KRYS_NODISCARD bool DirectChildNeedsStyleRecalc() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::DirectChildNeedsStyleResolution);
    }

    void SetChildNeedsStyleRecalc() noexcept
    {
      SetStyleFlag(NodeStyleFlags::DescendantNeedsStyleResolution);
    }

    void SetDirectChildNeedsStyleRecalc() noexcept
    {
      SetStyleFlag(NodeStyleFlags::DirectChildNeedsStyleResolution);
    }

    void ClearChildNeedsStyleRecalc() noexcept
    {
      ClearStyleFlags(NodeStyleFlags::DescendantNeedsStyleResolution
                      | NodeStyleFlags::DirectChildNeedsStyleResolution);
    }

    void SetHasValidStyle() noexcept
    {
      _styleValidity = style::Validity::Valid;
      ClearNodeFlag(NodeFlags::IsComputedStyleInvalid | NodeFlags::HasInvalidRenderer
                    | NodeFlags::StyleResolutionShouldRecompositeLayer);
    }

    /// @brief The object the node is rendered as, or null if it isn't rendered.
    KRYS_NODISCARD RenderObject *Renderer() const noexcept
    {
      return _renderer;
    }

    /// @brief The node's renderer if it's a box, or null.
    KRYS_NODISCARD boo::RenderBox *GetRenderBox() const noexcept;

    void SetRenderer(RenderObject *renderer) noexcept
    {
      _renderer = renderer;
    }

    /// @brief The style the node is rendered with, or null if it isn't rendered.
    KRYS_NODISCARD const RenderStyle *GetRenderStyle() const noexcept;

    /// @brief The computed style of the node: an element's own, or else its parent element's in the composed
    /// tree.
    KRYS_NODISCARD const RenderStyle *ComputedStyle() noexcept;
    KRYS_NODISCARD virtual const RenderStyle *
      ComputedStyle(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    /// @brief The parent in the composed (flat) tree: the assigned slot, the host of a shadow root, or else
    /// the parent.
    KRYS_NODISCARD ContainerNode *ParentInComposedTree() const noexcept;
    KRYS_NODISCARD Element *ParentElementInComposedTree() const noexcept;
    KRYS_NODISCARD bool IsComposedTreeDescendantOf(const Node &node) const noexcept;

#pragma endregion

#pragma region Tree Helpers

    KRYS_NODISCARD uint32 CountChildNodes() const noexcept;
    /// @brief The child at @p index, or null if there are fewer children.
    KRYS_NODISCARD Node *TraverseToChildAt(uint32 index) const noexcept;
    /// @brief The node's index among its siblings.
    KRYS_NODISCARD uint32 ComputeNodeIndex() const noexcept;
    /// @see https://dom.spec.whatwg.org/#concept-node-length
    KRYS_NODISCARD uint32 Length() const noexcept;
    /// @brief The parent, or a shadow root's host.
    KRYS_NODISCARD ContainerNode *ParentOrShadowHostNode() const noexcept;
    /// @brief The parent, unless it's a shadow root.
    KRYS_NODISCARD ContainerNode *NonShadowBoundaryParentNode() const noexcept;
    /// @brief The ancestor that's a child of a shadow root, or null outside shadow trees.
    KRYS_NODISCARD Node *NonBoundaryShadowTreeRootNode() noexcept;
    KRYS_NODISCARD bool IsDescendantOf(const Node *other) const noexcept;
    KRYS_NODISCARD bool IsDescendantOf(const Node &other) const noexcept
    {
      return IsDescendantOf(&other);
    }
    KRYS_NODISCARD bool IsInclusiveDescendantOf(const Node &other) const noexcept
    {
      return this == &other || IsDescendantOf(other);
    }
    KRYS_NODISCARD bool IsShadowIncludingDescendantOf(const Node *other) const noexcept;
    /// @brief The last node in the subtree, in tree order.
    KRYS_NODISCARD Node *LastDescendant() const noexcept;

#pragma endregion

#pragma region Editing

    enum class UserSelectAllTreatment : bool
    {
      NotEditable,
      Editable
    };

    enum class Editability : uint8
    {
      ReadOnly,
      CanEditPlainText,
      CanEditRichly
    };

    enum class ShouldUpdateStyle : bool
    {
      Update,
      DoNotUpdate
    };

    /// @brief Whether the node is editable, resolving style first if it's out of date.
    KRYS_NODISCARD bool IsContentEditable() const noexcept;
    KRYS_NODISCARD bool IsContentRichlyEditable() const noexcept;

    /// @brief Whether the node is editable, by the style it has (`-webkit-user-modify`, which
    /// `contenteditable` maps to, and `user-select`).
    KRYS_NODISCARD bool
      HasEditableStyle(UserSelectAllTreatment treatment = UserSelectAllTreatment::NotEditable) const noexcept
    {
      return ComputeEditability(treatment, ShouldUpdateStyle::DoNotUpdate) != Editability::ReadOnly;
    }

    // FIXME: Replace every use of this function by helpers in Editing.h
    KRYS_NODISCARD bool HasRichlyEditableStyle() const noexcept
    {
      return ComputeEditability(UserSelectAllTreatment::NotEditable, ShouldUpdateStyle::DoNotUpdate)
             == Editability::CanEditRichly;
    }

    KRYS_NODISCARD Editability ComputeEditability(UserSelectAllTreatment treatment,
                                                  ShouldUpdateStyle shouldUpdateStyle) const noexcept;
    KRYS_NODISCARD Editability
      ComputeEditabilityWithStyle(const RenderStyle *incomingStyle, UserSelectAllTreatment treatment,
                                  ShouldUpdateStyle shouldUpdateStyle) const noexcept;

    /// @brief Whether the node is an editing host: an editable element whose parent isn't editable (or the
    /// body).
    KRYS_NODISCARD bool IsRootEditableElement() const noexcept;
    /// @brief The editing host the node is in, or null if it isn't editable.
    KRYS_NODISCARD Element *RootEditableElement() const noexcept;

    /// @brief Whether a selection can start at the node (dragging takes priority in draggable elements that
    /// can't be selected).
    KRYS_NODISCARD virtual bool CanStartSelection() const noexcept;

    /// @brief Handles @p event after its dispatch when no listener canceled it: called for the event's
    /// target, then for the nodes on its path until one handles it (`Event::SetDefaultHandled`), if it
    /// bubbles.
    virtual void DefaultEventHandler(Event &event) noexcept
    {
      (void)event;
    }
    /// @brief Whether a position can be inside the node; the content of nodes it can't be inside (images,
    /// `<br>`, form controls) is ignored by editing.
    KRYS_NODISCARD virtual bool CanContainRangeEndPoint() const noexcept
    {
      return false;
    }

#pragma endregion

#pragma region Style Sheet Owner

    /// @brief Called when the style sheet this node owns has finished loading.
    /// @return Whether the node is done loading its sheet.
    virtual bool SheetLoaded() noexcept
    {
      return true;
    }

    /// @brief Called once the sheet this node owns and its critical subresources have loaded.
    virtual void NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept
    {
      (void)errorOccurred;
    }

    /// @brief Called when the style sheet this node owns starts loading a subresource after it was created.
    virtual void StartLoadingDynamicSheet() noexcept
    {
      krys_unreachable();
    }

#pragma endregion

  protected:
    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD EventTarget *GetParent(Event &event) const noexcept override;

  public:
#pragma region Tree Scope

    /// @brief The document or shadow root of the tree this node is in: its shadow root's when its root is
    /// one, its document's otherwise (even while it's disconnected).
    KRYS_NODISCARD DocumentOrShadowRoot &TreeScope() const noexcept;

    /// @brief The tree scope the node's SVG references are looked up in: its own, or, inside a `<use>`
    /// element's shadow tree, the `<use>` element's.
    KRYS_NODISCARD DocumentOrShadowRoot &TreeScopeForSVGReferences() const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-tree
    KRYS_NODISCARD bool IsInShadowTree() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsInShadowTree);
    }

    /// @brief Whether the node is in a user agent shadow tree.
    KRYS_NODISCARD bool IsInUserAgentShadowTree() const noexcept;

    /// @brief Whether the node's root is a document: its shadow-including root is, and it isn't in a shadow
    /// tree.
    /// @see https://dom.spec.whatwg.org/#in-a-document-tree
    /// @see https://dom.spec.whatwg.org/#concept-document-tree
    KRYS_NODISCARD bool IsInDocumentTree() const noexcept
    {
      return IsConnected() && !IsInShadowTree();
    }

#pragma endregion

  protected:
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

    KRYS_NODISCARD bool IsPseudoElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsPseudoElement);
    }

    /// @brief Whether this is an element with an inline style (`StyledElement`).
    KRYS_NODISCARD bool IsStyledElement() const noexcept
    {
      return !!(_flags & (NodeFlags::IsHTMLElement | NodeFlags::IsSVGElement | NodeFlags::IsMathMLElement));
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
    void SetParentNode(ContainerNode *parent) noexcept
    {
      _parentNode = ShareCheckedPtr(parent);
    }

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetNextSibling(Node *sibling) noexcept
    {
      _nextSibling = ShareCheckedPtr(sibling);
    }

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetPreviousSibling(Node *sibling) noexcept
    {
      _previousSibling = ShareCheckedPtr(sibling);
    }

#pragma endregion

#pragma region Style Invalidation

    /// @brief Marks the node's style invalid, and its composed tree ancestors as having a descendant to
    /// resolve.
    void InvalidateStyle(style::Validity validity,
                         style::InvalidationMode mode = style::InvalidationMode::Normal) noexcept;
    void UpdateAncestorsForStyleRecalc() noexcept;
    void MarkAncestorsForInvalidatedStyle() noexcept;

    KRYS_NODISCARD bool HasStyleFlag(NodeStyleFlags flag) const noexcept
    {
      return HasFlag(_styleFlags, flag);
    }

    void SetStyleFlag(NodeStyleFlags flag) noexcept
    {
      _styleFlags = _styleFlags | flag;
    }

    void ClearStyleFlags(NodeStyleFlags flags) noexcept
    {
      _styleFlags = _styleFlags & ~flags;
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

  private:
    void AdjustStyleValidity(style::Validity validity, style::InvalidationMode mode) noexcept;
  };
}

namespace krys::boo::dom
{
  /// @brief The trees a node's ancestors can be walked in: the node tree, the tree including shadow roots'
  /// hosts, and the composed (flat) tree.
  enum class TreeType : uint8
  {
    Tree,
    ShadowIncludingTree,
    ComposedTree
  };

  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD ContainerNode *Parent(const Node &node) noexcept;
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD Node *CommonInclusiveAncestor(const Node &a, const Node &b) noexcept;
  /// @brief The order of @p a and @p b in the tree, or unordered if they aren't in the same one.
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD std::partial_ordering TreeOrder(const Node &a, const Node &b) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::Node)
  static bool IsType(const krys::boo::dom::EventTarget &target) noexcept
  {
    return target.IsNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();