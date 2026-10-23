#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLPlugInElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlembedelement
  class HTMLEmbedElement : public HTMLPlugInElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLEmbedElement);

  public:
    HTMLEmbedElement(dom::Document &document) noexcept;

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override
    {
      // FIXME: A plug-in replacement that creates a renderer makes the element not replaced.
      return true;
    }

#pragma region HTMLEmbedElement - https://html.spec.whatwg.org/#htmlembedelement

    /// @see https://html.spec.whatwg.org/#dom-embed-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-src
    void Src(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-width
    void Width(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-height
    KRYS_NODISCARD dom::DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-height
    void Height(dom::DOMString value) noexcept;

    // TODO(HTMLEmbedElement, HTML): Document? getSVGDocument();

#pragma endregion

#pragma region HTMLEmbedElement Obsolete members - https://html.spec.whatwg.org/#HTMLEmbedElement-partial

    /// @see https://html.spec.whatwg.org/#dom-embed-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-name
    void Name(dom::DOMString &&value) noexcept;

#pragma endregion

  protected:
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLEmbedElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLEmbedElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();