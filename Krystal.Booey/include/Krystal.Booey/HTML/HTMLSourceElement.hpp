#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlsourceelement
  class HTMLSourceElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSourceElement);

  public:
    HTMLSourceElement(Document &document) noexcept;

#pragma region HTMLSourceElement - https://html.spec.whatwg.org/#htmlsourceelement

    /// @see https://html.spec.whatwg.org/#dom-source-src
    KRYS_NODISCARD dom::ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-src
    dom::ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-srcset
    KRYS_NODISCARD dom::ExceptionOr<USVString> SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-srcset
    dom::ExceptionOr<void> SrcSet(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-sizes
    KRYS_NODISCARD dom::DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-sizes
    void Sizes(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-media
    KRYS_NODISCARD dom::DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-media
    void Media(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-height
    void Height(uint32 value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLSourceElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLSourceElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
