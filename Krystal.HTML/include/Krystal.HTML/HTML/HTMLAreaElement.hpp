#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/USVString.hpp"

namespace Krys::HTML
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
    KRYS_NODISCARD DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-alt
    void Alt(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-coords
    KRYS_NODISCARD DOMString Coords() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-coords
    void Coords(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-shape
    KRYS_NODISCARD DOMString Shape() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-shape
    void Shape(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-download
    KRYS_NODISCARD DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-download
    void Download(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-ping
    KRYS_NODISCARD ExceptionOr<USVString> Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-ping
    ExceptionOr<void> Ping(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rel
    void Rel(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-referrerpolicy
    void ReferrerPolicy(DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLAreaElement Obsolete members - https://html.spec.whatwg.org/#HTMLAreaElement-partial

    /// @see https://html.spec.whatwg.org/#dom-area-nohref
    KRYS_NODISCARD bool NoHref() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-nohref
    void NoHref(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLAreaElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLAreaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();