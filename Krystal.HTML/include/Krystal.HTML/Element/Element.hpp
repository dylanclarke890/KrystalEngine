#pragma once

#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Attr;
  class HTMLCollection;

  class Element : public ContainerNode
  {
  protected:
    Element(Document &document, NodeFlag nodeFlags = NodeFlag::None) noexcept;

  public:
    KRYS_NODISCARD DOMString NodeName() const noexcept final;

    KRYS_NODISCARD ExceptionOr<void> RemoveAttributeNode(Attr &attribute) const noexcept;

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    ExceptionOr<void> Before(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> After(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> ReplaceWith(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    KRYS_NODISCARD RawPtr<Element> PreviousElementSibling() const noexcept;
    KRYS_NODISCARD RawPtr<Element> NextElementSibling() const noexcept;

#pragma endregion

#pragma region ParentNode Mixin - https://dom.spec.whatwg.org/#parentnode

    KRYS_NODISCARD Ref<HTMLCollection> Children() const noexcept;
    KRYS_NODISCARD RawPtr<Element> FirstElementChild() const noexcept;
    KRYS_NODISCARD RawPtr<Element> LastElementChild() const noexcept;
    KRYS_NODISCARD size_t ChildElementCount() const noexcept;

    ExceptionOr<void> Prepend(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> Append(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> ReplaceChildren(const List<NodeOrString> &nodes) noexcept;

    ExceptionOr<void> MoveBefore(Node &node, RefPtr<Node> &&child) noexcept;

    KRYS_NODISCARD RawPtr<Element> QuerySelector(const DOMString &selectors) noexcept;
    KRYS_NODISCARD Ref<NodeList> QuerySelectorAll(const DOMString &selectors) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Element)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
