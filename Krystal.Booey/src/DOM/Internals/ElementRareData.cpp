/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/DOM/Internals/ElementRareData.hpp"
#include "Krystal.Booey/Animation/ElementAnimationRareData.hpp"
#include "Krystal.Booey/DOM/Collections/DatasetDOMStringMap.hpp"
#include "Krystal.Booey/DOM/Collections/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomStateSet.hpp"
#include "Krystal.Booey/Page/IntersectionObserver/IntersectionObserver.hpp"
#include "Krystal.Booey/Page/ResizeObserver/ResizeObserver.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo::dom
{
  ElementRareData::ElementRareData() noexcept = default;

  ElementRareData::~ElementRareData() noexcept
  {
    krys_debug_assert(!_beforePseudoElement);
    krys_debug_assert(!_afterPseudoElement);
  }

  void ElementRareData::SetIntersectionObserverData(UniquePtr<IntersectionObserverData> &&data) noexcept
  {
    _intersectionObserverData = krys::move(data);
  }

  void ElementRareData::SetResizeObserverData(UniquePtr<ResizeObserverData> &&data) noexcept
  {
    _resizeObserverData = krys::move(data);
  }

  void ElementRareData::SetPartList(UniquePtr<DOMTokenList> &&partList) noexcept
  {
    _partList = krys::move(partList);
  }

  void ElementRareData::SetDataset(UniquePtr<DatasetDOMStringMap> &&dataset) noexcept
  {
    _dataset = krys::move(dataset);
  }

  void ElementRareData::SetCustomElementReactionQueue(
    UniquePtr<html::CustomElementReactionQueue> &&queue) noexcept
  {
    _customElementReactionQueue = krys::move(queue);
  }

  void ElementRareData::SetCustomElement(UniquePtr<html::CustomElement> &&customElement) noexcept
  {
    _customElement = krys::move(customElement);
  }

  void ElementRareData::SetCustomStateSet(RefPtr<html::CustomStateSet> &&customStateSet) noexcept
  {
    _customStateSet = krys::move(customStateSet);
  }

  void ElementRareData::SetPopoverData(UniquePtr<PopoverData> &&popoverData) noexcept
  {
    _popoverData = krys::move(popoverData);
  }

  void ElementRareData::SetComputedStyle(UniquePtr<RenderStyle> style) noexcept
  {
    _computedStyle = krys::move(style);
  }

  void ElementRareData::SetDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept
  {
    _displayContentsOrNoneStyle = krys::move(style);
  }

  void ElementRareData::SetBeforePseudoElement(RefPtr<PseudoElement> &&pseudoElement) noexcept
  {
    krys_debug_assert(!_beforePseudoElement || !pseudoElement);
    _beforePseudoElement = krys::move(pseudoElement);
  }

  void ElementRareData::SetAfterPseudoElement(RefPtr<PseudoElement> &&pseudoElement) noexcept
  {
    krys_debug_assert(!_afterPseudoElement || !pseudoElement);
    _afterPseudoElement = krys::move(pseudoElement);
  }

  ElementAnimationRareData *ElementRareData::AnimationRareData(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    auto it = _animationRareData.find(pseudoElementIdentifier);
    return it == _animationRareData.end() ? nullptr : it->second.get();
  }

  ElementAnimationRareData &ElementRareData::EnsureAnimationRareData(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    auto it = _animationRareData.find(pseudoElementIdentifier);
    if (it == _animationRareData.end())
    {
      it =
        _animationRareData.emplace(pseudoElementIdentifier, CreateUnique<ElementAnimationRareData>()).first;
    }
    return *it->second;
  }
}
