#include "Krystal.Booey/DOM/Text.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("Text: constructed, or created as a text node, with the data in the document",
            "[DOM][Text][dom:interface-text][dom:dom-text-text][dom:create-a-text-node]")
  {
    auto document = CreateRef<XMLDocument>();

    auto constructed = CreateRef<Text>(*document, u8"data");
    CHECK(constructed->Data() == u8"data");
    CHECK(&constructed->NodeDocument() == document.get());
    CHECK(constructed->ParentNode() == nullptr);
    CHECK(CreateRef<Text>(*document)->Data() == u8"");

    auto created = document->CreateTextNode(u8"created");
    CHECK(created->Data() == u8"created");
    CHECK(&created->NodeDocument() == document.get());
  }

  TEST_CASE("Text.splitText(): splits it at the offset, returning the new node",
            "[DOM][Text][dom:dom-text-splittext]")
  {
    auto document = CreateRef<XMLDocument>();
    auto parent = document->CreateElementNS(u8"urn:x", u8"p").ReleaseValue();
    auto text = document->CreateTextNode(u8"Hello, world!");
    REQUIRE_FALSE(parent->AppendChild(*text).HasException());

    auto result = text->SplitText(7);
    REQUIRE_FALSE(result.HasException());
    CHECK(text->Data() == u8"Hello, ");
    CHECK(result.Value()->Data() == u8"world!");
    CHECK(text->NextSibling() == result.Value().get());

    auto outOfBounds = text->SplitText(8);
    REQUIRE(outOfBounds.HasException());
    CHECK(outOfBounds.GetException().Code() == ExceptionCode::IndexSizeError);
  }

  TEST_CASE("Text.wholeText: the data of its contiguous Text nodes, CDATA sections included, in tree order",
            "[DOM][Text][dom:dom-text-wholetext]")
  {
    auto document = CreateRef<XMLDocument>();
    auto parent = document->CreateElementNS(u8"urn:x", u8"p").ReleaseValue();
    auto append = [&](Ref<Node> node)
    {
      REQUIRE_FALSE(parent->AppendChild(*node).HasException());
      return node;
    };
    append(document->CreateTextNode(u8"before"));
    append(document->CreateComment(u8"stop"));
    auto first = append(document->CreateTextNode(u8"Hello,"));
    auto cdata = append(document->CreateCDATASection(u8" ").ReleaseValue());
    auto last = append(document->CreateTextNode(u8"world!"));
    append(document->CreateElementNS(u8"urn:x", u8"stop").ReleaseValue());
    append(document->CreateTextNode(u8"after"));

    CHECK(Downcast<Text>(*first).WholeText() == u8"Hello, world!");
    CHECK(Downcast<Text>(*cdata).WholeText() == u8"Hello, world!");
    CHECK(Downcast<Text>(*last).WholeText() == u8"Hello, world!");
    CHECK(document->CreateTextNode(u8"alone")->WholeText() == u8"alone");
  }

  TEST_CASE("Node.nodeName: a Text's is #text", "[DOM][Text][dom:dom-node-nodename]")
  {
    auto doc = CreateRef<dom::Document>();
    auto textNode = CreateRef<dom::Text>(*doc, u8"Hello, world!");

    REQUIRE(textNode->NodeName() == u8"#text");
  }

  TEST_CASE("Node.nodeType: a Text is a TEXT_NODE",
            "[DOM][Text][dom:dom-node-nodetype][dom:dom-node-text_node]")
  {
    auto doc = CreateRef<dom::Document>();
    auto textNode = CreateRef<dom::Text>(*doc, u8"Hello, world!");

    REQUIRE(textNode->NodeType() == NodeType::TEXT_NODE);
  }
}
