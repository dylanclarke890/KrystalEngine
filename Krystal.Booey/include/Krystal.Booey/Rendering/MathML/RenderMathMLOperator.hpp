#pragma once

#include "Krystal.Booey/MathML/MathMLOperatorDictionary.hpp"
#include "Krystal.Booey/Rendering/MathML/MathOperator.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLToken.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLOperatorElement;
  }

  /// @brief The renderer of `<mo>`: a token spaced and stretched as its operator properties say, drawn from
  /// the font's larger glyphs when it stretches or is a large operator.
  class RenderMathMLOperator final : public RenderMathMLToken
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLOperator);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLOperator);

  private:
    LayoutUnit _stretchHeightAboveBaseline {0};
    LayoutUnit _stretchDepthBelowBaseline {0};
    LayoutUnit _stretchWidth;
    bool _isStretchWidthLocked {false};

    MathOperator _mathOperator;

  public:
    RenderMathMLOperator(mathml::MathMLOperatorElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLOperator() noexcept override;

    KRYS_NODISCARD mathml::MathMLOperatorElement &GetElement() const noexcept;

    void StretchTo(LayoutUnit heightAboveBaseline, LayoutUnit depthBelowBaseline) noexcept;
    void StretchTo(LayoutUnit width) noexcept;
    KRYS_NODISCARD LayoutUnit StretchSize() const noexcept
    {
      return IsVertical() ? _stretchHeightAboveBaseline + _stretchDepthBelowBaseline : _stretchWidth;
    }
    void ResetStretchSize() noexcept;
    void SetStretchWidthLocked(bool stretchWidthLocked) noexcept
    {
      _isStretchWidthLocked = stretchWidthLocked;
    }
    KRYS_NODISCARD bool IsStretchWidthLocked() const noexcept
    {
      return _isStretchWidthLocked;
    }

    KRYS_NODISCARD bool HasOperatorFlag(mathml::MathMLOperatorDictionary::Flag flag) const noexcept;
    KRYS_NODISCARD bool IsLargeOperatorInDisplayStyle() const noexcept;
    KRYS_NODISCARD bool ShouldMoveLimits() const noexcept
    {
      return HasOperatorFlag(mathml::MathMLOperatorDictionary::MovableLimits);
    }
    KRYS_NODISCARD bool IsVertical() const noexcept;
    KRYS_NODISCARD LayoutUnit ItalicCorrection() const noexcept
    {
      return _mathOperator.ItalicCorrection();
    }

    void UpdateTokenContent() noexcept final;
    void UpdateFromElement() noexcept final;
    KRYS_NODISCARD char32 TextContent() const noexcept;
    KRYS_NODISCARD bool IsStretchy() const noexcept
    {
      return TextContent() && HasOperatorFlag(mathml::MathMLOperatorDictionary::Stretchy);
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLOperator";
    }

  private:
    void UpdateMathOperator() noexcept;
    KRYS_NODISCARD LayoutUnit LeadingSpace() const noexcept;
    KRYS_NODISCARD LayoutUnit TrailingSpace() const noexcept;
    KRYS_NODISCARD LayoutUnit MinSize() const noexcept;
    KRYS_NODISCARD LayoutUnit MaxSize() const noexcept;
    KRYS_NODISCARD bool UseMathOperator() const noexcept;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept final;
    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
    void Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept final;

    void PaintChildren(PaintInfo &forSelf, const LayoutPoint &paintOffset,
                       PaintInfo &forChild) noexcept final;
    KRYS_NODISCARD bool IsInvisibleOperator() const noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept final;
    KRYS_NODISCARD RenderMathMLOperator *UnembellishedOperator() const noexcept final
    {
      return const_cast<RenderMathMLOperator *>(this);
    }

    KRYS_NODISCARD LayoutUnit VerticalStretchedOperatorShift() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLOperator)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLOperator();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
