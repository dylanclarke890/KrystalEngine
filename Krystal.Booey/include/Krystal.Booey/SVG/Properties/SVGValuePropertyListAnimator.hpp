/*
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGPropertyAnimator.hpp"

namespace krys::boo::svg
{
  /// @brief Animates a presentation attribute whose value is a list (`stroke-dasharray`).
  template <typename TList, typename TAnimationFunction>
  class SVGValuePropertyListAnimator : public SVGPropertyAnimator<TAnimationFunction>
  {
    using Base = SVGPropertyAnimator<TAnimationFunction>;
    using Base::_function;
    using Base::ApplyAnimatedStylePropertyChange;

  protected:
    using Base::_attributeName;
    using Base::ComputeCSSPropertyValue;

    RefPtr<TList> _list;

  public:
    template <typename... TArgs>
    SVGValuePropertyListAnimator(const dom::QualifiedName &attributeName, Ref<SVGProperty> &&property,
                                 TArgs &&...arguments) noexcept
        : Base(attributeName, std::forward<TArgs>(arguments)...),
          _list(ShareRef(static_cast<TList &>(*property)))
    {
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept override
    {
      _function.Animate(targetElement, progress, repeatCount, _list);
    }

    void Apply(SVGElement &targetElement) noexcept override
    {
      ApplyAnimatedStylePropertyChange(targetElement, _list->ValueAsString());
    }
  };
}
