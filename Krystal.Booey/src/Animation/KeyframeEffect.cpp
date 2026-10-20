#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Interpolation.hpp"
#include "Krystal.Booey/Style/PendingResources.hpp"
#include "Krystal.Booey/Style/PropertyAllowlist.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/Values/WillChange/WillChange.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    void InvalidateElement(const Maybe<style::Styleable> &styleable) noexcept
    {
      if (!styleable)
      {
        return;
      }

      Ref element = styleable->ProtectedElement();
      if (!element->NodeDocument().InStyleRecalc())
      {
        element->InvalidateStyleForAnimation();
      }
    }

    KRYS_NODISCARD bool IsLinearTimingFunctionWithPoints(const TimingFunction *timingFunction) noexcept
    {
      auto *linearTimingFunction = DynamicDowncast<LinearTimingFunction>(timingFunction);
      return linearTimingFunction && !linearTimingFunction->Points().empty();
    }
  }

  KeyframeEffect::~KeyframeEffect() noexcept
  {
    if (_inTargetEffectStack)
    {
      if (auto target = TargetStyleable())
      {
        if (auto *keyframeEffectStack = target->GetKeyframeEffectStack())
        {
          keyframeEffectStack->RemoveEffect(*this);
        }
      }
    }

    krys_debug_assert(!_inTargetEffectStack);
  }

  Ref<KeyframeEffect>
    KeyframeEffect::Create(const dom::Element &target,
                           const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return AdoptRef(*new KeyframeEffect(const_cast<dom::Element *>(&target), pseudoElementIdentifier));
  }

  KeyframeEffect::KeyframeEffect(
    dom::Element *target, const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
      : _target(ShareRefPtr(target)), _pseudoElementIdentifier(pseudoElementIdentifier)
  {
    if (_target)
    {
      _document = CreateWeakPtr(&_target->NodeDocument());
    }
  }

  const AnimatableCSSPropertySet &KeyframeEffect::AnimatedProperties() noexcept
  {
    if (!_blendingKeyframes.IsEmpty())
    {
      return _blendingKeyframes.Properties();
    }

    // FIXME: The properties of the parsed keyframes, once there are keyframes from scripts.
    return _animatedProperties;
  }

  bool KeyframeEffect::AnimatesProperty(const AnimatableCSSProperty &property) const noexcept
  {
    if (!_blendingKeyframes.IsEmpty())
    {
      return _blendingKeyframes.ContainsProperty(property);
    }

    // FIXME: The properties of the parsed keyframes, once there are keyframes from scripts.
    return false;
  }

  void KeyframeEffect::ClearBlendingKeyframes() noexcept
  {
    _animationType = WebAnimationType::WebAnimation;
    _blendingKeyframes.Clear();
  }

  void KeyframeEffect::SetBlendingKeyframes(BlendingKeyframes &&blendingKeyframes) noexcept
  {
    _blendingKeyframes = krys::move(blendingKeyframes);
    _animatedProperties.clear();

    ComputeStackingContextImpact();
    ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints();
  }

  Maybe<uint32> KeyframeEffect::TransformFunctionListPrefix() const noexcept
  {
    // FIXME: The prefix of transform function primitives the keyframes share, for interpolation that matches
    // accelerated animations, once there are accelerated animations.
    return null;
  }

  void KeyframeEffect::ComputeStyleOriginatedAnimationBlendingKeyframes(
    const RenderStyle *oldStyle, const RenderStyle &newStyle, const style::ResolutionContext &) noexcept
  {
    krys_debug_assert(Animation() && Is<StyleOriginatedAnimation>(*Animation()));

    // FIXME: The keyframes of CSS animations, once there are CSS animations.
    if (Animation() && Is<CSSTransition>(*Animation()))
    {
      krys_debug_assert(oldStyle);
      ComputeCSSTransitionBlendingKeyframes(*oldStyle, newStyle);
    }
  }

  void KeyframeEffect::ComputeCSSTransitionBlendingKeyframes(const RenderStyle &oldStyle,
                                                             const RenderStyle &newStyle) noexcept
  {
    krys_debug_assert(Document());

    if (_blendingKeyframes.Size())
    {
      return;
    }

    auto property = DynamicDowncast<CSSTransition>(Animation())->Property();

    auto toStyle = RenderStyle::ClonePtr(newStyle);
    if (_target)
    {
      style::LoadPendingResources(*toStyle, *Document(), _target.get());
    }

    BlendingKeyframes blendingKeyframes(_blendingKeyframes.Identifier());

    BlendingKeyframe fromBlendingKeyframe(0, RenderStyle::ClonePtr(oldStyle));
    fromBlendingKeyframe.AddProperty(property);
    blendingKeyframes.Insert(krys::move(fromBlendingKeyframe));

    BlendingKeyframe toBlendingKeyframe(1, krys::move(toStyle));
    toBlendingKeyframe.AddProperty(property);
    blendingKeyframes.Insert(krys::move(toBlendingKeyframe));

    _animationType = WebAnimationType::CSSTransition;
    SetBlendingKeyframes(krys::move(blendingKeyframes));
  }

  void KeyframeEffect::ComputeStackingContextImpact() noexcept
  {
    _triggersStackingContext = false;
    for (const auto &property : _blendingKeyframes.Properties())
    {
      if (const auto *propertyId = std::get_if<css::PropertyId>(&property);
          propertyId && style::WillChangeAnimatableFeature::PropertyCreatesStackingContext(*propertyId))
      {
        _triggersStackingContext = true;
        break;
      }
    }
  }

  void KeyframeEffect::AnimationTimelineDidChange() noexcept
  {
    AnimationEffect::AnimationTimelineDidChange();
    UpdateEffectStackMembership();
  }

  void KeyframeEffect::AnimationRelevancyDidChange() noexcept
  {
    UpdateEffectStackMembership();
  }

  void KeyframeEffect::UpdateEffectStackMembership() noexcept
  {
    auto target = TargetStyleable();
    if (!target)
    {
      return;
    }

    bool isRelevant = Animation() && Animation()->IsRelevant();
    if (isRelevant && !_inTargetEffectStack)
    {
      (void)target->EnsureKeyframeEffectStack().AddEffect(*this);
    }
    else if (!isRelevant && _inTargetEffectStack)
    {
      target->EnsureKeyframeEffectStack().RemoveEffect(*this);
    }
  }

  void KeyframeEffect::SetAnimation(WebAnimation *animation) noexcept
  {
    bool animationChanged = animation != Animation();
    AnimationEffect::SetAnimation(animation);
    if (!animationChanged)
    {
      return;
    }

    if (_animationType == WebAnimationType::CSSAnimation)
    {
      ClearBlendingKeyframes();
    }

    UpdateEffectStackMembership();
  }

  Maybe<style::Styleable> KeyframeEffect::TargetStyleable() const noexcept
  {
    if (_target)
    {
      return style::Styleable(*_target, _pseudoElementIdentifier);
    }
    return null;
  }

  bool KeyframeEffect::TargetsPseudoElement() const noexcept
  {
    return _target && _pseudoElementIdentifier;
  }

  void KeyframeEffect::SetTarget(RefPtr<dom::Element> &&newTarget) noexcept
  {
    if (_target == newTarget)
    {
      return;
    }

    auto previousTargetStyleable = TargetStyleable();
    RefPtr<dom::Element> protector;
    if (previousTargetStyleable)
    {
      protector = previousTargetStyleable->ProtectedElement();
    }

    _target = krys::move(newTarget);
    DidChangeTargetStyleable(previousTargetStyleable);
  }

  css::CSSOMString KeyframeEffect::PseudoElement() const noexcept
  {
    // https://drafts.csswg.org/web-animations/#dom-keyframeeffect-pseudoelement
    // The target pseudo-selector. null if this effect has no effect target or if the effect target is an
    // element (i.e. not a pseudo-element). When the effect target is a pseudo-element, this specifies the
    // pseudo-element selector (e.g. ::before).
    if (TargetsPseudoElement())
    {
      return PseudoElementIdentifierAsString(_pseudoElementIdentifier);
    }
    return {};
  }

  void
    KeyframeEffect::DidChangeTargetStyleable(const Maybe<style::Styleable> &previousTargetStyleable) noexcept
  {
    auto newTargetStyleable = TargetStyleable();

    if (RefPtr effectAnimation = ShareRefPtr(Animation()))
    {
      effectAnimation->EffectTargetDidChange(previousTargetStyleable, newTargetStyleable);
    }

    ClearBlendingKeyframes();

    // We need to invalidate the effect now that the target has changed to ensure the effect's styles are
    // applied to the new target right away.
    Invalidate();

    // Likewise, we need to invalidate styles on the previous target so that any animated styles are removed
    // immediately.
    InvalidateElement(previousTargetStyleable);

    if (previousTargetStyleable)
    {
      previousTargetStyleable->EnsureKeyframeEffectStack().RemoveEffect(*this);
    }
    if (newTargetStyleable)
    {
      (void)newTargetStyleable->EnsureKeyframeEffectStack().AddEffect(*this);
    }
  }

  EnumSet<AnimationImpact>
    KeyframeEffect::Apply(RenderStyle &targetStyle, const style::ResolutionContext &,
                          EndpointInclusiveActiveInterval endpointInclusiveActiveInterval) noexcept
  {
    EnumSet<AnimationImpact> impact;
    if (!_target)
    {
      return impact;
    }

    // FIXME: Resolving the parsed keyframes into blending keyframes, once there are keyframes from scripts.

    auto computedTiming = GetComputedTiming(UseCachedCurrentTime::Yes, endpointInclusiveActiveInterval);
    if (_phaseAtLastApplication != computedTiming.Phase)
    {
      _phaseAtLastApplication = computedTiming.Phase;
      impact.Add(AnimationImpact::RequiresRecomposite);
    }

    if (!computedTiming.Progress)
    {
      return impact;
    }

    krys_debug_assert(computedTiming.CurrentIteration);
    SetAnimatedPropertiesInStyle(targetStyle, computedTiming);
    return impact;
  }

  bool KeyframeEffect::IsCurrentlyAffectingProperty(css::PropertyId property) const noexcept
  {
    if (!_blendingKeyframes.Properties().contains(property))
    {
      return false;
    }

    if (_pseudoElementIdentifier && _pseudoElementIdentifier->Type == PseudoElementType::Marker
        && !style::IsValidMarkerStyleProperty(property))
    {
      return false;
    }

    return _phaseAtLastApplication == AnimationEffectPhase::Active;
  }

  void KeyframeEffect::Invalidate() noexcept
  {
    InvalidateElement(TargetStyleable());
  }

  void KeyframeEffect::ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints() noexcept
  {
    _someKeyframesUseStepsTimingFunction = false;
    _someKeyframesUseLinearTimingFunctionWithPoints = false;

    // FIXME: Accounting for the default steps() or linear() timing function of CSS animations, once there are
    // CSS animations.

    // For any other type of animation, we just need to check whether any of the keyframes specify an explicit
    // steps() or linear() timing function.
    for (const auto &keyframe : _blendingKeyframes)
    {
      auto *timingFunction = keyframe.GetTimingFunction();
      if (!_someKeyframesUseStepsTimingFunction && timingFunction && Is<StepsTimingFunction>(*timingFunction))
      {
        _someKeyframesUseStepsTimingFunction = true;
      }
      if (!_someKeyframesUseLinearTimingFunctionWithPoints
          && IsLinearTimingFunctionWithPoints(timingFunction))
      {
        _someKeyframesUseLinearTimingFunctionWithPoints = true;
      }
      if (_someKeyframesUseStepsTimingFunction && _someKeyframesUseLinearTimingFunctionWithPoints)
      {
        return;
      }
    }
  }

  void KeyframeEffect::GetAnimatedStyle(UniquePtr<RenderStyle> &animatedStyle) noexcept
  {
    if (!Renderer() || !Animation())
    {
      return;
    }

    auto computedTiming = GetComputedTiming();
    if (!computedTiming.Progress)
    {
      return;
    }

    if (!animatedStyle)
    {
      if (auto *style = TargetStyleable()->LastStyleChangeEventStyle())
      {
        animatedStyle = RenderStyle::ClonePtr(*style);
      }
      else
      {
        animatedStyle = RenderStyle::ClonePtr(Renderer()->Style());
      }
    }

    krys_debug_assert(computedTiming.CurrentIteration);
    SetAnimatedPropertiesInStyle(*animatedStyle, computedTiming);
  }

  void KeyframeEffect::SetAnimatedPropertiesInStyle(RenderStyle &targetStyle,
                                                    const ComputedEffectTiming &computedTiming) const noexcept
  {
    krys_debug_assert(computedTiming.Progress);
    krys_debug_assert(computedTiming.CurrentIteration);

    auto iterationProgress = *computedTiming.Progress;
    auto currentIteration = *computedTiming.CurrentIteration;
    auto before = computedTiming.Before;

    const auto &properties = _blendingKeyframes.Properties();

    // In the case of CSS Transitions we already know that there are only two keyframes, one where offset=0
    // and one where offset=1, and only a single CSS property so we can simply blend based on the style
    // available on those keyframes with the provided iteration progress which already accounts for the
    // transition's timing function.
    if (_animationType == WebAnimationType::CSSTransition)
    {
      krys_debug_assert(properties.size() == 1);
      style::interpolation::Interpolate(*properties.begin(), targetStyle, *_blendingKeyframes[0].Style(),
                                        *_blendingKeyframes[1].Style(), iterationProgress,
                                        _compositeOperation, *this);
      return;
    }

    // 4.4.3. The effect value of a keyframe effect
    // https://drafts.csswg.org/web-animations-1/#the-effect-value-of-a-keyframe-animation-effect
    //
    // The effect value of a single property referenced by a keyframe effect as one of its target properties,
    // for a given iteration progress, current iteration and underlying value is calculated as follows.

    if (_blendingKeyframes.IsEmpty())
    {
      return;
    }

    BlendingKeyframe propertySpecificKeyframeWithZeroOffset(0, RenderStyle::ClonePtr(targetStyle));
    BlendingKeyframe propertySpecificKeyframeWithOneOffset(1, RenderStyle::ClonePtr(targetStyle));

    for (const auto &property : properties)
    {
      auto interval =
        InterpolationKeyframes(property, iterationProgress, propertySpecificKeyframeWithZeroOffset,
                               propertySpecificKeyframeWithOneOffset);
      if (interval.Endpoints.empty())
      {
        continue;
      }

      auto *startBlendingKeyframe = DynamicDowncast<BlendingKeyframe>(interval.Endpoints.front());
      auto *endBlendingKeyframe = DynamicDowncast<BlendingKeyframe>(interval.Endpoints.back());
      if (!startBlendingKeyframe || !endBlendingKeyframe)
      {
        krys_debug_assert(false);
        continue;
      }

      auto startKeyframeStyle = RenderStyle::Clone(*startBlendingKeyframe->Style());
      auto endKeyframeStyle = RenderStyle::Clone(*endBlendingKeyframe->Style());

      KeyframeInterpolation::CompositionCallback composeProperty =
        [&](const KeyframeInterpolation::Keyframe &keyframe, boo::CompositeOperation compositeOperation)
      {
        auto *blendingKeyframe = DynamicDowncast<BlendingKeyframe>(&keyframe);
        if (!blendingKeyframe)
        {
          krys_debug_assert(false);
          return;
        }

        if (blendingKeyframe->Offset() == startBlendingKeyframe->Offset())
        {
          style::interpolation::Interpolate(property, startKeyframeStyle, targetStyle,
                                            *blendingKeyframe->Style(), 1, compositeOperation, *this);
        }
        else
        {
          style::interpolation::Interpolate(property, endKeyframeStyle, targetStyle,
                                            *blendingKeyframe->Style(), 1, compositeOperation, *this);
        }
      };

      KeyframeInterpolation::AccumulationCallback accumulateProperty =
        [&](const KeyframeInterpolation::Keyframe &keyframe)
      {
        auto *blendingKeyframe = DynamicDowncast<BlendingKeyframe>(&keyframe);
        if (!blendingKeyframe)
        {
          krys_debug_assert(false);
          return;
        }

        if (blendingKeyframe->Offset() == startBlendingKeyframe->Offset())
        {
          style::interpolation::Interpolate(property, startKeyframeStyle, *endBlendingKeyframe->Style(),
                                            startKeyframeStyle, 1, boo::CompositeOperation::Accumulate,
                                            *this);
        }
        else
        {
          style::interpolation::Interpolate(property, endKeyframeStyle, *endBlendingKeyframe->Style(),
                                            endKeyframeStyle, 1, boo::CompositeOperation::Accumulate, *this);
        }
      };

      KeyframeInterpolation::InterpolationCallback interpolateProperty =
        [&](double intervalProgress, double currentIteration,
            boo::IterationCompositeOperation iterationCompositeOperation)
      {
        style::interpolation::Interpolate(property, targetStyle, startKeyframeStyle, endKeyframeStyle,
                                          intervalProgress, boo::CompositeOperation::Replace,
                                          iterationCompositeOperation, currentIteration, *this);
      };

      KeyframeInterpolation::RequiresInterpolationForAccumulativeIterationCallback
        requiresInterpolationForAccumulativeIterationCallback = [&]()
      {
        return style::interpolation::RequiresInterpolationForAccumulativeIteration(
          property, startKeyframeStyle, endKeyframeStyle, *this);
      };

      InterpolateKeyframes(property, interval, iterationProgress, currentIteration, IterationDuration(),
                           before, composeProperty, accumulateProperty, interpolateProperty,
                           requiresInterpolationForAccumulativeIterationCallback);
    }

    // In case one of the animated properties has its value set to "inherit" in one of the keyframes, let's
    // mark the resulting animated style as having an explicitly inherited property such that a future style
    // update accounts for this in a future call to TreeResolver::DetermineResolutionType().
    if (_blendingKeyframes.HasExplicitlyInheritedKeyframeProperty())
    {
      targetStyle.SetHasExplicitlyInheritedProperties();
    }
  }

  const TimingFunction *
    KeyframeEffect::TimingFunctionForBlendingKeyframe(const BlendingKeyframe &keyframe) const noexcept
  {
    if (auto *styleOriginatedAnimation = DynamicDowncast<StyleOriginatedAnimation>(Animation()))
    {
      // If we're dealing with a CSS Animation, the timing function is specified either on the keyframe
      // itself.
      if (styleOriginatedAnimation->IsCSSAnimation())
      {
        if (auto *timingFunction = keyframe.GetTimingFunction())
        {
          return timingFunction;
        }
      }

      // Failing that, or for a CSS Transition, the timing function is inherited from the backing Animation
      // object.
      return styleOriginatedAnimation->BackingAnimationTimingFunction();
    }

    return keyframe.GetTimingFunction();
  }

  const TimingFunction *KeyframeEffect::TimingFunctionForKeyframeAtIndex(size_t index) const noexcept
  {
    // FIXME: The timing functions of the parsed keyframes, once there are keyframes from scripts.
    if (index >= _blendingKeyframes.Size())
    {
      return nullptr;
    }

    return TimingFunctionForBlendingKeyframe(_blendingKeyframes[index]);
  }

  void KeyframeEffect::AnimationDidTick() noexcept
  {
    Invalidate();
  }

  void KeyframeEffect::AnimationDidChangeTimingProperties() noexcept
  {
    ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints();
    Invalidate();
  }

  Maybe<KeyframeEffect::RecomputationReason>
    KeyframeEffect::RecomputeKeyframesIfNecessary(const RenderStyle *previousUnanimatedStyle,
                                                  const RenderStyle &unanimatedStyle,
                                                  const style::ResolutionContext &) noexcept
  {
    if (_animationType == WebAnimationType::CSSTransition)
    {
      return null;
    }

    auto fontSizeChanged = [&]()
    {
      return previousUnanimatedStyle
             && previousUnanimatedStyle->ComputedFontSize() != unanimatedStyle.ComputedFontSize();
    };

    auto fontWeightChanged = [&]()
    {
      return _blendingKeyframes.UsesRelativeFontWeight() && previousUnanimatedStyle
             && previousUnanimatedStyle->GetFontWeight() != unanimatedStyle.GetFontWeight();
    };

    auto cssVariableChanged = [&]()
    {
      if (previousUnanimatedStyle && _blendingKeyframes.HasCSSVariableReferences())
      {
        if (!previousUnanimatedStyle->CustomPropertiesEqual(unanimatedStyle))
        {
          return true;
        }
      }
      return false;
    };

    auto hasPropertyExplicitlySetToInherit = [&]()
    {
      return !_blendingKeyframes.PropertiesSetToInherit().empty();
    };

    auto propertySetToCurrentColorChanged = [&]()
    {
      // If the "color" property itself is set to "currentcolor" on a keyframe, we always recompute keyframes.
      if (_blendingKeyframes.HasColorSetToCurrentColor())
      {
        return true;
      }
      // For all other color-related properties set to "currentcolor" on a keyframe, it's sufficient to check
      // whether the value "color" resolves to has changed since the last style resolution.
      return _blendingKeyframes.HasPropertySetToCurrentColor() && previousUnanimatedStyle
             && previousUnanimatedStyle->GetColor() != unanimatedStyle.GetColor();
    };

    auto logicalPropertyChanged = [&]()
    {
      if (!previousUnanimatedStyle)
      {
        return false;
      }
      if (previousUnanimatedStyle->GetWritingMode() == unanimatedStyle.GetWritingMode())
      {
        return false;
      }
      // FIXME: The direction-aware properties of the parsed keyframes, once there are keyframes from scripts.
      return !_blendingKeyframes.IsEmpty() && _blendingKeyframes.ContainsDirectionAwareProperty();
    }();

    auto usesAnchorFunctions = _blendingKeyframes.UsesAnchorFunctions();

    if (logicalPropertyChanged || fontSizeChanged() || fontWeightChanged() || cssVariableChanged()
        || hasPropertyExplicitlySetToInherit() || propertySetToCurrentColorChanged() || usesAnchorFunctions)
    {
      switch (_animationType)
      {
        case WebAnimationType::CSSTransition: krys_debug_assert(false); break;
        case WebAnimationType::CSSAnimation:
          // FIXME: Recomputing the keyframes of CSS animations, once there are CSS animations.
          break;
        case WebAnimationType::WebAnimation: ClearBlendingKeyframes(); break;
      }
      return logicalPropertyChanged ? RecomputationReason::LogicalPropertyChange : RecomputationReason::Other;
    }

    return null;
  }

  void KeyframeEffect::WasAddedToEffectStack() noexcept
  {
    _inTargetEffectStack = true;
    Invalidate();
  }

  void KeyframeEffect::WasRemovedFromEffectStack() noexcept
  {
    _inTargetEffectStack = false;
  }

  void KeyframeEffect::WillChangeRenderer() noexcept
  {
    // FIXME: Stopping accelerated animations, once there are accelerated animations.
  }

  dom::Document *KeyframeEffect::Document() const noexcept
  {
    if (_document)
    {
      return _document.get();
    }
    return _target ? &_target->NodeDocument() : nullptr;
  }

  RenderElement *KeyframeEffect::Renderer() const noexcept
  {
    if (auto target = TargetStyleable())
    {
      return target->Renderer();
    }
    return nullptr;
  }

  const RenderStyle &KeyframeEffect::CurrentStyle() const noexcept
  {
    if (auto *renderer = Renderer())
    {
      return renderer->Style();
    }
    return RenderStyle::DefaultStyleSingleton();
  }

  bool KeyframeEffect::RequiresPseudoElement() const noexcept
  {
    return _animationType == WebAnimationType::WebAnimation && TargetsPseudoElement();
  }

  Maybe<double> KeyframeEffect::ProgressUntilNextStep(double iterationProgress) const noexcept
  {
    krys_debug_assert(iterationProgress >= 0 && iterationProgress <= 1);

    if (auto progress = AnimationEffect::ProgressUntilNextStep(iterationProgress))
    {
      return progress;
    }

    if (!TimingFunction() || !Is<LinearTimingFunction>(*TimingFunction())
        || !_someKeyframesUseStepsTimingFunction)
    {
      return null;
    }

    if (_blendingKeyframes.IsEmpty())
    {
      return null;
    }

    auto progressUntilNextStepInInterval =
      [iterationProgress](double intervalStartProgress, double intervalEndProgress,
                          const boo::TimingFunction *timingFunction) -> Maybe<double>
    {
      auto *stepsTimingFunction = DynamicDowncast<StepsTimingFunction>(timingFunction);
      if (!stepsTimingFunction)
      {
        return null;
      }

      auto numberOfSteps = stepsTimingFunction->NumberOfSteps();
      auto intervalProgress = intervalEndProgress - intervalStartProgress;
      auto iterationProgressMappedToCurrentInterval =
        (iterationProgress - intervalStartProgress) / intervalProgress;
      auto nextStepProgress =
        std::ceil(iterationProgressMappedToCurrentInterval * numberOfSteps) / numberOfSteps;
      return (nextStepProgress - iterationProgressMappedToCurrentInterval) * intervalProgress;
    };

    for (size_t i = 0; i < _blendingKeyframes.Size(); ++i)
    {
      auto intervalEndProgress = _blendingKeyframes[i].Offset();
      // We can stop once we find a keyframe for which the progress is more than the provided iteration
      // progress.
      if (intervalEndProgress <= iterationProgress)
      {
        continue;
      }

      // In case we're on the first keyframe, then this means we are dealing with an implicit 0% keyframe.
      // This will be a linear timing function unless we're dealing with a CSS Animation which might have the
      // default timing function for its keyframes defined on its backing Animation object.
      // FIXME: The default timing function of CSS animations, once there are CSS animations.
      if (!i)
      {
        return null;
      }

      return progressUntilNextStepInInterval(_blendingKeyframes[i - 1].Offset(), intervalEndProgress,
                                             TimingFunctionForKeyframeAtIndex(i - 1));
    }

    // If we end up here, then this means we are dealing with an implicit 100% keyframe. This will be a linear
    // timing function unless we're dealing with a CSS Animation which might have the default timing function
    // for its keyframes defined on its backing Animation object. In any other case, we are not dealing with
    // an interval with a steps() timing function.
    return null;
  }

  bool KeyframeEffect::TicksContinuouslyWhileActive() const noexcept
  {
    auto doesNotAffectStyles = _blendingKeyframes.IsEmpty() || _blendingKeyframes.Properties().empty();
    if (doesNotAffectStyles)
    {
      return false;
    }

    auto targetHasDisplayContents = [&]()
    {
      return _target && !_pseudoElementIdentifier && _target->HasDisplayContents();
    };

    if (!Renderer() && !_blendingKeyframes.Properties().contains(css::PropertyId::Display)
        && !targetHasDisplayContents())
    {
      return false;
    }

    // FIXME: Effects that run completely accelerated, once there are accelerated animations.
    return true;
  }

  Seconds KeyframeEffect::TimeToNextTick(const BasicEffectTiming &timing) noexcept
  {
    // FIXME: Waiting for the next iteration of CSS animations with `animationiteration` listeners, once there
    // are CSS animations.
    return AnimationEffect::TimeToNextTick(timing);
  }

  void KeyframeEffect::SetIterationComposite(
    boo::IterationCompositeOperation iterationCompositeOperation) noexcept
  {
    if (_iterationCompositeOperation == iterationCompositeOperation)
    {
      return;
    }

    _iterationCompositeOperation = iterationCompositeOperation;
    Invalidate();
  }

  void KeyframeEffect::SetComposite(boo::CompositeOperation compositeOperation) noexcept
  {
    if (_compositeOperation == compositeOperation)
    {
      return;
    }

    _compositeOperation = compositeOperation;
    Invalidate();
  }

  bool KeyframeEffect::PreventsAnimationReadiness() const noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#ready
    // An animation cannot be ready if it's associated with a document that does not have a browsing context
    // since this will prevent the first frame of the animmation from being rendered.
    return Document() && !Document()->Frame();
  }

  const KeyframeInterpolation::Keyframe &KeyframeEffect::KeyframeAtIndex(size_t index) const noexcept
  {
    krys_debug_assert(index < _blendingKeyframes.Size());
    return _blendingKeyframes[index];
  }

  const TimingFunction *
    KeyframeEffect::TimingFunctionForKeyframe(const KeyframeInterpolation::Keyframe &keyframe) const noexcept
  {
    if (auto *blendingKeyframe = DynamicDowncast<BlendingKeyframe>(&keyframe))
    {
      return TimingFunctionForBlendingKeyframe(*blendingKeyframe);
    }

    krys_debug_assert(false);
    return nullptr;
  }

  bool KeyframeEffect::IsPropertyAdditiveOrCumulative(KeyframeInterpolation::Property property) const noexcept
  {
    if (const auto *animatableCSSProperty = std::get_if<AnimatableCSSProperty>(&property))
    {
      return style::interpolation::IsAdditiveOrCumulative(*animatableCSSProperty);
    }

    krys_debug_assert(false);
    return false;
  }
}
