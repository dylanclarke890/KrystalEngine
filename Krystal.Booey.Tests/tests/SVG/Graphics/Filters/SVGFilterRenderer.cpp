#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterRenderer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/Filters/FEOffset.hpp"
#include "Krystal.Booey/Graphics/NullGraphicsContext.hpp"
#include "Krystal.Booey/SVG/SVGFilterElement.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief A document with an SVG holding @p filters and a rect `#target` to filter, laid out.
    struct SVGFilterFixture : ResolverFixture
    {
      explicit SVGFilterFixture(utf8_stringview filters) noexcept
          : ResolverFixture(u8"<!DOCTYPE html><svg width=200 height=100>" + utf8_string(filters)
                            + u8"<rect id=target x=10 y=10 width=40 height=20 /></svg>")
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD svg::SVGFilterElement &FilterById(dom::DOMStringView id) noexcept
      {
        return Downcast<svg::SVGFilterElement>(Downcast<svg::SVGElement>(ById(id)));
      }

      /// @brief The renderer of the filter `#id`, applied to the target, or null when the filter is invalid.
      KRYS_NODISCARD RefPtr<SVGFilterRenderer> Create(dom::DOMStringView id) noexcept
      {
        NullGraphicsContext context;
        auto &target = Downcast<svg::SVGElement>(ById(u8"target"));
        return SVGFilterRenderer::Create(&target, FilterById(id),
                                         {
                                           .ReferenceBox = {10, 10, 40, 20},
                                           .FilterRegion = {6, 8, 48, 24},
                                           .Scale = {1, 1},
                                         },
                                         {FilterRenderingMode::Accelerated}, context);
      }
    };

    KRYS_NODISCARD utf8_string Representation(const SVGFilterRenderer &filter) noexcept
    {
      TextStream ts(TextStream::LineMode::MultipleLine);
      filter.ExternalRepresentation(ts, FilterRepresentation::TestOutput);
      return ts.Release();
    }
  }

  TEST_CASE("SVGFilterRenderer: builds an expression over the filter's graph",
            "[SVG][Filters][SVGFilterRenderer]")
  {
    SVGFilterFixture fixture(
      u8R"(<filter id=f><feGaussianBlur in=SourceAlpha stdDeviation=2 result=blur />)"
      u8R"(<feOffset in=blur dx=3 dy=4 result=shadow /><feBlend in=SourceGraphic in2=shadow /></filter>)");

    auto filter = fixture.Create(u8"f");
    REQUIRE(filter);

    // Each effect follows its inputs; SourceAlpha's own input, the source graphic, isn't dumped.
    auto text = Representation(*filter);
    CAPTURE(css::tests::ToNarrow(text));
    CHECK(text == u8R"([feBlend mode="normal"]
  [SourceGraphic]
  [feOffset dx="3.00" dy="4.00"]
    [feGaussianBlur stdDeviation="2.00, 2.00"]
      [SourceAlpha]
)");

    // The source graphic, the source alpha and the three primitives; the source graphic is visited twice.
    CHECK(filter->Effects().size() == 5);
    CHECK(filter->Expression().size() == 6);
    CHECK(SVGFilterRenderer::IsValidSVGFilterExpression(filter->Expression(), filter->Effects()));
    CHECK(filter->Expression().back().Level == 0);
    CHECK(filter->EffectsOfType(FilterFunction::Type::FEOffset).size() == 1);
    CHECK(filter->FilterRenderingModes() == EnumSet<FilterRenderingMode> {FilterRenderingMode::Accelerated});
  }

  TEST_CASE("SVGFilterRenderer: a primitive's default input is the previous result",
            "[SVG][Filters][SVGFilterRenderer]")
  {
    SVGFilterFixture fixture(u8R"(<filter id=f><feOffset dx=1 result=a /><feGaussianBlur stdDeviation=1 />)"
                             u8R"(<feBlend in=a /></filter>)");

    auto filter = fixture.Create(u8"f");
    REQUIRE(filter);

    // The blur's input is the offset's result, named or not; the blend's second input is the blur's.
    auto text = Representation(*filter);
    CAPTURE(css::tests::ToNarrow(text));
    CHECK(text == u8R"([feBlend mode="normal"]
  [feOffset dx="1.00" dy="0.00"]
    [SourceGraphic]
  [feGaussianBlur stdDeviation="1.00, 1.00"]
    [feOffset dx="1.00" dy="0.00"]
      [SourceGraphic]
)");
  }

  TEST_CASE("SVGFilterRenderer: an unknown input is the previous result", "[SVG][Filters][SVGFilterRenderer]")
  {
    SVGFilterFixture fixture(
      u8R"(<filter id=f><feOffset in=missing dx=1 /><feOffset in=missing dx=2 /></filter>)");

    auto filter = fixture.Create(u8"f");
    REQUIRE(filter);
    auto text = Representation(*filter);
    CAPTURE(css::tests::ToNarrow(text));
    CHECK(text == u8R"([feOffset dx="2.00" dy="0.00"]
  [feOffset dx="1.00" dy="0.00"]
    [SourceGraphic]
)");
  }

  TEST_CASE("SVGFilterRenderer: a bad primitive or no primitives make the filter invalid",
            "[SVG][Filters][SVGFilterRenderer]")
  {
    SVGFilterFixture fixture(u8R"(<filter id=negative><feGaussianBlur stdDeviation=-1 /></filter>)"
                             u8R"(<filter id=empty></filter>)");

    CHECK_FALSE(fixture.Create(u8"negative"));
    CHECK_FALSE(fixture.Create(u8"empty"));
  }

  TEST_CASE("SVGFilterRenderer: effects operate in linear RGB unless the primitive asks for sRGB",
            "[SVG][Filters][SVGFilterRenderer]")
  {
    SVGFilterFixture fixture(
      u8R"(<filter id=f><feOffset id=linear dx=1 />)"
      u8R"(<feOffset id=srgb dx=2 style="color-interpolation-filters: sRGB" /></filter>)");

    auto filter = fixture.Create(u8"f");
    REQUIRE(filter);

    auto offsets = filter->EffectsOfType(FilterFunction::Type::FEOffset);
    REQUIRE(offsets.size() == 2);
    for (const auto &offset : offsets)
    {
      auto dx = Downcast<FEOffset>(*offset).Dx();
      CHECK(offset->OperatingColorSpace()
            == (dx == 1 ? DestinationColorSpace::LinearSRGB() : DestinationColorSpace::SRGB()));
    }
  }

  TEST_CASE("SVGFilterRenderer: a filter is the identity when all its primitives are",
            "[SVG][Filters][SVGFilterRenderer]")
  {
    SVGFilterFixture fixture(u8R"(<filter id=identity><feOffset /><feGaussianBlur stdDeviation=0 /></filter>)"
                             u8R"(<filter id=moves><feOffset dx=1 /></filter><filter id=empty></filter>)"
                             u8R"(<filter id=flood><feFlood /></filter>)");

    CHECK(SVGFilterRenderer::IsIdentity(fixture.FilterById(u8"identity")));
    CHECK_FALSE(SVGFilterRenderer::IsIdentity(fixture.FilterById(u8"moves")));
    CHECK_FALSE(SVGFilterRenderer::IsIdentity(fixture.FilterById(u8"empty")));
    CHECK_FALSE(SVGFilterRenderer::IsIdentity(fixture.FilterById(u8"flood")));
  }

  TEST_CASE("SVGFilterRenderer: a filter's outsets add up along its graph",
            "[SVG][Filters][SVGFilterRenderer]")
  {
    SVGFilterFixture fixture(
      u8R"(<filter id=chain><feGaussianBlur stdDeviation=2 /><feOffset dx=5 dy=-3 /></filter>)"
      u8R"(<filter id=fork><feOffset dx=4 result=a /><feOffset in=SourceGraphic dx=-6 />)"
      u8R"(<feBlend in=a /></filter>)"
      u8R"(<filter id=units primitiveUnits=objectBoundingBox><feOffset dx=.5 /></filter>)");

    // A chain adds its primitives' outsets.
    auto blurOutsets = FEGaussianBlur::CalculateOutsets({2, 2});
    auto offsetOutsets = FEOffset::CalculateOutsets({5, -3});
    CHECK(SVGFilterRenderer::CalculateOutsets(fixture.FilterById(u8"chain"), {10, 10, 40, 20})
          == blurOutsets + offsetOutsets);

    // Inputs side by side take the larger of their outsets.
    CHECK(SVGFilterRenderer::CalculateOutsets(fixture.FilterById(u8"fork"), {10, 10, 40, 20})
          == max(FEOffset::CalculateOutsets({4, 0}), FEOffset::CalculateOutsets({-6, 0})));

    // In object bounding box units, the offset is a fraction of the target's size.
    CHECK(SVGFilterRenderer::CalculateOutsets(fixture.FilterById(u8"units"), {10, 10, 40, 20})
          == FEOffset::CalculateOutsets({20, 0}));
    CHECK(SVGFilterRenderer::CalculateResolvedSize({.5f, .25f}, {10, 10, 40, 20},
                                                   svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
          == FloatSize {20, 5});
    CHECK(SVGFilterRenderer::CalculateResolvedSize({.5f, .25f}, {10, 10, 40, 20},
                                                   svg::SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)
          == FloatSize {.5f, .25f});
  }

  TEST_CASE("SVGFilterRenderer: a reused result name refers to the latest primitive",
            "[SVG][Filters][SVGFilterRenderer]")
  {
    // The second offset reads the first (the only `a` so far), and the blend reads the second.
    SVGFilterFixture fixture(u8R"(<filter id=f><feOffset dx=1 result=a /><feOffset in=a dx=2 result=a />)"
                             u8R"(<feBlend in=a in2=SourceGraphic /></filter>)");

    auto filter = fixture.Create(u8"f");
    REQUIRE(filter);
    auto text = Representation(*filter);
    CAPTURE(css::tests::ToNarrow(text));
    CHECK(text == u8R"([feBlend mode="normal"]
  [feOffset dx="2.00" dy="0.00"]
    [feOffset dx="1.00" dy="0.00"]
      [SourceGraphic]
  [SourceGraphic]
)");
  }
}
