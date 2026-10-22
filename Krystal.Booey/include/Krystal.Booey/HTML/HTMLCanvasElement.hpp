#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlcanvaselement
  class HTMLCanvasElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLCanvasElement);

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool CanStartSelection() const noexcept final
    {
      return false;
    }

    HTMLCanvasElement(dom::Document &document) noexcept;

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override
    {
      // FIXME: A canvas is replaced when scripting is enabled for it; there is no scripting yet.
      return false;
    }

#pragma region HTMLCanvasElement - https://html.spec.whatwg.org/#htmlcanvaselement

    /// @see https://html.spec.whatwg.org/#dom-canvas-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-canvas-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-canvas-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-canvas-height
    void Height(uint32 value) noexcept;

    // TODO(HTMLCANVAS, HTML): Implement the context attribute and related APIs.
    // RenderingContext? getContext(dom::DOMString contextId, optional any options = null);
    //
    // USVString toDataURL(optional dom::DOMString type = "image/png", optional any quality);
    // undefined toBlob(BlobCallback _callback, optional dom::DOMString type = "image/png",
    //                   optional any quality);
    // OffscreenCanvas transferControlToOffscreen();

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLCanvasElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLCanvasElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();