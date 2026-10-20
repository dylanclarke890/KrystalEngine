#include "Krystal.Booey/Style/MatchedDeclarationsCache.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    struct CacheFixture : ResolverFixture
    {
      CacheFixture() noexcept
          : ResolverFixture(u8R"(<!DOCTYPE html><style>.c { width: 5px } .v { width: 1vw }</style>
              <body><div id=x class=c></div><div id=y class=c></div><div id=v class=v></div>)")
      {
      }

      KRYS_NODISCARD UniquePtr<RenderStyle> DefaultStyle() noexcept
      {
        return Resolver::DefaultStyleForElement(*Document, nullptr);
      }

      /// @brief A match result with one (mutable) author declaration block, parsed from @p css.
      KRYS_NODISCARD static Ref<MatchResult> Matched(utf8_stringview css,
                                                     IsCacheable isCacheable = IsCacheable::Yes) noexcept
      {
        auto result = MatchResult::Create();
        result->AuthorDeclarations.push_back(
          MatchedProperties {.Properties = css::tests::ParseDeclarations(css), .IsCacheable = isCacheable});
        return result;
      }
    };
  }

  TEST_CASE_METHOD(CacheFixture, "MatchedDeclarationsCache: cacheable styles",
                   "[Style][MatchedDeclarationsCache]")
  {
    auto &x = ById(u8"x");
    auto style = Resolve(x).Style;
    auto parentStyle = Resolve(*Document->Body()).Style;

    CHECK(MatchedDeclarationsCache::IsCacheable(x, *style, *parentStyle));
    // The document element's style changes the document's state.
    CHECK_FALSE(MatchedDeclarationsCache::IsCacheable(*Document->DocumentElement(), *style, *parentStyle));

    SECTION("Zoom")
    {
      (void)style->SetZoom(Zoom {2.0f});
      CHECK_FALSE(MatchedDeclarationsCache::IsCacheable(x, *style, *parentStyle));
    }

    SECTION("Writing mode")
    {
      Builder builder {*style, BuilderContext {.Document = Document, .ParentStyle = parentStyle.get()},
                       *Matched(u8"writing-mode: vertical-rl")};
      builder.ApplyAllProperties();
      CHECK_FALSE(MatchedDeclarationsCache::IsCacheable(x, *style, *parentStyle));
    }

    SECTION("attr() in content")
    {
      style->SetHasAttrContent();
      CHECK_FALSE(MatchedDeclarationsCache::IsCacheable(x, *style, *parentStyle));
    }

    SECTION("Elements with random() values")
    {
      (void)x.LookupCSSRandomBaseValue(
        {},
        css::calc::RandomCachingKey {Variant<css::calc::Random::SharingOptions::Auto, css::CSSOMStringAtom> {
          css::CSSOMStringAtom(u8"k")}});
      CHECK_FALSE(MatchedDeclarationsCache::IsCacheable(x, *style, *parentStyle));
    }
  }

  TEST_CASE_METHOD(CacheFixture, "MatchedDeclarationsCache: hashes", "[Style][MatchedDeclarationsCache]")
  {
    auto style = DefaultStyle();
    auto &customProperties = style->InheritedCustomProperties();

    auto nonCacheable = Matched(u8"width: 1px");
    nonCacheable->IsCompletelyNonCacheable = true;
    CHECK(MatchedDeclarationsCache::ComputeHash(*nonCacheable, customProperties) == 0u);

    // No point caching when only non-cacheable author declarations apply.
    CHECK(MatchedDeclarationsCache::ComputeHash(*Matched(u8"width: 1px", IsCacheable::Partially),
                                                customProperties)
          == 0u);

    auto first = MatchedDeclarationsCache::ComputeHash(*Matched(u8"width: 1px"), customProperties);
    CHECK(first != 0u);
  }

  TEST_CASE_METHOD(CacheFixture, "MatchedDeclarationsCache: entries", "[Style][MatchedDeclarationsCache]")
  {
    MatchedDeclarationsCache cache {GetResolver()};
    auto parentStyle = DefaultStyle();
    auto style = DefaultStyle();
    auto matched = Matched(u8"width: 1px");
    auto &customProperties = parentStyle->InheritedCustomProperties();
    auto hash = MatchedDeclarationsCache::ComputeHash(*matched, customProperties);

    CHECK_FALSE(cache.Find(hash, *matched, customProperties, *parentStyle));
    CHECK_FALSE(cache.Find(0, *matched, customProperties, *parentStyle));

    cache.Add(*style, *parentStyle, hash, *matched);

    SECTION("Found with equal inherited properties")
    {
      auto result = cache.Find(hash, *matched, customProperties, *parentStyle);
      REQUIRE(result);
      CHECK(result->InheritedEqual);
      CHECK(result->Entry.MatchResult.get() == matched.get());
      CHECK(*result->Entry.RenderStyle == *style);
      CHECK(result->Entry.IsUsableAfterHighPriorityProperties(*style));
    }

    SECTION("Found with different inherited properties")
    {
      auto otherParent = DefaultStyle();
      Builder builder {*otherParent, BuilderContext {.Document = Document, .ParentStyle = parentStyle.get()},
                       *Matched(u8"visibility: hidden")};
      builder.ApplyAllProperties();
      auto result = cache.Find(hash, *matched, customProperties, *otherParent);
      REQUIRE(result);
      CHECK_FALSE(result->InheritedEqual);
    }

    SECTION("Different declarations aren't found")
    {
      auto other = Matched(u8"width: 1px");
      CHECK_FALSE(cache.Find(hash, *other, customProperties, *parentStyle));
    }

    SECTION("Not usable after a zoom change")
    {
      auto zoomed = DefaultStyle();
      (void)zoomed->SetUsedZoom(2.0f);
      auto result = cache.Find(hash, *matched, customProperties, *parentStyle);
      REQUIRE(result);
      CHECK_FALSE(result->Entry.IsUsableAfterHighPriorityProperties(*zoomed));
    }

    SECTION("Remove and invalidate")
    {
      cache.Remove(hash);
      CHECK_FALSE(cache.Find(hash, *matched, customProperties, *parentStyle));
      cache.Add(*style, *parentStyle, hash, *matched);
      cache.Invalidate();
      CHECK_FALSE(cache.Find(hash, *matched, customProperties, *parentStyle));
    }

    SECTION("Entries affected by viewport units")
    {
      auto viewportStyle = Resolve(ById(u8"v")).Style;
      REQUIRE(viewportStyle->UsesViewportUnits());
      auto viewportMatched = Matched(u8"width: 1vw");
      cache.Add(*viewportStyle, *parentStyle, hash + 1, *viewportMatched);
      cache.ClearEntriesAffectedByViewportUnits();
      CHECK(cache.Find(hash, *matched, customProperties, *parentStyle));
      CHECK_FALSE(cache.Find(hash + 1, *viewportMatched, customProperties, *parentStyle));
    }
  }

  TEST_CASE_METHOD(CacheFixture, "MatchedDeclarationsCache: sweeping", "[Style][MatchedDeclarationsCache]")
  {
    MatchedDeclarationsCache cache {GetResolver()};
    auto parentStyle = DefaultStyle();
    auto style = DefaultStyle();
    auto &customProperties = parentStyle->InheritedCustomProperties();

    // Only the match result holds these declarations, as when an element's inline style is replaced.
    auto released = Matched(u8"width: 1px");
    cache.Add(*style, *parentStyle, 1, *released);

    auto kept = Matched(u8"width: 2px");
    auto keptProperties = kept->AuthorDeclarations.front().Properties;
    for (uint32 hash = 2; hash <= 100; ++hash)
    {
      cache.Add(*style, *parentStyle, hash, *kept);
    }
    CHECK(cache.Find(1, *released, customProperties, *parentStyle));

    // The 100th addition starts a sweep, a minute later.
    auto &loop = html::EventLoop::Current();
    loop.Run(loop.Now() + std::chrono::seconds(59));
    CHECK(cache.Find(1, *released, customProperties, *parentStyle));
    loop.Run(loop.Now() + std::chrono::seconds(1));
    CHECK(cache.Find(2, *kept, customProperties, *parentStyle));
    CHECK_FALSE(cache.Find(1, *released, customProperties, *parentStyle));
  }

  TEST_CASE_METHOD(CacheFixture, "MatchedDeclarationsCache: the resolver shares styles",
                   "[Style][MatchedDeclarationsCache]")
  {
    auto x = Resolve(ById(u8"x"));
    auto y = Resolve(ById(u8"y"));
    // The second element copies the non-inherited data of the first.
    CHECK(*x.Style == *y.Style);
    CHECK(Serialised(*y.Style, y.Style->Width()) == "5px");

    // Invalidating the cache doesn't change the result.
    GetResolver().InvalidateMatchedDeclarationsCache();
    auto again = Resolve(ById(u8"y"));
    CHECK(*again.Style == *y.Style);
  }
}
