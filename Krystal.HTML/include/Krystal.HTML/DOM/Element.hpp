#pragma once

#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Dicts/ShadowRootInit.hpp"
#include "Krystal.HTML/DOM/DOMTokenList.hpp"
#include "Krystal.HTML/DOM/Enums/CustomElementState.hpp"
#include "Krystal.HTML/DOM/Enums/InsertAdjacentWhere.hpp"
#include "Krystal.HTML/Types/QualifiedName.hpp"
#include "Krystal.HTML/DOM/Internals/RareData/ElementRareData.hpp"
#include "Krystal.HTML/DOM/NamedNodeMap.hpp"
#include "Krystal.HTML/DOM/Types/NodeOrString.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class HTMLCollection;
  class HTMLSlotElement;
  class NodeList;
  class ShadowRoot;

  /// @see https://dom.spec.whatwg.org/#interface-element
  class Element : public ContainerNode
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Element);

    friend class ElementFactory;
    friend class DocumentAlgorithms;
    friend class ElementAlgorithms;
    friend class ExtensibilityHooks;
    friend class SlotAlgorithms;
    friend class HTMLCollectionAlgorithms;
    friend class Node;
    friend class NodeAlgorithms;
    friend class NamedNodeMap;
    friend class TreeQueries;

  private:
    QualifiedName _qualifiedName;
    RefPtr<ShadowRoot> _shadowRoot;
    RefPtr<CustomElementRegistry> _customElementRegistry;
    UniquePtr<ElementRareData> _elementRareData;
    UniquePtr<NamedNodeMap> _namedNodeMap;
    UniquePtr<DOMTokenList> _domTokenList;
    List<Ref<Attr>> _attributes;
    DOMStringAtom _is {DOMStringAtom::Null()};
    CustomElementState _customElementState {CustomElementState::Undefined};

    /// @see https://dom.spec.whatwg.org/#slotable-name
    DOMString _slottableName;
    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    RefPtr<HTMLSlotElement> _assignedSlot;
    /// @see https://dom.spec.whatwg.org/#slotable-manual-slot-assignment
    WeakPtr<HTMLSlotElement> _manuallyAssignedSlot;

  protected:
    Element(Document &document, const QualifiedName &name, NodeFlags nodeFlags) noexcept;

  public:
#pragma region Element - https://dom.spec.whatwg.org/#element

    /// @see https://dom.spec.whatwg.org/#dom-element-namespaceuri
    KRYS_NODISCARD DOMStringAtom NamespaceURI() const noexcept
    {
      return _qualifiedName.NamespaceURI();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-namespaceuri
    KRYS_NODISCARD DOMStringAtom Prefix() const noexcept
    {
      return _qualifiedName.NamespacePrefix();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-namespaceuri
    KRYS_NODISCARD DOMStringAtom LocalName() const noexcept
    {
      return _qualifiedName.LocalName();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-tagname
    KRYS_NODISCARD DOMString TagName() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-id
    void Id(DOMString &&id) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-id
    KRYS_NODISCARD DOMString Id() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classname
    void ClassName(DOMString &&className) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classname
    KRYS_NODISCARD DOMString ClassName() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classlist
    KRYS_NODISCARD DOMTokenList &ClassList() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-slot
    void Slot(DOMString &&slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-slot
    KRYS_NODISCARD DOMString Slot() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-hasattributes
    KRYS_NODISCARD bool HasAttributes() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-attributes
    KRYS_NODISCARD NamedNodeMap &Attributes() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributenames
    KRYS_NODISCARD List<DOMString> GetAttributeNames() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattribute
    KRYS_NODISCARD Maybe<DOMString> GetAttribute(DOMStringAtom qualifiedName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributens
    KRYS_NODISCARD Maybe<DOMString> GetAttributeNS(DOMStringAtom namespaceURI,
                                                   DOMStringAtom localName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattribute
    ExceptionOr<void> SetAttribute(DOMStringAtom qualifiedName, DOMString &&value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattributens
    ExceptionOr<void> SetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                                     DOMString &&value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-removeattribute
    void RemoveAttribute(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-removeattributens
    void RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-toggleattribute
    ExceptionOr<bool> ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-hasattribute
    KRYS_NODISCARD bool HasAttribute(DOMStringAtom qualifiedName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-hasattributens
    KRYS_NODISCARD bool HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributenode
    KRYS_NODISCARD RawPtr<Attr> GetAttributeNode(DOMStringAtom qualifiedName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributenodens
    KRYS_NODISCARD RawPtr<Attr> GetAttributeNodeNS(DOMStringAtom namespaceURI,
                                                   DOMStringAtom localName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattributenode
    ExceptionOr<RefPtr<Attr>> SetAttributeNode(Attr &attr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattributenodens
    ExceptionOr<RefPtr<Attr>> SetAttributeNodeNS(Attr &attr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-removeattributenode
    ExceptionOr<Ref<Attr>> RemoveAttributeNode(Attr &attr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-attachshadow
    ExceptionOr<Ref<ShadowRoot>> AttachShadow(const ShadowRootInit &init) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-shadowroot
    KRYS_NODISCARD RefPtr<ShadowRoot> ShadowRoot() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-customelementregistry
    KRYS_NODISCARD RefPtr<CustomElementRegistry> CustomElementRegistry() const noexcept
    {
      return _customElementRegistry;
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-closest
    KRYS_NODISCARD RefPtr<Element> Closest(DOMStringView selectors) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-matches
    KRYS_NODISCARD bool Matches(DOMStringView selectors) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-webkitmatchesselector
    KRYS_NODISCARD bool WebkitMatchesSelector(DOMStringView selectors) const noexcept
    {
      return Matches(selectors);
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbytagname
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagName(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbytagnamens
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                              DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbyclassname
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByClassName(DOMStringAtom classNames) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-insertadjacentelement
    ExceptionOr<RawPtr<Element>> InsertAdjacentElement(InsertAdjacentWhere where, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-insertadjacenttext
    ExceptionOr<void> InsertAdjacentText(InsertAdjacentWhere where, DOMString &&data) noexcept;

#pragma endregion

#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final;

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    KRYS_NODISCARD Maybe<DOMString> TextContent() const noexcept final;

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    ExceptionOr<void> TextContent(DOMString &&value) noexcept final;

#pragma endregion

#pragma region ParentNode Mixin - https://dom.spec.whatwg.org/#parentnode

    /// @brief Returns the child elements.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-children
    KRYS_NODISCARD Ref<HTMLCollection> Children() noexcept;

    /// @brief Returns the first child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-firstelementchild
    KRYS_NODISCARD RefPtr<const Element> FirstElementChild() const noexcept;

    /// @brief Returns the first child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-firstelementchild
    KRYS_NODISCARD RefPtr<Element> FirstElementChild() noexcept;

    /// @brief Returns the last child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-lastelementchild
    KRYS_NODISCARD RefPtr<const Element> LastElementChild() const noexcept;

    /// @brief Returns the last child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-lastelementchild
    KRYS_NODISCARD RefPtr<Element> LastElementChild() noexcept;

    /// @brief Returns the number of child elements.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-childelementcount
    KRYS_NODISCARD size_t ChildElementCount() const noexcept;

    /// @brief Inserts `nodes` before the first child of `this`, while replacing strings in `nodes` with
    /// equivalent Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-prepend
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    ExceptionOr<void> Prepend(const List<NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` after the last child of `this`, while replacing strings in `nodes` with
    /// equivalent Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-append
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    ExceptionOr<void> Append(const List<NodeOrString> &nodes) noexcept;

    /// @brief Replace all children of `this` with `nodes`, while replacing strings in `nodes` with equivalent
    /// Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-replacechildren
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    ExceptionOr<void> ReplaceChildren(const List<NodeOrString> &nodes) noexcept;

    /// @brief Moves, without first removing, `movedNode` into `this` after `child` if `child` is non-null;
    /// otherwise after the last child of node. This method preserves state associated with `movedNode`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-movebefore
    /// @throws HierarchyRequestError if the constraints of the node tree are violated, or the state
    /// associated with the moved node cannot be preserved.
    ExceptionOr<void> MoveBefore(Node &node, RawPtr<Node> child) noexcept;

    /// @brief Returns the first element that is a descendant of `this` that matches `selectors`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-queryselector
    KRYS_NODISCARD ExceptionOr<RefPtr<Element>> QuerySelector(DOMStringView selectors) noexcept;

    /// @brief Returns all element descendants of `this` that match `selectors`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-queryselectorall
    KRYS_NODISCARD ExceptionOr<Ref<NodeList>> QuerySelectorAll(DOMStringView selectors) noexcept;

#pragma endregion

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    /// @brief Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<const Element> PreviousElementSibling() const noexcept;

    /// @brief Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<Element> PreviousElementSibling() noexcept;

    /// @brief Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<const Element> NextElementSibling() const noexcept;

    /// @brief Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<Element> NextElementSibling() noexcept;

#pragma endregion

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    /// @brief Inserts `nodes` just before `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-before
    ExceptionOr<void> Before(const List<NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` just after `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-after
    ExceptionOr<void> After(const List<NodeOrString> &nodes) noexcept;

    /// @brief Replaces `this` with `nodes`, while replacing strings in `nodes` with equivalent Text nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-replacewith
    ExceptionOr<void> ReplaceWith(const List<NodeOrString> &nodes) noexcept;

    /// @brief Removes `this`.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-remove
    ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region Slottable Mixin - https://dom.spec.whatwg.org/#slotable

    /// @see https://dom.spec.whatwg.org/#dom-slotable-assignedslot
    KRYS_NODISCARD RefPtr<HTMLSlotElement> AssignedSlot() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Element)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
