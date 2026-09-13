#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocking-mechanism
  class RenderBlocking
  {
  public:
    /// @see https://html.spec.whatwg.org/multipage/dom.html#allows-adding-render-blocking-elements
    KRYS_NODISCARD static bool AllowsAddingRenderBlockingElements(const dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocked
    KRYS_NODISCARD static bool IsRenderBlocked(const dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocking
    KRYS_NODISCARD static bool IsRenderBlocking(const dom::Element &element, const dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#block-rendering
    KRYS_NODISCARD static void BlockRendering(dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#unblock-rendering
    KRYS_NODISCARD static void UnblockRendering(dom::Element &element) noexcept;
  };
}