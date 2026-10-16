#pragma once

#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief An inline box.
  // FIXME: Line boxes, continuations, hit testing, the geometry and painting come with layout and painting.
  class RenderInline : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderInline);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderInline);

  public:
    RenderInline(Type type, dom::Element &element, RenderStyle &&style) noexcept;
    RenderInline(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderInline() noexcept override;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    // FIXME: Layout queries, reachable only through `RendererOf` and `RenderBoxOf`, which return null until
    // there's layout.
    KRYS_NODISCARD LayoutUnit InnerPaddingBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit InnerPaddingBoxHeight() const noexcept;

  protected:
    void UpdateFromStyle() noexcept override;
  };

  /// @brief Whether @p renderer holds nothing but collapsible whitespace (and floating or out-of-flow boxes).
  KRYS_NODISCARD bool IsEmptyInline(const RenderInline &renderer) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderInline)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderInline();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
