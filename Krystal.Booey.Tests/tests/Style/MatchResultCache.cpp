#include "Krystal.Booey/Style/MatchResultCache.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    struct MatchResultCacheFixture : ResolverFixture
    {
      MatchResultCacheFixture() noexcept
          : ResolverFixture(u8R"(<!DOCTYPE html><style>div { height: 2px }</style>
              <body><div id=x style="width: 5px; opacity: 0.5"></div><div id=plain></div>)")
      {
      }

      UniquePtr<RenderStyle> BodyStyle {};

      KRYS_NODISCARD ResolutionContext Context() noexcept
      {
        if (!BodyStyle)
        {
          BodyStyle = Resolve(*Document->Body()).Style;
        }
        return {.ParentStyle = BodyStyle.get()};
      }

      KRYS_NODISCARD dom::StyledElement &Styled(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::StyledElement>(ById(id));
      }
    };
  }

  TEST_CASE_METHOD(MatchResultCacheFixture, "MatchResultCache", "[Style][MatchResultCache]")
  {
    auto &resolver = GetResolver();
    auto &x = Styled(u8"x");
    MatchResultCache cache;

    // Parsed inline styles are immutable: only inline styles changed through the CSSOM are cached.
    cache.Set(x, resolver.UnadjustedStyleForElement(x, Context()));
    CHECK_FALSE(cache.ResultWithCurrentInlineStyle(x));
    REQUIRE(x.SetInlineStyleProperty(css::PropertyId::Width, css::CSSOMString(u8"6px")));

    auto unadjusted = resolver.UnadjustedStyleForElement(x, Context());
    REQUIRE(unadjusted.MatchResult);
    cache.Set(x, unadjusted);

    SECTION("An unchanged inline style uses the cached result as is")
    {
      auto cached = cache.ResultWithCurrentInlineStyle(x);
      REQUIRE(cached);
      CHECK(cached->ChangedProperties.IsEmpty());
      auto style = resolver.UnadjustedStyleForCachedMatchResult(x, Context(), krys::move(*cached));
      CHECK(Serialised(*style.Style, style.Style->Width()) == "6px");
      CHECK(Serialised(*style.Style, style.Style->Height()) == "2px");
    }

    SECTION("Changed low-priority properties are applied alone")
    {
      REQUIRE(x.SetInlineStyleProperty(css::PropertyId::Width, css::CSSOMString(u8"7px")));
      auto cached = cache.ResultWithCurrentInlineStyle(x);
      REQUIRE(cached);
      REQUIRE(cached->ChangedProperties.Ids.size() == 1u);
      CHECK(cached->ChangedProperties.Ids.front() == css::PropertyId::Width);
      CHECK_FALSE(cached->ChangedProperties.Types);
      auto *styleToUpdate = cached->StyleToUpdate;
      REQUIRE(styleToUpdate);

      auto style = resolver.UnadjustedStyleForCachedMatchResult(x, Context(), krys::move(*cached));
      CHECK(Serialised(*style.Style, style.Style->Width()) == "7px");
      CHECK(style.Style->GetOpacity() == Opacity {0.5f});
      CHECK(Serialised(*style.Style, style.Style->Height()) == "2px");

      CachedMatchResult result {.StyleToUpdate = styleToUpdate};
      MatchResultCache::Update(result, *style.Style);
      CHECK(*styleToUpdate == *style.Style);

      // A property that changed once is assumed to keep changing.
      auto again = cache.ResultWithCurrentInlineStyle(x);
      REQUIRE(again);
      CHECK(again->ChangedProperties.Ids.size() == 1u);
    }

    SECTION("Changed high-priority properties apply every property")
    {
      REQUIRE(x.SetInlineStyleProperty(css::PropertyId::Width, css::CSSOMString(u8"1em")));
      REQUIRE(x.SetInlineStyleProperty(css::PropertyId::Opacity, css::CSSOMString(u8"0.25")));
      // Rebuild the entry with a font size in it.
      REQUIRE(x.SetInlineStyleProperty(css::PropertyId::FontSize, css::CSSOMString(u8"10px")));
      cache.Set(x, resolver.UnadjustedStyleForElement(x, Context()));
      REQUIRE(x.SetInlineStyleProperty(css::PropertyId::FontSize, css::CSSOMString(u8"30px")));

      auto cached = cache.ResultWithCurrentInlineStyle(x);
      REQUIRE(cached);
      CHECK(cached->ChangedProperties.Ids.empty());
      CHECK(cached->ChangedProperties.Types);

      auto style = resolver.UnadjustedStyleForCachedMatchResult(x, Context(), krys::move(*cached));
      CHECK(style.Style->ComputedFontSize() == 30);
      CHECK(Serialised(*style.Style, style.Style->Width()) == "30px");
      CHECK(style.Style->GetOpacity() == Opacity {0.25f});
    }

    SECTION("Adding a property drops the entry")
    {
      REQUIRE(x.SetInlineStyleProperty(css::PropertyId::Height, css::CSSOMString(u8"9px")));
      CHECK_FALSE(cache.ResultWithCurrentInlineStyle(x));
      CHECK_FALSE(cache.ResultWithCurrentInlineStyle(x));
    }

    SECTION("Updating for fast path inheritance")
    {
      cache.UpdateForFastPathInherit(x, *Context().ParentStyle);
      CHECK(cache.ResultWithCurrentInlineStyle(x));
    }

    SECTION("Elements without an inline style aren't cached")
    {
      auto &plain = Styled(u8"plain");
      cache.Set(plain, resolver.UnadjustedStyleForElement(plain, Context()));
      CHECK_FALSE(cache.ResultWithCurrentInlineStyle(plain));
    }

    SECTION("The scope's cache")
    {
      auto &scopeCache = Document->StyleScope().GetMatchResultCache();
      CHECK(&Document->StyleScope().GetMatchResultCache() == &scopeCache);
      scopeCache.Set(x, unadjusted);
      CHECK(scopeCache.ResultWithCurrentInlineStyle(x));
    }
  }
}
