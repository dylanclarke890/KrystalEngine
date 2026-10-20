#include "Krystal.Booey/Animation/KeyframeInterpolation.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  KeyframeInterpolation::KeyframeInterval
    KeyframeInterpolation::InterpolationKeyframes(Property property, double iterationProgress,
                                                  const Keyframe &defaultStartKeyframe,
                                                  const Keyframe &defaultEndKeyframe) const noexcept
  {
    // 1. If iteration progress is unresolved abort this procedure.
    // 2. Let target property be the longhand property for which the effect value is to be calculated.
    // 3. If animation type of the target property is not animatable abort this procedure since the effect
    // cannot
    //    be applied.
    // 4. Define the neutral value for composition as a value which, when combined with an underlying value
    // using
    //    the add composite operation, produces the underlying value.
    // 5. Let property-specific keyframes be the result of getting the set of computed keyframes for this
    // keyframe
    //    effect.
    // 6. Remove any keyframes from property-specific keyframes that do not have a property value for target
    //    property.
    uint32 numberOfKeyframesWithZeroOffset = 0;
    uint32 numberOfKeyframesWithOneOffset = 0;
    List<const Keyframe *> propertySpecificKeyframes;
    for (size_t i = 0; i < NumberOfKeyframes(); ++i)
    {
      const auto &keyframe = KeyframeAtIndex(i);
      if (!keyframe.HasResolvedOffset() || !keyframe.AnimatesProperty(property))
      {
        continue;
      }
      auto offset = keyframe.Offset();
      if (!offset)
      {
        numberOfKeyframesWithZeroOffset++;
      }
      if (offset == 1)
      {
        numberOfKeyframesWithOneOffset++;
      }
      propertySpecificKeyframes.push_back(&keyframe);
    }

    // 7. If property-specific keyframes is empty, return underlying value.
    if (propertySpecificKeyframes.empty())
    {
      return {};
    }

    auto hasImplicitZeroKeyframe = !numberOfKeyframesWithZeroOffset;
    auto hasImplicitOneKeyframe = !numberOfKeyframesWithOneOffset;

    // 8. If there is no keyframe in property-specific keyframes with a computed keyframe offset of 0, create
    // a new
    //    keyframe with a computed keyframe offset of 0, a property value set to the neutral value for
    //    composition, and a composite operation of add, and prepend it to the beginning of property-specific
    //    keyframes.
    if (hasImplicitZeroKeyframe)
    {
      propertySpecificKeyframes.insert(propertySpecificKeyframes.begin(), &defaultStartKeyframe);
      numberOfKeyframesWithZeroOffset = 1;
    }

    // 9. Similarly, if there is no keyframe in property-specific keyframes with a computed keyframe offset of
    // 1,
    //    create a new keyframe with a computed keyframe offset of 1, a property value set to the neutral
    //    value for composition, and a composite operation of add, and append it to the end of
    //    property-specific keyframes.
    if (hasImplicitOneKeyframe)
    {
      propertySpecificKeyframes.push_back(&defaultEndKeyframe);
      numberOfKeyframesWithOneOffset = 1;
    }

    // 10. Let interval endpoints be an empty sequence of keyframes.
    List<const Keyframe *> intervalEndpoints;

    // 11. Populate interval endpoints by following the steps from the first matching condition from below:
    if (iterationProgress < 0 && numberOfKeyframesWithZeroOffset > 1)
    {
      // If iteration progress < 0 and there is more than one keyframe in property-specific keyframes with a
      // computed keyframe offset of 0, Add the first keyframe in property-specific keyframes to interval
      // endpoints.
      intervalEndpoints.push_back(propertySpecificKeyframes.front());
    }
    else if (iterationProgress >= 1 && numberOfKeyframesWithOneOffset > 1)
    {
      // If iteration progress ≥ 1 and there is more than one keyframe in property-specific keyframes with a
      // computed keyframe offset of 1, Add the last keyframe in property-specific keyframes to interval
      // endpoints.
      intervalEndpoints.push_back(propertySpecificKeyframes.back());
    }
    else
    {
      // Otherwise,
      // 1. Append to interval endpoints the last keyframe in property-specific keyframes whose computed
      // keyframe
      //    offset is less than or equal to iteration progress and less than 1. If there is no such keyframe
      //    (because, for example, the iteration progress is negative), add the last keyframe whose computed
      //    keyframe offset is 0.
      // 2. Append to interval endpoints the next keyframe in property-specific keyframes after the one added
      // in
      //    the previous step.
      size_t indexOfLastKeyframeWithZeroOffset = 0;
      int64 indexOfFirstKeyframeToAddToIntervalEndpoints = -1;
      for (size_t i = 0; i < propertySpecificKeyframes.size(); ++i)
      {
        auto offset = propertySpecificKeyframes[i]->Offset();
        if (!offset)
        {
          indexOfLastKeyframeWithZeroOffset = i;
        }
        if (offset <= iterationProgress && offset < 1)
        {
          indexOfFirstKeyframeToAddToIntervalEndpoints = static_cast<int64>(i);
        }
        else
        {
          break;
        }
      }

      if (indexOfFirstKeyframeToAddToIntervalEndpoints >= 0)
      {
        auto index = static_cast<size_t>(indexOfFirstKeyframeToAddToIntervalEndpoints);
        intervalEndpoints.push_back(propertySpecificKeyframes[index]);
        intervalEndpoints.push_back(propertySpecificKeyframes[index + 1]);
      }
      else
      {
        krys_debug_assert(indexOfLastKeyframeWithZeroOffset < propertySpecificKeyframes.size() - 1);
        intervalEndpoints.push_back(propertySpecificKeyframes[indexOfLastKeyframeWithZeroOffset]);
        intervalEndpoints.push_back(propertySpecificKeyframes[indexOfLastKeyframeWithZeroOffset + 1]);
      }
    }

    return {krys::move(intervalEndpoints), hasImplicitZeroKeyframe, hasImplicitOneKeyframe};
  }

  namespace
  {
    KRYS_NODISCARD double TransformProgressDuration(const WebAnimationTime &duration) noexcept
    {
      if (auto time = duration.Time())
      {
        return time->count();
      }
      return 1.0;
    }
  }

  void KeyframeInterpolation::InterpolateKeyframes(
    Property property, const KeyframeInterval &interval, double iterationProgress, double currentIteration,
    const WebAnimationTime &iterationDuration, TimingFunction::Before before,
    const CompositionCallback &compositionCallback, const AccumulationCallback &accumulationCallback,
    const InterpolationCallback &interpolationCallback,
    const RequiresInterpolationForAccumulativeIterationCallback
      &requiresInterpolationForAccumulativeIterationCallback) const noexcept
  {
    const auto &intervalEndpoints = interval.Endpoints;
    if (intervalEndpoints.empty())
    {
      return;
    }

    const auto &startKeyframe = *intervalEndpoints.front();
    const auto &endKeyframe = *intervalEndpoints.back();

    auto usedInterpolationForAccumulativeIteration = false;

    // 12. For each keyframe in interval endpoints:
    //     If keyframe has a composite operation that is not replace, or keyframe has no composite operation
    //     and the composite operation of this keyframe effect is not replace, then perform the following
    //     steps:
    //         1. Let composite operation to use be the composite operation of keyframe, or if it has none,
    //         the
    //            composite operation of this keyframe effect.
    //         2. Let value to combine be the property value of target property specified on keyframe.
    //         3. Replace the property value of target property on keyframe with the result of combining
    //            underlying value (Va) and value to combine (Vb) using the procedure for the composite
    //            operation to use corresponding to the target property’s animation type.
    if (IsPropertyAdditiveOrCumulative(property))
    {
      // Only do this for the 0 keyframe if it was provided explicitly, since otherwise we want to use the
      // "neutral value for composition" which really means we don't want to do anything but rather just use
      // the underlying style which is already set on startKeyframe.
      if (startKeyframe.Offset() || !interval.HasImplicitZeroKeyframe)
      {
        auto startKeyframeCompositeOperation =
          startKeyframe.CompositeOperation().value_or(CompositeOperation());
        if (startKeyframeCompositeOperation != boo::CompositeOperation::Replace)
        {
          compositionCallback(startKeyframe, startKeyframeCompositeOperation);
        }
      }

      // Only do this for the 1 keyframe if it was provided explicitly, since otherwise we want to use the
      // "neutral value for composition" which really means we don't want to do anything but rather just use
      // the underlying style which is already set on endKeyframe.
      if (endKeyframe.Offset() != 1 || !interval.HasImplicitOneKeyframe)
      {
        auto endKeyframeCompositeOperation = endKeyframe.CompositeOperation().value_or(CompositeOperation());
        if (endKeyframeCompositeOperation != boo::CompositeOperation::Replace)
        {
          compositionCallback(endKeyframe, endKeyframeCompositeOperation);
        }
      }

      // If this keyframe effect has an iteration composite operation of accumulate,
      if (IterationCompositeOperation() == boo::IterationCompositeOperation::Accumulate && currentIteration
          && requiresInterpolationForAccumulativeIterationCallback())
      {
        usedInterpolationForAccumulativeIteration = true;
        // apply the following step current iteration times:
        for (auto i = 0; i < currentIteration; ++i)
        {
          // replace the property value of target property on keyframe with the result of combining the
          // property value on the final keyframe in property-specific keyframes (Va) with the property
          // value on keyframe (Vb) using the accumulation procedure defined for target property.
          if (!startKeyframe.Offset() && !interval.HasImplicitZeroKeyframe)
          {
            accumulationCallback(startKeyframe);
          }
          if (endKeyframe.Offset() == 1 && !interval.HasImplicitOneKeyframe)
          {
            accumulationCallback(endKeyframe);
          }
        }
      }
    }

    // 13. If there is only one keyframe in interval endpoints return the property value of target property on
    // that
    //     keyframe.
    if (intervalEndpoints.size() == 1)
    {
      interpolationCallback(0, 1, boo::IterationCompositeOperation::Replace);
      return;
    }

    // 14. Let start offset be the computed keyframe offset of the first keyframe in interval endpoints.
    auto startOffset = startKeyframe.Offset();

    // 15. Let end offset be the computed keyframe offset of last keyframe in interval endpoints.
    auto endOffset = endKeyframe.Offset();

    // 16. Let interval distance be the result of evaluating (iteration progress - start offset) / (end offset
    // -
    //     start offset).
    auto intervalDistance = (iterationProgress - startOffset) / (endOffset - startOffset);

    // 17. Let transformed distance be the result of evaluating the timing function associated with the first
    //     keyframe in interval endpoints passing interval distance as the input progress.
    auto transformedDistance = intervalDistance;
    if (!iterationDuration.IsZero())
    {
      auto rangeDuration = (endOffset - startOffset) * TransformProgressDuration(iterationDuration);
      if (const auto *timingFunction = TimingFunctionForKeyframe(startKeyframe))
      {
        transformedDistance = timingFunction->TransformProgress(intervalDistance, rangeDuration, before);
      }
    }

    // 18. Return the result of applying the interpolation procedure defined by the animation type of the
    // target
    //     property, to the values of the target property specified on the two keyframes in interval endpoints
    //     taking the first such value as Vstart and the second as Vend and using transformed distance as the
    //     interpolation parameter p.
    auto iterationCompositeOperation = usedInterpolationForAccumulativeIteration
                                         ? boo::IterationCompositeOperation::Replace
                                         : IterationCompositeOperation();
    currentIteration = usedInterpolationForAccumulativeIteration ? 0 : currentIteration;
    interpolationCallback(transformedDistance, currentIteration, iterationCompositeOperation);
  }
}
