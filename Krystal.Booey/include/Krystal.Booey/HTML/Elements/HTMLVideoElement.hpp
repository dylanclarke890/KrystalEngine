/*
 * Copyright (C) 2007-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMediaElement.hpp"

namespace krys::boo
{
  // FIXME: The poster frame (its loading, events and renderer) and decoded video, with a host media player.
  /// @see https://html.spec.whatwg.org/multipage/media.html#htmlvideoelement
  class HTMLVideoElement : public HTMLMediaElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLVideoElement);

  public:
    HTMLVideoElement(Document &document) noexcept;

#pragma region HTMLVideoElement - https://html.spec.whatwg.org/multipage/media.html#htmlvideoelement

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-videowidth
    KRYS_NODISCARD static uint32 VideoWidth() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-videoheight
    KRYS_NODISCARD static uint32 VideoHeight() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-poster
    KRYS_NODISCARD dom::USVString Poster() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-poster
    void Poster(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-playsinline
    KRYS_NODISCARD bool PlaysInline() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/media.html#dom-video-playsinline
    void PlaysInline(bool value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLVideoElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLVideoElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
