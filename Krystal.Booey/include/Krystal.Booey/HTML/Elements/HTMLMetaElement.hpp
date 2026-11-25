#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlmetaelement
  class HTMLMetaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMetaElement);

  public:
    HTMLMetaElement(Document &document) noexcept;

#pragma region HTMLMetaElement - https://html.spec.whatwg.org/multipage/semantics.html#htmlmetaelement

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-meta-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-meta-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-meta-httpequiv
    KRYS_NODISCARD DOMString HttpEquiv() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-meta-httpequiv
    void HttpEquiv(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-meta-content
    KRYS_NODISCARD DOMString Content() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-meta-content
    void Content(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-meta-media
    KRYS_NODISCARD DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-meta-media
    void Media(DOMString value) noexcept;

#pragma endregion

#pragma region HTMLMetaElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLMetaElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-meta-scheme
    KRYS_NODISCARD DOMString Scheme() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-meta-scheme
    void Scheme(DOMString value) noexcept;

#pragma endregion

  protected:
    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    /// @brief Updates the document's colour schemes if the element's name is (or, as @p oldName, was)
    /// `color-scheme`, and runs its pragma directive, if it has one, while it's connected.
    /// @see https://html.spec.whatwg.org/multipage/semantics.html#pragma-directives
    void Process(Maybe<DOMStringView> oldName = null) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLMetaElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLMetaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
