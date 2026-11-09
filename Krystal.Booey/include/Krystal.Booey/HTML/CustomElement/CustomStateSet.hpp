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

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::html
{
  /// @brief The custom states of a custom element, which `:state()` matches; set through its internals.
  /// @see https://html.spec.whatwg.org/#customstateset
  class CustomStateSet : public RefCounted<CustomStateSet>
  {
  private:
    ListHashSet<dom::DOMStringAtom> _states;
    WeakPtr<dom::Element> _element;

  public:
    explicit CustomStateSet(dom::Element &element) noexcept;
    ~CustomStateSet() noexcept;

    KRYS_NODISCARD static Ref<CustomStateSet> Create(dom::Element &element) noexcept
    {
      return CreateRef<CustomStateSet>(element);
    }

    /// @return Whether @p state wasn't in the set.
    bool Add(dom::DOMStringAtom state) noexcept;

    /// @return Whether @p state was in the set.
    bool Delete(dom::DOMStringAtom state) noexcept;

    void Clear() noexcept;

    KRYS_NODISCARD bool Has(dom::DOMStringAtom state) const noexcept;

    KRYS_NODISCARD size_t Size() const noexcept;

    /// @brief The states, in the order they were added.
    KRYS_NODISCARD const ListHashSet<dom::DOMStringAtom> &States() const noexcept
    {
      return _states;
    }
  };
}
