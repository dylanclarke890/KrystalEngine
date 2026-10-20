#pragma once

#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class CSSTransition;
  class KeyframeEffectStack;
  class RenderStyle;

  using AnimatableCSSPropertyToTransitionMap =
    HashMap<AnimatableCSSProperty, Ref<CSSTransition>, AnimatableCSSPropertyHash>;

  /// @brief The animation state of an element, or of one of its pseudo-elements.
  // FIXME: The CSS animations created by markup and their pending keyframes updates, once there are CSS
  // animations.
  class ElementAnimationRareData
  {
    KRYS_NON_COPY_MOVABLE(ElementAnimationRareData);

  private:
    UniquePtr<boo::KeyframeEffectStack> _keyframeEffectStack;
    UniquePtr<const RenderStyle> _lastStyleChangeEventStyle;
    AnimationCollection _animations;
    AnimatableCSSPropertyToTransitionMap _completedTransitionsByProperty;
    AnimatableCSSPropertyToTransitionMap _runningTransitionsByProperty;
    bool _hasPropertiesOverridenAfterAnimation {false};

  public:
    ElementAnimationRareData() noexcept;
    ~ElementAnimationRareData() noexcept;

    KRYS_NODISCARD boo::KeyframeEffectStack *KeyframeEffectStack() const noexcept
    {
      return _keyframeEffectStack.get();
    }
    KRYS_NODISCARD boo::KeyframeEffectStack &EnsureKeyframeEffectStack() noexcept;

    KRYS_NODISCARD AnimationCollection &Animations() noexcept
    {
      return _animations;
    }

    KRYS_NODISCARD AnimatableCSSPropertyToTransitionMap &CompletedTransitionsByProperty() noexcept
    {
      return _completedTransitionsByProperty;
    }
    KRYS_NODISCARD AnimatableCSSPropertyToTransitionMap &RunningTransitionsByProperty() noexcept
    {
      return _runningTransitionsByProperty;
    }

    /// @brief The style of the last style change event, before animations were applied.
    /// @see https://drafts.csswg.org/css-transitions-1/#before-change-style
    KRYS_NODISCARD const RenderStyle *LastStyleChangeEventStyle() const noexcept
    {
      return _lastStyleChangeEventStyle.get();
    }
    void SetLastStyleChangeEventStyle(UniquePtr<const RenderStyle> &&style) noexcept;

    KRYS_NODISCARD bool HasPropertiesOverridenAfterAnimation() const noexcept
    {
      return _hasPropertiesOverridenAfterAnimation;
    }
    void SetHasPropertiesOverridenAfterAnimation(bool value) noexcept
    {
      _hasPropertiesOverridenAfterAnimation = value;
    }
  };
}
