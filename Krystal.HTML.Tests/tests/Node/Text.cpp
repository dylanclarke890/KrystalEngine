#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML.Tests/TestContainerNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("Text::SplitText", "[HTML][Text]")
  {
    auto doc = CreateRef<Document>();
    auto textNode = CreateRef<Krys::HTML::Text>(*doc, u8"Hello, world!");
    
    auto splitResult = textNode->SplitText(7);
    REQUIRE_FALSE(splitResult.HasException());

    REQUIRE(textNode->Data() == u8"Hello, ");
    REQUIRE(splitResult.Value()->Data() == u8"world!");
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
