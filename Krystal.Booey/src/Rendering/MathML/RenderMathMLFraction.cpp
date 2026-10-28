#include "Krystal.Booey/Rendering/MathML/RenderMathMLFraction.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Booey/MathML/MathMLFractionElement.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  using MathConstant = OpenTypeMathData::MathConstant;

  RenderMathMLFraction::RenderMathMLFraction(mathml::MathMLFractionElement &element,
                                             RenderStyle &&style) noexcept
      : RenderMathMLRow(Type::MathMLFraction, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMathMLFraction());
  }

  RenderMathMLFraction::~RenderMathMLFraction() noexcept = default;

  bool RenderMathMLFraction::IsValid() const noexcept
  {
    // Verify whether the list of children is valid:
    // <mfrac> numerator denominator </mfrac>
    auto *child = FirstInFlowChildBox();
    if (!child)
    {
      return false;
    }
    child = child->NextInFlowSiblingBox();
    return child && !child->NextInFlowSiblingBox();
  }

  RenderBox &RenderMathMLFraction::Numerator() const noexcept
  {
    krys_debug_assert(IsValid());
    return *FirstInFlowChildBox();
  }

  RenderBox &RenderMathMLFraction::Denominator() const noexcept
  {
    krys_debug_assert(IsValid());
    return *FirstInFlowChildBox()->NextInFlowSiblingBox();
  }

  LayoutUnit RenderMathMLFraction::DefaultLineThickness() const noexcept
  {
    auto primaryFont = Style().GetFontCascade().PrimaryFont();
    if (const auto *mathData = primaryFont->MathData())
    {
      return LayoutUnit(mathData->GetMathConstant(*primaryFont, MathConstant::FractionRuleThickness));
    }
    return RuleThicknessFallback();
  }

  LayoutUnit RenderMathMLFraction::LineThickness() const noexcept
  {
    return std::max<LayoutUnit>(ToUserUnits(GetElement().LineThickness(), Style(), DefaultLineThickness()),
                                0);
  }

  float RenderMathMLFraction::RelativeLineThickness() const noexcept
  {
    if (LayoutUnit defaultThickness = DefaultLineThickness())
    {
      return LineThickness() / defaultThickness;
    }
    return 0;
  }

  RenderMathMLFraction::FractionParameters RenderMathMLFraction::GetFractionParameters() const noexcept
  {
    // See https://mathml-refresh.github.io/mathml-core/#fraction-with-nonzero-line-thickness.

    krys_debug_assert(LineThickness());
    FractionParameters parameters;
    LayoutUnit numeratorGapMin, denominatorGapMin, numeratorMinShiftUp, denominatorMinShiftDown;

    // We try and read constants to draw the fraction from the OpenType MATH and use fallback values
    // otherwise.
    bool display = Style().GetMathStyle() == MathStyle::Normal;
    auto primaryFont = Style().GetFontCascade().PrimaryFont();
    if (const auto *mathData = primaryFont->MathData())
    {
      numeratorGapMin = LayoutUnit(
        mathData->GetMathConstant(*primaryFont, display ? MathConstant::FractionNumDisplayStyleGapMin
                                                        : MathConstant::FractionNumeratorGapMin));
      denominatorGapMin = LayoutUnit(
        mathData->GetMathConstant(*primaryFont, display ? MathConstant::FractionDenomDisplayStyleGapMin
                                                        : MathConstant::FractionDenominatorGapMin));
      numeratorMinShiftUp = LayoutUnit(
        mathData->GetMathConstant(*primaryFont, display ? MathConstant::FractionNumeratorDisplayStyleShiftUp
                                                        : MathConstant::FractionNumeratorShiftUp));
      denominatorMinShiftDown = LayoutUnit(mathData->GetMathConstant(
        *primaryFont, display ? MathConstant::FractionDenominatorDisplayStyleShiftDown
                              : MathConstant::FractionDenominatorShiftDown));
    }
    else
    {
      // The MATH table specification suggests default rule thickness or (in displaystyle) 3 times default
      // rule thickness for the gaps.
      numeratorGapMin = display ? 3 * RuleThicknessFallback() : RuleThicknessFallback();
      denominatorGapMin = numeratorGapMin;

      // The MATH table specification does not suggest any values for shifts, so we leave them at zero.
      numeratorMinShiftUp = 0;
      denominatorMinShiftDown = 0;
    }

    // Adjust fraction shifts to satisfy min gaps.
    LayoutUnit numeratorAscent = AscentForChild(Numerator()) + Numerator().MarginBefore();
    LayoutUnit numeratorDescent =
      Numerator().LogicalHeight() + Numerator().MarginLogicalHeight() - numeratorAscent;
    LayoutUnit denominatorAscent = AscentForChild(Denominator()) + Denominator().MarginBefore();
    LayoutUnit thickness = LineThickness();
    parameters.NumeratorShiftUp =
      std::max(numeratorMinShiftUp, MathAxisHeight() + thickness / 2 + numeratorGapMin + numeratorDescent);
    parameters.DenominatorShiftDown = std::max(
      denominatorMinShiftDown, thickness / 2 + denominatorGapMin + denominatorAscent - MathAxisHeight());

    return parameters;
  }

  RenderMathMLFraction::FractionParameters RenderMathMLFraction::GetStackParameters() const noexcept
  {
    // See https://mathml-refresh.github.io/mathml-core/#fraction-with-zero-line-thickness.

    krys_debug_assert(!LineThickness());
    krys_debug_assert(IsValid());
    FractionParameters parameters;
    LayoutUnit gapMin;

    // We try and read constants to draw the stack from the OpenType MATH and use fallback values otherwise.
    bool display = Style().GetMathStyle() == MathStyle::Normal;
    auto primaryFont = Style().GetFontCascade().PrimaryFont();
    if (const auto *mathData = primaryFont->MathData())
    {
      gapMin = LayoutUnit(mathData->GetMathConstant(
        *primaryFont, display ? MathConstant::StackDisplayStyleGapMin : MathConstant::StackGapMin));
      parameters.NumeratorShiftUp = LayoutUnit(mathData->GetMathConstant(
        *primaryFont, display ? MathConstant::StackTopDisplayStyleShiftUp : MathConstant::StackTopShiftUp));
      parameters.DenominatorShiftDown = LayoutUnit(
        mathData->GetMathConstant(*primaryFont, display ? MathConstant::StackBottomDisplayStyleShiftDown
                                                        : MathConstant::StackBottomShiftDown));
    }
    else
    {
      // We use the values suggested in the MATH table specification.
      gapMin = display ? 7 * RuleThicknessFallback() : 3 * RuleThicknessFallback();

      // The MATH table specification does not suggest any values for shifts, so we leave them at zero.
      parameters.NumeratorShiftUp = 0;
      parameters.DenominatorShiftDown = 0;
    }

    // Adjust fraction shifts to satisfy min gaps.
    LayoutUnit numeratorAscent = AscentForChild(Numerator()) + Numerator().MarginBefore();
    LayoutUnit numeratorDescent =
      Numerator().LogicalHeight() + Numerator().MarginLogicalHeight() - numeratorAscent;
    LayoutUnit denominatorAscent = AscentForChild(Denominator()) + Denominator().MarginBefore();
    LayoutUnit gap =
      parameters.NumeratorShiftUp - numeratorDescent + parameters.DenominatorShiftDown - denominatorAscent;
    if (gap < gapMin)
    {
      LayoutUnit delta = (gapMin - gap) / 2;
      parameters.NumeratorShiftUp += delta;
      parameters.DenominatorShiftDown += delta;
    }

    return parameters;
  }

  RenderMathMLOperator *RenderMathMLFraction::UnembellishedOperator() const noexcept
  {
    if (!IsValid())
    {
      return RenderMathMLRow::UnembellishedOperator();
    }

    auto *mathMLBlock = DynamicDowncast<RenderMathMLBlock>(Numerator());
    return mathMLBlock ? mathMLBlock->UnembellishedOperator() : nullptr;
  }

  mathml::MathMLFractionElement &RenderMathMLFraction::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLFractionElement &>(NodeForNonAnonymous());
  }

  void RenderMathMLFraction::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    if (!IsValid())
    {
      RenderMathMLRow::ComputePreferredLogicalWidths();
      return;
    }

    LayoutUnit numeratorWidth =
      Numerator().MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(Numerator());
    LayoutUnit denominatorWidth =
      Denominator().MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(Denominator());
    _minPreferredLogicalWidth = _maxPreferredLogicalWidth = std::max(numeratorWidth, denominatorWidth);

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::CalculatePreferredLogicalWidth);
    ApplySizeToMathContent(LayoutPhase::CalculatePreferredLogicalWidth, sizes);

    AdjustPreferredLogicalWidthsForBorderAndPadding();

    ClearNeedsPreferredWidthsUpdate();
  }

  LayoutUnit RenderMathMLFraction::HorizontalOffset(const RenderBox &child) const noexcept
  {
    // The numerator and denominator are centred.
    LayoutUnit contentBoxInlineSize = LogicalWidth();
    LayoutUnit childMarginBoxInlineSize = child.MarginStart() + child.LogicalWidth() + child.MarginEnd();
    return LayoutUnit((contentBoxInlineSize - childMarginBoxInlineSize) / 2);
  }

  LayoutUnit RenderMathMLFraction::FractionAscent() const noexcept
  {
    krys_debug_assert(IsValid());

    LayoutUnit numeratorAscent = AscentForChild(Numerator()) + Numerator().MarginBefore();
    if (LayoutUnit thickness = LineThickness())
    {
      return std::max(MathAxisHeight() + thickness / 2,
                      numeratorAscent + GetFractionParameters().NumeratorShiftUp);
    }

    return numeratorAscent + GetStackParameters().NumeratorShiftUp;
  }

  void RenderMathMLFraction::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    if (!IsValid())
    {
      RenderMathMLRow::LayoutBlock(relayoutChildren);
      return;
    }

    LayoutFloatingChildren();

    RecomputeLogicalWidth();

    Numerator().LayoutIfNeeded();
    Denominator().LayoutIfNeeded();
    ComputeAndSetBlockDirectionMarginsOfChildren();

    LayoutUnit numeratorMarginBoxInlineSize =
      Numerator().MarginStart() + Numerator().LogicalWidth() + Numerator().MarginEnd();
    LayoutUnit denominatorMarginBoxInlineSize =
      Denominator().MarginStart() + Denominator().LogicalWidth() + Denominator().MarginEnd();
    SetLogicalWidth(std::max(numeratorMarginBoxInlineSize, denominatorMarginBoxInlineSize));

    LayoutUnit verticalOffset = 0; // This is the top of the renderer.
    verticalOffset += Numerator().MarginBefore();
    LayoutPoint numeratorLocation(Numerator().MarginLeft() + HorizontalOffset(Numerator()), verticalOffset);
    Numerator().SetLocation(numeratorLocation);

    LayoutUnit denominatorAscent = AscentForChild(Denominator()) + Denominator().MarginBefore();
    verticalOffset = FractionAscent();
    FractionParameters parameters = LineThickness() ? GetFractionParameters() : GetStackParameters();
    verticalOffset += parameters.DenominatorShiftDown - denominatorAscent;

    verticalOffset += Denominator().MarginBefore();
    LayoutPoint denominatorLocation(Denominator().MarginLeft() + HorizontalOffset(Denominator()),
                                    verticalOffset);
    Denominator().SetLocation(denominatorLocation);

    verticalOffset +=
      Denominator().LogicalHeight() + Denominator().MarginAfter(); // This is the bottom of our renderer.

    SetLogicalHeight(verticalOffset);

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::Layout);
    auto shift = ApplySizeToMathContent(LayoutPhase::Layout, sizes);
    ShiftInFlowChildren(shift, 0);

    AdjustLayoutForBorderAndPadding();

    LayoutOutOfFlowBoxes(relayoutChildren);
  }

  void RenderMathMLFraction::Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept
  {
    RenderMathMLRow::Paint(info, paintOffset);
    LayoutUnit thickness = LineThickness();
    if (info.Context().PaintingDisabled() || info.Phase != PaintPhase::Foreground
        || Style().UsedVisibility() != Visibility::Visible || !IsValid() || !thickness)
    {
      return;
    }

    LayoutUnit borderAndPaddingLeft =
      GetWritingMode().IsBidiLTR() ? BorderAndPaddingStart() : BorderAndPaddingEnd();
    float deviceScaleFactor = GetDocument().DeviceScaleFactor();
    auto adjustedPaintOffset = RoundPointToDevicePixels(
      paintOffset + Location()
        + LayoutPoint(borderAndPaddingLeft, BorderAndPaddingBefore() + FractionAscent() - MathAxisHeight()),
      deviceScaleFactor);

    GraphicsContextStateSaver stateSaver(info.Context());

    info.Context().SetStrokeThickness(thickness);
    info.Context().SetStrokeStyle(StrokeStyle::SolidStroke);
    info.Context().SetStrokeColor(Style().VisitedDependentColorWithColorFilter(css::PropertyId::Color));
    // MathML Core says the fraction bar takes the full width of the content box.
    auto endPoint = RoundPointToDevicePixels(
      LayoutPoint(LayoutUnit(adjustedPaintOffset.X()) + LogicalWidth() - BorderAndPaddingLogicalWidth(),
                  LayoutUnit(adjustedPaintOffset.Y())),
      deviceScaleFactor);
    info.Context().DrawLine(adjustedPaintOffset, endPoint);
  }

  Maybe<LayoutUnit> RenderMathMLFraction::FirstLineBaseline() const noexcept
  {
    if (IsValid())
    {
      // Subpixel inline layout is off, so the baseline is rounded.
      return LayoutUnit(std::round((BorderAndPaddingBefore() + FractionAscent()).ToFloat()));
    }
    return RenderMathMLRow::FirstLineBaseline();
  }
}
