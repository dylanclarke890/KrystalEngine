#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<dom::Text> Node;

      CommonTestData()
          : Document(CreateRef<dom::Document>()), Node(CreateRef<dom::Text>(*Document, u8"Hello, world!"))
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
    auto document = CreateRef<dom::Document>();
    auto parent = CreateRef<TestElement>(*document);
    auto text = CreateRef<dom::Text>(*document, u8"Hello, world!");

    REQUIRE_FALSE(parent->AppendChild(*text).HasException());

    auto beforeRange = document->CreateRange();
    beforeRange->SetStart(*text, 1uz);
    beforeRange->SetEnd(*text, 2uz);

    auto movedStartRange = document->CreateRange();
    movedStartRange->SetStart(*text, 5uz);
    movedStartRange->SetEnd(*text, 6uz);

    auto movedEndRange = document->CreateRange();
    movedEndRange->SetStart(*text, 1uz);
    movedEndRange->SetEnd(*text, 8uz);

    auto exactOffsetRange = document->CreateRange();
    exactOffsetRange->SetStart(*text, 2uz);
    exactOffsetRange->SetEnd(*text, 2uz);

    auto parentRange = document->CreateRange();

    parentRange->SetStart(*parent, 1uz);
    parentRange->SetEnd(*parent, 1uz);

    auto splitResult = text->SplitText(2);
    REQUIRE_FALSE(splitResult.HasException());

    auto &newNode = splitResult.Value();

    REQUIRE(text->Data() == u8"He");
    REQUIRE(newNode->Data() == u8"llo, world!");

    REQUIRE(beforeRange->StartContainer() == text.get());
    REQUIRE(beforeRange->EndContainer() == text.get());
    REQUIRE(beforeRange->StartOffset() == 1uz);
    REQUIRE(beforeRange->EndOffset() == 2uz);

    REQUIRE(movedStartRange->StartContainer() == newNode.get());
    REQUIRE(movedStartRange->EndContainer() == newNode.get());
    REQUIRE(movedStartRange->StartOffset() == 3uz);
    REQUIRE(movedStartRange->EndOffset() == 4uz);

    REQUIRE(movedEndRange->StartContainer() == text.get());
    REQUIRE(movedEndRange->EndContainer() == newNode.get());
    REQUIRE(movedEndRange->StartOffset() == 1uz);
    REQUIRE(movedEndRange->EndOffset() == 6uz);

    REQUIRE(exactOffsetRange->StartContainer() == text.get());
    REQUIRE(exactOffsetRange->EndContainer() == text.get());
    REQUIRE(exactOffsetRange->StartOffset() == 2uz);
    REQUIRE(exactOffsetRange->EndOffset() == 2uz);

    REQUIRE(parentRange->StartContainer() == parent.get());
    REQUIRE(parentRange->EndContainer() == parent.get());
    REQUIRE(parentRange->StartOffset() == 2uz);
    REQUIRE(parentRange->EndOffset() == 2uz);

    REQUIRE_FALSE(parent->RemoveChild(*newNode).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*text).HasException());
  }

  // TODO(test): check that mutation records are queued for the text nodes whose data is changed by SplitText.

  TEST_CASE("Text::WholeText", "[HTML][Text]")
  {
    auto doc = CreateRef<dom::Document>();
    auto parent = CreateRef<TestElement>(*doc);
    auto textNodeA = CreateRef<dom::Text>(*doc, u8"Hello,");
    auto textNodeB = CreateRef<dom::Text>(*doc, u8" ");
    auto textNodeC = CreateRef<dom::Text>(*doc, u8"world!");
    auto childNode = CreateRef<TestElement>(*doc);

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
    auto doc = CreateRef<dom::Document>();
    auto textNode = CreateRef<dom::Text>(*doc, u8"Hello, world!");

    REQUIRE(textNode->NodeName() == u8"#text");
  }

  TEST_CASE("Text::NodeType", "[HTML][Text]")
  {
    auto doc = CreateRef<dom::Document>();
    auto textNode = CreateRef<dom::Text>(*doc, u8"Hello, world!");

    REQUIRE(textNode->NodeType() == NodeType::TEXT_NODE);
  }
}
