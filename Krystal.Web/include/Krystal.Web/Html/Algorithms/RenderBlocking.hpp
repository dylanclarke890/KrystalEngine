#pragma once

#include "Krystal.Core/Attributes.hpp"
#include <chrono>

namespace krys::web
{
  class Document;
  class Element;
}

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocking-mechanism
  class RenderBlocking
  {
  public:
    /// @brief The implementation-defined timeout: how long after its window's time origin a document can be
    /// render-blocked, so that a style sheet that never loads doesn't keep the page from showing.
    constexpr static auto Timeout = std::chrono::seconds(5);

    /// @see https://html.spec.whatwg.org/multipage/dom.html#allows-adding-render-blocking-elements
    KRYS_NODISCARD static bool AllowsAddingRenderBlockingElements(const Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocked
    KRYS_NODISCARD static bool IsRenderBlocked(const Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocking
    KRYS_NODISCARD static bool IsRenderBlocking(const Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#block-rendering
    static void BlockRendering(Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#unblock-rendering
    static void UnblockRendering(Element &element) noexcept;

    /// @brief Whether @p element's blocking tokens set contains "render", the only possible blocking token:
    /// an element whose blocking tokens set contains it is potentially render-blocking.
    /// @see https://html.spec.whatwg.org/multipage/urls-and-fetching.html#blocking-tokens-set
    /// @see https://html.spec.whatwg.org/multipage/urls-and-fetching.html#potentially-render-blocking
    KRYS_NODISCARD static bool HasRenderBlockingToken(const Element &element) noexcept;
  };
}
