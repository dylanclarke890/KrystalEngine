#pragma once

#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLPaddedElement;
  }

  /// @brief The renderer of `<mpadded>`: a row whose size and position are set by the element's attributes.
  class RenderMathMLPadded final : public RenderMathMLRow
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLPadded);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLPadded);

  public:
    RenderMathMLPadded(mathml::MathMLPaddedElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLPadded() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLPadded";
    }

  private:
    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept final;

    KRYS_NODISCARD mathml::MathMLPaddedElement &GetElement() const noexcept;
    KRYS_NODISCARD LayoutUnit Voffset() const noexcept;
    KRYS_NODISCARD LayoutUnit Lspace() const noexcept;
    KRYS_NODISCARD LayoutUnit MpaddedWidth(LayoutUnit contentWidth) const noexcept;
    KRYS_NODISCARD LayoutUnit MpaddedHeight(LayoutUnit contentHeight) const noexcept;
    KRYS_NODISCARD LayoutUnit MpaddedDepth(LayoutUnit contentDepth) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLPadded)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLPadded();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
