#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Booey/MathML/MathMLPresentationElement.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo
{
  RenderMathMLBlock::RenderMathMLBlock(Type type, mathml::MathMLPresentationElement &container,
                                       RenderStyle &&style) noexcept
      : RenderBlock(type, container, krys::move(style))
  {
    SetChildrenInline(false); // All of our children must be block-level.
  }

  RenderMathMLBlock::RenderMathMLBlock(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlock(type, document, krys::move(style))
  {
    SetChildrenInline(false); // All of our children must be block-level.
  }

  RenderMathMLBlock::~RenderMathMLBlock() noexcept = default;

  bool RenderMathMLBlock::IsChildAllowed(const RenderObject &child, const RenderStyle &) const noexcept
  {
    return Is<dom::Element>(child.GetNode());
  }

  namespace
  {
    KRYS_NODISCARD LayoutUnit MathMLAxisHeight(const RenderStyle &style) noexcept
    {
      // If we have a MATH table we just return the AxisHeight constant.
      auto primaryFont = style.GetFontCascade().PrimaryFont();
      if (const auto *mathData = primaryFont->MathData())
      {
        return LayoutUnit(
          mathData->GetMathConstant(*primaryFont, OpenTypeMathData::MathConstant::AxisHeight));
      }

      // Otherwise, the idea is to try and use the middle of operators as the math axis which we thus
      // approximate by "half of the x-height". Note that Gecko has a slower but more accurate version that
      // measures half of the height of U+2212 MINUS SIGN.
      return LayoutUnit(style.MetricsOfPrimaryFont().XHeight().value_or(0) / 2);
    }
  }

  LayoutUnit RenderMathMLBlock::RuleThicknessFallback() const noexcept
  {
    return LayoutUnit(Style().MetricsOfPrimaryFont().UnderlineThickness().value_or(0.0f));
  }

  LayoutUnit RenderMathMLBlock::MathAxisHeight() const noexcept
  {
    return MathMLAxisHeight(Style());
  }

  LayoutUnit RenderMathMLBlock::MirrorIfNeeded(LayoutUnit horizontalOffset,
                                               LayoutUnit boxWidth) const noexcept
  {
    if (GetWritingMode().IsBidiRTL())
    {
      return LogicalWidth() - boxWidth - horizontalOffset;
    }

    return horizontalOffset;
  }

  LayoutUnit RenderMathMLBlock::MirrorIfNeeded(LayoutUnit horizontalOffset,
                                               const RenderBox &child) const noexcept
  {
    return MirrorIfNeeded(horizontalOffset, child.LogicalWidth());
  }

  LayoutUnit RenderMathMLBlock::AscentForChild(const RenderBox &child) noexcept
  {
    // Subpixel inline layout is off, so the height is snapped to an integer.
    auto logicalHeight = LayoutUnit(child.LogicalHeight().ToInt());
    return child.FirstLineBaseline().value_or(logicalHeight);
  }

  LayoutUnit ToUserUnits(const mathml::MathMLElement::Length &length, const RenderStyle &style,
                         LayoutUnit referenceValue) noexcept
  {
    using LengthType = mathml::MathMLElement::LengthType;

    switch (length.Type)
    {
      // Zoom for physical units needs to be accounted for.
      case LengthType::Cm:
        return LayoutUnit(style.UsedZoom() * length.Value
                          * static_cast<float>(css::pxPerInch / css::cmPerInch));
      case LengthType::In:
        return LayoutUnit(style.UsedZoom() * length.Value * static_cast<float>(css::pxPerInch));
      case LengthType::Mm:
        return LayoutUnit(style.UsedZoom() * length.Value
                          * static_cast<float>(css::pxPerInch / css::mmPerInch));
      case LengthType::Pc:
        return LayoutUnit(style.UsedZoom() * length.Value
                          * static_cast<float>(css::pxPerInch / css::picasPerInch));
      case LengthType::Pt:
        return LayoutUnit(style.UsedZoom() * length.Value
                          * static_cast<float>(css::pxPerInch / css::ptsPerInch));
      case LengthType::Px: return LayoutUnit(style.UsedZoom() * length.Value);

      // Zoom for logical units is accounted for either in the font info or referenceValue.
      case LengthType::Em: return LayoutUnit(length.Value * style.GetFontCascade().Size());
      case LengthType::Ex:
        return LayoutUnit(length.Value * style.MetricsOfPrimaryFont().XHeight().value_or(0)
                          * style.UsedZoom());
      case LengthType::MathUnit:      return LayoutUnit(length.Value * style.GetFontCascade().Size() / 18);
      case LengthType::Percentage:    return LayoutUnit(referenceValue * length.Value / 100);
      case LengthType::UnitLess:      return LayoutUnit(referenceValue * length.Value);
      case LengthType::ParsingFailed: return referenceValue;
    }

    krys_unreachable();
  }

  RenderMathMLTable::RenderMathMLTable(mathml::MathMLElement &element, RenderStyle &&style) noexcept
      : RenderTable(Type::MathMLTable, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMathMLTable());
  }

  RenderMathMLTable::~RenderMathMLTable() noexcept = default;

  Maybe<LayoutUnit> RenderMathMLTable::FirstLineBaseline() const noexcept
  {
    // By default the vertical center of <mtable> is aligned on the math axis.
    // This is different than RenderTable::firstLineBoxBaseline, which returns the baseline of the first row
    // of a <table>.
    auto baseline = LogicalHeight() / 2 + MathMLAxisHeight(Style());
    // Subpixel inline layout is off, so the baseline is snapped to an integer.
    return LayoutUnit(baseline.ToInt());
  }

  void RenderMathMLBlock::LayoutItems(RelayoutChildren relayoutChildren) noexcept
  {
    LayoutUnit verticalOffset = BorderAndPaddingBefore();
    LayoutUnit horizontalOffset = BorderAndPaddingStart();

    LayoutUnit preferredHorizontalExtent;
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      LayoutUnit childHorizontalExtent =
        child->MaxPreferredLogicalWidth() - child->HorizontalBorderAndPaddingExtent();
      LayoutUnit childHorizontalMarginBoxExtent =
        child->HorizontalBorderAndPaddingExtent() + childHorizontalExtent;
      childHorizontalMarginBoxExtent += child->HorizontalMarginExtent();

      preferredHorizontalExtent += childHorizontalMarginBoxExtent;
    }

    LayoutUnit currentHorizontalExtent = ContentBoxLogicalWidth();
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      auto everHadLayout = child->EverHadLayout();
      LayoutUnit childSize = child->MaxPreferredLogicalWidth() - child->HorizontalBorderAndPaddingExtent();

      if (preferredHorizontalExtent > currentHorizontalExtent)
      {
        childSize = currentHorizontalExtent;
      }

      LayoutUnit childPreferredSize = childSize + child->HorizontalBorderAndPaddingExtent();

      if (childPreferredSize != child->Width())
      {
        child->SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }

      UpdateBlockChildDirtyBitsBeforeLayout(relayoutChildren, *child);
      child->LayoutIfNeeded();

      LayoutUnit childVerticalMarginBoxExtent;
      childVerticalMarginBoxExtent = child->Height() + child->VerticalMarginExtent();

      SetLogicalHeight(std::max(LogicalHeight(), verticalOffset + BorderAndPaddingAfter()
                                                   + childVerticalMarginBoxExtent
                                                   + LayoutUnit(HorizontalScrollbarHeight())));

      horizontalOffset += child->MarginStart();

      LayoutUnit childHorizontalExtent = child->Width();
      LayoutPoint childLocation(
        GetWritingMode().IsBidiLTR() ? horizontalOffset : Width() - horizontalOffset - childHorizontalExtent,
        verticalOffset + child->MarginBefore());

      child->SetLocation(childLocation);
      horizontalOffset += childHorizontalExtent + child->MarginEnd();
      if (!everHadLayout && child->CheckForRepaintDuringLayout())
      {
        child->Repaint();
      }
    }
  }

  void RenderMathMLBlock::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    LayoutFloatingChildren();

    LayoutRepainter repainter(*this);

    if (RecomputeLogicalWidth())
    {
      relayoutChildren = RelayoutChildren::Yes;
    }

    SetLogicalHeight(BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight()));

    LayoutItems(relayoutChildren);

    UpdateLogicalHeight();

    LayoutOutOfFlowBoxes(relayoutChildren);

    repainter.RepaintAfterLayout();
  }

  void RenderMathMLBlock::ComputeAndSetBlockDirectionMarginsOfChildren() noexcept
  {
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      child->ComputeAndSetBlockDirectionMargins(*this);
    }
  }

  void RenderMathMLBlock::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlock::StyleDidChange(diff, oldStyle);

    // MathML displaystyle changes can affect layout.
    if (oldStyle && Style().GetMathStyle() != oldStyle->GetMathStyle())
    {
      SetNeedsLayoutAndPreferredWidthsUpdate();
    }
  }

  void RenderMathMLBlock::InsertPositionedChildrenIntoContainingBlock() noexcept
  {
    for (auto &child : ChildrenOfType<RenderBox>(*this))
    {
      if (child.IsOutOfFlowPositioned())
      {
        child.ContainingBlock()->AddOutOfFlowBox(child);
      }
    }
  }

  void RenderMathMLBlock::LayoutFloatingChildren() noexcept
  {
    // According to the spec, https://w3c.github.io/mathml-core/#css-styling:
    // > The float property does not create floating of elements whose parent's computed display value is
    // block math or inline math, > and does not take them out-of-flow.
    // FIXME: `display: math` is unimplemented (webkit.org/b/278533), so floats are neither positioned nor
    // in-flow; perform dummy layout for floating children. Per the spec, there should be no floating children
    // inside MathML renderers.
    for (auto &child : ChildrenOfType<RenderBox>(*this))
    {
      if (child.IsFloating())
      {
        child.LayoutIfNeeded();
      }
    }
  }

  void RenderMathMLBlock::ShiftInFlowChildren(LayoutUnit left, LayoutUnit top) noexcept
  {
    LayoutPoint shift(left, top);
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      child->SetLocation(child->Location() + shift);
    }
  }

  void RenderMathMLBlock::AdjustPreferredLogicalWidthsForBorderAndPadding() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());
    _minPreferredLogicalWidth += BorderAndPaddingLogicalWidth();
    _maxPreferredLogicalWidth += BorderAndPaddingLogicalWidth();
  }

  void RenderMathMLBlock::AdjustLayoutForBorderAndPadding() noexcept
  {
    SetLogicalWidth(LogicalWidth() + BorderAndPaddingLogicalWidth());
    SetLogicalHeight(LogicalHeight() + BorderAndPaddingLogicalHeight());
    ShiftInFlowChildren(Style().IsLeftToRightDirection() ? BorderAndPaddingStart() : BorderAndPaddingEnd(),
                        BorderAndPaddingBefore());
  }

  RenderMathMLBlock::SizeAppliedToMathContent
    RenderMathMLBlock::SizeAppliedToMathContentFor(LayoutPhase phase) const noexcept
  {
    SizeAppliedToMathContent sizes;
    const auto &style = Style();
    auto usedZoom = style.UsedZoomForLength();

    // Handle size containment with contain-intrinsic-inline-size
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto intrinsicWidth = ExplicitIntrinsicInnerLogicalWidth())
      {
        sizes.LogicalWidth = *intrinsicWidth;
      }

      if (phase == LayoutPhase::Layout && ShouldApplySizeContainment())
      {
        if (auto intrinsicHeight = ExplicitIntrinsicInnerLogicalHeight())
        {
          sizes.LogicalHeight = *intrinsicHeight;
        }
      }

      return sizes;
    }

    // FIXME: Resolve percentages.
    // https://github.com/w3c/mathml-core/issues/76
    if (auto fixedLogicalWidth = style.LogicalWidth().TryFixed())
    {
      sizes.LogicalWidth = LayoutUnit(fixedLogicalWidth->ResolveZoom(usedZoom));
    }

    // FIXME: Resolve percentages.
    // https://github.com/w3c/mathml-core/issues/77
    if (auto fixedLogicalHeight = style.LogicalHeight().TryFixed();
        phase == LayoutPhase::Layout && fixedLogicalHeight)
    {
      sizes.LogicalHeight = LayoutUnit(fixedLogicalHeight->ResolveZoom(usedZoom));
    }

    return sizes;
  }

  LayoutUnit RenderMathMLBlock::ApplySizeToMathContent(LayoutPhase phase,
                                                       const SizeAppliedToMathContent &sizes) noexcept
  {
    if (phase == LayoutPhase::CalculatePreferredLogicalWidth)
    {
      krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());
      if (sizes.LogicalWidth)
      {
        _minPreferredLogicalWidth = *sizes.LogicalWidth;
        _maxPreferredLogicalWidth = *sizes.LogicalWidth;
      }
      return LayoutUnit();
    }

    krys_debug_assert(phase == LayoutPhase::Layout);

    LayoutUnit inlineShift;
    if (sizes.LogicalWidth)
    {
      auto oldWidth = LogicalWidth();
      if (IsMathContentCentered())
      {
        inlineShift = (*sizes.LogicalWidth - oldWidth) / 2;
      }
      else if (!Style().IsLeftToRightDirection())
      {
        inlineShift = *sizes.LogicalWidth - oldWidth;
      }
      SetLogicalWidth(*sizes.LogicalWidth);
    }

    if (sizes.LogicalHeight)
    {
      SetLogicalHeight(*sizes.LogicalHeight);
    }

    return inlineShift;
  }
}
