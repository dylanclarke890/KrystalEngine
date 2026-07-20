#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlanchorelement
  class HTMLAnchorElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAnchorElement);

  private:
    UniquePtr<DOMTokenList> _relList;

  public:
    HTMLAnchorElement(Document &document) noexcept;

#pragma region HTMLAnchorElement - https://html.spec.whatwg.org/#htmlanchorelement

    /// @see https://html.spec.whatwg.org/#dom-a-download
    KRYS_NODISCARD DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-download
    void Download(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-ping
    KRYS_NODISCARD ExceptionOr<USVString> Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-ping
    ExceptionOr<void> Ping(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rel
    void Rel(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-text
    ExceptionOr<void> Text(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-referrerpolicy
    void ReferrerPolicy(DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLAnchorElement Obsolete members - https://html.spec.whatwg.org/#HTMLAnchorElement-partial

    /// @see https://html.spec.whatwg.org/#dom-a-coords
    KRYS_NODISCARD DOMString Coords() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-coords
    void Coords(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-charset
    KRYS_NODISCARD DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-charset
    void Charset(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rev
    KRYS_NODISCARD DOMString Rev() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rev
    void Rev(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-shape
    KRYS_NODISCARD DOMString Shape() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-shape
    void Shape(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLAnchorElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLAnchorElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();