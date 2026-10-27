#pragma once

#include "Krystal.Booey/Rendering/LegacyRootInlineBox.hpp"

namespace krys::boo
{
  class RenderSVGText;
  class SVGInlineTextBox;

  /// @brief The legacy root inline box of an SVG text renderer's single line.
  class SVGRootInlineBox final : public LegacyRootInlineBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGRootInlineBox);

  private:
    float _logicalHeight {0};

  public:
    explicit SVGRootInlineBox(RenderSVGText &) noexcept;

    KRYS_NODISCARD float VirtualLogicalHeight() const noexcept override
    {
      return _logicalHeight;
    }
    void SetLogicalHeight(float height) noexcept
    {
      _logicalHeight = height;
    }

  private:
    KRYS_NODISCARD RenderSVGText &GetRenderSVGText() const noexcept;

    KRYS_NODISCARD bool IsSVGRootInlineBox() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGRootInlineBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyInlineBox &box) noexcept
  {
    return box.IsSVGRootInlineBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
