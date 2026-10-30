#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo
{
  class RenderLineBreak;
}

namespace krys::boo::html
{
  /// @brief A `<wbr>`, a line break opportunity. Its interface is `HTMLElement`.
  /// @see https://html.spec.whatwg.org/#the-wbr-element
  class HTMLWBRElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLWBRElement);

  public:
    explicit HTMLWBRElement(dom::Document &document) noexcept;

    KRYS_NODISCARD RenderLineBreak *Renderer() const noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLWBRElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLWBRElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
