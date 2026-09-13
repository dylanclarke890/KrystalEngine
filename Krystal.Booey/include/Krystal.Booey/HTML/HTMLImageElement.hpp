#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlimageelement
  class HTMLImageElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLImageElement);

  public:
    HTMLImageElement(dom::Document &document) noexcept;

#pragma region HTMLImageElement - https://html.spec.whatwg.org/#htmlimageelement

    /// @see https://html.spec.whatwg.org/#dom-img-alt
    KRYS_NODISCARD dom::DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-alt
    void Alt(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-src
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-src
    dom::ExceptionOr<void> Src(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-srcset
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-srcset
    dom::ExceptionOr<void> SrcSet(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    KRYS_NODISCARD dom::DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    void Sizes(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-crossorigin
    KRYS_NODISCARD Maybe<dom::DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-crossorigin
    void CrossOrigin(Maybe<dom::DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-usemap
    KRYS_NODISCARD dom::DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-usemap
    void UseMap(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-ismap
    KRYS_NODISCARD bool IsMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-ismap
    void IsMap(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-controls
    KRYS_NODISCARD bool Controls() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-controls
    void Controls(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-naturalwidth
    KRYS_NODISCARD uint32 NaturalWidth() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-naturalheight
    KRYS_NODISCARD uint32 NaturalHeight() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-complete
    KRYS_NODISCARD bool Complete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-currentsrc
    KRYS_NODISCARD dom::USVString CurrentSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-decoding
    KRYS_NODISCARD dom::DOMString Decoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-decoding
    void Decoding(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-loading
    KRYS_NODISCARD dom::DOMString Loading() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-loading
    void Loading(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-fetchpriority
    KRYS_NODISCARD dom::DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-fetchpriority
    void FetchPriority(dom::DOMString &&value) noexcept;

    // TODO(HTMLIMAGEELEMENT, HTML): Promise<undefined> decode();

#pragma endregion

#pragma region HTMLImageElement Obsolete members - https://html.spec.whatwg.org/#HTMLImageElement-partial

    /// @see https://html.spec.whatwg.org/#dom-img-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-lowsrc
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> LowSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-lowsrc
    dom::ExceptionOr<void> LowSrc(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-vspace
    void VSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-longdesc
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> LongDesc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-longdesc
    dom::ExceptionOr<void> LongDesc(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-border
    KRYS_NODISCARD dom::DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-border
    void Border(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLImageElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLImageElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();