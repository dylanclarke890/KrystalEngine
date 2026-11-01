#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"

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
}
