#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey.Tests/HTML/CustomElementTestUtils.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief An observer that only collects records, for takeRecords().
    KRYS_NODISCARD Ref<MutationObserver> NewCollectingObserver() noexcept
    {
      return CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
    }
  }

  TEST_CASE(
    "Ensure pre-insert validity: the node tree's constraints, with the exceptions they throw",
    "[DOM][MutationAlgorithms][dom:mutation-algorithms][dom:concept-node-ensure-pre-insertion-validity]"
    "[dom:node-trees][dom:concept-node-tree]")
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
      auto attrResult = document->CreateAttribute(u8"test");
      REQUIRE_FALSE(attrResult.HasException());
      auto &&attr = attrResult.ReleaseValue();

      auto result = MutationAlgorithms::EnsurePreInsertValidity(*attr, *element, nullptr);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);
    }

    SECTION("Inserting a Text node into a Document returns a HierarchyRequestError")
    {
      auto textNode = CreateRef<krys::boo::dom::Text>(*document, u8"Hello, world!");

      auto result = MutationAlgorithms::EnsurePreInsertValidity(*textNode, *document, nullptr);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::HierarchyRequestError);
    }

    SECTION("Inserting a DocumentType node into a non-Document parent returns a "
            "HierarchyRequestError")
    {
      auto docType = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();

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
          auto textNode = CreateRef<krys::boo::dom::Text>(*document, u8"Hello, world!");

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

          auto docType = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();

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
            auto comment = CreateRef<krys::boo::dom::Comment>(*document, u8"This is a comment node.");

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
        auto docType = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();

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
        auto docType = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();

        SECTION("Document already has a DocumentType child returns a HierarchyRequestError")
        {
          auto childDocType = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();

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

  TEST_CASE(
    "Pre-insert: ensures validity, then inserts before the child (or its next sibling if it's the node)",
    "[DOM][MutationAlgorithms][dom:concept-node-pre-insert]")
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

  TEST_CASE("Insert: a node or a fragment's children, updating live ranges and queueing a record",
            "[DOM][MutationAlgorithms][dom:concept-node-insert][dom:insert-suppressobservers]")
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

      // FIXME: Need to test that a mutation record is queued for the nodes removed
      // from the DocumentFragment.

      REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
      REQUIRE_FALSE(document->RemoveChild(*element).HasException());
    }

    SECTION("Inserting before a refChild that is contained within a live range correctly updates the offsets "
            "of the live range")
    {
      auto element = CreateRef<TestElement>(*document);
      auto child1 = CreateRef<TestElement>(*document);
      auto child2 = CreateRef<TestElement>(*document);
      auto child3 = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(document->AppendChild(*element).HasException());
      REQUIRE_FALSE(element->AppendChild(*child2).HasException());
      REQUIRE_FALSE(element->AppendChild(*child3).HasException());

      auto range = document->CreateRange();
      REQUIRE_FALSE(range->SetStart(*element, 0uz).HasException());
      REQUIRE_FALSE(range->SetEnd(*element, 2uz).HasException());

      REQUIRE_FALSE(MutationAlgorithms::Insert(*child1, *element, child2.get()).HasException());

      REQUIRE(range->StartContainer() == element.get());
      REQUIRE(range->StartOffset() == 0uz);

      REQUIRE(range->EndContainer() == element.get());
      REQUIRE(range->EndOffset() == 3uz);

      REQUIRE_FALSE(document->RemoveChild(*element).HasException());
      REQUIRE_FALSE(element->RemoveChild(*child1).HasException());
      REQUIRE_FALSE(element->RemoveChild(*child2).HasException());
      REQUIRE_FALSE(element->RemoveChild(*child3).HasException());
    }

    SECTION("Inserting a node from another Document adopts the node into the new Document")
    {
      auto otherDocument = CreateRef<Document>();
      auto element = CreateRef<TestElement>(*otherDocument);

      REQUIRE(&element->NodeDocument() == otherDocument.get());
      REQUIRE_FALSE(element->IsConnected());

      REQUIRE_FALSE(MutationAlgorithms::Insert(*element, *document, nullptr).HasException());

      REQUIRE(&element->NodeDocument() == document.get());
      REQUIRE(element->IsConnected());

      REQUIRE_FALSE(document->RemoveChild(*element).HasException());
    }

    SECTION("A record is queued for the parent unless suppressObservers is true")
    {
      auto parent = CreateRef<TestElement>(*document);
      auto observer = NewCollectingObserver();
      REQUIRE_FALSE(observer->Observe(*parent, {.ChildList = true}).HasException());

      auto first = CreateRef<TestElement>(*document);
      REQUIRE_FALSE(MutationAlgorithms::Insert(*first, *parent, nullptr).HasException());
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(records[0]->AddedNodes()->Length() == 1);
      CHECK(records[0]->AddedNodes()->Item(0).get() == first.get());

      auto second = CreateRef<TestElement>(*document);
      REQUIRE_FALSE(
        MutationAlgorithms::Insert(*second, *parent, first.get(), SuppressObservers(true)).HasException());
      CHECK(observer->TakeRecords().empty());
      CHECK(parent->FirstChild() == second.get());
    }
  }

  TEST_CASE("Append: pre-inserts before null", "[DOM][MutationAlgorithms][dom:concept-node-append]")
  {
    SECTION("If EnsurePreInsertValidity returns an exception, it is propagated")
    {
      auto document = CreateRef<Document>();
      auto element = CreateRef<TestElement>(*document);
      auto otherElement = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(document->AppendChild(*element).HasException());

      auto result = MutationAlgorithms::Append(*element, *document);
      REQUIRE(result == ExceptionCode::HierarchyRequestError);

      REQUIRE_FALSE(document->RemoveChild(*element).HasException());
    }

    SECTION("Otherwise, the node is appended to the parent and the node is returned")
    {
      auto document = CreateRef<Document>();
      auto element = CreateRef<TestElement>(*document);
      auto otherElement = CreateRef<TestElement>(*document);

      auto result = MutationAlgorithms::Append(*element, *otherElement);
      REQUIRE_FALSE(result.HasException());

      REQUIRE(&result.Value() == element);
      REQUIRE(element->ParentNode() == otherElement);
      REQUIRE(otherElement->FirstChild() == element);

      REQUIRE_FALSE(otherElement->RemoveChild(*element).HasException());
    }
  }

  TEST_CASE("Move: checks the move, then moves the node keeping its state",
            "[DOM][MutationAlgorithms][dom:move]")
  {
    auto document = CreateRef<Document>();
    auto element = CreateRef<TestElement>(*document);
    auto otherElement = CreateRef<TestElement>(*document);

    REQUIRE_FALSE(document->AppendChild(*element).HasException());

    SECTION("HierarchyRequestError if the node being moved does not share the same root as newParent")
    {
      REQUIRE(MutationAlgorithms::Move(*element, *otherElement, nullptr)
              == ExceptionCode::HierarchyRequestError);
    }

    SECTION("HierarchyRequestError if the node being moved is an inclusive ancestor of newParent")
    {
      REQUIRE(MutationAlgorithms::Move(*document, *element, nullptr) == ExceptionCode::HierarchyRequestError);
    }

    SECTION("NotFoundError if the refChild is not a child of newParent")
    {
      REQUIRE(MutationAlgorithms::Move(*element, *document, otherElement.get())
              == ExceptionCode::NotFoundError);
    }

    SECTION("HierarchyRequestError if node is not an Element or CharacterData node")
    {
      auto doctype = document->Implementation().CreateDocumentType(u8"comment", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      REQUIRE_FALSE(document->InsertBefore(*doctype.Value(), element.get()).HasException());
      REQUIRE(MutationAlgorithms::Move(*doctype.Value(), *document, nullptr)
              == ExceptionCode::HierarchyRequestError);

      REQUIRE_FALSE(document->RemoveChild(*doctype.Value()).HasException());
    }

    SECTION("HierarchyRequestError if node is a Text node and newParent is a Document")
    {
      auto textNode = CreateRef<krys::boo::dom::Text>(*document, u8"Hello, world!");
      REQUIRE_FALSE(element->AppendChild(*textNode).HasException());

      REQUIRE(MutationAlgorithms::Move(*textNode, *document, nullptr)
              == ExceptionCode::HierarchyRequestError);

      REQUIRE_FALSE(element->RemoveChild(*textNode).HasException());
    }

    SECTION("HierarchyRequest error if newParent is a Document, node is an Element node and")
    {
      SECTION("newParent has an element child")
      {
        REQUIRE_FALSE(element->AppendChild(*otherElement).HasException());

        REQUIRE(MutationAlgorithms::Move(*otherElement, *document, nullptr)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(element->RemoveChild(*otherElement).HasException());
      }

      SECTION("refChild is a doctype")
      {
        auto doctype = document->Implementation().CreateDocumentType(u8"comment", u8"", u8"");
        REQUIRE_FALSE(doctype.HasException());
        REQUIRE_FALSE(document->InsertBefore(*doctype.Value(), element.get()).HasException());

        REQUIRE(MutationAlgorithms::Move(*element, *document, doctype.Value().get())
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(document->RemoveChild(*doctype.Value()).HasException());
      }

      SECTION("refChild is non null and a DocumentType is following child")
      {
        auto comment = CreateRef<Comment>(*document, u8"comment");
        auto doctype = document->Implementation().CreateDocumentType(u8"comment", u8"", u8"");

        REQUIRE_FALSE(document->InsertBefore(*doctype.Value(), element.get()).HasException());
        REQUIRE_FALSE(document->InsertBefore(*comment, doctype.Value().get()).HasException());

        REQUIRE(MutationAlgorithms::Move(*element, *document, comment.get())
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(document->RemoveChild(*doctype.Value()).HasException());
        REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
      }
    }

    SECTION("Otherwise, the node is moved to the new parent")
    {
      auto anotherElement = CreateRef<TestElement>(*document);
      auto child1 = CreateRef<TestElement>(*document);
      auto child2 = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(element->AppendChild(*otherElement).HasException());
      REQUIRE_FALSE(element->AppendChild(*anotherElement).HasException());
      REQUIRE_FALSE(otherElement->AppendChild(*child1).HasException());
      REQUIRE_FALSE(otherElement->AppendChild(*child2).HasException());

      auto result = MutationAlgorithms::Move(*child1, *anotherElement, nullptr);
      REQUIRE_FALSE(result.HasException());

      REQUIRE(otherElement->FirstChild() == child2.get());
      REQUIRE(otherElement->LastChild() == child2.get());
      REQUIRE(anotherElement->FirstChild() == child1.get());
      REQUIRE(anotherElement->LastChild() == child1.get());

      result = MutationAlgorithms::Move(*child2, *anotherElement, child1.get());
      REQUIRE_FALSE(result.HasException());

      REQUIRE(otherElement->FirstChild() == nullptr);
      REQUIRE(otherElement->LastChild() == nullptr);
      REQUIRE(anotherElement->FirstChild() == child2.get());
      REQUIRE(anotherElement->LastChild() == child1.get());

      REQUIRE_FALSE(element->RemoveChild(*otherElement).HasException());
      REQUIRE_FALSE(element->RemoveChild(*anotherElement).HasException());
      REQUIRE_FALSE(anotherElement->RemoveChild(*child1).HasException());
      REQUIRE_FALSE(anotherElement->RemoveChild(*child2).HasException());
    }

    // FIXME: Tests for the mutation records queued.

    // FIXME: Tests for live ranges.
  }

  TEST_CASE("Replace: ensures validity excluding the child, then swaps in the node",
            "[DOM][MutationAlgorithms][dom:concept-node-replace]")
  {
    SECTION("HierarchyRequestError if parent is not a Document, DocumentFragment, or Element")
    {
      // Nothing to do here as it's enforced by the type system since the parent parameter is a
      // ContainerNode.
    }

    auto document = CreateRef<Document>();
    auto element = CreateRef<TestElement>(*document);
    auto child1 = CreateRef<TestElement>(*document);
    auto child2 = CreateRef<TestElement>(*document);

    REQUIRE_FALSE(document->AppendChild(*element).HasException());
    REQUIRE_FALSE(element->AppendChild(*child1).HasException());
    REQUIRE_FALSE(element->AppendChild(*child2).HasException());

    SECTION("HierarchyRequestError if newChild is an inclusive ancestor of parent")
    {
      REQUIRE(MutationAlgorithms::Replace(*element, *document, *child1)
              == ExceptionCode::HierarchyRequestError);
    }

    SECTION("NotFoundError if oldChild is not a child of parent")
    {
      auto otherElement = CreateRef<TestElement>(*document);
      REQUIRE(MutationAlgorithms::Replace(*otherElement, *child1, *element) == ExceptionCode::NotFoundError);
    }

    SECTION("HierarchyRequestError if newChild is not a DocumentFragment, DocumentType, Element, or "
            "CharacterData node")
    {
      auto otherDocument = CreateRef<Document>();
      REQUIRE(MutationAlgorithms::Replace(*element, *otherDocument, *document)
              == ExceptionCode::HierarchyRequestError);
    }

    SECTION("HierarchyRequestError if newChild is a Text node and parent is a Document")
    {
      auto textNode = CreateRef<krys::boo::dom::Text>(*document, u8"Hello, world!");
      REQUIRE(MutationAlgorithms::Replace(*element, *textNode, *document)
              == ExceptionCode::HierarchyRequestError);
    }

    SECTION("HierarchyRequestError if newChild is a DocumentType node and parent is not a Document")
    {
      auto docType = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();
      REQUIRE(MutationAlgorithms::Replace(*child1, *docType, *element)
              == ExceptionCode::HierarchyRequestError);
    }

    SECTION("HierarchyRequestError if parent is a Document and")
    {
      auto docType = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();

      SECTION("newChild is an Element node and parent already has an element child")
      {
        REQUIRE_FALSE(document->InsertBefore(*docType, element.get()).HasException());

        auto otherElement = CreateRef<TestElement>(*document);
        REQUIRE(MutationAlgorithms::Replace(*docType, *otherElement, *document)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(document->RemoveChild(*docType).HasException());
      }

      SECTION("newChild is an Element node and parent has a DocumentType following oldChild")
      {
        REQUIRE_FALSE(document->InsertBefore(*docType, element.get()).HasException());

        auto comment = CreateRef<Comment>(*document, u8"comment");
        REQUIRE_FALSE(document->InsertBefore(*comment, docType.get()).HasException());

        auto otherElement = CreateRef<TestElement>(*document);
        REQUIRE(MutationAlgorithms::Replace(*comment, *otherElement, *document)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
        REQUIRE_FALSE(document->RemoveChild(*docType).HasException());
      }

      SECTION("newChild is a DocumentType node and parent already has a DocumentType child")
      {
        auto comment = CreateRef<Comment>(*document, u8"comment");
        auto otherDocType = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();

        REQUIRE_FALSE(document->InsertBefore(*docType, element.get()).HasException());
        REQUIRE_FALSE(document->AppendChild(*comment).HasException());

        REQUIRE(MutationAlgorithms::Replace(*comment, *otherDocType, *document)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(document->RemoveChild(*docType).HasException());
        REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
      }

      SECTION("newChild is a DocumentType node and an Element node precedes oldChild")
      {
        auto comment = CreateRef<Comment>(*document, u8"comment");

        REQUIRE_FALSE(document->AppendChild(*comment).HasException());

        REQUIRE(MutationAlgorithms::Replace(*comment, *docType, *document)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
      }

      SECTION("newChild is a DocumentFragment node and has more than one Element child")
      {
        auto docFragment = CreateRef<DocumentFragment>(*document);

        auto element1 = CreateRef<TestElement>(*document);
        auto element2 = CreateRef<TestElement>(*document);

        REQUIRE_FALSE(docFragment->AppendChild(*element1).HasException());
        REQUIRE_FALSE(docFragment->AppendChild(*element2).HasException());

        REQUIRE(MutationAlgorithms::Replace(*element, *docFragment, *document)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(docFragment->RemoveChild(*element1).HasException());
        REQUIRE_FALSE(docFragment->RemoveChild(*element2).HasException());
      }

      SECTION("newChild is a DocumentFragment node and has an Text node child and parent already")
      {
        auto docFragment = CreateRef<DocumentFragment>(*document);
        auto text = CreateRef<krys::boo::dom::Text>(*document, u8"");

        REQUIRE_FALSE(docFragment->AppendChild(*text).HasException());

        REQUIRE(MutationAlgorithms::Replace(*element, *docFragment, *document)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(docFragment->RemoveChild(*text).HasException());
      }

      SECTION("newChild is a DocumentFragment node and has an Element child and parent already has an "
              "Element child")
      {
        auto docFragment = CreateRef<DocumentFragment>(*document);
        auto comment = CreateRef<Comment>(*document, u8"comment");
        auto element1 = CreateRef<TestElement>(*document);

        REQUIRE_FALSE(document->AppendChild(*comment).HasException());
        REQUIRE_FALSE(docFragment->AppendChild(*element1).HasException());

        REQUIRE(MutationAlgorithms::Replace(*comment, *docFragment, *document)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(docFragment->RemoveChild(*element1).HasException());
        REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
      }

      SECTION("newChild is a DocumentFragment node, has an Element child and parent already has a "
              "DocumentType child following oldChild")
      {
        auto docFragment = CreateRef<DocumentFragment>(*document);
        auto comment = CreateRef<Comment>(*document, u8"comment");
        auto element1 = CreateRef<TestElement>(*document);

        REQUIRE_FALSE(document->InsertBefore(*docType, element.get()).HasException());
        REQUIRE_FALSE(document->AppendChild(*comment).HasException());
        REQUIRE_FALSE(docFragment->AppendChild(*element1).HasException());

        REQUIRE(MutationAlgorithms::Replace(*comment, *docFragment, *document)
                == ExceptionCode::HierarchyRequestError);

        REQUIRE_FALSE(docFragment->RemoveChild(*element1).HasException());
        REQUIRE_FALSE(document->RemoveChild(*comment).HasException());
        REQUIRE_FALSE(document->RemoveChild(*docType).HasException());
      }

      SECTION("Otherwise, the newChild replaces the oldChild")
      {
        auto otherElement = CreateRef<TestElement>(*document);

        auto result = MutationAlgorithms::Replace(*child1, *otherElement, *element);
        REQUIRE_FALSE(result.HasException());

        REQUIRE(element->FirstChild() == otherElement.get());
        REQUIRE(element->LastChild() == child2.get());
        REQUIRE_FALSE(child1->ParentNode());
      }

      if (element->ParentNode())
      {
        REQUIRE_FALSE(element->ParentNode()->RemoveChild(*element).HasException());
      }
      if (child1->ParentNode())
      {
        REQUIRE_FALSE(child1->ParentNode()->RemoveChild(*child1).HasException());
      }
      if (child2->ParentNode())
      {
        REQUIRE_FALSE(child2->ParentNode()->RemoveChild(*child2).HasException());
      }
    }

    // FIXME: Tests for the mutation records queued.
  }

  TEST_CASE("Replace all: removes every child, then inserts the node, in one record",
            "[DOM][MutationAlgorithms][dom:concept-node-replace-all]")
  {
    SECTION("Replacing with nullptr removes all the children")
    {
      auto document = CreateRef<Document>();
      auto element = CreateRef<TestElement>(*document);
      auto child1 = CreateRef<TestElement>(*document);
      auto child2 = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(document->AppendChild(*element).HasException());
      REQUIRE_FALSE(element->AppendChild(*child1).HasException());
      REQUIRE_FALSE(element->AppendChild(*child2).HasException());

      REQUIRE_FALSE(MutationAlgorithms::ReplaceAll(nullptr, *element).HasException());

      REQUIRE_FALSE(element->HasChildNodes());
      REQUIRE_FALSE(child1->ParentNode());
      REQUIRE_FALSE(child2->ParentNode());
    }

    SECTION("Otherwise, the children of the element are replaced with the new children")
    {
      auto document = CreateRef<Document>();
      auto element = CreateRef<TestElement>(*document);

      auto child1 = CreateRef<TestElement>(*document);
      auto child2 = CreateRef<TestElement>(*document);

      auto docFragment = CreateRef<DocumentFragment>(*document);
      auto newChild1 = CreateRef<TestElement>(*document);
      auto newChild2 = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(document->AppendChild(*element).HasException());
      REQUIRE_FALSE(element->AppendChild(*child1).HasException());
      REQUIRE_FALSE(element->AppendChild(*child2).HasException());
      REQUIRE_FALSE(docFragment->AppendChild(*newChild1).HasException());
      REQUIRE_FALSE(docFragment->AppendChild(*newChild2).HasException());

      REQUIRE_FALSE(MutationAlgorithms::ReplaceAll(docFragment.get(), *element).HasException());
      REQUIRE(element->FirstChild() == newChild1.get());
      REQUIRE(element->LastChild() == newChild2.get());
      REQUIRE_FALSE(child1->ParentNode());
      REQUIRE_FALSE(child2->ParentNode());

      REQUIRE_FALSE(element->RemoveChild(*newChild1).HasException());
      REQUIRE_FALSE(element->RemoveChild(*newChild2).HasException());
    }

    // FIXME: Tests for the mutation records queued.
  }

  TEST_CASE("Pre-remove: removes the child, which must be the parent's",
            "[DOM][MutationAlgorithms][dom:concept-node-pre-remove]")
  {
    auto document = CreateRef<Document>();
    auto element = CreateRef<TestElement>(*document);

    SECTION("Removing a node that doesn't have a parent returns a NotFoundError")
    {
      REQUIRE(MutationAlgorithms::PreRemove(*element, *document) == ExceptionCode::NotFoundError);
    }

    SECTION("Otherwise, the node is removed from the parent and returned")
    {
      REQUIRE_FALSE(document->AppendChild(*element).HasException());

      auto result = MutationAlgorithms::PreRemove(*element, *document);
      REQUIRE_FALSE(result.HasException());

      REQUIRE_FALSE(element->ParentNode());
      REQUIRE_FALSE(document->HasChildNodes());
      REQUIRE(&result.Value() == element.get());
    }
  }

  TEST_CASE("Remove: detaches the node, updating live ranges and node iterators and queueing a record",
            "[DOM][MutationAlgorithms][dom:concept-node-remove][dom:remove-suppressobservers]")
  {
    auto document = CreateRef<Document>();
    auto element = CreateRef<TestElement>(*document);

    SECTION("A live range after the node moves back")
    {
      auto parent = CreateRef<TestElement>(*document);
      auto first = CreateRef<TestElement>(*document);
      auto second = CreateRef<TestElement>(*document);
      REQUIRE_FALSE(parent->AppendChild(*first).HasException());
      REQUIRE_FALSE(parent->AppendChild(*second).HasException());
      auto range = document->CreateRange();
      REQUIRE_FALSE(range->SetStart(*parent, 2).HasException());
      REQUIRE_FALSE(range->SetEnd(*parent, 2).HasException());

      REQUIRE_FALSE(MutationAlgorithms::Remove(*first).HasException());
      CHECK(range->StartOffset() == 1);
      CHECK(range->EndOffset() == 1);
    }

    SECTION("A node iterator rooted at an element moves off the removed node")
    {
      auto root = CreateRef<TestElement>(*document);
      auto first = CreateRef<TestElement>(*document);
      auto second = CreateRef<TestElement>(*document);
      REQUIRE_FALSE(root->AppendChild(*first).HasException());
      REQUIRE_FALSE(root->AppendChild(*second).HasException());
      auto iterator = document->CreateNodeIterator(*root);
      REQUIRE(iterator->NextNode().Value().get() == root.get());
      REQUIRE(iterator->NextNode().Value().get() == first.get());
      REQUIRE(&iterator->ReferenceNode() == first.get());

      REQUIRE_FALSE(MutationAlgorithms::Remove(*first).HasException());
      CHECK(&iterator->ReferenceNode() == root.get());
      CHECK(iterator->NextNode().Value().get() == second.get());
    }

    SECTION("A record is queued for the parent unless suppressObservers is true")
    {
      auto parent = CreateRef<TestElement>(*document);
      auto first = CreateRef<TestElement>(*document);
      auto second = CreateRef<TestElement>(*document);
      REQUIRE_FALSE(parent->AppendChild(*first).HasException());
      REQUIRE_FALSE(parent->AppendChild(*second).HasException());
      auto observer = NewCollectingObserver();
      REQUIRE_FALSE(observer->Observe(*parent, {.ChildList = true}).HasException());

      REQUIRE_FALSE(MutationAlgorithms::Remove(*second).HasException());
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(records[0]->RemovedNodes()->Item(0).get() == second.get());
      CHECK(records[0]->PreviousSibling().get() == first.get());

      REQUIRE_FALSE(MutationAlgorithms::Remove(*first, SuppressObservers(true)).HasException());
      CHECK(observer->TakeRecords().empty());
    }

    SECTION("Otherwise, the node is removed from the parent and returned")
    {
      REQUIRE_FALSE(document->AppendChild(*element).HasException());

      auto result = MutationAlgorithms::Remove(*element);
      REQUIRE_FALSE(result.HasException());

      REQUIRE_FALSE(element->ParentNode());
      REQUIRE_FALSE(document->HasChildNodes());
    }
  }

  TEST_CASE("Ensure pre-insert validity: children to exclude don't count against the constraints",
            "[DOM][MutationAlgorithms][dom:concept-node-ensure-pre-insertion-validity]")
  {
    auto document = CreateRef<Document>();
    auto doctype = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").ReleaseValue();
    auto element = CreateRef<TestElement>(*document);
    REQUIRE_FALSE(document->AppendChild(*doctype).HasException());
    REQUIRE_FALSE(document->AppendChild(*element).HasException());

    auto newElement = CreateRef<TestElement>(*document);
    auto newDoctype = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").ReleaseValue();

    // Without exclusions, a second element or doctype is refused.
    CHECK(MutationAlgorithms::EnsurePreInsertValidity(*newElement, *document, nullptr).HasException());
    CHECK(MutationAlgorithms::EnsurePreInsertValidity(*newDoctype, *document, element.get()).HasException());

    // Excluding the one there, as replace does, allows them.
    Node *excludeElement[] {element.get()};
    CHECK_FALSE(
      MutationAlgorithms::EnsurePreInsertValidity(*newElement, *document, element.get(), excludeElement)
        .HasException());
    Node *excludeDoctype[] {doctype.get()};
    CHECK_FALSE(
      MutationAlgorithms::EnsurePreInsertValidity(*newDoctype, *document, doctype.get(), excludeDoctype)
        .HasException());

    // An excluded doctype child still can't have an element inserted before it.
    CHECK(MutationAlgorithms::EnsurePreInsertValidity(*newElement, *document, doctype.get(), excludeDoctype)
            .HasException());
  }

  TEST_CASE("Move: assigns slots, signals slot changes and adds transient registered observers",
            "[DOM][MutationAlgorithms][dom:move]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*root).HasException());
    auto oldParent = document->CreateElement(u8"div").ReleaseValue();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(root->AppendChild(*oldParent).HasException());
    REQUIRE_FALSE(root->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
    auto slot = document->CreateElement(u8"slot").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
    auto span = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(oldParent->AppendChild(*span).HasException());

    SECTION("Moving a slottable into a shadow host assigns it a slot")
    {
      REQUIRE_FALSE(MutationAlgorithms::Move(*span, *host, nullptr).HasException());
      CHECK(SlotAlgorithms::GetAssignedSlot(*span) == slot.get());
    }

    SECTION("Moving into an empty slot signals a slot change")
    {
      int changes = 0;
      auto listener = slot->AddEventListener(DOMStringAtom(u8"slotchange"), [&](Event &) { ++changes; });
      auto fallback = document->CreateElement(u8"b").ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*fallback).HasException());
      loop.PerformMicrotaskCheckpoint();
      changes = 0;

      REQUIRE_FALSE(MutationAlgorithms::Move(*fallback, *slot, nullptr).HasException());
      loop.PerformMicrotaskCheckpoint();
      CHECK(changes == 1);
    }

    SECTION("An observer of the old parent's subtree keeps seeing the moved node until it's notified")
    {
      auto observer = NewCollectingObserver();
      REQUIRE_FALSE(observer->Observe(*oldParent, {.ChildList = true, .Subtree = true}).HasException());

      REQUIRE_FALSE(MutationAlgorithms::Move(*span, *root, nullptr).HasException());
      REQUIRE_FALSE(span->AppendChild(*document->CreateElement(u8"i").ReleaseValue()).HasException());

      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 2);
      CHECK(records[0]->Target().get() == oldParent.get());
      CHECK(records[1]->Target().get() == span.get());
    }
  }

  TEST_CASE_METHOD(html::tests::CustomElementFixture,
                   "Replace: the node is adopted, leaving its old parent, before the child is removed",
                   "[DOM][MutationAlgorithms][dom:concept-node-replace]")
  {
    Define(u8"x-a");
    Parse(u8"<x-a id=old></x-a><div><x-a id=new></x-a></div>");
    (void)TakeCalls();

    auto &oldChild = ById(u8"old");
    auto &newChild = ById(u8"new");
    REQUIRE_FALSE(Body().ReplaceChild(newChild, oldChild).HasException());
    CHECK(TakeCalls() == html::tests::Log {u8"disconnected new", u8"connected new", u8"disconnected old"});
  }
}
