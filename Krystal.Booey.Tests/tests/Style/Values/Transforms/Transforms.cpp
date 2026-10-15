#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Perspective.hpp"
#include "Krystal.Booey/Style/Values/Transforms/PerspectiveOrigin.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Rotate.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Scale.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"
#include "Krystal.Booey/Style/Values/Transforms/TransformOrigin.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Translate.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  namespace
  {
    KRYS_NODISCARD TransformationMatrix Applied(const Transform &transform,
                                                FloatSize size = {100, 50}) noexcept
    {
      TransformationMatrix matrix;
      transform.Apply(matrix, size);
      return matrix;
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style Transform", "[Style][Values][Transforms]")
  {
    CHECK(Convert<Transform>(PropertyId::Transform, u8"none").IsNone());
    CHECK(Serialised(Convert<Transform>(PropertyId::Transform, u8"none")) == "none");

    auto transform =
      Convert<Transform>(PropertyId::Transform, u8"translate(10px, 20%) rotate(45deg) scale(2)");
    CHECK(Serialised(transform) == "translate(10px, 20%) rotate(45deg) scale(2)");
    CHECK(Created(transform) == "translate(10px, 20%) rotate(45deg) scale(2)");
    CHECK(transform.HasTransformOfType<TransformFunctionType::Rotate>());
    CHECK_FALSE(transform.Has3DOperation());
    CHECK(transform.AffectedByTransformOrigin());
    // The percentage resolves against the box height.
    CHECK(transform.ComputeSizeDependencies().IsHeightDependent);
    CHECK_FALSE(transform.ComputeSizeDependencies().IsWidthDependent);

    auto matrix = Applied(Convert<Transform>(PropertyId::Transform, u8"translate(10px, 20%)"));
    CHECK(matrix.E() == 10);
    CHECK(matrix.F() == 10);

    auto platform = ConvertToPlatform(transform, FloatSize {100, 50});
    REQUIRE(platform.size() == 3);
    CHECK(platform[0]->GetType() == TransformOperation::Type::Translate);
    TransformationMatrix platformMatrix;
    platform.Apply(platformMatrix);
    CHECK(platformMatrix == Applied(transform));

    SECTION("functions")
    {
      CHECK(Serialised(Convert<Transform>(PropertyId::Transform, u8"translate3d(1px, 2px, 3px)"))
            == "translate3d(1px, 2px, 3px)");
      CHECK(Serialised(Convert<Transform>(PropertyId::Transform, u8"translateX(5px) translateY(0)"))
            == "translateX(5px) translateY(0px)");
      CHECK(Serialised(Convert<Transform>(PropertyId::Transform, u8"scale(2, 2) scale3d(1, 2, 3)"))
            == "scale(2) scale3d(1, 2, 3)");
      CHECK(Serialised(Convert<Transform>(PropertyId::Transform, u8"rotate3d(0, 0, 1, 10deg) rotateX(1turn)"))
            == "rotate3d(0, 0, 1, 10deg) rotateX(360deg)");
      CHECK(
        Serialised(Convert<Transform>(PropertyId::Transform, u8"skew(10deg) skew(10deg, 20deg) skewY(5deg)"))
        == "skew(10deg) skew(10deg, 20deg) skewY(5deg)");
      CHECK(Serialised(Convert<Transform>(PropertyId::Transform, u8"perspective(100px)"))
            == "perspective(100px)");
      CHECK(Serialised(Convert<Transform>(PropertyId::Transform, u8"matrix(1, 2, 3, 4, 5, 6)"))
            == "matrix(1, 2, 3, 4, 5, 6)");
      CHECK(Serialised(Convert<Transform>(PropertyId::Transform,
                                          u8"matrix3d(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2)"))
            == "matrix3d(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2)");
      CHECK(Convert<Transform>(PropertyId::Transform, u8"translateZ(1px)").Has3DOperation());
    }

    SECTION("blending")
    {
      auto from = Convert<Transform>(PropertyId::Transform, u8"translate(0px) rotate(0deg)");
      auto to = Convert<Transform>(PropertyId::Transform, u8"translate(20px) rotate(90deg)");
      CHECK(Blending<Transform> {}.CanBlend(from, to, CompositeOperation::Replace));
      auto blended = Blending<Transform> {}.Blend(from, to, BlendingContext {0.5});
      CHECK(Serialised(blended) == "translate(10px) rotate(45deg)");

      // Different primitives blend as matrices from there on.
      auto mismatched = Blending<Transform> {}.Blend(
        Convert<Transform>(PropertyId::Transform, u8"rotate(90deg)"),
        Convert<Transform>(PropertyId::Transform, u8"translate(10px)"), BlendingContext {0});
      CHECK(Serialised(mismatched).starts_with("matrix("));

      auto added =
        Blending<Transform> {}.Blend(from, to, BlendingContext {1, false, CompositeOperation::Add});
      CHECK(Serialised(added) == "translate(0px) rotate(0deg) translate(20px) rotate(90deg)");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style individual transforms", "[Style][Values][Transforms]")
  {
    SECTION("rotate")
    {
      CHECK(Convert<Rotate>(PropertyId::Rotate, u8"none").IsNone());
      CHECK(Serialised(Convert<Rotate>(PropertyId::Rotate, u8"45deg")) == "45deg");
      CHECK(Serialised(Convert<Rotate>(PropertyId::Rotate, u8"x 90deg")) == "x 90deg");
      CHECK(Serialised(Convert<Rotate>(PropertyId::Rotate, u8"1 2 3 10deg")) == "1 2 3 10deg");
      CHECK(Convert<Rotate>(PropertyId::Rotate, u8"y 10deg").Is3DOperation());
      auto blended =
        Blending<Rotate> {}.Blend(Rotate {css::keywords::None {}},
                                  Convert<Rotate>(PropertyId::Rotate, u8"90deg"), BlendingContext {0.5});
      CHECK(Serialised(blended) == "45deg");
    }

    SECTION("scale")
    {
      CHECK(Convert<Scale>(PropertyId::Scale, u8"none").IsNone());
      CHECK(Serialised(Convert<Scale>(PropertyId::Scale, u8"2")) == "2");
      CHECK(Serialised(Convert<Scale>(PropertyId::Scale, u8"2 3")) == "2 3");
      CHECK(Serialised(Convert<Scale>(PropertyId::Scale, u8"50% 1 2")) == "0.5 1 2");
      auto blended =
        Blending<Scale> {}.Blend(Convert<Scale>(PropertyId::Scale, u8"1"),
                                 Convert<Scale>(PropertyId::Scale, u8"3"), BlendingContext {0.5});
      CHECK(Serialised(blended) == "2");
    }

    SECTION("translate")
    {
      CHECK(Convert<Translate>(PropertyId::Translate, u8"none").IsNone());
      CHECK(Serialised(Convert<Translate>(PropertyId::Translate, u8"10px")) == "10px");
      CHECK(Serialised(Convert<Translate>(PropertyId::Translate, u8"10px 20%")) == "10px 20%");
      CHECK(Serialised(Convert<Translate>(PropertyId::Translate, u8"10px 20px 30px")) == "10px 20px 30px");
      auto translate = Convert<Translate>(PropertyId::Translate, u8"10% 5px");
      CHECK(translate.ComputeSizeDependencies().IsWidthDependent);
      TransformationMatrix matrix;
      translate.Apply(matrix, FloatSize {200, 100});
      CHECK(matrix.E() == 20);
      auto blended = Blending<Translate> {}.Blend(Convert<Translate>(PropertyId::Translate, u8"0px"),
                                                  Convert<Translate>(PropertyId::Translate, u8"10px 20px"),
                                                  BlendingContext {0.5});
      CHECK(Serialised(blended) == "5px 10px");
    }

    SECTION("origins")
    {
      using namespace css::Literals;
      TransformOrigin origin {Convert<TransformOriginX>(PropertyId::TransformOriginX, u8"left"),
                              Convert<TransformOriginY>(PropertyId::TransformOriginY, u8"25%"),
                              TransformOriginZ {5_css_px}};
      CHECK(origin.XY() == TransformOriginXY {origin.X, origin.Y});
      CHECK(Serialised(origin) == "0% 25% 5px");
      PerspectiveOrigin perspectiveOrigin {
        Convert<PerspectiveOriginX>(PropertyId::PerspectiveOriginX, u8"right"),
        Convert<PerspectiveOriginY>(PropertyId::PerspectiveOriginY, u8"10px")};
      CHECK(Serialised(perspectiveOrigin) == "100% 10px");
    }

    SECTION("perspective")
    {
      CHECK(Convert<Perspective>(PropertyId::Perspective, u8"none").IsNone());
      auto perspective = Convert<Perspective>(PropertyId::Perspective, u8"100px");
      CHECK(perspective.IsLength());
      CHECK(perspective.UsedPerspective() == 100);
      CHECK(Serialised(perspective) == "100px");
    }
  }
}
