#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/Style/Values/Borders/BoxShadow.hpp"
#include "Krystal.Booey/Style/Values/Borders/CornerShapeValue.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using Catch::Approx;
  using css::PropertyId;

  TEST_CASE_METHOD(BuilderFixture, "Style border radius", "[Style][Values][Borders]")
  {
    auto circle = Convert<BorderRadiusValue>(PropertyId::BorderTopLeftRadius, u8"10px");
    CHECK(Serialised(circle) == "10px");
    auto ellipse = Convert<BorderRadiusValue>(PropertyId::BorderTopLeftRadius, u8"10px 50%");
    CHECK(Serialised(ellipse) == "10px 50%");
    CHECK(Created(ellipse) == "10px 50%");

    BorderRadius radius {ellipse, circle, circle, circle};
    auto radii = Evaluate<FloatRoundedRect::Radii>(radius, FloatSize {100, 40}, ZoomNeeded {});
    CHECK(radii.TopLeft() == FloatSize {10, 20});
    CHECK(radii.TopRight() == FloatSize {10, 10});

    auto layoutRadii = Evaluate<LayoutRoundedRect::Radii>(radius, LayoutSize {100, 40}, ZoomNeeded {});
    CHECK(layoutRadii.TopLeft() == LayoutSize {10, 20});
  }

  TEST_CASE_METHOD(BuilderFixture, "Style box-shadow", "[Style][Values][Borders]")
  {
    CHECK(Convert<BoxShadows>(PropertyId::BoxShadow, u8"none").IsNone());

    auto shadows = Convert<BoxShadows>(PropertyId::BoxShadow, u8"1px 2px 3px 4px red, inset 0 0 5px blue");
    REQUIRE_FALSE(shadows.IsNone());
    CHECK(Serialised(shadows) == "rgb(255, 0, 0) 1px 2px 3px 4px, rgb(0, 0, 255) 0px 0px 5px 0px inset");
    CHECK(Created(shadows) == "rgb(255, 0, 0) 1px 2px 3px 4px, rgb(0, 0, 255) 0px 0px 5px 0px inset");

    // An unspecified colour is the current colour, which the computed value resolves against `color`.
    CHECK(Serialised(Convert<BoxShadows>(PropertyId::BoxShadow, u8"1px 1px"))
          == "rgb(0, 0, 0) 1px 1px 0px 0px");

    // Painting extents: the outset shadow reaches beyond the box by its offset, blur and spread.
    auto extent = ShadowOutsetExtent(shadows, ZoomFactor {1, 1});
    CHECK(extent.Right() > 0);
    CHECK(extent.Left() < 0);

    SECTION("blending")
    {
      auto from = Convert<BoxShadows>(PropertyId::BoxShadow, u8"0 0 0 0 black");
      auto to = Convert<BoxShadows>(PropertyId::BoxShadow, u8"10px 20px 4px 2px black");
      CHECK(Blending<BoxShadows> {}.CanBlend(from, to, CompositeOperation::Replace));
      CHECK(Serialised(Blending<BoxShadows> {}.Blend(from, to, Style, Style, BlendingContext {0.5}))
            == "rgb(0, 0, 0) 5px 10px 2px 1px");
      // Inset and outset shadows don't blend.
      CHECK_FALSE(Blending<BoxShadows> {}.CanBlend(
        from, Convert<BoxShadows>(PropertyId::BoxShadow, u8"inset 0 0 black"), CompositeOperation::Replace));
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style corner-shape", "[Style][Values][Borders]")
  {
    CHECK(Serialised(Convert<CornerShapeValue>(PropertyId::CornerTopLeftShape, u8"round")) == "round");
    CHECK(Serialised(Convert<CornerShapeValue>(PropertyId::CornerTopLeftShape, u8"superellipse(4)"))
          == "squircle");
    CHECK(Serialised(Convert<CornerShapeValue>(PropertyId::CornerTopLeftShape, u8"superellipse(3)"))
          == "superellipse(3)");
    CHECK(Serialised(Convert<CornerShapeValue>(PropertyId::CornerTopLeftShape, u8"superellipse(infinity)"))
          == "straight");
    CHECK(Convert<CornerShapeValue>(PropertyId::CornerTopLeftShape, u8"bevel")
          == CornerShapeValue {css::keywords::Bevel {}});

    // Interpolation happens in a normalised space, so bevel (1) and round (2) meet between them.
    auto blended =
      Blending<CornerShapeValue> {}.Blend(CornerShapeValue {css::keywords::Bevel {}},
                                          CornerShapeValue {css::keywords::Round {}}, BlendingContext {0.5});
    CHECK(blended.Superellipse->Value > 1);
    CHECK(blended.Superellipse->Value < 2);
  }
}
