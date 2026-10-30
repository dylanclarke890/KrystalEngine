#pragma once

#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLFractionElement;
  }

  /// @brief The renderer of `<mfrac>`: its numerator centred over its denominator, with a fraction bar
  /// between them on the math axis.
  /// @see https://w3c.github.io/mathml-core/#fractions-mfrac
  class RenderMathMLFraction final : public RenderMathMLRow
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLFraction);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLFraction);

  public:
    RenderMathMLFraction(mathml::MathMLFractionElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLFraction() noexcept override;

    KRYS_NODISCARD LayoutUnit DefaultLineThickness() const noexcept;
    KRYS_NODISCARD LayoutUnit LineThickness() const noexcept;
    KRYS_NODISCARD float RelativeLineThickness() const noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLFraction";
    }

  private:
    struct FractionParameters
    {
      LayoutUnit NumeratorShiftUp;
      LayoutUnit DenominatorShiftDown;
    };

    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept final;
    void Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept final;
    KRYS_NODISCARD RenderMathMLOperator *UnembellishedOperator() const noexcept final;
    KRYS_NODISCARD bool IsMathContentCentered() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD mathml::MathMLFractionElement &GetElement() const noexcept;

    KRYS_NODISCARD bool IsValid() const noexcept;
    KRYS_NODISCARD RenderBox &Numerator() const noexcept;
    KRYS_NODISCARD RenderBox &Denominator() const noexcept;
    KRYS_NODISCARD LayoutUnit HorizontalOffset(const RenderBox &child) const noexcept;
    KRYS_NODISCARD FractionParameters GetFractionParameters() const noexcept;
    KRYS_NODISCARD FractionParameters GetStackParameters() const noexcept;
    KRYS_NODISCARD LayoutUnit FractionAscent() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLFraction)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLFraction();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
