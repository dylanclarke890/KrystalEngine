#pragma once

#include "Krystal.HTML/DOMTokenList.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ElementRareData.hpp"
#include "Krystal.HTML/Node/InsertAdjacentWhere.hpp"
#include "Krystal.HTML/Node/ShadowRootInit.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class Attr;
  class HTMLCollection;
  class HTMLSlotElement;
  class NamedNodeMap;
  class NodeList;
  class ShadowRoot;
  class ElementAttributeAlgorithms;

  class Element : public ContainerNode
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Element);

    friend class ElementAttributeAlgorithms;

  private:
    QualifiedName _qualifiedName;
    RefPtr<ShadowRoot> _shadowRoot;
    UniquePtr<ElementRareData> _elementRareData;
    List<Ref<Attr>> _attributes;

  protected:
    Element(Document &document, NodeFlag nodeFlags = NodeFlag::None) noexcept;

  public:
#pragma region Element

    KRYS_NODISCARD DOMStringAtom LocalName() const noexcept
    {
      return _qualifiedName.LocalName;
    }
    KRYS_NODISCARD DOMStringAtom Prefix() const noexcept
    {
      return _qualifiedName.Prefix;
    }
    KRYS_NODISCARD DOMStringAtom NamespaceURI() const noexcept
    {
      return _qualifiedName.NamespaceURI;
    }
    KRYS_NODISCARD DOMString TagName() const noexcept
    {
      auto qualifiedName = _qualifiedName.Name();
      // TODO(impl):
      // If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName to
      // qualifiedName in ASCII uppercase.

      return qualifiedName;
    }

    void Id(DOMString &&id) noexcept;
    KRYS_NODISCARD DOMString Id() const noexcept;
    void ClassName(DOMString &&className) noexcept;
    KRYS_NODISCARD DOMString ClassName() const noexcept;
    KRYS_NODISCARD DOMTokenList ClassList() noexcept;
    void Slot(DOMString &&slot) noexcept;
    KRYS_NODISCARD DOMString Slot() const noexcept;

    KRYS_NODISCARD bool HasAttributes() const noexcept;
    KRYS_NODISCARD NamedNodeMap &Attributes() const noexcept;
    KRYS_NODISCARD List<DOMString> GetAttributeNames() const noexcept;
    KRYS_NODISCARD Maybe<DOMString> GetAttribute(DOMStringAtom qualifiedName) const noexcept;
    KRYS_NODISCARD Maybe<DOMString> GetAttributeNS(DOMStringAtom namespaceURI,
                                                   DOMStringAtom localName) const noexcept;
    ExceptionOr<void> SetAttribute(DOMStringAtom qualifiedName, DOMString &&value) noexcept;
    ExceptionOr<void> SetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                                     DOMString &&value) noexcept;
    void RemoveAttribute(DOMStringAtom qualifiedName) noexcept;
    void RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept;
    ExceptionOr<bool> ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept;
    KRYS_NODISCARD bool HasAttribute(DOMStringAtom qualifiedName) const noexcept;
    KRYS_NODISCARD bool HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept;

    KRYS_NODISCARD RawPtr<Attr> GetAttributeNode(DOMStringAtom qualifiedName) const noexcept;
    KRYS_NODISCARD RawPtr<Attr> GetAttributeNodeNS(DOMStringAtom namespaceURI,
                                                   DOMStringAtom localName) const noexcept;
    ExceptionOr<RefPtr<Attr>> SetAttributeNode(Attr &attr) noexcept;
    ExceptionOr<RefPtr<Attr>> SetAttributeNodeNS(Attr &attr) noexcept;
    ExceptionOr<Ref<Attr>> RemoveAttributeNode(Attr &attr) noexcept;

    Ref<ShadowRoot> AttachShadow(const ShadowRootInit &init) noexcept;
    KRYS_NODISCARD RawPtr<ShadowRoot> ShadowRoot() const noexcept
    {
      return _shadowRoot.get();
    }

    KRYS_NODISCARD RawPtr<CustomElementRegistry> CustomElementRegistry() const noexcept
    {
      // TODO(impl): Implement custom element registry and return it here.
      return nullptr;
    }

    KRYS_NODISCARD RawPtr<Element> Closest(const DOMString &selectors) noexcept;
    KRYS_NODISCARD bool Matches(const DOMString &selectors) const noexcept;
    KRYS_NODISCARD bool WebkitMatchesSelector(const DOMString &selectors) const noexcept
    {
      return Matches(selectors);
    }

    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagName(DOMStringAtom qualifiedName) noexcept;
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                              DOMStringAtom localName) noexcept;
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByClassName(const DOMString &classNames) noexcept;

    RawPtr<Element> InsertAdjacentElement(InsertAdjacentWhere where, Element &element) noexcept;
    void InsertAdjacentText(InsertAdjacentWhere where, const DOMString &data) noexcept;

#pragma endregion

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final;

    KRYS_NODISCARD DOMString TextContent() const noexcept final;

    ExceptionOr<void> SetTextContent(DOMString &&value) noexcept final;

#pragma endregion

#pragma region ParentNode Mixin - https://dom.spec.whatwg.org/#parentnode

    KRYS_NODISCARD Ref<HTMLCollection> Children() noexcept;
    KRYS_NODISCARD RefPtr<const Element> FirstElementChild() const noexcept;
    KRYS_NODISCARD RefPtr<Element> FirstElementChild() noexcept;
    KRYS_NODISCARD RefPtr<const Element> LastElementChild() const noexcept;
    KRYS_NODISCARD RefPtr<Element> LastElementChild() noexcept;
    KRYS_NODISCARD size_t ChildElementCount() const noexcept;

    ExceptionOr<void> Prepend(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> Append(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> ReplaceChildren(const List<NodeOrString> &nodes) noexcept;

    ExceptionOr<void> MoveBefore(Node &node, RawPtr<Node> child) noexcept;

    KRYS_NODISCARD ExceptionOr<RefPtr<Element>> QuerySelector(const DOMString &selectors) noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<NodeList>> QuerySelectorAll(const DOMString &selectors) noexcept;

#pragma endregion

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    KRYS_NODISCARD RefPtr<const Element> PreviousElementSibling() const noexcept;
    KRYS_NODISCARD RefPtr<Element> PreviousElementSibling() noexcept;

    KRYS_NODISCARD RefPtr<const Element> NextElementSibling() const noexcept;
    KRYS_NODISCARD RefPtr<Element> NextElementSibling() noexcept;

#pragma endregion

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    ExceptionOr<void> Before(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> After(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> ReplaceWith(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region Slottable Mixin - https://dom.spec.whatwg.org/#slotable

    KRYS_NODISCARD RawPtr<HTMLSlotElement> AssignedSlot() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Element)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
