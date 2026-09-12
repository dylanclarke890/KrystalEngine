#pragma once

#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmllinkelement
  class HTMLLinkElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLinkElement);

  private:
    UniquePtr<DOMTokenList> _relList;
    UniquePtr<DOMTokenList> _sizes;
    UniquePtr<DOMTokenList> _blocking;

    /// @see https://html.spec.whatwg.org/#explicitly-enabled
    bool _explicitlyEnabled {false};

  public:
    HTMLLinkElement(Document &document) noexcept;

#pragma region HTMLLinkElement - https://html.spec.whatwg.org/#htmllinkelement

    /// @see https://html.spec.whatwg.org/#dom-link-href
    KRYS_NODISCARD dom::ExceptionOr<USVString> Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-href
    dom::ExceptionOr<void> Href(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-crossorigin
    KRYS_NODISCARD Maybe<dom::DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-crossorigin
    void CrossOrigin(Maybe<dom::DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rel
    KRYS_NODISCARD dom::DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rel
    void Rel(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-as
    KRYS_NODISCARD dom::DOMString As() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-as
    void As(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-media
    KRYS_NODISCARD dom::DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-media
    void Media(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-integrity
    KRYS_NODISCARD dom::DOMString Integrity() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-integrity
    void Integrity(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-hreflang
    KRYS_NODISCARD dom::DOMString HrefLang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-hreflang
    void HrefLang(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-sizes
    KRYS_NODISCARD DOMTokenList &Sizes() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesrcset
    KRYS_NODISCARD dom::ExceptionOr<USVString> ImageSrcset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesrcset
    dom::ExceptionOr<void> ImageSrcset(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesizes
    KRYS_NODISCARD dom::DOMString ImageSizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesizes
    void ImageSizes(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-blocking
    KRYS_NODISCARD DOMTokenList &Blocking() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-fetchpriority
    KRYS_NODISCARD dom::DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-fetchpriority
    void FetchPriority(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLLinkElement Obsolete members - https://html.spec.whatwg.org/#HTMLLinkElement-partial

    /// @see https://html.spec.whatwg.org/#dom-link-charset
    KRYS_NODISCARD dom::DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-charset
    void Charset(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rev
    KRYS_NODISCARD dom::DOMString Rev() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rev
    void Rev(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-target
    KRYS_NODISCARD dom::DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-target
    void Target(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLLinkElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLLinkElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();