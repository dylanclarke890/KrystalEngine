#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Font/FontFaceSet.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include <filesystem>

namespace krys::boo::css::tests
{
  namespace
  {
    using namespace std::chrono_literals;
    using boo::tests::LoaderFixture;
    using boo::tests::PageInFrameFixture;
    using boo::tests::RunNetworkingTasks;

    KRYS_NODISCARD std::filesystem::path FontPath(const char *directory, const char *name) noexcept
    {
      return std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / directory / name;
    }

    /// @brief Serves the test fonts under `file:///fonts/`.
    void AddFonts(LoaderFixture &loader) noexcept
    {
      loader.Strategy.AddFile(u8"file:///fonts/ahem.ttf", FontPath("fonts", "Ahem.ttf"), u8"font/ttf");
      loader.Strategy.AddFile(u8"file:///fonts/ahem.otf", FontPath("webfonts", "AhemCFF.otf"), u8"font/otf");
      loader.Strategy.AddFile(u8"file:///fonts/ahem.woff", FontPath("webfonts", "Ahem.woff"), u8"font/woff");
      loader.Strategy.AddFile(u8"file:///fonts/ahem.woff2", FontPath("webfonts", "Ahem.woff2"),
                              u8"font/woff2");
      loader.Strategy.AddText(u8"file:///fonts/broken.woff2", u8"wOF2 is not a font", u8"font/woff2");
    }

    /// @brief Runs the event loop @p delta of host time after the last run.
    void Advance(MonotonicTime::duration delta) noexcept
    {
      auto &loop = html::EventLoop::Current();
      loop.Run(loop.Now() + delta);
    }

    KRYS_NODISCARD Ref<const Font> PrimaryFont(PageInFrameFixture &fixture, dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      auto *style = fixture.ById(id).GetRenderStyle();
      REQUIRE(style);
      return style->GetFontCascade().PrimaryFont();
    }

    /// @brief The font @p id's `X`s are drawn with.
    KRYS_NODISCARD const Font &GlyphFont(PageInFrameFixture &fixture, dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      auto *style = fixture.ById(id).GetRenderStyle();
      REQUIRE(style);
      auto glyphData = style->GetFontCascade().GlyphDataForCharacter(U'X', false);
      REQUIRE(glyphData.Font);
      return *glyphData.Font;
    }

    KRYS_NODISCARD utf8_string PrimaryFamily(PageInFrameFixture &fixture, dom::DOMStringView id) noexcept
    {
      return PrimaryFont(fixture, id)->PlatformData().FamilyName();
    }

    /// @brief The laid out width of @p id: Ahem's glyphs are 1em wide, Liberation Sans' `X` 1366/2048em.
    KRYS_NODISCARD int32 Width(PageInFrameFixture &fixture, dom::DOMStringView id) noexcept
    {
      return fixture.RendererSize(id).Width();
    }

    KRYS_NODISCARD utf8_string Page(utf8_stringview fontFaces, utf8_stringview text = u8"XXXX") noexcept
    {
      return u8"<style>" + utf8_string(fontFaces)
             + u8" #t { font: 10px Web, 'Liberation Sans'; display: inline-block }</style><body><span id=t>"
             + utf8_string(text) + u8"</span></body>";
    }

    /// @brief Loads the page's fonts: lays out (which starts them loading), lets the font loader start the
    /// loads, and completes them.
    void LoadFonts(PageInFrameFixture &fixture) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      RunNetworkingTasks();
      (void)fixture.Document->UpdateLayout();
      RunNetworkingTasks();
    }
  }

  TEST_CASE("CSSFontFace: a web font loads once text uses it, and the page lays out again with it",
            "[CSS][CSSFontFace]")
  {
    LoaderFixture loader;
    AddFonts(loader);
    loader.Strategy.Defer = true;

    PageInFrameFixture fixture(
      Page(u8"@font-face { font-family: Web; src: url(file:///fonts/ahem.woff2) format('woff2') }"
           u8"@font-face { font-family: Unused; src: url(file:///fonts/ahem.ttf) }"));

    // Requesting the font doesn't load it: laying the text out does, soon after.
    (void)fixture.Document->UpdateLayout();
    CHECK(loader.Strategy.Requests.empty());
    RunNetworkingTasks();
    REQUIRE(loader.Strategy.Requests == List<utf8_string> {u8"file:///fonts/ahem.woff2"});

    // While it loads (in the block period), the text is laid out in the fallback, invisibly; the metrics are
    // the next family's.
    auto &loading = GlyphFont(fixture, u8"t");
    CHECK(loading.IsInterstitial());
    CHECK(loading.GetVisibility() == Font::Visibility::Invisible);
    CHECK(PrimaryFamily(fixture, u8"t") == u8"Liberation Sans");
    CHECK(Width(fixture, u8"t") == 26);
    auto &faces = fixture.Document->GetFontSelector().GetCSSFontFaceSet();
    REQUIRE(faces.FaceCount() == 2);
    CHECK(faces[0].GetStatus() == CSSFontFace::Status::Loading);
    CHECK(faces[1].GetStatus() == CSSFontFace::Status::Pending);
    CHECK(faces.HasActiveFontFaces());

    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();

    CHECK(faces[0].GetStatus() == CSSFontFace::Status::Success);
    CHECK_FALSE(faces.HasActiveFontFaces());
    CHECK(PrimaryFamily(fixture, u8"t") == u8"Ahem");
    CHECK_FALSE(GlyphFont(fixture, u8"t").IsInterstitial());
    CHECK(Width(fixture, u8"t") == 40);
  }

  TEST_CASE("CSSFontFace: TrueType, OpenType, WOFF and WOFF 2 files load", "[CSS][CSSFontFace]")
  {
    LoaderFixture loader;
    AddFonts(loader);

    auto file = GENERATE(as<utf8_string> {}, u8"ahem.ttf", u8"ahem.otf", u8"ahem.woff", u8"ahem.woff2");
    INFO(ToNarrow(file));

    PageInFrameFixture fixture(
      Page(u8"@font-face { font-family: Web; src: url(file:///fonts/" + file + u8") }"));
    LoadFonts(fixture);

    CHECK(fixture.Document->GetFontSelector().GetCSSFontFaceSet()[0].GetStatus()
          == CSSFontFace::Status::Success);
    CHECK(Width(fixture, u8"t") == 40);
  }

  TEST_CASE("CSSFontFace: sources are tried in order", "[CSS][CSSFontFace]")
  {
    LoaderFixture loader;
    AddFonts(loader);

    SECTION("unsupported formats and technologies are skipped without loading")
    {
      PageInFrameFixture fixture(
        Page(u8"@font-face { font-family: Web; src: url(file:///fonts/a.eot) "
             u8"format('embedded-opentype'), url(file:///fonts/b.svg) format('svg'), "
             u8"url(file:///fonts/c.ttf) tech(incremental), url(file:///fonts/d.eot), "
             u8"url(file:///fonts/ahem.woff) format('woff') }"));
      LoadFonts(fixture);

      CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///fonts/ahem.woff"});
      CHECK(Width(fixture, u8"t") == 40);
    }

    SECTION("a missing file or one that isn't a font falls through to the next source")
    {
      PageInFrameFixture fixture(Page(u8"@font-face { font-family: Web; src: url(file:///fonts/missing.ttf), "
                                      u8"url(file:///fonts/broken.woff2), url(file:///fonts/ahem.ttf) }"));
      LoadFonts(fixture);
      LoadFonts(fixture);

      CHECK(loader.Strategy.Requests
            == List<utf8_string> {u8"file:///fonts/missing.ttf", u8"file:///fonts/broken.woff2",
                                  u8"file:///fonts/ahem.ttf"});
      CHECK(Width(fixture, u8"t") == 40);
    }

    SECTION("when every source fails, the next family is used")
    {
      PageInFrameFixture fixture(
        Page(u8"@font-face { font-family: Web; src: url(file:///fonts/missing.ttf) }"));
      LoadFonts(fixture);

      CHECK(fixture.Document->GetFontSelector().GetCSSFontFaceSet()[0].GetStatus()
            == CSSFontFace::Status::Failure);
      CHECK(PrimaryFamily(fixture, u8"t") == u8"Liberation Sans");
      CHECK_FALSE(PrimaryFont(fixture, u8"t")->IsInterstitial());
      CHECK(Width(fixture, u8"t") == 26);
    }

    SECTION("local() sources name registered families, and load at once")
    {
      PageInFrameFixture fixture(
        Page(u8"@font-face { font-family: Web; src: local('No Such Font'), local('Liberation Sans') }"));
      (void)fixture.Document->UpdateLayout();

      CHECK(loader.Strategy.Requests.empty());
      CHECK(PrimaryFamily(fixture, u8"t") == u8"Liberation Sans");
      CHECK_FALSE(PrimaryFont(fixture, u8"t")->IsInterstitial());
    }
  }

  TEST_CASE("CSSFontFace: faces are matched by weight, style and unicode-range", "[CSS][CSSFontFace]")
  {
    LoaderFixture loader;
    AddFonts(loader);

    SECTION("weight and style")
    {
      PageInFrameFixture fixture(
        u8"<style>"
        u8"@font-face { font-family: Web; src: local('Liberation Sans') }"
        u8"@font-face { font-family: Web; font-weight: 600 900; src: url(file:///fonts/ahem.ttf) }"
        u8"@font-face { font-family: Web; font-style: italic; src: url(file:///fonts/ahem.woff) }"
        u8"span { font: 10px Web; display: inline-block }"
        u8"</style><body><span id=normal>XXXX</span><span id=bold style='font-weight: bold'>XXXX</span>"
        u8"<span id=italic style='font-style: italic'>XXXX</span></body>");
      LoadFonts(fixture);

      CHECK(Width(fixture, u8"normal") == 26);
      CHECK(Width(fixture, u8"bold") == 40);
      CHECK(Width(fixture, u8"italic") == 40);
      // Only the faces the text needed loaded.
      CHECK(loader.Strategy.Requests.size() == 2);
    }

    SECTION("unicode-range")
    {
      PageInFrameFixture fixture(Page(u8"@font-face { font-family: Web; src: url(file:///fonts/ahem.ttf); "
                                      u8"unicode-range: U+58 }",
                                      u8"XXyy"));
      LoadFonts(fixture);

      // The X's are Ahem's, the y's the fallback's (Liberation Sans' is 1024/2048em).
      CHECK(Width(fixture, u8"t") == 30);
    }

    SECTION("a family without the space loads its first face for the primary font's metrics only")
    {
      PageInFrameFixture fixture(Page(u8"@font-face { font-family: Web; src: url(file:///fonts/ahem.ttf); "
                                      u8"unicode-range: U+4E00-9FFF }"));
      LoadFonts(fixture);

      CHECK(loader.Strategy.Requests.size() == 1);
      CHECK(PrimaryFamily(fixture, u8"t") == u8"Ahem");
      // The X's are the fallback's.
      CHECK(Width(fixture, u8"t") == 26);
    }
  }

  TEST_CASE("CSSFontFace: font-display periods", "[CSS][CSSFontFace]")
  {
    LoaderFixture loader;
    AddFonts(loader);
    loader.Strategy.Defer = true;

    auto startLoading = [&](utf8_stringview display)
    {
      auto fixture = CreateUnique<PageInFrameFixture>(
        Page(u8"@font-face { font-family: Web; src: url(file:///fonts/ahem.ttf); font-display: "
             + utf8_string(display) + u8" }"));
      LoadFonts(*fixture);
      REQUIRE(loader.Strategy.Requests.size() == 1);
      return fixture;
    };

    SECTION("block: invisible for 3s, then the fallback, then the font whenever it loads")
    {
      auto fixture = startLoading(u8"block");
      CHECK(GlyphFont(*fixture, u8"t").GetVisibility() == Font::Visibility::Invisible);

      Advance(2'999ms);
      CHECK(GlyphFont(*fixture, u8"t").GetVisibility() == Font::Visibility::Invisible);
      Advance(1ms);
      auto &face = fixture->Document->GetFontSelector().GetCSSFontFaceSet()[0];
      CHECK(face.GetStatus() == CSSFontFace::Status::TimedOut);
      CHECK(GlyphFont(*fixture, u8"t").GetVisibility() == Font::Visibility::Visible);

      Advance(1h);
      loader.Strategy.CompleteDeferred();
      RunNetworkingTasks();
      CHECK(face.GetStatus() == CSSFontFace::Status::Success);
      CHECK(Width(*fixture, u8"t") == 40);
    }

    SECTION("swap: the fallback at once")
    {
      auto fixture = startLoading(u8"swap");
      CHECK(fixture->Document->GetFontSelector().GetCSSFontFaceSet()[0].GetStatus()
            == CSSFontFace::Status::TimedOut);
      CHECK(GlyphFont(*fixture, u8"t").GetVisibility() == Font::Visibility::Visible);

      loader.Strategy.CompleteDeferred();
      RunNetworkingTasks();
      CHECK(Width(*fixture, u8"t") == 40);
    }

    SECTION("fallback: a font that takes more than 3.1s isn't used")
    {
      auto fixture = startLoading(u8"fallback");
      Advance(100ms);
      auto &face = fixture->Document->GetFontSelector().GetCSSFontFaceSet()[0];
      CHECK(face.GetStatus() == CSSFontFace::Status::TimedOut);
      Advance(3s);
      CHECK(face.GetStatus() == CSSFontFace::Status::Failure);

      loader.Strategy.CompleteDeferred();
      RunNetworkingTasks();
      CHECK(face.GetStatus() == CSSFontFace::Status::Failure);
      CHECK(Width(*fixture, u8"t") == 26);
    }

    SECTION("optional: no swap period")
    {
      auto fixture = startLoading(u8"optional");
      Advance(100ms);
      CHECK(fixture->Document->GetFontSelector().GetCSSFontFaceSet()[0].GetStatus()
            == CSSFontFace::Status::Failure);
    }
  }

  TEST_CASE("CSSFontFace: faces follow their rules", "[CSS][CSSFontFace]")
  {
    LoaderFixture loader;
    AddFonts(loader);

    PageInFrameFixture fixture(
      Page(u8"")
      + u8"<style id=faces>@font-face { font-family: Web; src: url(file:///fonts/ahem.ttf) }</style>");
    LoadFonts(fixture);
    auto &selector = fixture.Document->GetFontSelector();
    CHECK(selector.GetCSSFontFaceSet().FaceCount() == 1);
    CHECK_FALSE(selector.IsSimpleFontSelectorForDescription());
    CHECK(Width(fixture, u8"t") == 40);

    SECTION("a style change keeps the face")
    {
      auto *face = &selector.GetCSSFontFaceSet()[0];
      REQUIRE_FALSE(fixture.ById(u8"t").SetAttribute(u8"style", u8"color: red").HasException());
      (void)fixture.Document->UpdateLayout();
      REQUIRE(selector.GetCSSFontFaceSet().FaceCount() == 1);
      CHECK(&selector.GetCSSFontFaceSet()[0] == face);
    }

    SECTION("removing the rule's sheet removes the face")
    {
      REQUIRE_FALSE(fixture.ById(u8"faces").Remove().HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(selector.GetCSSFontFaceSet().FaceCount() == 0);
      CHECK(Width(fixture, u8"t") == 26);
    }
  }
}
