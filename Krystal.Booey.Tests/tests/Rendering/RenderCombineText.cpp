#include "Krystal.Booey/Rendering/RenderCombineText.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief The text renderer of the element with @p id, after layout.
    KRYS_NODISCARD RenderText &TextOf(ResolverFixture &fixture, dom::DOMStringView id)
    {
      (void)fixture.Document->UpdateLayout();
      auto *text = DynamicDowncast<RenderText>(fixture.ById(id).FirstChild()->Renderer());
      REQUIRE(text);
      return *text;
    }

    /// @brief The logical width of the element with @p id's box, in a vertical writing mode.
    KRYS_NODISCARD int LogicalWidthOf(ResolverFixture &fixture, dom::DOMStringView id)
    {
      auto *inlineBox = DynamicDowncast<RenderInline>(fixture.ById(id).Renderer());
      REQUIRE(inlineBox);
      return inlineBox->LinesBoundingBox().Height();
    }

    /// @brief Whether @p width is 20px (one em), give or take the enclosing rect's rounding.
    KRYS_NODISCARD bool IsAboutAnEm(int width)
    {
      return width == 20 || width == 21;
    }
  }

  TEST_CASE("RenderCombineText: text-combine-upright combines text into an em",
            "[Rendering][RenderCombineText]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { writing-mode: vertical-rl; font: 20px sans-serif }
      .tcy { text-combine-upright: all }
    </style><p>a<span id=two class=tcy>12</span><span id=four class=tcy>2026</span>
    <span id=plain>12</span></p><p style="writing-mode: horizontal-tb"><span id=horizontal class=tcy>12</span></p>)html"};

    SECTION("Its text is replaced by one character, an em wide")
    {
      auto *combined = DynamicDowncast<RenderCombineText>(TextOf(fixture, u8"two"));
      REQUIRE(combined);
      CHECK(combined->IsCombined());
      CHECK(combined->Text() == u8"￼");
      CHECK(combined->OriginalText() == u8"12");
      CHECK(combined->HorizontalScale() == 1.f);
      CHECK(IsAboutAnEm(LogicalWidthOf(fixture, u8"two")));
    }

    SECTION("Text too wide for an em is compressed to one")
    {
      auto *combined = DynamicDowncast<RenderCombineText>(TextOf(fixture, u8"four"));
      REQUIRE(combined);
      CHECK(combined->IsCombined());
      CHECK(combined->HorizontalScale() < 1.f);
      CHECK(combined->HorizontalScale() > 0.3f);
      CHECK(IsAboutAnEm(LogicalWidthOf(fixture, u8"four")));
    }

    SECTION("Without the property the text is set sideways")
    {
      auto &text = TextOf(fixture, u8"plain");
      CHECK_FALSE(Is<RenderCombineText>(text));
      CHECK(LogicalWidthOf(fixture, u8"plain") > 20);
    }

    SECTION("In horizontal writing it isn't combined")
    {
      auto *combined = DynamicDowncast<RenderCombineText>(TextOf(fixture, u8"horizontal"));
      REQUIRE(combined);
      CHECK_FALSE(combined->IsCombined());
      CHECK(combined->Text() == u8"12");
    }

    SECTION("Changing the property swaps the renderer")
    {
      REQUIRE(Is<RenderCombineText>(TextOf(fixture, u8"two")));
      REQUIRE_FALSE(fixture.ById(u8"two").SetAttribute(u8"class", u8"").HasException());
      CHECK_FALSE(Is<RenderCombineText>(TextOf(fixture, u8"two")));
      REQUIRE_FALSE(fixture.ById(u8"plain").SetAttribute(u8"class", u8"tcy").HasException());
      CHECK(Is<RenderCombineText>(TextOf(fixture, u8"plain")));
    }

    SECTION("Changing the text combines it again")
    {
      auto *combined = DynamicDowncast<RenderCombineText>(TextOf(fixture, u8"two"));
      REQUIRE(combined);
      REQUIRE_FALSE(fixture.ById(u8"two").TextContent(u8"123456").HasException());
      combined = DynamicDowncast<RenderCombineText>(TextOf(fixture, u8"two"));
      REQUIRE(combined);
      CHECK(combined->IsCombined());
      CHECK(combined->OriginalText() == u8"123456");
      CHECK(combined->HorizontalScale() < 1.f);
      CHECK(IsAboutAnEm(LogicalWidthOf(fixture, u8"two")));
    }
  }
}
