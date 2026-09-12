#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
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
    KRYS_NODISCARD dom::DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-download
    void Download(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-ping
    KRYS_NODISCARD dom::ExceptionOr<USVString> Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-ping
    dom::ExceptionOr<void> Ping(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rel
    KRYS_NODISCARD dom::DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rel
    void Rel(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-text
    KRYS_NODISCARD dom::DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-text
    dom::ExceptionOr<void> Text(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLAnchorElement Obsolete members - https://html.spec.whatwg.org/#HTMLAnchorElement-partial

    /// @see https://html.spec.whatwg.org/#dom-a-coords
    KRYS_NODISCARD dom::DOMString Coords() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-coords
    void Coords(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-charset
    KRYS_NODISCARD dom::DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-charset
    void Charset(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rev
    KRYS_NODISCARD dom::DOMString Rev() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rev
    void Rev(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-shape
    KRYS_NODISCARD dom::DOMString Shape() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-shape
    void Shape(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLAnchorElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLAnchorElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();