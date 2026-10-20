#pragma once

#include "Krystal.Booey/Animation/StyleOriginatedAnimation.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/Animations/Animation.hpp"

namespace krys::boo
{
  /// @brief An animation created by an element's `animation` properties from an `@keyframes` rule.
  /// @see https://drafts.csswg.org/css-animations-2/#cssanimation
  // FIXME: The properties scripts override (`effectTimingWasUpdatedUsingBindings` and the other bindings
  // overrides), named, scroll and view timelines, and the animation range, once there are scripting and
  // scroll-driven animations.
  class CSSAnimation final : public StyleOriginatedAnimation
  {
  private:
    style::ScopedName _animationName;
    Maybe<AnimationPlayState> _lastStyleOriginatedPlayState;
    style::Animation _backingStyleAnimation;

    CSSAnimation(const style::Styleable &styleable, style::ScopedName &&animationName,
                 style::Animation &&backingStyleAnimation) noexcept;

  public:
    ~CSSAnimation() noexcept override;

    /// @attention @p backingStyleAnimation must name keyframes (not `none`).
    KRYS_NODISCARD static Ref<CSSAnimation>
      Create(const style::Styleable &owningElement, style::Animation &&backingStyleAnimation,
             const RenderStyle *oldStyle, const RenderStyle &newStyle,
             const style::ResolutionContext &resolutionContext) noexcept;

    KRYS_NODISCARD bool IsCSSAnimation() const noexcept final
    {
      return true;
    }

    /// @see https://drafts.csswg.org/css-animations-2/#dom-cssanimation-animationname
    KRYS_NODISCARD const css::CSSOMStringAtom &AnimationName() const noexcept
    {
      return _animationName.Name;
    }
    KRYS_NODISCARD const style::ScopedName &ScopedAnimationName() const noexcept
    {
      return _animationName;
    }

    /// @brief Recomputes the keyframes, since the `@keyframes` rule they're from changed.
    void KeyframesRuleDidChange() noexcept;
    void UpdateKeyframesIfNeeded(const RenderStyle *oldStyle, const RenderStyle &newStyle,
                                 const style::ResolutionContext &resolutionContext) noexcept;
    void SyncStyleOriginatedTimeline() noexcept;

    KRYS_NODISCARD const style::Animation &BackingStyleAnimation() const noexcept
    {
      return _backingStyleAnimation;
    }
    void SetBackingStyleAnimation(const style::Animation &backingStyleAnimation) noexcept;

  private:
    void SyncPropertiesWithBackingAnimation() noexcept final;
    KRYS_NODISCARD AnimationPlayState BackingAnimationPlayState() const noexcept final;
    KRYS_NODISCARD boo::TimingFunction *BackingAnimationTimingFunction() const noexcept final;
    KRYS_NODISCARD Ref<StyleOriginatedAnimationEvent>
      CreateEvent(dom::DOMStringAtom eventType, Maybe<Seconds> scheduledTime, double elapsedTime,
                  const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_WEB_ANIMATION(krys::boo::CSSAnimation, IsCSSAnimation())
