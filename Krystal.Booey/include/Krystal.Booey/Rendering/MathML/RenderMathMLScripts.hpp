#pragma once

#include "Krystal.Booey/MathML/MathMLScriptsElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief The renderer of `<msub>`, `<msup>`, `<msubsup>` and `<mmultiscripts>`: a base with scripts, and
  /// the base of the under and over renderer, which lays out as one when its limits move.
  /// @see https://w3c.github.io/mathml-core/#script-and-limit-schemata
  class RenderMathMLScripts : public RenderMathMLRow
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLScripts);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLScripts);

  public:
    RenderMathMLScripts(Type type, mathml::MathMLScriptsElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLScripts() noexcept override;

    KRYS_NODISCARD RenderMathMLOperator *UnembellishedOperator() const noexcept final;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMathMLScripts";
    }

  protected:
    KRYS_NODISCARD mathml::MathMLScriptsElement::ScriptType GetScriptType() const noexcept;
    void ComputePreferredLogicalWidths() noexcept override;
    void LayoutBlock(RelayoutChildren relayoutChildren,
                     LayoutUnit pageLogicalHeight = 0_lu) noexcept override;

  private:
    struct ReferenceChildren
    {
      RenderBox *Base {nullptr};
      RenderBox *PrescriptDelimiter {nullptr};
      RenderBox *FirstPostScript {nullptr};
      RenderBox *FirstPreScript {nullptr};
    };

    struct VerticalParameters
    {
      LayoutUnit SubscriptShiftDown;
      LayoutUnit SuperscriptShiftUp;
      LayoutUnit SubscriptBaselineDropMin;
      LayoutUnit SuperScriptBaselineDropMax;
      LayoutUnit SubSuperscriptGapMin;
      LayoutUnit SuperscriptBottomMin;
      LayoutUnit SubscriptTopMax;
      LayoutUnit SuperscriptBottomMaxWithSubscript;
    };

    struct VerticalMetrics
    {
      LayoutUnit SubShift;
      LayoutUnit SupShift;
      LayoutUnit Ascent;
      LayoutUnit Descent;
    };

    KRYS_NODISCARD mathml::MathMLScriptsElement &GetElement() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept final;
    KRYS_NODISCARD Maybe<ReferenceChildren> ValidateAndGetReferenceChildren() const noexcept;
    KRYS_NODISCARD LayoutUnit SpaceAfterScript() const noexcept;
    KRYS_NODISCARD static LayoutUnit ItalicCorrection(const ReferenceChildren &reference) noexcept;
    KRYS_NODISCARD VerticalParameters GetVerticalParameters() const noexcept;
    KRYS_NODISCARD VerticalMetrics GetVerticalMetrics(const ReferenceChildren &reference) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLScripts)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLScripts();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
