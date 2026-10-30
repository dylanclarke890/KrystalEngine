#include "Krystal.Booey/Style/StyleSheetContentsCache.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::style::tests
{
  using namespace css;
  using namespace css::tests;

  TEST_CASE("StyleSheetContentsCache", "[Style][StyleSheetContentsCache]")
  {
    auto &cache = StyleSheetContentsCache::Singleton();
    cache.Clear();
    REQUIRE(&cache == &StyleSheetContentsCache::Singleton());

    auto context = CSSOMTestContext();
    auto contents = StyleSheetContents::Create(context);
    contents->ParseString(u8"a { }");
    contents->CheckLoaded();
    REQUIRE(contents->IsCacheable());
    REQUIRE_FALSE(contents->IsInMemoryCache());

    StyleSheetContentsCache::Key key {u8"a { }", context};
    CHECK_FALSE(cache.Get(key));

    cache.Add(StyleSheetContentsCache::Key(key), contents);
    CHECK(cache.Get(key) == contents.get());
    CHECK(contents->IsInMemoryCache());

    SECTION("the key is the text and the context")
    {
      CHECK_FALSE(cache.Get({u8"a {}", context}));

      auto otherContext = context;
      otherContext.Mode = ParserMode::HTMLQuirks;
      CHECK_FALSE(cache.Get({u8"a { }", otherContext}));
    }

    SECTION("adding the same key again keeps the first contents")
    {
      auto other = StyleSheetContents::Create(context);
      other->CheckLoaded();
      cache.Add(StyleSheetContentsCache::Key(key), other);
      CHECK(cache.Get(key) == contents.get());
      CHECK_FALSE(other->IsInMemoryCache());
    }

    SECTION("the cache holds at most 256 sheets")
    {
      for (auto i = 0; i < 256; ++i)
      {
        auto sheet = StyleSheetContents::Create(context);
        sheet->CheckLoaded();
        auto text =
          CSSOMString(u8"b") + CSSOMString(reinterpret_cast<const char8 *>(std::to_string(i).c_str()));
        cache.Add({text, context}, sheet);
      }

      auto inCache = cache.Get(key) ? 1 : 0;
      for (auto i = 0; i < 256; ++i)
      {
        auto text =
          CSSOMString(u8"b") + CSSOMString(reinterpret_cast<const char8 *>(std::to_string(i).c_str()));
        inCache += cache.Get({text, context}) ? 1 : 0;
      }
      CHECK(inCache == 256);
    }

    cache.Clear();
    CHECK_FALSE(cache.Get(key));
  }
}
