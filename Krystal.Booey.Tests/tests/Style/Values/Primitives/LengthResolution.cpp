#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"

namespace krys::boo::style::tests
{
  using css::LengthUnit;

  TEST_CASE("ComputeUnzoomedNonCalcLengthDouble", "[Style][LengthResolution]")
  {
    auto compute = [](double value, LengthUnit unit)
    {
      return ComputeUnzoomedNonCalcLengthDouble(value, unit, css::PropertyId::Width);
    };

    CHECK(compute(2.0, LengthUnit::Px) == 2.0);
    CHECK(compute(1.0, LengthUnit::In) == 96.0);
    CHECK(compute(2.54, LengthUnit::Cm) == Catch::Approx(96.0));
    CHECK(compute(25.4, LengthUnit::Mm) == Catch::Approx(96.0));
    CHECK(compute(101.6, LengthUnit::Q) == Catch::Approx(96.0));
    CHECK(compute(72.0, LengthUnit::Pt) == Catch::Approx(96.0));
    CHECK(compute(6.0, LengthUnit::Pc) == Catch::Approx(96.0));

    // Without a render view, viewport units are 0, except the min/max ones, which return the value unchanged.
    CHECK(compute(50.0, LengthUnit::Vw) == 0.0);
    CHECK(compute(50.0, LengthUnit::Svb) == 0.0);
    CHECK(compute(50.0, LengthUnit::Dvi) == 0.0);
    CHECK(compute(50.0, LengthUnit::Vmax) == 50.0);
    CHECK(compute(50.0, LengthUnit::Lvmin) == 50.0);
  }

  TEST_CASE("ComputeNonCalcLengthDouble(default conversion data)", "[Style][LengthResolution]")
  {
    auto conversionData = css::ToLengthConversionData {};

    CHECK(ComputeNonCalcLengthDouble(2.0, LengthUnit::Px, conversionData) == 2.0);
    CHECK(ComputeNonCalcLengthDouble(1.0, LengthUnit::In, conversionData) == 96.0);

    // Absolute lengths are scaled by zoom, except when computing font-size.
    auto zoomed = conversionData.CopyWithAdjustedZoom(2.0f);
    CHECK(ComputeNonCalcLengthDouble(1.0, LengthUnit::In, zoomed) == 192.0);
    CHECK(ComputeNonCalcLengthDouble(1.0, LengthUnit::In, zoomed.CopyForFontSize()) == 96.0);

    // Without a render view, viewport units resolve against an empty viewport.
    CHECK(ComputeNonCalcLengthDouble(50.0, LengthUnit::Vw, conversionData) == 0.0);
    CHECK(ComputeNonCalcLengthDouble(50.0, LengthUnit::Dvmax, conversionData) == 0.0);

    // Without an element, container units fall back to the small viewport units.
    CHECK(ComputeNonCalcLengthDouble(50.0, LengthUnit::Cqw, conversionData) == 0.0);
  }

  TEST_CASE("ToLengthConversionData(default)", "[CSS][ToLengthConversionData]")
  {
    auto conversionData = css::ToLengthConversionData {};
    CHECK(conversionData.Style() == nullptr);
    CHECK(conversionData.RootStyle() == nullptr);
    CHECK(conversionData.ParentStyle() == nullptr);
    CHECK(conversionData.GetRenderView() == nullptr);
    CHECK(conversionData.ElementForContainerUnitResolution() == nullptr);
    CHECK(conversionData.StyleBuilderState() == nullptr);
    CHECK(conversionData.Zoom() == 1.0f);
    CHECK(conversionData.RangeZoomOption() == css::RangeZoomOptions::Default);
    CHECK(conversionData.PropertyToCompute() == css::PropertyId::Invalid);
    CHECK_FALSE(conversionData.ComputingFontSize());
    CHECK_FALSE(conversionData.ComputingLineHeight());
    CHECK(conversionData.DefaultViewportFactor() == FloatSize {});
    CHECK(conversionData.SmallViewportFactor() == FloatSize {});
    CHECK(conversionData.LargeViewportFactor() == FloatSize {});
    CHECK(conversionData.DynamicViewportFactor() == FloatSize {});
    conversionData.SetUsesContainerUnits();

    auto fontSize = conversionData.CopyForFontSize();
    CHECK(fontSize.ComputingFontSize());
    CHECK(fontSize.PropertyToCompute() == css::PropertyId::FontSize);
    CHECK(fontSize.Zoom() == 1.0f);

    auto adjusted = conversionData.CopyWithAdjustedZoom(1.5f, css::RangeZoomOptions::Unzoomed);
    CHECK(adjusted.Zoom() == 1.5f);
    CHECK(adjusted.RangeZoomOption() == css::RangeZoomOptions::Unzoomed);
    CHECK_FALSE(adjusted.ComputingFontSize());

    auto lineHeight = conversionData.CopyForLineHeight(3.0f);
    CHECK(lineHeight.ComputingLineHeight());
    CHECK(lineHeight.Zoom() == 3.0f);
    CHECK(lineHeight.RangeZoomOption() == css::RangeZoomOptions::Unzoomed);
  }
}
