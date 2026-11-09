/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Style/Values/Animations/Animations.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  struct ResolutionContext;
}

namespace krys::boo
{
  class KeyframeEffect;
  class RenderStyle;
  struct Settings;

  /// @brief The relevant keyframe effects targeting an element, or one of its pseudo-elements, sorted by
  /// composite order when they're applied.
  /// @see https://drafts.csswg.org/web-animations-1/#the-effect-stack
  // FIXME: Accelerated effects (threaded animations), once there are any.
  class KeyframeEffectStack
  {
    KRYS_NON_COPY_MOVABLE(KeyframeEffectStack);

  private:
    List<WeakPtr<KeyframeEffect>> _effects;
    HashSet<css::CSSOMString> _invalidCSSAnimationNames;
    AnimatableCSSPropertySet _acceleratedPropertiesOverriddenByCascade;
    Maybe<style::Animations> _cssAnimationList;
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

    /// @brief The `animation` list of the target's last style change event, which orders its CSS animations.
    KRYS_NODISCARD const Maybe<style::Animations> &CSSAnimationList() const noexcept
    {
      return _cssAnimationList;
    }
    void SetCSSAnimationList(Maybe<style::Animations> &&cssAnimationList) noexcept;

    KRYS_NODISCARD bool ContainsProperty(css::PropertyId property) const noexcept;
    KRYS_NODISCARD bool IsCurrentlyAffectingProperty(css::PropertyId property) const noexcept;
    KRYS_NODISCARD bool RequiresPseudoElement() const noexcept;
    /// @brief Applies the effects to @p targetStyle in composite order, collecting the properties they
    /// animate.
    EnumSet<AnimationImpact> ApplyKeyframeEffects(RenderStyle &targetStyle,
                                                  AnimatableCSSPropertySet &affectedProperties,
                                                  const RenderStyle *previousLastStyleChangeEventStyle,
                                                  const style::ResolutionContext &resolutionContext) noexcept;
    /// @brief The properties that CSS transitions animate and no other effect currently in effect does.
    KRYS_NODISCARD AnimatableCSSPropertySet PropertiesAnimatedOnlyByTransitions() noexcept;

    KRYS_NODISCARD bool HasMatchingEffect(const Func<bool(const KeyframeEffect &)> &function) const noexcept;

    /// @brief The animation names of the target's `animation` list that no `@keyframes` rule defines.
    void ClearInvalidCSSAnimationNames() noexcept;
    KRYS_NODISCARD bool HasInvalidCSSAnimationNames() const noexcept;
    KRYS_NODISCARD bool ContainsInvalidCSSAnimationName(const css::CSSOMString &name) const noexcept;
    void AddInvalidCSSAnimationName(const css::CSSOMString &name) noexcept;

    void EffectAbilityToBeAcceleratedDidChange(const KeyframeEffect &effect) noexcept;
    /// @brief Whether the effects can run accelerated: none prevents it, and no two animate the same
    /// accelerated property where one has an implicit keyframe for it.
    KRYS_NODISCARD bool AllowsAcceleration() const noexcept;

    void LastStyleChangeEventStyleDidChange(const RenderStyle *previousStyle,
                                            const RenderStyle *currentStyle) noexcept;
    void CascadeDidOverrideProperties(const AnimatableCSSPropertySet &overriddenProperties,
                                      const dom::Document &document) noexcept;

    KRYS_NODISCARD const AnimatableCSSPropertySet &AcceleratedPropertiesOverriddenByCascade() const noexcept
    {
      return _acceleratedPropertiesOverriddenByCascade;
    }

    void ApplyPendingAcceleratedActions() const noexcept;

    KRYS_NODISCARD bool HasAcceleratedEffects(const Settings &settings) const noexcept;

  private:
    void StartAcceleratedAnimationsIfPossible() noexcept;
    void StopAcceleratedAnimations() noexcept;
  };
}
