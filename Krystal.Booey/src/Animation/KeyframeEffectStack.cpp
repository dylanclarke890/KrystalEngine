#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Interpolation.hpp"
#include <algorithm>

namespace krys::boo
{
  KeyframeEffectStack::KeyframeEffectStack() noexcept = default;

  KeyframeEffectStack::~KeyframeEffectStack() noexcept = default;

  bool KeyframeEffectStack::AddEffect(KeyframeEffect &effect) noexcept
  {
    // To qualify for membership in an effect stack, an effect must have a target, an animation, a timeline
    // and be relevant. This method will be called in WebAnimation and KeyframeEffect as those properties
    // change.
    if (!effect.TargetStyleable() || !effect.Animation() || !effect.Animation()->IsRelevant())
    {
      return false;
    }

    krys_debug_assert(
      std::ranges::none_of(_effects, [&](const auto &existing) { return existing.get() == &effect; }));

    _effects.push_back(CreateWeakPtr(&effect));
    _isSorted = false;

    if (_effects.size() > 1 && effect.PreventsAcceleration())
    {
      StopAcceleratedAnimations();
    }

    effect.WasAddedToEffectStack();

    return true;
  }

  void KeyframeEffectStack::RemoveEffect(KeyframeEffect &effect) noexcept
  {
    auto it = std::ranges::find_if(_effects, [&](const auto &existing) { return existing.get() == &effect; });
    if (it == _effects.end())
    {
      return;
    }

    _effects.erase(it);
    effect.WasRemovedFromEffectStack();

    if (_effects.empty())
    {
      return;
    }

    if (!effect.CanBeAccelerated())
    {
      StartAcceleratedAnimationsIfPossible();
    }
  }

  bool
    KeyframeEffectStack::HasMatchingEffect(const Func<bool(const KeyframeEffect &)> &function) const noexcept
  {
    for (const auto &effect : _effects)
    {
      if (function(*effect))
      {
        return true;
      }
    }
    return false;
  }

  bool KeyframeEffectStack::ContainsProperty(css::PropertyId property) const noexcept
  {
    return HasMatchingEffect([property](const KeyframeEffect &effect)
                             { return effect.AnimatesProperty(property); });
  }

  bool KeyframeEffectStack::RequiresPseudoElement() const noexcept
  {
    return HasMatchingEffect([](const KeyframeEffect &effect) { return effect.RequiresPseudoElement(); });
  }

  bool KeyframeEffectStack::IsCurrentlyAffectingProperty(css::PropertyId property) const noexcept
  {
    return HasMatchingEffect(
      [property](const KeyframeEffect &effect)
      {
        return effect.IsCurrentlyAffectingProperty(property)
               || effect.IsRunningAcceleratedAnimationForProperty(property);
      });
  }

  const List<WeakPtr<KeyframeEffect>> &KeyframeEffectStack::SortedEffects() noexcept
  {
    if (!_isSorted && _effects.size() > 1)
    {
      std::ranges::stable_sort(_effects,
                               [](const auto &a, const auto &b)
                               {
                                 krys_assert(a->Animation() && b->Animation());
                                 return CompareAnimationsByCompositeOrder(*a->Animation(), *b->Animation());
                               });
      _isSorted = true;
    }
    return _effects;
  }

  void KeyframeEffectStack::SetCSSAnimationList(Maybe<style::Animations> &&cssAnimationList) noexcept
  {
    _cssAnimationList = krys::move(cssAnimationList);
    // Since the list of animation names has changed, the sorting order of the animation effects may have
    // changed as well.
    _isSorted = false;
  }

  EnumSet<AnimationImpact>
    KeyframeEffectStack::ApplyKeyframeEffects(RenderStyle &targetStyle,
                                              AnimatableCSSPropertySet &affectedProperties,
                                              const RenderStyle *previousLastStyleChangeEventStyle,
                                              const style::ResolutionContext &resolutionContext) noexcept
  {
    EnumSet<AnimationImpact> impact;

    const auto &previousStyle = previousLastStyleChangeEventStyle ? *previousLastStyleChangeEventStyle
                                                                  : RenderStyle::DefaultStyleSingleton();

    auto transformRelatedPropertyChanged = targetStyle.GetTranslate() != previousStyle.GetTranslate()
                                           || targetStyle.GetScale() != previousStyle.GetScale()
                                           || targetStyle.GetRotate() != previousStyle.GetRotate()
                                           || targetStyle.GetTransform() != previousStyle.GetTransform();

    auto unanimatedStyle = RenderStyle::Clone(targetStyle);

    // We iterate over a snapshot of the effect list as it may mutate during application.
    List<RefPtr<KeyframeEffect>> effects;
    for (const auto &effect : SortedEffects())
    {
      effects.push_back(ShareRefPtr(effect.get()));
    }

    for (const auto &effect : effects)
    {
      if (!effect || !effect->Animation())
      {
        continue;
      }

      auto keyframeRecomputationReason = effect->RecomputeKeyframesIfNecessary(
        previousLastStyleChangeEventStyle, unanimatedStyle, resolutionContext);

      auto wasOrWasAboutToRunAccelerated =
        effect->IsRunningAccelerated() || effect->IsAboutToRunAccelerated();

      Ref animation = ShareRef(*effect->Animation());
      impact.Add(animation->Resolve(targetStyle, resolutionContext));

      if (!wasOrWasAboutToRunAccelerated
          && (effect->IsRunningAccelerated() || effect->IsAboutToRunAccelerated()))
      {
        impact.Add(AnimationImpact::RequiresRecomposite);
      }

      if (effect->TriggersStackingContext())
      {
        impact.Add(AnimationImpact::ForcesStackingContext);
      }

      if (transformRelatedPropertyChanged && effect->IsRunningAcceleratedTransformRelatedAnimation())
      {
        effect->TransformRelatedPropertyDidChange();
      }

      // If one of the effect's resolved property changed it could affect whether that effect's animation is
      // removed.
      if (keyframeRecomputationReason
          && *keyframeRecomputationReason == KeyframeEffect::RecomputationReason::LogicalPropertyChange)
      {
        if (RefPtr timeline = ShareRefPtr(animation->Timeline()))
        {
          timeline->AnimationTimingDidChange(*animation);
        }
      }

      for (const auto &property : effect->AnimatedProperties())
      {
        affectedProperties.insert(property);
      }
    }

    return impact;
  }

  void KeyframeEffectStack::ClearInvalidCSSAnimationNames() noexcept
  {
    _invalidCSSAnimationNames.clear();
  }

  bool KeyframeEffectStack::HasInvalidCSSAnimationNames() const noexcept
  {
    return !_invalidCSSAnimationNames.empty();
  }

  bool KeyframeEffectStack::ContainsInvalidCSSAnimationName(const css::CSSOMString &name) const noexcept
  {
    return _invalidCSSAnimationNames.contains(name);
  }

  void KeyframeEffectStack::AddInvalidCSSAnimationName(const css::CSSOMString &name) noexcept
  {
    _invalidCSSAnimationNames.insert(name);
  }

  void KeyframeEffectStack::EffectAbilityToBeAcceleratedDidChange(const KeyframeEffect &effect) noexcept
  {
    krys_debug_assert(
      std::ranges::any_of(_effects, [&](const auto &existing) { return existing.get() == &effect; }));
    if (effect.PreventsAcceleration())
    {
      StopAcceleratedAnimations();
    }
    else
    {
      StartAcceleratedAnimationsIfPossible();
    }
  }

  bool KeyframeEffectStack::AllowsAcceleration() const noexcept
  {
    // We could try and be a lot smarter here and do this on a per-property basis and
    // account for fully replacing effects which could co-exist with effects that
    // don't support acceleration lower in the stack, etc. But, if we are not able to run
    // all effects that could support acceleration using acceleration, then we might
    // as well not run any at all since we'll be updating effects for this stack
    // for each animation frame. So for now, we simply return false if any effect in the
    // stack is unable to be accelerated, or if we have more than one effect animating
    // an accelerated property with an implicit keyframe.

    AnimatableCSSPropertySet allAcceleratedProperties;

    for (const auto &effect : _effects)
    {
      if (effect->PreventsAcceleration())
      {
        return false;
      }
      const auto &acceleratedProperties = effect->AcceleratedProperties();
      if (!allAcceleratedProperties.empty())
      {
        for (const auto &property : acceleratedProperties)
        {
          if (allAcceleratedProperties.contains(property)
              && effect->AcceleratedPropertiesWithImplicitKeyframe().contains(property))
          {
            return false;
          }
        }
      }

      allAcceleratedProperties.insert(acceleratedProperties.begin(), acceleratedProperties.end());
    }

    return true;
  }

  void KeyframeEffectStack::StartAcceleratedAnimationsIfPossible() noexcept
  {
    if (!AllowsAcceleration())
    {
      return;
    }

    for (const auto &effect : _effects)
    {
      effect->EffectStackNoLongerPreventsAcceleration();
    }
  }

  void KeyframeEffectStack::StopAcceleratedAnimations() noexcept
  {
    for (const auto &effect : _effects)
    {
      effect->EffectStackNoLongerAllowsAcceleration();
    }
  }

  void KeyframeEffectStack::LastStyleChangeEventStyleDidChange(const RenderStyle *previousStyle,
                                                               const RenderStyle *currentStyle) noexcept
  {
    for (const auto &effect : _effects)
    {
      effect->LastStyleChangeEventStyleDidChange(previousStyle, currentStyle);
    }
  }

  void KeyframeEffectStack::CascadeDidOverrideProperties(const AnimatableCSSPropertySet &overriddenProperties,
                                                         const dom::Document &document) noexcept
  {
    AnimatableCSSPropertySet acceleratedPropertiesOverriddenByCascade;
    for (const auto &animatedProperty : overriddenProperties)
    {
      if (style::interpolation::IsAccelerated(animatedProperty, document.GetSettings()))
      {
        acceleratedPropertiesOverriddenByCascade.insert(animatedProperty);
      }
    }

    if (acceleratedPropertiesOverriddenByCascade == _acceleratedPropertiesOverriddenByCascade)
    {
      return;
    }

    _acceleratedPropertiesOverriddenByCascade = krys::move(acceleratedPropertiesOverriddenByCascade);

    for (const auto &effect : _effects)
    {
      effect->AcceleratedPropertiesOverriddenByCascadeDidChange();
    }
  }

  void KeyframeEffectStack::ApplyPendingAcceleratedActions() const noexcept
  {
    bool hasActiveAcceleratedEffect =
      std::ranges::any_of(_effects,
                          [](const auto &effect)
                          {
                            return effect->CanBeAccelerated()
                                   && effect->Animation()->GetPlayState() == WebAnimation::PlayState::Running;
                          });

    auto accelerationWasPrevented = false;

    for (const auto &effect : _effects)
    {
      if (hasActiveAcceleratedEffect)
      {
        effect->ApplyPendingAcceleratedActionsOrUpdateTimingProperties();
      }
      else
      {
        effect->ApplyPendingAcceleratedActions();
      }
      accelerationWasPrevented =
        accelerationWasPrevented || effect->AccelerationWasPrevented() || effect->PreventsAcceleration();
    }

    if (accelerationWasPrevented)
    {
      for (const auto &effect : _effects)
      {
        effect->EffectStackNoLongerAllowsAccelerationDuringAcceleratedActionApplication();
      }
    }
  }

  bool KeyframeEffectStack::HasAcceleratedEffects(const Settings &) const noexcept
  {
    return HasMatchingEffect([](const auto &effect) { return effect.IsRunningAccelerated(); });
  }
}
