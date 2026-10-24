#pragma once

#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlframesetelement
  class HTMLFrameSetElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFrameSetElement);

  public:
    HTMLFrameSetElement(dom::Document &document) noexcept;

#pragma region WindowEventHandlers - https://html.spec.whatwg.org/#windoweventhandlers

    /// @note The body and frameset elements' window event handlers are their document's window's.
    KRYS_WINDOW_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

    /// @see https://html.spec.whatwg.org/#dom-frameset-cols
    KRYS_NODISCARD dom::DOMString Cols() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-frameset-cols
    void Cols(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-frameset-rows
    KRYS_NODISCARD dom::DOMString Rows() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-frameset-rows
    void Rows(dom::DOMString &&value) noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLFrameSetElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLFrameSetElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
