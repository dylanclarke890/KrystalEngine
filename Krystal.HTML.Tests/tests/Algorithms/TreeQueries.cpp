#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

#pragma region Node Trees

  TEST_CASE("TreeQueries::Length", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);
    auto textNode = CreateRef<HTML::Text>(*doc, u8"Hello, world!");

    REQUIRE(TreeQueries::Length(*parent) == 0);
    REQUIRE(TreeQueries::Length(*child) == 0);
    REQUIRE(TreeQueries::Length(*textNode) == 13);

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(TreeQueries::Length(*parent) == 1);
    REQUIRE(TreeQueries::Length(*child) == 0);

    REQUIRE_FALSE(child->AppendChild(*textNode).HasException());

    REQUIRE(TreeQueries::Length(*parent) == 1);
    REQUIRE(TreeQueries::Length(*child) == 1);
    REQUIRE(TreeQueries::Length(*textNode) == 13);

    REQUIRE_FALSE(child->RemoveChild(*textNode).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
  }

  TEST_CASE("TreeQueries::IsEmpty", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);
    auto textNode = CreateRef<HTML::Text>(*doc, u8"Hello, world!");

    REQUIRE(TreeQueries::IsEmpty(*parent));
    REQUIRE(TreeQueries::IsEmpty(*child));
    REQUIRE_FALSE(TreeQueries::IsEmpty(*textNode));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE_FALSE(TreeQueries::IsEmpty(*parent));
    REQUIRE(TreeQueries::IsEmpty(*child));

    REQUIRE_FALSE(child->AppendChild(*textNode).HasException());

    REQUIRE_FALSE(TreeQueries::IsEmpty(*parent));
    REQUIRE_FALSE(TreeQueries::IsEmpty(*child));
    REQUIRE_FALSE(TreeQueries::IsEmpty(*textNode));

    REQUIRE_FALSE(child->RemoveChild(*textNode).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
  }

#pragma endregion

#pragma region Document Trees

  TEST_CASE("TreeQueries::IsInDocumentTree", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsInDocumentTree(*parent));
    REQUIRE_FALSE(TreeQueries::IsInDocumentTree(*child));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE_FALSE(TreeQueries::IsInDocumentTree(*parent));
    REQUIRE_FALSE(TreeQueries::IsInDocumentTree(*child));

    REQUIRE_FALSE(doc->AppendChild(*parent).HasException());

    REQUIRE(TreeQueries::IsInDocumentTree(*parent));
    REQUIRE(TreeQueries::IsInDocumentTree(*child));

    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
    REQUIRE_FALSE(doc->RemoveChild(*parent).HasException());
  }

  TEST_CASE("TreeQueries::DocumentElement", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE(TreeQueries::DocumentElement(*doc) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());
    REQUIRE(TreeQueries::DocumentElement(*doc) == nullptr);

    REQUIRE_FALSE(doc->AppendChild(*parent).HasException());
    REQUIRE(TreeQueries::DocumentElement(*doc) == parent.get());

    REQUIRE_FALSE(child->Remove().HasException());
    REQUIRE_FALSE(parent->Remove().HasException());
  }

#pragma endregion

#pragma region Shadow Trees

  TEST_CASE("TreeQueries::IsInShadowTree", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto docChild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsInShadowTree(*docChild));

    REQUIRE_FALSE(doc->AppendChild(*docChild).HasException());

    REQUIRE_FALSE(TreeQueries::IsInShadowTree(*docChild));

    auto shadowRoot = CreateRef<ShadowRoot>(*doc, nullptr);
    auto shadowChild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsInShadowTree(*shadowChild));

    REQUIRE_FALSE(shadowRoot->AppendChild(*shadowChild).HasException());

    REQUIRE(TreeQueries::IsInShadowTree(*shadowChild));

    REQUIRE_FALSE(doc->RemoveChild(*docChild).HasException());
    REQUIRE_FALSE(shadowRoot->RemoveChild(*shadowChild).HasException());
  }

#pragma endregion

#pragma region Trees

  TEST_CASE("TreeQueries::IsParent", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsParent(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsParent(*child, *parent));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(TreeQueries::IsParent(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsParent(*child, *parent));

    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
  }

  TEST_CASE("TreeQueries::IsChild", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsChild(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsChild(*child, *parent));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE_FALSE(TreeQueries::IsChild(*parent, *child));
    REQUIRE(TreeQueries::IsChild(*child, *parent));

    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
  }

  TEST_CASE("TreeQueries::Root", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE(&TreeQueries::Root(*doc) == doc.get());
    REQUIRE(&TreeQueries::Root(*parent) == parent.get());
    REQUIRE(&TreeQueries::Root(*child) == child.get());

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(&TreeQueries::Root(*doc) == doc.get());
    REQUIRE(&TreeQueries::Root(*parent) == parent.get());
    REQUIRE(&TreeQueries::Root(*child) == parent.get());

    REQUIRE_FALSE(doc->AppendChild(*parent).HasException());

    REQUIRE(&TreeQueries::Root(*doc) == doc.get());
    REQUIRE(&TreeQueries::Root(*parent) == doc.get());
    REQUIRE(&TreeQueries::Root(*child) == doc.get());

    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
    REQUIRE_FALSE(doc->RemoveChild(*parent).HasException());
  }

  TEST_CASE("TreeQueries::IsDescendant", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);
    auto grandchild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsDescendant(*child, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *child));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(TreeQueries::IsDescendant(*child, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *child));

    REQUIRE_FALSE(child->AppendChild(*grandchild).HasException());

    REQUIRE(TreeQueries::IsDescendant(*grandchild, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *grandchild));

    REQUIRE(TreeQueries::IsDescendant(*grandchild, *child));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*child, *grandchild));

    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*child, *child));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*grandchild, *grandchild));

    REQUIRE_FALSE(child->RemoveChild(*grandchild).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());

    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*child, *child));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*grandchild, *grandchild));
  }

  TEST_CASE("TreeQueries::IsInclusiveDescendant", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);
    auto grandchild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsInclusiveDescendant(*child, *parent));
    REQUIRE_FALSE(TreeQueries::IsInclusiveDescendant(*parent, *child));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(TreeQueries::IsInclusiveDescendant(*child, *parent));
    REQUIRE_FALSE(TreeQueries::IsInclusiveDescendant(*parent, *child));

    REQUIRE_FALSE(child->AppendChild(*grandchild).HasException());

    REQUIRE(TreeQueries::IsInclusiveDescendant(*grandchild, *parent));
    REQUIRE_FALSE(TreeQueries::IsInclusiveDescendant(*parent, *grandchild));

    REQUIRE(TreeQueries::IsInclusiveDescendant(*grandchild, *child));
    REQUIRE_FALSE(TreeQueries::IsInclusiveDescendant(*child, *grandchild));

    REQUIRE(TreeQueries::IsInclusiveDescendant(*child, *child));
    REQUIRE(TreeQueries::IsInclusiveDescendant(*grandchild, *grandchild));
    REQUIRE(TreeQueries::IsInclusiveDescendant(*parent, *parent));

    REQUIRE_FALSE(child->RemoveChild(*grandchild).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());

    REQUIRE(TreeQueries::IsInclusiveDescendant(*parent, *parent));
    REQUIRE(TreeQueries::IsInclusiveDescendant(*child, *child));
    REQUIRE(TreeQueries::IsInclusiveDescendant(*grandchild, *grandchild));
  }

  TEST_CASE("TreeQueries::IsAncestor", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);
    auto grandchild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsAncestor(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*child, *parent));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(TreeQueries::IsAncestor(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*child, *parent));

    REQUIRE_FALSE(child->AppendChild(*grandchild).HasException());

    REQUIRE(TreeQueries::IsAncestor(*parent, *grandchild));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*grandchild, *parent));

    REQUIRE(TreeQueries::IsAncestor(*child, *grandchild));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*grandchild, *child));

    REQUIRE_FALSE(TreeQueries::IsAncestor(*parent, *parent));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*child, *child));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*grandchild, *grandchild));

    REQUIRE_FALSE(child->RemoveChild(*grandchild).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());

    REQUIRE_FALSE(TreeQueries::IsAncestor(*parent, *parent));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*child, *child));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*grandchild, *grandchild));
  }

  TEST_CASE("TreeQueries::IsInclusiveAncestor", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);
    auto grandchild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsInclusiveAncestor(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsInclusiveAncestor(*child, *parent));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(TreeQueries::IsInclusiveAncestor(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsInclusiveAncestor(*child, *parent));

    REQUIRE_FALSE(child->AppendChild(*grandchild).HasException());

    REQUIRE(TreeQueries::IsInclusiveAncestor(*parent, *grandchild));
    REQUIRE_FALSE(TreeQueries::IsInclusiveAncestor(*grandchild, *parent));

    REQUIRE(TreeQueries::IsInclusiveAncestor(*child, *grandchild));
    REQUIRE_FALSE(TreeQueries::IsInclusiveAncestor(*grandchild, *child));

    REQUIRE(TreeQueries::IsInclusiveAncestor(*parent, *parent));
    REQUIRE(TreeQueries::IsInclusiveAncestor(*child, *child));
    REQUIRE(TreeQueries::IsInclusiveAncestor(*grandchild, *grandchild));

    REQUIRE_FALSE(child->RemoveChild(*grandchild).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());

    REQUIRE(TreeQueries::IsInclusiveAncestor(*parent, *parent));
    REQUIRE(TreeQueries::IsInclusiveAncestor(*child, *child));
    REQUIRE(TreeQueries::IsInclusiveAncestor(*grandchild, *grandchild));
  }

  TEST_CASE("TreeQueries::IsSibling", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto childA = CreateRef<TestNode>(*doc);
    auto childB = CreateRef<TestNode>(*doc);
    auto childC = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsSibling(*childA, *childB));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childB, *childA));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childA, *childC));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childC, *childA));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childB, *childC));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childC, *childB));

    REQUIRE_FALSE(parent->AppendChild(*childA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childB).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childC).HasException());

    REQUIRE(TreeQueries::IsSibling(*childA, *childB));
    REQUIRE(TreeQueries::IsSibling(*childB, *childA));
    REQUIRE(TreeQueries::IsSibling(*childA, *childC));
    REQUIRE(TreeQueries::IsSibling(*childC, *childA));
    REQUIRE(TreeQueries::IsSibling(*childB, *childC));
    REQUIRE(TreeQueries::IsSibling(*childC, *childB));

    REQUIRE_FALSE(TreeQueries::IsSibling(*childA, *childA));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childB, *childB));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childC, *childC));

    REQUIRE_FALSE(parent->RemoveChild(*childC).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childB).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childA).HasException());

    REQUIRE_FALSE(TreeQueries::IsSibling(*childA, *childA));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childB, *childB));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childC, *childC));
  }

  TEST_CASE("TreeQueries::IsInclusiveSibling", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto childA = CreateRef<TestNode>(*doc);
    auto childB = CreateRef<TestNode>(*doc);
    auto childC = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsInclusiveSibling(*childA, *childB));
    REQUIRE_FALSE(TreeQueries::IsInclusiveSibling(*childB, *childA));
    REQUIRE_FALSE(TreeQueries::IsInclusiveSibling(*childA, *childC));
    REQUIRE_FALSE(TreeQueries::IsInclusiveSibling(*childC, *childA));
    REQUIRE_FALSE(TreeQueries::IsInclusiveSibling(*childB, *childC));
    REQUIRE_FALSE(TreeQueries::IsInclusiveSibling(*childC, *childB));

    REQUIRE_FALSE(parent->AppendChild(*childA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childB).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childC).HasException());

    REQUIRE(TreeQueries::IsInclusiveSibling(*childA, *childB));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childB, *childA));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childA, *childC));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childC, *childA));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childB, *childC));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childC, *childB));

    REQUIRE(TreeQueries::IsInclusiveSibling(*childA, *childA));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childB, *childB));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childC, *childC));

    REQUIRE_FALSE(parent->RemoveChild(*childA).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childB).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childC).HasException());

    REQUIRE(TreeQueries::IsInclusiveSibling(*childA, *childA));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childB, *childB));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childC, *childC));
  }

  TEST_CASE("TreeQueries::IsPreceding", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto childA = CreateRef<TestNode>(*doc);
    auto childB = CreateRef<TestNode>(*doc);
    auto childC = CreateRef<TestNode>(*doc);
    auto grandchildA = CreateRef<TestNode>(*doc);
    auto grandchildB = CreateRef<TestNode>(*doc);
    auto grandchildC = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsPreceding(*childA, *childB));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childB, *childA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childA, *childC));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childC, *childA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childB, *childC));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childC, *childB));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childA, *grandchildA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*grandchildA, *childA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childB, *grandchildB));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*grandchildB, *childB));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childC, *grandchildC));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*grandchildC, *childC));

    REQUIRE_FALSE(parent->AppendChild(*childA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childB).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childC).HasException());
    REQUIRE_FALSE(childA->AppendChild(*grandchildA).HasException());
    REQUIRE_FALSE(childB->AppendChild(*grandchildB).HasException());
    REQUIRE_FALSE(childC->AppendChild(*grandchildC).HasException());

    REQUIRE(TreeQueries::IsPreceding(*childA, *childB));
    REQUIRE(TreeQueries::IsPreceding(*childA, *childC));
    REQUIRE(TreeQueries::IsPreceding(*childB, *childC));
    REQUIRE(TreeQueries::IsPreceding(*childA, *grandchildA));
    REQUIRE(TreeQueries::IsPreceding(*childA, *grandchildB));
    REQUIRE(TreeQueries::IsPreceding(*childA, *grandchildC));
    REQUIRE(TreeQueries::IsPreceding(*childB, *grandchildB));
    REQUIRE(TreeQueries::IsPreceding(*childB, *grandchildC));
    REQUIRE(TreeQueries::IsPreceding(*childC, *grandchildC));

    REQUIRE_FALSE(TreeQueries::IsPreceding(*childB, *childA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childC, *childA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childC, *childB));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*grandchildA, *childA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*grandchildB, *childB));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*grandchildC, *childC));

    REQUIRE_FALSE(parent->RemoveChild(*childA).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childB).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childC).HasException());
    REQUIRE_FALSE(childA->RemoveChild(*grandchildA).HasException());
    REQUIRE_FALSE(childB->RemoveChild(*grandchildB).HasException());
    REQUIRE_FALSE(childC->RemoveChild(*grandchildC).HasException());
  }

  TEST_CASE("TreeQueries::IsFollowing", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto childA = CreateRef<TestNode>(*doc);
    auto childB = CreateRef<TestNode>(*doc);
    auto childC = CreateRef<TestNode>(*doc);
    auto grandchildA = CreateRef<TestNode>(*doc);
    auto grandchildB = CreateRef<TestNode>(*doc);
    auto grandchildC = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *childB));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childB, *childA));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *childC));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childC, *childA));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childB, *childC));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childC, *childB));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *grandchildA));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*grandchildA, *childA));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childB, *grandchildB));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*grandchildB, *childB));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childC, *grandchildC));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*grandchildC, *childC));

    REQUIRE_FALSE(parent->AppendChild(*childA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childB).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childC).HasException());
    REQUIRE_FALSE(childA->AppendChild(*grandchildA).HasException());
    REQUIRE_FALSE(childB->AppendChild(*grandchildB).HasException());
    REQUIRE_FALSE(childC->AppendChild(*grandchildC).HasException());

    REQUIRE(TreeQueries::IsFollowing(*childB, *childA));
    REQUIRE(TreeQueries::IsFollowing(*childC, *childA));
    REQUIRE(TreeQueries::IsFollowing(*childC, *childB));
    REQUIRE(TreeQueries::IsFollowing(*grandchildA, *childA));
    REQUIRE(TreeQueries::IsFollowing(*grandchildB, *childB));
    REQUIRE(TreeQueries::IsFollowing(*grandchildC, *childC));

    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *childB));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *childC));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childB, *childC));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *grandchildA));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *grandchildB));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *grandchildC));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childB, *grandchildB));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childB, *grandchildC));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childC, *grandchildC));

    REQUIRE_FALSE(childC->RemoveChild(*grandchildC).HasException());
    REQUIRE_FALSE(childB->RemoveChild(*grandchildB).HasException());
    REQUIRE_FALSE(childA->RemoveChild(*grandchildA).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childC).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childB).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childA).HasException());
  }

  TEST_CASE("TreeQueries::Index", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto childA = CreateRef<TestNode>(*doc);
    auto childB = CreateRef<TestNode>(*doc);
    auto childC = CreateRef<TestNode>(*doc);
    auto grandchildA = CreateRef<TestNode>(*doc);
    auto grandchildB = CreateRef<TestNode>(*doc);
    auto grandchildC = CreateRef<TestNode>(*doc);

    REQUIRE(TreeQueries::Index(*doc) == 0);
    REQUIRE(TreeQueries::Index(*parent) == 0);
    REQUIRE(TreeQueries::Index(*childA) == 0);
    REQUIRE(TreeQueries::Index(*childB) == 0);
    REQUIRE(TreeQueries::Index(*childC) == 0);
    REQUIRE(TreeQueries::Index(*grandchildA) == 0);
    REQUIRE(TreeQueries::Index(*grandchildB) == 0);
    REQUIRE(TreeQueries::Index(*grandchildC) == 0);

    REQUIRE_FALSE(parent->AppendChild(*childA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childB).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childC).HasException());
    REQUIRE_FALSE(childA->AppendChild(*grandchildA).HasException());
    REQUIRE_FALSE(childB->AppendChild(*grandchildB).HasException());
    REQUIRE_FALSE(childC->AppendChild(*grandchildC).HasException());

    REQUIRE(TreeQueries::Index(*doc) == 0);
    REQUIRE(TreeQueries::Index(*parent) == 0);
    REQUIRE(TreeQueries::Index(*childA) == 0);
    REQUIRE(TreeQueries::Index(*childB) == 1);
    REQUIRE(TreeQueries::Index(*childC) == 2);
    REQUIRE(TreeQueries::Index(*grandchildA) == 0);
    REQUIRE(TreeQueries::Index(*grandchildB) == 0);
    REQUIRE(TreeQueries::Index(*grandchildC) == 0);

    REQUIRE_FALSE(childC->RemoveChild(*grandchildC).HasException());
    REQUIRE_FALSE(childB->RemoveChild(*grandchildB).HasException());
    REQUIRE_FALSE(childA->RemoveChild(*grandchildA).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childC).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childB).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childA).HasException());

    REQUIRE(TreeQueries::Index(*doc) == 0);
    REQUIRE(TreeQueries::Index(*parent) == 0);
    REQUIRE(TreeQueries::Index(*childA) == 0);
    REQUIRE(TreeQueries::Index(*childB) == 0);
    REQUIRE(TreeQueries::Index(*childC) == 0);
    REQUIRE(TreeQueries::Index(*grandchildA) == 0);
    REQUIRE(TreeQueries::Index(*grandchildB) == 0);
    REQUIRE(TreeQueries::Index(*grandchildC) == 0);
  }

#pragma endregion
}