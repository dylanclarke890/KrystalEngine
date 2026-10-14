#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSKeyframesRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Style/StyleSheetContentsCache.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<CSSStyleSheet> ConstructedSheet(dom::Document &document, CSSStyleSheet::Init init = {})
    {
      auto sheet = CSSStyleSheet::Create(document, krys::move(init));
      REQUIRE(sheet.HasValue());
      return sheet.ReleaseValue();
    }
  }

  TEST_CASE("CSSStyleSheet: a sheet from parsed contents", "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto sheet = SheetFromText(u8"a { color: red; } @media screen { b { color: blue; } }");

    REQUIRE(static_cast<StyleSheet &>(*sheet).IsCSSStyleSheet());
    REQUIRE(Is<CSSStyleSheet>(static_cast<StyleSheet &>(*sheet)));
    REQUIRE(ToNarrow(static_cast<StyleSheet &>(*sheet).Type()) == "text/css");
    REQUIRE(sheet->Href().empty());
    REQUIRE(sheet->Title().empty());
    REQUIRE_FALSE(sheet->Disabled());
    REQUIRE(sheet->OwnerNode() == nullptr);
    REQUIRE(sheet->OwnerRule() == nullptr);
    REQUIRE(sheet->ParentStyleSheet() == nullptr);
    REQUIRE(sheet->OwnerDocument() == nullptr);
    REQUIRE(sheet->ConstructorDocument() == nullptr);
    REQUIRE(sheet->StyleScope() == nullptr);
    REQUIRE(&sheet->RootStyleSheet() == sheet.get());
    REQUIRE_FALSE(sheet->WasConstructedByJS());
    REQUIRE_FALSE(sheet->IsLoading());
    REQUIRE(sheet->IsDetached());
    REQUIRE(sheet->CanAccessRules());

    REQUIRE(sheet->Length() == 2);
    REQUIRE(sheet->Item(2) == nullptr);
    REQUIRE(Is<CSSStyleRule>(*sheet->Item(0)));
    REQUIRE(Is<CSSMediaRule>(*sheet->Item(1)));
    // Wrappers are created once and kept.
    REQUIRE(sheet->Item(0) == sheet->Item(0));
    REQUIRE(sheet->Item(0)->ParentStyleSheet() == sheet.get());
    REQUIRE(sheet->Item(0)->ParentRule() == nullptr);

    REQUIRE(SheetText(*sheet) == "a { color: red; } @media screen {\n  b { color: blue; }\n}");

    SECTION("cssRules is a live list")
    {
      auto rules = sheet->CssRules();
      REQUIRE(rules);
      REQUIRE(rules == sheet->CssRules());
      REQUIRE(rules->GetStyleSheet() == sheet.get());
      REQUIRE(rules->Length() == 2);
      REQUIRE(rules->Item(1) == sheet->Item(1));
      REQUIRE(rules->IsSupportedPropertyIndex(1));
      REQUIRE_FALSE(rules->IsSupportedPropertyIndex(2));

      REQUIRE_FALSE(sheet->DeleteRule(0).HasException());
      REQUIRE(rules->Length() == 1);
      REQUIRE(Is<CSSMediaRule>(*rules->Item(0)));

      auto bindings = sheet->CssRulesForBindings();
      REQUIRE(bindings.HasValue());
      REQUIRE(bindings.Value().get() == rules.get());
      auto legacy = sheet->Rules();
      REQUIRE(legacy.HasValue());
      REQUIRE(legacy.Value().get() == rules.get());
    }

    SECTION("disabled")
    {
      sheet->SetDisabled(true);
      REQUIRE(sheet->Disabled());
      sheet->SetDisabled(false);
      REQUIRE_FALSE(sheet->Disabled());
    }

    SECTION("title")
    {
      sheet->SetTitle(u8"Title");
      REQUIRE(ToNarrow(sheet->Title()) == "Title");
    }
  }

  TEST_CASE("CSSStyleSheet: insertRule", "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto sheet = SheetFromText(u8"a { color: red; }");
    auto *first = sheet->Item(0);

    SECTION("inserts at the index and keeps the wrappers")
    {
      auto result = sheet->InsertRule(u8"b { color: blue; }", 0);
      REQUIRE(result.HasValue());
      REQUIRE(result.Value() == 0);
      REQUIRE(sheet->Length() == 2);
      REQUIRE(sheet->Item(1) == first);
      REQUIRE(SheetText(*sheet) == "b { color: blue; } a { color: red; }");

      REQUIRE(sheet->InsertRule(u8"c { }", 2).HasValue());
      REQUIRE(SheetText(*sheet) == "b { color: blue; } a { color: red; } c { }");
    }

    SECTION("an index past the end")
    {
      REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"b { }", 2)) == dom::ExceptionCode::IndexSizeError);
    }

    SECTION("a rule that doesn't parse")
    {
      REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"x", 0)) == dom::ExceptionCode::SyntaxError);
      REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"", 0)) == dom::ExceptionCode::SyntaxError);
      REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"a { } b { }", 0)) == dom::ExceptionCode::SyntaxError);
    }

    SECTION("a rule in the wrong place")
    {
      // @import must come before style rules.
      REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"@import \"a.css\";", 1))
              == dom::ExceptionCode::HierarchyRequestError);
      REQUIRE(sheet->InsertRule(u8"@import \"a.css\";", 0).HasValue());
      REQUIRE(Is<CSSImportRule>(*sheet->Item(0)));
      REQUIRE(sheet->Item(1) == first);
    }

    SECTION("an @namespace rule after other rules")
    {
      REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"@namespace a url(a);", 0))
              == dom::ExceptionCode::InvalidStateError);
    }
  }

  TEST_CASE("CSSStyleSheet: deleteRule", "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto sheet = SheetFromText(u8"a { } b { }");
    RefPtr<CSSRule> first = ShareRefPtr(sheet->Item(0));

    REQUIRE(ExceptionCodeOf(sheet->DeleteRule(2)) == dom::ExceptionCode::IndexSizeError);

    REQUIRE_FALSE(sheet->DeleteRule(0).HasException());
    REQUIRE(sheet->Length() == 1);
    REQUIRE(SheetText(*sheet) == "b { }");
    // The deleted rule's wrapper is detached.
    REQUIRE(first->ParentStyleSheet() == nullptr);

    REQUIRE_FALSE(sheet->RemoveRule(0).HasException());
    REQUIRE(sheet->Length() == 0);
    REQUIRE(SheetText(*sheet).empty());
  }

  TEST_CASE("CSSStyleSheet: addRule", "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto sheet = SheetFromText(u8"a { }");

    auto result = sheet->AddRule(u8"b", u8"color: red", null);
    REQUIRE(result.HasValue());
    REQUIRE(result.Value() == -1);
    REQUIRE(SheetText(*sheet) == "a { } b { color: red; }");

    REQUIRE(sheet->AddRule(u8"c", u8"", 0).HasValue());
    REQUIRE(SheetText(*sheet) == "c { } a { } b { color: red; }");

    REQUIRE(ExceptionCodeOf(sheet->AddRule(u8"d", u8"", 4)) == dom::ExceptionCode::IndexSizeError);
  }

  TEST_CASE("CSSStyleSheet: rules of a sheet that isn't origin-clean", "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto contents = StyleSheetContents::Create(CSSOMTestContext());
    contents->ParseString(u8"a { }");
    auto sheet = CSSStyleSheet::Create(krys::move(contents), nullptr, false);

    REQUIRE_FALSE(sheet->CanAccessRules());
    REQUIRE_FALSE(sheet->CssRules());
    REQUIRE(ExceptionCodeOf(sheet->CssRulesForBindings()) == dom::ExceptionCode::SecurityError);
    REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"b { }", 0)) == dom::ExceptionCode::SecurityError);
    REQUIRE(ExceptionCodeOf(sheet->DeleteRule(0)) == dom::ExceptionCode::SecurityError);
    // The serialisation skips the access check.
    REQUIRE(SheetText(*sheet) == "a { }");
  }

  TEST_CASE("CSSStyleSheet: a sheet with a base URL needs an owner document to be accessed",
            "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto context = CSSOMTestContext();
    context.BaseURL = url::URL(u8"https://example.com/a.css");
    auto contents = StyleSheetContents::Create(context);
    contents->ParseString(u8"a { }");

    SECTION("no owner")
    {
      auto sheet = CSSStyleSheet::Create(Ref<StyleSheetContents>(contents));
      REQUIRE_FALSE(sheet->CanAccessRules());
      REQUIRE(ToNarrow(sheet->BaseURL().String()) == "https://example.com/a.css");
    }

    SECTION("an owner node")
    {
      auto document = CreateRef<dom::Document>();
      auto text = document->CreateTextNode(u8"a");
      auto sheet = CSSStyleSheet::Create(Ref<StyleSheetContents>(contents), *text);
      REQUIRE(sheet->OwnerNode() == text.get());
      REQUIRE(sheet->OwnerDocument() == document.get());
      REQUIRE(sheet->StyleScope() == &document->StyleScope());
      REQUIRE_FALSE(sheet->IsDetached());
      REQUIRE(sheet->CanAccessRules());

      sheet->DetachFromDocument();
      REQUIRE(sheet->OwnerNode() == nullptr);
      REQUIRE(sheet->IsDetached());
    }
  }

  TEST_CASE("CSSStyleSheet: the media list", "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto sheet = SheetFromText(u8"a { }");
    auto *media = sheet->Media();
    REQUIRE(media);
    REQUIRE(media == sheet->Media());
    REQUIRE(media->ParentStyleSheet() == sheet.get());
    REQUIRE(media->Length() == 0);

    media->SetMediaText(u8"screen, print");
    REQUIRE(sheet->MediaQueries().size() == 2);
    REQUIRE(ToNarrow(media->MediaText()) == "screen, print");
  }

  TEST_CASE("CSSStyleSheet: constructed sheets", "[CSS][CSSOM][CSSStyleSheet]")
  {
    style::StyleSheetContentsCache::Singleton().Clear();
    auto document = CreateRef<dom::Document>();

    SECTION("defaults")
    {
      auto sheet = ConstructedSheet(*document);
      REQUIRE(sheet->WasConstructedByJS());
      REQUIRE(sheet->ConstructorDocument() == document.get());
      REQUIRE(sheet->OwnerDocument() == nullptr);
      REQUIRE_FALSE(sheet->Disabled());
      REQUIRE(sheet->CanAccessRules());
      REQUIRE(sheet->Length() == 0);
      REQUIRE(sheet->MediaQueries().empty());
      REQUIRE(ToNarrow(sheet->BaseURL().String()) == "about:blank");
    }

    SECTION("options")
    {
      auto sheet = ConstructedSheet(
        *document, {.BaseURL = u8"https://example.com/", .Media = CSSOMString(u8"screen"), .Disabled = true});
      REQUIRE(sheet->Disabled());
      REQUIRE(ToNarrow(sheet->Media()->MediaText()) == "screen");
      REQUIRE(ToNarrow(sheet->BaseURL().String()) == "https://example.com/");

      auto other = ConstructedSheet(*document, {.Media = ShareRefPtr(sheet->Media())});
      REQUIRE(ToNarrow(other->Media()->MediaText()) == "screen");
    }

    SECTION("a relative base URL is resolved against the document base URL")
    {
      document->SetURL(url::URL(u8"https://example.com/dir/page.html"));

      auto sheet = ConstructedSheet(*document, {.BaseURL = u8"sheets/"});
      REQUIRE(ToNarrow(sheet->BaseURL().String()) == "https://example.com/dir/sheets/");

      auto defaultSheet = ConstructedSheet(*document);
      REQUIRE(ToNarrow(defaultSheet->BaseURL().String()) == "https://example.com/dir/page.html");
    }

    SECTION("an invalid base URL")
    {
      auto sheet = CSSStyleSheet::Create(*document, {.BaseURL = u8"relative"});
      REQUIRE(ExceptionCodeOf(sheet) == dom::ExceptionCode::NotAllowedError);
    }

    SECTION("@import rules can't be inserted")
    {
      auto sheet = ConstructedSheet(*document);
      REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"@import \"a.css\";", 0))
              == dom::ExceptionCode::SyntaxError);
    }

    SECTION("replaceSync ignores @import rules")
    {
      auto sheet = ConstructedSheet(*document);
      REQUIRE_FALSE(sheet->ReplaceSync(u8"@import \"a.css\"; a { color: red; }").HasException());
      REQUIRE(SheetText(*sheet) == "a { color: red; }");
    }

    SECTION("replaceSync replaces the rules")
    {
      auto sheet = ConstructedSheet(*document);
      REQUIRE_FALSE(sheet->ReplaceSync(u8"a { color: red; }").HasException());
      RefPtr<CSSRule> first = ShareRefPtr(sheet->Item(0));

      REQUIRE_FALSE(sheet->ReplaceSync(u8"b { color: blue; } c { }").HasException());
      REQUIRE(sheet->Length() == 2);
      REQUIRE(SheetText(*sheet) == "b { color: blue; } c { }");
      REQUIRE(first->ParentStyleSheet() == nullptr);
    }

    SECTION("replaceSync on a sheet that wasn't constructed")
    {
      auto sheet = SheetFromText(u8"a { }");
      REQUIRE(ExceptionCodeOf(sheet->ReplaceSync(u8"b { }")) == dom::ExceptionCode::NotAllowedError);
      REQUIRE(SheetText(*sheet) == "a { }");
    }
  }

  TEST_CASE("CSSStyleSheet: replaceSync shares cached contents, copied on write",
            "[CSS][CSSOM][CSSStyleSheet]")
  {
    style::StyleSheetContentsCache::Singleton().Clear();
    auto document = CreateRef<dom::Document>();

    auto first = ConstructedSheet(*document);
    auto second = ConstructedSheet(*document);
    REQUIRE_FALSE(first->ReplaceSync(u8"a { color: red; }").HasException());
    REQUIRE_FALSE(second->ReplaceSync(u8"a { color: red; }").HasException());
    REQUIRE(&first->Contents() == &second->Contents());
    REQUIRE(first->Contents().IsInMemoryCache());

    SECTION("inserting a rule")
    {
      REQUIRE(first->InsertRule(u8"b { }", 1).HasValue());
      REQUIRE(&first->Contents() != &second->Contents());
      REQUIRE(SheetText(*first) == "a { color: red; } b { }");
      REQUIRE(SheetText(*second) == "a { color: red; }");
    }

    SECTION("changing a declaration through an existing wrapper")
    {
      auto &rule = RuleAt<CSSStyleRule>(*first, 0);
      auto &style = rule.Style();
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
      REQUIRE(&first->Contents() != &second->Contents());
      // The wrapper and its declarations follow the copy.
      REQUIRE(RuleText(rule) == "a { color: blue; }");
      REQUIRE(ToNarrow(style.CssText()) == "color: blue;");
      REQUIRE(SheetText(*second) == "a { color: red; }");
    }

    SECTION("a sheet whose rules were wrapped doesn't use the cache")
    {
      std::ignore = first->Item(0);
      REQUIRE_FALSE(first->ReplaceSync(u8"a { color: red; }").HasException());
      REQUIRE(&first->Contents() != &second->Contents());
      REQUIRE(SheetText(*first) == "a { color: red; }");
    }
  }

  TEST_CASE("CSSStyleSheet: copying contents reattaches every wrapper", "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto context = CSSOMTestContext();
    auto contents = StyleSheetContents::Create(context);
    contents->ParseString(u8"@media screen { a { color: red; } } b { color: blue; }");
    contents->CheckLoaded();
    REQUIRE(contents->IsCacheable());

    auto first = CSSStyleSheet::Create(Ref<StyleSheetContents>(contents));
    auto second = CSSStyleSheet::Create(Ref<StyleSheetContents>(contents));

    auto &media = RuleAt<CSSMediaRule>(*first, 0);
    auto &nested = Downcast<CSSStyleRule>(*media.Item(0));
    auto &plain = RuleAt<CSSStyleRule>(*first, 1);

    nested.SetSelectorText(u8"c");
    REQUIRE(&first->Contents() != contents.get());
    REQUIRE(&second->Contents() == contents.get());
    REQUIRE(SheetText(*first) == "@media screen {\n  c { color: red; }\n} b { color: blue; }");
    REQUIRE(SheetText(*second) == "@media screen {\n  a { color: red; }\n} b { color: blue; }");

    // The other wrappers point at the copy too, so changes through them don't reach the shared contents.
    REQUIRE_FALSE(plain.Style().SetProperty(u8"color", u8"green", u8"").HasException());
    REQUIRE(SheetText(*first) == "@media screen {\n  c { color: red; }\n} b { color: green; }");
    REQUIRE(SheetText(*second) == "@media screen {\n  a { color: red; }\n} b { color: blue; }");
  }

  TEST_CASE("CSSStyleSheet: the wrappers outlive their sheet", "[CSS][CSSOM][CSSStyleSheet]")
  {
    RefPtr<CSSRule> rule;
    RefPtr<MediaList> media;
    {
      auto sheet = SheetFromText(u8"a { color: red; }");
      rule = ShareRefPtr(sheet->Item(0));
      media = ShareRefPtr(sheet->Media());
      sheet->Media()->SetMediaText(u8"print");
    }

    REQUIRE(rule->ParentStyleSheet() == nullptr);
    REQUIRE(RuleText(*rule) == "a { color: red; }");
    REQUIRE(media->ParentStyleSheet() == nullptr);
    REQUIRE(ToNarrow(media->MediaText()) == "print");
  }

  TEST_CASE("CSSStyleSheet: getChildStyleSheets with an @import that hasn't loaded",
            "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto sheet = SheetFromText(u8"@import \"a.css\"; a { }");
    HashSet<RefPtr<CSSStyleSheet>> children;
    sheet->GetChildStyleSheets(children);
    REQUIRE(children.empty());
    REQUIRE(RuleAt<CSSImportRule>(*sheet, 0).GetStyleSheet() == nullptr);
  }

  TEST_CASE("CSSStyleSheet: insertRule into a sheet with namespaces", "[CSS][CSSOM][CSSStyleSheet]")
  {
    auto sheet = SheetFromText(u8"@namespace svg url(http://www.w3.org/2000/svg);");

    REQUIRE(sheet->InsertRule(u8"@namespace html url(http://www.w3.org/1999/xhtml);", 1).HasValue());
    REQUIRE(Is<CSSNamespaceRule>(*sheet->Item(1)));
    REQUIRE(sheet->InsertRule(u8"html|b { }", 2).HasValue());
    REQUIRE(RuleText(*sheet->Item(2)) == "html|b { }");

    // Once there are other rules, @namespace rules can't be inserted.
    REQUIRE(ExceptionCodeOf(sheet->InsertRule(u8"@namespace x url(x);", 2))
            == dom::ExceptionCode::InvalidStateError);
  }
}
