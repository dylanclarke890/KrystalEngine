#pragma once

#include "Krystal.Web/Dom/Types/USVString.hpp"
#include "Krystal.Web/Html/Elements/HTMLPlugInElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#htmlembedelement
  class HTMLEmbedElement : public HTMLPlugInElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLEmbedElement);

  public:
    HTMLEmbedElement(Document &document) noexcept;

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override
    {
      return true;
    }

#pragma region HTMLEmbedElement - https://html.spec.whatwg.org/multipage/iframe-embed-object.html#htmlembedelement

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-embed-src
    KRYS_NODISCARD USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-embed-src
    void Src(USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-embed-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-embed-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-embed-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-embed-width
    void Width(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-embed-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-embed-height
    void Height(DOMString value) noexcept;

    // (No `getSVGDocument()`: an embed loads no nested document, as subframes aren't planned; see
    // OVERVIEW.md.)

#pragma endregion

#pragma region HTMLEmbedElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLEmbedElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-embed-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-embed-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-embed-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-embed-name
    void Name(DOMString value) noexcept;

#pragma endregion

  protected:
    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLEmbedElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLEmbedElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
