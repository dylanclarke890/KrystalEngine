#pragma once

#include "Krystal.Booey/Rendering/MathML/RenderMathMLScripts.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLUnderOverElement;
  }

  /// @brief The renderer of `<munder>`, `<mover>` and `<munderover>`: a base with scripts centred below and
  /// above it, which move to its side as sub- and superscripts when its operator's limits move.
  /// @see https://w3c.github.io/mathml-core/#underscripts-and-overscripts-munder-mover-munderover
  class RenderMathMLUnderOver final : public RenderMathMLScripts
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLUnderOver);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLUnderOver);

  public:
    RenderMathMLUnderOver(mathml::MathMLUnderOverElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLUnderOver() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLUnderOver";
    }

  private:
    struct VerticalParameters
    {
      bool UseUnderOverBarFallBack {false};
      LayoutUnit UnderGapMin;
      LayoutUnit OverGapMin;
      LayoutUnit UnderShiftMin;
      LayoutUnit OverShiftMin;
      LayoutUnit UnderExtraDescender;
      LayoutUnit OverExtraAscender;
      LayoutUnit AccentBaseHeight;
    };

    KRYS_NODISCARD bool IsMathContentCentered() const noexcept final
    {
      return !ShouldMoveLimits();
    }
    KRYS_NODISCARD mathml::MathMLUnderOverElement &GetElement() const noexcept;

    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;

    void StretchHorizontalOperatorsAndLayoutChildren() noexcept;
    KRYS_NODISCARD bool IsValid() const noexcept;
    KRYS_NODISCARD bool ShouldMoveLimits() const noexcept;
    KRYS_NODISCARD RenderBox &Base() const noexcept;
    KRYS_NODISCARD RenderBox &Under() const noexcept;
    KRYS_NODISCARD RenderBox &Over() const noexcept;
    KRYS_NODISCARD LayoutUnit HorizontalOffset(const RenderBox &child) const noexcept;
    KRYS_NODISCARD bool HasAccent(bool accentUnder = false) const noexcept;
    KRYS_NODISCARD bool HasAccentUnder() const noexcept
    {
      return HasAccent(true);
    }
    KRYS_NODISCARD VerticalParameters GetVerticalParameters() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLUnderOver)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLUnderOver();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
