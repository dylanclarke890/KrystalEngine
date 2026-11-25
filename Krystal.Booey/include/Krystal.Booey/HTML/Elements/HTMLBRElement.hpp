#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#htmlbrelement
  class HTMLBRElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBRElement);

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }

    HTMLBRElement(Document &document) noexcept;

#pragma region HTMLBRElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLBRElement-partial

    /// @see https://html.spec.whatwg.org/#dom-br-compact
    KRYS_NODISCARD DOMString Clear() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-br-compact
    void Clear(DOMString value) noexcept;

#pragma endregion

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLBRElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLBRElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
