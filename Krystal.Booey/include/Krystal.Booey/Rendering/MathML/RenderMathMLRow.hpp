#pragma once

#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLRowElement;
  }

  /// @brief Lays its children out in a row, on a common baseline, stretching its vertical operators to the
  /// row's height: the renderer of `<mrow>` and the base of the renderers with more specific layouts.
  class RenderMathMLRow : public RenderMathMLBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLRow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLRow);

  public:
    RenderMathMLRow(Type type, mathml::MathMLRowElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLRow() noexcept override;

    KRYS_NODISCARD mathml::MathMLRowElement &GetElement() const noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMathMLRow";
    }

  protected:
    void LayoutBlock(RelayoutChildren relayoutChildren,
                     LayoutUnit pageLogicalHeight = 0_lu) noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override;

    void StretchVerticalOperatorsAndLayoutChildren() noexcept;
    void GetContentBoundingBox(LayoutUnit &width, LayoutUnit &ascent, LayoutUnit &descent) const noexcept;
    void LayoutRowItems(LayoutUnit width, LayoutUnit ascent) noexcept;
    KRYS_NODISCARD LayoutUnit PreferredLogicalWidthOfRowItems() noexcept;
    void ComputePreferredLogicalWidths() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLRow)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLRow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
