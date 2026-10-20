#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/Values/Animations/Animations.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderElement;
  class StyleOriginatedAnimation;
  class WebAnimation;
}

namespace krys::boo::style
{
  struct ResolutionContext;
  enum class IsInDisplayNoneTree : bool;

  using WeakStyleOriginatedAnimations = List<WeakPtr<StyleOriginatedAnimation>>;

  /// @brief An element, or one of its pseudo-elements: something that is styled.
  // FIXME: Scroll and view timelines, accelerated animations and view transitions, once there are any.
  struct Styleable
  {
    dom::Element &Element;
    Maybe<style::PseudoElementIdentifier> PseudoElementIdentifier;

    Styleable(dom::Element &element,
              const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
        : Element(element), PseudoElementIdentifier(pseudoElementIdentifier)
    {
    }

    KRYS_NODISCARD static const Styleable FromElement(dom::Element &element) noexcept
    {
      return Styleable(element, null);
    }

    KRYS_NODISCARD bool operator==(const Styleable &other) const noexcept
    {
      return &Element == &other.Element && PseudoElementIdentifier == other.PseudoElementIdentifier;
    }

    KRYS_NODISCARD Ref<dom::Element> ProtectedElement() const noexcept
    {
      return ShareRef(Element);
    }

    /// @brief The renderer of the element or of its pseudo-element.
    KRYS_NODISCARD RenderElement *Renderer() const noexcept;

    /// @brief The style with the effects of its keyframe effect stack applied, if it has any effects in
    /// effect.
    KRYS_NODISCARD UniquePtr<RenderStyle> ComputeAnimatedStyle() const noexcept;

    KRYS_NODISCARD boo::KeyframeEffectStack *GetKeyframeEffectStack() const noexcept
    {
      return Element.GetKeyframeEffectStack(PseudoElementIdentifier);
    }
    KRYS_NODISCARD boo::KeyframeEffectStack &EnsureKeyframeEffectStack() const noexcept
    {
      return Element.EnsureKeyframeEffectStack(PseudoElementIdentifier);
    }
    KRYS_NODISCARD bool HasKeyframeEffects() const noexcept
    {
      return Element.HasKeyframeEffects(PseudoElementIdentifier);
    }

    EnumSet<AnimationImpact> ApplyKeyframeEffects(RenderStyle &targetStyle,
                                                  AnimatableCSSPropertySet &affectedProperties,
                                                  const RenderStyle *previousLastStyleChangeEventStyle,
                                                  const ResolutionContext &resolutionContext) const noexcept;

    KRYS_NODISCARD const AnimationCollection *Animations() const noexcept
    {
      return Element.Animations(PseudoElementIdentifier);
    }
    KRYS_NODISCARD bool
      HasCompletedTransitionForProperty(const AnimatableCSSProperty &property) const noexcept
    {
      return Element.HasCompletedTransitionForProperty(PseudoElementIdentifier, property);
    }
    KRYS_NODISCARD bool HasRunningTransitionForProperty(const AnimatableCSSProperty &property) const noexcept
    {
      return Element.HasRunningTransitionForProperty(PseudoElementIdentifier, property);
    }
    KRYS_NODISCARD bool HasRunningTransitions() const noexcept
    {
      return Element.HasRunningTransitions(PseudoElementIdentifier);
    }
    KRYS_NODISCARD AnimationCollection &EnsureAnimations() const noexcept
    {
      return Element.EnsureAnimations(PseudoElementIdentifier);
    }
    KRYS_NODISCARD AnimatableCSSPropertyToTransitionMap &EnsureCompletedTransitionsByProperty() const noexcept
    {
      return Element.EnsureCompletedTransitionsByProperty(PseudoElementIdentifier);
    }
    KRYS_NODISCARD AnimatableCSSPropertyToTransitionMap &EnsureRunningTransitionsByProperty() const noexcept
    {
      return Element.EnsureRunningTransitionsByProperty(PseudoElementIdentifier);
    }

    KRYS_NODISCARD const RenderStyle *LastStyleChangeEventStyle() const noexcept
    {
      return Element.LastStyleChangeEventStyle(PseudoElementIdentifier);
    }
    void SetLastStyleChangeEventStyle(UniquePtr<const RenderStyle> &&style) const noexcept
    {
      Element.SetLastStyleChangeEventStyle(PseudoElementIdentifier, krys::move(style));
    }

    KRYS_NODISCARD bool HasPropertiesOverridenAfterAnimation() const noexcept
    {
      return Element.HasPropertiesOverridenAfterAnimation(PseudoElementIdentifier);
    }
    void SetHasPropertiesOverridenAfterAnimation(bool value) const noexcept
    {
      Element.SetHasPropertiesOverridenAfterAnimation(PseudoElementIdentifier, value);
    }

    KRYS_NODISCARD CSSAnimationCollection &AnimationsCreatedByMarkup() const noexcept
    {
      return Element.AnimationsCreatedByMarkup(PseudoElementIdentifier);
    }
    void SetAnimationsCreatedByMarkup(CSSAnimationCollection &&collection) const noexcept
    {
      Element.SetAnimationsCreatedByMarkup(PseudoElementIdentifier, krys::move(collection));
    }
    void KeyframesRuleDidChange() const noexcept
    {
      Element.KeyframesRuleDidChange(PseudoElementIdentifier);
    }

    /// @brief Recomputes the keyframes that use container units, since the element's query container changed.
    void QueryContainerDidChange() const noexcept;
    /// @brief Whether @p animations names an `@keyframes` rule that didn't exist when the CSS animations were
    /// last updated.
    KRYS_NODISCARD bool AnimationListContainsNewlyValidAnimation(const style::Animations &animations) const noexcept;

    void ElementWasRemoved() const noexcept;
    void WillChangeRenderer() const noexcept;

    void CancelStyleOriginatedAnimations() const noexcept;
    /// @brief Cancels the style-originated animations, silently for those in @p animationsToCancelSilently.
    void CancelStyleOriginatedAnimations(
      const WeakStyleOriginatedAnimations &animationsToCancelSilently) const noexcept;

    void AnimationWasAdded(WebAnimation &animation) const noexcept;
    void AnimationWasRemoved(WebAnimation &animation) const noexcept;
    void RemoveStyleOriginatedAnimationFromListsForOwningElement(WebAnimation &animation) const noexcept;

    /// @brief Creates, updates and cancels the CSS animations of a style change event.
    /// @see https://drafts.csswg.org/css-animations-1/#animations
    void UpdateCSSAnimations(const RenderStyle *currentStyle, const RenderStyle &afterChangeStyle,
                             const ResolutionContext &resolutionContext,
                             WeakStyleOriginatedAnimations &newStyleOriginatedAnimations,
                             IsInDisplayNoneTree isInDisplayNoneTree) const noexcept;
    /// @brief Starts, reverses and cancels the CSS transitions of a style change event.
    /// @see https://drafts.csswg.org/css-transitions-1/#starting
    void UpdateCSSTransitions(const RenderStyle &currentStyle, const RenderStyle &newStyle,
                              WeakStyleOriginatedAnimations &newStyleOriginatedAnimations) const noexcept;
  };

  /// @brief A weak reference to a `Styleable`.
  class WeakStyleable
  {
  private:
    WeakPtr<dom::Element> _element;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;

  public:
    WeakStyleable() noexcept = default;

    WeakStyleable(const Styleable &styleable) noexcept
        : _element(CreateWeakPtr(&styleable.Element)),
          _pseudoElementIdentifier(styleable.PseudoElementIdentifier)
    {
    }

    WeakStyleable &operator=(const Styleable &styleable) noexcept
    {
      _element = CreateWeakPtr(&styleable.Element);
      _pseudoElementIdentifier = styleable.PseudoElementIdentifier;
      return *this;
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return !!_element;
    }

    KRYS_NODISCARD bool operator==(const WeakStyleable &other) const noexcept = default;

    KRYS_NODISCARD Maybe<Styleable> GetStyleable() const noexcept
    {
      if (!_element)
      {
        return null;
      }
      return Styleable(*_element, _pseudoElementIdentifier);
    }

    KRYS_NODISCARD const WeakPtr<dom::Element> &GetElement() const noexcept
    {
      return _element;
    }

    KRYS_NODISCARD const Maybe<style::PseudoElementIdentifier> &GetPseudoElementIdentifier() const noexcept
    {
      return _pseudoElementIdentifier;
    }
  };
}
