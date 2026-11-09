/*
 * Copyright (C) 2019-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveFunction.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <utility>

namespace krys::boo::svg
{
  /// @brief An additive animation function over a list type, interpolating item by item.
  template <typename TList>
  class SVGAnimationAdditiveListFunction : public SVGAnimationAdditiveFunction
  {
  protected:
    const Ref<TList> _from;
    const Ref<TList> _to;
    const Ref<TList> _toAtEndOfDuration;

  public:
    template <typename... TArgs>
    SVGAnimationAdditiveListFunction(AnimationMode animationMode, CalcMode calcMode, bool isAccumulated,
                                     bool isAdditive, TArgs &&...arguments) noexcept
        : SVGAnimationAdditiveFunction(animationMode, calcMode, isAccumulated, isAdditive),
          _from(TList::Create(std::forward<TArgs>(arguments)...)),
          _to(TList::Create(std::forward<TArgs>(arguments)...)),
          _toAtEndOfDuration(TList::Create(std::forward<TArgs>(arguments)...))
    {
    }

  protected:
    KRYS_NODISCARD const Ref<TList> &ToAtEndOfDuration() const noexcept
    {
      return !_toAtEndOfDuration->IsEmpty() ? _toAtEndOfDuration : _to;
    }

    bool AdjustAnimatedList(AnimationMode animationMode, float percentage, RefPtr<TList> &animated,
                            bool resizeAnimatedIfNeeded = true) noexcept
    {
      if (!_to->NumberOfItems())
      {
        return false;
      }

      if (_from->NumberOfItems() && _from->Size() != _to->Size())
      {
        if (percentage >= 0.5)
        {
          *animated = *_to;
        }
        else if (animationMode != AnimationMode::To)
        {
          *animated = *_from;
        }
        return false;
      }

      if (resizeAnimatedIfNeeded && animated->Size() < _to->Size())
      {
        animated->Resize(_to->Size());
      }
      return true;
    }
  };
}
