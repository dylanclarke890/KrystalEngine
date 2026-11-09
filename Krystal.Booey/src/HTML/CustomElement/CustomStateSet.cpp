/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/CustomElement/CustomStateSet.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::html
{
  CustomStateSet::CustomStateSet(dom::Element &element) noexcept : _element(CreateWeakPtr(&element))
  {
  }

  CustomStateSet::~CustomStateSet() noexcept = default;

  bool CustomStateSet::Add(dom::DOMStringAtom state) noexcept
  {
    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (auto *element = _element.get())
    {
      styleInvalidation.emplace(*element, css::SelectorPseudoClass::State,
                                style::PseudoClassChangeInvalidation::AnyValue);
    }

    return _states.add(state).second;
  }

  bool CustomStateSet::Delete(dom::DOMStringAtom state) noexcept
  {
    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (auto *element = _element.get())
    {
      styleInvalidation.emplace(*element, css::SelectorPseudoClass::State,
                                style::PseudoClassChangeInvalidation::AnyValue);
    }

    return _states.remove(state);
  }

  void CustomStateSet::Clear() noexcept
  {
    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (auto *element = _element.get())
    {
      styleInvalidation.emplace(*element, css::SelectorPseudoClass::State,
                                style::PseudoClassChangeInvalidation::AnyValue);
    }

    _states.clear();
  }

  bool CustomStateSet::Has(dom::DOMStringAtom state) const noexcept
  {
    return _states.contains(state);
  }

  size_t CustomStateSet::Size() const noexcept
  {
    return _states.size();
  }
}
