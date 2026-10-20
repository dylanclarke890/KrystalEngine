#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::style
{
  struct ResolutionContext;
}

namespace krys::boo
{
  class KeyframeEffect;
  class RenderStyle;

  /// @brief The relevant keyframe effects targeting an element, or one of its pseudo-elements, sorted by
  /// composite order when they're applied.
  /// @see https://drafts.csswg.org/web-animations-1/#the-effect-stack
  // FIXME: CSS animation names (the `animation-name` list and invalid names), and accelerated animations,
  // once there are CSS animations and accelerated animations.
  class KeyframeEffectStack
  {
    KRYS_NON_COPY_MOVABLE(KeyframeEffectStack);

  private:
    List<WeakPtr<KeyframeEffect>> _effects;
    bool _isSorted {true};

  public:
    KeyframeEffectStack() noexcept;
    ~KeyframeEffectStack() noexcept;

    /// @return Whether the effect qualified for membership: it has a target, an animation and is relevant.
    bool AddEffect(KeyframeEffect &effect) noexcept;
    void RemoveEffect(KeyframeEffect &effect) noexcept;
    KRYS_NODISCARD bool HasEffects() const noexcept
    {
      return !_effects.empty();
    }
    KRYS_NODISCARD const List<WeakPtr<KeyframeEffect>> &SortedEffects() noexcept;

    KRYS_NODISCARD bool ContainsProperty(css::PropertyId property) const noexcept;
    KRYS_NODISCARD bool IsCurrentlyAffectingProperty(css::PropertyId property) const noexcept;
    KRYS_NODISCARD bool RequiresPseudoElement() const noexcept;
    /// @brief Applies the effects to @p targetStyle in composite order, collecting the properties they
    /// animate.
    EnumSet<AnimationImpact> ApplyKeyframeEffects(RenderStyle &targetStyle,
                                                  AnimatableCSSPropertySet &affectedProperties,
                                                  const RenderStyle *previousLastStyleChangeEventStyle,
                                                  const style::ResolutionContext &resolutionContext) noexcept;
    KRYS_NODISCARD bool HasMatchingEffect(const Func<bool(const KeyframeEffect &)> &function) const noexcept;
  };
}
