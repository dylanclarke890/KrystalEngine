#include "Krystal.Booey/Rendering/CSSFilterRenderer.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Graphics/Filters/FEColorMatrix.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/Filters/SourceGraphic.hpp"
#include "Krystal.Booey/Graphics/NullGraphicsContext.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using Catch::Approx;

    /// @brief The style @p filterStyle draws with, which must be a `T`.
    template <typename T>
    KRYS_NODISCARD const T &StyleOf(const FilterStyle &filterStyle) noexcept
    {
      krys_assert(filterStyle.Style && HoldsAlternative<T>(*filterStyle.Style));
      return std::get<T>(*filterStyle.Style);
    }

    /// @brief A document with a 40x30 box `#box` styled with @p filter, laid out.
    struct FilterFixture : PaintFixture
    {
      explicit FilterFixture(utf8_stringview filter) noexcept
          : PaintFixture(
              u8"<!DOCTYPE html><style>body{margin:0}#box{width:40px;height:30px;background:red;filter:"
                + utf8_string(filter) + u8"}</style><div id=box></div>",
              {200, 100}, true)
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD RenderElement &Box() noexcept
      {
        auto *renderer = ById(u8"box").Renderer();
        krys_assert(renderer);
        return *renderer;
      }

      KRYS_NODISCARD RenderLayer &BoxLayer() noexcept
      {
        auto *layer = Downcast<RenderLayerModelObject>(Box()).Layer();
        krys_assert(layer);
        return *layer;
      }

      /// @brief The filter of the box, built for drawing through a graphics context.
      KRYS_NODISCARD RefPtr<CSSFilterRenderer> Renderer() noexcept
      {
        NullGraphicsContext context;
        auto geometry = FilterGeometry {
          .ReferenceBox = {0, 0, 40, 30},
          .FilterRegion = {-20, -20, 80, 70},
          .Scale = {1, 1},
        };
        return CSSFilterRenderer::Create(Box(), Box().Style().GetFilter(), geometry,
                                         {FilterRenderingMode::GraphicsContext}, context);
      }

      /// @brief The styles the box's filter draws with, outermost first.
      KRYS_NODISCARD FilterStyleVector Styles() noexcept
      {
        auto renderer = Renderer();
        REQUIRE(renderer);
        NullGraphicsContext context;
        return static_cast<const Filter &>(*renderer).CreateFilterStyles(context, FloatRect {0, 0, 40, 30});
      }
    };
  }

  TEST_CASE("CSSFilterRenderer: builds an effect per function, after the source graphic",
            "[Rendering][CSSFilterRenderer]")
  {
    FilterFixture fixture(u8"grayscale(1) blur(2px) drop-shadow(1px 2px 3px red)");
    auto renderer = fixture.Renderer();
    REQUIRE(renderer);
    const auto &functions = renderer->Functions();
    REQUIRE(functions.size() == 4u);
    CHECK(Is<SourceGraphic>(*functions[0]));
    REQUIRE(Is<FEColorMatrix>(*functions[1]));
    CHECK(Downcast<FEColorMatrix>(*functions[1]).GetType() == ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX);
    REQUIRE(Is<FEGaussianBlur>(*functions[2]));
    CHECK(Downcast<FEGaussianBlur>(*functions[2]).StdDeviationX() == 2.f);
    REQUIRE(Is<FEDropShadow>(*functions[3]));
    const auto &shadow = Downcast<FEDropShadow>(*functions[3]);
    CHECK(shadow.Dx() == 1.f);
    CHECK(shadow.Dy() == 2.f);
    CHECK(shadow.StdDeviationX() == 3.f);
    CHECK(renderer->HasFilterThatMovesPixels());
    CHECK(renderer->FilterRenderingModes()
          == EnumSet<FilterRenderingMode> {FilterRenderingMode::GraphicsContext});
  }

  TEST_CASE("CSSFilterRenderer: draws with a style per function, the last outermost",
            "[Rendering][CSSFilterRenderer]")
  {
    SECTION("a chain")
    {
      FilterFixture fixture(u8"saturate(0) blur(3px) drop-shadow(4px 5px 2px rgb(0 0 255))");
      auto styles = fixture.Styles();
      REQUIRE(styles.size() == 3u);

      REQUIRE((styles[0].Style && HoldsAlternative<GraphicsDropShadow>(*styles[0].Style)));
      const auto &shadow = StyleOf<GraphicsDropShadow>(styles[0]);
      CHECK(shadow.Offset == FloatSize {4, 5});
      // Shadows' blur radii are twice the standard deviation.
      CHECK(shadow.Radius == 4.f);
      CHECK(shadow.Color == Color {SRGBA<uint8> {0, 0, 255}});

      REQUIRE((styles[1].Style && HoldsAlternative<GraphicsGaussianBlur>(*styles[1].Style)));
      CHECK(StyleOf<GraphicsGaussianBlur>(styles[1]).Radius == FloatSize {6, 6});

      REQUIRE((styles[2].Style && HoldsAlternative<GraphicsColorMatrix>(*styles[2].Style)));
      const auto &values = StyleOf<GraphicsColorMatrix>(styles[2]).Values;
      // saturate(0) is the luminance in every channel.
      CHECK(values[0] == Approx(0.213f));
      CHECK(values[1] == Approx(0.715f));
      CHECK(values[2] == Approx(0.072f));
      CHECK(values[18] == 1.f);
    }

    SECTION("component transfers and opacity are colour matrices")
    {
      FilterFixture fixture(u8"brightness(2) contrast(0.5) invert(1) opacity(0.25)");
      auto styles = fixture.Styles();
      REQUIRE(styles.size() == 4u);
      for (const auto &style : styles)
      {
        CHECK((style.Style && HoldsAlternative<GraphicsColorMatrix>(*style.Style)));
      }
      // opacity(0.25), outermost, scales alpha.
      CHECK(StyleOf<GraphicsColorMatrix>(styles[0]).Values[18] == Approx(0.25f));
      // invert(1): 1 - c.
      CHECK(StyleOf<GraphicsColorMatrix>(styles[1]).Values[0] == Approx(-1.f));
      CHECK(StyleOf<GraphicsColorMatrix>(styles[1]).Values[4] == Approx(1.f));
      // contrast(0.5): 0.5c + 0.25.
      CHECK(StyleOf<GraphicsColorMatrix>(styles[2]).Values[0] == Approx(0.5f));
      CHECK(StyleOf<GraphicsColorMatrix>(styles[2]).Values[4] == Approx(0.25f));
      // brightness(2): 2c.
      CHECK(StyleOf<GraphicsColorMatrix>(styles[3]).Values[0] == Approx(2.f));
    }

    SECTION("identity functions in a chain still build their effects")
    {
      FilterFixture fixture(u8"blur(0) saturate(1) grayscale(1)");
      auto styles = fixture.Styles();
      REQUIRE(styles.size() == 3u);
      CHECK(StyleOf<GraphicsGaussianBlur>(styles[2]).Radius == FloatSize {});
    }
  }

  TEST_CASE("CSSFilterRenderer: blurs and shadows reach past the box", "[Rendering][CSSFilterRenderer]")
  {
    SECTION("blur")
    {
      FilterFixture fixture(u8"blur(4px)");
      auto outsets = CSSFilterRenderer::CalculateOutsets(fixture.Box(), fixture.Box().Style().GetFilter(),
                                                         FloatRect {0, 0, 40, 30});
      // Three standard deviations.
      CHECK(outsets == IntOutsets {12, 12, 12, 12});
      CHECK(fixture.BoxLayer().HasFilterOutsets());
      CHECK(fixture.BoxLayer().FilterMovesPixels());
    }

    SECTION("drop shadow")
    {
      FilterFixture fixture(u8"drop-shadow(4px -6px 0 black)");
      auto outsets = CSSFilterRenderer::CalculateOutsets(fixture.Box(), fixture.Box().Style().GetFilter(),
                                                         FloatRect {0, 0, 40, 30});
      // Top, right, bottom, left: the offset moves the shadow up and right.
      CHECK(outsets == IntOutsets {6, 4, 0, 0});
      CHECK(fixture.BoxLayer().FilterMovesPixels());
    }

    SECTION("colour matrices don't")
    {
      FilterFixture fixture(u8"sepia(1) hue-rotate(30deg)");
      CHECK_FALSE(fixture.BoxLayer().HasFilterOutsets());
      CHECK_FALSE(fixture.BoxLayer().FilterMovesPixels());
      CHECK(fixture.BoxLayer().ShouldPaintWithFilters());
    }

    SECTION("an identity filter doesn't paint")
    {
      FilterFixture fixture(u8"blur(0)");
      CHECK_FALSE(fixture.BoxLayer().ShouldPaintWithFilters());
    }
  }

  TEST_CASE("RenderLayer: paints its contents through its filter", "[Rendering][RenderLayer][Filters]")
  {
    // A transparency layer per function, the last outermost, each clipped to what it paints (the box grown by
    // the blur's reach) and composited with its function's style.
    FilterFixture fixture(u8"blur(2px) sepia(1)");
    CHECK(fixture.Painted()
          == Canvas()
               + u8R"(
(save)
(begin-transparency-layer
  (opacity 1.00))
(save)
(clip
  (rect at (-6,-6) size 52x42))
(set-state
  (change-flags [style])
  (style)"
                 u8" "
                 u8R"(
    (values [0.39, 0.77, 0.19, 0.00, 0.00, 0.35, 0.69, 0.17, 0.00, 0.00, 0.27, 0.53, 0.13, 0.00, 0.00, 0.00, 0.00, 0.00, 1.00, 0.00])))
(begin-transparency-layer
  (opacity 1.00))
(save)
(clip
  (rect at (-6,-6) size 52x42))
(set-state
  (change-flags [style])
  (style)"
                 u8" "
                 u8R"(
    (radius width=4 height=4)))
(begin-transparency-layer
  (opacity 1.00))
(fill-composited-rect
  (rect at (0,0) size 40x30)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal))
(end-transparency-layer)
(restore)
(end-transparency-layer)
(restore)
(end-transparency-layer)
(restore))");
  }

  TEST_CASE("RenderLayer: an identity filter paints as no filter", "[Rendering][RenderLayer][Filters]")
  {
    FilterFixture fixture(u8"blur(0) saturate(1)");
    CHECK(fixture.Painted() == Canvas() + u8R"(
(fill-composited-rect
  (rect at (0,0) size 40x30)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal)))");
  }
}
