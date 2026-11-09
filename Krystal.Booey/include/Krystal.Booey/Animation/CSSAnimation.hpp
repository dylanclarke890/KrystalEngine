/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Animation/StyleOriginatedAnimation.hpp"
#include "Krystal.Booey/Animation/Timing/OptionalEffectTiming.hpp"
#include "Krystal.Booey/Style/Scope/ScopedName.hpp"
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
    void SetBindingsRangeStart(TimelineRangeValue &&range) noexcept final;
    void SetBindingsRangeEnd(TimelineRangeValue &&range) noexcept final;
    dom::ExceptionOr<void> BindingsPlay() noexcept final;
    dom::ExceptionOr<void> BindingsPause() noexcept final;
    void SetBindingsEffect(RefPtr<AnimationEffect> &&newEffect) noexcept final;
    dom::ExceptionOr<void> SetBindingsStartTime(const Maybe<WebAnimationTime> &startTime) noexcept final;
    dom::ExceptionOr<void> BindingsReverse() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_WEB_ANIMATION(krys::boo::CSSAnimation, IsCSSAnimation())
