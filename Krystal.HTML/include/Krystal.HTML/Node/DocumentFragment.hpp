#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/DocumentFragmentRareData.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"

namespace Krys::HTML
{
  class Document;
  class Element;
  class HTMLCollection;

  class DocumentFragment : public ContainerNode
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DocumentFragment);

    friend class DocumentFragmentRareData;

  private:
    UniquePtr<DocumentFragmentRareData> _documentFragmentRareData;

  public:
    DocumentFragment(Document &document, NodeFlag flags = NodeFlag::None) noexcept;

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return u8"#document-fragment";
    }

    KRYS_NODISCARD DOMString TextContent() const noexcept final;

    ExceptionOr<void> SetTextContent(DOMString &&value) noexcept final;

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

    ExceptionOr<void> MoveBefore(Node &node, RefPtr<Node> &&child) noexcept;

    KRYS_NODISCARD RawPtr<Element> QuerySelector(const DOMString &selectors) noexcept;
    KRYS_NODISCARD Ref<NodeList> QuerySelectorAll(const DOMString &selectors) noexcept;

#pragma endregion

#pragma region NonElementParentNode Mixin - https://dom.spec.whatwg.org/#interface-nonelementparentnode

    RefPtr<Element> GetElementById(const DOMStringAtom &id) const noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::DocumentFragment)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsDocumentFragmentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();