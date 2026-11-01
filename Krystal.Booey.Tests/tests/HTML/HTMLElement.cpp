#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    KRYS_NODISCARD Maybe<SRGBA<uint8>> Legacy(dom::DOMStringView value) noexcept
    {
      return HTMLElement::ParseLegacyColorValue(value);
    }
  }

  TEST_CASE("HTMLElement::ParseLegacyColorValue: named colours, #rgb, then thirds of the hex digits",
            "[HTML][PresentationalHints][html:rules-for-parsing-a-legacy-colour-value]")
  {
    CHECK_FALSE(Legacy(u8""));
    CHECK_FALSE(Legacy(u8"transparent"));
    CHECK_FALSE(Legacy(u8" TransParent "));
    CHECK(Legacy(u8"   ") == SRGBA<uint8> {0, 0, 0});
    CHECK(Legacy(u8"red") == SRGBA<uint8> {255, 0, 0});
    CHECK(Legacy(u8" ReD\t") == SRGBA<uint8> {255, 0, 0});
    CHECK(Legacy(u8"#abc") == SRGBA<uint8> {0xaa, 0xbb, 0xcc});
    CHECK(Legacy(u8"#00ff00") == SRGBA<uint8> {0, 255, 0});
    // Only named colours are keywords.
    CHECK(Legacy(u8"currentcolor") == SRGBA<uint8> {0xc0, 0xe0, 0});
    // The legacy rules: non-hex digits become 0, components are taken from thirds of the string.
    CHECK(Legacy(u8"chucknorris") == SRGBA<uint8> {0xc0, 0, 0});
    CHECK(Legacy(u8"sick") == SRGBA<uint8> {0, 0xc0, 0});
    CHECK(Legacy(u8"#1") == SRGBA<uint8> {0x01, 0, 0});
    CHECK(Legacy(u8"#") == SRGBA<uint8> {0, 0, 0});
    CHECK(Legacy(u8"abcd") == SRGBA<uint8> {0xab, 0xcd, 0});
    // Components longer than eight digits keep their last eight, then leading zeros all three share go.
    CHECK(Legacy(u8"1234567890ab1234567890ab1234567890ab") == SRGBA<uint8> {0x56, 0x56, 0x56});
    CHECK(Legacy(u8"000120003400056") == SRGBA<uint8> {0x12, 0x34, 0x56});
    // A non-ASCII code point is one digit, or two outside the BMP.
    CHECK(Legacy(u8"éff") == SRGBA<uint8> {0, 0x0f, 0x0f});
    CHECK(Legacy(u8"\U0001F600ff") == SRGBA<uint8> {0, 0xff, 0});
  }

  TEST_CASE("HTMLElement::ParseLegacyColorValue: 128 digits follow a leading #",
            "[HTML][PresentationalHints][html:rules-for-parsing-a-legacy-colour-value]")
  {
    // The specification truncates to 128 code points before removing the "#", which would leave out the "f".
    dom::DOMString value = u8"#";
    value.append(127, u8'0');
    value += u8'f';
    CHECK(Legacy(value) == SRGBA<uint8> {0, 0, 0xf0});
  }

  TEST_CASE("HTMLElement::IsInteractiveContent: the interactive content elements",
            "[HTML][HTMLElement][html:interactive-content-2]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto isInteractive =
      [&](utf8_stringview localName, utf8_stringview attribute = {}, utf8_stringview value = {})
    {
      auto element = document->CreateElement(dom::DOMStringAtom(localName)).ReleaseValue();
      if (!attribute.empty())
      {
        REQUIRE_FALSE(
          element->SetAttribute(dom::DOMStringAtom(attribute), dom::DOMString(value)).HasException());
      }
      return Downcast<HTMLElement>(*element).IsInteractiveContent();
    };

    for (auto name : {u8"button", u8"details", u8"embed", u8"iframe", u8"label", u8"select", u8"textarea"})
    {
      INFO(reinterpret_cast<const char *>(name));
      CHECK(isInteractive(name));
    }
    CHECK(isInteractive(u8"a", u8"href", u8"x"));
    CHECK_FALSE(isInteractive(u8"a"));
    CHECK(isInteractive(u8"img", u8"usemap", u8"#m"));
    CHECK_FALSE(isInteractive(u8"img"));
    CHECK(isInteractive(u8"input"));
    CHECK_FALSE(isInteractive(u8"input", u8"type", u8"hidden"));
    CHECK_FALSE(isInteractive(u8"div"));
  }

  TEST_CASE(
    "HTMLElement.translate: the translation mode, inherited from the nearest yes or no",
    "[HTML][HTMLElement][html:dom-translate][html:translation-mode][html:attr-translate-inherit-state]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto create = [&](utf8_stringview translate = {})
    {
      auto element = document->CreateElement(u8"span").ReleaseValue();
      if (translate.data())
      {
        REQUIRE_FALSE(element->SetAttribute(u8"translate", dom::DOMString(translate)).HasException());
      }
      return ShareRef(Downcast<HTMLElement>(*element));
    };

    // Translate-enabled without a parent, and an empty value is yes.
    CHECK(create()->Translate());
    CHECK(create(u8"")->Translate());
    CHECK(create(u8"YES")->Translate());
    CHECK_FALSE(create(u8"no")->Translate());

    // Inherit, an invalid value and a non-HTML element take the parent's.
    auto outer = create(u8"no");
    auto inherit = create();
    auto invalid = create(u8"maybe");
    auto svg = document->CreateElementNS(infra::Namespaces::SVG, u8"g").ReleaseValue();
    auto insideSvg = create();
    REQUIRE_FALSE(outer->AppendChild(*inherit).HasException());
    REQUIRE_FALSE(inherit->AppendChild(*invalid).HasException());
    REQUIRE_FALSE(invalid->AppendChild(*svg).HasException());
    REQUIRE_FALSE(svg->AppendChild(*insideSvg).HasException());
    CHECK_FALSE(inherit->Translate());
    CHECK_FALSE(invalid->Translate());
    CHECK_FALSE(HTMLElement::IsTranslateEnabled(*svg));
    CHECK_FALSE(insideSvg->Translate());

    auto yes = create(u8"yes");
    REQUIRE_FALSE(invalid->AppendChild(*yes).HasException());
    CHECK(yes->Translate());

    // Setting it writes yes or no.
    inherit->Translate(true);
    CHECK(inherit->GetAttribute(u8"translate") == dom::DOMString(u8"yes"));
    CHECK(invalid->Translate());
    inherit->Translate(false);
    CHECK(inherit->GetAttribute(u8"translate") == dom::DOMString(u8"no"));
  }

  TEST_CASE(
    "HTMLElement.innerText: the rendered text, or the text content when not rendered",
    "[HTML][HTMLElement][html:dom-innertext][html:get-the-text-steps][html:rendered-text-collection-steps]"
    "[html:dom-outertext]")
  {
    style::tests::ResolverFixture fixture {
      u8"<!DOCTYPE html><div id=d>a<br>b<span style=display:none>hidden</span>   c</div>"
      u8"<div id=p><p>One</p><p>Two</p></div><div id=n style=display:none>x  <b>y</b></div>"};

    auto &d = Downcast<HTMLElement>(fixture.ById(u8"d"));
    CHECK(d.InnerText() == u8"a\nb c");
    CHECK(d.OuterText() == d.InnerText());
    CHECK(Downcast<HTMLElement>(fixture.ById(u8"p")).InnerText() == u8"One\n\nTwo");

    // Not being rendered: the descendant text content.
    CHECK(Downcast<HTMLElement>(fixture.ById(u8"n")).InnerText() == u8"x  y");
  }

  TEST_CASE(
    "HTMLElement.innerText: setting it replaces the children with text and a br per line break",
    "[HTML][HTMLElement][html:dom-innertext][html:set-the-inner-text-steps][html:rendered-text-fragment]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto div = document->CreateElement(u8"div").ReleaseValue();
    auto &element = Downcast<HTMLElement>(*div);
    REQUIRE_FALSE(div->AppendChild(*document->CreateElement(u8"b").ReleaseValue()).HasException());

    element.InnerText(dom::DOMString(u8"a\r\nb\n\nc\r"));
    List<std::string> children;
    for (auto *child = div->FirstChild(); child; child = child->NextSibling())
    {
      auto *text = DynamicDowncast<dom::Text>(*child);
      children.push_back(
        text ? std::string(reinterpret_cast<const char *>(text->Data().data()), text->Data().size())
             : std::string("<")
                 + reinterpret_cast<const char *>(Downcast<dom::Element>(*child).LocalName().View().data())
                 + ">");
    }
    CHECK(children == List<std::string> {"a", "<br>", "b", "<br>", "<br>", "c", "<br>"});

    element.InnerText(dom::DOMString());
    CHECK_FALSE(div->HasChildNodes());
  }

  TEST_CASE("HTMLElement.outerText: setting it replaces the element with text, merged with its neighbours",
            "[HTML][HTMLElement][html:dom-outertext][html:merge-with-the-next-text-node][html:rendered-text-"
            "fragment]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto check = [](const auto &result, dom::ExceptionCode code)
    {
      return result.HasException() && result.GetException().Code() == code;
    };

    // Without a parent, it throws.
    auto orphan = document->CreateElement(u8"span").ReleaseValue();
    CHECK(check(Downcast<HTMLElement>(*orphan).OuterText(dom::DOMString(u8"x")),
                dom::ExceptionCode::NoModificationAllowedError));

    auto p = document->CreateElement(u8"p").ReleaseValue();
    auto span = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(p->AppendChild(*document->CreateTextNode(u8"x")).HasException());
    REQUIRE_FALSE(p->AppendChild(*span).HasException());
    REQUIRE_FALSE(p->AppendChild(*document->CreateTextNode(u8"z")).HasException());

    REQUIRE_FALSE(Downcast<HTMLElement>(*span).OuterText(dom::DOMString(u8"1\n2")).HasException());
    REQUIRE(p->ChildNodes()->Length() == 3);
    CHECK(Downcast<dom::Text>(*p->FirstChild()).Data() == u8"x1");
    CHECK(Downcast<dom::Element>(*p->FirstChild()->NextSibling()).LocalName() == u8"br");
    CHECK(Downcast<dom::Text>(*p->LastChild()).Data() == u8"2z");

    // An empty value leaves an empty text node, merged away.
    auto b = document->CreateElement(u8"b").ReleaseValue();
    REQUIRE_FALSE(p->InsertBefore(*b, p->LastChild()).HasException());
    REQUIRE_FALSE(Downcast<HTMLElement>(*b).OuterText(dom::DOMString()).HasException());
    REQUIRE(p->ChildNodes()->Length() == 3);
    CHECK(Downcast<dom::Text>(*p->LastChild()).Data() == u8"2z");
  }

  TEST_CASE("HTMLElement.dir: limited to ltr, rtl and auto", "[HTML][HTMLElement][html:dom-dir]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto div = document->CreateElement(u8"div").ReleaseValue();
    auto &element = Downcast<HTMLElement>(*div);
    CHECK(element.Dir() == u8"");
    element.Dir(dom::DOMString(u8"RTL"));
    CHECK(element.Dir() == u8"rtl");
    CHECK(element.GetAttribute(u8"dir") == dom::DOMString(u8"RTL"));
    element.Dir(dom::DOMString(u8"up"));
    CHECK(element.Dir() == u8"");
  }
}
