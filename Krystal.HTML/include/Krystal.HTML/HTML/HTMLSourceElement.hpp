#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlsourceelement
  class HTMLSourceElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSourceElement);

  public:
    HTMLSourceElement(Document &document) noexcept;

#pragma region HTMLSourceElement - https://html.spec.whatwg.org/#htmlsourceelement

    /// @see https://html.spec.whatwg.org/#dom-source-src
    KRYS_NODISCARD ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-src
    ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-type
    void Type(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-srcset
    KRYS_NODISCARD ExceptionOr<USVString> SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-srcset
    ExceptionOr<void> SrcSet(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-sizes
    KRYS_NODISCARD DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-sizes
    void Sizes(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-media
    KRYS_NODISCARD DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-media
    void Media(DOMString &&value) noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLSourceElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLSourceElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
