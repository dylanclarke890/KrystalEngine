#include "Krystal.HTML/Ranges/Range.hpp"
#include "Krystal.HTML.Tests/TestContainerNode.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<TestContainerNode> Node;
      Ref<Range> Range;

      CommonTestData()
          : Document(CreateRef<HTMLDocument>()), Node(CreateRef<TestContainerNode>(*Document)),
            Range(Document->CreateRange())
      {
      }
    };
  }

  TEST_CASE("Range::CommonAncestorContainer", "[HMTL][Range]")
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

  TEST_CASE("Range::SetStart", "[HMTL][Range]")
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

  TEST_CASE("Range::SetEnd", "[HMTL][Range]")
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

  TEST_CASE("Range::SetStartBefore", "[HMTL][Range]")
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

  TEST_CASE("Range::SetEndBefore", "[HMTL][Range]")
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

  TEST_CASE("Range::SetStartAfter", "[HMTL][Range]")
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

  TEST_CASE("Range::SetEndAfter", "[HMTL][Range]")
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

  TEST_CASE("Range::IsCollapsed", "[HMTL][Range]")
  {
    CommonTestData data;

    SECTION("True if the start and end boundary points are the same")
    {
      REQUIRE(data.Range->IsCollapsed());
    }

    SECTION("False if the start and end boundary points are different")
    {
      auto child1 = CreateRef<TestContainerNode>(*data.Document);
      auto child2 = CreateRef<TestContainerNode>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*child2, 0uz).HasException());
      REQUIRE_FALSE(data.Range->IsCollapsed());

      REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    }
  }

  TEST_CASE("Range::Collapse", "[HMTL][Range]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestContainerNode>(*data.Document);
    auto child2 = CreateRef<TestContainerNode>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

    REQUIRE_FALSE(data.Range->SetStart(*child1, 0uz).HasException());
    REQUIRE_FALSE(data.Range->SetEnd(*child2, 0uz).HasException());

    SECTION("collapses to the start boundary point if the argument is true")
    {
      data.Range->Collapse(true);
      REQUIRE(data.Range->IsCollapsed());
      REQUIRE(data.Range->StartContainer() == child1);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == child1);
      REQUIRE(data.Range->EndOffset() == 0uz);
    }

    SECTION("collapses to the end boundary point if the argument is false")
    {
      data.Range->Collapse(false);
      REQUIRE(data.Range->IsCollapsed());
      REQUIRE(data.Range->StartContainer() == child2);
      REQUIRE(data.Range->StartOffset() == 0uz);
      REQUIRE(data.Range->EndContainer() == child2);
      REQUIRE(data.Range->EndOffset() == 0uz);
    }

    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
  }

  TEST_CASE("Range::SelectNode", "[HMTL][Range]")
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
      auto child = CreateRef<TestContainerNode>(*data.Document);
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

  TEST_CASE("Range::SelectNodeContents", "[HMTL][Range]")
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
      auto child1 = CreateRef<TestContainerNode>(*data.Document);
      auto child2 = CreateRef<TestContainerNode>(*data.Document);

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

  TEST_CASE("Range::CompareBoundaryPoints", "[HMTL][Range]")
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

    auto child1 = CreateRef<TestContainerNode>(*data.Document);
    auto child2 = CreateRef<TestContainerNode>(*data.Document);
    auto child3 = CreateRef<TestContainerNode>(*data.Document);

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

  TEST_CASE("Range::DeleteContents", "[HMTL][Range]")
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
      auto child1 = CreateRef<TestContainerNode>(*data.Document);
      auto child2 = CreateRef<TestContainerNode>(*data.Document);
      auto child3 = CreateRef<TestContainerNode>(*data.Document);

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

  TEST_CASE("Range::ExtractContents", "[HMTL][Range]")
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

      REQUIRE(Downcast<HTML::Text>(extractedTextNode.get())->Data() == u8"world");
      REQUIRE(textNode->Data() == u8"Hello, !");

      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 7uz);
      REQUIRE(data.Range->EndContainer() == textNode);
      REQUIRE(data.Range->EndOffset() == 7uz);
    }

    SECTION("Extracts the content between the start and end boundary points into a document fragment")
    {
      auto child1 = CreateRef<TestContainerNode>(*data.Document);
      auto child2 = CreateRef<TestContainerNode>(*data.Document);
      auto child3 = CreateRef<TestContainerNode>(*data.Document);

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

  TEST_CASE("Range::CloneContents", "[HMTL][Range]")
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

      REQUIRE(Downcast<HTML::Text>(extractedTextNode.get())->Data() == u8"world");
      REQUIRE(textNode->Data() == u8"Hello, world!");

      REQUIRE(data.Range->StartContainer() == textNode);
      REQUIRE(data.Range->StartOffset() == 7uz);
      REQUIRE(data.Range->EndContainer() == textNode);
      REQUIRE(data.Range->EndOffset() == 12uz);
    }

    SECTION("Clones the content between the start and end boundary points into a document fragment")
    {
      auto child1 = CreateRef<TestContainerNode>(*data.Document);
      auto child2 = CreateRef<TestContainerNode>(*data.Document);
      auto child3 = CreateRef<TestContainerNode>(*data.Document);

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

  TEST_CASE("Range::InsertNode", "[HMTL][Range]")
  {
    CommonTestData data;
    auto newNode = CreateRef<TestContainerNode>(*data.Document);

    SECTION("HierarchyRequestError if start container is a ProcessingInstruction node")
    {
      auto node = CreateRef<ProcessingInstruction>(*data.Document, u8"target", u8"data");
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
      auto node = CreateRef<HTML::Text>(*data.Document, u8"data");
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

  TEST_CASE("Range::SurroundContents", "[HMTL][Range]")
  {
    CommonTestData data;
    auto newNode = CreateRef<TestContainerNode>(*data.Document);

    SECTION("InvalidStateError if range partially contains a non-Text node")
    {
      auto comment = CreateRef<Comment>(*data.Document, u8"Hello, world!");
      auto otherChild = CreateRef<TestContainerNode>(*data.Document);

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
      auto child1 = CreateRef<TestContainerNode>(*data.Document);
      auto child2 = CreateRef<TestContainerNode>(*data.Document);

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
      auto child1 = CreateRef<TestContainerNode>(*data.Document);
      auto child2 = CreateRef<TestContainerNode>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

      REQUIRE_FALSE(data.Range->SetStart(*data.Node, 0uz).HasException());
      REQUIRE_FALSE(data.Range->SetEnd(*data.Node, 2uz).HasException());

      auto newNodeChild1 = CreateRef<TestContainerNode>(*data.Document);
      auto newNodeChild2 = CreateRef<TestContainerNode>(*data.Document);

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

  TEST_CASE("Range::CloneRange", "[HMTL][Range]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestContainerNode>(*data.Document);
    auto child2 = CreateRef<TestContainerNode>(*data.Document);
    auto child3 = CreateRef<TestContainerNode>(*data.Document);

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

  TEST_CASE("Range::Detach", "[HMTL][Range]")
  {
    // Does nothing and is kept for legacy reasons, so just test that it doesn't throw an exception or modify
    // the range

    CommonTestData data;
    data.Range->Detach();

    REQUIRE(data.Range->StartContainer() == data.Document);
    REQUIRE(data.Range->StartOffset() == 0uz);
    REQUIRE(data.Range->EndContainer() == data.Document);
    REQUIRE(data.Range->EndOffset() == 0uz);
  }

  TEST_CASE("Range::IsPointInRange", "[HMTL][Range]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestContainerNode>(*data.Document);
    auto child2 = CreateRef<TestContainerNode>(*data.Document);
    auto child3 = CreateRef<TestContainerNode>(*data.Document);
    auto child4 = CreateRef<TestContainerNode>(*data.Document);
    auto child5 = CreateRef<TestContainerNode>(*data.Document);

    REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child4).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child5).HasException());

    SECTION("WrongDocumentError if node is from a different document")
    {
      auto otherDocument = CreateRef<HTMLDocument>();
      auto otherNode = CreateRef<TestContainerNode>(*otherDocument);
      auto result = data.Range->IsPointInRange(*otherNode, 0uz);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::WrongDocumentError);
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

  TEST_CASE("Range::ComparePoint", "[HMTL][Range]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestContainerNode>(*data.Document);
    auto child2 = CreateRef<TestContainerNode>(*data.Document);
    auto child3 = CreateRef<TestContainerNode>(*data.Document);
    auto child4 = CreateRef<TestContainerNode>(*data.Document);
    auto child5 = CreateRef<TestContainerNode>(*data.Document);

    REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child4).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child5).HasException());

    SECTION("WrongDocumentError if node is from a different document")
    {
      auto otherDocument = CreateRef<HTMLDocument>();
      auto otherNode = CreateRef<TestContainerNode>(*otherDocument);
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

  TEST_CASE("Range::IntersectsNode", "[HMTL][Range]")
  {
    CommonTestData data;

    auto child1 = CreateRef<TestContainerNode>(*data.Document);
    auto child2 = CreateRef<TestContainerNode>(*data.Document);
    auto child3 = CreateRef<TestContainerNode>(*data.Document);

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

  TEST_CASE("Range::ToString", "[HMTL][Range]")
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
}