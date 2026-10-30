#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/MathML/MathMLMencloseElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief The renderer of `<menclose>`: a row with space around it for the notations it draws.
  class RenderMathMLMenclose final : public RenderMathMLRow
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLMenclose);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLMenclose);

  private:
    struct SpaceAroundContent
    {
      LayoutUnit Left;
      LayoutUnit Right;
      LayoutUnit Top;
      LayoutUnit Bottom;
    };

    LayoutRect _contentRect;

  public:
    RenderMathMLMenclose(mathml::MathMLMencloseElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLMenclose() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLMenclose";
    }

  private:
    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
    void Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept final;

    KRYS_NODISCARD LayoutUnit RuleThickness() const noexcept;
    KRYS_NODISCARD bool
      HasNotation(mathml::MathMLMencloseElement::MencloseNotationFlag notationFlag) const noexcept
    {
      return static_cast<mathml::MathMLMencloseElement &>(NodeForNonAnonymous()).HasNotation(notationFlag);
    }

    KRYS_NODISCARD SpaceAroundContent GetSpaceAroundContent(LayoutUnit contentWidth,
                                                            LayoutUnit contentHeight) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLMenclose)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLMenclose();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
