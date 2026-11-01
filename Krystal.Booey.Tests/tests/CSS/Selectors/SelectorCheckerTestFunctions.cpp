#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Dicts/ShadowRootInit.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE(
    "EffectiveLang: xml:lang anywhere, lang on HTML and SVG elements, then ancestors and the pragma",
    "[CSS][Selectors][SelectorCheckerTestFunctions][html:language][html:attr-lang][html:attr-xml-lang]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto html = document->CreateElement(u8"html").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*html).HasException());

    // Nothing sets it: the pragma-set default language, which is empty here.
    CHECK(EffectiveLang(*html) == u8"");

    REQUIRE_FALSE(html->SetAttribute(u8"lang", u8"en").HasException());
    auto div = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(html->AppendChild(*div).HasException());
    CHECK(EffectiveLang(*div) == u8"en");

    // lang in no namespace counts on SVG elements, but not on MathML ones.
    auto svg = document->CreateElementNS(infra::Namespaces::SVG, u8"svg").ReleaseValue();
    REQUIRE_FALSE(svg->SetAttribute(u8"lang", u8"fr").HasException());
    auto math = document->CreateElementNS(infra::Namespaces::MathML, u8"math").ReleaseValue();
    REQUIRE_FALSE(math->SetAttribute(u8"lang", u8"de").HasException());
    REQUIRE_FALSE(div->AppendChild(*svg).HasException());
    REQUIRE_FALSE(div->AppendChild(*math).HasException());
    CHECK(EffectiveLang(*svg) == u8"fr");
    CHECK(EffectiveLang(*math) == u8"en");

    // xml:lang counts on any element, and comes before lang.
    REQUIRE_FALSE(math->SetAttributeNS(infra::Namespaces::XML, u8"xml:lang", u8"es").HasException());
    CHECK(EffectiveLang(*math) == u8"es");
    REQUIRE_FALSE(svg->SetAttributeNS(infra::Namespaces::XML, u8"xml:lang", u8"it").HasException());
    CHECK(EffectiveLang(*svg) == u8"it");

    // An empty value is the language too (unknown).
    auto empty = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(empty->SetAttribute(u8"lang", u8"").HasException());
    REQUIRE_FALSE(div->AppendChild(*empty).HasException());
    CHECK(EffectiveLang(*empty) == u8"");

    // A shadow tree's elements take the host's.
    auto host = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(div->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({}).ReleaseValue();
    auto inShadow = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inShadow).HasException());
    CHECK(EffectiveLang(*inShadow) == u8"en");
  }
}
