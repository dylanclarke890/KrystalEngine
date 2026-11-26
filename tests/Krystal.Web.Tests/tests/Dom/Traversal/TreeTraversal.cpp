#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Web.Tests/Dom/TestElement.hpp"
#include "Krystal.Web.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Web/Dom/Events/EventTargetAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/CDATASection.hpp"
#include "Krystal.Web/Dom/Nodes/Comment.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Web/Dom/Nodes/DOMImplementation.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Dom/Nodes/PseudoElement.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Nodes/XMLDocument.hpp"
#include "Krystal.Web/Html/Elements/HTMLTemplateElement.hpp"

namespace krys::web::dom::tests
{
  TEST_CASE("TreeTraversal::GetShadowRoot", "[HTML][TreeTraversal]")
  {
    auto doc = CreateRef<HTMLDocument>();
    auto host = doc->CreateElement(u8"div").ReleaseValue();
    REQUIRE(TreeTraversal::GetShadowRoot(*host) == nullptr);

    auto mode = GENERATE(ShadowRootMode::Open, ShadowRootMode::Closed);
    auto shadow = host->AttachShadow({.Mode = mode}).ReleaseValue();
    REQUIRE(shadow->Mode() == mode);
    REQUIRE(TreeTraversal::GetShadowRoot(*host) == shadow.get());
    // The web-exposed getter hides closed shadow roots.
    REQUIRE((host->ShadowRoot().get() == shadow.get()) == (mode == ShadowRootMode::Open));
  }

  TEST_CASE("TreeTraversal::ParentOrShadowHostElement", "[HTML][TreeTraversal]")
  {
    auto doc = CreateRef<HTMLDocument>();
    auto host = doc->CreateElement(u8"div").ReleaseValue();
    auto child = doc->CreateElement(u8"span").ReleaseValue();
    auto inner = doc->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(doc->AppendChild(*host).HasException());
    REQUIRE_FALSE(host->AppendChild(*child).HasException());
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Closed}).ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

    REQUIRE(TreeTraversal::ParentOrShadowHostElement(*child) == host.get());
    REQUIRE(TreeTraversal::ParentOrShadowHostElement(*inner) == host.get());
    REQUIRE(TreeTraversal::ParentOrShadowHostElement(*host) == nullptr);
    REQUIRE(TreeTraversal::ParentOrShadowHostElement(*shadow) == nullptr);
  }

  TEST_CASE("TreeTraversal: the shadow boundary helpers", "[DOM][TreeTraversal]")
  {
    auto doc = CreateRef<HTMLDocument>();
    auto host = doc->CreateElement(u8"div").ReleaseValue();
    auto inner = doc->CreateElement(u8"span").ReleaseValue();
    auto innermost = doc->CreateElement(u8"b").ReleaseValue();
    REQUIRE_FALSE(doc->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Closed}).ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
    REQUIRE_FALSE(inner->AppendChild(*innermost).HasException());

    SECTION("ParentOrShadowHostNode: a shadow root's host, otherwise the parent")
    {
      CHECK(TreeTraversal::ParentOrShadowHostNode(*shadow) == host.get());
      CHECK(TreeTraversal::ParentOrShadowHostNode(*inner) == shadow.get());
      CHECK(TreeTraversal::ParentOrShadowHostNode(*host) == doc.get());
      CHECK(TreeTraversal::ParentOrShadowHostNode(*doc) == nullptr);
    }

    SECTION("NonShadowBoundaryParentNode: the parent, unless it's a shadow root")
    {
      CHECK(TreeTraversal::NonShadowBoundaryParentNode(*innermost) == inner.get());
      CHECK(TreeTraversal::NonShadowBoundaryParentNode(*inner) == nullptr);
      CHECK(TreeTraversal::NonShadowBoundaryParentNode(*host) == doc.get());
    }

    SECTION("NonBoundaryShadowTreeRootNode: the ancestor that's a shadow root's child")
    {
      CHECK(TreeTraversal::NonBoundaryShadowTreeRootNode(*innermost) == inner.get());
      CHECK(TreeTraversal::NonBoundaryShadowTreeRootNode(*inner) == inner.get());
      CHECK(TreeTraversal::NonBoundaryShadowTreeRootNode(*host) == nullptr);
    }
  }

  TEST_CASE("TreeTraversal: ChildAt and DeepLastChild on any node", "[DOM][TreeTraversal]")
  {
    auto doc = CreateRef<HTMLDocument>();
    auto parent = doc->CreateElement(u8"div").ReleaseValue();
    auto first = doc->CreateElement(u8"span").ReleaseValue();
    auto last = doc->CreateElement(u8"span").ReleaseValue();
    auto text = doc->CreateTextNode(u8"text");
    REQUIRE_FALSE(parent->AppendChild(*first).HasException());
    REQUIRE_FALSE(parent->AppendChild(*last).HasException());
    REQUIRE_FALSE(last->AppendChild(*text).HasException());

    Node &node = *parent;
    const Node &constNode = *parent;
    CHECK(TreeTraversal::ChildAt(node, 0) == first.get());
    CHECK(TreeTraversal::ChildAt(constNode, 1) == last.get());
    CHECK(TreeTraversal::ChildAt(node, 2) == nullptr);
    // Nodes that can't have children have none at any index.
    CHECK(TreeTraversal::ChildAt(static_cast<Node &>(*text), 0) == nullptr);

    CHECK(TreeTraversal::DeepLastChild(node) == text.get());
    CHECK(TreeTraversal::DeepLastChild(constNode) == text.get());
    CHECK(TreeTraversal::DeepLastChild(static_cast<Node &>(*text)) == text.get());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TreeTraversal: traversal including pseudo-elements",
                   "[DOM][TreeTraversal]")
  {
    style::tests::ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #p::before, #p::after, #empty::before { content: "x" }
    </style><div id=root><p id=p><b id=b></b></p><i id=empty></i><s id=s></s></div>)"};
    fixture.Document->UpdateStyleIfNeeded();

    auto &root = fixture.ById(u8"root");
    auto &p = fixture.ById(u8"p");
    auto &b = fixture.ById(u8"b");
    auto &empty = fixture.ById(u8"empty");
    auto *before = p.BeforePseudoElement();
    auto *after = p.AfterPseudoElement();
    auto *emptyBefore = empty.BeforePseudoElement();
    REQUIRE(before);
    REQUIRE(after);
    REQUIRE(emptyBefore);

    CHECK(TreeTraversal::PseudoAwareFirstChild(p) == before);
    CHECK(TreeTraversal::PseudoAwareLastChild(p) == after);
    CHECK(TreeTraversal::PseudoAwareNextSibling(*before) == &b);
    CHECK(TreeTraversal::PseudoAwareNextSibling(b) == after);
    CHECK(TreeTraversal::PseudoAwarePreviousSibling(b) == before);
    CHECK(TreeTraversal::PseudoAwarePreviousSibling(*after) == &b);
    CHECK_FALSE(TreeTraversal::PseudoAwarePreviousSibling(*before));
    // An element with only a ::before.
    CHECK(TreeTraversal::PseudoAwareFirstChild(empty) == emptyBefore);
    CHECK(TreeTraversal::PseudoAwareLastChild(empty) == emptyBefore);

    // Tree order: root, p, ::before, b, ::after, empty, ::before, s.
    List<Element *> order;
    for (Element *element = &root; element;
         element = TreeTraversal::NextElementIncludingPseudo(*element, &root))
    {
      order.push_back(element);
    }
    CHECK(order == List<Element *> {&root, &p, before, &b, after, &empty, emptyBefore, &fixture.ById(u8"s")});

    List<Element *> reversed;
    for (Element *element = &fixture.ById(u8"s"); element;
         element = TreeTraversal::PreviousElementIncludingPseudo(*element, &root))
    {
      reversed.push_back(element);
    }
    CHECK(reversed
          == List<Element *> {&fixture.ById(u8"s"), emptyBefore, &empty, after, &b, before, &p, &root});

    CHECK(TreeTraversal::NextElementIncludingPseudoSkippingChildren(p, &root) == &empty);
    CHECK(TreeTraversal::PseudoAwarePreviousElementSibling(empty) == &p);
  }
}
