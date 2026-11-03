#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::NavigationFixture;

  namespace
  {
    KRYS_NODISCARD std::string Narrow(dom::DOMStringView string)
    {
      return std::string(reinterpret_cast<const char *>(string.data()), string.size());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLScriptElement: async reflects force async",
                   "[HTML][HTMLScriptElement][html:dom-script-async]")
  {
    NavigationFixture fixture(u8"<script id=parsed></script><div id=div></div>");
    auto &document = *fixture.Document;

    SECTION("A script from the parser isn't forced async, even an inert fragment's; a created one is")
    {
      CHECK_FALSE(fixture.Get<HTMLScriptElement>(u8"parsed").Async());

      auto &div = fixture.ById(u8"div");
      REQUIRE_FALSE(div.InnerHTML(u8"<script></script>").HasException());
      CHECK_FALSE(Downcast<HTMLScriptElement>(*div.FirstChild()).Async());

      auto created = document.CreateElement(u8"script").ReleaseValue();
      CHECK(Downcast<HTMLScriptElement>(*created).Async());
    }

    SECTION("Setting async clears force async")
    {
      auto created = document.CreateElement(u8"script").ReleaseValue();
      auto &script = Downcast<HTMLScriptElement>(*created);
      script.Async(true);
      CHECK(script.HasAttribute(u8"async"));
      CHECK(script.Async());
      script.Async(false);
      CHECK_FALSE(script.HasAttribute(u8"async"));
      CHECK_FALSE(script.Async());
    }

    SECTION("Adding an async attribute clears force async")
    {
      auto created = document.CreateElement(u8"script").ReleaseValue();
      auto &script = Downcast<HTMLScriptElement>(*created);
      REQUIRE_FALSE(script.SetAttribute(u8"async", u8"").HasException());
      script.RemoveAttribute(u8"async");
      CHECK_FALSE(script.Async());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLScriptElement: text is the child text content",
                   "[HTML][HTMLScriptElement][html:dom-script-text]")
  {
    NavigationFixture fixture(u8"<script id=s></script>");
    auto &document = *fixture.Document;
    auto &script = fixture.Get<HTMLScriptElement>(u8"s");
    REQUIRE_FALSE(script.AppendChild(*document.CreateTextNode(u8"a")).HasException());
    REQUIRE_FALSE(script.AppendChild(*document.CreateElement(u8"b").ReleaseValue()).HasException());
    REQUIRE_FALSE(script.LastChild()->AppendChild(*document.CreateTextNode(u8"x")).HasException());
    REQUIRE_FALSE(script.AppendChild(*document.CreateTextNode(u8"c")).HasException());
    CHECK(Narrow(script.Text()) == "ac");

    script.Text(u8"replaced");
    REQUIRE(script.FirstChild());
    CHECK(script.FirstChild() == script.LastChild());
    CHECK(Is<dom::Text>(*script.FirstChild()));
    CHECK(Narrow(script.Text()) == "replaced");
  }

  TEST_CASE("HTMLScriptElement: supports() matches the script types exactly",
            "[HTML][HTMLScriptElement][html:dom-script-supports]")
  {
    CHECK(HTMLScriptElement::Supports(u8"classic"));
    CHECK(HTMLScriptElement::Supports(u8"module"));
    CHECK(HTMLScriptElement::Supports(u8"importmap"));
    CHECK(HTMLScriptElement::Supports(u8"speculationrules"));
    CHECK_FALSE(HTMLScriptElement::Supports(u8"Module"));
    CHECK_FALSE(HTMLScriptElement::Supports(u8"text/javascript"));
    CHECK_FALSE(HTMLScriptElement::Supports(u8""));
  }
}
