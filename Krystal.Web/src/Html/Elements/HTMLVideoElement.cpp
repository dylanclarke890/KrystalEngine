/*
 * Copyright (C) 2007-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Html/Elements/HTMLVideoElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/Internals/PresentationalHints.hpp"

namespace krys::web
{
  HTMLVideoElement::HTMLVideoElement(Document &document) noexcept
      : HTMLMediaElement(document, html::HTMLElementInterface::Video)
  {
  }

#pragma region HTMLVideoElement

  uint32 HTMLVideoElement::Width() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"width");
  }

  void HTMLVideoElement::Width(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLVideoElement::Height() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"height");
  }

  void HTMLVideoElement::Height(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

  uint32 HTMLVideoElement::VideoWidth() noexcept
  {
    // 1. If this's readyState attribute is HAVE_NOTHING, then return 0.
    // 2. Return the natural width of the video in CSS pixels.
    // FIXME: The video's natural width, once media elements play media; until then they have nothing.
    return 0u;
  }

  uint32 HTMLVideoElement::VideoHeight() noexcept
  {
    // 1. If this's readyState attribute is HAVE_NOTHING, then return 0.
    // 2. Return the natural height of the video in CSS pixels.
    // FIXME: The video's natural height, once media elements play media; until then they have nothing.
    return 0u;
  }

  dom::USVString HTMLVideoElement::Poster() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"poster");
  }

  void HTMLVideoElement::Poster(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"poster", krys::move(value));
  }

  bool HTMLVideoElement::PlaysInline() const noexcept
  {
    return html::Reflect<bool>(*this, u8"playsinline");
  }

  void HTMLVideoElement::PlaysInline(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"playsinline", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLVideoElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                            const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"width" || localName == u8"height"))
    {
      return true;
    }
    return HTMLMediaElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLVideoElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                                const DOMStringAtom &namespaceURI,
                                                                DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
    {
      HTMLMediaElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"width")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
      ApplyAspectRatioFromWidthAndHeightAttributesToStyle(value, AttributeOrEmpty(u8"height"), style);
      return;
    }
    if (localName == u8"height")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      ApplyAspectRatioFromWidthAndHeightAttributesToStyle(AttributeOrEmpty(u8"width"), value, style);
      return;
    }
    HTMLMediaElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
