#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <catch_all.hpp>

namespace krys::tests
{
  using namespace krys::boo;

  template <typename T>
  concept NonElementParentNodeMixin = requires(T &node, const T &constNode, dom::DOMStringView id) {
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
                                     Node *optionalNode, dom::DOMString selectors) {
    { node.Children() } -> SameType<Ref<HTMLCollection>>;
    { constNode.FirstElementChild() } -> SameType<RefPtr<const Element>>;
    { node.FirstElementChild() } -> SameType<RefPtr<Element>>;
    { constNode.LastElementChild() } -> SameType<RefPtr<const Element>>;
    { node.LastElementChild() } -> SameType<RefPtr<Element>>;
    { node.ChildElementCount() } -> SameType<size_t>;

    { node.Prepend(nodes) } -> SameType<dom::ExceptionOr<void>>;
    { node.Append(nodes) } -> SameType<dom::ExceptionOr<void>>;
    { node.ReplaceChildren(nodes) } -> SameType<dom::ExceptionOr<void>>;

    { node.MoveBefore(node, optionalNode) } -> SameType<dom::ExceptionOr<void>>;

    { node.QuerySelector(selectors) } -> SameType<dom::ExceptionOr<RefPtr<Element>>>;
    { node.QuerySelectorAll(selectors) } -> SameType<dom::ExceptionOr<Ref<NodeList>>>;
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
    { node.Before(nodes) } -> SameType<dom::ExceptionOr<void>>;
    { node.After(nodes) } -> SameType<dom::ExceptionOr<void>>;
    { node.ReplaceWith(nodes) } -> SameType<dom::ExceptionOr<void>>;
    { node.Remove() } -> SameType<dom::ExceptionOr<void>>;
  };

  TEST_CASE("Mixin::ChildNode", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(ChildNodeMixin<DocumentType>);
    STATIC_REQUIRE(ChildNodeMixin<Element>);
    STATIC_REQUIRE(ChildNodeMixin<CharacterData>);
  }

  template <typename T>
  concept SlottableMixin = requires(T &node) {
    { node.AssignedSlot() } -> SameType<RefPtr<HTMLSlotElement>>;
  };

  TEST_CASE("Mixin::Slottable", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(SlottableMixin<Element>);
    STATIC_REQUIRE(SlottableMixin<krys::boo::Text>);
  }
}