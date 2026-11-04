#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLMeterElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<meter>` the theme draws itself.
  class RenderMeter final : public RenderBlockFlow
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMeter);

  public:
    RenderMeter(html::HTMLElement &, RenderStyle &&) noexcept;
    ~RenderMeter() noexcept override;

    KRYS_NODISCARD html::HTMLMeterElement *MeterElement() const noexcept;

    void UpdateFromElement() noexcept override;

  private:
    void UpdateLogicalWidth() noexcept override;
    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMeter";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMeter)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMeter();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
