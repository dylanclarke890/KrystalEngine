#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
{
  class Document;
  class Element;

  /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocking-mechanism
  class RenderBlocking
  {
  public:
    /// @see https://html.spec.whatwg.org/multipage/dom.html#allows-adding-render-blocking-elements
    KRYS_NODISCARD static bool AllowsAddingRenderBlockingElements(const Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocked
    KRYS_NODISCARD static bool IsRenderBlocked(const Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocking
    KRYS_NODISCARD static bool IsRenderBlocking(const Element &element, const Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#block-rendering
    KRYS_NODISCARD static void BlockRendering(Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#unblock-rendering
    KRYS_NODISCARD static void UnblockRendering(Element &element) noexcept;
  };
}