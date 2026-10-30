#pragma once

#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLPresentationElement;
  }

  class RenderMathMLOperator;

  /// @brief The base of the MathML renderers: a block whose children are all block-level, laid out by the
  /// layout of its element.
  class RenderMathMLBlock : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLBlock);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLBlock);

  public:
    RenderMathMLBlock(Type type, mathml::MathMLPresentationElement &container, RenderStyle &&style) noexcept;
    RenderMathMLBlock(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderMathMLBlock() noexcept override;

    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &child,
                                       const RenderStyle &) const noexcept override;

    // MathML defines an "embellished operator" as roughly an <mo> that may have subscripts,
    // superscripts, underscripts, overscripts, or a denominator (as in d/dx, where "d" is some
    // differential operator). The padding, precedence, and stretchiness of the base <mo> should
    // apply to the embellished operator as a whole. unembellishedOperator() checks for being an
    // embellished operator, and omits any embellishments.
    // FIXME: We don't yet handle all the cases in the MathML spec. See
    // https://bugs.webkit.org/show_bug.cgi?id=78617.
    KRYS_NODISCARD virtual RenderMathMLOperator *UnembellishedOperator() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMathMLBlock";
    }

  protected:
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    /// @brief The default rule thickness, for fonts without a MATH table.
    /// @see https://w3c.github.io/mathml-core/#dfn-default-rule-thickness
    KRYS_NODISCARD LayoutUnit RuleThicknessFallback() const noexcept;
    KRYS_NODISCARD LayoutUnit MathAxisHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit MirrorIfNeeded(LayoutUnit horizontalOffset,
                                             LayoutUnit boxWidth = 0_lu) const noexcept;
    KRYS_NODISCARD LayoutUnit MirrorIfNeeded(LayoutUnit horizontalOffset,
                                             const RenderBox &child) const noexcept;
    KRYS_NODISCARD static LayoutUnit AscentForChild(const RenderBox &child) noexcept;

    void LayoutBlock(RelayoutChildren relayoutChildren,
                     LayoutUnit pageLogicalHeight = 0_lu) noexcept override;

    void ComputeAndSetBlockDirectionMarginsOfChildren() noexcept;
    void InsertPositionedChildrenIntoContainingBlock() noexcept;
    void LayoutFloatingChildren() noexcept;
    void ShiftInFlowChildren(LayoutUnit left, LayoutUnit top) noexcept;

    void AdjustPreferredLogicalWidthsForBorderAndPadding() noexcept;
    void AdjustLayoutForBorderAndPadding() noexcept;

    enum class LayoutPhase : uint8
    {
      CalculatePreferredLogicalWidth,
      Layout,
    };

    struct SizeAppliedToMathContent
    {
      Maybe<LayoutUnit> LogicalWidth;
      Maybe<LayoutUnit> LogicalHeight;
    };

    // Retrieve the specified (and supported) CSS width/height to apply to math
    // content box, if any.
    KRYS_NODISCARD SizeAppliedToMathContent SizeAppliedToMathContentFor(LayoutPhase phase) const noexcept;

    // Whether math content should be centered on the inline axis if a different size is specified by the
    // user.
    KRYS_NODISCARD virtual bool IsMathContentCentered() const noexcept
    {
      return false;
    }

    // Apply the specified CSS width/height to the math content box and return inline shift for further
    // adjustments.
    LayoutUnit ApplySizeToMathContent(LayoutPhase phase, const SizeAppliedToMathContent &sizes) noexcept;

  private:
    KRYS_NODISCARD bool CanDropAnonymousBlockChild() const noexcept final
    {
      return false;
    }

    void LayoutItems(RelayoutChildren relayoutChildren) noexcept;
  };

  /// @brief The renderer of `<mtable>`: a table centred on the math axis.
  class RenderMathMLTable final : public RenderTable
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLTable);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLTable);

  public:
    RenderMathMLTable(mathml::MathMLElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLTable() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLTable";
    }

  private:
    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept final;
  };

  /// @brief Resolves the MathML length @p length against @p style.
  /// @param referenceValue What percentages and unitless values are relative to, and the result if @p length
  /// failed to parse.
  KRYS_NODISCARD LayoutUnit ToUserUnits(const mathml::MathMLElement::Length &length, const RenderStyle &style,
                                        LayoutUnit referenceValue) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLBlock)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLBlock();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLTable)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLTable();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
