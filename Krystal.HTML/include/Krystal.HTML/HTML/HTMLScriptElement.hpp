#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlscriptelement
  class HTMLScriptElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLScriptElement);

  private:
    UniquePtr<DOMTokenList> _blocking;

  public:
    HTMLScriptElement(Document &document) noexcept;

#pragma region HTMLScriptElement - https://html.spec.whatwg.org/#htmlscriptelement

    /// @see https://html.spec.whatwg.org/#dom-script-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-type
    void Type(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-src
    KRYS_NODISCARD ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-src
    ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-nomodule
    KRYS_NODISCARD bool NoModule() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-nomodule
    void NoModule(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-async
    KRYS_NODISCARD bool Async() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-async
    void Async(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-defer
    KRYS_NODISCARD bool Defer() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-defer
    void Defer(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-blocking
    KRYS_NODISCARD DOMTokenList &Blocking() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-crossorigin
    KRYS_NODISCARD Maybe<DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-crossorigin
    void CrossOrigin(Maybe<DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-referrerpolicy
    void ReferrerPolicy(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-integrity
    KRYS_NODISCARD DOMString Integrity() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-integrity
    void Integrity(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-fetchpriority
    KRYS_NODISCARD DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-fetchpriority
    void FetchPriority(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-text
    void Text(DOMString &&value) noexcept;

    // TODO(HTMLSCRIPTELEMENT, HTML): Implement Supports().
    // static boolean supports(DOMString type);

#pragma endregion

#pragma region HTMLScriptElement Obsolete members  - https://html.spec.whatwg.org/#HTMLScriptElement-partial

    /// @see https://html.spec.whatwg.org/#dom-script-charset
    KRYS_NODISCARD DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-charset
    void Charset(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-event
    KRYS_NODISCARD DOMString Event() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-event
    void Event(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-htmlfor
    KRYS_NODISCARD DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-htmlfor
    void HtmlFor(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLScriptElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLScriptElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();