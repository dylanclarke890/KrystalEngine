#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML.Tests/TestContainerNode.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("Text::SplitText", "[Text]")
  {
    auto doc = CreateRef<Document>();
    auto textNode = CreateRef<Text>(*doc, u8"Hello, world!");
    auto splitResult = textNode->SplitText(7);

    REQUIRE(textNode->Data() == u8"Hello, ");
    REQUIRE(!splitResult.HasException());
    REQUIRE(splitResult.Value()->Data() == u8"world!");
  }

  TEST_CASE("Text::WholeText", "[Text]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestContainerNode>(*doc);
    auto textNodeA = CreateRef<Text>(*doc, u8"Hello,");
    auto textNodeB = CreateRef<Text>(*doc, u8" ");
    auto textNodeC = CreateRef<Text>(*doc, u8"world!");
    auto childNode = CreateRef<TestContainerNode>(*doc);

    parent->AppendChild(*textNodeA);
    parent->AppendChild(*textNodeB);
    parent->AppendChild(*textNodeC);
    parent->AppendChild(*childNode);

    REQUIRE(textNodeA->WholeText() == u8"Hello, world!");
    REQUIRE(textNodeB->WholeText() == u8"Hello, world!");
    REQUIRE(textNodeC->WholeText() == u8"Hello, world!");

    parent->RemoveChild(*textNodeA);
    parent->RemoveChild(*textNodeB);
    parent->RemoveChild(*textNodeC);
    parent->RemoveChild(*childNode);
  }

  TEST_CASE("Text::NodeName", "[Text]")
  {
    auto doc = CreateRef<Document>();
    auto textNode = CreateRef<Text>(*doc, u8"Hello, world!");
    
    REQUIRE(textNode->NodeName() == u8"#text");
  }
}
