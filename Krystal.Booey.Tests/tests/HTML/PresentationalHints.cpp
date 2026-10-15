#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/HTML/HTMLFontElement.hpp"
#include "Krystal.Booey/HTML/HTMLFrameSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLMarqueeElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableColElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::html::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief The presentational hints of the element with @p id in @p markup, serialised.
    struct HintFixture : css::tests::ValueFixture
    {
      RefPtr<dom::HTMLDocument> Document;

      KRYS_NODISCARD dom::StyledElement &Element(utf8_string markup, dom::DOMStringView id = u8"e") noexcept
      {
        Document = ResolverFixture::Parse(krys::move(markup));
        auto element = Document->GetElementById(id);
        krys_assert(element);
        return Downcast<dom::StyledElement>(*element);
      }

      KRYS_NODISCARD static std::string Text(const css::StyleProperties *style) noexcept
      {
        return style ? css::tests::ToNarrow(style->AsText(css::DefaultSerialisationContext()))
                     : std::string {};
      }

      KRYS_NODISCARD std::string Hints(utf8_string markup, dom::DOMStringView id = u8"e") noexcept
      {
        return Text(Element(krys::move(markup), id).PresentationalHintStyle());
      }
    };

    KRYS_NODISCARD Maybe<SRGBA<uint8>> Legacy(dom::DOMStringView value) noexcept
    {
      return HTMLElement::ParseLegacyColorValue(value);
    }
  }

  TEST_CASE("HTMLElement::ParseLegacyColorValue", "[HTML][PresentationalHints]")
  {
    CHECK_FALSE(Legacy(u8""));
    CHECK_FALSE(Legacy(u8"transparent"));
    CHECK_FALSE(Legacy(u8" TransParent "));
    CHECK(Legacy(u8"   ") == SRGBA<uint8> {0, 0, 0});
    CHECK(Legacy(u8"red") == SRGBA<uint8> {255, 0, 0});
    CHECK(Legacy(u8"#abc") == SRGBA<uint8> {0xaa, 0xbb, 0xcc});
    CHECK(Legacy(u8"#00ff00") == SRGBA<uint8> {0, 255, 0});
    // The legacy rules: non-hex digits become 0, components are taken from thirds of the string.
    CHECK(Legacy(u8"chucknorris") == SRGBA<uint8> {0xc0, 0, 0});
    CHECK(Legacy(u8"sick") == SRGBA<uint8> {0, 0xc0, 0});
    CHECK(Legacy(u8"#1") == SRGBA<uint8> {0x01, 0, 0});
    CHECK(Legacy(u8"#") == SRGBA<uint8> {0, 0, 0});
    // A non-ASCII code point is one digit, or two outside the BMP.
    CHECK(Legacy(u8"éff") == SRGBA<uint8> {0, 0x0f, 0x0f});
    CHECK(Legacy(u8"\U0001F600ff") == SRGBA<uint8> {0, 0xff, 0});
  }

  TEST_CASE_METHOD(HintFixture, "Presentational hints: HTMLElement", "[HTML][PresentationalHints]")
  {
    CHECK(Hints(u8"<span id=e></span>").empty());
    CHECK(Hints(u8"<span id=e align=middle></span>") == "text-align: center;");
    CHECK(Hints(u8"<span id=e align=right></span>") == "text-align: right;");
    CHECK(Hints(u8"<span id=e hidden></span>") == "display: none;");
    CHECK(Hints(u8"<span id=e hidden=until-found></span>") == "content-visibility: hidden;");
    CHECK(Hints(u8"<span id=e dir=rtl></span>") == "direction: rtl; unicode-bidi: isolate;");
    CHECK(Hints(u8"<bdo id=e dir=ltr></bdo>") == "direction: ltr;");
    CHECK(Hints(u8"<span id=e dir=auto></span>") == "unicode-bidi: isolate;");
    CHECK(Hints(u8"<pre id=e dir=auto></pre>") == "unicode-bidi: plaintext;");
    CHECK(Hints(u8"<span id=e dir=sideways></span>").empty());
    CHECK(Hints(u8"<span id=e draggable=true></span>") == "-webkit-user-drag: element;");
    CHECK(Hints(u8"<span id=e draggable=false></span>") == "-webkit-user-drag: none;");
    CHECK(Hints(u8"<span id=e contenteditable></span>")
          == "overflow-wrap: break-word; line-break: after-white-space; -webkit-user-modify: read-write;");
    CHECK(Hints(u8"<span id=e contenteditable=plaintext-only></span>")
          == "overflow-wrap: break-word; line-break: after-white-space; -webkit-user-modify: "
             "read-write-plaintext-only;");
    CHECK(Hints(u8"<span id=e contenteditable=false></span>") == "-webkit-user-modify: read-only;");
    CHECK(Hints(u8"<span id=e contenteditable=maybe></span>").empty());
    CHECK(Hints(u8"<span id=e lang=en></span>") == "-webkit-locale: \"en\";");
    CHECK(Hints(u8"<span id=e lang></span>") == "-webkit-locale: auto;");

    SECTION("Attribute changes rebuild the hints")
    {
      auto &element = Element(u8"<span id=e align=left></span>");
      CHECK(Text(element.PresentationalHintStyle()) == "text-align: left;");
      REQUIRE_FALSE(element.SetAttribute(u8"align", u8"center").HasException());
      CHECK(Text(element.PresentationalHintStyle()) == "text-align: center;");
      element.RemoveAttribute(u8"align");
      CHECK_FALSE(element.PresentationalHintStyle());
    }
  }

  TEST_CASE_METHOD(HintFixture, "Presentational hints: text elements", "[HTML][PresentationalHints]")
  {
    CHECK(Hints(u8"<div id=e align=center></div>") == "text-align: -webkit-center;");
    CHECK(Hints(u8"<p id=e align=left></p>") == "text-align: -webkit-left;");
    CHECK(Hints(u8"<p id=e align=justify></p>") == "text-align: justify;");
    CHECK(Hints(u8"<br id=e clear=all>") == "clear: both;");
    CHECK(Hints(u8"<br id=e clear=left>") == "clear: left;");
    CHECK(Hints(u8"<br id=e clear>").empty());
    // white-space-collapse and text-wrap-mode serialise as the white-space shorthand.
    CHECK(Hints(u8"<pre id=e wrap></pre>") == "white-space: pre-wrap;");
    CHECK(Hints(u8"<textarea id=e wrap=off></textarea>")
          == "white-space: pre; overflow-wrap: normal;");
    CHECK(Hints(u8"<textarea id=e wrap=hard></textarea>")
          == "white-space: pre-wrap; overflow-wrap: break-word;");
    CHECK(Hints(u8"<font id=e size=1 color=red face=serif></font>")
          == "font-size: x-small; color: rgb(255, 0, 0); font-family: serif;");
    CHECK(Hints(u8"<font id=e size=+2></font>") == "font-size: x-large;");
    CHECK(Hints(u8"<font id=e size=-1></font>") == "font-size: small;");
    CHECK(Hints(u8"<font id=e size=9></font>") == "font-size: xxx-large;");
    CHECK(Hints(u8"<font id=e size=0></font>") == "font-size: x-small;");
    CHECK(Hints(u8"<font id=e size=big></font>").empty());
    CHECK(Hints(u8"<hr id=e align=left width=50% size=4>")
          == "margin-left: 0px; margin-right: auto; width: 50%; height: 2px;");
    CHECK(Hints(u8"<hr id=e size=1>") == "border-bottom-width: 0px;");
    CHECK(Hints(u8"<hr id=e color=blue>")
          == "border-style: solid; border-color: rgb(0, 0, 255); background-color: rgb(0, 0, 255);");
    CHECK(Hints(u8"<hr id=e noshade>")
          == "border-style: solid; border-color: rgb(128, 128, 128); background-color: rgb(128, 128, 128);");
  }

  TEST_CASE_METHOD(HintFixture, "Presentational hints: lists", "[HTML][PresentationalHints]")
  {
    CHECK(Hints(u8"<ol id=e type=a></ol>") == "list-style-type: lower-alpha;");
    CHECK(Hints(u8"<ol id=e type=I></ol>") == "list-style-type: upper-roman;");
    CHECK(Hints(u8"<ol id=e type=disc></ol>").empty());
    CHECK(Hints(u8"<ul id=e type=SQUARE></ul>") == "list-style-type: square;");
    CHECK(Hints(u8"<ul id=e type=a></ul>").empty());
    CHECK(Hints(u8"<ul><li id=e type=1 value=5></li></ul>")
          == "list-style-type: decimal; counter-set: list-item 5;");
    CHECK(Hints(u8"<ul><li id=e type=Circle value=x></li></ul>") == "list-style-type: circle;");
  }

  TEST_CASE_METHOD(HintFixture, "Presentational hints: embedded content", "[HTML][PresentationalHints]")
  {
    CHECK(Hints(u8"<img id=e width=100 height=50>")
          == "width: 100px; aspect-ratio: auto 100 / 50; height: 50px;");
    CHECK(Hints(u8"<img id=e width=50% border=2 hspace=3 align=left>")
          == "width: 50%; border-width: 2px; border-style: solid; margin-left: 3px; margin-right: 3px; "
             "float: left; "
             "vertical-align: top;");
    CHECK(Hints(u8"<img id=e align=absmiddle>") == "vertical-align: middle;");
    CHECK(Hints(u8"<canvas id=e width=300 height=150></canvas>") == "aspect-ratio: auto 300 / 150;");
    CHECK(Hints(u8"<video id=e width=320></video>") == "width: 320px;");
    CHECK(Hints(u8"<iframe id=e frameborder=0 width=10></iframe>") == "border-width: 0px; width: 10px;");
    CHECK(Hints(u8"<embed id=e hidden>") == "width: 0px; height: 0px;");
    CHECK(Hints(u8"<object id=e border=1 vspace=2></object>")
          == "border-width: 1px; border-style: solid; margin-top: 2px; margin-bottom: 2px;");
    CHECK(Hints(u8"<input id=e type=image width=20 align=right>")
          == "width: 20px; float: right; vertical-align: top;");
    CHECK(Hints(u8"<input id=e width=20 align=right>").empty());

    SECTION("Changing the type changes the hints")
    {
      auto &input = Element(u8"<input id=e width=20>");
      CHECK_FALSE(input.PresentationalHintStyle());
      REQUIRE_FALSE(input.SetAttribute(u8"type", u8"image").HasException());
      CHECK(Text(input.PresentationalHintStyle()) == "width: 20px;");
    }
  }

  TEST_CASE_METHOD(HintFixture, "Presentational hints: legacy elements", "[HTML][PresentationalHints]")
  {
    auto &marquee = Element(u8"<marquee id=e width=100 bgcolor=red hspace=1 scrollamount=5></marquee>");
    CHECK(Is<HTMLMarqueeElement>(Downcast<HTMLElement>(marquee)));
    CHECK(Text(marquee.PresentationalHintStyle())
          == "width: 100px; background-color: rgb(255, 0, 0); margin-left: 1px; margin-right: 1px;");

    auto &font = Element(u8"<font id=e></font>");
    CHECK(Is<HTMLFontElement>(Downcast<HTMLElement>(font)));

    auto document = ResolverFixture::Parse(u8"<frameset id=e bordercolor=lime></frameset>");
    auto frameset = document->GetElementById(u8"e");
    REQUIRE(frameset);
    CHECK(Is<HTMLFrameSetElement>(Downcast<HTMLElement>(*frameset)));
    CHECK(Text(Downcast<dom::StyledElement>(*frameset).PresentationalHintStyle())
          == "border-color: rgb(0, 255, 0);");
  }

  TEST_CASE_METHOD(HintFixture, "Presentational hints: tables", "[HTML][PresentationalHints]")
  {
    CHECK(Hints(u8"<table id=e width=0 height=10 cellspacing=2 bgcolor=#fff align=center></table>")
          == "height: 10px; border-spacing: 2px; background-color: rgb(255, 255, 255); margin-inline: auto;");
    CHECK(Hints(u8"<table id=e align=right valign=top></table>") == "float: right; vertical-align: top;");
    CHECK(Hints(u8"<table id=e frame=hsides></table>")
          == "border-width: thin; border-style: solid hidden;");
    CHECK(Hints(u8"<table id=e rules=bogus></table>").empty());
    CHECK(Hints(u8"<table id=e rules=all></table>") == "border-collapse: collapse;");
    CHECK(Hints(u8"<table id=e border></table>") == "border-width: 1px;");
    CHECK(Hints(u8"<table><tr><td id=e nowrap width=0 height=5 valign=middle align=center "
                u8"bgcolor=red></td></tr></table>")
          == "white-space: nowrap; height: 5px; vertical-align: middle; "
             "text-align: -webkit-center; background-color: rgb(255, 0, 0);");
    CHECK(Hints(u8"<table><colgroup id=e width=10%></colgroup></table>") == "width: 10%;");
    CHECK(Hints(u8"<table><caption id=e align=bottom></caption></table>") == "caption-side: bottom;");

    SECTION("Additional styles")
    {
      auto document = ResolverFixture::Parse(u8R"(<table id=t border=2 cellpadding=3>
        <colgroup id=cg></colgroup><tbody id=tb><tr><td id=c></td></tr></tbody></table>)");
      auto &table = Downcast<HTMLTableElement>(Downcast<HTMLElement>(*document->GetElementById(u8"t")));
      auto &cell = Downcast<dom::StyledElement>(*document->GetElementById(u8"c"));
      auto &section = Downcast<dom::StyledElement>(*document->GetElementById(u8"tb"));
      auto &colgroup = Downcast<dom::StyledElement>(*document->GetElementById(u8"cg"));

      CHECK(Text(table.AdditionalPresentationalHintStyle())
            == "border-style: outset;");
      CHECK(Text(cell.AdditionalPresentationalHintStyle())
            == "border-width: 1px; border-style: inset; border-color: inherit; padding: 3px;");
      CHECK_FALSE(section.AdditionalPresentationalHintStyle());
      CHECK_FALSE(colgroup.AdditionalPresentationalHintStyle());

      REQUIRE_FALSE(table.SetAttribute(u8"rules", u8"groups").HasException());
      CHECK(Text(cell.AdditionalPresentationalHintStyle()) == "padding: 3px;");
      CHECK(Text(section.AdditionalPresentationalHintStyle())
            == "border-top-width: thin; border-bottom-width: thin; border-top-style: solid; "
               "border-bottom-style: solid;");
      CHECK(Text(colgroup.AdditionalPresentationalHintStyle())
            == "border-left-width: thin; border-right-width: thin; border-left-style: solid; "
               "border-right-style: solid;");

      REQUIRE_FALSE(table.SetAttribute(u8"rules", u8"cols").HasException());
      table.RemoveAttribute(u8"cellpadding");
      CHECK(Text(cell.AdditionalPresentationalHintStyle())
            == "border-left-width: thin; border-right-width: thin; border-left-style: solid; "
               "border-right-style: solid; "
               "border-color: inherit; padding: 1px;");

      REQUIRE_FALSE(table.SetAttribute(u8"frame", u8"void").HasException());
      CHECK_FALSE(table.AdditionalPresentationalHintStyle());
    }
  }

  TEST_CASE_METHOD(HintFixture, "Presentational hints: <body>", "[HTML][PresentationalHints]")
  {
    CHECK(
      Hints(u8"<body id=e bgcolor=black text=white marginwidth=4 topmargin=2>")
      == "background-color: rgb(0, 0, 0); color: rgb(255, 255, 255); margin: 2px 4px;");
    CHECK(Hints(u8"<body id=e background=' a.png '>") == "background-image: url(\"a.png\");");

    SECTION("Link colours")
    {
      auto document = ResolverFixture::Parse(u8"<body link=red vlink=#00f alink=lime>");
      auto style = RenderStyle::Create();
      CHECK(document->LinkColor(style) == boo::Color(SRGBA<uint8> {255, 0, 0}));
      CHECK(document->VisitedLinkColor(style) == boo::Color(SRGBA<uint8> {0, 0, 255}));
      CHECK(document->ActiveLinkColor(style) == boo::Color(SRGBA<uint8> {0, 255, 0}));

      document->Body()->RemoveAttribute(u8"link");
      CHECK(document->LinkColor(style) == css::ColorFromKeyword(css::ValueId::WebkitLink, {}));
      REQUIRE_FALSE(document->Body()->SetAttribute(u8"vlink", u8"transparent").HasException());
      CHECK(document->VisitedLinkColor(style) != boo::Color(SRGBA<uint8> {0, 0, 255}));
    }
  }

  TEST_CASE("Presentational hints: resolution", "[HTML][PresentationalHints]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>.shown { display: block }</style>
      <body link=red><p id=hidden hidden></p><p id=shown class=shown hidden></p><img id=img width=100>
      <a id=link href=x></a><table cellpadding=5><tr><td id=cell></td></tr></table>)"};

    // Hints sit below author rules in the cascade.
    CHECK(fixture.Resolve(fixture.ById(u8"hidden")).Style->Display() == DisplayType::None);
    CHECK(fixture.Resolve(fixture.ById(u8"shown")).Style->Display() == DisplayType::Block);

    auto image = fixture.Resolve(fixture.ById(u8"img"));
    CHECK(ResolverFixture::Serialised(*image.Style, image.Style->Width()) == "100px");

    auto cell = fixture.Resolve(fixture.ById(u8"cell"));
    CHECK(ResolverFixture::Serialised(*cell.Style, cell.Style->PaddingTop()) == "5px");
    REQUIRE(cell.MatchResult);
    CHECK(std::ranges::any_of(cell.MatchResult->AuthorDeclarations,
                              [](const style::MatchedProperties &properties)
                              {
                                return properties.CascadeLayerPriority
                                       == style::RuleSet::CascadeLayerPriorityForPresentationalHints;
                              }));

    // The user agent's `:link { color: -webkit-link }` resolves to the <body link> colour.
    auto link = fixture.Resolve(fixture.ById(u8"link"));
    CHECK(link.Style->GetColor() == style::Color(boo::Color(SRGBA<uint8> {255, 0, 0})));
  }
}
