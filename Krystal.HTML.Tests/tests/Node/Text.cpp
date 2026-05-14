#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML.Tests/TestContainerNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<HTML::Text> Node;

      CommonTestData()
          : Document(CreateRef<HTML::Document>()), Node(CreateRef<HTML::Text>(*Document, u8"Hello, world!"))
      {
      }
    };
  }

  TEST_CASE("Text::SplitText", "[HTML][Text]")
  {
    CommonTestData data;

    auto splitResult = data.Node->SplitText(2);
    REQUIRE_FALSE(splitResult.HasException());

    REQUIRE(data.Node->Data() == u8"He");
    REQUIRE(splitResult.Value()->Data() == u8"llo, world!");
  }

  TEST_CASE("Text::SplitText updates live ranges", "[HTML][Text]")
  {
    auto document = CreateRef<Document>();
    auto parent = CreateRef<TestContainerNode>(*document);
    auto text = CreateRef<HTML::Text>(*document, u8"Hello, world!");

    REQUIRE_FALSE(parent->AppendChild(*text).HasException());

    // Text content: "Hello, world!"
    // Split at offset 2:
    // Original: "He"
    // New node: "llo, world!"

    // Range entirely before split
    // Should remain unchanged.
    auto beforeRange = document->CreateRange();
    beforeRange->SetStart(*text, 1uz);
    beforeRange->SetEnd(*text, 2uz);

    // Range start > split offset
    // Start container should move to new node.
    auto movedStartRange = document->CreateRange();
    movedStartRange->SetStart(*text, 5uz);
    movedStartRange->SetEnd(*text, 6uz);

    // Range end > split offset
    // End container should move to new node.
    auto movedEndRange = document->CreateRange();
    movedEndRange->SetStart(*text, 1uz);
    movedEndRange->SetEnd(*text, 8uz);

    // Range exactly at split offset
    // MUST NOT move because spec says "greater than offset".
    auto exactOffsetRange = document->CreateRange();
    exactOffsetRange->SetStart(*text, 2uz);
    exactOffsetRange->SetEnd(*text, 2uz);

    // Parent-based boundary points.
    auto parentRange = document->CreateRange();

    // text is child index 0
    // index + 1 == 1
    parentRange->SetStart(*parent, 1uz);
    parentRange->SetEnd(*parent, 1uz);

    auto splitResult = text->SplitText(2);
    REQUIRE_FALSE(splitResult.HasException());

    auto& newNode = splitResult.Value();

    // Verify text contents
    REQUIRE(text->Data() == u8"He");
    REQUIRE(newNode->Data() == u8"llo, world!");

    // beforeRange unchanged
    REQUIRE(beforeRange->StartContainer() == text.get());
    REQUIRE(beforeRange->EndContainer() == text.get());
    REQUIRE(beforeRange->StartOffset() == 1uz);
    REQUIRE(beforeRange->EndOffset() == 2uz);

    // movedStartRange:
    // start offset 5 -> new node offset 3
    // end offset 6 -> new node offset 4
    REQUIRE(movedStartRange->StartContainer() == newNode.get());
    REQUIRE(movedStartRange->EndContainer() == newNode.get());
    REQUIRE(movedStartRange->StartOffset() == 3uz);
    REQUIRE(movedStartRange->EndOffset() == 4uz);

    // movedEndRange:
    // start remains
    // end moves: 8 -> 6
    REQUIRE(movedEndRange->StartContainer() == text.get());
    REQUIRE(movedEndRange->EndContainer() == newNode.get());
    REQUIRE(movedEndRange->StartOffset() == 1uz);
    REQUIRE(movedEndRange->EndOffset() == 6uz);

    // exactOffsetRange unchanged
    REQUIRE(exactOffsetRange->StartContainer() == text.get());
    REQUIRE(exactOffsetRange->EndContainer() == text.get());
    REQUIRE(exactOffsetRange->StartOffset() == 2uz);
    REQUIRE(exactOffsetRange->EndOffset() == 2uz);

    // parentRange offsets incremented:
    // spec says: "if offset == index(node)+1 increase by 1"
    REQUIRE(parentRange->StartContainer() == parent.get());
    REQUIRE(parentRange->EndContainer() == parent.get());
    REQUIRE(parentRange->StartOffset() == 2uz);
    REQUIRE(parentRange->EndOffset() == 2uz);

    REQUIRE_FALSE(parent->RemoveChild(*newNode).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*text).HasException());
  }

  TEST_CASE("Text::WholeText", "[HTML][Text]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestContainerNode>(*doc);
    auto textNodeA = CreateRef<Krys::HTML::Text>(*doc, u8"Hello,");
    auto textNodeB = CreateRef<Krys::HTML::Text>(*doc, u8" ");
    auto textNodeC = CreateRef<Krys::HTML::Text>(*doc, u8"world!");
    auto childNode = CreateRef<TestContainerNode>(*doc);

    REQUIRE_FALSE(parent->AppendChild(*textNodeA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*textNodeB).HasException());
    REQUIRE_FALSE(parent->AppendChild(*textNodeC).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childNode).HasException());

    REQUIRE(textNodeA->WholeText() == u8"Hello, world!");
    REQUIRE(textNodeB->WholeText() == u8"Hello, world!");
    REQUIRE(textNodeC->WholeText() == u8"Hello, world!");

    REQUIRE_FALSE(parent->RemoveChild(*textNodeA).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*textNodeB).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*textNodeC).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childNode).HasException());
  }

  TEST_CASE("Text::NodeName", "[HTML][Text]")
  {
    auto doc = CreateRef<Document>();
    auto textNode = CreateRef<Krys::HTML::Text>(*doc, u8"Hello, world!");

    REQUIRE(textNode->NodeName() == u8"#text");
  }

  TEST_CASE("Text::NodeType", "[HTML][Text]")
  {
    auto doc = CreateRef<Document>();
    auto textNode = CreateRef<Krys::HTML::Text>(*doc, u8"Hello, world!");

    REQUIRE(textNode->NodeType() == NodeType::TEXT_NODE);
  }
}
