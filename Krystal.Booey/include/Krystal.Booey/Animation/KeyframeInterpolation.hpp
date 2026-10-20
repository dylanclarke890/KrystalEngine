#pragma once

#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/IterationCompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <cmath>

namespace krys::boo
{
  /// @brief Interpolates a property between the keyframes of an effect, as Web Animations' effect value
  /// procedure does.
  /// @see https://drafts.csswg.org/web-animations-1/#the-effect-value-of-a-keyframe-animation-effect
  class KeyframeInterpolation
  {
  public:
    using Property = Variant<AnimatableCSSProperty, AcceleratedEffectProperty>;

    /// @brief A keyframe of the effect.
    class Keyframe
    {
    public:
      virtual ~Keyframe() noexcept = default;

      KRYS_NODISCARD bool HasResolvedOffset() const noexcept
      {
        return !std::isnan(Offset());
      }
      KRYS_NODISCARD virtual double Offset() const noexcept = 0;
      KRYS_NODISCARD virtual Maybe<boo::CompositeOperation> CompositeOperation() const noexcept = 0;
      KRYS_NODISCARD virtual bool AnimatesProperty(Property) const noexcept = 0;
      KRYS_NODISCARD virtual bool IsAcceleratedEffectKeyframe() const noexcept
      {
        return false;
      }
      KRYS_NODISCARD virtual bool IsBlendingKeyframe() const noexcept
      {
        return false;
      }
    };

    struct KeyframeInterval
    {
      List<const Keyframe *> Endpoints;
      bool HasImplicitZeroKeyframe {false};
      bool HasImplicitOneKeyframe {false};
    };

    using CompositionCallback = Func<void(const Keyframe &, boo::CompositeOperation)>;
    using AccumulationCallback = Func<void(const Keyframe &)>;
    using InterpolationCallback =
      Func<void(double intervalProgress, double currentIteration, boo::IterationCompositeOperation)>;
    using RequiresInterpolationForAccumulativeIterationCallback = Func<bool()>;

    virtual ~KeyframeInterpolation() noexcept = default;

    KRYS_NODISCARD virtual boo::CompositeOperation CompositeOperation() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsPropertyAdditiveOrCumulative(Property) const noexcept = 0;
    KRYS_NODISCARD virtual boo::IterationCompositeOperation IterationCompositeOperation() const noexcept
    {
      return boo::IterationCompositeOperation::Replace;
    }
    KRYS_NODISCARD virtual const Keyframe &KeyframeAtIndex(size_t) const noexcept = 0;
    KRYS_NODISCARD virtual size_t NumberOfKeyframes() const noexcept = 0;
    KRYS_NODISCARD virtual const TimingFunction *
      TimingFunctionForKeyframe(const Keyframe &) const noexcept = 0;

    /// @brief The keyframes of the interval @p iterationProgress is in, for @p property, with the default
    /// start and end keyframes standing in for implicit 0% and 100% keyframes.
    KRYS_NODISCARD KeyframeInterval InterpolationKeyframes(Property property, double iterationProgress,
                                                           const Keyframe &defaultStartKeyframe,
                                                           const Keyframe &defaultEndKeyframe) const noexcept;

    /// @brief Composes, accumulates and interpolates @p property over @p interval through the callbacks.
    void InterpolateKeyframes(Property property, const KeyframeInterval &interval, double iterationProgress,
                              double currentIteration, const WebAnimationTime &iterationDuration,
                              TimingFunction::Before before, const CompositionCallback &compositionCallback,
                              const AccumulationCallback &accumulationCallback,
                              const InterpolationCallback &interpolationCallback,
                              const RequiresInterpolationForAccumulativeIterationCallback
                                &requiresInterpolationForAccumulativeIterationCallback) const noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_KEYFRAME_INTERPOLATION_KEYFRAME(ToClassName, Predicate)                  \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToClassName)                                             \
    KRYS_NODISCARD static bool IsType(const krys::boo::KeyframeInterpolation::Keyframe &value) noexcept      \
    {                                                                                                        \
      return value.Predicate;                                                                                \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
