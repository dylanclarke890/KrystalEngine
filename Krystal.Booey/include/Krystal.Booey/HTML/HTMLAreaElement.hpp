#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlareaelement
  class HTMLAreaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAreaElement);

  private:
    UniquePtr<DOMTokenList> _relList;

  public:
    HTMLAreaElement(Document &document) noexcept;

#pragma region HTMLAreaElement - https://html.spec.whatwg.org/#htmlareaelement

    /// @see https://html.spec.whatwg.org/#dom-area-alt
    KRYS_NODISCARD dom::DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-alt
    void Alt(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-coords
    KRYS_NODISCARD dom::DOMString Coords() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-coords
    void Coords(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-shape
    KRYS_NODISCARD dom::DOMString Shape() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-shape
    void Shape(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-download
    KRYS_NODISCARD dom::DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-download
    void Download(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-ping
    KRYS_NODISCARD dom::ExceptionOr<USVString> Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-ping
    dom::ExceptionOr<void> Ping(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rel
    KRYS_NODISCARD dom::DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rel
    void Rel(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLAreaElement Obsolete members - https://html.spec.whatwg.org/#HTMLAreaElement-partial

    /// @see https://html.spec.whatwg.org/#dom-area-nohref
    KRYS_NODISCARD bool NoHref() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-nohref
    void NoHref(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLAreaElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLAreaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();