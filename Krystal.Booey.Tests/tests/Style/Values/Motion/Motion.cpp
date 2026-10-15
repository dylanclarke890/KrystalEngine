#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Rendering/PathOperation.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetAnchor.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetDistance.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetPath.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetPosition.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetRotate.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using Catch::Approx;
  using css::PropertyId;

  namespace
  {
    /// @brief A 200x100 containing block with the element's offset starting position at its centre.
    KRYS_NODISCARD TransformOperationData MotionData() noexcept
    {
      return TransformOperationData {
        FloatRect {0, 0, 20, 20},
        MotionPathData {.ContainingBlockBoundingRect = FloatRoundedRect {FloatRect {0, 0, 200, 100}},
                        .OffsetFromContainingBlock = FloatPoint {0, 0},
                        .UsedStartingPosition = FloatPoint {100, 50}}};
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style offset-path", "[Style][Values][Motion]")
  {
    auto none = Convert<OffsetPath>(PropertyId::OffsetPath, u8"none");
    CHECK(none.IsNone());
    CHECK_FALSE(none.AffectedByTransformOrigin());
    CHECK_FALSE(TryPath(none, MotionData()));

    SECTION("ray()")
    {
      auto ray = Convert<OffsetPath>(PropertyId::OffsetPath, u8"ray(90deg closest-side)");
      REQUIRE(ray.IsRay());
      CHECK(Serialised(ray) == "ray(90deg)");
      CHECK(Created(ray) == "ray(90deg)");
      CHECK(ray.AffectedByTransformOrigin());
      CHECK(MotionPath::NeedsUpdateAfterContainingBlockLayout(ray));

      // From the centre, the closest side is 50px away; 90deg points right.
      auto path = TryPath(ray, MotionData());
      REQUIRE(path);
      CHECK(path->Length() == Approx(50));
      CHECK(path->CurrentPoint().X() == Approx(150));
      CHECK(path->CurrentPoint().Y() == Approx(50).margin(1e-4));

      // `contain` shortens the ray by half the element's larger side.
      auto contained = Convert<OffsetPath>(PropertyId::OffsetPath, u8"ray(90deg closest-side contain)");
      CHECK(TryPath(contained, MotionData())->Length() == Approx(40));

      // `farthest-corner` reaches the corner diagonally.
      auto corner = Convert<OffsetPath>(PropertyId::OffsetPath, u8"ray(0deg farthest-corner)");
      CHECK(TryPath(corner, MotionData())->Length() == Approx(std::hypot(100.0, 50.0)));

      // A ray needs the containing block.
      CHECK_FALSE(TryPath(ray, TransformOperationData {FloatRect {0, 0, 20, 20}}));
    }

    SECTION("basic shapes and boxes")
    {
      auto circle = Convert<OffsetPath>(PropertyId::OffsetPath, u8"circle(10px)");
      REQUIRE(circle.IsBasicShape());
      // Without a position, the circle is centred on the starting position.
      CHECK(TryPath(circle, MotionData())->BoundingRect() == FloatRect {90, 40, 20, 20});

      auto box = Convert<OffsetPath>(PropertyId::OffsetPath, u8"border-box");
      REQUIRE(box.IsBox());
      CHECK(TryPath(box, MotionData())->BoundingRect() == FloatRect {0, 0, 200, 100});
      CHECK_FALSE(TryPath(box, TransformOperationData {FloatRect {0, 0, 20, 20}}));

      auto reference = Convert<OffsetPath>(PropertyId::OffsetPath, u8"url(#path)");
      CHECK(reference.IsReference());
      CHECK_FALSE(MotionPath::NeedsUpdateAfterContainingBlockLayout(reference));
    }

    SECTION("blending")
    {
      auto from = Convert<OffsetPath>(PropertyId::OffsetPath, u8"ray(0deg)");
      auto to = Convert<OffsetPath>(PropertyId::OffsetPath, u8"ray(90deg)");
      CHECK(Blending<OffsetPath> {}.CanBlend(from, to));
      CHECK(Serialised(Blending<OffsetPath> {}.Blend(from, to, BlendingContext {0.5})) == "ray(45deg)");
      CHECK_FALSE(Blending<OffsetPath> {}.CanBlend(
        from, Convert<OffsetPath>(PropertyId::OffsetPath, u8"circle(10px)")));
    }
  }

  TEST_CASE("Path operations", "[Rendering][PathOperation]")
  {
    auto box = BoxPathOperation::Create(CSSBoxType::ContentBox);
    CHECK(box->GetType() == PathOperation::Type::Box);
    CHECK(*box == *box->Clone());
    CHECK_FALSE(*box == *BoxPathOperation::Create(CSSBoxType::BorderBox));
    auto operation = box->Clone();
    CHECK(Is<BoxPathOperation>(*operation));
    CHECK_FALSE(Is<RayPathOperation>(*operation));
    CHECK_FALSE(box->CanBlend(*box->Clone()));

    boo::Path square;
    square.AddRect({0, 0, 10, 10});
    auto reference = ReferencePathOperation::Create(Maybe<boo::Path> {square});
    REQUIRE(reference->GetPath());
    CHECK(reference->GetPath(TransformOperationData {FloatRect {}})->BoundingRect()
          == FloatRect {0, 0, 10, 10});
    CHECK(Downcast<ReferencePathOperation>(*reference->Clone()).GetPath()->BoundingRect()
          == FloatRect {0, 0, 10, 10});
  }

  TEST_CASE("Motion path transform", "[Rendering][MotionPath]")
  {
    boo::Path line;
    line.MoveTo({0, 0});
    line.AddLineTo({100, 0});

    TransformationMatrix matrix;
    MotionPath::ApplyMotionPathTransform(matrix, TransformOperationData {FloatRect {0, 0, 10, 10}},
                                         FloatPoint {5, 5}, TransformBox::BorderBox, line, std::nullopt, 40,
                                         0, false);
    // The anchor (the transform origin) moves to 40px along the line.
    CHECK(matrix.MapPoint(FloatPoint {5, 5}).X() == Approx(40));
    CHECK(matrix.MapPoint(FloatPoint {5, 5}).Y() == Approx(0).margin(1e-4));

    // Distances wrap around closed paths and clamp on open ones.
    TransformationMatrix clamped;
    MotionPath::ApplyMotionPathTransform(clamped, TransformOperationData {FloatRect {0, 0, 10, 10}},
                                         FloatPoint {5, 5}, TransformBox::BorderBox, line, std::nullopt, 400,
                                         0, false);
    CHECK(clamped.MapPoint(FloatPoint {5, 5}).X() == Approx(100));
  }

  TEST_CASE_METHOD(BuilderFixture, "Style offset values", "[Style][Values][Motion]")
  {
    CHECK(Convert<OffsetAnchor>(PropertyId::OffsetAnchor, u8"auto").IsAuto());
    CHECK(Serialised(Convert<OffsetAnchor>(PropertyId::OffsetAnchor, u8"left top")) == "0% 0%");

    CHECK(Convert<OffsetPosition>(PropertyId::OffsetPosition, u8"normal").IsNormal());
    CHECK(Convert<OffsetPosition>(PropertyId::OffsetPosition, u8"auto").IsAuto());
    CHECK(Serialised(Convert<OffsetPosition>(PropertyId::OffsetPosition, u8"10px 20%")) == "10px 20%");

    auto rotate = Convert<OffsetRotate>(PropertyId::OffsetRotate, u8"reverse 10deg");
    CHECK(rotate.HasAuto());
    CHECK(rotate.GetAngle().Value == Approx(190));
    CHECK(Serialised(Convert<OffsetRotate>(PropertyId::OffsetRotate, u8"30deg")) == "30deg");

    auto from = Convert<OffsetRotate>(PropertyId::OffsetRotate, u8"auto 0deg");
    auto to = Convert<OffsetRotate>(PropertyId::OffsetRotate, u8"auto 90deg");
    CHECK(Blending<OffsetRotate> {}.CanBlend(from, to));
    CHECK(Blending<OffsetRotate> {}.Blend(from, to, BlendingContext {0.5}).GetAngle().Value == Approx(45));
    CHECK_FALSE(
      Blending<OffsetRotate> {}.CanBlend(from, Convert<OffsetRotate>(PropertyId::OffsetRotate, u8"0deg")));

    CHECK(Serialised(Convert<OffsetDistance>(PropertyId::OffsetDistance, u8"50%")) == "50%");
  }
}
