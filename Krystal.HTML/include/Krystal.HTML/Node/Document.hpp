#pragma once

#include "Krystal.HTML/Iterator/NodeIterator.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/DocumentRareData.hpp"
#include "Krystal.HTML/Node/TreeScope.hpp"
#include "Krystal.HTML/Ranges/Range.hpp"
#include "Krystal.HTML/URL.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class HTMLCollection;
  class MutationAlgorithms;

  class Document : public ContainerNode, public TreeScope
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Document);

    friend class DocumentRareData;
    friend class MutationAlgorithms;
    friend class TreeMutationDispatcher;

  private:
    UniquePtr<DocumentRareData> _documentRareData;
    URL _baseURL {u8"about:blank"};
    List<RawPtr<Range>> _liveRanges;
    List<RawPtr<NodeIterator>> _nodeIterators;
    RefPtr<CustomElementRegistry> _customElementRegistry;

  public:
    Document() noexcept;

#pragma region Document

    KRYS_NODISCARD ExceptionOr<Ref<Node>> AdoptNode(Node &node) noexcept;

#pragma endregion

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return u8"#document";
    }

    KRYS_NODISCARD URL BaseURI() const noexcept
    {
      return _baseURL;
    }

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

    KRYS_NODISCARD ExceptionOr<RefPtr<Element>> QuerySelector(DOMStringView selectors) noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<NodeList>> QuerySelectorAll(DOMStringView selectors) noexcept;

#pragma endregion

#pragma region NonElementParentNode Mixin - https://dom.spec.whatwg.org/#interface-nonelementparentnode

    RefPtr<Element> GetElementById(DOMStringView elementId) noexcept;

    RefPtr<const Element> GetElementById(DOMStringView elementId) const noexcept;

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin - https://dom.spec.whatwg.org/#mixin-documentorshadowroot

    RefPtr<CustomElementRegistry> CustomElementRegistry() const noexcept
    {
      return _customElementRegistry;
    }

#pragma endregion

    // TODO(fix): this shouldn't be part of the public API
    KRYS_NODISCARD List<RawPtr<Range>> &LiveRanges() noexcept
    {
      return _liveRanges;
    }

    KRYS_NODISCARD const List<RawPtr<Range>> &LiveRanges() const noexcept
    {
      return _liveRanges;
    }

    KRYS_NODISCARD List<RawPtr<NodeIterator>> &NodeIterators() noexcept
    {
      return _nodeIterators;
    }

    KRYS_NODISCARD const List<RawPtr<NodeIterator>> &NodeIterators() const noexcept
    {
      return _nodeIterators;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Document)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsDocumentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
