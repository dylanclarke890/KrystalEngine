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

#include "Krystal.Booey/Animation/Timing/AnimationEffectPhase.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo
{
  class StyleOriginatedAnimationEvent;
  class TimingFunction;

  /// @brief An animation created by style: a CSS animation or a CSS transition, owned by the element whose
  /// style created it until scripts take it over.
  class StyleOriginatedAnimation : public WebAnimation
  {
  private:
    bool _wasPending {false};
    AnimationEffectPhase _previousPhase {AnimationEffectPhase::Idle};
    WeakPtr<dom::Element> _owningElement;
    Maybe<style::PseudoElementIdentifier> _owningPseudoElementIdentifier;
    double _previousIteration {0};

  protected:
    explicit StyleOriginatedAnimation(const style::Styleable &styleable) noexcept;

  public:
    ~StyleOriginatedAnimation() noexcept override;

    KRYS_NODISCARD bool IsStyleOriginatedAnimation() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD Maybe<style::Styleable> OwningElement() const noexcept;

    /// @brief Cancels the animation because the style that created it changed, disassociating it from its
    /// owning element.
    void CancelFromStyle(WebAnimation::Silently silently = WebAnimation::Silently::No) noexcept;

    void SetTimeline(RefPtr<AnimationTimeline> &&newTimeline) noexcept final;
    void Cancel(WebAnimation::Silently silently = WebAnimation::Silently::No) noexcept final;

    void Tick() noexcept override;

    KRYS_NODISCARD bool CanHaveGlobalPosition() noexcept final;

    // The Web Animations interface brings the animation up to date with its style first.
    KRYS_NODISCARD Maybe<WebAnimationTime> BindingsStartTime() const noexcept final;
    KRYS_NODISCARD Maybe<WebAnimationTime> BindingsCurrentTime() const noexcept final;
    KRYS_NODISCARD WebAnimation::PlayState BindingsPlayState() const noexcept final;
    KRYS_NODISCARD WebAnimation::ReplaceState BindingsReplaceState() const noexcept final;
    KRYS_NODISCARD bool BindingsPending() const noexcept final;
    void BindingsWhenReady(ReadyCallback onReady, ErrorCallback onError = nullptr) noexcept final;
    void BindingsWhenFinished(ReadyCallback onFinished, ErrorCallback onError = nullptr) noexcept final;
    dom::ExceptionOr<void> BindingsPlay() noexcept override;
    dom::ExceptionOr<void> BindingsPause() noexcept override;

    /// @brief Brings the animation up to date with the owning element's style.
    void FlushPendingStyleChanges() const noexcept;

    KRYS_NODISCARD virtual AnimationPlayState BackingAnimationPlayState() const noexcept = 0;
    KRYS_NODISCARD virtual boo::TimingFunction *BackingAnimationTimingFunction() const noexcept = 0;

  protected:
    void Initialize(const RenderStyle *oldStyle, const RenderStyle &newStyle,
                    const style::ResolutionContext &resolutionContext) noexcept;
    virtual void SyncPropertiesWithBackingAnimation() noexcept;
    KRYS_NODISCARD virtual Ref<StyleOriginatedAnimationEvent>
      CreateEvent(dom::DOMStringAtom eventType, Maybe<Seconds> scheduledTime, double elapsedTime,
                  const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept = 0;

  private:
    void DisassociateFromOwningElement() noexcept;
    KRYS_NODISCARD AnimationEffectPhase PhaseWithoutEffect() const noexcept;
    /// @see https://drafts.csswg.org/css-transitions-2/#transition-events
    void InvalidateDOMEvents(WebAnimationTime cancelationTime = Seconds {0}) noexcept;
    void EnqueueDOMEvent(dom::DOMStringAtom eventType, WebAnimationTime elapsedTime,
                         WebAnimationTime scheduledEffectTime) noexcept;
    KRYS_NODISCARD WebAnimationTime ComputeCancelationTime() const noexcept;
    KRYS_NODISCARD WebAnimationTime EffectTimeAtStart() const noexcept;
    KRYS_NODISCARD WebAnimationTime EffectTimeAtIteration(double iteration) const noexcept;
    KRYS_NODISCARD WebAnimationTime EffectTimeAtEnd() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_WEB_ANIMATION(krys::boo::StyleOriginatedAnimation, IsStyleOriginatedAnimation())
