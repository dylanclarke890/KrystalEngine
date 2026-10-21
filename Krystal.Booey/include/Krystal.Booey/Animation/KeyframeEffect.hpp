#pragma once

#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/BlendingKeyframes.hpp"
#include "Krystal.Booey/Animation/KeyframeInterpolation.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/IterationCompositeOperation.hpp"
#include "Krystal.Booey/Style/InterpolationClient.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::style
{
  struct ResolutionContext;
  struct Styleable;
}

namespace krys::boo
{
  /// @brief An animation effect that animates properties of an element, or of one of its pseudo-elements,
  /// through keyframes.
  /// @see https://drafts.csswg.org/web-animations-1/#keyframe-effects
  // FIXME: Keyframes from scripts (the parsed keyframes, `getKeyframes()` and `setKeyframes()`), the
  // `KeyframeEffect` interface, accelerated effects (threaded animations) and view timeline keyframe offsets,
  // once there are scripting, threaded animations and view timelines.
  class KeyframeEffect final : public AnimationEffect,
                               public style::interpolation::Client,
                               public KeyframeInterpolation
  {
  public:
    enum class RecomputationReason : uint8
    {
      LogicalPropertyChange,
      Other
    };

    enum class Accelerated : bool
    {
      No,
      Yes
    };

  private:
    enum class AcceleratedAction : uint8
    {
      Play,
      Pause,
      UpdateProperties,
      TransformChange,
      Stop
    };
    enum class AcceleratedPropertiesState : uint8
    {
      None,
      Some,
      All
    };
    enum class RunningAccelerated : uint8
    {
      NotStarted,
      Yes,
      Prevented,
      Failed
    };

    /// @brief Tells the effect stack when the effect's ability to run accelerated changed in its scope.
    class CanBeAcceleratedMutationScope
    {
      KRYS_NON_COPYABLE(CanBeAcceleratedMutationScope);

    private:
      KeyframeEffect *_effect;
      bool _couldOriginallyPreventAcceleration;

    public:
      explicit CanBeAcceleratedMutationScope(KeyframeEffect *effect) noexcept;
      ~CanBeAcceleratedMutationScope() noexcept;
    };

    WeakPtr<dom::Document> _document;
    BlendingKeyframes _blendingKeyframes;
    AnimatableCSSPropertySet _animatedProperties;
    AnimatableCSSPropertySet _acceleratedProperties;
    AnimatableCSSPropertySet _acceleratedPropertiesWithImplicitKeyframe;
    List<AcceleratedAction> _pendingAcceleratedActions;
    RefPtr<dom::Element> _target;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;
    WebAnimationType _animationType {WebAnimationType::WebAnimation};
    boo::IterationCompositeOperation _iterationCompositeOperation {boo::IterationCompositeOperation::Replace};
    boo::CompositeOperation _compositeOperation {boo::CompositeOperation::Replace};
    AnimationEffectPhase _phaseAtLastApplication {AnimationEffectPhase::Idle};
    AcceleratedAction _lastRecordedAcceleratedAction {AcceleratedAction::Stop};
    AcceleratedPropertiesState _acceleratedPropertiesState {AcceleratedPropertiesState::None};
    RunningAccelerated _runningAccelerated {RunningAccelerated::NotStarted};
    uint32 _transformFunctionListsMatchPrefix {0};
    bool _needsForcedLayout {false};
    bool _triggersStackingContext {false};
    bool _inTargetEffectStack {false};
    bool _someKeyframesUseLinearTimingFunctionWithPoints {false};
    bool _someKeyframesUseStepsTimingFunction {false};
    bool _hasImplicitKeyframeForAcceleratedProperty {false};
    bool _hasKeyframeComposingAcceleratedProperty {false};
    bool _hasAcceleratedPropertyOverriddenByCascadeProperty {false};
    bool _hasReferenceFilter {false};
    bool _animatesSizeAndSizeDependentTransform {false};
    bool _isAssociatedWithProgressBasedTimeline {false};

    KeyframeEffect(dom::Element *target,
                   const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

  public:
    ~KeyframeEffect() noexcept override;

    KRYS_NODISCARD static Ref<KeyframeEffect>
      Create(const dom::Element &target,
             const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    KRYS_NODISCARD dom::Element *Target() const noexcept
    {
      return _target.get();
    }
    void SetTarget(RefPtr<dom::Element> &&newTarget) noexcept;
    KRYS_NODISCARD bool TargetsPseudoElement() const noexcept;
    KRYS_NODISCARD css::CSSOMString PseudoElement() const noexcept;
    KRYS_NODISCARD Maybe<style::Styleable> TargetStyleable() const noexcept;

    KRYS_NODISCARD boo::IterationCompositeOperation IterationComposite() const noexcept
    {
      return _iterationCompositeOperation;
    }
    void SetIterationComposite(boo::IterationCompositeOperation iterationCompositeOperation) noexcept;
    KRYS_NODISCARD boo::CompositeOperation Composite() const noexcept
    {
      return _compositeOperation;
    }
    void SetComposite(boo::CompositeOperation compositeOperation) noexcept;

    /// @brief Applies the effect at the animation's current time to @p targetStyle.
    EnumSet<AnimationImpact> Apply(RenderStyle &targetStyle,
                                   const style::ResolutionContext &resolutionContext,
                                   EndpointInclusiveActiveInterval endpointInclusiveActiveInterval =
                                     EndpointInclusiveActiveInterval::No) noexcept;
    /// @brief Applies the effect at the animation's current time to @p animatedStyle, creating it from the
    /// target's style if it's null.
    void GetAnimatedStyle(UniquePtr<RenderStyle> &animatedStyle) noexcept;
    /// @brief Invalidates the target's style.
    void Invalidate() noexcept;

    void AnimationRelevancyDidChange() noexcept;

    /// @brief Recomputes the keyframes when the unanimated style changed in a way their resolution depends
    /// on.
    Maybe<RecomputationReason>
      RecomputeKeyframesIfNecessary(const RenderStyle *previousUnanimatedStyle,
                                    const RenderStyle &unanimatedStyle,
                                    const style::ResolutionContext &resolutionContext) noexcept;

    void WillChangeRenderer() noexcept;
    /// @brief Clears the keyframes, so they're computed again when the effect is next applied.
    void RecomputeKeyframesAtNextOpportunity() noexcept;

    KRYS_NODISCARD dom::Document *Document() const noexcept final;
    KRYS_NODISCARD RenderElement *Renderer() const noexcept final;
    KRYS_NODISCARD const RenderStyle &CurrentStyle() const noexcept final;
    KRYS_NODISCARD Maybe<uint32> TransformFunctionListPrefix() const noexcept override;

    KRYS_NODISCARD bool TriggersStackingContext() const noexcept
    {
      return _triggersStackingContext;
    }

    /// @brief Computes the keyframes of a CSS transition (from @p oldStyle to @p newStyle) or a CSS
    /// animation.
    void ComputeStyleOriginatedAnimationBlendingKeyframes(
      const RenderStyle *oldStyle, const RenderStyle &newStyle,
      const style::ResolutionContext &resolutionContext) noexcept;
    KRYS_NODISCARD const BlendingKeyframes &GetBlendingKeyframes() const noexcept
    {
      return _blendingKeyframes;
    }
    KRYS_NODISCARD const AnimatableCSSPropertySet &AnimatedProperties() noexcept;
    KRYS_NODISCARD bool AnimatesProperty(const AnimatableCSSProperty &property) const noexcept;

    KRYS_NODISCARD bool
      IsCurrentlyAffectingProperty(css::PropertyId property,
                                   Accelerated accelerated = Accelerated::No) const noexcept;
    KRYS_NODISCARD bool IsRunningAcceleratedAnimationForProperty(css::PropertyId property) const noexcept;
    KRYS_NODISCARD bool IsRunningAcceleratedTransformRelatedAnimation() const noexcept;

    void ApplyPendingAcceleratedActions() noexcept;
    void ApplyPendingAcceleratedActionsOrUpdateTimingProperties() noexcept;

    KRYS_NODISCARD bool IsRunningAccelerated() const noexcept;
    // FIXME: These ignore the fact that some timing functions can prevent acceleration.
    KRYS_NODISCARD bool IsAboutToRunAccelerated() const noexcept
    {
      return _acceleratedPropertiesState != AcceleratedPropertiesState::None
             && _lastRecordedAcceleratedAction != AcceleratedAction::Stop;
    }

    KRYS_NODISCARD const AnimatableCSSPropertySet &AcceleratedProperties() const noexcept
    {
      return _acceleratedProperties;
    }
    KRYS_NODISCARD const AnimatableCSSPropertySet &AcceleratedPropertiesWithImplicitKeyframe() const noexcept
    {
      return _acceleratedPropertiesWithImplicitKeyframe;
    }

    KRYS_NODISCARD bool CanBeAccelerated() const noexcept;
    KRYS_NODISCARD bool AccelerationWasPrevented() const noexcept
    {
      return _runningAccelerated == RunningAccelerated::Prevented;
    }
    KRYS_NODISCARD bool PreventsAcceleration() const noexcept;
    void EffectStackNoLongerPreventsAcceleration() noexcept;
    void EffectStackNoLongerAllowsAcceleration() noexcept;
    void EffectStackNoLongerAllowsAccelerationDuringAcceleratedActionApplication() noexcept;

    void LastStyleChangeEventStyleDidChange(const RenderStyle *previousStyle,
                                            const RenderStyle *currentStyle) noexcept;
    void AcceleratedPropertiesOverriddenByCascadeDidChange() noexcept;
    void TransformRelatedPropertyDidChange() noexcept;

    /// @brief Lays out the document when the keyframes depend on the target's size, before accelerated
    /// actions are applied.
    bool ForceLayoutIfNeeded() noexcept;

    /// @brief Grows @p bounds to cover each keyframe's transform of them.
    /// @return False if a keyframe's transform isn't affine or can't be decomposed.
    KRYS_NODISCARD bool ComputeExtentOfTransformAnimation(LayoutRect &bounds) const noexcept;
    KRYS_NODISCARD bool RequiresPseudoElement() const noexcept;

    void WasAddedToEffectStack() noexcept;
    void WasRemovedFromEffectStack() noexcept;

    KRYS_NODISCARD WebAnimationType AnimationType() const noexcept
    {
      return _animationType;
    }

  private:
    void AddPendingAcceleratedAction(AcceleratedAction action) noexcept;
    KRYS_NODISCARD bool IsCompletelyAccelerated() const noexcept
    {
      return _acceleratedPropertiesState == AcceleratedPropertiesState::All;
    }
    void UpdateAcceleratedActions() noexcept;
    KRYS_NODISCARD Ref<const GraphicsLayerAnimation> BackingAnimationForCompositedRenderer() noexcept;
    void ComputeAcceleratedPropertiesState() noexcept;
    void ComputeHasImplicitKeyframeForAcceleratedProperty() noexcept;
    void ComputeHasKeyframeComposingAcceleratedProperty() noexcept;
    void ComputeHasAcceleratedPropertyOverriddenByCascadeProperty() noexcept;
    void ComputeHasReferenceFilter() noexcept;
    void ComputeHasSizeDependentTransform() noexcept;
    void ComputedNeedsForcedLayout() noexcept;
    void AnalyzeAcceleratedProperties() noexcept;
    void CheckForMatchingTransformFunctionLists() noexcept;
    void AbilityToBeAcceleratedDidChange() noexcept;
    void UpdateAcceleratedAnimationIfNecessary() noexcept;
    void UpdateIsAssociatedWithProgressBasedTimeline() noexcept;
    KRYS_NODISCARD bool AnimatesMotionPath() const noexcept;

    void UpdateEffectStackMembership() noexcept;
    void DidChangeTargetStyleable(const Maybe<style::Styleable> &previousTargetStyleable) noexcept;
    void SetAnimatedPropertiesInStyle(RenderStyle &targetStyle,
                                      const ComputedEffectTiming &computedTiming) const noexcept;
    KRYS_NODISCARD const boo::TimingFunction *TimingFunctionForKeyframeAtIndex(size_t index) const noexcept;
    KRYS_NODISCARD const boo::TimingFunction *
      TimingFunctionForBlendingKeyframe(const BlendingKeyframe &keyframe) const noexcept;
    void ComputeStackingContextImpact() noexcept;
    void ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints() noexcept;
    void ClearBlendingKeyframes() noexcept;
    void ComputeCSSAnimationBlendingKeyframes(const RenderStyle &unanimatedStyle,
                                              const style::ResolutionContext &resolutionContext) noexcept;
    void ComputeCSSTransitionBlendingKeyframes(const RenderStyle &oldStyle,
                                               const RenderStyle &newStyle) noexcept;
    void SetBlendingKeyframes(BlendingKeyframes &&blendingKeyframes) noexcept;

    // AnimationEffect
    KRYS_NODISCARD bool IsKeyframeEffect() const noexcept final
    {
      return true;
    }
    void AnimationDidTick() noexcept final;
    void AnimationDidChangeTimingProperties() noexcept final;
    void AnimationWasCanceled() noexcept final;
    void AnimationSuspensionStateDidChange(bool animationIsSuspended) noexcept final;
    void AnimationPlaybackRateDidChange() noexcept final;
    void AnimationTimelineDidChange() noexcept final;
    void SetAnimation(WebAnimation *animation) noexcept final;
    KRYS_NODISCARD Seconds TimeToNextTick(const BasicEffectTiming &timing) noexcept final;
    KRYS_NODISCARD bool TicksContinuouslyWhileActive() const noexcept final;
    KRYS_NODISCARD Maybe<double> ProgressUntilNextStep(double iterationProgress) const noexcept final;
    KRYS_NODISCARD bool PreventsAnimationReadiness() const noexcept final;

    // KeyframeInterpolation
    KRYS_NODISCARD boo::CompositeOperation CompositeOperation() const noexcept final
    {
      return _compositeOperation;
    }
    KRYS_NODISCARD boo::IterationCompositeOperation IterationCompositeOperation() const noexcept final
    {
      return _iterationCompositeOperation;
    }
    KRYS_NODISCARD const KeyframeInterpolation::Keyframe &KeyframeAtIndex(size_t index) const noexcept final;
    KRYS_NODISCARD size_t NumberOfKeyframes() const noexcept final
    {
      return _blendingKeyframes.Size();
    }
    KRYS_NODISCARD const boo::TimingFunction *
      TimingFunctionForKeyframe(const KeyframeInterpolation::Keyframe &keyframe) const noexcept final;
    KRYS_NODISCARD bool
      IsPropertyAdditiveOrCumulative(KeyframeInterpolation::Property property) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_EFFECT(krys::boo::KeyframeEffect, IsKeyframeEffect())
