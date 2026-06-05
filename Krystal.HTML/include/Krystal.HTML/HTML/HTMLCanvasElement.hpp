#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlcanvaselement
  class HTMLCanvasElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLCanvasElement);

  public:
    HTMLCanvasElement(Document &document) noexcept;

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
    // RenderingContext? getContext(DOMString contextId, optional any options = null);
    //
    // USVString toDataURL(optional DOMString type = "image/png", optional any quality);
    // undefined toBlob(BlobCallback _callback, optional DOMString type = "image/png", optional any quality);
    // OffscreenCanvas transferControlToOffscreen();

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLCanvasElement)
  static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLCanvasElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();