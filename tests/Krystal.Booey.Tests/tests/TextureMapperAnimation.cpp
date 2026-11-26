#include "Krystal.Booey/Compositing/TextureMapperAnimation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerFloatAnimationValue.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerTransformAnimationValue.hpp"
#include "Krystal.Web/Graphics/Transforms/ScaleTransformOperation.hpp"
#include "Krystal.Web/Graphics/Transforms/TransformOperations.hpp"
#include <chrono>

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    constexpr MonotonicTime::duration Start = std::chrono::duration_cast<MonotonicTime::duration>(10s);

    /// @brief A linear one second animation, @p iterations times in @p direction.
    KRYS_NODISCARD Ref<web::GraphicsLayerAnimation>
      LinearAnimation(double iterations = 1, web::GraphicsLayerAnimation::Direction direction =
                                               web::GraphicsLayerAnimation::Direction::Normal) noexcept
    {
      auto animation = web::GraphicsLayerAnimation::Create();
      animation->SetDuration(1.0);
      animation->SetIterationCount(iterations);
      animation->SetDirection(direction);
      animation->SetTimingFunction(web::LinearTimingFunction::Create());
      return animation;
    }

    /// @brief Opacity from 0 to 1.
    KRYS_NODISCARD web::GraphicsLayerKeyframeValueList Fade() noexcept
    {
      web::GraphicsLayerKeyframeValueList keyframes(web::AnimatedProperty::Opacity);
      keyframes.Insert(CreateUnique<web::GraphicsLayerFloatAnimationValue>(0, 0.f));
      keyframes.Insert(CreateUnique<web::GraphicsLayerFloatAnimationValue>(1, 1.f));
      return keyframes;
    }

    KRYS_NODISCARD double OpacityAt(TextureMapperAnimations &animations,
                                    MonotonicTime::duration time) noexcept
    {
      TextureMapperAnimation::ApplicationResult result;
      animations.Apply(result, time);
      REQUIRE(result.Opacity);
      return *result.Opacity;
    }
  }

  TEST_CASE("TextureMapperAnimation: samples its keyframes at the layer's times", "[UI][Compositing]")
  {
    TextureMapperAnimations animations;
    animations.Add(TextureMapperAnimation(u8"fade", Fade(), *LinearAnimation(), Start, {},
                                          TextureMapperAnimation::State::Playing));
    CHECK(OpacityAt(animations, Start) == 0);
    CHECK(OpacityAt(animations, Start + 250ms) == Catch::Approx(0.25));
    CHECK(OpacityAt(animations, Start + 500ms) == Catch::Approx(0.5));
    CHECK(animations.HasRunningAnimations());

    SECTION("it ends on its last keyframe")
    {
      CHECK(OpacityAt(animations, Start + 2s) == 1);
      CHECK_FALSE(animations.HasRunningAnimations());
    }

    SECTION("pausing holds it at an offset")
    {
      animations.Pause(u8"fade", web::Seconds {0.3});
      CHECK(OpacityAt(animations, Start + 900ms) == Catch::Approx(0.3));
      CHECK_FALSE(animations.HasRunningAnimations());
    }

    SECTION("a suspension holds it where it was, and it plays on from there")
    {
      (void)OpacityAt(animations, Start + 200ms);
      animations.Suspend(Start + 300ms);
      CHECK(OpacityAt(animations, Start + 800ms) == Catch::Approx(0.3));
      CHECK_FALSE(animations.HasRunningAnimations());

      // Resumed 500ms later, it's 100ms further on 100ms after that.
      animations.Resume(Start + 800ms);
      CHECK(OpacityAt(animations, Start + 900ms) == Catch::Approx(0.4));
      CHECK(animations.HasRunningAnimations());
    }

    SECTION("a paused animation stays paused through a suspension")
    {
      animations.Pause(u8"fade", web::Seconds {0.3});
      animations.Suspend(Start + 500ms);
      animations.Resume(Start + 600ms);
      CHECK(OpacityAt(animations, Start + 900ms) == Catch::Approx(0.3));
      CHECK_FALSE(animations.HasRunningAnimations());
    }

    SECTION("removing it leaves the layer's own values")
    {
      animations.Remove(u8"fade");
      TextureMapperAnimation::ApplicationResult result;
      animations.Apply(result, Start + 500ms);
      CHECK_FALSE(result.Opacity);
    }
  }

  TEST_CASE("TextureMapperAnimation: alternate iterations play backwards", "[UI][Compositing]")
  {
    TextureMapperAnimations animations;
    animations.Add(TextureMapperAnimation(
      u8"fade", Fade(), *LinearAnimation(2, web::GraphicsLayerAnimation::Direction::Alternate), Start, {},
      TextureMapperAnimation::State::Playing));
    CHECK(OpacityAt(animations, Start + 250ms) == Catch::Approx(0.25));
    CHECK(OpacityAt(animations, Start + 1'250ms) == Catch::Approx(0.75));
  }

  TEST_CASE("TextureMapperAnimation: transform animations combine into the layer's transform",
            "[UI][Compositing]")
  {
    web::GraphicsLayerKeyframeValueList keyframes(web::AnimatedProperty::Transform);
    keyframes.Insert(CreateUnique<web::GraphicsLayerTransformAnimationValue>(
      0, web::TransformOperations(
           web::ScaleTransformOperation::Create(1, 1, web::TransformOperation::Type::Scale))));
    keyframes.Insert(CreateUnique<web::GraphicsLayerTransformAnimationValue>(
      1, web::TransformOperations(
           web::ScaleTransformOperation::Create(3, 3, web::TransformOperation::Type::Scale))));

    TextureMapperAnimations animations;
    animations.Add(TextureMapperAnimation(u8"grow", keyframes, *LinearAnimation(), Start, {},
                                          TextureMapperAnimation::State::Playing));
    TextureMapperAnimation::ApplicationResult result;
    animations.Apply(result, Start + 500ms);
    REQUIRE(result.Transform);
    CHECK(result.Transform->A() == Catch::Approx(2));
    CHECK(result.Transform->D() == Catch::Approx(2));
    CHECK(animations.HasRunningTransformAnimations());
  }
}
