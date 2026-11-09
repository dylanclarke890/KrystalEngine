/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/ElementAnimationRareData.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo
{
  ElementAnimationRareData::ElementAnimationRareData() noexcept = default;

  ElementAnimationRareData::~ElementAnimationRareData() noexcept
  {
    krys_debug_assert(!_keyframeEffectStack || !_keyframeEffectStack->HasEffects());
  }

  KeyframeEffectStack &ElementAnimationRareData::EnsureKeyframeEffectStack() noexcept
  {
    if (!_keyframeEffectStack)
    {
      _keyframeEffectStack = CreateUnique<boo::KeyframeEffectStack>();
    }
    return *_keyframeEffectStack;
  }

  void ElementAnimationRareData::SetAnimationsCreatedByMarkup(CSSAnimationCollection &&animations) noexcept
  {
    if (_keyframeEffectStack)
    {
      for (const auto &animation : _animationsCreatedByMarkup)
      {
        if (RefPtr keyframeEffect = ShareRefPtr(animation->GetKeyframeEffect()))
        {
          _keyframeEffectStack->RemoveEffect(*keyframeEffect);
        }
      }
    }

    _animationsCreatedByMarkup = krys::move(animations);
  }

  void ElementAnimationRareData::SetLastStyleChangeEventStyle(UniquePtr<const RenderStyle> &&style) noexcept
  {
    if (_keyframeEffectStack && _lastStyleChangeEventStyle != style)
    {
      auto previousStyleChangeEventStyle = std::exchange(_lastStyleChangeEventStyle, krys::move(style));
      _keyframeEffectStack->LastStyleChangeEventStyleDidChange(previousStyleChangeEventStyle.get(),
                                                               _lastStyleChangeEventStyle.get());
      return;
    }

    _lastStyleChangeEventStyle = krys::move(style);
  }
}
