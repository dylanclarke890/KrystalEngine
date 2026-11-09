/*
 * Copyright (C) 2020 Igalia S.L.
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

#include "Krystal.Booey/Page/IntersectionObserver/IntersectionObserver.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::html
{
  /// @brief Loads a document's lazily loaded images (`loading=lazy`) once they intersect the viewport, with
  /// an intersection observer the document makes on first use.
  /// @see https://html.spec.whatwg.org/#lazy-load-intersection-observer
  class LazyLoadImageObserver
  {
    KRYS_NON_COPY_MOVABLE(LazyLoadImageObserver);

  private:
    RefPtr<IntersectionObserver> _observer;

  public:
    LazyLoadImageObserver() noexcept = default;

    /// @brief Starts observing @p element, the image of a deferred load, with its document's observer.
    /// @see https://html.spec.whatwg.org/#start-intersection-observing-a-lazy-loading-element
    static void Observe(dom::Element &element) noexcept;
    /// @brief Stops observing @p element with @p document's observer.
    /// @see https://html.spec.whatwg.org/#stop-intersection-observing-a-lazy-loading-element
    static void Unobserve(dom::Element &element, dom::Document &document) noexcept;

    KRYS_NODISCARD bool IsObserved(dom::Element &element) const noexcept;

  private:
    KRYS_NODISCARD IntersectionObserver *GetIntersectionObserver(dom::Document &document) noexcept;
  };
}
