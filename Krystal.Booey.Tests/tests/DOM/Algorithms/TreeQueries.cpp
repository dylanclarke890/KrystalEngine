#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::dom::tests
{
#pragma region Node Trees

  TEST_CASE("Length: 0 for doctypes and attributes, the data's length for character data, else the children",
            "[DOM][TreeQueries][dom:concept-node-length]")
  {
    SECTION("Doctypes, attributes and documents")
    {
      auto doc = CreateRef<Document>();
      auto doctype = doc->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());
      auto attr = doc->CreateAttribute(u8"a");
      REQUIRE_FALSE(attr.HasException());
      attr.Value()->Value(u8"value");

      CHECK(TreeQueries::Length(*doctype.Value()) == 0);
      CHECK(TreeQueries::Length(*attr.Value()) == 0);
      CHECK(TreeQueries::Length(*doc) == 0);
      REQUIRE_FALSE(doc->AppendChild(*doctype.Value()).HasException());
      CHECK(TreeQueries::Length(*doc) == 1);
      CHECK(doc->Length() == 1);
    }

    SECTION("Character data counts UTF-8 code units")
    {
      auto doc = CreateRef<Document>();
      CHECK(TreeQueries::Length(*CreateRef<krys::boo::dom::Text>(*doc, u8"")) == 0);
      CHECK(TreeQueries::Length(*CreateRef<krys::boo::dom::Text>(*doc, u8"\u00e9")) == 2);
    }

    SECTION("Elements count their children")
    {
      auto doc = CreateRef<Document>();
      auto parent = CreateRef<TestElement>(*doc);
      auto child = CreateRef<TestElement>(*doc);
      auto textNode = CreateRef<krys::boo::dom::Text>(*doc, u8"Hello, world!");

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
  }

  TEST_CASE("Empty: a node whose length is 0", "[DOM][TreeQueries][dom:concept-node-empty]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);
    auto textNode = CreateRef<krys::boo::dom::Text>(*doc, u8"Hello, world!");

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

  TEST_CASE("In a document tree: a node whose root is a document",
            "[DOM][TreeQueries][dom:document-trees][dom:in-a-document-tree][dom:in-a-document]"
            "[dom:concept-document-tree]")
  {
    SECTION("A document, and a connected node in a shadow tree")
    {
      auto doc = CreateRef<HTMLDocument>();
      CHECK(TreeQueries::IsInDocumentTree(*doc));

      auto host = doc->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(doc->AppendChild(*host).HasException());
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto inShadow = doc->CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*inShadow).HasException());

      CHECK(TreeQueries::IsInDocumentTree(*host));
      CHECK(inShadow->IsConnected());
      CHECK_FALSE(TreeQueries::IsInDocumentTree(*shadow));
      CHECK_FALSE(TreeQueries::IsInDocumentTree(*inShadow));
    }

    SECTION("Elements in and out of a document")
    {
      auto doc = CreateRef<Document>();
      auto parent = CreateRef<TestElement>(*doc);
      auto child = CreateRef<TestElement>(*doc);

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
  }

  TEST_CASE("Document element: the element whose parent is the document, if any",
            "[DOM][TreeQueries][dom:document-element]")
  {
    SECTION("Between comments")
    {
      auto doc = CreateRef<Document>();
      REQUIRE_FALSE(doc->AppendChild(*doc->CreateComment(u8"c")).HasException());
      auto element = CreateRef<TestElement>(*doc);
      REQUIRE_FALSE(doc->AppendChild(*element).HasException());
      REQUIRE_FALSE(doc->AppendChild(*doc->CreateComment(u8"d")).HasException());
      CHECK(TreeQueries::DocumentElement(*doc) == element.get());
      CHECK(doc->DocumentElement() == element);
    }

    SECTION("Elements in and out of a document")
    {
      auto doc = CreateRef<Document>();
      auto parent = CreateRef<TestElement>(*doc);
      auto child = CreateRef<TestElement>(*doc);

      REQUIRE(TreeQueries::DocumentElement(*doc) == nullptr);

      REQUIRE_FALSE(parent->AppendChild(*child).HasException());
      REQUIRE(TreeQueries::DocumentElement(*doc) == nullptr);

      REQUIRE_FALSE(doc->AppendChild(*parent).HasException());
      REQUIRE(TreeQueries::DocumentElement(*doc) == parent.get());

      REQUIRE_FALSE(child->Remove().HasException());
      REQUIRE_FALSE(parent->Remove().HasException());
    }
  }

#pragma endregion

#pragma region Shadow Trees

  TEST_CASE("Shadow tree: a node whose root is a shadow root",
            "[DOM][TreeQueries][dom:shadow-trees][dom:concept-shadow-tree][dom:concept-light-tree]")
  {
    SECTION("The shadow root itself and its descendants, until removed")
    {
      auto doc = CreateRef<HTMLDocument>();
      auto host = doc->CreateElement(u8"div").ReleaseValue();
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto child = doc->CreateElement(u8"span").ReleaseValue();
      auto grandchild = doc->CreateElement(u8"b").ReleaseValue();
      REQUIRE_FALSE(child->AppendChild(*grandchild).HasException());
      REQUIRE_FALSE(shadow->AppendChild(*child).HasException());

      CHECK(TreeQueries::IsInShadowTree(*shadow));
      CHECK(TreeQueries::IsInShadowTree(*child));
      CHECK(TreeQueries::IsInShadowTree(*grandchild));
      // The host is in its light tree.
      CHECK_FALSE(TreeQueries::IsInShadowTree(*host));

      REQUIRE_FALSE(child->Remove().HasException());
      CHECK_FALSE(TreeQueries::IsInShadowTree(*child));
      CHECK_FALSE(TreeQueries::IsInShadowTree(*grandchild));
    }

    SECTION("A light tree can be a shadow tree itself")
    {
      auto doc = CreateRef<HTMLDocument>();
      auto outerHost = doc->CreateElement(u8"div").ReleaseValue();
      auto outer = outerHost->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto innerHost = doc->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(outer->AppendChild(*innerHost).HasException());
      auto inner = innerHost->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto child = doc->CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(inner->AppendChild(*child).HasException());

      CHECK(TreeQueries::IsInShadowTree(*innerHost));
      CHECK(TreeQueries::IsInShadowTree(*child));
      CHECK(&TreeQueries::Root(*child) == inner.get());
    }

    SECTION("Without a host")
    {
      auto doc = CreateRef<Document>();
      auto docChild = CreateRef<TestElement>(*doc);

      REQUIRE_FALSE(TreeQueries::IsInShadowTree(*docChild));

      REQUIRE_FALSE(doc->AppendChild(*docChild).HasException());

      REQUIRE_FALSE(TreeQueries::IsInShadowTree(*docChild));

      auto shadowRoot = CreateRef<ShadowRoot>(*doc, nullptr);
      auto shadowChild = CreateRef<TestElement>(*doc);

      REQUIRE_FALSE(TreeQueries::IsInShadowTree(*shadowChild));

      REQUIRE_FALSE(shadowRoot->AppendChild(*shadowChild).HasException());

      REQUIRE(TreeQueries::IsInShadowTree(*shadowChild));

      REQUIRE_FALSE(doc->RemoveChild(*docChild).HasException());
      REQUIRE_FALSE(shadowRoot->RemoveChild(*shadowChild).HasException());
    }
  }

#pragma endregion

#pragma region Trees

  TEST_CASE("IsParent: a node is the parent of its children only", "[TreeQueries][dom:concept-tree-parent]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);

    REQUIRE_FALSE(TreeQueries::IsParent(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsParent(*child, *parent));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(TreeQueries::IsParent(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsParent(*child, *parent));

    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
  }

  TEST_CASE("IsChild: a node is a child of its parent only", "[TreeQueries][dom:concept-tree-child]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);

    REQUIRE_FALSE(TreeQueries::IsChild(*parent, *child));
    REQUIRE_FALSE(TreeQueries::IsChild(*child, *parent));

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE_FALSE(TreeQueries::IsChild(*parent, *child));
    REQUIRE(TreeQueries::IsChild(*child, *parent));

    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
  }

  TEST_CASE("Root: a node without a parent is its own root, others share their parent's",
            "[TreeQueries][dom:concept-tree-root]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);

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

  TEST_CASE("IsDescendant: children and their descendants, never the node itself",
            "[TreeQueries][dom:concept-tree-descendant]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);
    auto grandchild = CreateRef<TestElement>(*doc);

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

  TEST_CASE("IsInclusiveDescendant: the node itself and its descendants",
            "[TreeQueries][dom:concept-tree-inclusive-descendant]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);
    auto grandchild = CreateRef<TestElement>(*doc);

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

  TEST_CASE("IsAncestor: a node is an ancestor of its descendants, never of itself",
            "[TreeQueries][dom:concept-tree-ancestor]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);
    auto grandchild = CreateRef<TestElement>(*doc);

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

  TEST_CASE("IsInclusiveAncestor: the node itself and its ancestors",
            "[TreeQueries][dom:concept-tree-inclusive-ancestor]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);
    auto grandchild = CreateRef<TestElement>(*doc);

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

  TEST_CASE("IsSibling: different nodes with the same non-null parent",
            "[TreeQueries][dom:concept-tree-sibling]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto childA = CreateRef<TestElement>(*doc);
    auto childB = CreateRef<TestElement>(*doc);
    auto childC = CreateRef<TestElement>(*doc);

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

  TEST_CASE("IsInclusiveSibling: the node itself and its siblings",
            "[TreeQueries][dom:concept-tree-inclusive-sibling]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto childA = CreateRef<TestElement>(*doc);
    auto childB = CreateRef<TestElement>(*doc);
    auto childC = CreateRef<TestElement>(*doc);

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

  TEST_CASE("IsPreceding: earlier in tree order in the same tree",
            "[TreeQueries][dom:concept-tree-preceding]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto childA = CreateRef<TestElement>(*doc);
    auto childB = CreateRef<TestElement>(*doc);
    auto childC = CreateRef<TestElement>(*doc);
    auto grandchildA = CreateRef<TestElement>(*doc);
    auto grandchildB = CreateRef<TestElement>(*doc);
    auto grandchildC = CreateRef<TestElement>(*doc);

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
    REQUIRE(TreeQueries::IsPreceding(*grandchildA, *childB));
    REQUIRE(TreeQueries::IsPreceding(*grandchildB, *grandchildC));
    REQUIRE(TreeQueries::IsPreceding(*parent, *grandchildC));

    REQUIRE_FALSE(TreeQueries::IsPreceding(*childB, *grandchildA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*childA, *childA));
    REQUIRE_FALSE(TreeQueries::IsPreceding(*doc, *parent));
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

  TEST_CASE("IsFollowing: later in tree order in the same tree", "[TreeQueries][dom:concept-tree-following]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto childA = CreateRef<TestElement>(*doc);
    auto childB = CreateRef<TestElement>(*doc);
    auto childC = CreateRef<TestElement>(*doc);
    auto grandchildA = CreateRef<TestElement>(*doc);
    auto grandchildB = CreateRef<TestElement>(*doc);
    auto grandchildC = CreateRef<TestElement>(*doc);

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
    REQUIRE(TreeQueries::IsFollowing(*childB, *grandchildA));
    REQUIRE(TreeQueries::IsFollowing(*grandchildC, *grandchildB));
    REQUIRE(TreeQueries::IsFollowing(*grandchildC, *parent));

    REQUIRE_FALSE(TreeQueries::IsFollowing(*grandchildA, *childB));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*childA, *childA));
    REQUIRE_FALSE(TreeQueries::IsFollowing(*parent, *doc));
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

  TEST_CASE("Index: the number of preceding siblings", "[TreeQueries][dom:concept-tree-index]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto childA = CreateRef<TestElement>(*doc);
    auto childB = CreateRef<TestElement>(*doc);
    auto childC = CreateRef<TestElement>(*doc);
    auto grandchildA = CreateRef<TestElement>(*doc);
    auto grandchildB = CreateRef<TestElement>(*doc);
    auto grandchildC = CreateRef<TestElement>(*doc);

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

  TEST_CASE("Next and Previous: preorder, depth-first traversal", "[TreeQueries][dom:concept-tree-order]")
  {
    // root
    // +-- a
    // |   +-- a1
    // |   +-- a2
    // +-- b
    auto doc = CreateRef<Document>();
    auto before = CreateRef<TestElement>(*doc);
    auto root = CreateRef<TestElement>(*doc);
    auto a = CreateRef<TestElement>(*doc);
    auto a1 = CreateRef<TestElement>(*doc);
    auto a2 = CreateRef<TestElement>(*doc);
    auto b = CreateRef<TestElement>(*doc);
    auto after = CreateRef<TestElement>(*doc);
    REQUIRE_FALSE(doc->AppendChild(*before).HasException());
    REQUIRE_FALSE(before->AppendChild(*root).HasException());
    REQUIRE_FALSE(root->AppendChild(*a).HasException());
    REQUIRE_FALSE(a->AppendChild(*a1).HasException());
    REQUIRE_FALSE(a->AppendChild(*a2).HasException());
    REQUIRE_FALSE(root->AppendChild(*b).HasException());
    REQUIRE_FALSE(before->AppendChild(*after).HasException());

    SECTION("Next visits a node, then its children, then its following siblings")
    {
      CHECK(TreeQueries::Next(*root) == a.get());
      CHECK(TreeQueries::Next(*a) == a1.get());
      CHECK(TreeQueries::Next(*a1) == a2.get());
      CHECK(TreeQueries::Next(*a2) == b.get());
      CHECK(TreeQueries::Next(*b) == after.get());
      CHECK(TreeQueries::Next(*after) == nullptr);
    }

    SECTION("Previous is the reverse of Next")
    {
      CHECK(TreeQueries::Previous(*after) == b.get());
      CHECK(TreeQueries::Previous(*b) == a2.get());
      CHECK(TreeQueries::Previous(*a2) == a1.get());
      CHECK(TreeQueries::Previous(*a1) == a.get());
      CHECK(TreeQueries::Previous(*a) == root.get());
      CHECK(TreeQueries::Previous(*root) == before.get());
      CHECK(TreeQueries::Previous(*doc) == nullptr);
    }

    SECTION("Within a node, Next stops after its last descendant")
    {
      CHECK(TreeQueries::Next(*a2, root.get()) == b.get());
      CHECK(TreeQueries::Next(*b, root.get()) == nullptr);
      CHECK(TreeQueries::Next(*root, root.get()) == a.get());
    }

    SECTION("Within a node, Previous stops at the node")
    {
      CHECK(TreeQueries::Previous(*a, root.get()) == root.get());
      CHECK(TreeQueries::Previous(*root, root.get()) == nullptr);
    }

    SECTION("Within a node with a previous sibling, Previous doesn't leave it")
    {
      REQUIRE_FALSE(before->InsertBefore(*after, root.get()).HasException());
      CHECK(TreeQueries::Previous(*root) == after.get());
      CHECK(TreeQueries::Previous(*root, root.get()) == nullptr);
    }
  }

#pragma endregion

#pragma region Shadow Roots

  TEST_CASE("TreeQueries::ShadowIncludingRoot", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto child = CreateRef<TestElement>(*doc);

    REQUIRE(&TreeQueries::ShadowIncludingRoot(*doc) == doc.get());
    REQUIRE(&TreeQueries::ShadowIncludingRoot(*parent) == parent.get());
    REQUIRE(&TreeQueries::ShadowIncludingRoot(*child) == child.get());

    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    REQUIRE(&TreeQueries::ShadowIncludingRoot(*doc) == doc.get());
    REQUIRE(&TreeQueries::ShadowIncludingRoot(*parent) == parent.get());
    REQUIRE(&TreeQueries::ShadowIncludingRoot(*child) == parent.get());

    REQUIRE_FALSE(doc->AppendChild(*parent).HasException());

    REQUIRE(&TreeQueries::ShadowIncludingRoot(*doc) == doc.get());
    REQUIRE(&TreeQueries::ShadowIncludingRoot(*parent) == doc.get());
    REQUIRE(&TreeQueries::ShadowIncludingRoot(*child) == doc.get());

    // FIXME: Test with ShadowRoot when implemented

    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
    REQUIRE_FALSE(doc->RemoveChild(*parent).HasException());
  }

  TEST_CASE("TreeQueries::GetShadowRoot", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<HTMLDocument>();
    auto host = doc->CreateElement(u8"div").ReleaseValue();
    REQUIRE(TreeQueries::GetShadowRoot(*host) == nullptr);

    auto mode = GENERATE(ShadowRootMode::Open, ShadowRootMode::Closed);
    auto shadow = host->AttachShadow({.Mode = mode}).ReleaseValue();
    REQUIRE(shadow->Mode() == mode);
    REQUIRE(TreeQueries::GetShadowRoot(*host) == shadow.get());
    // The web-exposed getter hides closed shadow roots.
    REQUIRE((host->ShadowRoot().get() == shadow.get()) == (mode == ShadowRootMode::Open));
  }

  TEST_CASE("TreeQueries::NextShadowIncluding", "[HTML][TreeQueries]")
  {
    // <div host>[shadow: <b inside>] <span light/></div> <i after/>, with the host in a container.
    auto doc = CreateRef<HTMLDocument>();
    auto container = doc->CreateElement(u8"div").ReleaseValue();
    auto host = doc->CreateElement(u8"div").ReleaseValue();
    auto inside = doc->CreateElement(u8"b").ReleaseValue();
    auto light = doc->CreateElement(u8"span").ReleaseValue();
    auto after = doc->CreateElement(u8"i").ReleaseValue();
    REQUIRE_FALSE(container->AppendChild(*host).HasException());
    REQUIRE_FALSE(container->AppendChild(*after).HasException());
    REQUIRE_FALSE(host->AppendChild(*light).HasException());
    auto shadow = host->AttachShadow({}).ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inside).HasException());

    // The shadow tree comes just after its host, before the host's children.
    List<Node *> order;
    for (Node *node = container.get(); node; node = TreeQueries::NextShadowIncluding(*node))
    {
      order.push_back(node);
    }
    CHECK(
      order
      == List<Node *> {container.get(), host.get(), shadow.get(), inside.get(), light.get(), after.get()});

    // Staying within the host: its shadow tree and children.
    order.clear();
    for (Node *node = host.get(); node; node = TreeQueries::NextShadowIncluding(*node, host.get()))
    {
      order.push_back(node);
    }
    CHECK(order == List<Node *> {host.get(), shadow.get(), inside.get(), light.get()});

    // Staying within the shadow root.
    CHECK(TreeQueries::NextShadowIncluding(*inside, shadow.get()) == nullptr);
  }

  TEST_CASE("TreeQueries::ParentOrShadowHostElement", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<HTMLDocument>();
    auto host = doc->CreateElement(u8"div").ReleaseValue();
    auto child = doc->CreateElement(u8"span").ReleaseValue();
    auto inner = doc->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(doc->AppendChild(*host).HasException());
    REQUIRE_FALSE(host->AppendChild(*child).HasException());
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Closed}).ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

    REQUIRE(TreeQueries::ParentOrShadowHostElement(*child) == host.get());
    REQUIRE(TreeQueries::ParentOrShadowHostElement(*inner) == host.get());
    REQUIRE(TreeQueries::ParentOrShadowHostElement(*host) == nullptr);
    REQUIRE(TreeQueries::ParentOrShadowHostElement(*shadow) == nullptr);
  }

  TEST_CASE("TreeQueries shadow-including ancestry and retargeting", "[HTML][TreeQueries]")
  {
    auto doc = CreateRef<HTMLDocument>();
    auto host = doc->CreateElement(u8"div").ReleaseValue();
    auto inner = doc->CreateElement(u8"span").ReleaseValue();
    auto deep = doc->CreateElement(u8"b").ReleaseValue();
    REQUIRE_FALSE(doc->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({}).ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
    REQUIRE_FALSE(inner->AppendChild(*deep).HasException());

    // Elements in a host's shadow tree are its shadow-including descendants.
    CHECK(TreeQueries::IsShadowIncludingDescendant(*inner, *host));
    CHECK(TreeQueries::IsShadowIncludingDescendant(*deep, *host));
    CHECK(TreeQueries::IsShadowIncludingInclusiveAncestor(*host, *deep));
    CHECK(TreeQueries::IsShadowIncludingAncestor(*doc, *deep));
    CHECK_FALSE(TreeQueries::IsShadowIncludingDescendant(*host, *inner));

    // Retargeting a node in a shadow tree against a node outside it gives the host.
    CHECK(EventTargetAlgorithms::Retarget(deep.get(), *doc) == host.get());
    CHECK(EventTargetAlgorithms::Retarget(deep.get(), *host) == host.get());
    // Against a node in the same shadow tree, the node itself.
    CHECK(EventTargetAlgorithms::Retarget(deep.get(), *inner) == deep.get());
    // Nodes outside shadow trees are returned as they are.
    CHECK(EventTargetAlgorithms::Retarget(host.get(), *inner) == host.get());
  }

#pragma endregion

#pragma region Pseudo-element aware traversal

  TEST_CASE_METHOD(css::tests::ValueFixture, "TreeQueries: traversal including pseudo-elements",
                   "[DOM][TreeQueries]")
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

    CHECK(TreeQueries::PseudoAwareFirstChild(p) == before);
    CHECK(TreeQueries::PseudoAwareLastChild(p) == after);
    CHECK(TreeQueries::PseudoAwareNextSibling(*before) == &b);
    CHECK(TreeQueries::PseudoAwareNextSibling(b) == after);
    CHECK(TreeQueries::PseudoAwarePreviousSibling(b) == before);
    CHECK(TreeQueries::PseudoAwarePreviousSibling(*after) == &b);
    CHECK_FALSE(TreeQueries::PseudoAwarePreviousSibling(*before));
    // An element with only a ::before.
    CHECK(TreeQueries::PseudoAwareFirstChild(empty) == emptyBefore);
    CHECK(TreeQueries::PseudoAwareLastChild(empty) == emptyBefore);

    // Tree order: root, p, ::before, b, ::after, empty, ::before, s.
    List<Element *> order;
    for (Element *element = &root; element;
         element = TreeQueries::NextElementIncludingPseudo(*element, &root))
    {
      order.push_back(element);
    }
    CHECK(order == List<Element *> {&root, &p, before, &b, after, &empty, emptyBefore, &fixture.ById(u8"s")});

    List<Element *> reversed;
    for (Element *element = &fixture.ById(u8"s"); element;
         element = TreeQueries::PreviousElementIncludingPseudo(*element, &root))
    {
      reversed.push_back(element);
    }
    CHECK(reversed
          == List<Element *> {&fixture.ById(u8"s"), emptyBefore, &empty, after, &b, before, &p, &root});

    CHECK(TreeQueries::NextElementIncludingPseudoSkippingChildren(p, &root) == &empty);
    CHECK(TreeQueries::PseudoAwarePreviousElementSibling(empty) == &p);
  }

#pragma endregion
}
