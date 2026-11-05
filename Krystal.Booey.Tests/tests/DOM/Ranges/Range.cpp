#include "Krystal.Booey/DOM/Ranges/Range.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/Comment.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Ranges/StaticRange.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Node;
      Ref<Range> Range;

      CommonTestData()
          : Document(CreateRef<HTMLDocument>()), Node(CreateRef<TestElement>(*Document)),
            Range(Document->CreateRange())
      {
      }
    };
  }

  TEST_CASE(
    "Range.createContextualFragment(): the markup parsed in the context of the start node",
    "[DOM][Range][html:the-createcontextualfragment()-method][html:dom-range-createcontextualfragment]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto parser = CreateRef<html::DocumentParser>(*document);
    parser->InputStream().Append(u8"<!DOCTYPE html><table id=t></table><p id=p>text</p>", IsEOF(true));
    parser->PumpTokenizer();
    auto range = document->CreateRange();

    SECTION("in a table, table rows are kept, in an implied tbody")
    {
      REQUIRE_FALSE(range->SetStart(*document->GetElementById(u8"t"), 0).HasException());
      auto fragment = range->CreateContextualFragment(u8"<tr><td>cell</td></tr>");
      REQUIRE_FALSE(fragment.HasException());
      auto *tbody = DynamicDowncast<Element>(fragment.Value()->FirstChild());
      REQUIRE(tbody);
      CHECK(tbody->LocalName() == u8"tbody");
      REQUIRE(Is<Element>(tbody->FirstChild()));
      CHECK(Downcast<Element>(*tbody->FirstChild()).LocalName() == u8"tr");
    }

    SECTION("in text, the context is its parent element")
    {
      REQUIRE_FALSE(range->SetStart(*document->GetElementById(u8"p")->FirstChild(), 1).HasException());
      auto fragment = range->CreateContextualFragment(u8"<tr><td>cell</td></tr>");
      REQUIRE_FALSE(fragment.HasException());
      // Outside a table, the row's tags are ignored.
      CHECK(fragment.Value()->TextContent() == u8"cell");
      REQUIRE(Is<Text>(fragment.Value()->FirstChild()));
    }

    SECTION("in the document or the html element, the context is a body")
    {
      for (Node *start :
           {static_cast<Node *>(document.get()), static_cast<Node *>(document->DocumentElement().get())})
      {
        REQUIRE_FALSE(range->SetStart(*start, 0).HasException());
        auto fragment = range->CreateContextualFragment(u8"<b>bold</b>");
        REQUIRE_FALSE(fragment.HasException());
        REQUIRE(Is<Element>(fragment.Value()->FirstChild()));
        CHECK(Downcast<Element>(*fragment.Value()->FirstChild()).LocalName() == u8"b");
      }
    }
  }

  TEST_CASE("Range.commonAncestorContainer: the common ancestor of its start and end nodes",
            "[DOM][Range][dom:dom-range-commonancestorcontainer][dom:get-the-common-ancestor]")
  {
    CommonTestData data;

    auto textNode1 = data.Document->CreateTextNode(u8"test1");
    auto textNode2 = data.Document->CreateTextNode(u8"test2");

    REQUIRE_FALSE(data.Node->AppendChild(*textNode1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*textNode2).HasException());

    REQUIRE_FALSE(data.Range->SetStart(*textNode1, 0).HasException());
    REQUIRE_FALSE(data.Range->SetEnd(*textNode2, 0).HasException());

    REQUIRE(data.Range->CommonAncestorContainer() == data.Node);

    REQUIRE_FALSE(data.Node->RemoveChild(*textNode1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*textNode2).HasException());
  }

  TEST_CASE("Range.setStart(): sets the start, and the end too if the start would be after it",
            "[DOM][Range][dom:dom-range-setstart][dom:concept-range-bp-set]")
  {
    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      CommonTestData data;

      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      auto setStartResult = data.Range->SetStart(*doctype.Value(), 0);
      REQUIRE(setStartResult.HasException());
      REQUIRE(setStartResult.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("IndexSizeError if offset is greater than the length of the node's data")
    {
      CommonTestData data;

      auto textNode = data.Document->CreateTextNode(u8"test");
      REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

      auto setStartResult = data.Range->SetStart(*textNode, 5);
      REQUIRE(setStartResult.HasException());
      REQUIRE(setStartResult.GetException().Code() == ExceptionCode::IndexSizeError);
    }

    SECTION("Set start boundary point to the given node and offset")
    {
      CommonTestData data;

      auto textNode = data.Document->CreateTextNode(u8"test");
      REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

      auto setStartResult = data.Range->SetStart(*textNode, 2);
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 2);
    }

    SECTION("If the new start boundary point is after the end boundary point, set the end boundary point to "
            "be the same as the start boundary point")
    {
      CommonTestData data;

      auto textNode = data.Document->CreateTextNode(u8"test");
      REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

      auto setStartResult = data.Range->SetStart(*textNode, 2);
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 2);
      REQUIRE(data.Range->EndContainer() == textNode);
      REQUIRE(data.Range->EndOffset() == 2);
    }

    SECTION("If the new start boundary point is in a different root than the end boundary point, set the end "
            "boundary point to be the same as the start boundary point")
    {
      CommonTestData data;
      CommonTestData otherData;

      auto setStartResult = data.Range->SetStart(*otherData.Node, 0uz);
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == otherData.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == otherData.Node);
      REQUIRE(data.Range->EndOffset() == 0uz);
    }
  }

  TEST_CASE("Range.setEnd(): sets the end, and the start too if the end would be before it",
            "[DOM][Range][dom:dom-range-setend][dom:concept-range-bp-set]")
  {
    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      CommonTestData data;

      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      auto setEndResult = data.Range->SetEnd(*doctype.Value(), 0);
      REQUIRE(setEndResult.HasException());
      REQUIRE(setEndResult.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("IndexSizeError if offset is greater than the length of the node's data")
    {
      CommonTestData data;

      auto textNode = data.Document->CreateTextNode(u8"test");
      REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

      auto setEndResult = data.Range->SetEnd(*textNode, 5);
      REQUIRE(setEndResult.HasException());
      REQUIRE(setEndResult.GetException().Code() == ExceptionCode::IndexSizeError);
    }

    SECTION("Set end boundary point to the given node and offset")
    {
      CommonTestData data;

      auto textNode = data.Document->CreateTextNode(u8"test");
      REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

      auto setEndResult = data.Range->SetEnd(*textNode, 2);
      REQUIRE_FALSE(setEndResult.HasException());
      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 2);
    }

    SECTION("If the new end boundary point is after the start boundary point, set the start boundary point to"
            "be the same as the end boundary point")
    {
      CommonTestData data;

      auto textNode = data.Document->CreateTextNode(u8"test");
      REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

      auto setEndResult = data.Range->SetEnd(*textNode, 2);
      REQUIRE_FALSE(setEndResult.HasException());
      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 2);
      REQUIRE(data.Range->EndContainer() == textNode);
      REQUIRE(data.Range->EndOffset() == 2);
    }

    SECTION(
      "If the new end boundary point is in a different root than the start boundary point, set the start"
      "boundary point to be the same as the end boundary point")
    {
      CommonTestData data;
      CommonTestData otherData;

      auto setEndResult = data.Range->SetEnd(*otherData.Node, 0uz);
      REQUIRE_FALSE(setEndResult.HasException());
      REQUIRE(data.Range->StartContainer() == otherData.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == otherData.Node);
      REQUIRE(data.Range->EndOffset() == 0uz);
    }
  }

  TEST_CASE("Range.setStartBefore(): sets the start to before the node in its parent",
            "[DOM][Range][dom:dom-range-setstartbefore]")
  {
    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      CommonTestData data;
      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());
      auto result = data.Range->SetStartBefore(*doctype.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("InvalidNodeTypeError if node does not have a parent")
    {
      CommonTestData data;

      auto orphanNode = data.Document->CreateElement(u8"orphan");
      REQUIRE_FALSE(orphanNode.HasException());

      auto result = data.Range->SetStartBefore(*orphanNode.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("Set start boundary point to be before the given node")
    {
      CommonTestData data;

      auto child1 = data.Document->CreateElement(u8"child1");
      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());

      auto child2 = data.Document->CreateElement(u8"child2");
      REQUIRE_FALSE(child2.HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto result = data.Range->SetStartBefore(*child2.Value());
      REQUIRE_FALSE(result.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
    }

    SECTION("If the new start boundary point is after the end boundary point, set the end boundary point to "
            "be the same as the start boundary point")
    {
      CommonTestData data;

      auto child1 = data.Document->CreateElement(u8"child1");
      auto child2 = data.Document->CreateElement(u8"child1");

      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(child2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto setStartResult = data.Range->SetStartBefore(*child2.Value());
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2.Value()).HasException());
    }

    SECTION("If the new start boundary point is in a different root than the end boundary point, set the end "
            "boundary point to be the same as the start boundary point")
    {
      CommonTestData data;
      CommonTestData otherData;

      auto child1 = otherData.Document->CreateElement(u8"child1");
      auto child2 = otherData.Document->CreateElement(u8"child1");

      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(child2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto setStartResult = data.Range->SetStartBefore(*child2.Value());
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2.Value()).HasException());
    }
  }

  TEST_CASE("Range.setEndBefore(): sets the end to before the node in its parent",
            "[DOM][Range][dom:dom-range-setendbefore]")
  {
    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      CommonTestData data;

      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      auto result = data.Range->SetEndBefore(*doctype.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("InvalidNodeTypeError if node does not have a parent")
    {
      CommonTestData data;

      auto orphanNode = data.Document->CreateElement(u8"orphan");
      REQUIRE_FALSE(orphanNode.HasException());

      auto result = data.Range->SetEndBefore(*orphanNode.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("Set end boundary point to be before the given node")
    {
      CommonTestData data;

      auto child1 = data.Document->CreateElement(u8"child1");
      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());

      auto child2 = data.Document->CreateElement(u8"child2");
      REQUIRE_FALSE(child2.HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto result = data.Range->SetEndBefore(*child2.Value());
      REQUIRE_FALSE(result.HasException());
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);
    }

    SECTION(
      "If the new end boundary point is after the start boundary point, set the start boundary point to "
      "be the same as the end boundary point")
    {
      CommonTestData data;

      auto child1 = data.Document->CreateElement(u8"child1");
      auto child2 = data.Document->CreateElement(u8"child1");

      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(child2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto setStartResult = data.Range->SetEndBefore(*child2.Value());
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2.Value()).HasException());
    }

    SECTION(
      "If the new end boundary point is in a different root than the start boundary point, set the start "
      "boundary point to be the same as the end boundary point")
    {
      CommonTestData data;
      CommonTestData otherData;

      auto child1 = otherData.Document->CreateElement(u8"child1");
      auto child2 = otherData.Document->CreateElement(u8"child1");

      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(child2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto setStartResult = data.Range->SetEndBefore(*child2.Value());
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2.Value()).HasException());
    }
  }

  TEST_CASE("Range.setStartAfter(): sets the start to after the node in its parent",
            "[DOM][Range][dom:dom-range-setstartafter]")
  {
    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      CommonTestData data;

      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      auto result = data.Range->SetStartAfter(*doctype.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("InvalidNodeTypeError if node does not have a parent")
    {
      CommonTestData data;

      auto orphanNode = data.Document->CreateElement(u8"orphan");
      REQUIRE_FALSE(orphanNode.HasException());

      auto result = data.Range->SetStartAfter(*orphanNode.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("Set start boundary point to be before the given node")
    {
      CommonTestData data;

      auto child1 = data.Document->CreateElement(u8"child1");
      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());

      auto child2 = data.Document->CreateElement(u8"child2");
      REQUIRE_FALSE(child2.HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto result = data.Range->SetStartAfter(*child1.Value());
      REQUIRE_FALSE(result.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
    }

    SECTION("If the new start boundary point is after the end boundary point, set the end boundary point to "
            "be the same as the start boundary point")
    {
      CommonTestData data;

      auto child1 = data.Document->CreateElement(u8"child1");
      auto child2 = data.Document->CreateElement(u8"child1");

      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(child2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto setStartResult = data.Range->SetStartAfter(*child1.Value());
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2.Value()).HasException());
    }

    SECTION("If the new start boundary point is in a different root than the end boundary point, set the end "
            "boundary point to be the same as the start boundary point")
    {
      CommonTestData data;
      CommonTestData otherData;

      auto child1 = otherData.Document->CreateElement(u8"child1");
      auto child2 = otherData.Document->CreateElement(u8"child1");

      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(child2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto setStartResult = data.Range->SetStartAfter(*child1.Value());
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2.Value()).HasException());
    }
  }

  TEST_CASE("Range.setEndAfter(): sets the end to after the node in its parent",
            "[DOM][Range][dom:dom-range-setendafter]")
  {
    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      CommonTestData data;

      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      auto result = data.Range->SetEndAfter(*doctype.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("InvalidNodeTypeError if node does not have a parent")
    {
      CommonTestData data;

      auto orphanNode = data.Document->CreateElement(u8"orphan");
      REQUIRE_FALSE(orphanNode.HasException());

      auto result = data.Range->SetEndAfter(*orphanNode.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("Set end boundary point to be before the given node")
    {
      CommonTestData data;

      auto child1 = data.Document->CreateElement(u8"child1");
      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());

      auto child2 = data.Document->CreateElement(u8"child2");
      REQUIRE_FALSE(child2.HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto result = data.Range->SetEndAfter(*child1.Value());
      REQUIRE_FALSE(result.HasException());
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);
    }

    SECTION(
      "If the new end boundary point is after the start boundary point, set the start boundary point to "
      "be the same as the end boundary point")
    {
      CommonTestData data;

      auto child1 = data.Document->CreateElement(u8"child1");
      auto child2 = data.Document->CreateElement(u8"child1");

      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(child2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto setStartResult = data.Range->SetEndAfter(*child1.Value());
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2.Value()).HasException());
    }

    SECTION(
      "If the new end boundary point is in a different root than the start boundary point, set the start "
      "boundary point to be the same as the end boundary point")
    {
      CommonTestData data;
      CommonTestData otherData;

      auto child1 = otherData.Document->CreateElement(u8"child1");
      auto child2 = otherData.Document->CreateElement(u8"child1");

      REQUIRE_FALSE(child1.HasException());
      REQUIRE_FALSE(child2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2.Value()).HasException());

      auto setStartResult = data.Range->SetEndAfter(*child1.Value());
      REQUIRE_FALSE(setStartResult.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 1uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2.Value()).HasException());
    }
  }

  TEST_CASE("Range.collapsed: whether its start is its end", "[DOM][Range][dom:dom-range-collapsed]")
  {
    CommonTestData data;

    SECTION("True if the start and end boundary points are the same")
    {
      REQUIRE(data.Range->Collapsed());
    }

    SECTION("False if the start and end boundary points are different")
    {
      auto child1 = CreateRef<TestElement>(*data.Document);
      auto child2 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*child2, 0uz).HasException());
      REQUIRE_FALSE(data.Range->Collapsed());

      REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    }
  }

  TEST_CASE("Range.collapse(): sets the end to the start, or the start to the end",
            "[DOM][Range][dom:dom-range-collapse]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

    REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
    REQUIRE_FALSE(data.Range->SetEnd(*child2, 0uz).HasException());

    SECTION("collapses to the start boundary point if the argument is true")
    {
      data.Range->Collapse(true);
      REQUIRE(data.Range->Collapsed());
      REQUIRE(data.Range->StartContainer() == child1);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == child1);
      REQUIRE(data.Range->EndOffset() == 0uz);
    }

    SECTION("collapses to the end boundary point if the argument is false")
    {
      data.Range->Collapse(false);
      REQUIRE(data.Range->Collapsed());
      REQUIRE(data.Range->StartContainer() == child2);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == child2);
      REQUIRE(data.Range->EndOffset() == 0uz);
    }

    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
  }

  TEST_CASE("Range.selectNode(): selects the node within it",
            "[DOM][Range][dom:dom-range-selectnode][dom:concept-range-select]")
  {
    CommonTestData data;

    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());
      auto result = data.Range->SelectNode(*doctype.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("InvalidNodeTypeError if node does not have a parent")
    {
      auto orphanNode = data.Document->CreateElement(u8"orphan");
      REQUIRE_FALSE(orphanNode.HasException());

      auto result = data.Range->SelectNode(*orphanNode.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("Selects the given node")
    {
      auto child = CreateRef<TestElement>(*data.Document);
      REQUIRE_FALSE(data.Node->AppendChild(*child).HasException());

      auto result = data.Range->SelectNode(*child);
      REQUIRE_FALSE(result.HasException());
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child).HasException());
    }
  }

  TEST_CASE("Range.selectNodeContents(): from the start to the end of the node",
            "[DOM][Range][dom:dom-range-selectnodecontents]")
  {
    CommonTestData data;

    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      auto result = data.Range->SelectNodeContents(*doctype.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("Selects the contents of the given node")
    {
      auto child1 = CreateRef<TestElement>(*data.Document);
      auto child2 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

      auto result = data.Range->SelectNodeContents(*child1);
      REQUIRE_FALSE(result.HasException());

      REQUIRE(data.Range->StartContainer() == child1);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == child1);
      REQUIRE(data.Range->EndOffset() == 0uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    }
  }

  TEST_CASE(
    "Range.compareBoundaryPoints(): the position of one of its boundary points relative to another range's",
    "[DOM][Range][dom:dom-range-compareboundarypoints]")
  {
    CommonTestData data;

    SECTION("WrongDocumentError if otherRange is from a different document")
    {
      auto document = CreateRef<HTMLDocument>();
      auto otherRange = document->CreateRange();
      auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::StartToStart, *otherRange);

      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::WrongDocumentError);
    }

    SECTION("NotSupportedError if BoundaryPointComparator is out of range")
    {
      auto result = data.Range->CompareBoundaryPoints(static_cast<BoundaryPointComparator>(4), *data.Range);
      REQUIRE(result == ExceptionCode::NotSupportedError);
    }

    auto otherRange = data.Document->CreateRange();

    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);
    auto child3 = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());

    SECTION("StartToStart")
    {
      SECTION(
        "Returns strong_ordering::less if the boundary point of this range is before the boundary point "
        "of otherRange")
      {
        REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetStart(*child2, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::StartToStart, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::less);
      }

      SECTION("Returns strong_ordering::equal if the boundary point of this range is equal to the boundary "
              "point of otherRange")
      {
        REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetStart(*child1, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::StartToStart, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::equal);
      }

      SECTION(
        "Returns strong_ordering::greater if the boundary point of this range is after the boundary point of "
        "otherRange")
      {
        REQUIRE_FALSE(data.Range->SetStart(*child2, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetStart(*child1, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::StartToStart, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::greater);
      }
    }

    SECTION("StartToEnd")
    {
      SECTION(
        "Returns strong_ordering::less if the boundary point of this range is before the boundary point "
        "of otherRange")
      {
        REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetEnd(*child2, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::StartToEnd, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::less);
      }

      SECTION("Returns strong_ordering::equal if the boundary point of this range is equal to the boundary "
              "point of otherRange")
      {
        REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetEnd(*child1, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::StartToEnd, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::equal);
      }

      SECTION(
        "Returns strong_ordering::greater if the boundary point of this range is after the boundary point of "
        "otherRange")
      {
        REQUIRE_FALSE(data.Range->SetStart(*child2, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetEnd(*child1, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::StartToEnd, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::greater);
      }
    }

    SECTION("EndToEnd")
    {
      SECTION(
        "Returns strong_ordering::less if the boundary point of this range is before the boundary point "
        "of otherRange")
      {
        REQUIRE_FALSE(data.Range->SetEnd(*child1, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetEnd(*child2, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::EndToEnd, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::less);
      }

      SECTION("Returns strong_ordering::equal if the boundary point of this range is equal to the boundary "
              "point of otherRange")
      {
        REQUIRE_FALSE(data.Range->SetEnd(*child1, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetEnd(*child1, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::EndToEnd, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::equal);
      }

      SECTION(
        "Returns strong_ordering::greater if the boundary point of this range is after the boundary point of "
        "otherRange")
      {
        REQUIRE_FALSE(data.Range->SetEnd(*child2, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetEnd(*child1, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::EndToEnd, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::greater);
      }
    }

    SECTION("EndToStart")
    {
      SECTION(
        "Returns strong_ordering::less if the boundary point of this range is before the boundary point "
        "of otherRange")
      {
        REQUIRE_FALSE(data.Range->SetEnd(*child1, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetStart(*child2, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::EndToStart, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::less);
      }

      SECTION("Returns strong_ordering::equal if the boundary point of this range is equal to the boundary "
              "point of otherRange")
      {
        REQUIRE_FALSE(data.Range->SetEnd(*child1, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetStart(*child1, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::EndToStart, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::equal);
      }

      SECTION(
        "Returns strong_ordering::greater if the boundary point of this range is after the boundary point of "
        "otherRange")
      {
        REQUIRE_FALSE(data.Range->SetEnd(*child2, 0uz).HasException());
        REQUIRE_FALSE(otherRange->SetStart(*child1, 0uz).HasException());

        auto result = data.Range->CompareBoundaryPoints(BoundaryPointComparator::EndToStart, *otherRange);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value() == std::strong_ordering::greater);
      }
    }

    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
  }

  TEST_CASE("Range.deleteContents(): removes what it contains, collapsing it",
            "[DOM][Range][dom:dom-range-deletecontents]")
  {
    CommonTestData data;

    SECTION("When start and end container are the same and it is a CharacterData node, deletes the content "
            "from start to end offset")
    {
      auto textNode = data.Document->CreateTextNode(u8"Hello, world!");

      REQUIRE_FALSE(data.Range->SetStart(*textNode, 7uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*textNode, 12uz).HasException());

      REQUIRE_FALSE(data.Range->DeleteContents().HasException());

      REQUIRE(textNode->Data() == u8"Hello, !");
      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 7uz);
      REQUIRE(data.Range->EndContainer() == textNode);
      REQUIRE(data.Range->EndOffset() == 7uz);
    }

    SECTION("Deletes the content between the start and end boundary points")
    {
      auto child1 = CreateRef<TestElement>(*data.Document);
      auto child2 = CreateRef<TestElement>(*data.Document);
      auto child3 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*data.Node, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*data.Node, 2uz).HasException());

      REQUIRE_FALSE(data.Range->DeleteContents().HasException());

      REQUIRE(data.Node->ChildNodes()->Length() == 1uz);
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 0uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
    }
  }

  TEST_CASE("Range.extractContents(): moves what it contains into a fragment",
            "[DOM][Range][dom:dom-range-extractcontents][dom:concept-range-extract]")
  {
    CommonTestData data;

    SECTION("Collapsed range returns an empty document fragment")
    {
      auto result = data.Range->ExtractContents();
      REQUIRE_FALSE(result.HasException());

      auto &fragment = result.Value();
      REQUIRE(fragment->ChildNodes()->Length() == 0uz);
    }

    SECTION("When start and end container are the same and it is a CharacterData node, extracts the content "
            "from start to end offset into a document fragment")
    {
      auto textNode = data.Document->CreateTextNode(u8"Hello, world!");

      REQUIRE_FALSE(data.Range->SetStart(*textNode, 7uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*textNode, 12uz).HasException());

      auto result = data.Range->ExtractContents();
      REQUIRE_FALSE(result.HasException());

      auto &fragment = result.Value();
      REQUIRE(fragment->ChildNodes()->Length() == 1uz);

      auto extractedTextNode = fragment->ChildNodes()->Item(0uz);
      REQUIRE(extractedTextNode);
      REQUIRE(extractedTextNode->NodeType() == NodeType::TEXT_NODE);

      REQUIRE(Downcast<dom::Text>(extractedTextNode.get())->Data() == u8"world");
      REQUIRE(textNode->Data() == u8"Hello, !");

      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 7uz);
      REQUIRE(data.Range->EndContainer() == textNode);
      REQUIRE(data.Range->EndOffset() == 7uz);
    }

    SECTION("Extracts the content between the start and end boundary points into a document fragment")
    {
      auto child1 = CreateRef<TestElement>(*data.Document);
      auto child2 = CreateRef<TestElement>(*data.Document);
      auto child3 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*data.Node, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*data.Node, 2uz).HasException());

      auto result = data.Range->ExtractContents();
      REQUIRE_FALSE(result.HasException());

      auto &fragment = result.Value();
      REQUIRE(fragment->ChildNodes()->Length() == 2uz);

      auto extractedChild1 = fragment->ChildNodes()->Item(0uz);
      REQUIRE(extractedChild1);
      REQUIRE(extractedChild1 == child1);

      auto extractedChild2 = fragment->ChildNodes()->Item(1uz);
      REQUIRE(extractedChild2);
      REQUIRE(extractedChild2 == child2);
      REQUIRE(data.Node->ChildNodes()->Length() == 1uz);
      REQUIRE(data.Node->ChildNodes()->Item(0uz) == child3);
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 0uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
    }
  }

  TEST_CASE("Range.cloneContents(): copies what it contains into a fragment",
            "[DOM][Range][dom:dom-range-clonecontents][dom:concept-range-clone]")
  {
    CommonTestData data;

    SECTION("Collapsed range returns an empty document fragment")
    {
      auto result = data.Range->CloneContents();
      REQUIRE_FALSE(result.HasException());

      auto &fragment = result.Value();
      REQUIRE(fragment->ChildNodes()->Length() == 0uz);
    }

    SECTION("When start and end container are the same and it is a CharacterData node, clones the content "
            "from start to end offset into a text node appended to a document fragment")
    {
      auto textNode = data.Document->CreateTextNode(u8"Hello, world!");

      REQUIRE_FALSE(data.Range->SetStart(*textNode, 7uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*textNode, 12uz).HasException());

      auto result = data.Range->CloneContents();
      REQUIRE_FALSE(result.HasException());

      auto &fragment = result.Value();
      REQUIRE(fragment->ChildNodes()->Length() == 1uz);

      auto extractedTextNode = fragment->ChildNodes()->Item(0uz);
      REQUIRE(extractedTextNode);
      REQUIRE(extractedTextNode->NodeType() == NodeType::TEXT_NODE);

      REQUIRE(Downcast<dom::Text>(extractedTextNode.get())->Data() == u8"world");
      REQUIRE(textNode->Data() == u8"Hello, world!");

      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 7uz);
      REQUIRE(data.Range->EndContainer() == textNode);
      REQUIRE(data.Range->EndOffset() == 12uz);
    }

    SECTION("Clones the content between the start and end boundary points into a document fragment")
    {
      auto child1 = CreateRef<TestElement>(*data.Document);
      auto child2 = CreateRef<TestElement>(*data.Document);
      auto child3 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*data.Node, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*data.Node, 2uz).HasException());

      auto result = data.Range->CloneContents();
      REQUIRE_FALSE(result.HasException());

      auto &fragment = result.Value();
      REQUIRE(fragment->ChildNodes()->Length() == 2uz);

      REQUIRE(data.Node->ChildNodes()->Length() == 3uz);
      REQUIRE(data.Node->ChildNodes()->Item(0uz) == child1);
      REQUIRE(data.Node->ChildNodes()->Item(1uz) == child2);
      REQUIRE(data.Node->ChildNodes()->Item(2uz) == child3);
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 2uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
    }
  }

  TEST_CASE("Range.insertNode(): inserts the node at its start",
            "[DOM][Range][dom:dom-range-insertnode][dom:concept-range-insert]")
  {
    CommonTestData data;
    auto newNode = CreateRef<TestElement>(*data.Document);

    SECTION("HierarchyRequestError if start container is a ProcessingInstruction node")
    {
      auto node = *data.Document->CreateProcessingInstruction(u8"target", u8"data");
      REQUIRE_FALSE(data.Range->SetStart(*node, 0uz).HasException());
      REQUIRE(data.Range->InsertNode(*node) == ExceptionCode::HierarchyRequestError);
    }

    SECTION("HierarchyRequestError if start container is a Comment node")
    {
      auto node = CreateRef<Comment>(*data.Document, u8"data");
      REQUIRE_FALSE(data.Range->SetStart(*node, 0uz).HasException());
      REQUIRE(data.Range->InsertNode(*node) == ExceptionCode::HierarchyRequestError);
    }

    SECTION("HierarchyRequestError if start container is a Text node with a null parent")
    {
      auto node = CreateRef<dom::Text>(*data.Document, u8"data");
      REQUIRE_FALSE(data.Range->SetStart(*node, 0uz).HasException());
      REQUIRE(data.Range->InsertNode(*node) == ExceptionCode::HierarchyRequestError);
    }

    SECTION("HierarchyRequestError if newNode is the start container")
    {
      REQUIRE_FALSE(data.Range->SetStart(*newNode, 0uz).HasException());
      REQUIRE(data.Range->InsertNode(*newNode) == ExceptionCode::HierarchyRequestError);
    }

    SECTION("Inserts the node at the start boundary point")
    {
      REQUIRE_FALSE(data.Node->AppendChild(*newNode).HasException());
      REQUIRE_FALSE(data.Range->SetStart(*data.Node, 0uz).HasException());

      REQUIRE_FALSE(data.Range->InsertNode(*newNode).HasException());

      REQUIRE(data.Node->ChildNodes()->Length() == 1uz);
      REQUIRE(data.Node->ChildNodes()->Item(0uz) == newNode);
      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);
      REQUIRE_FALSE(data.Node->RemoveChild(*newNode).HasException());
    }
  }

  TEST_CASE("Range.surroundContents(): moves what it contains into the new parent, which it then selects",
            "[DOM][Range][dom:dom-range-surroundcontents]")
  {
    CommonTestData data;
    auto newNode = CreateRef<TestElement>(*data.Document);

    SECTION("InvalidStateError if range partially contains a non-Text node")
    {
      auto comment = CreateRef<Comment>(*data.Document, u8"Hello, world!");
      auto otherChild = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*comment).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*otherChild).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*comment, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*otherChild, 0uz).HasException());

      REQUIRE(data.Range->SurroundContents(*newNode) == ExceptionCode::InvalidStateError);

      REQUIRE_FALSE(data.Node->RemoveChild(*comment).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*otherChild).HasException());
    }

    SECTION("InvalidNodeTypeError if newParent is a Document node")
    {
      auto document = CreateRef<HTMLDocument>();
      REQUIRE(data.Range->SurroundContents(*document) == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("InvalidNodeTypeError if newParent is a DocumentFragment node")
    {
      auto fragment = CreateRef<DocumentFragment>(*data.Document);
      REQUIRE(data.Range->SurroundContents(*fragment) == ExceptionCode::InvalidNodeTypeError);
    }

    SECTION("Surrounds the contents of the range with the given node")
    {
      auto child1 = CreateRef<TestElement>(*data.Document);
      auto child2 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*data.Node, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*data.Node, 2uz).HasException());

      REQUIRE_FALSE(data.Range->SurroundContents(*newNode).HasException());

      REQUIRE(data.Node->ChildNodes()->Length() == 1uz);
      REQUIRE(data.Node->ChildNodes()->Item(0uz) == newNode);

      REQUIRE(newNode->ChildNodes()->Length() == 2uz);
      REQUIRE(newNode->ChildNodes()->Item(0uz) == child1);
      REQUIRE(newNode->ChildNodes()->Item(1uz) == child2);

      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);

      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*newNode).HasException());
      REQUIRE_FALSE(newNode->RemoveChild(*child1).HasException());
      REQUIRE_FALSE(newNode->RemoveChild(*child2).HasException());
    }

    SECTION("newParent's children are replaced with the contents of the range")
    {
      auto child1 = CreateRef<TestElement>(*data.Document);
      auto child2 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*data.Node, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*data.Node, 2uz).HasException());

      auto newNodeChild1 = CreateRef<TestElement>(*data.Document);
      auto newNodeChild2 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(newNode->AppendChild(*newNodeChild1).HasException());
      REQUIRE_FALSE(newNode->AppendChild(*newNodeChild2).HasException());

      REQUIRE_FALSE(data.Range->SurroundContents(*newNode).HasException());

      REQUIRE(data.Node->ChildNodes()->Length() == 1uz);
      REQUIRE(data.Node->ChildNodes()->Item(0uz) == newNode);

      REQUIRE(newNode->ChildNodes()->Length() == 2uz);
      REQUIRE(newNode->ChildNodes()->Item(0uz) == child1);
      REQUIRE(newNode->ChildNodes()->Item(1uz) == child2);

      REQUIRE(data.Range->StartContainer() == data.Node);
      REQUIRE(data.Range->StartOffset() == 0uz);

      REQUIRE(data.Range->EndContainer() == data.Node);
      REQUIRE(data.Range->EndOffset() == 1uz);

      REQUIRE_FALSE(data.Node->RemoveChild(*newNode).HasException());
      REQUIRE_FALSE(newNode->RemoveChild(*child1).HasException());
      REQUIRE_FALSE(newNode->RemoveChild(*child2).HasException());
    }
  }

  TEST_CASE("Range.cloneRange(): a new live range with its start and end",
            "[DOM][Range][dom:dom-range-clonerange]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);
    auto child3 = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());

    REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
    REQUIRE_FALSE(data.Range->SetEnd(*child3, 0uz).HasException());

    auto clone = data.Range->CloneRange();

    REQUIRE(clone->StartContainer() == data.Range->StartContainer());
    REQUIRE(clone->StartOffset() == data.Range->StartOffset());
    REQUIRE(clone->EndContainer() == data.Range->EndContainer());
    REQUIRE(clone->EndOffset() == data.Range->EndOffset());

    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
  }

  TEST_CASE("Range.isPointInRange(): whether the point is between its start and end, inclusive",
            "[DOM][Range][dom:dom-range-ispointinrange]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);
    auto child3 = CreateRef<TestElement>(*data.Document);
    auto child4 = CreateRef<TestElement>(*data.Document);
    auto child5 = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child4).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child5).HasException());

    SECTION("False if node is in another tree, before checking the node and offset")
    {
      auto otherDocument = CreateRef<HTMLDocument>();
      auto otherNode = CreateRef<TestElement>(*otherDocument);
      auto result = data.Range->IsPointInRange(*otherNode, 0uz);
      REQUIRE_FALSE(result.HasException());
      CHECK_FALSE(result.Value());

      auto doctype = otherDocument->Implementation().CreateDocumentType(u8"html", u8"", u8"").ReleaseValue();
      result = data.Range->IsPointInRange(*doctype, 0uz);
      REQUIRE_FALSE(result.HasException());
      CHECK_FALSE(result.Value());
      result = data.Range->IsPointInRange(*otherNode, 99uz);
      REQUIRE_FALSE(result.HasException());
      CHECK_FALSE(result.Value());
    }

    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      REQUIRE_FALSE(data.Document->InsertBefore(*doctype.Value(), data.Node.get()).HasException());

      auto result = data.Range->IsPointInRange(*doctype.Value(), 0uz);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);

      REQUIRE_FALSE(data.Document->RemoveChild(*doctype.Value()).HasException());
    }

    SECTION("IndexSizeError if offset is greater than the length of the node")
    {
      auto result = data.Range->IsPointInRange(*child3, 1uz);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::IndexSizeError);
    }

    REQUIRE_FALSE(data.Range->SetStart(*child2, 0uz).HasException());
    REQUIRE_FALSE(data.Range->SetEnd(*child4, 0uz).HasException());

    SECTION("True if the point is between the start and end boundary points")
    {
      REQUIRE(data.Range->IsPointInRange(*child3, 0uz) == true);
    }
    SECTION("False if the point is before the start boundary point")
    {
      REQUIRE(data.Range->IsPointInRange(*child1, 0uz) == false);
    }
    SECTION("False if the point is after the end boundary point")
    {
      REQUIRE(data.Range->IsPointInRange(*child5, 0uz) == false);
    }
    SECTION("True if the point is at the start boundary point")
    {
      REQUIRE(data.Range->IsPointInRange(*child2, 0uz) == true);
    }
    SECTION("True if the point is at the end boundary point")
    {
      REQUIRE(data.Range->IsPointInRange(*child4, 0uz) == true);
    }

    REQUIRE_FALSE(data.Document->RemoveChild(*data.Node).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child4).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child5).HasException());
  }

  TEST_CASE("Range.comparePoint(): whether the point is before, in or after it",
            "[DOM][Range][dom:dom-range-comparepoint]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);
    auto child3 = CreateRef<TestElement>(*data.Document);
    auto child4 = CreateRef<TestElement>(*data.Document);
    auto child5 = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child4).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child5).HasException());

    SECTION("WrongDocumentError if node is from a different document")
    {
      auto otherDocument = CreateRef<HTMLDocument>();
      auto otherNode = CreateRef<TestElement>(*otherDocument);
      auto result = data.Range->ComparePoint(*otherNode, 0uz);

      REQUIRE(result.HasException());
      REQUIRE(result == ExceptionCode::WrongDocumentError);
    }

    SECTION("InvalidNodeTypeError if node is a DocumentType")
    {
      auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
      REQUIRE_FALSE(doctype.HasException());

      REQUIRE_FALSE(data.Document->InsertBefore(*doctype.Value(), data.Node.get()).HasException());

      auto result = data.Range->ComparePoint(*doctype.Value(), 0uz);
      REQUIRE(result.HasException());
      REQUIRE(result == ExceptionCode::InvalidNodeTypeError);

      REQUIRE_FALSE(data.Document->RemoveChild(*doctype.Value()).HasException());
    }

    SECTION("IndexSizeError if offset is greater than the length of the node")
    {
      auto result = data.Range->ComparePoint(*child3, 1uz);
      REQUIRE(result.HasException());
      REQUIRE(result == ExceptionCode::IndexSizeError);
    }

    REQUIRE_FALSE(data.Range->SetStart(*child2, 0uz).HasException());
    REQUIRE_FALSE(data.Range->SetEnd(*child4, 0uz).HasException());

    SECTION("Equal if the point is between the start and end boundary points")
    {
      REQUIRE(data.Range->ComparePoint(*child3, 0uz) == std::strong_ordering::equal);
    }
    SECTION("Less if the point is before the start boundary point")
    {
      REQUIRE(data.Range->ComparePoint(*child1, 0uz) == std::strong_ordering::less);
    }
    SECTION("Greater if the point is after the end boundary point")
    {
      REQUIRE(data.Range->ComparePoint(*child5, 0uz) == std::strong_ordering::greater);
    }
    SECTION("Equal if the point is at the start boundary point")
    {
      REQUIRE(data.Range->ComparePoint(*child2, 0uz) == std::strong_ordering::equal);
    }
    SECTION("Equal if the point is at the end boundary point")
    {
      REQUIRE(data.Range->ComparePoint(*child4, 0uz) == std::strong_ordering::equal);
    }

    REQUIRE_FALSE(data.Document->RemoveChild(*data.Node).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child4).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child5).HasException());
  }

  TEST_CASE("Range.intersectsNode(): whether any of the node is in it",
            "[DOM][Range][dom:dom-range-intersectsnode]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);
    auto child3 = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());

    REQUIRE_FALSE(data.Range->SetStart(*child2, 0uz).HasException());
    REQUIRE_FALSE(data.Range->SetEnd(*child2, 0uz).HasException());
    SECTION("True if the node intersects with the range")
    {
      REQUIRE(data.Range->IntersectsNode(*child2) == true);
    }
    SECTION("False if the node does not intersect with the range")
    {
      REQUIRE(data.Range->IntersectsNode(*child1) == false);
      REQUIRE(data.Range->IntersectsNode(*child3) == false);
    }

    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
  }

  TEST_CASE("Range: its stringification is the text it contains", "[DOM][Range][dom:dom-range-stringifier]")
  {
    CommonTestData data;

    SECTION("When start and end container are the same and it is not a Text node, returns an empty string")
    {
      auto element = data.Document->CreateElement(u8"div");
      REQUIRE_FALSE(element.HasException());

      REQUIRE_FALSE(data.Range->SetStart(*element.Value(), 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*element.Value(), 0uz).HasException());

      auto result = data.Range->ToString();
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result == u8"");
    }

    SECTION("When start and end container are the same and it is a Text node, returns the content "
            "from start to end offset")
    {
      auto textNode = data.Document->CreateTextNode(u8"Hello, world!");

      REQUIRE_FALSE(data.Range->SetStart(*textNode, 7uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*textNode, 12uz).HasException());

      auto result = data.Range->ToString();
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result == u8"world");

      REQUIRE(textNode->Data() == u8"Hello, world!");
    }

    SECTION("When start is a partially contained text node, returns the substringed content")
    {
      auto textNode = data.Document->CreateTextNode(u8"Hello, world!");
      auto element = data.Document->CreateElement(u8"div");
      REQUIRE_FALSE(element.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*element.Value()).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*textNode, 7uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*element.Value(), 0uz).HasException());

      auto result = data.Range->ToString();
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result == u8"world!");

      REQUIRE_FALSE(data.Node->RemoveChild(*textNode).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*element.Value()).HasException());
    }

    SECTION("When range contains text nodes and non-text nodes, returns the concatenation of the text "
            "content of the contained text nodes")
    {
      auto textNode1 = data.Document->CreateTextNode(u8"Hello, ");
      auto textNode2 = data.Document->CreateTextNode(u8"nested text");
      auto textNode3 = data.Document->CreateTextNode(u8"World!");

      auto element = data.Document->CreateElement(u8"div");
      REQUIRE_FALSE(element.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*textNode1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*element.Value()).HasException());
      REQUIRE_FALSE(element->AppendChild(*textNode2).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*textNode3).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*textNode1, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*textNode3, 6uz).HasException());

      auto result = data.Range->ToString();
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result == u8"Hello, nested textWorld!");

      REQUIRE_FALSE(data.Node->RemoveChild(*textNode1).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*element.Value()).HasException());
      REQUIRE_FALSE(element->RemoveChild(*textNode2).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*textNode3).HasException());
    }

    SECTION("When end is a partially contained text node, returns the substringed content")
    {
      auto textNode = data.Document->CreateTextNode(u8"Hello, world!");
      auto element = data.Document->CreateElement(u8"div");
      REQUIRE_FALSE(element.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*element.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*element.Value(), 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*textNode, 5uz).HasException());

      auto result = data.Range->ToString();
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result == u8"Hello");

      REQUIRE_FALSE(data.Node->RemoveChild(*element.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*textNode).HasException());
    }
  }

  TEST_CASE(
    "Ranges: the introduction's example, and a live range follows mutations where a static one doesn't",
    "[DOM][Range][dom:ranges][dom:introduction-to-dom-ranges]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto document = Document::ParseHTMLUnsafe(*context, u8"<!DOCTYPE html><p id=p><img src=insanity-wolf "
                                                        u8"alt=\"Little-endian BOM\"> CSS 2.1 syndata is "
                                                        u8"<em id=em>awesome</em>!</p>")
                      .ReleaseValue();
    auto p = document->GetElementById(u8"p");
    auto em = document->GetElementById(u8"em");
    REQUIRE(p != nullptr);
    REQUIRE(em != nullptr);

    auto &firstText = Downcast<Text>(*p->ChildNodes()->Item(1));
    auto &secondText = Downcast<Text>(*em->FirstChild());
    auto range = document->CreateRange();
    REQUIRE_FALSE(range->SetStart(firstText, 9).HasException()); // do not forget the leading space
    REQUIRE_FALSE(range->SetEnd(secondText, 4).HasException());
    CHECK(range->ToString().ReleaseValue() == u8"syndata is awes");

    auto staticRange = CreateRef<StaticRange>(
      StaticRangeInit::Create(BoundaryPoint {ShareRef(firstText), 9}, BoundaryPoint {ShareRef(secondText), 4})
        .ReleaseValue());
    CHECK(range->IsLiveRange());
    CHECK_FALSE(staticRange->IsLiveRange());

    REQUIRE_FALSE(firstText.InsertData(0, u8"<<").HasException());
    CHECK(range->StartOffset() == 11uz);
    CHECK(range->ToString().ReleaseValue() == u8"syndata is awes");
    CHECK(staticRange->StartOffset() == 9uz);
  }

  namespace
  {
    /// @brief A connected tree: body > div#d > (b#b > "bb", i#i > "ii", u#u).
    struct ConnectedTestData
    {
      Ref<dom::Document> Document;
      Ref<Element> Div;
      Ref<Element> B;
      Ref<Text> BText;
      Ref<Element> I;
      Ref<Text> IText;
      Ref<Element> U;

      ConnectedTestData()
          : Document(dom::Document::ParseHTMLUnsafe(*CreateRef<HTMLDocument>(),
                                                    u8"<!DOCTYPE html><body><div id=d><b id=b>bb</b>"
                                                    u8"<i id=i>ii</i><u id=u></u></div>")
                       .ReleaseValue()),
            Div(ShareRef(*Document->GetElementById(u8"d"))), B(ShareRef(*Document->GetElementById(u8"b"))),
            BText(ShareRef(Downcast<Text>(*B->FirstChild()))), I(ShareRef(*Document->GetElementById(u8"i"))),
            IText(ShareRef(Downcast<Text>(*I->FirstChild()))), U(ShareRef(*Document->GetElementById(u8"u")))
      {
      }

      Ref<dom::Range> RangeOf(Node &startNode, uint64 startOffset, Node &endNode, uint64 endOffset)
      {
        auto range = Document->CreateRange();
        REQUIRE_FALSE(range->SetStart(startNode, startOffset).HasException());
        REQUIRE_FALSE(range->SetEnd(endNode, endOffset).HasException());
        return range;
      }
    };
  }

  TEST_CASE(
    "Range: new Range() is a live range collapsed at the start of the document",
    "[DOM][Range][dom:interface-range][dom:dom-range-range][dom:concept-live-range][dom:concept-range-root]")
  {
    ConnectedTestData data;
    auto range = data.Document->CreateRange();
    CHECK(range->IsLiveRange());
    CHECK(range->StartContainer() == data.Document.get());
    CHECK(range->StartOffset() == 0uz);
    CHECK(range->Collapsed());

    // Its root is its start node's root: setting its start in another tree moves its end there too.
    auto detached = data.Document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(range->SetEnd(*data.U, 0).HasException());
    REQUIRE_FALSE(range->SetStart(*detached, 0).HasException());
    CHECK(range->EndContainer() == detached.get());
  }

  TEST_CASE("Contained and partially contained: by a range's start and end",
            "[DOM][Range][dom:contained][dom:partially-contained]")
  {
    ConnectedTestData data;
    // From inside <b> to inside <i>: <b> and <i> are partially contained, nothing between them is contained.
    auto range = data.RangeOf(*data.BText, 1, *data.IText, 1);
    CHECK(range->IntersectsNode(*data.B));
    CHECK(range->ToString().ReleaseValue() == u8"bi");
    // Cloning keeps the partially contained elements, with only their contained parts.
    auto fragment = range->CloneContents().ReleaseValue();
    CHECK(fragment->ChildNodes()->Length() == 2u);
    CHECK(Downcast<Element>(*fragment->FirstChild()).OuterHTML() == u8"<b id=\"b\">b</b>");
    CHECK(Downcast<Element>(*fragment->LastChild()).OuterHTML() == u8"<i id=\"i\">i</i>");

    // (div, 1) to (div, 2) contains <i> and its text.
    auto around = data.RangeOf(*data.Div, 1, *data.Div, 2);
    CHECK(around->ToString().ReleaseValue() == u8"ii");
    CHECK(around->CloneContents().ReleaseValue()->ChildNodes()->Length() == 1u);
  }

  TEST_CASE("Range.compareBoundaryPoints(): START_TO_END compares its end with the other's start",
            "[DOM][Range][dom:dom-range-compareboundarypoints]")
  {
    ConnectedTestData data;
    auto first = data.RangeOf(*data.Div, 0, *data.Div, 1);  // <b>
    auto second = data.RangeOf(*data.Div, 1, *data.Div, 2); // <i>
    auto third = data.RangeOf(*data.Div, 2, *data.Div, 3);  // <u>

    auto compare = [](dom::Range &a, BoundaryPointComparator how, dom::Range &b)
    {
      return a.CompareBoundaryPoints(how, b).ReleaseValue();
    };
    // START_TO_END: this's end against sourceRange's start.
    CHECK(compare(*first, BoundaryPointComparator::StartToEnd, *second) == std::strong_ordering::equal);
    CHECK(compare(*first, BoundaryPointComparator::StartToEnd, *third) == std::strong_ordering::less);
    CHECK(compare(*third, BoundaryPointComparator::StartToEnd, *first) == std::strong_ordering::greater);
    // END_TO_START: this's start against sourceRange's end.
    CHECK(compare(*second, BoundaryPointComparator::EndToStart, *first) == std::strong_ordering::equal);
    CHECK(compare(*first, BoundaryPointComparator::EndToStart, *third) == std::strong_ordering::less);
    CHECK(compare(*third, BoundaryPointComparator::EndToStart, *first) == std::strong_ordering::greater);
    CHECK(compare(*first, BoundaryPointComparator::StartToStart, *second) == std::strong_ordering::less);
    CHECK(compare(*first, BoundaryPointComparator::EndToEnd, *second) == std::strong_ordering::less);

    SECTION("An invalid how throws a NotSupportedError, even for a range in another tree")
    {
      auto other = CreateRef<HTMLDocument>()->CreateRange();
      auto result = first->CompareBoundaryPoints(static_cast<BoundaryPointComparator>(4), *other);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
      result = first->CompareBoundaryPoints(BoundaryPointComparator::StartToStart, *other);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::WrongDocumentError);
    }
  }

  TEST_CASE("Range.deleteContents() and extractContents(): collapse to after the start's highest partially "
            "contained ancestor",
            "[DOM][Range][dom:dom-range-deletecontents][dom:concept-range-extract]")
  {
    ConnectedTestData data;

    SECTION("deleteContents()")
    {
      auto range = data.RangeOf(*data.BText, 1, *data.IText, 1);
      REQUIRE_FALSE(range->DeleteContents().HasException());
      CHECK(data.Div->InnerHTML() == u8"<b id=\"b\">b</b><i id=\"i\">i</i><u id=\"u\"></u>");
      // After <b> in the common ancestor <div>, not after the <div> in <body>.
      CHECK(range->StartContainer() == data.Div.get());
      CHECK(range->StartOffset() == 1uz);
      CHECK(range->Collapsed());
    }

    SECTION("extractContents()")
    {
      auto range = data.RangeOf(*data.BText, 1, *data.IText, 1);
      auto fragment = range->ExtractContents().ReleaseValue();
      CHECK(data.Div->InnerHTML() == u8"<b id=\"b\">b</b><i id=\"i\">i</i><u id=\"u\"></u>");
      CHECK(fragment->ChildNodes()->Length() == 2u);
      CHECK(range->StartContainer() == data.Div.get());
      CHECK(range->StartOffset() == 1uz);
      CHECK(range->Collapsed());
    }

    SECTION("A start that's an inclusive ancestor of the end keeps its start")
    {
      auto range = data.RangeOf(*data.Div, 0, *data.IText, 1);
      REQUIRE_FALSE(range->DeleteContents().HasException());
      CHECK(data.Div->InnerHTML() == u8"<i id=\"i\">i</i><u id=\"u\"></u>");
      CHECK(range->StartContainer() == data.Div.get());
      CHECK(range->StartOffset() == 0uz);
    }

    SECTION("A doctype among the contained children throws a HierarchyRequestError, changing nothing")
    {
      auto range = data.RangeOf(*data.Document, 0, *data.Document, 1);
      auto fragment = range->ExtractContents();
      REQUIRE(fragment.HasException());
      CHECK(fragment.GetException().Code() == ExceptionCode::HierarchyRequestError);
      CHECK(data.Document->DocType() != nullptr);
    }
  }

  TEST_CASE("Range.insertNode(): into a Text node it splits it; a collapsed range grows to include the node",
            "[DOM][Range][dom:concept-range-insert]")
  {
    ConnectedTestData data;
    auto range = data.RangeOf(*data.BText, 1, *data.BText, 1);
    auto em = data.Document->CreateElement(u8"em").ReleaseValue();
    REQUIRE_FALSE(range->InsertNode(*em).HasException());
    CHECK(data.B->InnerHTML() == u8"b<em></em>b");
    CHECK(range->StartContainer() == data.BText.get());
    CHECK(range->StartOffset() == 1uz);
    CHECK(range->EndContainer() == data.B.get());
    CHECK(range->EndOffset() == 2uz);

    SECTION("A comment, or the node itself, as the start throws a HierarchyRequestError")
    {
      auto comment = data.Document->CreateComment(u8"c");
      REQUIRE_FALSE(data.U->AppendChild(*comment).HasException());
      auto inComment = data.RangeOf(*comment, 0, *comment, 0);
      auto result = inComment->InsertNode(*data.Document->CreateElement(u8"s").ReleaseValue());
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

      auto inU = data.RangeOf(*data.U, 0, *data.U, 0);
      result = inU->InsertNode(*data.U);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::HierarchyRequestError);
    }
  }

  TEST_CASE("Range.surroundContents(): a partially contained element throws an InvalidStateError",
            "[DOM][Range][dom:dom-range-surroundcontents]")
  {
    ConnectedTestData data;
    auto range = data.RangeOf(*data.BText, 1, *data.IText, 1);
    auto result = range->SurroundContents(*data.Document->CreateElement(u8"s").ReleaseValue());
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::InvalidStateError);

    // Partially contained Text nodes are fine.
    auto inText = data.RangeOf(*data.BText, 0, *data.BText, 1);
    auto s = data.Document->CreateElement(u8"s").ReleaseValue();
    REQUIRE_FALSE(s->AppendChild(*data.Document->CreateTextNode(u8"old")).HasException());
    REQUIRE_FALSE(inText->SurroundContents(*s).HasException());
    // Inserting <s> split the text at the start, leaving an empty Text node before it.
    CHECK(data.B->InnerHTML() == u8"<s>b</s>b");
    CHECK(data.B->ChildNodes()->Length() == 3u);
    CHECK(inText->StartContainer() == data.B.get());
    CHECK(inText->StartOffset() == 1uz);
    CHECK(inText->EndOffset() == 2uz);
  }
}