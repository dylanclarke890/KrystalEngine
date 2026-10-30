#include "Krystal.UI/Compositing/TextureMapperAnimation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFloatAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerTransformAnimationValue.hpp"
#include "Krystal.Booey/Graphics/Transforms/ScaleTransformOperation.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperations.hpp"
#include <chrono>

namespace krys::ui::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    constexpr MonotonicTime::duration Start = std::chrono::duration_cast<MonotonicTime::duration>(10s);

    /// @brief A linear one second animation, @p iterations times in @p direction.
    KRYS_NODISCARD Ref<boo::GraphicsLayerAnimation>
      LinearAnimation(double iterations = 1, boo::GraphicsLayerAnimation::Direction direction =
                                               boo::GraphicsLayerAnimation::Direction::Normal) noexcept
    {
      auto animation = boo::GraphicsLayerAnimation::Create();
      animation->SetDuration(1.0);
      animation->SetIterationCount(iterations);
      animation->SetDirection(direction);
      animation->SetTimingFunction(boo::LinearTimingFunction::Create());
      return animation;
    }

    /// @brief Opacity from 0 to 1.
    KRYS_NODISCARD boo::GraphicsLayerKeyframeValueList Fade() noexcept
    {
      boo::GraphicsLayerKeyframeValueList keyframes(boo::AnimatedProperty::Opacity);
      keyframes.Insert(CreateUnique<boo::GraphicsLayerFloatAnimationValue>(0, 0.f));
      keyframes.Insert(CreateUnique<boo::GraphicsLayerFloatAnimationValue>(1, 1.f));
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
      animations.Pause(u8"fade", boo::Seconds {0.3});
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
      u8"fade", Fade(), *LinearAnimation(2, boo::GraphicsLayerAnimation::Direction::Alternate), Start, {},
      TextureMapperAnimation::State::Playing));
    CHECK(OpacityAt(animations, Start + 250ms) == Catch::Approx(0.25));
    CHECK(OpacityAt(animations, Start + 1'250ms) == Catch::Approx(0.75));
  }

  TEST_CASE("TextureMapperAnimation: transform animations combine into the layer's transform",
            "[UI][Compositing]")
  {
    boo::GraphicsLayerKeyframeValueList keyframes(boo::AnimatedProperty::Transform);
    keyframes.Insert(CreateUnique<boo::GraphicsLayerTransformAnimationValue>(
      0, boo::TransformOperations(
           boo::ScaleTransformOperation::Create(1, 1, boo::TransformOperation::Type::Scale))));
    keyframes.Insert(CreateUnique<boo::GraphicsLayerTransformAnimationValue>(
      1, boo::TransformOperations(
           boo::ScaleTransformOperation::Create(3, 3, boo::TransformOperation::Type::Scale))));

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
