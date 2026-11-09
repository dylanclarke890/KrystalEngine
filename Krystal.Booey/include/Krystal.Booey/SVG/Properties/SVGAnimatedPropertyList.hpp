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

#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <utility>

namespace krys::boo::svg
{
  /// @brief An animated list (of lengths, numbers, points or transforms).
  template <typename TList>
  class SVGAnimatedPropertyList : public SVGAnimatedProperty
  {
  protected:
    Ref<TList> _baseVal;
    mutable RefPtr<TList> _animVal;

  public:
    template <typename... TArgs>
    KRYS_NODISCARD static Ref<SVGAnimatedPropertyList> Create(SVGElement *contextElement,
                                                              TArgs &&...arguments) noexcept
    {
      return AdoptRef(*new SVGAnimatedPropertyList(contextElement, std::forward<TArgs>(arguments)...));
    }

    ~SVGAnimatedPropertyList() noexcept override
    {
      _baseVal->Detach();
      if (_animVal)
      {
        _animVal->Detach();
      }
    }

    // Used by the DOM.
    KRYS_NODISCARD const Ref<TList> &BaseVal() const noexcept
    {
      return _baseVal;
    }

    // Used by SVGElement::AttributeChanged().
    KRYS_NODISCARD Ref<TList> &BaseVal() noexcept
    {
      return _baseVal;
    }

    // Used by the DOM.
    KRYS_NODISCARD const RefPtr<TList> &AnimVal() const noexcept
    {
      return const_cast<SVGAnimatedPropertyList *>(this)->EnsureAnimVal();
    }

    // Called by SVGAnimatedPropertyAnimator to pass the animVal to the SVGAnimationFunction::progress.
    KRYS_NODISCARD RefPtr<TList> &AnimVal() noexcept
    {
      return EnsureAnimVal();
    }

    // Used when committing a change from the SVGAnimatedProperty to the attribute.
    KRYS_NODISCARD dom::DOMString BaseValAsString() const noexcept override
    {
      return _baseVal->ValueAsString();
    }

    // Used to apply the SVGAnimator change to the target element.
    KRYS_NODISCARD dom::DOMString AnimValAsString() const noexcept override
    {
      krys_assert(IsAnimating());
      return _animVal->ValueAsString();
    }

    // Managing the relationship with the owner.
    void SetDirty() noexcept override
    {
      _baseVal->SetDirty();
    }

    KRYS_NODISCARD bool IsDirty() const noexcept override
    {
      return _baseVal->IsDirty();
    }

    Maybe<dom::DOMString> Synchronize() noexcept override
    {
      return _baseVal->Synchronize();
    }

    // Used by RenderSVGElements.
    KRYS_NODISCARD const TList &CurrentValue() const noexcept
    {
      krys_assert(!IsAnimating() || _animVal);
      return IsAnimating() ? *_animVal : *_baseVal;
    }

    // Controlling the animation.
    void StartAnimation(SVGAttributeAnimator &animator) noexcept override
    {
      if (_animVal)
      {
        *_animVal = *_baseVal;
      }
      else
      {
        EnsureAnimVal();
      }
      SVGAnimatedProperty::StartAnimation(animator);
    }

    void StopAnimation(SVGAttributeAnimator &animator) noexcept override
    {
      SVGAnimatedProperty::StopAnimation(animator);
      if (_animVal)
      {
        *_animVal = *_baseVal;
      }
    }

    // Controlling the instance animation.
    void InstanceStartAnimation(SVGAttributeAnimator &animator,
                                SVGAnimatedProperty &animated) noexcept override
    {
      if (!IsAnimating())
      {
        _animVal = static_cast<SVGAnimatedPropertyList &>(animated).AnimVal();
      }
      SVGAnimatedProperty::InstanceStartAnimation(animator, animated);
    }

    void InstanceStopAnimation(SVGAttributeAnimator &animator) noexcept override
    {
      SVGAnimatedProperty::InstanceStopAnimation(animator);
      if (!IsAnimating())
      {
        _animVal = nullptr;
      }
    }

  protected:
    template <typename... TArgs>
    SVGAnimatedPropertyList(SVGElement *contextElement, TArgs &&...arguments) noexcept
        : SVGAnimatedProperty(contextElement),
          _baseVal(TList::Create(this, SVGPropertyAccess::ReadWrite, std::forward<TArgs>(arguments)...))
    {
    }

    RefPtr<TList> &EnsureAnimVal() noexcept
    {
      if (!_animVal)
      {
        _animVal = TList::Create(*_baseVal, SVGPropertyAccess::ReadOnly);
      }
      return _animVal;
    }

    // Called when _baseVal changes or an item in _baseVal changes.
    void CommitPropertyChange(SVGProperty *property) noexcept override
    {
      if (_animVal)
      {
        *_animVal = *_baseVal;
      }
      SVGAnimatedProperty::CommitPropertyChange(property);
    }
  };
}
