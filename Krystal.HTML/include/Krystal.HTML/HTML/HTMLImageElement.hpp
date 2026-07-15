#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlimageelement
  class HTMLImageElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLImageElement);

  public:
    HTMLImageElement(Document &document) noexcept;

#pragma region HTMLImageElement - https://html.spec.whatwg.org/#htmlimageelement

    /// @see https://html.spec.whatwg.org/#dom-img-alt
    KRYS_NODISCARD DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-alt
    void Alt(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-src
    KRYS_NODISCARD ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-src
    ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-srcset
    KRYS_NODISCARD ExceptionOr<USVString> SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-srcset
    ExceptionOr<void> SrcSet(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    KRYS_NODISCARD DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    void Sizes(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-crossorigin
    KRYS_NODISCARD Maybe<DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-crossorigin
    void CrossOrigin(Maybe<DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-usemap
    KRYS_NODISCARD DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-usemap
    void UseMap(DOMString &&value) noexcept;

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
    KRYS_NODISCARD USVString CurrentSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-referrerpolicy
    void ReferrerPolicy(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-decoding
    KRYS_NODISCARD DOMString Decoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-decoding
    void Decoding(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-loading
    KRYS_NODISCARD DOMString Loading() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-loading
    void Loading(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-fetchpriority
    KRYS_NODISCARD DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-fetchpriority
    void FetchPriority(DOMString &&value) noexcept;

    // TODO(HTMLIMAGEELEMENT, HTML): Promise<undefined> decode();

#pragma endregion

#pragma region HTMLImageElement Obsolete members - https://html.spec.whatwg.org/#HTMLImageElement-partial

    /// @see https://html.spec.whatwg.org/#dom-img-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-lowsrc
    KRYS_NODISCARD ExceptionOr<USVString> LowSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-lowsrc
    ExceptionOr<void> LowSrc(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-vspace
    void VSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-longdesc
    KRYS_NODISCARD ExceptionOr<USVString> LongDesc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-longdesc
    ExceptionOr<void> LongDesc(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-border
    KRYS_NODISCARD DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-border
    void Border(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLImageElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLImageElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();