#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  namespace
  {
    constexpr auto Red = boo::Color::red;
    constexpr auto Blue = boo::Color::blue;

    /// @brief A page showing paragraphs coloured `light-dark(red, blue)`, one for each `color-scheme`.
    struct ColorSchemeFixture : LoadFixture
    {
      ColorSchemeFixture() noexcept
      {
        LoadMarkup(u8R"(<!DOCTYPE html><head id=head></head><style>
          p { color: light-dark(rgb(255, 0, 0), rgb(0, 0, 255)) }
          #both { color-scheme: light dark }
          #dark { color-scheme: dark }
          #light { color-scheme: light }
        </style><p id=normal>x</p><p id=both>x</p><p id=dark>x</p><p id=light>x</p>)",
                   u8"https://example.com/page.html");
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) const
      {
        auto element = Document().GetElementById(id);
        REQUIRE(element.get());
        return *element;
      }

      /// @brief The colour of the paragraph @p id as 8-bit sRGB, after the document's style is updated.
      KRYS_NODISCARD SRGBA<uint8> ColorOf(dom::DOMStringView id) const
      {
        Document().UpdateStyleIfNeeded();
        auto *style = ById(id).GetRenderStyle();
        REQUIRE(style);
        return style->GetColor().ToColorTypeLossy<SRGBA<uint8>>();
      }

      void SetHostDark(bool dark) noexcept
      {
        css::mq::MediaEnvironment environment = Frame().GetMediaEnvironment();
        environment.UseDarkAppearance = dark;
        Frame().SetMediaEnvironment(environment);
      }

      KRYS_NODISCARD Ref<dom::Element> AddMeta(dom::DOMStringView name, dom::DOMStringView content) const
      {
        auto meta = Document().CreateElement(u8"meta").ReleaseValue();
        REQUIRE_FALSE(meta->SetAttribute(u8"name", dom::DOMString(name)).HasException());
        REQUIRE_FALSE(meta->SetAttribute(u8"content", dom::DOMString(content)).HasException());
        REQUIRE_FALSE(ById(u8"head").AppendChild(*meta).HasException());
        return meta;
      }
    };
  }

  TEST_CASE("The dark appearance follows the style's colour scheme and the host's",
            "[DOM][Document][ColorScheme]")
  {
    ColorSchemeFixture fixture;
    auto &document = fixture.Document();
    REQUIRE(document.GetPage());

    CHECK(fixture.ColorOf(u8"normal") == Red);
    CHECK(fixture.ColorOf(u8"both") == Red);
    CHECK(fixture.ColorOf(u8"dark") == Blue);
    CHECK(fixture.ColorOf(u8"light") == Red);
    CHECK_FALSE(document.UseDarkAppearance(nullptr));
    CHECK(document.StyleColorOptions(nullptr) == style::ColorOptions::None);

    fixture.SetHostDark(true);
    CHECK(document.GetPage()->UseDarkAppearance());
    CHECK(fixture.ColorOf(u8"normal") == Red);
    CHECK(fixture.ColorOf(u8"both") == Blue);
    CHECK(fixture.ColorOf(u8"dark") == Blue);
    CHECK(fixture.ColorOf(u8"light") == Red);
    CHECK(document.StyleColorOptions(fixture.ById(u8"both").GetRenderStyle())
          == style::ColorOptions::UseDarkAppearance);

    fixture.SetHostDark(false);
    CHECK(fixture.ColorOf(u8"both") == Red);
  }

  TEST_CASE("<meta name=color-scheme> sets the document's colour scheme", "[DOM][Document][ColorScheme]")
  {
    ColorSchemeFixture fixture;
    auto &document = fixture.Document();
    using enum boo::ColorScheme;

    SECTION("light dark follows the host")
    {
      auto meta = fixture.AddMeta(u8"color-scheme", u8"light dark");
      CHECK(document.ResolvedColorScheme(nullptr) == EnumSet<boo::ColorScheme> {Light, Dark});
      CHECK(fixture.ColorOf(u8"normal") == Red);
      fixture.SetHostDark(true);
      CHECK(fixture.ColorOf(u8"normal") == Blue);
      // A style's own colour scheme wins.
      CHECK(fixture.ColorOf(u8"light") == Red);
    }

    SECTION("dark alone is dark on a light host")
    {
      auto meta = fixture.AddMeta(u8"COLOR-SCHEME", u8"dark");
      CHECK(fixture.ColorOf(u8"normal") == Blue);

      // Renaming the element or removing it drops its colour scheme.
      REQUIRE_FALSE(meta->SetAttribute(u8"name", u8"description").HasException());
      CHECK(document.ResolvedColorScheme(nullptr).IsEmpty());
      CHECK(fixture.ColorOf(u8"normal") == Red);
      REQUIRE_FALSE(meta->SetAttribute(u8"name", u8"supported-color-schemes").HasException());
      CHECK(fixture.ColorOf(u8"normal") == Blue);
      REQUIRE_FALSE(fixture.ById(u8"head").RemoveChild(*meta).HasException());
      CHECK(fixture.ColorOf(u8"normal") == Red);
    }

    SECTION("the first valid one counts")
    {
      auto invalid = fixture.AddMeta(u8"color-scheme", u8"light, dark");
      auto valid = fixture.AddMeta(u8"color-scheme", u8"dark");
      auto later = fixture.AddMeta(u8"color-scheme", u8"light");
      CHECK(document.ResolvedColorScheme(nullptr) == EnumSet<boo::ColorScheme> {Dark});
      REQUIRE_FALSE(valid->SetAttribute(u8"content", u8"only").HasException());
      // An element whose content doesn't parse is skipped.
      CHECK(document.ResolvedColorScheme(nullptr) == EnumSet<boo::ColorScheme> {Light});
    }
  }
}
