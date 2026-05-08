#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
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

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::Length(*parent) == 1);
    REQUIRE(TreeQueries::Length(*child) == 0);

    appendResult = child->AppendChild(*textNode);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::Length(*parent) == 1);
    REQUIRE(TreeQueries::Length(*child) == 1);
    REQUIRE(TreeQueries::Length(*textNode) == 13);

    child->RemoveChild(*textNode);
    parent->RemoveChild(*child);
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

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE_FALSE(TreeQueries::IsEmpty(*parent));
    REQUIRE(TreeQueries::IsEmpty(*child));

    appendResult = child->AppendChild(*textNode);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE_FALSE(TreeQueries::IsEmpty(*parent));
    REQUIRE_FALSE(TreeQueries::IsEmpty(*child));
    REQUIRE_FALSE(TreeQueries::IsEmpty(*textNode));

    child->RemoveChild(*textNode);
    parent->RemoveChild(*child);
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

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE_FALSE(TreeQueries::IsInDocumentTree(*parent));
    REQUIRE_FALSE(TreeQueries::IsInDocumentTree(*child));

    appendResult = doc->AppendChild(*parent);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsInDocumentTree(*parent));
    REQUIRE(TreeQueries::IsInDocumentTree(*child));

    auto removeResult = parent->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = doc->RemoveChild(*parent);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("TreeQueries::DocumentElement", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE(TreeQueries::DocumentElement(*doc) == nullptr);
    REQUIRE(TreeQueries::DocumentElement(*parent) == nullptr);
    REQUIRE(TreeQueries::DocumentElement(*child) == nullptr);

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::DocumentElement(*doc) == nullptr);
    REQUIRE(TreeQueries::DocumentElement(*parent) == nullptr);
    REQUIRE(TreeQueries::DocumentElement(*child) == nullptr);

    appendResult = doc->AppendChild(*parent);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::DocumentElement(*doc) == parent.get());
    REQUIRE(TreeQueries::DocumentElement(*parent) == parent.get());
    REQUIRE(TreeQueries::DocumentElement(*child) == parent.get());

    auto removeResult = parent->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = doc->RemoveChild(*parent);
    REQUIRE_FALSE(removeResult.HasException());
  }

#pragma endregion

#pragma region Shadow Trees

  TEST_CASE("TreeQueries::IsInShadowTree", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto docChild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsInShadowTree(*docChild));

    auto appendResult = doc->AppendChild(*docChild);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE_FALSE(TreeQueries::IsInShadowTree(*docChild));

    auto shadowRoot = CreateRef<ShadowRoot>(*doc, nullptr);
    auto shadowChild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsInShadowTree(*shadowChild));

    appendResult = shadowRoot->AppendChild(*shadowChild);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsInShadowTree(*shadowChild));

    auto removeResult = doc->RemoveChild(*docChild);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = shadowRoot->RemoveChild(*shadowChild);
    REQUIRE_FALSE(removeResult.HasException());
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

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsParent(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsParent(*child, *parent));

    parent->RemoveChild(*child);
  }

  TEST_CASE("TreeQueries::IsChild", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsChild(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsChild(*child, *parent));

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE_FALSE(TreeQueries::IsChild(*parent, *child));
    REQUIRE(TreeQueries::IsChild(*child, *parent));

    parent->RemoveChild(*child);
  }

  TEST_CASE("TreeQueries::Root", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE(&TreeQueries::Root(*doc) == doc.get());
    REQUIRE(&TreeQueries::Root(*parent) == parent.get());
    REQUIRE(&TreeQueries::Root(*child) == child.get());

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&TreeQueries::Root(*doc) == doc.get());
    REQUIRE(&TreeQueries::Root(*parent) == parent.get());
    REQUIRE(&TreeQueries::Root(*child) == parent.get());

    appendResult = doc->AppendChild(*parent);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&TreeQueries::Root(*doc) == doc.get());
    REQUIRE(&TreeQueries::Root(*parent) == doc.get());
    REQUIRE(&TreeQueries::Root(*child) == doc.get());

    auto removeResult = parent->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = doc->RemoveChild(*parent);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("TreeQueries::IsDescendant", "[TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);
    auto grandchild = CreateRef<TestNode>(*doc);

    REQUIRE_FALSE(TreeQueries::IsDescendant(*child, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *child));

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsDescendant(*child, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *child));

    appendResult = child->AppendChild(*grandchild);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsDescendant(*grandchild, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *grandchild));

    REQUIRE(TreeQueries::IsDescendant(*grandchild, *child));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*child, *grandchild));

    REQUIRE_FALSE(TreeQueries::IsDescendant(*parent, *parent));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*child, *child));
    REQUIRE_FALSE(TreeQueries::IsDescendant(*grandchild, *grandchild));

    auto removeResult = parent->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());

    removeResult = child->RemoveChild(*grandchild);
    REQUIRE_FALSE(removeResult.HasException());

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

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsInclusiveDescendant(*child, *parent));
    REQUIRE_FALSE(TreeQueries::IsInclusiveDescendant(*parent, *child));

    appendResult = child->AppendChild(*grandchild);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsInclusiveDescendant(*grandchild, *parent));
    REQUIRE_FALSE(TreeQueries::IsInclusiveDescendant(*parent, *grandchild));

    REQUIRE(TreeQueries::IsInclusiveDescendant(*grandchild, *child));
    REQUIRE_FALSE(TreeQueries::IsInclusiveDescendant(*child, *grandchild));

    REQUIRE(TreeQueries::IsInclusiveDescendant(*child, *child));
    REQUIRE(TreeQueries::IsInclusiveDescendant(*grandchild, *grandchild));
    REQUIRE(TreeQueries::IsInclusiveDescendant(*parent, *parent));

    auto removeResult = parent->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());

    removeResult = child->RemoveChild(*grandchild);
    REQUIRE_FALSE(removeResult.HasException());

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

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsAncestor(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*child, *parent));

    appendResult = child->AppendChild(*grandchild);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsAncestor(*parent, *grandchild));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*grandchild, *parent));

    REQUIRE(TreeQueries::IsAncestor(*child, *grandchild));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*grandchild, *child));

    REQUIRE_FALSE(TreeQueries::IsAncestor(*parent, *parent));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*child, *child));
    REQUIRE_FALSE(TreeQueries::IsAncestor(*grandchild, *grandchild));

    auto removeResult = parent->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());

    removeResult = child->RemoveChild(*grandchild);
    REQUIRE_FALSE(removeResult.HasException());

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

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsInclusiveAncestor(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsInclusiveAncestor(*child, *parent));

    appendResult = child->AppendChild(*grandchild);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsInclusiveAncestor(*parent, *grandchild));
    REQUIRE_FALSE(TreeQueries::IsInclusiveAncestor(*grandchild, *parent));

    REQUIRE(TreeQueries::IsInclusiveAncestor(*child, *grandchild));
    REQUIRE_FALSE(TreeQueries::IsInclusiveAncestor(*grandchild, *child));

    REQUIRE(TreeQueries::IsInclusiveAncestor(*parent, *parent));
    REQUIRE(TreeQueries::IsInclusiveAncestor(*child, *child));
    REQUIRE(TreeQueries::IsInclusiveAncestor(*grandchild, *grandchild));

    auto removeResult = parent->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());

    removeResult = child->RemoveChild(*grandchild);
    REQUIRE_FALSE(removeResult.HasException());

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

    auto appendResult = parent->AppendChild(*childA);
    REQUIRE_FALSE(appendResult.HasException());

    appendResult = parent->AppendChild(*childB);
    REQUIRE_FALSE(appendResult.HasException());

    appendResult = parent->AppendChild(*childC);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsSibling(*childA, *childB));
    REQUIRE(TreeQueries::IsSibling(*childB, *childA));
    REQUIRE(TreeQueries::IsSibling(*childA, *childC));
    REQUIRE(TreeQueries::IsSibling(*childC, *childA));
    REQUIRE(TreeQueries::IsSibling(*childB, *childC));
    REQUIRE(TreeQueries::IsSibling(*childC, *childB));

    REQUIRE_FALSE(TreeQueries::IsSibling(*childA, *childA));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childB, *childB));
    REQUIRE_FALSE(TreeQueries::IsSibling(*childC, *childC));

    auto removeResult = parent->RemoveChild(*childA);
    REQUIRE_FALSE(removeResult.HasException());

    removeResult = parent->RemoveChild(*childB);
    REQUIRE_FALSE(removeResult.HasException());

    removeResult = parent->RemoveChild(*childC);
    REQUIRE_FALSE(removeResult.HasException());

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

    auto appendResult = parent->AppendChild(*childA);
    REQUIRE_FALSE(appendResult.HasException());

    appendResult = parent->AppendChild(*childB);
    REQUIRE_FALSE(appendResult.HasException());

    appendResult = parent->AppendChild(*childC);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::IsInclusiveSibling(*childA, *childB));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childB, *childA));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childA, *childC));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childC, *childA));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childB, *childC));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childC, *childB));

    REQUIRE(TreeQueries::IsInclusiveSibling(*childA, *childA));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childB, *childB));
    REQUIRE(TreeQueries::IsInclusiveSibling(*childC, *childC));

    auto removeResult = parent->RemoveChild(*childA);
    REQUIRE_FALSE(removeResult.HasException());

    removeResult = parent->RemoveChild(*childB);
    REQUIRE_FALSE(removeResult.HasException());

    removeResult = parent->RemoveChild(*childC);
    REQUIRE_FALSE(removeResult.HasException());

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

    auto appendResult = parent->AppendChild(*childA);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = parent->AppendChild(*childB);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = parent->AppendChild(*childC);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childA->AppendChild(*grandchildA);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childB->AppendChild(*grandchildB);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childC->AppendChild(*grandchildC);
    REQUIRE_FALSE(appendResult.HasException());

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

    auto removeResult = parent->RemoveChild(*childA);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = parent->RemoveChild(*childB);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = parent->RemoveChild(*childC);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childA->RemoveChild(*grandchildA);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childB->RemoveChild(*grandchildB);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childC->RemoveChild(*grandchildC);
    REQUIRE_FALSE(removeResult.HasException());
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

    auto appendResult = parent->AppendChild(*childA);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = parent->AppendChild(*childB);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = parent->AppendChild(*childC);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childA->AppendChild(*grandchildA);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childB->AppendChild(*grandchildB);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childC->AppendChild(*grandchildC);
    REQUIRE_FALSE(appendResult.HasException());

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

    auto removeResult = parent->RemoveChild(*childA);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = parent->RemoveChild(*childB);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = parent->RemoveChild(*childC);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childA->RemoveChild(*grandchildA);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childB->RemoveChild(*grandchildB);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childC->RemoveChild(*grandchildC);
    REQUIRE_FALSE(removeResult.HasException());
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

    auto appendResult = parent->AppendChild(*childA);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = parent->AppendChild(*childB);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = parent->AppendChild(*childC);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childA->AppendChild(*grandchildA);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childB->AppendChild(*grandchildB);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = childC->AppendChild(*grandchildC);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(TreeQueries::Index(*doc) == 0);
    REQUIRE(TreeQueries::Index(*parent) == 0);
    REQUIRE(TreeQueries::Index(*childA) == 0);
    REQUIRE(TreeQueries::Index(*childB) == 1);
    REQUIRE(TreeQueries::Index(*childC) == 2);
    REQUIRE(TreeQueries::Index(*grandchildA) == 0);
    REQUIRE(TreeQueries::Index(*grandchildB) == 0);
    REQUIRE(TreeQueries::Index(*grandchildC) == 0);

    auto removeResult = parent->RemoveChild(*childA);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = parent->RemoveChild(*childB);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = parent->RemoveChild(*childC);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childA->RemoveChild(*grandchildA);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childB->RemoveChild(*grandchildB);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = childC->RemoveChild(*grandchildC);
    REQUIRE_FALSE(removeResult.HasException());

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