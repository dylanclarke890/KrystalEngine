/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2015, 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/DOM/Collections/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo::dom
{
  DOMTokenList::DOMTokenList(Element &element, DOMStringAtom attributeName,
                             IsSupportedTokenFunction &&isSupportedToken) noexcept
      : _attributeName {attributeName}, _element {ShareCheckedRef(element)},
        _isSupportedToken {krys::move(isSupportedToken)}
  {
    // 1. Let element be set’s element.
    // 2. Let attributeName be set’s attribute name.
    // 3. Let value be the result of getting an attribute value given element and attributeName.
    // 4. Run the attribute change steps for element, attributeName, value, value, and null.
    UpdateTokensFromAttributeValue();
  }

  void DOMTokenList::UpdateTokensFromAttributeValue() const noexcept
  {
    // A missing attribute's value is the empty string, which has no tokens.
    auto tokens = OrderedSet::Parser(ElementAlgorithms::GetAttributeValue(*_element, _attributeName));
    _tokens.clear();
    _tokens.append(tokens.begin(), tokens.end());
    _tokensNeedUpdating = false;
  }

  void DOMTokenList::Value(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    ElementAlgorithms::SetAttributeValue(*_element, _attributeName, krys::move(value));
  }

  ExceptionOr<bool> DOMTokenList::ValidationSteps(DOMStringView token) const noexcept
  {
    // 1. If set’s element and attribute name does not define supported tokens, then throw a TypeError.
    if (!_isSupportedToken)
    {
      return Exception {ExceptionCode::TypeError};
    }

    // 2. Let lowercaseToken be token, in ASCII lowercase.
    auto lowercaseToken = krys::text::ToASCIILower(token);

    // 3. If lowercaseToken is present in the supported tokens of set’s element and attribute name, then
    // return true.
    // 4. Return false.
    return _isSupportedToken(_element->NodeDocument(), lowercaseToken);
  }

  void DOMTokenList::UpdateSteps() noexcept
  {
    // 1. If get an attribute by namespace and local name given null, set’s attribute name, and set’s element
    // returns
    //    null and set’s token set is empty, then return.
    auto attr = ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom::Null(), _attributeName, *_element);
    if (attr == nullptr && _tokens.empty())
    {
      return;
    }

    // 2. Set an attribute value given set’s element, set’s attribute name, and the result of running the
    // ordered set
    //    serializer for set’s token set.
    SetForScope inAttributeUpdate(_inUpdateAssociatedAttributeFromTokens, true);
    ElementAlgorithms::SetAttributeValue(*_element, _attributeName, OrderedSet::Serialiser(_tokens));
  }

  DOMString DOMTokenList::SerialiseSteps() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*_element, _attributeName);
  }
}
