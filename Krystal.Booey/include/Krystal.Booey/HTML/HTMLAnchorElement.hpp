#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/URLDecomposition.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlanchorelement
  class HTMLAnchorElement : public HTMLElement, public URLDecomposition
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAnchorElement);

  private:
    UniquePtr<dom::DOMTokenList> _relList;

  public:
    /// @brief Links start a selection only when editable.
    KRYS_NODISCARD bool CanStartSelection() const noexcept final;

    HTMLAnchorElement(dom::Document &document) noexcept;

    /// @brief Follows the link, if it is one.
    /// @see https://html.spec.whatwg.org/#the-a-element:activation-behaviour
    void ActivationBehavior(dom::Event &event) noexcept override;

    /// @brief Follows a focused link on Enter (as a simulated click), and on a click of the middle button.
    void DefaultEventHandler(dom::Event &event) noexcept override;

    /// @brief A link is interactive content.
    KRYS_NODISCARD bool IsInteractiveContent() const noexcept override;

#pragma region HTMLAnchorElement - https://html.spec.whatwg.org/#htmlanchorelement

    /// @see https://html.spec.whatwg.org/#dom-a-target
    KRYS_NODISCARD dom::DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-target
    void Target(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-download
    KRYS_NODISCARD dom::DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-download
    void Download(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-ping
    KRYS_NODISCARD dom::USVString Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-ping
    void Ping(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rel
    KRYS_NODISCARD dom::DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rel
    void Rel(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rellist
    KRYS_NODISCARD dom::DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-hreflang
    KRYS_NODISCARD dom::DOMString HrefLang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-hreflang
    void HrefLang(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-text
    KRYS_NODISCARD dom::DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-text
    dom::ExceptionOr<void> Text(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLHyperlinkElementUtils - https://html.spec.whatwg.org/#htmlhyperlinkelementutils

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    void Href(dom::USVString &&value) noexcept;

    /// @brief The `href` stringifier.
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString ToString() const noexcept;

    using URLDecomposition::Protocol;

    /// @brief Changes the link's scheme, unless its URL is invalid.
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-protocol
    void Protocol(dom::DOMStringView value) noexcept;

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

  private:
    KRYS_NODISCARD url::URL FullURL() const noexcept final;
    void SetFullURL(const url::URL &url) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLAnchorElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLAnchorElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLAnchorElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
