#pragma once

#include "Krystal.Booey/Animation/OptionalEffectTiming.hpp"
#include "Krystal.Booey/Animation/StyleOriginatedAnimation.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/Animations/Animation.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  /// @brief A CSS animation's property that stops following the style once it's changed through the Web
  /// Animations interfaces.
  enum class CSSAnimationProperty : uint8
  {
    Name,
    Duration,
    TimingFunction,
    IterationCount,
    Direction,
    PlayState,
    Delay,
    FillMode,
    Keyframes,
    CompositeOperation,
    Timeline,
    RangeStart,
    RangeEnd,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::CSSAnimationProperty, 13uz);

namespace krys::boo
{
  /// @brief An animation created by an element's `animation` properties from an `@keyframes` rule.
  /// @see https://drafts.csswg.org/css-animations-2/#cssanimation
  // FIXME: Named, scroll and view timelines, and the animation range, once there are scroll-driven
  // animations.
  class CSSAnimation final : public StyleOriginatedAnimation
  {
    using Property = CSSAnimationProperty;

  private:
    style::ScopedName _animationName;
    EnumSet<Property> _overriddenProperties;
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

    void EffectTimingWasUpdatedUsingBindings(const OptionalEffectTiming &timing) noexcept;
    void EffectKeyframesWereSetUsingBindings() noexcept;
    void EffectCompositeOperationWasSetUsingBindings() noexcept;
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

    KRYS_NODISCARD AnimationTimeline *BindingsTimeline() const noexcept final;
    void SetBindingsTimeline(RefPtr<AnimationTimeline> &&timeline) noexcept final;
    dom::ExceptionOr<void> BindingsPlay() noexcept final;
    dom::ExceptionOr<void> BindingsPause() noexcept final;
    void SetBindingsEffect(RefPtr<AnimationEffect> &&newEffect) noexcept final;
    dom::ExceptionOr<void> SetBindingsStartTime(const Maybe<WebAnimationTime> &startTime) noexcept final;
    dom::ExceptionOr<void> BindingsReverse() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_WEB_ANIMATION(krys::boo::CSSAnimation, IsCSSAnimation())
