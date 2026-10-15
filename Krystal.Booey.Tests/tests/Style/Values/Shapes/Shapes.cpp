#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/ShapeOutside.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  namespace
  {
    const FloatRect Reference {0, 0, 100, 200};

    /// @brief The path of the shape in a `shape-outside` value, against `Reference`.
    KRYS_NODISCARD boo::Path ShapePath(const ShapeOutside &value) noexcept
    {
      const auto *shape = value.GetShape();
      REQUIRE(shape);
      return ComputePath(*shape, Reference);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style shape-outside", "[Style][Values][Shapes]")
  {
    CHECK(Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"none").IsNone());

    auto box = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"margin-box");
    CHECK(Serialised(box) == "margin-box");
    CHECK(box.GetShape() == nullptr);
    CHECK(box.EffectiveCSSBox() == CSSBoxType::MarginBox);

    SECTION("circle")
    {
      auto circle =
        Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"circle(50px at 50px 100px) content-box");
      CHECK(Serialised(circle) == "circle(50px at 50px 100px) content-box");
      CHECK(Created(circle) == "circle(50px at 50px 100px) content-box");
      CHECK(circle.EffectiveCSSBox() == CSSBoxType::ContentBox);
      CHECK(ShapePath(circle).BoundingRect() == FloatRect {0, 50, 100, 100});
    }

    SECTION("ellipse")
    {
      auto ellipse = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"ellipse(50% 25% at center)");
      CHECK(Serialised(ellipse) == "ellipse(50% 25% at 50% 50%)");
      CHECK(ShapePath(ellipse).BoundingRect() == FloatRect {0, 50, 100, 100});
    }

    SECTION("inset and its aliases")
    {
      auto inset = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"inset(10px 20px)");
      CHECK(Serialised(inset) == "inset(10px 20px)");
      CHECK(ShapePath(inset).BoundingRect() == FloatRect {20, 10, 60, 180});

      // rect() and xywh() become insets.
      auto rect = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"rect(10px 80px 190px 20px)");
      CHECK(ShapePath(rect).BoundingRect() == FloatRect {20, 10, 60, 180});
      auto xywh = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"xywh(20px 10px 60px 180px)");
      CHECK(ShapePath(xywh).BoundingRect() == FloatRect {20, 10, 60, 180});
    }

    SECTION("polygon")
    {
      auto polygon =
        Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"polygon(evenodd, 0 0, 100% 0, 0 100%)");
      CHECK(Serialised(polygon) == "polygon(evenodd, 0px 0px, 100% 0px, 0px 100%)");
      CHECK(ShapePath(polygon).BoundingRect() == FloatRect {0, 0, 100, 200});
      CHECK(ComputeWindRule(*polygon.GetShape()) == WindRule::EvenOdd);

      auto nonzero = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"polygon(0 0, 10px 0, 0 10px)");
      CHECK(ComputeWindRule(*nonzero.GetShape()) == WindRule::NonZero);
    }

    SECTION("shape()")
    {
      auto shape = Convert<ShapeOutside>(PropertyId::ShapeOutside,
                                         u8"shape(from 10px 10px, line to 50px 10px, vline by 30px, close)");
      CHECK(ShapePath(shape).BoundingRect() == FloatRect {10, 10, 40, 30});
    }

    SECTION("blending")
    {
      auto from = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"circle(10px at 0px 0px)");
      auto to = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"circle(30px at 20px 40px)");
      CHECK(Blending<ShapeOutside> {}.CanBlend(from, to));
      CHECK(Serialised(Blending<ShapeOutside> {}.Blend(from, to, BlendingContext {0.5}))
            == "circle(20px at 10px 20px)");

      // Different shape kinds don't blend.
      auto inset = Convert<ShapeOutside>(PropertyId::ShapeOutside, u8"inset(10px)");
      CHECK_FALSE(Blending<ShapeOutside> {}.CanBlend(from, inset));
    }
  }
}
