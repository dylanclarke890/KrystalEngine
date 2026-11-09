/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/CSSOM/StyleAttributeMutationScope.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  namespace
  {
    uint32 scopeCount = 0;
    StyleAttributeMutationScope *currentScope = nullptr;
  }

  StyleAttributeMutationScope::StyleAttributeMutationScope(dom::Element *element) noexcept
      : _element(ShareRefPtr(element))
  {
    ++scopeCount;

    if (scopeCount != 1)
    {
      return;
    }

    krys_debug_assert(!currentScope);
    currentScope = this;

    if (!_element)
    {
      return;
    }

    bool shouldReadOldValue =
      dom::MutationObserverAlgorithms::IsAttributeOldValueRequested(*_element, u8"style");

    if (_element->IsDefinedCustomElement()) KRYS_UNLIKELY
    {
      auto *reactionQueue = _element->ReactionQueue();
      if (reactionQueue && reactionQueue->ObservesStyleAttribute())
      {
        _isCustomElement = true;
        shouldReadOldValue = true;
      }
    }

    if (shouldReadOldValue)
    {
      _oldValue = _element->GetAttribute(u8"style");
    }
  }

  StyleAttributeMutationScope::~StyleAttributeMutationScope() noexcept
  {
    --scopeCount;
    if (scopeCount)
    {
      return;
    }
    krys_debug_assert(currentScope == this);
    currentScope = nullptr;

    if (!_shouldDeliver || !_element)
    {
      return;
    }

    dom::MutationObserverAlgorithms::QueueMutationRecord(
      dom::MutationRecordType::Attributes, _element, u8"style", dom::DOMStringAtom::Null(),
      _oldValue ? dom::DOMStringAtom(*_oldValue) : dom::DOMStringAtom::Null(), {}, {}, nullptr, nullptr);

    if (_isCustomElement)
    {
      auto newValue = _element->GetAttribute(u8"style");
      html::CustomElementReactionQueue::EnqueueAttributeChangedCallbackIfNeeded(
        *_element, dom::QualifiedName(dom::DOMStringAtom::Null(), dom::DOMStringAtom::Null(), u8"style"),
        _oldValue ? dom::DOMStringAtom(*_oldValue) : dom::DOMStringAtom::Null(),
        newValue ? dom::DOMStringAtom(*newValue) : dom::DOMStringAtom::Null());
    }
  }

  void StyleAttributeMutationScope::EnqueueMutationRecord() noexcept
  {
    currentScope->_shouldDeliver = true;
  }
}
