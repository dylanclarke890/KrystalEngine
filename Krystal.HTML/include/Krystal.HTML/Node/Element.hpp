#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ElementRareData.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"

namespace Krys::HTML
{
  class Attr;
  class HTMLCollection;
  class HTMLSlotElement;
  class NodeList;

  class Element : public ContainerNode
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Element);

  private:
    QualifiedName _qualifiedName;
    DOMStringAtom _tagName;
    RefPtr<ShadowRoot> _shadowRoot;
    UniquePtr<ElementRareData> _elementRareData;

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

    KRYS_NODISCARD DOMStringAtom TagName() const noexcept
    {
      return _tagName;
    }

    KRYS_NODISCARD RawPtr<ShadowRoot> GetShadowRoot() const noexcept
    {
      return _shadowRoot.get();
    }

    KRYS_NODISCARD ExceptionOr<void> RemoveAttributeNode(Attr &attribute) const noexcept;

#pragma endregion

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final;

    KRYS_NODISCARD DOMString TextContent() const noexcept final;

    ExceptionOr<void> SetTextContent(DOMString &&value) noexcept final;

#pragma endregion

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    ExceptionOr<void> Before(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> After(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> ReplaceWith(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    KRYS_NODISCARD RawPtr<const Element> PreviousElementSibling() const noexcept;
    KRYS_NODISCARD RawPtr<Element> PreviousElementSibling() noexcept;

    KRYS_NODISCARD RawPtr<const Element> NextElementSibling() const noexcept;
    KRYS_NODISCARD RawPtr<Element> NextElementSibling() noexcept;

#pragma endregion

#pragma region ParentNode Mixin - https://dom.spec.whatwg.org/#parentnode

    KRYS_NODISCARD Ref<HTMLCollection> Children() noexcept;

    KRYS_NODISCARD RawPtr<const Element> FirstElementChild() const noexcept;
    KRYS_NODISCARD RawPtr<Element> FirstElementChild() noexcept;

    KRYS_NODISCARD RawPtr<const Element> LastElementChild() const noexcept;
    KRYS_NODISCARD RawPtr<Element> LastElementChild() noexcept;

    KRYS_NODISCARD size_t ChildElementCount() const noexcept;

    ExceptionOr<void> Prepend(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> Append(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> ReplaceChildren(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> MoveBefore(Node &node, RawPtr<Node> child) noexcept;

    KRYS_NODISCARD ExceptionOr<RawPtr<Element>> QuerySelector(const DOMString &selectors) noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<NodeList>> QuerySelectorAll(const DOMString &selectors) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Element)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
