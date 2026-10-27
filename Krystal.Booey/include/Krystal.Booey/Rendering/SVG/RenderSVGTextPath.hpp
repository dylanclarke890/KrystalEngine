#pragma once

#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"

namespace krys::boo::svg
{
  class SVGGeometryElement;
  class SVGLengthValue;
  class SVGTextPathElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<textPath>`: its text is laid out along the referenced shape's path.
  class RenderSVGTextPath final : public RenderSVGInline
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSVGTextPath);

  public:
    RenderSVGTextPath(svg::SVGTextPathElement &, RenderStyle &&) noexcept;
    ~RenderSVGTextPath() noexcept override;

    KRYS_NODISCARD svg::SVGTextPathElement &TextPathElement() const noexcept;
    KRYS_NODISCARD svg::SVGGeometryElement *TargetElement() const noexcept;

    KRYS_NODISCARD Path LayoutPath() const noexcept;
    KRYS_NODISCARD const svg::SVGLengthValue &StartOffset() const noexcept;

  private:
    void GraphicsElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGTextPath";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderSVGTextPath)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSVGTextPath();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
