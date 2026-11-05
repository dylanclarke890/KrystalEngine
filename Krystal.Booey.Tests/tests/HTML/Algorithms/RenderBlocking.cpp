#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE(
    "RenderBlocking::HasRenderBlockingToken: \"render\" among the blocking attribute's tokens, in any case",
    "[HTML][RenderBlocking][html:blocking-tokens-set][html:potentially-render-blocking]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto element = document->CreateElement(u8"link").ReleaseValue();
    CHECK_FALSE(RenderBlocking::HasRenderBlockingToken(*element));

    for (auto value : {u8"render", u8"RENDER", u8" other\tRender\n", u8"render render"})
    {
      INFO(reinterpret_cast<const char *>(value));
      REQUIRE_FALSE(element->SetAttribute(u8"blocking", value).HasException());
      CHECK(RenderBlocking::HasRenderBlockingToken(*element));
    }
    for (auto value : {u8"", u8"renders", u8"re nder", u8"other"})
    {
      INFO(reinterpret_cast<const char *>(value));
      REQUIRE_FALSE(element->SetAttribute(u8"blocking", value).HasException());
      CHECK_FALSE(RenderBlocking::HasRenderBlockingToken(*element));
    }
  }

  namespace
  {
    /// @brief An HTML document with an html element and a head, but no body yet.
    Ref<dom::HTMLDocument> CreateDocumentWithoutBody() noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto html = document->CreateElement(u8"html").ReleaseValue();
      REQUIRE_FALSE(html->AppendChild(*document->CreateElement(u8"head").ReleaseValue()).HasException());
      REQUIRE_FALSE(document->AppendChild(*html).HasException());
      return document;
    }
  }

  TEST_CASE("RenderBlocking::AllowsAddingRenderBlockingElements: an HTML document without a body element",
            "[HTML][RenderBlocking][html:allows-adding-render-blocking-elements]")
  {
    auto document = CreateDocumentWithoutBody();
    CHECK(RenderBlocking::AllowsAddingRenderBlockingElements(*document));

    // A frameset is a body element too.
    auto frameset = document->CreateElement(u8"frameset").ReleaseValue();
    REQUIRE_FALSE(document->DocumentElement()->AppendChild(*frameset).HasException());
    CHECK_FALSE(RenderBlocking::AllowsAddingRenderBlockingElements(*document));

    // Not an XML document.
    CHECK_FALSE(RenderBlocking::AllowsAddingRenderBlockingElements(*CreateRef<dom::XMLDocument>()));
  }

  TEST_CASE("RenderBlocking: blocking and unblocking rendering on an element",
            "[HTML][RenderBlocking][html:block-rendering][html:unblock-rendering][html:render-blocked]"
            "[html:render-blocking][html:render-blocking-element-set]")
  {
    auto document = CreateDocumentWithoutBody();
    auto link = document->CreateElement(u8"link").ReleaseValue();
    REQUIRE_FALSE(document->Head()->AppendChild(*link).HasException());

    // Without a body, the document is render-blocked even with no render-blocking elements.
    CHECK(RenderBlocking::IsRenderBlocked(*document));
    CHECK_FALSE(RenderBlocking::IsRenderBlocking(*link));

    RenderBlocking::BlockRendering(*link);
    CHECK(RenderBlocking::IsRenderBlocking(*link));

    // Once there's a body, the element still blocks rendering until it's unblocked.
    auto body = document->CreateElement(u8"body").ReleaseValue();
    REQUIRE_FALSE(document->DocumentElement()->AppendChild(*body).HasException());
    CHECK(RenderBlocking::IsRenderBlocked(*document));
    CHECK(RenderBlocking::IsRenderBlocking(*link));

    RenderBlocking::UnblockRendering(*link);
    CHECK_FALSE(RenderBlocking::IsRenderBlocking(*link));
    CHECK_FALSE(RenderBlocking::IsRenderBlocked(*document));

    // With a body, elements can't be added.
    RenderBlocking::BlockRendering(*link);
    CHECK_FALSE(RenderBlocking::IsRenderBlocking(*link));
    CHECK_FALSE(RenderBlocking::IsRenderBlocked(*document));
  }

  TEST_CASE("RenderBlocking: style elements block rendering until their sheet is processed",
            "[HTML][RenderBlocking][html:update-a-style-block][html:potentially-render-blocking]")
  {
    auto document = CreateDocumentWithoutBody();
    auto style = document->CreateElement(u8"style").ReleaseValue();
    REQUIRE_FALSE(style->SetAttribute(u8"blocking", u8"render").HasException());

    // Updating the style block blocks rendering on the element, and processing the sheet unblocks it, so a
    // sheet with no critical subresources leaves nothing blocked.
    REQUIRE_FALSE(style->AppendChild(*document->CreateTextNode(u8"p { color: red }")).HasException());
    REQUIRE_FALSE(document->Head()->AppendChild(*style).HasException());
    CHECK_FALSE(RenderBlocking::IsRenderBlocking(*style));
  }
}
