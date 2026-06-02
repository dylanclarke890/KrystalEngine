#pragma once

#include "Krystal.HTML/DOM/DOMTokenList.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmllinkelement
  class HTMLLinkElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLinkElement);

  private:
    /// @see https://html.spec.whatwg.org/#explicitly-enabled
    bool _explicitlyEnabled {false};

  public:
    HTMLLinkElement(Document &document) noexcept;

#pragma region HTMLLinkElement - https://html.spec.whatwg.org/#htmllinkelement

    /// @see https://html.spec.whatwg.org/#dom-link-href
    KRYS_NODISCARD ExceptionOr<USVString> Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-href
    ExceptionOr<void> Href(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-crossorigin
    KRYS_NODISCARD Maybe<DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-crossorigin
    void CrossOrigin(Maybe<DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rel
    void Rel(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-as
    KRYS_NODISCARD DOMString As() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-as
    void As(DOMString &&value) noexcept;

    // TODO(HTMLLINKELEMENT, HTML): Implement the rel DOMTokenList method.
    /// @see https://html.spec.whatwg.org/#dom-link-rellist
    // [SameObject, PutForwards=value, Reflect="rel"] readonly attribute DOMTokenList relList;

    /// @see https://html.spec.whatwg.org/#dom-link-media
    KRYS_NODISCARD DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-media
    void Media(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-integrity
    KRYS_NODISCARD DOMString Integrity() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-integrity
    void Integrity(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-hreflang
    KRYS_NODISCARD DOMString HrefLang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-hreflang
    void HrefLang(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-type
    void Type(DOMString &&value) noexcept;

    // TODO(HTMLLINKELEMENT, HTML): Implement the sizes DOMTokenList method.
    /// @see https://html.spec.whatwg.org/#dom-link-sizes
    // [SameObject, PutForwards=value, Reflect] readonly attribute DOMTokenList sizes;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesrcset
    KRYS_NODISCARD ExceptionOr<USVString> ImageSrcset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesrcset
    ExceptionOr<void> ImageSrcset(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesizes
    KRYS_NODISCARD DOMString ImageSizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesizes
    void ImageSizes(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-referrerpolicy
    void ReferrerPolicy(DOMString &&value) noexcept;

    // TODO(HTMLLINKELEMENT, HTML): Implement the blocking DOMTokenList method.
    /// @see https://html.spec.whatwg.org/#dom-link-blocking
    // [SameObject, PutForwards=value, Reflect] readonly attribute DOMTokenList blocking;

    /// @see https://html.spec.whatwg.org/#dom-link-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-fetchpriority
    KRYS_NODISCARD DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-fetchpriority
    void FetchPriority(DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLLinkElement Obsolete members - https://html.spec.whatwg.org/#HTMLLinkElement-partial

    /// @see https://html.spec.whatwg.org/#dom-link-charset
    KRYS_NODISCARD DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-charset
    void Charset(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rev
    KRYS_NODISCARD DOMString Rev() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rev
    void Rev(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-target
    void Target(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLLinkElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLLinkElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();