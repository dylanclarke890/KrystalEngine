#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    using css::tests::ToNarrow;

    Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    Ref<HTMLStyleElement> CreateStyle(dom::Document &document, dom::DOMString &&text) noexcept
    {
      auto style = CreateRef<HTMLStyleElement>(document);
      REQUIRE_FALSE(style->TextContent(krys::move(text)).HasException());
      return style;
    }

    Ref<HTMLStyleElement> AsStyle(dom::Node &node) noexcept
    {
      return ShareRef(Downcast<HTMLStyleElement>(Downcast<HTMLElement>(node)));
    }

    css::CSSStyleSheet *SheetAt(dom::Document &document, uint32 index) noexcept
    {
      return DynamicDowncast<css::CSSStyleSheet>(document.StyleSheets().Item(index));
    }

    uint32 RuleCount(css::CSSStyleSheet &sheet) noexcept
    {
      return sheet.CssRules()->Length();
    }

    struct EventCounter : public dom::EventListenerCallback
    {
      uint32 &Count;

      explicit EventCounter(uint32 &count) noexcept : Count(count)
      {
      }

      void HandleEvent(dom::Event &) noexcept override
      {
        Count++;
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLStyleElement: the style sheet", "[HTML][HTMLStyleElement]")
  {
    EventLoop::Current().RunUntilIdle();

    SECTION("A parsed style element has a sheet")
    {
      auto document = Parse(u8"<!DOCTYPE html><style>a { color: red } b { color: blue }</style>");
      REQUIRE(document->StyleSheets().Length() == 1u);

      auto *sheet = SheetAt(*document, 0);
      REQUIRE(sheet != nullptr);
      auto style = document->DocumentElement()->FirstElementChild()->FirstElementChild();
      REQUIRE(sheet->OwnerNode() == style.get());
      REQUIRE(RuleCount(*sheet) == 2u);
      REQUIRE(document->StyleScope().ActiveStyleSheetsContains(*sheet));
    }

    SECTION("Sheets are in tree order")
    {
      auto document = Parse(u8"<!DOCTYPE html><style>a {}</style><body><p></p><style>b {}</style>");
      auto *body = document->Body().get();
      auto inserted = CreateStyle(*document, u8"c {}");
      REQUIRE_FALSE(body->InsertBefore(*inserted, body->FirstChild()).HasException());

      REQUIRE(document->StyleSheets().Length() == 3u);
      REQUIRE(ToNarrow(SheetAt(*document, 0)->CssRules()->Item(0)->CssText()) == "a { }");
      REQUIRE(ToNarrow(SheetAt(*document, 1)->CssRules()->Item(0)->CssText()) == "c { }");
      REQUIRE(ToNarrow(SheetAt(*document, 2)->CssRules()->Item(0)->CssText()) == "b { }");
    }

    SECTION("Changing the text replaces the sheet")
    {
      auto document = Parse(u8"<!DOCTYPE html><body>");
      auto style = CreateStyle(*document, u8"a {}");
      REQUIRE(style->Sheet() == nullptr);
      REQUIRE_FALSE(document->Body()->AppendChild(*style).HasException());

      auto first = ShareRefPtr(style->Sheet());
      REQUIRE(first != nullptr);
      REQUIRE(RuleCount(*first) == 1u);

      auto &text = Downcast<dom::Text>(*style->FirstChild());
      REQUIRE_FALSE(text.Data(u8"a {} b {}").HasException());
      REQUIRE(style->Sheet() != first.get());
      REQUIRE(RuleCount(*style->Sheet()) == 2u);
      REQUIRE(first->OwnerNode() == nullptr);

      REQUIRE_FALSE(style->AppendChild(*document->CreateTextNode(u8" c {}")).HasException());
      REQUIRE(RuleCount(*style->Sheet()) == 3u);
      REQUIRE(document->StyleSheets().Item(0) == style->Sheet());
    }

    SECTION("Removing the element removes its sheet")
    {
      auto document = Parse(u8"<!DOCTYPE html><body><style>a {}</style>");
      auto style = AsStyle(*document->Body()->FirstElementChild());
      auto sheet = ShareRefPtr(style->Sheet());

      REQUIRE_FALSE(style->Remove().HasException());
      REQUIRE(style->Sheet() == nullptr);
      REQUIRE(sheet->OwnerNode() == nullptr);
      REQUIRE(document->StyleSheets().Length() == 0u);
      REQUIRE_FALSE(document->StyleScope().ActiveStyleSheetsContains(*sheet));
    }

    SECTION("Only CSS is a style sheet")
    {
      auto document = Parse(u8"<!DOCTYPE html><style type=text/plain>a {}</style>");
      auto style = AsStyle(*document->DocumentElement()->FirstElementChild()->FirstElementChild());
      REQUIRE(style->Sheet() == nullptr);

      REQUIRE_FALSE(style->SetAttribute(u8"type", u8"TEXT/CSS").HasException());
      REQUIRE(style->Sheet() != nullptr);
      REQUIRE(document->StyleSheets().Length() == 1u);

      style->RemoveAttribute(u8"type");
      REQUIRE(style->Sheet() != nullptr);
    }

    SECTION("The media attribute")
    {
      auto document = Parse(u8"<!DOCTYPE html><style media=print>a {}</style>");
      auto style = AsStyle(*document->DocumentElement()->FirstElementChild()->FirstElementChild());
      REQUIRE(ToNarrow(style->Sheet()->Media()->MediaText()) == "print");

      REQUIRE_FALSE(style->SetAttribute(u8"media", u8"screen and (min-width: 1px)").HasException());
      REQUIRE(ToNarrow(style->Sheet()->Media()->MediaText()) == "screen and (min-width: 1px)");
      REQUIRE(document->StyleScope().HasPendingUpdate());
    }

    SECTION("The title attribute")
    {
      auto document = Parse(u8"<!DOCTYPE html><style title=one>a {}</style>");
      auto style = AsStyle(*document->DocumentElement()->FirstElementChild()->FirstElementChild());
      REQUIRE(ToNarrow(style->Sheet()->Title()) == "one");

      REQUIRE_FALSE(style->SetAttribute(u8"title", u8"two").HasException());
      REQUIRE(ToNarrow(style->Sheet()->Title()) == "two");
    }

    SECTION("disabled")
    {
      auto document = Parse(u8"<!DOCTYPE html><style>a {}</style>");
      auto style = AsStyle(*document->DocumentElement()->FirstElementChild()->FirstElementChild());
      REQUIRE_FALSE(style->Disabled());

      style->Disabled(true);
      REQUIRE(style->Disabled());
      REQUIRE(style->Sheet()->Disabled());
      document->StyleScope().FlushPendingUpdate();
      REQUIRE(document->StyleScope().ActiveStyleSheets().empty());
      REQUIRE(document->StyleSheets().Length() == 1u);

      auto unconnected = CreateStyle(*document, u8"a {}");
      unconnected->Disabled(true);
      REQUIRE_FALSE(unconnected->Disabled());
    }

    SECTION("Style elements in shadow trees belong to the shadow root")
    {
      auto document = Parse(u8"<!DOCTYPE html><body><div></div>");
      auto host = document->Body()->FirstElementChild();
      auto shadow = host->AttachShadow({}).ReleaseValue();
      auto style = CreateStyle(*document, u8"a {}");
      REQUIRE_FALSE(shadow->AppendChild(*style).HasException());

      REQUIRE(document->StyleSheets().Length() == 0u);
      REQUIRE(shadow->StyleSheets().Length() == 1u);
      REQUIRE(shadow->StyleSheets().Item(0) == style->Sheet());
      REQUIRE(shadow->StyleSheets().OwnerNode() == shadow.get());
      REQUIRE(document->StyleSheets().OwnerNode() == document.get());
    }

    SECTION("Identical sheets share their contents")
    {
      auto document =
        Parse(u8"<!DOCTYPE html><style>a { color: red }</style><style>a { color: red }</style>");
      REQUIRE(document->StyleSheets().Length() == 2u);
      REQUIRE(&SheetAt(*document, 0)->Contents() == &SheetAt(*document, 1)->Contents());
    }

    SECTION("A load event is fired")
    {
      auto document = Parse(u8"<!DOCTYPE html><body>");
      uint32 loads = 0;
      auto style = CreateStyle(*document, u8"a {}");
      style->AddEventListener(u8"load", CreateRef<EventCounter>(loads), false);
      REQUIRE_FALSE(document->Body()->AppendChild(*style).HasException());

      REQUIRE(loads == 0u);
      EventLoop::Current().RunUntilIdle();
      REQUIRE(loads == 1u);
    }
  }
}
