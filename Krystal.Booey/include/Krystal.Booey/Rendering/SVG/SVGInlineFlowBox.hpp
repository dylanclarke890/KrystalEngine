#pragma once

#include "Krystal.Booey/Rendering/LegacyInlineFlowBox.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"

namespace krys::boo
{
  class RenderSVGInlineText;

  /// @brief An SVG inline renderer's legacy inline box.
  class SVGInlineFlowBox final : public LegacyInlineFlowBox
  {
  private:
    float _logicalHeight {0};

  public:
    explicit SVGInlineFlowBox(RenderSVGInline &renderer) noexcept : LegacyInlineFlowBox(renderer)
    {
    }

    KRYS_NODISCARD RenderSVGInline &Renderer() const noexcept
    {
      return static_cast<RenderSVGInline &>(LegacyInlineFlowBox::Renderer());
    }

    KRYS_NODISCARD FloatRect CalculateBoundaries() const noexcept;

    void SetLogicalHeight(float h) noexcept
    {
      _logicalHeight = h;
    }

  private:
    KRYS_NODISCARD bool IsSVGInlineFlowBox() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD float VirtualLogicalHeight() const noexcept override
    {
      return _logicalHeight;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGInlineFlowBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyInlineBox &box) noexcept
  {
    return box.IsSVGInlineFlowBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
