#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD dom::Element &ById(dom::Document &document, dom::DOMStringView id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return *element;
    }

    KRYS_NODISCARD bool Matches(const dom::Element &element, dom::DOMStringView selector)
    {
      auto result = element.Matches(selector);
      REQUIRE_FALSE(result.HasException());
      return result.Value();
    }
  }

  TEST_CASE("SelectorChecker: simple selectors", "[CSS][Selectors][SelectorChecker]")
  {
    auto document =
      Parse(u8"<!DOCTYPE html><div id=a class='x  Y' title=Hello lang=en-US data-v='foo bar-baz'>"
            u8"<input id=b type=TEXT></div>");
    auto &a = ById(*document, u8"a");
    auto &b = ById(*document, u8"b");

    SECTION("Type selectors")
    {
      REQUIRE(Matches(a, u8"div"));
      REQUIRE(Matches(a, u8"DIV"));
      REQUIRE(Matches(a, u8"*"));
      REQUIRE(Matches(a, u8"*|div"));
      REQUIRE_FALSE(Matches(a, u8"span"));
      REQUIRE_FALSE(Matches(a, u8"|div"));
    }

    SECTION("Classes and IDs are case-sensitive in no-quirks mode")
    {
      REQUIRE(Matches(a, u8".x"));
      REQUIRE(Matches(a, u8".Y"));
      REQUIRE(Matches(a, u8".x.Y"));
      REQUIRE_FALSE(Matches(a, u8".y"));
      REQUIRE_FALSE(Matches(a, u8".X"));
      REQUIRE(Matches(a, u8"#a"));
      REQUIRE_FALSE(Matches(a, u8"#A"));
    }

    SECTION("Attribute selectors")
    {
      REQUIRE(Matches(a, u8"[title]"));
      REQUIRE(Matches(a, u8"[TITLE]"));
      REQUIRE(Matches(a, u8"[title=Hello]"));
      REQUIRE_FALSE(Matches(a, u8"[title=hello]"));
      REQUIRE(Matches(a, u8"[title=hello i]"));
      REQUIRE(Matches(a, u8"[data-v~=bar-baz]"));
      REQUIRE_FALSE(Matches(a, u8"[data-v~=bar]"));
      REQUIRE_FALSE(Matches(a, u8"[data-v~='']"));
      REQUIRE(Matches(a, u8"[data-v^=foo]"));
      REQUIRE_FALSE(Matches(a, u8"[data-v^='']"));
      REQUIRE(Matches(a, u8"[data-v$=baz]"));
      REQUIRE(Matches(a, u8"[data-v*='o b']"));
      REQUIRE(Matches(a, u8"[lang|=en]"));
      REQUIRE_FALSE(Matches(a, u8"[lang|=e]"));
      REQUIRE(Matches(a, u8"[*|title]"));
      REQUIRE_FALSE(Matches(a, u8"[nope]"));
    }

    SECTION("Some HTML attribute values match case-insensitively")
    {
      REQUIRE(Matches(b, u8"[type=text]"));
      REQUIRE(Matches(b, u8"[type=TeXt]"));
      REQUIRE(Matches(b, u8"[type=text i]"));
      REQUIRE_FALSE(Matches(b, u8"[id=B]"));
    }

    SECTION("Pseudo-elements never match in queries")
    {
      REQUIRE_FALSE(Matches(a, u8"::before"));
      REQUIRE_FALSE(Matches(a, u8"div::after"));
    }
  }

  TEST_CASE("SelectorChecker: classes and IDs in quirks mode", "[CSS][Selectors][SelectorChecker]")
  {
    auto document = Parse(u8"<div id=Quirky class='Foo'></div>");
    REQUIRE(document->InQuirksMode());
    auto element = document->GetElementById(u8"Quirky");
    REQUIRE(element);
    REQUIRE(Matches(*element, u8".foo"));
    REQUIRE(Matches(*element, u8".FOO"));
    REQUIRE(Matches(*element, u8"#quirky"));
    REQUIRE(Matches(*element, u8"#QUIRKY"));
    REQUIRE_FALSE(Matches(*element, u8".bar"));
  }

  TEST_CASE("SelectorChecker: combinators", "[CSS][Selectors][SelectorChecker]")
  {
    auto document =
      Parse(u8"<!DOCTYPE html><section id=s><div id=d><p id=p1></p><p id=p2></p><span id=sp></span>"
            u8"</div></section>");
    auto &p1 = ById(*document, u8"p1");
    auto &p2 = ById(*document, u8"p2");
    auto &sp = ById(*document, u8"sp");

    REQUIRE(Matches(p1, u8"section p"));
    REQUIRE(Matches(p1, u8"body section div p"));
    REQUIRE(Matches(p1, u8"div > p"));
    REQUIRE_FALSE(Matches(p1, u8"section > p"));
    REQUIRE(Matches(p2, u8"p + p"));
    REQUIRE_FALSE(Matches(p1, u8"p + p"));
    REQUIRE(Matches(sp, u8"p ~ span"));
    REQUIRE_FALSE(Matches(sp, u8"p + span + span"));
    REQUIRE(Matches(sp, u8"#p1 ~ #sp"));
    REQUIRE(Matches(sp, u8"section > div > p ~ span"));
    REQUIRE_FALSE(Matches(p1, u8"span ~ p"));
  }

  TEST_CASE("SelectorChecker: logical combinations", "[CSS][Selectors][SelectorChecker]")
  {
    auto document =
      Parse(u8"<!DOCTYPE html><div id=d class=a><p id=p class=b><em id=e></em></p><span id=s></span>"
            u8"<i id=i></i></div>");
    auto &d = ById(*document, u8"d");
    auto &p = ById(*document, u8"p");
    auto &s = ById(*document, u8"s");
    auto &i = ById(*document, u8"i");

    SECTION(":not(), :is() and :where()")
    {
      REQUIRE(Matches(p, u8":not(div)"));
      REQUIRE_FALSE(Matches(p, u8":not(.b, span)"));
      REQUIRE(Matches(p, u8":is(span, .b)"));
      REQUIRE(Matches(p, u8":where(.a > p)"));
      REQUIRE_FALSE(Matches(p, u8":where(.a > span)"));
      REQUIRE(Matches(p, u8":is(.a .b)"));
    }

    SECTION(":has()")
    {
      REQUIRE(Matches(d, u8":has(em)"));
      REQUIRE(Matches(d, u8":has(> p)"));
      REQUIRE_FALSE(Matches(d, u8":has(> em)"));
      REQUIRE(Matches(p, u8":has(+ span)"));
      REQUIRE_FALSE(Matches(p, u8":has(+ i)"));
      REQUIRE(Matches(p, u8":has(~ i)"));
      REQUIRE_FALSE(Matches(s, u8":has(~ p)"));
      REQUIRE(Matches(d, u8":has(p > em)"));
      REQUIRE_FALSE(Matches(i, u8":has(*)"));
      REQUIRE(Matches(p, u8"div > :has(> em)"));
    }
  }

  TEST_CASE("SelectorChecker: tree-structural pseudo-classes", "[CSS][Selectors][SelectorChecker]")
  {
    auto document = Parse(u8"<!DOCTYPE html><ul id=u><li id=l1 class=x></li><li id=l2></li><b id=b></b>"
                          u8"<li id=l3 class=x></li><li id=l4><!-- comment --></li></ul><p id=empty></p>"
                          u8"<p id=space> </p><div id=only><span id=o></span></div>");
    auto &l1 = ById(*document, u8"l1");
    auto &l2 = ById(*document, u8"l2");
    auto &b = ById(*document, u8"b");
    auto &l3 = ById(*document, u8"l3");
    auto &l4 = ById(*document, u8"l4");

    REQUIRE(Matches(l1, u8":first-child"));
    REQUIRE_FALSE(Matches(l2, u8":first-child"));
    REQUIRE(Matches(l4, u8":last-child"));
    REQUIRE(Matches(b, u8":first-of-type"));
    REQUIRE(Matches(b, u8":last-of-type"));
    REQUIRE(Matches(b, u8":only-of-type"));
    REQUIRE_FALSE(Matches(l1, u8":only-of-type"));
    REQUIRE(Matches(ById(*document, u8"o"), u8":only-child"));
    REQUIRE_FALSE(Matches(l1, u8":only-child"));

    REQUIRE(Matches(l1, u8":nth-child(odd)"));
    REQUIRE(Matches(l2, u8":nth-child(2)"));
    REQUIRE(Matches(l3, u8":nth-child(2n)"));
    REQUIRE(Matches(l3, u8":nth-child(2 of .x)"));
    REQUIRE_FALSE(Matches(l2, u8":nth-child(1 of .x)"));
    REQUIRE(Matches(l3, u8":nth-of-type(3)"));
    REQUIRE(Matches(l1, u8":nth-last-child(5)"));
    REQUIRE(Matches(l1, u8":nth-last-child(2 of .x)"));
    REQUIRE(Matches(l1, u8":nth-last-of-type(4)"));
    REQUIRE(Matches(l4, u8":nth-last-of-type(1)"));

    REQUIRE(Matches(l4, u8":empty"));
    REQUIRE(Matches(ById(*document, u8"empty"), u8":empty"));
    REQUIRE_FALSE(Matches(ById(*document, u8"space"), u8":empty"));
    REQUIRE_FALSE(Matches(ById(*document, u8"only"), u8":empty"));

    REQUIRE(Matches(*document->DocumentElement(), u8":root"));
    REQUIRE_FALSE(Matches(l1, u8":root"));
  }

  TEST_CASE("SelectorChecker: :scope and :host", "[CSS][Selectors][SelectorChecker]")
  {
    auto document = Parse(u8"<!DOCTYPE html><div id=host><p id=light></p></div>");
    auto &host = ById(*document, u8"host");

    REQUIRE(Matches(host, u8":scope"));
    REQUIRE(Matches(*document->DocumentElement(), u8":root:scope"));

    auto shadow = host.AttachShadow({}).ReleaseValue();
    auto inner = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

    REQUIRE(Matches(*inner, u8":host > p"));
    REQUIRE(Matches(*inner, u8":host(div) p"));
    REQUIRE_FALSE(Matches(*inner, u8":host(span) p"));
    REQUIRE_FALSE(Matches(*inner, u8"div > p"));
    REQUIRE_FALSE(Matches(*inner, u8"body p"));
    REQUIRE_FALSE(Matches(host, u8":host"));
  }

  TEST_CASE("SelectorChecker: element state", "[CSS][Selectors][SelectorChecker]")
  {
    auto document = Parse(
      u8"<!DOCTYPE html><a id=link href=x></a><a id=nolink></a>"
      u8"<form><input id=check type=checkbox checked><input id=text required placeholder=hint>"
      u8"<input id=ro readonly><textarea id=ta></textarea><button id=submit1></button>"
      u8"<button id=submit2></button><select id=sel><optgroup disabled><option id=og></option></optgroup>"
      u8"<option id=opt selected></option></select>"
      u8"<fieldset id=fs disabled><legend><input id=inlegend></legend><input id=infs></fieldset></form>"
      u8"<progress id=prog></progress><progress id=progv value=1></progress>"
      u8"<x-widget id=custom></x-widget><div id=edit contenteditable><span id=inedit></span></div>");

    SECTION("Links")
    {
      REQUIRE(Matches(ById(*document, u8"link"), u8":link"));
      REQUIRE(Matches(ById(*document, u8"link"), u8":any-link"));
      REQUIRE_FALSE(Matches(ById(*document, u8"link"), u8":visited"));
      REQUIRE_FALSE(Matches(ById(*document, u8"nolink"), u8":any-link"));
    }

    SECTION("Form controls")
    {
      REQUIRE(Matches(ById(*document, u8"check"), u8":checked"));
      REQUIRE(Matches(ById(*document, u8"check"), u8":default"));
      REQUIRE(Matches(ById(*document, u8"opt"), u8":checked"));
      REQUIRE_FALSE(Matches(ById(*document, u8"og"), u8":checked"));
      REQUIRE(Matches(ById(*document, u8"submit1"), u8":default"));
      REQUIRE_FALSE(Matches(ById(*document, u8"submit2"), u8":default"));

      REQUIRE(Matches(ById(*document, u8"text"), u8":required"));
      REQUIRE(Matches(ById(*document, u8"check"), u8":optional"));
      REQUIRE(Matches(ById(*document, u8"sel"), u8":optional"));
      REQUIRE_FALSE(Matches(ById(*document, u8"link"), u8":optional"));

      REQUIRE(Matches(ById(*document, u8"text"), u8":placeholder-shown"));
      REQUIRE_FALSE(Matches(ById(*document, u8"ro"), u8":placeholder-shown"));

      REQUIRE(Matches(ById(*document, u8"text"), u8":read-write"));
      REQUIRE(Matches(ById(*document, u8"ta"), u8":read-write"));
      REQUIRE(Matches(ById(*document, u8"ro"), u8":read-only"));
      REQUIRE(Matches(ById(*document, u8"check"), u8":read-only"));
      REQUIRE(Matches(ById(*document, u8"link"), u8":read-only"));
      REQUIRE(Matches(ById(*document, u8"edit"), u8":read-write"));
      REQUIRE(Matches(ById(*document, u8"inedit"), u8":read-write"));

      REQUIRE(Matches(ById(*document, u8"prog"), u8":indeterminate"));
      REQUIRE_FALSE(Matches(ById(*document, u8"progv"), u8":indeterminate"));
    }

    SECTION("Disabled and enabled")
    {
      REQUIRE(Matches(ById(*document, u8"text"), u8":enabled"));
      REQUIRE(Matches(ById(*document, u8"og"), u8":disabled"));
      REQUIRE(Matches(ById(*document, u8"opt"), u8":enabled"));
      REQUIRE(Matches(ById(*document, u8"fs"), u8":disabled"));
      REQUIRE(Matches(ById(*document, u8"infs"), u8":disabled"));
      REQUIRE(Matches(ById(*document, u8"inlegend"), u8":enabled"));
      REQUIRE(Matches(ById(*document, u8"infs"), u8":read-only"));
      REQUIRE_FALSE(Matches(ById(*document, u8"link"), u8":enabled"));
      REQUIRE_FALSE(Matches(ById(*document, u8"link"), u8":disabled"));
    }

    SECTION("Custom elements")
    {
      REQUIRE(Matches(ById(*document, u8"link"), u8":defined"));
      REQUIRE_FALSE(Matches(ById(*document, u8"custom"), u8":defined"));
    }
  }

  TEST_CASE("SelectorChecker: :lang() and :dir()", "[CSS][Selectors][SelectorChecker]")
  {
    auto document = Parse(u8"<!DOCTYPE html><html lang=en-US><body><p id=p></p><div lang=fr-Latn-CA dir=rtl>"
                          u8"<span id=s></span><b id=b dir=ltr></b></div><i id=i lang=''></i></body></html>");
    auto &p = ById(*document, u8"p");
    auto &s = ById(*document, u8"s");

    REQUIRE(Matches(p, u8":lang(en)"));
    REQUIRE(Matches(p, u8":lang(en-US)"));
    REQUIRE(Matches(p, u8":lang(EN)"));
    REQUIRE(Matches(p, u8":lang(\"*-US\")"));
    REQUIRE_FALSE(Matches(p, u8":lang(fr)"));
    REQUIRE(Matches(s, u8":lang(fr, de)"));
    REQUIRE(Matches(s, u8":lang(fr-CA)"));
    REQUIRE_FALSE(Matches(s, u8":lang(en)"));
    REQUIRE_FALSE(Matches(ById(*document, u8"i"), u8":lang(en)"));

    REQUIRE(Matches(p, u8":dir(ltr)"));
    REQUIRE(Matches(s, u8":dir(rtl)"));
    REQUIRE(Matches(ById(*document, u8"b"), u8":dir(ltr)"));
    REQUIRE_FALSE(Matches(s, u8":dir(ltr)"));
  }

  TEST_CASE("SelectorChecker: link match type", "[CSS][Selectors][SelectorChecker]")
  {
    auto linkMatchType = [](dom::DOMStringView text)
    {
      auto list = SelectorParser::ParseSelectorList(text, {});
      REQUIRE(list);
      return SelectorChecker::DetermineLinkMatchType(list->FirstSelectorComponent());
    };
    REQUIRE(linkMatchType(u8"a") == SelectorChecker::MatchAll);
    REQUIRE(linkMatchType(u8"a:link") == SelectorChecker::MatchLink);
    REQUIRE(linkMatchType(u8"a:visited") == SelectorChecker::MatchVisited);
    REQUIRE(linkMatchType(u8":link:visited") == SelectorChecker::MatchDefault);
    REQUIRE(linkMatchType(u8":visited span") == SelectorChecker::MatchVisited);
    REQUIRE(linkMatchType(u8":visited + span") == SelectorChecker::MatchAll);
  }
}
