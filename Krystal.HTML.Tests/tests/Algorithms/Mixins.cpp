#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  template <typename T>
  concept NonElementParentNodeMixin = requires(T &node, const T &constNode, DOMStringView id) {
    { node.GetElementById(id) } -> SameType<RefPtr<Element>>;
    { constNode.GetElementById(id) } -> SameType<RefPtr<const Element>>;
  };

  TEST_CASE("Mixin::NonElementParentNode", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(NonElementParentNodeMixin<Document>);
    STATIC_REQUIRE(NonElementParentNodeMixin<DocumentFragment>);
  }

  template <typename T>
  concept DocumentOrShadowRootMixin = requires(T &node) {
    { node.CustomElementRegistry() } -> SameType<RefPtr<CustomElementRegistry>>;
  };

  TEST_CASE("Mixin::DocumentOrShadowRoot", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(DocumentOrShadowRootMixin<Document>);
    STATIC_REQUIRE(DocumentOrShadowRootMixin<ShadowRoot>);
  }

  template <typename T>
  concept ParentNodeMixin = requires(T &node, const T &constNode, const List<NodeOrString> &nodes,
                                     RawPtr<Node> optionalNode, DOMString selectors) {
    { node.Children() } -> SameType<Ref<HTMLCollection>>;
    { constNode.FirstElementChild() } -> SameType<RefPtr<const Element>>;
    { node.FirstElementChild() } -> SameType<RefPtr<Element>>;
    { constNode.LastElementChild() } -> SameType<RefPtr<const Element>>;
    { node.LastElementChild() } -> SameType<RefPtr<Element>>;
    { node.ChildElementCount() } -> SameType<size_t>;

    { node.Prepend(nodes) } -> SameType<ExceptionOr<void>>;
    { node.Append(nodes) } -> SameType<ExceptionOr<void>>;
    { node.ReplaceChildren(nodes) } -> SameType<ExceptionOr<void>>;

    { node.MoveBefore(node, optionalNode) } -> SameType<ExceptionOr<void>>;

    { node.QuerySelector(selectors) } -> SameType<ExceptionOr<RefPtr<Element>>>;
    { node.QuerySelectorAll(selectors) } -> SameType<ExceptionOr<Ref<NodeList>>>;
  };

  TEST_CASE("Mixin::ParentNode", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(ParentNodeMixin<Element>);
    STATIC_REQUIRE(ParentNodeMixin<Document>);
    STATIC_REQUIRE(ParentNodeMixin<DocumentFragment>);
  }

  template <typename T>
  concept NonDocumentTypeChildNodeMixin = requires(T &node, const T &constNode) {
    { constNode.PreviousElementSibling() } -> SameType<RefPtr<const Element>>;
    { node.PreviousElementSibling() } -> SameType<RefPtr<Element>>;
    { constNode.NextElementSibling() } -> SameType<RefPtr<const Element>>;
    { node.NextElementSibling() } -> SameType<RefPtr<Element>>;
  };

  TEST_CASE("Mixin::NonDocumentTypeChildNode", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(NonDocumentTypeChildNodeMixin<Element>);
    STATIC_REQUIRE(NonDocumentTypeChildNodeMixin<CharacterData>);
  }

  template <typename T>
  concept ChildNodeMixin = requires(T &node, const List<NodeOrString> &nodes) {
    { node.Before(nodes) } -> SameType<ExceptionOr<void>>;
    { node.After(nodes) } -> SameType<ExceptionOr<void>>;
    { node.ReplaceWith(nodes) } -> SameType<ExceptionOr<void>>;
    { node.Remove() } -> SameType<ExceptionOr<void>>;
  };

  TEST_CASE("Mixin::ChildNode", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(ChildNodeMixin<DocumentType>);
    STATIC_REQUIRE(ChildNodeMixin<Element>);
    STATIC_REQUIRE(ChildNodeMixin<CharacterData>);
  }

  template <typename T>
  concept SlottableMixin = requires(T &node) {
    { node.AssignedSlot() } -> SameType<RawPtr<HTMLSlotElement>>;
  };

  TEST_CASE("Mixin::Slottable", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(SlottableMixin<Element>);
    STATIC_REQUIRE(SlottableMixin<Krys::HTML::Text>);
  }
}