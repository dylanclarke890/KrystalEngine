#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML.Tests/TestElement.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Tests;

  TEST_CASE("MutationAlgorithms::EnsurePreInsertValidity", "[HTML][MutationAlgorithms]")
  {
    auto document = CreateRef<Document>();
    auto element = CreateRef<TestElement>(*document);

    SECTION("Inserting into a parent that isn't a Document, DocumentFragment, or Element returns a "
            "HierarchyRequestError")
    {
      // Nothing to do here as it's enforced by the type system since the parent parameter is a ContainerNode,
      // preventing invalid parent types from being passed in the first place.
    }

    SECTION("Inserting host inclusive ancestor into its descendant returns a HierarchyRequestError")
    {
      auto otherElement = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(document->AppendChild(*element).HasException());
      REQUIRE_FALSE(element->AppendChild(*otherElement).HasException());

      auto result = MutationAlgorithms::EnsurePreInsertValidity(*element, *otherElement, nullptr);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

      REQUIRE_FALSE(element->RemoveChild(*otherElement).HasException());
      REQUIRE_FALSE(document->RemoveChild(*element).HasException());
    }

    SECTION("Inserting before a node that isn't a child of the parent returns a NotFoundError")
    {
      auto otherElement = CreateRef<TestElement>(*document);
      auto notAChild = CreateRef<TestElement>(*document);

      auto result = MutationAlgorithms::EnsurePreInsertValidity(*otherElement, *element, notAChild.get());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NotFoundError);
    }

    SECTION("Inserting a Document returns a HierarchyRequestError")
    {
      auto otherDocument = CreateRef<Document>();

      auto result = MutationAlgorithms::EnsurePreInsertValidity(*otherDocument, *document, nullptr);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);
    }

    SECTION("Inserting an Attr returns a HierarchyRequestError")
    {
      // TODO(impl): test by creating via an element or document.
    }

    SECTION("Inserting a Text node into a Document returns a HierarchyRequestError")
    {
      auto textNode = CreateRef<HTML::Text>(*document, u8"Hello, world!");

      auto result = MutationAlgorithms::EnsurePreInsertValidity(*textNode, *document, nullptr);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);
    }

    SECTION("Inserting a DocumentType node into a non-Document parent returns a "
            "HierarchyRequestError")
    {
      auto docType = CreateRef<HTML::DocumentType>(*document, u8"html", u8"", u8"");

      auto result = MutationAlgorithms::EnsurePreInsertValidity(*docType, *element, nullptr);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);
    }

    SECTION("Inserting into a Document parent")
    {
      SECTION("with an DocumentFragment node")
      {
        auto docFragment = CreateRef<DocumentFragment>(*document);
        auto otherElement = CreateRef<TestElement>(*document);

        SECTION("Node has more than Element child returns a HierarchyRequestError")
        {
          REQUIRE_FALSE(docFragment->AppendChild(*element).HasException());
          REQUIRE_FALSE(docFragment->AppendChild(*otherElement).HasException());

          auto result = MutationAlgorithms::EnsurePreInsertValidity(*docFragment, *document, nullptr);
          REQUIRE(result.HasException());
          REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

          REQUIRE_FALSE(docFragment->RemoveChild(*otherElement).HasException());
          REQUIRE_FALSE(docFragment->RemoveChild(*element).HasException());
        }

        SECTION("Node has a Text child returns a HierarchyRequestError")
        {
          auto textNode = CreateRef<HTML::Text>(*document, u8"Hello, world!");

          REQUIRE_FALSE(docFragment->AppendChild(*textNode).HasException());

          auto result = MutationAlgorithms::EnsurePreInsertValidity(*docFragment, *document, nullptr);
          REQUIRE(result.HasException());
          REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

          REQUIRE_FALSE(docFragment->RemoveChild(*textNode).HasException());
        }

        SECTION("Node has one element child")
        {
          SECTION("Document has element child returns HierarchyRequestError")
          {
            REQUIRE_FALSE(document->AppendChild(*element).HasException());
            REQUIRE_FALSE(docFragment->AppendChild(*otherElement).HasException());

            auto result = MutationAlgorithms::EnsurePreInsertValidity(*docFragment, *document, nullptr);
            REQUIRE(result.HasException());
            REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

            REQUIRE_FALSE(docFragment->RemoveChild(*otherElement).HasException());
            REQUIRE_FALSE(document->RemoveChild(*element).HasException());
          }

          auto docType = CreateRef<HTML::DocumentType>(*document, u8"html", u8"", u8"");

          SECTION("Ref child is a DocumentType returns a HierarchyRequestError")
          {
            REQUIRE_FALSE(document->AppendChild(*docType).HasException());
            REQUIRE_FALSE(docFragment->AppendChild(*otherElement).HasException());

            auto result = MutationAlgorithms::EnsurePreInsertValidity(*docFragment, *document, docType.get());
            REQUIRE(result.HasException());
            REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

            REQUIRE_FALSE(docFragment->RemoveChild(*otherElement).HasException());
            REQUIRE_FALSE(document->RemoveChild(*docType).HasException());
          }

          SECTION(
            "Ref child non null and document type is following ref child returns a HierarchyRequestError")
          {
            auto comment = CreateRef<HTML::Comment>(*document, u8"This is a comment node.");

            REQUIRE_FALSE(docFragment->AppendChild(*otherElement).HasException());
            REQUIRE_FALSE(document->AppendChild(*comment).HasException());
            REQUIRE_FALSE(document->AppendChild(*docType).HasException());

            auto result = MutationAlgorithms::EnsurePreInsertValidity(*docFragment, *document, comment.get());
            REQUIRE(result.HasException());
            REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

            REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
            REQUIRE_FALSE(document->RemoveChild(*docType).HasException());
            REQUIRE_FALSE(docFragment->RemoveChild(*otherElement).HasException());
          }
        }
      }

      SECTION("with an Element node")
      {
        auto otherElement = CreateRef<TestElement>(*document);
        auto docType = CreateRef<HTML::DocumentType>(*document, u8"html", u8"", u8"");

        SECTION("Document already has an Element child returns a HierarchyRequestError")
        {
          REQUIRE_FALSE(document->AppendChild(*element).HasException());

          auto result = MutationAlgorithms::EnsurePreInsertValidity(*otherElement, *document, nullptr);
          REQUIRE(result.HasException());
          REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

          REQUIRE_FALSE(document->RemoveChild(*element).HasException());
        }

        SECTION("Ref child is a DocumentType returns a HierarchyRequestError")
        {
          REQUIRE_FALSE(document->AppendChild(*docType).HasException());

          auto result = MutationAlgorithms::EnsurePreInsertValidity(*otherElement, *document, docType.get());
          REQUIRE(result.HasException());
          REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

          REQUIRE_FALSE(document->RemoveChild(*docType).HasException());
        }

        SECTION("Ref child non null and document type is following ref child returns a HierarchyRequestError")
        {
          auto comment = CreateRef<Comment>(*document, u8"comment");

          REQUIRE_FALSE(document->AppendChild(*comment).HasException());
          REQUIRE_FALSE(document->AppendChild(*docType).HasException());

          auto result = MutationAlgorithms::EnsurePreInsertValidity(*otherElement, *document, comment.get());
          REQUIRE(result.HasException());
          REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

          REQUIRE_FALSE(document->RemoveChild(*docType).HasException());
          REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
        }
      }

      SECTION("with a DocumentType node")
      {
        auto docType = CreateRef<HTML::DocumentType>(*document, u8"html", u8"", u8"");

        SECTION("Document already has a DocumentType child returns a HierarchyRequestError")
        {
          auto childDocType = CreateRef<HTML::DocumentType>(*document, u8"html", u8"", u8"");

          REQUIRE_FALSE(document->AppendChild(*childDocType).HasException());

          auto result = MutationAlgorithms::EnsurePreInsertValidity(*docType, *document, nullptr);
          REQUIRE(result.HasException());
          REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

          REQUIRE_FALSE(document->RemoveChild(*childDocType).HasException());
        }

        SECTION("Document already has an Element child preceding ref child returns a HierarchyRequestError")
        {
          auto comment = CreateRef<Comment>(*document, u8"comment");

          REQUIRE_FALSE(document->AppendChild(*element).HasException());
          REQUIRE_FALSE(document->AppendChild(*comment).HasException());

          auto result = MutationAlgorithms::EnsurePreInsertValidity(*docType, *document, comment.get());
          REQUIRE(result.HasException());
          REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

          REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
          REQUIRE_FALSE(document->RemoveChild(*element).HasException());
        }

        SECTION("Document already has an element child and ref child is null returns a HierarchyRequestError")
        {
          REQUIRE_FALSE(document->AppendChild(*element).HasException());

          auto result = MutationAlgorithms::EnsurePreInsertValidity(*docType, *document, nullptr);
          REQUIRE(result.HasException());
          REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

          REQUIRE_FALSE(document->RemoveChild(*element).HasException());
        }
      }
    }

    SECTION("Otherwise returns success")
    {
      REQUIRE_FALSE(MutationAlgorithms::EnsurePreInsertValidity(*element, *document, nullptr).HasException());
    }
  }

  TEST_CASE("MutationAlgorithms::PreInsert", "[HTML][MutationAlgorithms]")
  {
    SECTION("If EnsurePreInsertValidity returns an exception, it is propagated")
    {
      auto document = CreateRef<Document>();
      auto element = CreateRef<TestElement>(*document);
      auto otherElement = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(document->AppendChild(*element).HasException());

      auto result = MutationAlgorithms::PreInsert(*element, *document, nullptr);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

      REQUIRE_FALSE(document->RemoveChild(*element).HasException());
    }

    SECTION("Otherwise, the node is pre-inserted into the parent before the child and the node is returned")
    {
      auto document = CreateRef<Document>();
      auto element = CreateRef<TestElement>(*document);
      auto otherElement = CreateRef<TestElement>(*document);

      auto result = MutationAlgorithms::PreInsert(*element, *otherElement, nullptr);
      REQUIRE_FALSE(result.HasException());

      REQUIRE(&result.Value() == element.get());
      REQUIRE(element->ParentNode() == otherElement.get());
      REQUIRE(otherElement->FirstChild() == element.get());

      REQUIRE_FALSE(otherElement->RemoveChild(*element).HasException());
    }

    SECTION("PreInsert with refChild same as node to insert inserts node in same place and returns node")
    {
      auto document = CreateRef<Document>();
      auto comment = CreateRef<Comment>(*document);

      REQUIRE_FALSE(document->AppendChild(*comment).HasException());

      auto result = MutationAlgorithms::PreInsert(*comment, *document, comment.get());
      REQUIRE_FALSE(result.HasException());

      REQUIRE(&result.Value() == comment.get());
      REQUIRE(comment->ParentNode() == document.get());
      REQUIRE(document->FirstChild() == comment.get());

      REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
    }
  }

  TEST_CASE("MutationAlgorithms::Insert", "[HTML][MutationAlgorithms]")
  {
    auto document = CreateRef<Document>();
    auto docFragment = CreateRef<DocumentFragment>(*document);

    SECTION("Inserting a DocumentFragment node with no children does nothing and returns success")
    {
      REQUIRE_FALSE(MutationAlgorithms::Insert(*docFragment, *document, nullptr).HasException());
      REQUIRE_FALSE(document->HasChildNodes());
    }

    SECTION("Inserting a DocumentFragment node with children removes those children and inserts them into "
            "the parent")
    {
      auto comment = CreateRef<Comment>(*document, u8"comment");
      auto element = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(docFragment->AppendChild(*comment).HasException());
      REQUIRE_FALSE(docFragment->AppendChild(*element).HasException());

      REQUIRE_FALSE(MutationAlgorithms::Insert(*docFragment, *document, nullptr).HasException());

      REQUIRE(document->FirstChild() == comment.get());
      REQUIRE(document->LastChild() == element.get());
      REQUIRE_FALSE(docFragment->HasChildNodes());

      // TODO(impl): need to test that a mutation record is queued for the nodes removed from the
      // DocumentFragment.

      REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
      REQUIRE_FALSE(document->RemoveChild(*element).HasException());
    }

    SECTION("Inserting before a refChild that is contained within a live range correctly updates the offsets "
            "of the live range")
    {
      // TODO(impl): need to be able to create a live range in order to test this.
    }

    // TODO(impl): uncomment this once document has AdoptNode implemented properly.
    // SECTION("Inserting a node from another Document adopts the node into the new Document")
    //{
    //  auto otherDocument = CreateRef<Document>();
    //  auto element = CreateRef<TestElement>(*otherDocument);

    // REQUIRE(&element->NodeDocument() == otherDocument.get());
    // REQUIRE_FALSE(element->IsConnected());

    // REQUIRE_FALSE(MutationAlgorithms::Insert(*element, *document, nullptr).HasException());

    // REQUIRE(&element->NodeDocument() == document.get());
    // REQUIRE(element->IsConnected());

    // REQUIRE_FALSE(document->RemoveChild(*element).HasException());
    // }
  }
}