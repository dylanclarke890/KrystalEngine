#include "Krystal.Booey/Loader/Cache/CachedFont.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/FontCreationContext.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFontClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include <filesystem>

namespace krys::boo::tests
{
  namespace
  {
    struct FontLoadCounter final : CachedFontClient
    {
      int32 Loaded {0};

      void FontLoaded(CachedFont &) noexcept override
      {
        ++Loaded;
      }
    };

    KRYS_NODISCARD CachedResourceHandle<CachedFont> RequestFont(dom::Document &document,
                                                                utf8_stringview url) noexcept
    {
      CachedResourceRequest request(ResourceRequest(url::URL(url)),
                                    CachedResourceLoader::DefaultCachedResourceOptions());
      auto font = document.GetCachedResourceLoader().RequestFont(krys::move(request), false);
      REQUIRE(font);
      return *font;
    }
  }

  TEST_CASE("CachedFont: the file loads when it's first needed, and becomes a font", "[Loader][CachedFont]")
  {
    LoaderFixture loader;
    loader.Strategy.AddFile(u8"file:///fonts/ahem.woff2",
                            std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "webfonts" / "Ahem.woff2",
                            u8"font/woff2");
    PageInFrameFixture fixture(u8"<body></body>");

    auto font = RequestFont(*fixture.Document, u8"file:///fonts/ahem.woff2");
    CHECK(font->StillNeedsLoad());
    CHECK(font->IsLoading());
    CHECK(loader.Strategy.Requests.empty());
    CHECK_FALSE(font->EnsureCustomFontData());

    FontLoadCounter client;
    font->AddClient(client);
    CHECK(client.Loaded == 0);

    font->BeginLoadIfNeeded(fixture.Document->GetCachedResourceLoader());
    font->BeginLoadIfNeeded(fixture.Document->GetCachedResourceLoader());
    RunNetworkingTasks();

    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///fonts/ahem.woff2"});
    CHECK(client.Loaded == 1);
    CHECK_FALSE(font->IsLoading());
    REQUIRE(font->EnsureCustomFontData());

    FontCascadeDescription description;
    description.SetComputedSize(20);
    auto created = font->CreateFont(description, false, false, FontCreationContext {});
    REQUIRE(created);
    CHECK(created->GetOrigin() == Font::Origin::Remote);
    CHECK(created->PlatformData().FamilyName() == u8"Ahem");
    CHECK(created->PlatformData().Size() == 20);

    // A client added once the font has loaded is told at once.
    FontLoadCounter late;
    font->AddClient(late);
    CHECK(late.Loaded == 1);

    // Another request for the file uses the cached font.
    CHECK(RequestFont(*fixture.Document, u8"file:///fonts/ahem.woff2").get() == font.get());

    font->RemoveClient(client);
    font->RemoveClient(late);
  }

  TEST_CASE("CachedFont: a file that isn't a font is a decode error", "[Loader][CachedFont]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///fonts/broken.ttf", u8"not a font", u8"font/ttf");
    PageInFrameFixture fixture(u8"<body></body>");

    auto font = RequestFont(*fixture.Document, u8"file:///fonts/broken.ttf");
    font->BeginLoadIfNeeded(fixture.Document->GetCachedResourceLoader());
    RunNetworkingTasks();

    CHECK_FALSE(font->ErrorOccurred());
    CHECK_FALSE(font->EnsureCustomFontData());
    CHECK(font->GetStatus() == CachedResource::DecodeError);
    CHECK(font->ErrorOccurred());
  }
}
