#pragma once

#include "Krystal.Core/Attributes.hpp"
#include <chrono>

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
    /// @brief The implementation-defined timeout: how long after its window's time origin a document can be
    /// render-blocked, so that a style sheet that never loads doesn't keep the page from showing.
    static constexpr auto Timeout = std::chrono::seconds(5);

    /// @see https://html.spec.whatwg.org/multipage/dom.html#allows-adding-render-blocking-elements
    KRYS_NODISCARD static bool AllowsAddingRenderBlockingElements(const dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocked
    KRYS_NODISCARD static bool IsRenderBlocked(const dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#render-blocking
    KRYS_NODISCARD static bool IsRenderBlocking(const dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#block-rendering
    static void BlockRendering(dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#unblock-rendering
    static void UnblockRendering(dom::Element &element) noexcept;

    /// @brief Whether @p element's blocking tokens set contains "render", the only possible blocking token:
    /// an element whose blocking tokens set contains it is potentially render-blocking.
    /// @see https://html.spec.whatwg.org/#blocking-tokens-set
    /// @see https://html.spec.whatwg.org/#potentially-render-blocking
    KRYS_NODISCARD static bool HasRenderBlockingToken(const dom::Element &element) noexcept;
  };
}