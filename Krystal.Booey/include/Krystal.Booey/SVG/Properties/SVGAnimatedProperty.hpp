/*
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwner.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief An element's animatable attribute, with its base value and the value an animation gives it.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGAnimatedLength
  class SVGAnimatedProperty : public RefCounted<SVGAnimatedProperty>, public SVGPropertyOwner
  {
  protected:
    WeakPtr<SVGElement> _contextElement;
    List<WeakPtr<SVGAttributeAnimator>> _animators;

  public:
    ~SVGAnimatedProperty() noexcept override = default;

    // Manage the relationship with the owner.
    KRYS_NODISCARD bool IsAttached() const noexcept
    {
      return !!_contextElement;
    }

    void Detach() noexcept
    {
      _contextElement = nullptr;
    }

    KRYS_NODISCARD SVGElement *ContextElement() const noexcept;

    KRYS_NODISCARD virtual dom::DOMString BaseValAsString() const noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual dom::DOMString AnimValAsString() const noexcept
    {
      return {};
    }

    // Control the synchronization between the attribute and its reflection in baseVal.
    KRYS_NODISCARD virtual bool IsDirty() const noexcept
    {
      return false;
    }

    virtual void SetDirty() noexcept
    {
    }

    virtual Maybe<dom::DOMString> Synchronize() noexcept
    {
      return {};
    }

    // Control the animation life cycle.
    KRYS_NODISCARD bool IsAnimating() const noexcept
    {
      return std::ranges::any_of(_animators, [](const auto &animator) { return !!animator; });
    }

    virtual void StartAnimation(SVGAttributeAnimator &animator) noexcept
    {
      std::erase_if(_animators, [](const auto &weakAnimator) { return !weakAnimator; });
      if (std::ranges::none_of(_animators,
                               [&](const auto &weakAnimator) { return weakAnimator.get() == &animator; }))
      {
        _animators.push_back(CreateWeakPtr(&animator));
      }
    }

    virtual void StopAnimation(SVGAttributeAnimator &animator) noexcept
    {
      std::erase_if(_animators, [&](const auto &weakAnimator)
                    { return !weakAnimator || weakAnimator.get() == &animator; });
    }

    // Attach/Detach the animVal of the target element's property by the instance element's property.
    virtual void InstanceStartAnimation(SVGAttributeAnimator &animator, SVGAnimatedProperty &) noexcept
    {
      StartAnimation(animator);
    }

    virtual void InstanceStopAnimation(SVGAttributeAnimator &animator) noexcept
    {
      StopAnimation(animator);
    }

  protected:
    explicit SVGAnimatedProperty(SVGElement *) noexcept;

    KRYS_NODISCARD SVGPropertyOwner *Owner() const noexcept override;
    void CommitPropertyChange(SVGProperty *) noexcept override;
  };
}
