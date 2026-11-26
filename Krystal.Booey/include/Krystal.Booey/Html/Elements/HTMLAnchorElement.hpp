#pragma once

#include "Krystal.Booey/Dom/Types/USVString.hpp"
#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Html/Mixins/UrlDecomposition.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#htmlanchorelement
  class HTMLAnchorElement : public HTMLElement, public html::UrlDecomposition
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAnchorElement);

  private:
    UniquePtr<DOMTokenList> _relList;
    /// @brief Whether the shift key was down when the mouse was last pressed on the link, for a drag it
    /// starts.
    bool _wasShiftKeyDownOnMouseDown {false};

  public:
    /// @brief Links start a selection only when editable.
    KRYS_NODISCARD bool CanStartSelection() const noexcept final;

    HTMLAnchorElement(Document &document) noexcept;

    ~HTMLAnchorElement() noexcept override;

    /// @brief Follows the link, if it is one.
    /// @see https://html.spec.whatwg.org/#the-a-element:activation-behaviour
    void ActivationBehavior(Event &event) noexcept override;

    /// @brief Follows a focused link on Enter (as a simulated click), and on a click of the middle button.
    void DefaultEventHandler(Event &event) noexcept override;

    /// @brief Whether the link is followed by a click with or without @p shiftKey: always outside editable
    /// content, and inside it as `Settings::EditableLinkBehavior` says.
    KRYS_NODISCARD bool TreatLinkAsLive(bool shiftKey) const noexcept;

    /// @brief Whether the element is a link that acts as one for the last mouse press on it.
    KRYS_NODISCARD bool IsLiveLink() const noexcept;

    /// @brief A link is interactive content.
    KRYS_NODISCARD bool IsInteractiveContent() const noexcept override;

#pragma region HTMLAnchorElement - https://html.spec.whatwg.org/multipage/text-level-semantics.html#htmlanchorelement

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-a-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-a-target
    void Target(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-download
    KRYS_NODISCARD DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-download
    void Download(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-ping
    KRYS_NODISCARD dom::USVString Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-ping
    void Ping(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-rel
    void Rel(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-a-hreflang
    KRYS_NODISCARD DOMString HrefLang() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-a-hreflang
    void HrefLang(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-a-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-a-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-text
    ExceptionOr<void> Text(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-a-referrerpolicy
    void ReferrerPolicy(DOMString value) noexcept;

#pragma endregion

#pragma region HTMLHyperlinkElementUtils - https://html.spec.whatwg.org/multipage/links.html#htmlhyperlinkelementutils

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString Href() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-href
    void Href(dom::USVString value) noexcept;

    /// @brief The `href` stringifier.
    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString ToString() const noexcept;

#pragma endregion

#pragma region HTMLAnchorElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLAnchorElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-coords
    KRYS_NODISCARD DOMString Coords() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-coords
    void Coords(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-charset
    KRYS_NODISCARD DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-charset
    void Charset(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-rev
    KRYS_NODISCARD DOMString Rev() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-rev
    void Rev(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-shape
    KRYS_NODISCARD DOMString Shape() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-a-shape
    void Shape(DOMString value) noexcept;

#pragma endregion

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    KRYS_NODISCARD net::Url FullUrl() const noexcept final;

    void SetFullUrl(const net::Url &url) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLAnchorElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLAnchorElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLAnchorElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
