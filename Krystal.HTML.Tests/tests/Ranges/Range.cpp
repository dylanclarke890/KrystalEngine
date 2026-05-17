#include "Krystal.HTML/Ranges/Range.hpp"
#include "Krystal.HTML.Tests/TestContainerNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
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

    SECTION("InvalidNodeTypeError if BoundaryPointComparator is out of range")
    {
      auto result = data.Range->CompareBoundaryPoints(static_cast<BoundaryPointComparator>(4), *data.Range);

      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
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

    // TODO(impl):
    // SECTION("Deletes the content between the start and end boundary points")
    //{
    //  auto child1 = CreateRef<TestContainerNode>(*data.Document);
    //  auto child2 = CreateRef<TestContainerNode>(*data.Document);
    //  auto child3 = CreateRef<TestContainerNode>(*data.Document);

    // REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    // REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    // REQUIRE_FALSE(data.Node->AppendChild(*child3).HasException());

    // REQUIRE_FALSE(data.Range->SetStart(*data.Node, 0uz).HasException());
    // REQUIRE_FALSE(data.Range->SetStart(*data.Node, 2uz).HasException());

    // REQUIRE_FALSE(data.Range->DeleteContents().HasException());

    // REQUIRE(data.Node->ChildNodes()->Length() == 2uz);
    // REQUIRE(data.Range->StartContainer() == data.Node);
    // REQUIRE(data.Range->StartOffset() == 0uz);
    // REQUIRE(data.Range->EndContainer() == data.Node);
    // REQUIRE(data.Range->EndOffset() == 0uz);

    // REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    // REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    // REQUIRE_FALSE(data.Node->RemoveChild(*child3).HasException());
    // }
  }

  // TODO(impl):
  // TEST_CASE("Range::ExtractContents", "[HMTL][Range]")

  // TODO(impl):
  // TEST_CASE("Range::CloneContents", "[HMTL][Range]")

  // TODO(impl):
  // TEST_CASE("Range::InsertNode", "[HMTL][Range]")

  // TODO(impl):
  // TEST_CASE("Range::SurroundContents", "[HMTL][Range]")

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

  // TODO(impl):
  // TEST_CASE("Range::IsPointInRange", "[HMTL][Range]")

  // TODO(impl):
  // TEST_CASE("Range::ComparePoint", "[HMTL][Range]")

  // TODO(impl):
  // TEST_CASE("Range::IntersectsNode", "[HMTL][Range]")

  // TODO(impl):
  // TEST_CASE("Range::ToString", "[HMTL][Range]")
}