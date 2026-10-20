#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
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
    return HasMatchingEffect([property](const KeyframeEffect &effect)
                             { return effect.IsCurrentlyAffectingProperty(property); });
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
    // Since the list of animation names has changed, the sorting order of the animation effects may have changed
    // as well.
    _isSorted = false;
  }

  EnumSet<AnimationImpact>
    KeyframeEffectStack::ApplyKeyframeEffects(RenderStyle &targetStyle,
                                              AnimatableCSSPropertySet &affectedProperties,
                                              const RenderStyle *previousLastStyleChangeEventStyle,
                                              const style::ResolutionContext &resolutionContext) noexcept
  {
    EnumSet<AnimationImpact> impact;

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

      Ref animation = ShareRef(*effect->Animation());
      impact.Add(animation->Resolve(targetStyle, resolutionContext));

      if (effect->TriggersStackingContext())
      {
        impact.Add(AnimationImpact::ForcesStackingContext);
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
}
