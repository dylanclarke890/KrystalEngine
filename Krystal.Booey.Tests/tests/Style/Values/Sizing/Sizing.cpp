#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexBasis.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexGrow.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexShrink.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Sizing/AspectRatio.hpp"
#include "Krystal.Booey/Style/Values/Sizing/ContainIntrinsicSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MaximumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;

  TEST_CASE_METHOD(BuilderFixture, "Style sizes", "[Style][Values][Sizing]")
  {
    SECTION("Preferred sizes")
    {
      CHECK(Convert<PreferredSize>(PropertyId::Width, u8"auto").IsAuto());
      CHECK(Convert<PreferredSize>(PropertyId::Width, u8"min-content").IsMinContent());
      CHECK(Convert<PreferredSize>(PropertyId::Width, u8"fit-content").IsFitContent());
      CHECK(Convert<PreferredSize>(PropertyId::Width, u8"10px") == PreferredSize {10_css_px});
      CHECK(Convert<PreferredSize>(PropertyId::Height, u8"50%") == PreferredSize {50_css_percentage});
      CHECK(Serialised(Convert<PreferredSize>(PropertyId::Width, u8"max-content")) == "max-content");
    }

    SECTION("Minimum and maximum sizes")
    {
      CHECK(Convert<MinimumSize>(PropertyId::MinWidth, u8"auto").IsAuto());
      CHECK(Convert<MinimumSize>(PropertyId::MinWidth, u8"1px") == MinimumSize {1_css_px});
      CHECK(Convert<MaximumSize>(PropertyId::MaxWidth, u8"none").IsNone());
      CHECK(Convert<MaximumSize>(PropertyId::MaxHeight, u8"max-content").IsMaxContent());
    }

    SECTION("Conversions between the size types")
    {
      auto size = Convert<PreferredSize>(PropertyId::Width, u8"20px");
      CHECK(size.AsMinimumSize() == MinimumSize {20_css_px});
      CHECK(size.AsFlexBasis() == FlexBasis {20_css_px});
      CHECK(PreferredSize {css::keywords::Auto {}}.AsMinimumSize().IsAuto());
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style AspectRatio", "[Style][Values][Sizing]")
  {
    CHECK(Convert<AspectRatio>(PropertyId::AspectRatio, u8"auto").IsAuto());

    auto ratio = Convert<AspectRatio>(PropertyId::AspectRatio, u8"16 / 9");
    REQUIRE(ratio.IsRatio());
    CHECK(ratio.Width() == 16);
    CHECK(ratio.Height() == 9);
    CHECK(Serialised(ratio) == "16 / 9");

    auto autoAndRatio = Convert<AspectRatio>(PropertyId::AspectRatio, u8"auto 2 / 1");
    CHECK(autoAndRatio.IsAutoAndRatio());
    CHECK(autoAndRatio.HasRatio());
    CHECK(Serialised(autoAndRatio) == "auto 2 / 1");

    CHECK(Convert<AspectRatio>(PropertyId::AspectRatio, u8"0 / 1").IsAutoZero());
    CHECK_FALSE(IsInvalid());

    SECTION("Blending interpolates the logical ratio")
    {
      RenderStyle aStyle;
      aStyle.SetAspectRatio(Ratio {1, 1});
      RenderStyle bStyle;
      bStyle.SetAspectRatio(Ratio {4, 1});
      CHECK(aStyle.LogicalAspectRatio() == 1);
      CHECK(bStyle.LogicalAspectRatio() == 4);

      // Vertical writing modes swap the ratio.
      bStyle.SetWritingMode(StyleWritingMode::VerticalRl);
      CHECK(bStyle.LogicalAspectRatio() == 0.25);
      bStyle.SetWritingMode(StyleWritingMode::HorizontalTb);

      CHECK(CanBlend(AspectRatio {Ratio {1, 1}}, AspectRatio {Ratio {4, 1}}));
      CHECK_FALSE(CanBlend(AspectRatio {Ratio {1, 1}}, AspectRatio {css::keywords::Auto {}, Ratio {4, 1}}));
      auto blended = Blending<AspectRatio> {}.Blend(AspectRatio {Ratio {1, 1}}, AspectRatio {Ratio {4, 1}},
                                                    aStyle, bStyle, BlendingContext {0.5});
      CHECK(blended.Width() == 2);
      CHECK(blended.Height() == 1);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style ContainIntrinsicSize", "[Style][Values][Sizing]")
  {
    CHECK(Convert<ContainIntrinsicSize>(PropertyId::ContainIntrinsicWidth, u8"none").IsNone());

    auto length = Convert<ContainIntrinsicSize>(PropertyId::ContainIntrinsicWidth, u8"10px");
    CHECK(length.HasLength());
    CHECK(length.TryLength() == ContainIntrinsicSize::Length {10});
    CHECK(Serialised(length) == "10px");

    auto autoLength = Convert<ContainIntrinsicSize>(PropertyId::ContainIntrinsicWidth, u8"auto 20px");
    CHECK(autoLength.HasAuto());
    CHECK(autoLength.HasLength());
    CHECK(Serialised(autoLength) == "auto 20px");

    auto autoNone = Convert<ContainIntrinsicSize>(PropertyId::ContainIntrinsicWidth, u8"auto none");
    CHECK(autoNone.IsAutoAndNone());
    CHECK(Serialised(autoNone) == "auto none");
    CHECK(length.AddingAuto()
          == ContainIntrinsicSize {css::keywords::Auto {}, ContainIntrinsicSize::Length {10}});
    CHECK_FALSE(IsInvalid());

    auto blended = Blending<ContainIntrinsicSize> {}.Blend(length, ContainIntrinsicSize {30_css_px},
                                                           BlendingContext {0.5});
    CHECK(blended.TryLength() == ContainIntrinsicSize::Length {20});
  }

  TEST_CASE_METHOD(BuilderFixture, "Style flex values", "[Style][Values][Flexbox]")
  {
    CHECK(Convert<FlexBasis>(PropertyId::FlexBasis, u8"content").IsContent());
    CHECK_FALSE(Convert<FlexBasis>(PropertyId::FlexBasis, u8"content").TryPreferredSize());
    CHECK(Convert<FlexBasis>(PropertyId::FlexBasis, u8"auto").IsAuto());
    CHECK(Convert<FlexBasis>(PropertyId::FlexBasis, u8"10px").TryPreferredSize()
          == PreferredSize {10_css_px});
    CHECK(Convert<FlexGrow>(PropertyId::FlexGrow, u8"2.5").Value == 2.5f);
    CHECK(Convert<FlexShrink>(PropertyId::FlexShrink, u8"0").Value == 0.f);
  }
}
