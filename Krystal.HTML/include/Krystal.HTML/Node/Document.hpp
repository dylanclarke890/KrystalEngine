#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/DocumentRareData.hpp"
#include "Krystal.HTML/Node/TreeScope.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class HTMLCollection;

  class Document : public ContainerNode, public TreeScope
  {
    friend class DocumentRareData;

  private:
    UniquePtr<DocumentRareData> _documentRareData;

  public:
    Document() noexcept;

    KRYS_NODISCARD DOMString NodeName() const noexcept final;

    KRYS_NODISCARD ExceptionOr<Ref<Node>> AdoptNode(Node &node) noexcept;

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

#pragma region NonElementParentNode Mixin - https://dom.spec.whatwg.org/#interface-nonelementparentnode

    RefPtr<Element> GetElementById(const DOMStringAtom &id) const noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Document)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsDocumentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
