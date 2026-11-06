#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Font/FontFaceSet.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDatabase.hpp"
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

  TEST_CASE("CSSFontFace: variable fonts", "[CSS][CSSFontFace][css-fonts:font-variation-settings-def]")
  {
    LoaderFixture loader;
    loader.Strategy.AddFile(u8"file:///fonts/variable.ttf", FontPath("webfonts", "VariableTest.ttf"),
                            u8"font/ttf");

    constexpr utf8_stringview ranged =
      u8"font-weight:100 900;font-width:50% 200%;font-style:oblique -90deg 90deg";
    struct Case
    {
      utf8_stringview Descriptors;
      utf8_stringview Style;
      // The width of `a` at 100px: 100 is the default instance, and each axis adds to it.
      int32 Width;
      bool SyntheticBold;
    };
    auto testCase = GENERATE_COPY(values<Case>({
      {u8"", u8"", 140, false},
      {u8"", u8"font-weight:700", 170, false},
      {ranged, u8"font-weight:700", 170, false},
      {ranged, u8"font-weight:900", 190, false},
      {ranged, u8"font-weight:250", 115, false},
      {ranged, u8"font-weight:950", 190, false},
      {u8"font-weight:300 500", u8"font-weight:700", 150, true},
      {ranged, u8"font-width:200%", 240, false},
      {ranged, u8"font-width:50%", 115, false},
      {ranged, u8"font-width:300%", 240, false},
      {u8"", u8"font-width:200%", 240, false},
      {ranged, u8"font-style:italic", 170, false},
      {u8"", u8"font-style:italic", 170, false},
      {u8"font-style:italic", u8"font-style:italic", 170, false},
      {u8"font-style:normal", u8"font-style:italic", 170, false},
      {ranged, u8"font-style:oblique 10deg", 150, false},
      {ranged, u8"font-style:oblique 20deg", 160, false},
      {ranged, u8"font-style:oblique -10deg", 140, false},
      {ranged, u8"font-optical-sizing:none", 100, false},
      {ranged, u8"font-variation-settings:'TEST' 100", 210, false},
      {ranged, u8"font-variation-settings:'TEST' 50", 175, false},
      {ranged, u8"font-weight:900;font-variation-settings:'wght' 100", 90, false},
      {ranged, u8"font-variation-settings:'opsz' 14", 100, false},
      {ranged, u8"font-variation-settings:'NOPE' 5", 140, false},
      {ranged, u8"font-variation-settings:'TEST' 500", 210, false},
      {ranged, u8"font-variation-settings:'slnt' -20", 160, false},
      {ranged, u8"font-variation-settings:'ital' 1", 170, false},
    }));

    PageInFrameFixture fixture(u8"<style>@font-face{font-family:Web;src:url(file:///fonts/variable.ttf);"
                               + utf8_string(testCase.Descriptors)
                               + u8"}#t{font:100px Web;display:inline-block;" + utf8_string(testCase.Style)
                               + u8"}</style><body><span id=t>a</span></body>");
    LoadFonts(fixture);

    INFO(reinterpret_cast<const char *>(utf8_string(testCase.Descriptors).c_str())
         << " / " << reinterpret_cast<const char *>(utf8_string(testCase.Style).c_str()));
    CHECK(Width(fixture, u8"t") == testCase.Width);
    auto font = PrimaryFont(fixture, u8"t");
    CHECK(font->PlatformData().SyntheticBold() == testCase.SyntheticBold);
    CHECK_FALSE(font->PlatformData().SyntheticOblique());
  }

  TEST_CASE("CSSFontFace: a variable font's instances", "[CSS][CSSFontFace]")
  {
    LoaderFixture loader;
    loader.Strategy.AddFile(u8"file:///fonts/variable.ttf", FontPath("webfonts", "VariableTest.ttf"),
                            u8"font/ttf");

    PageInFrameFixture fixture(
      u8"<style>@font-face{font-family:Web;src:url(file:///fonts/variable.ttf)}"
      u8"span{font-family:Web;display:inline-block}</style><body>"
      u8"<span id=a style=font-size:40px>a</span><span id=b style=font-size:40px>a</span>"
      u8"<span id=c style=font-size:20px>a</span><span id=d style='font-size:8px'>a</span>"
      u8"<span id=e style=font-size:14px>a</span><span id=f style='font:14px/1 "
      u8"Web;font-optical-sizing:none'>a</span>"
      u8"</body>");
    LoadFonts(fixture);

    auto faceOf = [&](dom::DOMStringView id)
    {
      return PrimaryFont(fixture, id)->PlatformData().Face();
    };

    SECTION("optical sizing is the font size in pixels")
    {
      REQUIRE(faceOf(u8"a")->Variations().size() == 1);
      CHECK(faceOf(u8"a")->Variations()[0] == text::FontVariationValue {FontAxisTag('o', 'p', 's', 'z'), 40});
      CHECK(faceOf(u8"c")->Variations()[0] == text::FontVariationValue {FontAxisTag('o', 'p', 's', 'z'), 20});
      CHECK(faceOf(u8"d")->Variations()[0] == text::FontVariationValue {FontAxisTag('o', 'p', 's', 'z'), 8});
      CHECK(PrimaryFont(fixture, u8"c")->PlatformData().HasVariations());
    }

    SECTION("the same values share a face, and the default instance is the font's own")
    {
      CHECK(faceOf(u8"a") == faceOf(u8"b"));
      CHECK(faceOf(u8"a") != faceOf(u8"c"));
      CHECK(faceOf(u8"e")->Variations().empty());
      CHECK(faceOf(u8"e") == faceOf(u8"f"));
      CHECK_FALSE(PrimaryFont(fixture, u8"e")->PlatformData().HasVariations());
    }

    SECTION("the axes")
    {
      const auto &axes = faceOf(u8"a")->VariationAxes();
      REQUIRE(axes.size() == 6);
      CHECK(axes[0].Tag == FontAxisTag('w', 'g', 'h', 't'));
      CHECK(axes[0].Minimum == 100);
      CHECK(axes[0].Default == 400);
      CHECK(axes[0].Maximum == 900);
    }
  }

  TEST_CASE("CSSFontFace: a host's variable font covers what its axes reach", "[CSS][CSSFontFace]")
  {
    auto &database = FontDatabase::Singleton();
    (void)database.AddFontFile(FontPath("webfonts", "VariableTest.ttf"));
    PageInFrameFixture fixture(
      u8"<style>span{font:100px 'Variable Test';display:inline-block;font-optical-sizing:none}</style><body>"
      u8"<span id=a>a</span><span id=b style=font-weight:700>a</span><span id=c "
      u8"style=font-style:italic>a</span>"
      u8"<span id=d style='font-style:oblique 10deg'>a</span><span id=e "
      u8"style=font-width:200%>a</span></body>");
    (void)fixture.Document->UpdateLayout();

    CHECK(Width(fixture, u8"a") == 100);
    CHECK(Width(fixture, u8"b") == 130);
    CHECK_FALSE(PrimaryFont(fixture, u8"b")->PlatformData().SyntheticBold());
    CHECK(Width(fixture, u8"c") == 130);
    CHECK_FALSE(PrimaryFont(fixture, u8"c")->PlatformData().SyntheticOblique());
    CHECK(Width(fixture, u8"d") == 110);
    CHECK_FALSE(PrimaryFont(fixture, u8"d")->PlatformData().SyntheticOblique());
    CHECK(Width(fixture, u8"e") == 200);
  }
}
