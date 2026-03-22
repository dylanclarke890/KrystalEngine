#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  template <typename T>
  concept NonElementParentNodeMixin = requires(T &t, DOMStringAtom elementId) {
    { t.GetElementById(elementId) } -> SameType<RefPtr<Element>>;
  };

  TEST_CASE("NonElementParentNode", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(NonElementParentNodeMixin<Document>);
    STATIC_REQUIRE(NonElementParentNodeMixin<DocumentFragment>);
  }

  template <typename T>
  concept DocumentOrShadowRootMixin = requires(T &t) {
    { t.CustomElementRegistry() } -> SameType<RefPtr<CustomElementRegistry>>;
  };

  TEST_CASE("DocumentOrShadowRoot", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(DocumentOrShadowRootMixin<Document>);
    STATIC_REQUIRE(DocumentOrShadowRootMixin<ShadowRoot>);
  }

  template <typename T>
  concept ParentNodeMixin = requires(T &node, const T &constNode, const List<NodeOrString> &nodes,
                                     RawPtr<Node> optionalNode, const DOMString &str) {
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

    { node.QuerySelector(str) } -> SameType<ExceptionOr<RefPtr<Element>>>;
    { node.QuerySelectorAll(str) } -> SameType<ExceptionOr<Ref<NodeList>>>;
  };

  TEST_CASE("ParentNode", "[HTML][Mixins]")
  {
    STATIC_REQUIRE(ParentNodeMixin<Element>);
    STATIC_REQUIRE(ParentNodeMixin<Document>);
    STATIC_REQUIRE(ParentNodeMixin<DocumentFragment>);
  }
}