#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLMediaElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlvideoelement
  class HTMLVideoElement : public HTMLMediaElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLVideoElement);

  public:
    HTMLVideoElement(dom::Document &document) noexcept;

#pragma region HTMLVideoElement - https://html.spec.whatwg.org/#htmlvideoelement

    /// @see https://html.spec.whatwg.org/#dom-video-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-videowidth
    KRYS_NODISCARD uint32 VideoWidth() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-videoheight
    KRYS_NODISCARD uint32 VideoHeight() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-poster
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Poster() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-poster
    dom::ExceptionOr<void> Poster(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-playsinline
    KRYS_NODISCARD bool PlaysInline() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-video-playsinline
    void PlaysInline(bool value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLVideoElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLVideoElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();