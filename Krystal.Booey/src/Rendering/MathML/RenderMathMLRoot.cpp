#include "Krystal.Booey/Rendering/MathML/RenderMathMLRoot.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Booey/MathML/MathMLRootElement.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    constexpr char32 RadicalCharacter = 0x221A;
  }

  using MathConstant = OpenTypeMathData::MathConstant;
  using mathml::RootType;

  RenderMathMLRoot::RenderMathMLRoot(mathml::MathMLRootElement &element, RenderStyle &&style) noexcept
      : RenderMathMLRow(Type::MathMLRoot, element, krys::move(style))
  {
    _radicalOperator.SetOperator(Style(), RadicalCharacter, MathOperator::Type::VerticalOperator);
    krys_debug_assert(IsRenderMathMLRoot());
  }

  RenderMathMLRoot::~RenderMathMLRoot() noexcept = default;

  mathml::MathMLRootElement &RenderMathMLRoot::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLRootElement &>(NodeForNonAnonymous());
  }

  RootType RenderMathMLRoot::GetRootType() const noexcept
  {
    return GetElement().GetRootType();
  }

  bool RenderMathMLRoot::IsValid() const noexcept
  {
    // Verify whether the list of children is valid:
    // <msqrt> child1 child2 ... childN </msqrt>
    // <mroot> base index </mroot>
    if (GetRootType() == RootType::SquareRoot)
    {
      return true;
    }

    krys_debug_assert(GetRootType() == RootType::RootWithIndex);
    auto *child = FirstInFlowChildBox();
    if (!child)
    {
      return false;
    }
    child = child->NextInFlowSiblingBox();
    return child && !child->NextInFlowSiblingBox();
  }

  RenderBox &RenderMathMLRoot::GetBase() const noexcept
  {
    krys_debug_assert(IsValid());
    krys_debug_assert(GetRootType() == RootType::RootWithIndex);
    return *FirstInFlowChildBox();
  }

  RenderBox &RenderMathMLRoot::GetIndex() const noexcept
  {
    krys_debug_assert(IsValid());
    krys_debug_assert(GetRootType() == RootType::RootWithIndex);
    return *FirstInFlowChildBox()->NextInFlowSiblingBox();
  }

  void RenderMathMLRoot::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderMathMLRow::StyleDidChange(diff, oldStyle);
    ResetRadicalOperator();
  }

  RenderMathMLRoot::HorizontalParameters
    RenderMathMLRoot::GetHorizontalParameters(LayoutUnit indexWidth) const noexcept
  {
    // Square roots do not require horizontal parameters.
    krys_debug_assert(GetRootType() == RootType::RootWithIndex);
    HorizontalParameters parameters;

    // We try and read constants to draw the radical from the OpenType MATH and use fallback values otherwise.
    auto primaryFont = Style().GetFontCascade().PrimaryFont();
    if (const auto *mathData = primaryFont->MathData())
    {
      parameters.KernBeforeDegree =
        LayoutUnit(mathData->GetMathConstant(*primaryFont, MathConstant::RadicalKernBeforeDegree));
      parameters.KernAfterDegree =
        LayoutUnit(mathData->GetMathConstant(*primaryFont, MathConstant::RadicalKernAfterDegree));
    }
    else
    {
      // RadicalKernBeforeDegree: No suggested value provided. OT Math Illuminated mentions 5/18 em, Gecko
      // uses 0. RadicalKernAfterDegree: Suggested value is -10/18 of em.
      parameters.KernBeforeDegree = LayoutUnit(5 * Style().GetFontCascade().Size() / 18);
      parameters.KernAfterDegree = LayoutUnit(-10 * Style().GetFontCascade().Size() / 18);
    }
    // Apply clamping from https://w3c.github.io/mathml-core/#root-with-index
    parameters.KernBeforeDegree = std::max<LayoutUnit>(0, parameters.KernBeforeDegree);
    parameters.KernAfterDegree = std::max<LayoutUnit>(-indexWidth, parameters.KernAfterDegree);
    return parameters;
  }

  RenderMathMLRoot::VerticalParameters RenderMathMLRoot::GetVerticalParameters() const noexcept
  {
    VerticalParameters parameters;
    // We try and read constants to draw the radical from the OpenType MATH and use fallback values otherwise.
    auto primaryFont = Style().GetFontCascade().PrimaryFont();
    if (const auto *mathData = primaryFont->MathData())
    {
      parameters.RuleThickness =
        LayoutUnit(mathData->GetMathConstant(*primaryFont, MathConstant::RadicalRuleThickness));
      parameters.VerticalGap =
        LayoutUnit(mathData->GetMathConstant(*primaryFont, Style().GetMathStyle() == MathStyle::Normal
                                                             ? MathConstant::RadicalDisplayStyleVerticalGap
                                                             : MathConstant::RadicalVerticalGap));
      parameters.ExtraAscender =
        LayoutUnit(mathData->GetMathConstant(*primaryFont, MathConstant::RadicalExtraAscender));
      if (GetRootType() == RootType::RootWithIndex)
      {
        parameters.DegreeBottomRaisePercent =
          mathData->GetMathConstant(*primaryFont, MathConstant::RadicalDegreeBottomRaisePercent);
      }
    }
    else
    {
      // RadicalVerticalGap: Suggested value is 5/4 default rule thickness.
      // RadicalDisplayStyleVerticalGap: Suggested value is default rule thickness + 1/4 x-height.
      // RadicalRuleThickness: Suggested value is default rule thickness.
      // RadicalExtraAscender: Suggested value is RadicalRuleThickness.
      // RadicalDegreeBottomRaisePercent: Suggested value is 60%.
      parameters.RuleThickness = RuleThicknessFallback();
      if (Style().GetMathStyle() == MathStyle::Normal)
      {
        parameters.VerticalGap =
          parameters.RuleThickness + LayoutUnit(Style().MetricsOfPrimaryFont().XHeight().value_or(0) / 4);
      }
      else
      {
        parameters.VerticalGap = 5 * parameters.RuleThickness / 4;
      }

      if (GetRootType() == RootType::RootWithIndex)
      {
        parameters.ExtraAscender = parameters.RuleThickness;
        parameters.DegreeBottomRaisePercent = 0.6f;
      }
    }
    return parameters;
  }

  void RenderMathMLRoot::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    if (!IsValid())
    {
      RenderMathMLRow::ComputePreferredLogicalWidths();
      return;
    }

    LayoutUnit preferredWidth = 0;
    if (GetRootType() == RootType::SquareRoot)
    {
      preferredWidth += _radicalOperator.MaxPreferredWidth();
      preferredWidth += PreferredLogicalWidthOfRowItems();
    }
    else
    {
      krys_debug_assert(GetRootType() == RootType::RootWithIndex);
      LayoutUnit indexPreferredWidth =
        GetIndex().MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(GetIndex());
      auto horizontal = GetHorizontalParameters(indexPreferredWidth);
      preferredWidth += horizontal.KernBeforeDegree;
      preferredWidth += indexPreferredWidth;
      preferredWidth += horizontal.KernAfterDegree;
      preferredWidth += _radicalOperator.MaxPreferredWidth();
      preferredWidth += GetBase().MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(GetBase());
    }
    _minPreferredLogicalWidth = _maxPreferredLogicalWidth = preferredWidth;

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::CalculatePreferredLogicalWidth);
    ApplySizeToMathContent(LayoutPhase::CalculatePreferredLogicalWidth, sizes);

    AdjustPreferredLogicalWidthsForBorderAndPadding();

    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderMathMLRoot::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    _radicalOperatorTop = 0;
    _baseWidth = 0;

    if (!IsValid())
    {
      RenderMathMLRow::LayoutBlock(relayoutChildren);
      return;
    }

    LayoutFloatingChildren();

    // We layout the children, determine the vertical metrics of the base and set the logical width.
    // Note: Per the MathML specification, the children of <msqrt> are wrapped in an inferred <mrow>, which is
    // the desired base.
    LayoutUnit baseAscent, baseDescent;
    RecomputeLogicalWidth();
    ComputeAndSetBlockDirectionMarginsOfChildren();
    if (GetRootType() == RootType::SquareRoot)
    {
      StretchVerticalOperatorsAndLayoutChildren();
      GetContentBoundingBox(_baseWidth, baseAscent, baseDescent);
      LayoutRowItems(_baseWidth, baseAscent);
    }
    else
    {
      GetBase().LayoutIfNeeded();
      _baseWidth = GetBase().LogicalWidth() + GetBase().MarginLogicalWidth();
      baseAscent = AscentForChild(GetBase()) + GetBase().MarginBefore();
      baseDescent = GetBase().LogicalHeight() + GetBase().MarginLogicalHeight() - baseAscent;
      GetIndex().LayoutIfNeeded();
    }

    HorizontalParameters horizontal;
    auto vertical = GetVerticalParameters();

    // Stretch the radical operator to cover the base height.
    // We can then determine the metrics of the radical operator + the base.
    _radicalOperator.StretchTo(Style(),
                               baseAscent + baseDescent + vertical.VerticalGap + vertical.RuleThickness);
    LayoutUnit radicalOperatorHeight = _radicalOperator.Ascent() + _radicalOperator.Descent();
    LayoutUnit indexBottomRaise {vertical.DegreeBottomRaisePercent * radicalOperatorHeight};
    LayoutUnit radicalAscent =
      baseAscent + vertical.VerticalGap + vertical.RuleThickness + vertical.ExtraAscender;
    LayoutUnit radicalDescent =
      std::max<LayoutUnit>(baseDescent, radicalOperatorHeight + vertical.ExtraAscender - radicalAscent);
    LayoutUnit descent = radicalDescent;
    LayoutUnit ascent = radicalAscent;

    // We set the logical width.
    if (GetRootType() == RootType::SquareRoot)
    {
      SetLogicalWidth(_radicalOperator.Width() + _baseWidth);
    }
    else
    {
      krys_debug_assert(GetRootType() == RootType::RootWithIndex);
      LayoutUnit indexWidth = GetIndex().LogicalWidth() + GetIndex().MarginLogicalWidth();
      horizontal = GetHorizontalParameters(indexWidth);
      SetLogicalWidth(horizontal.KernBeforeDegree + indexWidth + horizontal.KernAfterDegree
                      + _radicalOperator.Width() + _baseWidth);
    }

    // For <mroot>, we update the metrics to take into account the index.
    LayoutUnit indexAscent, indexDescent;
    if (GetRootType() == RootType::RootWithIndex)
    {
      indexAscent = AscentForChild(GetIndex()) + GetIndex().MarginBefore();
      indexDescent = GetIndex().LogicalHeight() + GetIndex().MarginLogicalHeight() - indexAscent;
      ascent = std::max<LayoutUnit>(radicalAscent, indexBottomRaise + indexDescent + indexAscent - descent);
    }

    // We set the final position of children.
    _radicalOperatorTop = ascent - radicalAscent + vertical.ExtraAscender;
    LayoutUnit horizontalOffset = _radicalOperator.Width();
    if (GetRootType() == RootType::RootWithIndex)
    {
      horizontalOffset += horizontal.KernBeforeDegree + GetIndex().LogicalWidth()
                          + GetIndex().MarginLogicalWidth() + horizontal.KernAfterDegree;
    }
    if (GetRootType() == RootType::SquareRoot)
    {
      LayoutPoint baseLocation(MirrorIfNeeded(horizontalOffset, _baseWidth), ascent - baseAscent);
      for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
      {
        child->SetLocation(child->Location() + baseLocation);
      }
    }
    else
    {
      krys_debug_assert(GetRootType() == RootType::RootWithIndex);
      LayoutPoint baseLocation(MirrorIfNeeded(horizontalOffset + GetBase().MarginStart(), GetBase()),
                               ascent - baseAscent + GetBase().MarginBefore());
      GetBase().SetLocation(baseLocation);
      LayoutPoint indexLocation(
        MirrorIfNeeded(horizontal.KernBeforeDegree + GetIndex().MarginStart(), GetIndex()),
        ascent + descent - indexBottomRaise - indexDescent - indexAscent + GetIndex().MarginBefore());
      GetIndex().SetLocation(indexLocation);
    }

    SetLogicalHeight(ascent + descent);

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::Layout);
    auto shift = ApplySizeToMathContent(LayoutPhase::Layout, sizes);
    ShiftInFlowChildren(shift, 0);

    AdjustLayoutForBorderAndPadding();

    LayoutOutOfFlowBoxes(relayoutChildren);
  }

  void RenderMathMLRoot::Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept
  {
    RenderMathMLRow::Paint(info, paintOffset);

    if (info.Context().PaintingDisabled() || info.Phase != PaintPhase::Foreground
        || Style().UsedVisibility() != Visibility::Visible || !IsValid())
    {
      return;
    }

    // We draw the radical operator.
    LayoutPoint radicalOperatorTopLeft = paintOffset + Location();
    LayoutUnit horizontalOffset = BorderAndPaddingStart();
    if (GetRootType() == RootType::RootWithIndex)
    {
      LayoutUnit indexWidth = GetIndex().LogicalWidth() + GetIndex().MarginLogicalWidth();
      auto horizontal = GetHorizontalParameters(indexWidth);
      horizontalOffset += horizontal.KernBeforeDegree + indexWidth + horizontal.KernAfterDegree;
    }
    radicalOperatorTopLeft.Move(MirrorIfNeeded(horizontalOffset, _radicalOperator.Width()),
                                _radicalOperatorTop);
    _radicalOperator.Paint(Style(), info, radicalOperatorTopLeft, GetDocument().DeviceScaleFactor());

    // We draw the radical line.
    LayoutUnit ruleThickness = GetVerticalParameters().RuleThickness;
    if (!ruleThickness)
    {
      return;
    }
    GraphicsContextStateSaver stateSaver(info.Context());

    info.Context().SetStrokeThickness(ruleThickness);
    info.Context().SetStrokeStyle(StrokeStyle::SolidStroke);
    info.Context().SetStrokeColor(Style().VisitedDependentColorWithColorFilter(css::PropertyId::Color));
    LayoutPoint ruleOffsetFrom =
      paintOffset + Location() + LayoutPoint(0_lu, _radicalOperatorTop + ruleThickness / 2);
    LayoutPoint ruleOffsetTo = ruleOffsetFrom;
    horizontalOffset += _radicalOperator.Width();
    ruleOffsetFrom.Move(MirrorIfNeeded(horizontalOffset), 0_lu);
    horizontalOffset += _baseWidth;
    ruleOffsetTo.Move(MirrorIfNeeded(horizontalOffset), 0_lu);
    info.Context().DrawLine(ruleOffsetFrom, ruleOffsetTo);
  }
}
