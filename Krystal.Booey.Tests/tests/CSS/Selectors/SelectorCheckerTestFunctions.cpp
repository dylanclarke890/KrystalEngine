#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
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

  TEST_CASE_METHOD(ValueFixture, "PseudoClasses: HTML's form and link pseudo-classes",
                   "[CSS][Selectors][SelectorCheckerTestFunctions][html:pseudo-classes][html:selector-link]"
                   "[html:selector-checked][html:selector-indeterminate][html:selector-default]"
                   "[html:selector-placeholder-shown][html:selector-invalid][html:selector-in-range]"
                   "[html:selector-out-of-range][html:selector-required][html:selector-optional]"
                   "[html:selector-read-only][html:selector-read-write][html:selector-ltr]")
  {
    boo::tests::EditorFixture fixture(
      u8"<a id=link href=x>L</a><a id=nolink>N</a>"
      u8"<form id=form><fieldset id=set><input id=req required></fieldset>"
      u8"<input type=radio name=g id=r1><input type=radio name=g id=r2>"
      u8"<input type=checkbox id=c checked><progress id=p></progress>"
      u8"<select id=s><option id=o1>A</option><option id=o2 selected>B</option></select>"
      u8"<button id=b>B</button><button id=b2>C</button>"
      u8"<input type=number id=n min=0 max=10 value=5><input type=number id=n2 min=0 max=10 value=20>"
      u8"<input id=ph placeholder=P><input id=ro readonly><div id=ce contenteditable>E</div>"
      u8"<p id=para dir=ltr>T</p></form>");
    auto matches = [&](utf8_stringview id, utf8_stringview selector)
    {
      return fixture.ById(id).Matches(selector).ReleaseValue();
    };

    CHECK(matches(u8"link", u8":link"));
    CHECK_FALSE(matches(u8"nolink", u8":link"));

    // A radio button group with nothing checked is indeterminate, and so is a progress without a value.
    CHECK(matches(u8"r1", u8":indeterminate"));
    CHECK(matches(u8"p", u8":indeterminate"));
    CHECK(matches(u8"c", u8":checked"));
    CHECK(matches(u8"o2", u8":checked"));
    CHECK_FALSE(matches(u8"o1", u8":checked"));

    // The form's first submit button and an initially checked control are default.
    CHECK(matches(u8"b", u8":default"));
    CHECK_FALSE(matches(u8"b2", u8":default"));
    CHECK(matches(u8"c", u8":default"));
    CHECK(matches(u8"o2", u8":default"));

    // A required empty control makes it, its fieldset and its form invalid.
    CHECK(matches(u8"req", u8":required"));
    CHECK(matches(u8"req", u8":invalid"));
    CHECK(matches(u8"set", u8":invalid"));
    CHECK(matches(u8"form", u8":invalid"));
    CHECK(matches(u8"ph", u8":optional"));
    CHECK(matches(u8"s", u8":optional"));

    CHECK(matches(u8"n", u8":in-range"));
    CHECK(matches(u8"n2", u8":out-of-range"));
    CHECK(matches(u8"ph", u8":placeholder-shown"));

    CHECK(matches(u8"ro", u8":read-only"));
    CHECK(matches(u8"ph", u8":read-write"));
    CHECK(matches(u8"ce", u8":read-write"));
    CHECK(matches(u8"para", u8":read-only"));
    CHECK(matches(u8"para", u8":dir(ltr)"));
    CHECK_FALSE(matches(u8"para", u8":dir(rtl)"));
  }
}
