/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Animation/Timing/WebAnimationTime.hpp"
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
    using Property = AnimatableCSSProperty;

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
