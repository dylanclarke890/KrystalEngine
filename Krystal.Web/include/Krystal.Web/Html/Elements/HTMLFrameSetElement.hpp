#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/obsolete.html#htmlframesetelement
  class HTMLFrameSetElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFrameSetElement);

  public:
    HTMLFrameSetElement(Document &document) noexcept;

#pragma region WindowEventHandlers - https://html.spec.whatwg.org/multipage/webappapis.html#windoweventhandlers

    /// @note The body and frameset elements' window event handlers are their document's window's.
    KRYS_WINDOW_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-frameset-cols
    KRYS_NODISCARD DOMString Cols() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-frameset-cols
    void Cols(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-frameset-rows
    KRYS_NODISCARD DOMString Rows() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-frameset-rows
    void Rows(DOMString value) noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLFrameSetElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLFrameSetElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
