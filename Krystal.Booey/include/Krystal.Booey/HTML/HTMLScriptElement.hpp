#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlscriptelement
  class HTMLScriptElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLScriptElement);

    friend class HTMLTreeBuilder;

  private:
    UniquePtr<dom::DOMTokenList> _blocking;
    dom::Document *_parserDocument {nullptr};
    bool _forceAsync : 1 {true};
    bool _alreadyStarted : 1 {false};

  public:
    HTMLScriptElement(dom::Document &document) noexcept;

#pragma region HTMLScriptElement - https://html.spec.whatwg.org/#htmlscriptelement

    /// @see https://html.spec.whatwg.org/#dom-script-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-src
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-src
    dom::ExceptionOr<void> Src(dom::USVString &&value) noexcept;

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
    KRYS_NODISCARD dom::DOMTokenList &Blocking() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-crossorigin
    KRYS_NODISCARD Maybe<dom::DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-crossorigin
    void CrossOrigin(Maybe<dom::DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-integrity
    KRYS_NODISCARD dom::DOMString Integrity() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-integrity
    void Integrity(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-fetchpriority
    KRYS_NODISCARD dom::DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-fetchpriority
    void FetchPriority(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-text
    KRYS_NODISCARD dom::DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-text
    void Text(dom::DOMString &&value) noexcept;

    // TODO(HTMLSCRIPTELEMENT, HTML): Implement Supports().
    // static boolean supports(dom::DOMString type);

#pragma endregion

#pragma region HTMLScriptElement Obsolete members  - https://html.spec.whatwg.org/#HTMLScriptElement-partial

    /// @see https://html.spec.whatwg.org/#dom-script-charset
    KRYS_NODISCARD dom::DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-charset
    void Charset(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-event
    KRYS_NODISCARD dom::DOMString Event() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-event
    void Event(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-htmlfor
    KRYS_NODISCARD dom::DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-script-htmlfor
    void HtmlFor(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLScriptElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLScriptElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLScriptElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();