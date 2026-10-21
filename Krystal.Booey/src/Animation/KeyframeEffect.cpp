#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerAnimation.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperationsSharedPrimitivesPrefix.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Interpolation.hpp"
#include "Krystal.Booey/Style/PendingResources.hpp"
#include "Krystal.Booey/Style/PropertyAllowlist.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"
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

  bool KeyframeEffect::ForceLayoutIfNeeded() noexcept
  {
    if (!_needsForcedLayout || !_target)
    {
      return false;
    }

    auto *renderer = Renderer();
    if (!renderer || !renderer->Parent())
    {
      return false;
    }

    krys_debug_assert(Document());
    auto *frameView = Document()->View();
    if (!frameView)
    {
      return false;
    }

    frameView->LayoutContext().ScheduleLayout();
    frameView->LayoutContext().Layout();
    return true;
  }

  void KeyframeEffect::SetBlendingKeyframes(BlendingKeyframes &&blendingKeyframes) noexcept
  {
    CanBeAcceleratedMutationScope mutationScope(this);

    _blendingKeyframes = krys::move(blendingKeyframes);
    _animatedProperties.clear();

    ComputedNeedsForcedLayout();
    ComputeStackingContextImpact();
    ComputeAcceleratedPropertiesState();
    ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints();
    ComputeHasImplicitKeyframeForAcceleratedProperty();
    ComputeHasKeyframeComposingAcceleratedProperty();
    ComputeHasAcceleratedPropertyOverriddenByCascadeProperty();
    ComputeHasReferenceFilter();
    ComputeHasSizeDependentTransform();
    AnalyzeAcceleratedProperties();

    CheckForMatchingTransformFunctionLists();

    UpdateAcceleratedAnimationIfNecessary();
  }

  void KeyframeEffect::AnalyzeAcceleratedProperties() noexcept
  {
    _acceleratedProperties.clear();
    _acceleratedPropertiesWithImplicitKeyframe.clear();

    krys_debug_assert(Document());
    const auto &settings = Document()->GetSettings();
    for (const auto &property : _blendingKeyframes.Properties())
    {
      if (!style::interpolation::IsAccelerated(property, settings))
      {
        continue;
      }
      _acceleratedProperties.insert(property);
      if (_blendingKeyframes.HasImplicitKeyframeForProperty(property))
      {
        _acceleratedPropertiesWithImplicitKeyframe.insert(property);
      }
    }
  }

  void KeyframeEffect::CheckForMatchingTransformFunctionLists() noexcept
  {
    if (_blendingKeyframes.Size() < 2 || !_blendingKeyframes.ContainsProperty(css::PropertyId::Transform))
    {
      _transformFunctionListsMatchPrefix = 0;
      return;
    }

    TransformOperationsSharedPrimitivesPrefix<style::TransformFunctionType> prefix;
    for (const auto &keyframe : _blendingKeyframes)
    {
      prefix.Update(keyframe.Style()->GetTransform());
    }

    _transformFunctionListsMatchPrefix = static_cast<uint32>(prefix.Primitives().size());
  }

  Maybe<uint32> KeyframeEffect::TransformFunctionListPrefix() const noexcept
  {
    // The accelerated animation code can only use direct function interpolation when all keyframes share the
    // same prefix of shared transform function primitives, whereas software animations simply calls
    // blend(...) which can do direct interpolation based on the function list of any two particular
    // keyframes. The prefix serves as a way to make sure that the results of blend(...) can be made to return
    // the same results as rendered by the accelerated animation code.
    if (PreventsAcceleration())
    {
      return null;
    }
    return _transformFunctionListsMatchPrefix;
  }

  void KeyframeEffect::ComputedNeedsForcedLayout() noexcept
  {
    _needsForcedLayout = [&]()
    {
      if (Animation() && Is<CSSTransition>(*Animation()))
      {
        return false;
      }
      return _blendingKeyframes.HasWidthDependentTransform()
             || _blendingKeyframes.HasHeightDependentTransform();
    }();
  }

  void KeyframeEffect::ComputeStyleOriginatedAnimationBlendingKeyframes(
    const RenderStyle *oldStyle, const RenderStyle &newStyle,
    const style::ResolutionContext &resolutionContext) noexcept
  {
    krys_debug_assert(Animation() && Is<StyleOriginatedAnimation>(*Animation()));

    if (Animation() && Is<CSSAnimation>(*Animation()))
    {
      ComputeCSSAnimationBlendingKeyframes(newStyle, resolutionContext);
    }
    else if (Animation() && Is<CSSTransition>(*Animation()))
    {
      krys_debug_assert(oldStyle);
      ComputeCSSTransitionBlendingKeyframes(*oldStyle, newStyle);
    }
  }

  void KeyframeEffect::ComputeCSSAnimationBlendingKeyframes(
    const RenderStyle &unanimatedStyle, const style::ResolutionContext &resolutionContext) noexcept
  {
    krys_debug_assert(Document());

    const auto &backingStyleAnimation = DynamicDowncast<CSSAnimation>(Animation())->BackingStyleAnimation();

    // NOTE: A CSSAnimation is always constructed with a backing style::Animation that has a valid, non-none,
    // name.
    auto backingStyleAnimationName = backingStyleAnimation.GetName().TryKeyframesName();
    BlendingKeyframes blendingKeyframes(KeyframesIdentifier {backingStyleAnimationName->Name});
    if (_target)
    {
      (void)style::Scope::ResolveTreeScopedReference(
        *_target, *backingStyleAnimationName,
        [&](const style::Scope &scope, const css::CSSOMStringAtom &)
        {
          krys_debug_assert(scope.ResolverIfExists());
          return scope.ResolverIfExists()->KeyframeStylesForAnimation(
            *_target, unanimatedStyle, resolutionContext, blendingKeyframes,
            backingStyleAnimation.GetTimingFunction().Value.get());
        });

      // Ensure resource loads for all the frames.
      for (const auto &keyframe : blendingKeyframes)
      {
        if (auto *style = const_cast<RenderStyle *>(keyframe.Style()))
        {
          style::LoadPendingResources(*style, *Document(), _target.get());
        }
      }
    }

    _animationType = WebAnimationType::CSSAnimation;
    SetBlendingKeyframes(krys::move(blendingKeyframes));
  }

  void KeyframeEffect::RecomputeKeyframesAtNextOpportunity() noexcept
  {
    ClearBlendingKeyframes();
    Invalidate();
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

  void KeyframeEffect::UpdateIsAssociatedWithProgressBasedTimeline() noexcept
  {
    auto wasAssociatedWithProgressBasedTimeline = _isAssociatedWithProgressBasedTimeline;

    _isAssociatedWithProgressBasedTimeline = [&]
    {
      if (auto *animation = Animation())
      {
        if (auto *timeline = animation->Timeline())
        {
          return timeline->IsProgressBased();
        }
      }
      return false;
    }();

    if (wasAssociatedWithProgressBasedTimeline != _isAssociatedWithProgressBasedTimeline)
    {
      UpdateAcceleratedAnimationIfNecessary();
    }
  }

  void KeyframeEffect::AnimationTimelineDidChange() noexcept
  {
    AnimationEffect::AnimationTimelineDidChange();

    UpdateIsAssociatedWithProgressBasedTimeline();

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

    UpdateIsAssociatedWithProgressBasedTimeline();
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

  bool KeyframeEffect::IsRunningAccelerated() const noexcept
  {
    return _runningAccelerated == RunningAccelerated::Yes;
  }

  bool KeyframeEffect::IsCurrentlyAffectingProperty(css::PropertyId property,
                                                    Accelerated accelerated) const noexcept
  {
    if (accelerated == Accelerated::Yes && !IsRunningAccelerated() && !IsAboutToRunAccelerated())
    {
      return false;
    }

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

  bool KeyframeEffect::IsRunningAcceleratedAnimationForProperty(css::PropertyId property) const noexcept
  {
    if (!IsRunningAccelerated())
    {
      return false;
    }

    krys_debug_assert(Document());
    return style::interpolation::IsAccelerated(property, Document()->GetSettings())
           && _blendingKeyframes.Properties().contains(property);
  }

  bool KeyframeEffect::IsRunningAcceleratedTransformRelatedAnimation() const noexcept
  {
    return IsRunningAccelerated()
           && AnimatablePropertiesContainTransformRelatedProperty(_blendingKeyframes.Properties());
  }

  void KeyframeEffect::Invalidate() noexcept
  {
    InvalidateElement(TargetStyleable());
  }

  void KeyframeEffect::ComputeAcceleratedPropertiesState() noexcept
  {
    bool hasSomeAcceleratedProperties = false;
    bool hasSomeUnacceleratedProperties = false;

    if (auto *document = Document())
    {
      const auto &settings = document->GetSettings();
      for (const auto &property : _blendingKeyframes.Properties())
      {
        // If any animated property can be accelerated, then the animation should run accelerated.
        if (style::interpolation::IsAccelerated(property, settings))
        {
          hasSomeAcceleratedProperties = true;
        }
        else
        {
          hasSomeUnacceleratedProperties = true;
        }
        if (hasSomeAcceleratedProperties && hasSomeUnacceleratedProperties)
        {
          break;
        }
      }
    }

    if (!hasSomeAcceleratedProperties)
    {
      _acceleratedPropertiesState = AcceleratedPropertiesState::None;
    }
    else if (hasSomeUnacceleratedProperties)
    {
      _acceleratedPropertiesState = AcceleratedPropertiesState::Some;
    }
    else
    {
      _acceleratedPropertiesState = AcceleratedPropertiesState::All;
    }
  }

  void KeyframeEffect::ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints() noexcept
  {
    _someKeyframesUseStepsTimingFunction = false;
    _someKeyframesUseLinearTimingFunctionWithPoints = false;

    // If we're dealing with a CSS Animation and it specifies a default steps() or linear() timing function,
    // we need to check that any of the specified keyframes either does not have an explicit timing function
    // or specifies an explicit steps() or linear() timing function.
    if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(Animation()))
    {
      auto *defaultTimingFunction = cssAnimation->BackingStyleAnimation().GetTimingFunction().Value.get();
      auto defaultTimingFunctionIsSteps = Is<StepsTimingFunction>(*defaultTimingFunction);
      auto defaultTimingFunctionIsLinearWithPoints = IsLinearTimingFunctionWithPoints(defaultTimingFunction);
      if (defaultTimingFunctionIsSteps || defaultTimingFunctionIsLinearWithPoints)
      {
        for (const auto &keyframe : _blendingKeyframes)
        {
          auto *timingFunction = keyframe.GetTimingFunction();
          if (defaultTimingFunctionIsSteps && !_someKeyframesUseStepsTimingFunction)
          {
            _someKeyframesUseStepsTimingFunction =
              !timingFunction || Is<StepsTimingFunction>(*timingFunction);
          }
          else if (defaultTimingFunctionIsLinearWithPoints
                   && !_someKeyframesUseLinearTimingFunctionWithPoints)
          {
            _someKeyframesUseLinearTimingFunctionWithPoints =
              !timingFunction || IsLinearTimingFunctionWithPoints(timingFunction);
          }
          if (defaultTimingFunctionIsSteps == _someKeyframesUseStepsTimingFunction
              && defaultTimingFunctionIsLinearWithPoints == _someKeyframesUseLinearTimingFunctionWithPoints)
          {
            break;
          }
        }
        return;
      }
    }

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

  bool KeyframeEffect::CanBeAccelerated() const noexcept
  {
    if (!Animation() || !Animation()->Timeline())
    {
      return false;
    }

    if (_acceleratedPropertiesState == AcceleratedPropertiesState::None)
    {
      return false;
    }

    if (_hasAcceleratedPropertyOverriddenByCascadeProperty)
    {
      return false;
    }

    if (_hasReferenceFilter)
    {
      return false;
    }

    if (_animatesSizeAndSizeDependentTransform)
    {
      return false;
    }

    if (_blendingKeyframes.HasDiscreteTransformInterval())
    {
      return false;
    }

    // FIXME: Quirks that prevent acceleration (shouldPreventKeyframeEffectAcceleration), once there are
    // quirks.

    if (_isAssociatedWithProgressBasedTimeline)
    {
      return false;
    }

    if (_someKeyframesUseStepsTimingFunction
        || (TimingFunction() && Is<StepsTimingFunction>(*TimingFunction())))
    {
      return false;
    }

    if (_someKeyframesUseLinearTimingFunctionWithPoints || IsLinearTimingFunctionWithPoints(TimingFunction()))
    {
      return false;
    }

    if (_compositeOperation != boo::CompositeOperation::Replace)
    {
      return false;
    }

    if (_hasKeyframeComposingAcceleratedProperty)
    {
      return false;
    }

    return true;
  }

  bool KeyframeEffect::AnimatesMotionPath() const noexcept
  {
    return AnimatesProperty(css::PropertyId::OffsetAnchor)
           || AnimatesProperty(css::PropertyId::OffsetDistance)
           || AnimatesProperty(css::PropertyId::OffsetPath)
           || AnimatesProperty(css::PropertyId::OffsetPosition)
           || AnimatesProperty(css::PropertyId::OffsetRotate);
  }

  bool KeyframeEffect::PreventsAcceleration() const noexcept
  {
    // We cannot run accelerated transform animations if a motion path is applied
    // to an element, either through the underlying style, or through a keyframe.
    if (auto target = TargetStyleable())
    {
      if (const auto *lastStyleChangeEventStyle = target->LastStyleChangeEventStyle())
      {
        if (lastStyleChangeEventStyle->HasOffsetPath())
        {
          return true;
        }
      }
    }

    if (AnimatesMotionPath())
    {
      return true;
    }

    if (_acceleratedPropertiesState == AcceleratedPropertiesState::None)
    {
      return false;
    }

    return !CanBeAccelerated() || _runningAccelerated == RunningAccelerated::Failed;
  }

  void KeyframeEffect::UpdateAcceleratedActions() noexcept
  {
    auto *renderer = Renderer();
    if (!renderer || !renderer->IsComposited())
    {
      return;
    }

    if (!CanBeAccelerated())
    {
      return;
    }

    auto computedTiming = GetComputedTiming();

    // If we're not already running accelerated, the only thing we're interested in is whether we need to
    // start the animation which we need to do once we're in the active phase. Otherwise, there's no change in
    // accelerated state to consider.
    bool isActive = computedTiming.Phase == AnimationEffectPhase::Active;
    if (_runningAccelerated == RunningAccelerated::NotStarted)
    {
      if (isActive && Animation()->GetPlayState() == WebAnimation::PlayState::Running)
      {
        AddPendingAcceleratedAction(AcceleratedAction::Play);
      }
      return;
    }

    // If we're no longer active, we need to remove the accelerated animation.
    if (!isActive)
    {
      AddPendingAcceleratedAction(AcceleratedAction::Stop);
      return;
    }

    auto playState = Animation()->GetPlayState();
    // The only thing left to consider is whether we need to pause or resume the animation following a change
    // of play-state.
    if (playState == WebAnimation::PlayState::Paused)
    {
      if (_lastRecordedAcceleratedAction != AcceleratedAction::Pause)
      {
        if (_lastRecordedAcceleratedAction == AcceleratedAction::Stop)
        {
          AddPendingAcceleratedAction(AcceleratedAction::Play);
        }
        AddPendingAcceleratedAction(AcceleratedAction::Pause);
      }
    }
    else if (playState == WebAnimation::PlayState::Running && isActive)
    {
      if (_lastRecordedAcceleratedAction != AcceleratedAction::Play)
      {
        AddPendingAcceleratedAction(AcceleratedAction::Play);
      }
    }
  }

  void KeyframeEffect::AddPendingAcceleratedAction(AcceleratedAction action) noexcept
  {
    if (!Animation())
    {
      return;
    }

    if (_runningAccelerated == RunningAccelerated::Prevented
        || _runningAccelerated == RunningAccelerated::Failed)
    {
      return;
    }

    if (action == _lastRecordedAcceleratedAction)
    {
      return;
    }

    if (action == AcceleratedAction::Stop)
    {
      _pendingAcceleratedActions.clear();
    }
    _pendingAcceleratedActions.push_back(action);
    if (action != AcceleratedAction::UpdateProperties && action != AcceleratedAction::TransformChange)
    {
      _lastRecordedAcceleratedAction = action;
    }
    Animation()->AcceleratedStateDidChange();
  }

  void KeyframeEffect::AnimationDidTick() noexcept
  {
    Invalidate();
    UpdateAcceleratedActions();
  }

  void KeyframeEffect::AnimationDidChangeTimingProperties() noexcept
  {
    ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints();
    UpdateAcceleratedAnimationIfNecessary();
    Invalidate();
  }

  void KeyframeEffect::UpdateAcceleratedAnimationIfNecessary() noexcept
  {
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      if (CanBeAccelerated())
      {
        AddPendingAcceleratedAction(AcceleratedAction::UpdateProperties);
      }
      else
      {
        AbilityToBeAcceleratedDidChange();
        AddPendingAcceleratedAction(AcceleratedAction::Stop);
      }
    }
    else if (CanBeAccelerated())
    {
      _runningAccelerated = RunningAccelerated::NotStarted;
    }
  }

  void KeyframeEffect::AnimationPlaybackRateDidChange() noexcept
  {
    AnimationEffect::AnimationPlaybackRateDidChange();

    UpdateAcceleratedAnimationIfNecessary();
  }

  void KeyframeEffect::TransformRelatedPropertyDidChange() noexcept
  {
    krys_debug_assert(IsRunningAcceleratedTransformRelatedAnimation());
    auto hasTransformRelatedPropertyWithImplicitKeyframe =
      AnimatablePropertiesContainTransformRelatedProperty(_acceleratedPropertiesWithImplicitKeyframe);
    AddPendingAcceleratedAction(hasTransformRelatedPropertyWithImplicitKeyframe
                                  ? AcceleratedAction::UpdateProperties
                                  : AcceleratedAction::TransformChange);
  }

  Maybe<KeyframeEffect::RecomputationReason>
    KeyframeEffect::RecomputeKeyframesIfNecessary(const RenderStyle *previousUnanimatedStyle,
                                                  const RenderStyle &unanimatedStyle,
                                                  const style::ResolutionContext &resolutionContext) noexcept
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
          ComputeCSSAnimationBlendingKeyframes(unanimatedStyle, resolutionContext);
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

  void KeyframeEffect::AnimationWasCanceled() noexcept
  {
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      AddPendingAcceleratedAction(AcceleratedAction::Stop);
    }
  }

  void KeyframeEffect::WasRemovedFromEffectStack() noexcept
  {
    _inTargetEffectStack = false;

    if (!CanBeAccelerated())
    {
      return;
    }

    // If the effect was running accelerated, we need to mark it for removal straight away
    // since it will not be invalidated by a future call to
    // KeyframeEffectStack::ApplyPendingAcceleratedActions().
    krys_debug_assert(Animation());
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      Ref animation = ShareRef(*Animation());
      bool isFinishingNaturally = animation->HasPendingFinishNotification()
                                  || animation->GetPlayState() == WebAnimation::PlayState::Finished;

      _pendingAcceleratedActions.clear();
      _pendingAcceleratedActions.push_back(AcceleratedAction::Stop);

      if (isFinishingNaturally)
      {
        // Don't immediately stop animations that are finishing naturally - delay cleanup via microtask
        // to allow the finished promise callback to observe the final animation state (e.g., layer tree).
        // Only immediately stop animations removed mid-flight.
        html::EventLoop::Current().QueueMicrotask([protectedThis = Ref<KeyframeEffect>(ShareRef(*this))]
                                                  { protectedThis->ApplyPendingAcceleratedActions(); });
      }
      else
      {
        ApplyPendingAcceleratedActions();
      }
    }
  }

  void KeyframeEffect::WillChangeRenderer() noexcept
  {
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      AddPendingAcceleratedAction(AcceleratedAction::Stop);
    }
  }

  void KeyframeEffect::AnimationSuspensionStateDidChange(bool animationIsSuspended) noexcept
  {
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      AddPendingAcceleratedAction(animationIsSuspended ? AcceleratedAction::Pause : AcceleratedAction::Play);
    }
  }

  void KeyframeEffect::ApplyPendingAcceleratedActionsOrUpdateTimingProperties() noexcept
  {
    if (_pendingAcceleratedActions.empty())
    {
      if (!CanBeAccelerated() || GetComputedTiming().Phase != AnimationEffectPhase::Active)
      {
        return;
      }
      _pendingAcceleratedActions.push_back(AcceleratedAction::UpdateProperties);
      _lastRecordedAcceleratedAction = AcceleratedAction::Play;
      ApplyPendingAcceleratedActions();
      _pendingAcceleratedActions.clear();
    }
    else
    {
      ApplyPendingAcceleratedActions();
    }
  }

  void KeyframeEffect::ApplyPendingAcceleratedActions() noexcept
  {
    CanBeAcceleratedMutationScope mutationScope(this);

    // Once an accelerated animation has been committed, we no longer want to force a layout.
    // This should have been performed by a call to ForceLayoutIfNeeded() prior to applying
    // pending accelerated actions.
    _needsForcedLayout = false;

    if (_pendingAcceleratedActions.empty())
    {
      return;
    }

    auto *renderer = Renderer();
    if (!renderer || !renderer->IsComposited())
    {
      // The renderer may no longer be composited because the accelerated animation ended before we had a
      // chance to update it, in which case if we asked for the animation to stop, we can discard the current
      // set of accelerated actions.
      if (_lastRecordedAcceleratedAction == AcceleratedAction::Stop)
      {
        _pendingAcceleratedActions.clear();
        _runningAccelerated = RunningAccelerated::NotStarted;
      }
      return;
    }

    auto pendingAcceleratedActions = _pendingAcceleratedActions;
    _pendingAcceleratedActions.clear();

    auto timeOffset = [&]
    {
      // To simplify the code we use a default of 0s for an unresolved current time since for a Stop action
      // that is acceptable.
      auto currentTime = Animation()->CurrentTime().value_or(WebAnimationTime(Seconds {0}));
      auto cssNumberishTimeOffset = currentTime - Delay();
      krys_debug_assert(cssNumberishTimeOffset.Time());
      return cssNumberishTimeOffset.Time()->count();
    };

    auto startAnimation = [&]() -> RunningAccelerated
    {
      if (IsRunningAccelerated())
      {
        renderer->AnimationFinished(_blendingKeyframes);
      }

      krys_debug_assert(_target);
      auto *effectStack = _target->GetKeyframeEffectStack(_pseudoElementIdentifier);
      krys_debug_assert(effectStack);

      if ((_blendingKeyframes.HasWidthDependentTransform()
           && effectStack->ContainsProperty(css::PropertyId::Width))
          || (_blendingKeyframes.HasHeightDependentTransform()
              && effectStack->ContainsProperty(css::PropertyId::Height)))
      {
        return RunningAccelerated::Prevented;
      }

      if (!effectStack->AllowsAcceleration())
      {
        return RunningAccelerated::Prevented;
      }

      if (!_hasImplicitKeyframeForAcceleratedProperty)
      {
        return renderer->StartAnimation(timeOffset(), *BackingAnimationForCompositedRenderer(),
                                        _blendingKeyframes)
                 ? RunningAccelerated::Yes
                 : RunningAccelerated::Failed;
      }

      // We need to resolve all animations up to this point to ensure any forward-filling
      // effect is accounted for when computing the "from" value for the accelerated animation.
      auto underlyingStyle = [&]()
      {
        if (const auto *lastStyleChangeEventStyle =
              _target->LastStyleChangeEventStyle(_pseudoElementIdentifier))
        {
          return RenderStyle::ClonePtr(*lastStyleChangeEventStyle);
        }
        return RenderStyle::ClonePtr(renderer->Style());
      }();

      for (const auto &effect : effectStack->SortedEffects())
      {
        if (this == effect.get())
        {
          break;
        }
        auto computedTiming = effect->GetComputedTiming();
        if (computedTiming.Progress)
        {
          effect->SetAnimatedPropertiesInStyle(*underlyingStyle, computedTiming);
        }
      }

      BlendingKeyframes explicitKeyframes(_blendingKeyframes.Identifier());
      explicitKeyframes.CopyKeyframes(_blendingKeyframes);
      explicitKeyframes.FillImplicitKeyframes(*this, *underlyingStyle);
      return renderer->StartAnimation(timeOffset(), *BackingAnimationForCompositedRenderer(),
                                      explicitKeyframes)
               ? RunningAccelerated::Yes
               : RunningAccelerated::Failed;
    };

    for (const auto &action : pendingAcceleratedActions)
    {
      switch (action)
      {
        case AcceleratedAction::Play:
          _runningAccelerated = startAnimation();
          if (!IsRunningAccelerated())
          {
            _lastRecordedAcceleratedAction = AcceleratedAction::Stop;
            return;
          }
          break;
        case AcceleratedAction::Pause: renderer->AnimationPaused(timeOffset(), _blendingKeyframes); break;
        case AcceleratedAction::UpdateProperties:
          _runningAccelerated = startAnimation();
          if (Animation()->GetPlayState() == WebAnimation::PlayState::Paused)
          {
            renderer->AnimationPaused(timeOffset(), _blendingKeyframes);
          }
          break;
        case AcceleratedAction::Stop:
          krys_debug_assert(Document());
          renderer->AnimationFinished(_blendingKeyframes);
          if (!Document()->RenderTreeBeingDestroyed())
          {
            _target->InvalidateStyleAndLayerComposition();
          }
          _runningAccelerated =
            CanBeAccelerated() ? RunningAccelerated::NotStarted : RunningAccelerated::Prevented;
          break;
        case AcceleratedAction::TransformChange: renderer->TransformRelatedPropertyDidChange(); break;
      }
    }
  }

  Ref<const GraphicsLayerAnimation> KeyframeEffect::BackingAnimationForCompositedRenderer() noexcept
  {
    Ref effectAnimation = ShareRef(*Animation());

    // FIXME: The iterationStart and endDelay AnimationEffectTiming properties do not have
    // corresponding GraphicsLayerAnimation properties.
    auto animation = GraphicsLayerAnimation::Create();
    animation->SetDuration(IterationDuration().Time()->count());
    animation->SetDelay(Delay().Time()->count());
    animation->SetIterationCount(Iterations());
    animation->SetTimingFunction(TimingFunction()->Clone());
    animation->SetPlaybackRate(effectAnimation->PlaybackRate());
    animation->SetCompositeOperation(_compositeOperation);

    switch (Fill())
    {
      case FillMode::None:
      case FillMode::Auto:      animation->SetFillMode(GraphicsLayerAnimation::FillMode::None); break;
      case FillMode::Backwards: animation->SetFillMode(GraphicsLayerAnimation::FillMode::Backwards); break;
      case FillMode::Forwards:  animation->SetFillMode(GraphicsLayerAnimation::FillMode::Forwards); break;
      case FillMode::Both:      animation->SetFillMode(GraphicsLayerAnimation::FillMode::Both); break;
    }

    switch (Direction())
    {
      case PlaybackDirection::Normal:
        animation->SetDirection(GraphicsLayerAnimation::Direction::Normal);
        break;
      case PlaybackDirection::Alternate:
        animation->SetDirection(GraphicsLayerAnimation::Direction::Alternate);
        break;
      case PlaybackDirection::Reverse:
        animation->SetDirection(GraphicsLayerAnimation::Direction::Reverse);
        break;
      case PlaybackDirection::AlternateReverse:
        animation->SetDirection(GraphicsLayerAnimation::Direction::AlternateReverse);
        break;
    }

    // In the case of CSS Animations, we must set the default timing function for keyframes to match
    // the current value set for animation-timing-function on the target element which affects only
    // keyframes and not the animation-wide timing.
    if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(effectAnimation.get()))
    {
      animation->SetDefaultTimingFunctionForKeyframes(
        ShareRefPtr(cssAnimation->BackingStyleAnimation().GetTimingFunction().Value.get()));
    }

    return animation;
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

  bool KeyframeEffect::ComputeExtentOfTransformAnimation(LayoutRect &bounds) const noexcept
  {
    krys_debug_assert(AnimatablePropertiesContainTransformRelatedProperty(_blendingKeyframes.Properties()));

    auto *box = DynamicDowncast<RenderBox>(Renderer());
    if (!box)
    {
      return true; // Non-boxes don't get transformed;
    }

    // FIXME: add extent computation for animated CSS Motion Path properties.
    if (AnimatesMotionPath())
    {
      return true;
    }

    auto rendererBox = SnapRectToDevicePixels(box->BorderBoxRect(), box->GetDocument().DeviceScaleFactor());
    // FIXME: The motion path data of the box (TransformOperationData's renderer constructor), once there are
    // motion paths.
    TransformOperationData transformOperationData(rendererBox);
    LayoutRect cumulativeBounds;

    const auto *unanimatedStyle = [&]() -> const RenderStyle *
    {
      if (auto target = TargetStyleable())
      {
        if (const auto *lastStyleChangeEventStyle = target->LastStyleChangeEventStyle())
        {
          return lastStyleChangeEventStyle;
        }
      }
      return &box->Style();
    }();

    auto addStyleToCumulativeBounds = [&](const RenderStyle &style)
    {
      auto keyframeBounds = bounds;

      auto transform = style::TransformResolver::ComputeTransform(style, transformOperationData);
      if (!transform.IsAffine())
      {
        return false;
      }

      TransformationMatrix::Decomposed2Type fromDecomp;
      if (!transform.Decompose2(fromDecomp))
      {
        return false;
      }

      auto hasRotation = [&]
      {
        if (fromDecomp.Angle || style.GetRotate().AffectedByTransformOrigin())
        {
          return true;
        }
        for (const auto &operation : style.GetTransform())
        {
          if (operation->GetType() == style::TransformFunctionType::Rotate)
          {
            return true;
          }
        }
        return false;
      }();

      // FIXME: it may be better to find a way to replace all rotation angles
      // in transforms with a 90deg angle.
      if (hasRotation)
      {
        keyframeBounds = LayoutRect(BoundsOfRotatingRect(FloatRect(keyframeBounds)));
      }

      cumulativeBounds.Unite(transform.MapRect(keyframeBounds));
      return true;
    };

    for (const auto &keyframe : _blendingKeyframes)
    {
      if (!AnimatablePropertiesContainTransformRelatedProperty(keyframe.Properties()))
      {
        continue;
      }

      auto blendedStyleForKeyframe = RenderStyle::ClonePtr(*unanimatedStyle);

      ComputedEffectTiming computedTiming;
      computedTiming.CurrentIteration = 0;
      computedTiming.Progress = keyframe.Offset();

      SetAnimatedPropertiesInStyle(*blendedStyleForKeyframe, computedTiming);

      if (!addStyleToCumulativeBounds(*blendedStyleForKeyframe))
      {
        return false;
      }
    }

    if (_blendingKeyframes.HasImplicitKeyframes())
    {
      if (!addStyleToCumulativeBounds(*unanimatedStyle))
      {
        return false;
      }
    }

    bounds = cumulativeBounds;
    return true;
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
      if (!i)
      {
        if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(Animation()))
        {
          return progressUntilNextStepInInterval(
            0, intervalEndProgress, cssAnimation->BackingStyleAnimation().GetTimingFunction().Value.get());
        }
        return null;
      }

      return progressUntilNextStepInInterval(_blendingKeyframes[i - 1].Offset(), intervalEndProgress,
                                             TimingFunctionForKeyframeAtIndex(i - 1));
    }

    // If we end up here, then this means we are dealing with an implicit 100% keyframe. This will be a linear
    // timing function unless we're dealing with a CSS Animation which might have the default timing function
    // for its keyframes defined on its backing Animation object.
    const auto &lastExplicitKeyframe = _blendingKeyframes[_blendingKeyframes.Size() - 1];
    if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(Animation()))
    {
      return progressUntilNextStepInInterval(
        lastExplicitKeyframe.Offset(), 1,
        cssAnimation->BackingStyleAnimation().GetTimingFunction().Value.get());
    }

    // In any other case, we are not dealing with an interval with a steps() timing function.
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

    if (IsCompletelyAccelerated() && IsRunningAccelerated())
    {
      return false;
    }

    return true;
  }

  Seconds KeyframeEffect::TimeToNextTick(const BasicEffectTiming &timing) noexcept
  {
    // CSS Animations need to trigger "animationiteration" events even if there is no need to update styles
    // while animating, so if we're dealing with one we must wait until the next iteration.
    // FIXME: Only when the document has `animationiteration` listeners (hasListenerType), once documents
    // track their listener types.
    if (timing.Phase == AnimationEffectPhase::Active && Animation() && Is<CSSAnimation>(*Animation())
        && !TicksContinuouslyWhileActive())
    {
      if (auto iterationProgress = GetComputedTiming().SimpleIterationProgress)
      {
        return static_cast<Seconds>(IterationDuration() * (1 - *iterationProgress));
      }
    }
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

    CanBeAcceleratedMutationScope mutationScope(this);
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

  void KeyframeEffect::ComputeHasImplicitKeyframeForAcceleratedProperty() noexcept
  {
    _hasImplicitKeyframeForAcceleratedProperty = [&]()
    {
      if (_acceleratedPropertiesState == AcceleratedPropertiesState::None)
      {
        return false;
      }

      krys_debug_assert(Document());
      const auto &settings = Document()->GetSettings();

      // FIXME: The parsed keyframes, before keyframes are computed, once there are keyframes from scripts.
      if (_blendingKeyframes.IsEmpty())
      {
        return false;
      }

      // We make a list of all animated properties and consider them all
      // implicit until proven otherwise as we iterate through all keyframes.
      auto implicitZeroProperties = _blendingKeyframes.Properties();
      auto implicitOneProperties = _blendingKeyframes.Properties();
      for (const auto &keyframe : _blendingKeyframes)
      {
        // If the keyframe is for 0% or 100%, let's remove all of its properties from
        // our list of implicit properties.
        if (!implicitZeroProperties.empty() && !keyframe.Offset())
        {
          for (const auto &property : keyframe.Properties())
          {
            implicitZeroProperties.erase(property);
          }
        }
        if (!implicitOneProperties.empty() && keyframe.Offset() == 1)
        {
          for (const auto &property : keyframe.Properties())
          {
            implicitOneProperties.erase(property);
          }
        }
      }
      // The only properties left are known to be implicit properties, so we must
      // check them for any accelerated property.
      for (const auto &implicitProperty : implicitZeroProperties)
      {
        if (style::interpolation::IsAccelerated(implicitProperty, settings))
        {
          return true;
        }
      }
      for (const auto &implicitProperty : implicitOneProperties)
      {
        if (style::interpolation::IsAccelerated(implicitProperty, settings))
        {
          return true;
        }
      }
      return false;
    }();
  }

  void KeyframeEffect::ComputeHasKeyframeComposingAcceleratedProperty() noexcept
  {
    _hasKeyframeComposingAcceleratedProperty = [&]()
    {
      if (_acceleratedPropertiesState == AcceleratedPropertiesState::None)
      {
        return false;
      }

      krys_debug_assert(Document());
      const auto &settings = Document()->GetSettings();

      // FIXME: The parsed keyframes, before keyframes are computed, once there are keyframes from scripts.
      for (const auto &keyframe : _blendingKeyframes)
      {
        // If we find a keyframe with a composite operation, we check whether one
        // of its properties is accelerated.
        if (auto keyframeComposite = keyframe.CompositeOperation())
        {
          if (*keyframeComposite != boo::CompositeOperation::Replace)
          {
            for (const auto &property : keyframe.Properties())
            {
              if (style::interpolation::IsAccelerated(property, settings))
              {
                return true;
              }
            }
          }
        }
      }
      return false;
    }();
  }

  void KeyframeEffect::ComputeHasAcceleratedPropertyOverriddenByCascadeProperty() noexcept
  {
    if (!_inTargetEffectStack)
    {
      return;
    }

    krys_debug_assert(_target);
    auto *effectStack = _target->GetKeyframeEffectStack(_pseudoElementIdentifier);
    if (!effectStack)
    {
      return;
    }

    const auto &animatedProperties = AnimatedProperties();
    _hasAcceleratedPropertyOverriddenByCascadeProperty =
      std::ranges::any_of(effectStack->AcceleratedPropertiesOverriddenByCascade(),
                          [&](const auto &property) { return animatedProperties.contains(property); });
  }

  void KeyframeEffect::ComputeHasReferenceFilter() noexcept
  {
    _hasReferenceFilter = [&]()
    {
      if (_blendingKeyframes.IsEmpty())
      {
        return false;
      }

      auto animatesFilterProperty = _blendingKeyframes.ContainsProperty(css::PropertyId::Filter)
                                    || _blendingKeyframes.ContainsProperty(css::PropertyId::BackdropFilter);

      if (!animatesFilterProperty)
      {
        return false;
      }

      auto styleContainsFilter = [](const RenderStyle &style)
      {
        return style.GetFilter().HasReferenceFilter() || style.BackdropFilter().HasReferenceFilter();
      };

      if (auto target = TargetStyleable())
      {
        if (const auto *style = target->LastStyleChangeEventStyle())
        {
          if (_blendingKeyframes.HasImplicitKeyframes() && styleContainsFilter(*style))
          {
            return true;
          }
        }
      }

      for (const auto &keyframe : _blendingKeyframes)
      {
        if (const auto *style = keyframe.Style())
        {
          if (styleContainsFilter(*style))
          {
            return true;
          }
        }
      }

      return false;
    }();
  }

  void KeyframeEffect::ComputeHasSizeDependentTransform() noexcept
  {
    _animatesSizeAndSizeDependentTransform =
      (_blendingKeyframes.HasWidthDependentTransform()
       && _blendingKeyframes.ContainsProperty(css::PropertyId::Width))
      || (_blendingKeyframes.HasHeightDependentTransform()
          && _blendingKeyframes.ContainsProperty(css::PropertyId::Height));

    // FIXME: View transition group pseudo-elements' size and transform animations, once there are view
    // transitions.
  }

  void KeyframeEffect::EffectStackNoLongerPreventsAcceleration() noexcept
  {
    if (_runningAccelerated == RunningAccelerated::Failed)
    {
      return;
    }

    if (_runningAccelerated == RunningAccelerated::Prevented)
    {
      _runningAccelerated = RunningAccelerated::NotStarted;
    }

    UpdateAcceleratedActions();
  }

  void KeyframeEffect::EffectStackNoLongerAllowsAcceleration() noexcept
  {
    AddPendingAcceleratedAction(AcceleratedAction::Stop);
  }

  void KeyframeEffect::EffectStackNoLongerAllowsAccelerationDuringAcceleratedActionApplication() noexcept
  {
    _pendingAcceleratedActions.push_back(AcceleratedAction::Stop);
    _lastRecordedAcceleratedAction = AcceleratedAction::Stop;
    ApplyPendingAcceleratedActions();
    _pendingAcceleratedActions.clear();
  }

  void KeyframeEffect::AbilityToBeAcceleratedDidChange() noexcept
  {
    if (!_inTargetEffectStack)
    {
      return;
    }

    krys_debug_assert(_target);
    if (auto *effectStack = _target->GetKeyframeEffectStack(_pseudoElementIdentifier))
    {
      effectStack->EffectAbilityToBeAcceleratedDidChange(*this);
    }
  }

  void KeyframeEffect::AcceleratedPropertiesOverriddenByCascadeDidChange() noexcept
  {
    CanBeAcceleratedMutationScope mutationScope(this);
    ComputeHasAcceleratedPropertyOverriddenByCascadeProperty();
  }

  KeyframeEffect::CanBeAcceleratedMutationScope::CanBeAcceleratedMutationScope(
    KeyframeEffect *effect) noexcept
      : _effect(effect)
  {
    krys_debug_assert(effect);
    _couldOriginallyPreventAcceleration = effect->PreventsAcceleration();
  }

  KeyframeEffect::CanBeAcceleratedMutationScope::~CanBeAcceleratedMutationScope() noexcept
  {
    if (!_effect)
    {
      return;
    }

    if (_couldOriginallyPreventAcceleration != _effect->PreventsAcceleration())
    {
      _effect->AbilityToBeAcceleratedDidChange();
    }
  }

  void KeyframeEffect::LastStyleChangeEventStyleDidChange(const RenderStyle *previousStyle,
                                                          const RenderStyle *currentStyle) noexcept
  {
    auto hasMotionPath = [](const RenderStyle *style)
    {
      return style && style->HasOffsetPath();
    };

    if (hasMotionPath(previousStyle) != hasMotionPath(currentStyle))
    {
      AbilityToBeAcceleratedDidChange();
    }
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
