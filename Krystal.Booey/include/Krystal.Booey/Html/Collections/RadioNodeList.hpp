/*
 * Copyright (c) 2012 Motorola Mobility, Inc. All rights reserved.
 * Copyright (C) 2014-2020 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY MOTOROLA MOBILITY, INC. AND ITS CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MOTOROLA MOBILITY, INC. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Dom/Collections/LiveNodeList.hpp"
#include "Krystal.Booey/Dom/Types/DOMString.hpp"

namespace krys::boo
{
  class ContainerNode;
  class Element;
}

namespace krys::boo::html
{
  /// @brief Which of a form's elements a RadioNodeList holds.
  enum class RadioNodeListCandidates : uint8
  {
    ListedElements,
    Images,
  };

  /// @brief The listed elements of a form with an ID or name, as its named getter returns them when there are
  /// several.
  /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#radionodelist
  class RadioNodeList final : public LiveNodeList
  {
  private:
    Ref<ContainerNode> _ownerNode;
    DOMString _name;
    RadioNodeListCandidates _candidates;

  public:
    RadioNodeList(ContainerNode &ownerNode, DOMStringView name,
                  RadioNodeListCandidates candidates = RadioNodeListCandidates::ListedElements) noexcept;

    ~RadioNodeList() noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#dom-radionodelist-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#dom-radionodelist-value
    void SetValue(DOMStringView value) noexcept;

    KRYS_NODISCARD bool ElementMatches(const Element &element) const noexcept;
  };
}
