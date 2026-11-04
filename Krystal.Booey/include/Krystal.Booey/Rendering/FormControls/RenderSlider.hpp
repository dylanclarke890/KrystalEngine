#pragma once

#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"

namespace krys::boo::html
{
  class HTMLInputElement;
}

namespace krys::boo
{
  /// @brief The renderer of `<input type=range>`: a flex box holding its track and thumb.
  class RenderSlider final : public RenderFlexibleBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSlider);

  public:
    static constexpr int32 DefaultTrackLength = 129;

    RenderSlider(html::HTMLInputElement &, RenderStyle &&) noexcept;
    ~RenderSlider() noexcept override;

    KRYS_NODISCARD html::HTMLInputElement &InputElement() const noexcept;

    KRYS_NODISCARD bool CanHaveGeneratedChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool InDragMode() const noexcept;
    KRYS_NODISCARD double ValueRatio() const noexcept;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSlider";
    }

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;
    void ComputePreferredLogicalWidths() noexcept override;

    KRYS_NODISCARD bool IsFlexibleBoxImpl() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderSlider)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSlider();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
