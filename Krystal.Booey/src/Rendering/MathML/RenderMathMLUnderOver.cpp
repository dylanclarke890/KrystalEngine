#include "Krystal.Booey/Rendering/MathML/RenderMathMLUnderOver.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Booey/MathML/MathMLOperatorDictionary.hpp"
#include "Krystal.Booey/MathML/MathMLUnderOverElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <algorithm>

namespace krys::boo
{
  using MathConstant = OpenTypeMathData::MathConstant;
  using ScriptType = mathml::MathMLScriptsElement::ScriptType;
  using mathml::MathMLOperatorDictionary;

  RenderMathMLUnderOver::RenderMathMLUnderOver(mathml::MathMLUnderOverElement &element,
                                               RenderStyle &&style) noexcept
      : RenderMathMLScripts(Type::MathMLUnderOver, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMathMLUnderOver());
  }

  RenderMathMLUnderOver::~RenderMathMLUnderOver() noexcept = default;

  mathml::MathMLUnderOverElement &RenderMathMLUnderOver::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLUnderOverElement &>(NodeForNonAnonymous());
  }

  namespace
  {
    KRYS_NODISCARD RenderMathMLOperator *HorizontalStretchyOperator(const RenderBox &box) noexcept
    {
      auto *mathMLBlock = DynamicDowncast<RenderMathMLBlock>(box);
      if (!mathMLBlock)
      {
        return nullptr;
      }

      auto *renderOperator = mathMLBlock->UnembellishedOperator();
      if (!renderOperator)
      {
        return nullptr;
      }

      if (!renderOperator->IsStretchy() || renderOperator->IsVertical()
          || renderOperator->IsStretchWidthLocked())
      {
        return nullptr;
      }

      return renderOperator;
    }

    void FixLayoutAfterStretch(RenderBox &ancestor, RenderMathMLOperator &stretchyOperator) noexcept
    {
      stretchyOperator.SetStretchWidthLocked(true);
      stretchyOperator.SetNeedsLayout();
      ancestor.LayoutIfNeeded();
      stretchyOperator.SetStretchWidthLocked(false);
    }
  }

  void RenderMathMLUnderOver::StretchHorizontalOperatorsAndLayoutChildren() noexcept
  {
    krys_debug_assert(IsValid());
    krys_debug_assert(NeedsLayout());

    // We apply horizontal stretchy rules from the MathML spec (sections 3.2.5.8.3 and 3.2.5.8.4), which
    // can be roughly summarized as "stretching opersators to the maximum widths of all children" and
    // minor variations of that algorithm do not affect the result. However, the spec is a bit ambiguous
    // for embellished operators (section 3.2.5.7.3) and different approaches can lead to significant
    // stretch size differences. We made the following decisions:
    // - The unstretched size is the embellished operator width with the <mo> at the core unstretched.
    // - In general, the target size is just the maximum widths of non-stretchy children because the
    // embellishments could make widths significantly larger.
    // - In the edge case when all operators of stretchy, we follow the specification and take the
    // maximum of all unstretched sizes.
    // - The <mo> at the core is stretched to cover the target size, even if the embellished operator
    // might become much wider.

    List<RenderBox *> embellishedOperators;
    List<RenderMathMLOperator *> stretchyOperators;
    bool isAllStretchyOperators = true;
    LayoutUnit stretchWidth;

    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      if (auto *stretchyOperator = HorizontalStretchyOperator(*child))
      {
        embellishedOperators.push_back(child);
        stretchyOperators.push_back(stretchyOperator);
      }
      else
      {
        isAllStretchyOperators = false;
        child->LayoutIfNeeded();
        stretchWidth = std::max(stretchWidth, child->LogicalWidth() + child->MarginLogicalWidth());
      }
    }

    if (isAllStretchyOperators)
    {
      for (size_t i = 0; i < embellishedOperators.size(); i++)
      {
        stretchyOperators[i]->ResetStretchSize();
        FixLayoutAfterStretch(*embellishedOperators[i], *stretchyOperators[i]);
        stretchWidth = std::max(stretchWidth, embellishedOperators[i]->LogicalWidth()
                                                + embellishedOperators[i]->MarginLogicalWidth());
      }
    }

    for (size_t i = 0; i < embellishedOperators.size(); i++)
    {
      stretchyOperators[i]->StretchTo(stretchWidth);
      FixLayoutAfterStretch(*embellishedOperators[i], *stretchyOperators[i]);
    }
  }

  bool RenderMathMLUnderOver::IsValid() const noexcept
  {
    // Verify whether the list of children is valid:
    // <munder> base under </munder>
    // <mover> base over </mover>
    // <munderover> base under over </munderover>
    auto *child = FirstInFlowChildBox();
    if (!child)
    {
      return false;
    }
    child = child->NextInFlowSiblingBox();
    if (!child)
    {
      return false;
    }
    child = child->NextInFlowSiblingBox();
    switch (GetScriptType())
    {
      case ScriptType::Over:
      case ScriptType::Under:     return !child;
      case ScriptType::UnderOver: return child && !child->NextInFlowSiblingBox();
      default:                    krys_debug_assert(false); return false;
    }
  }

  bool RenderMathMLUnderOver::ShouldMoveLimits() const noexcept
  {
    if (Style().GetMathStyle() == MathStyle::Normal)
    {
      return false;
    }
    if (auto *renderOperator = UnembellishedOperator())
    {
      return renderOperator->ShouldMoveLimits();
    }
    return false;
  }

  RenderBox &RenderMathMLUnderOver::Base() const noexcept
  {
    krys_debug_assert(IsValid());
    return *FirstInFlowChildBox();
  }

  RenderBox &RenderMathMLUnderOver::Under() const noexcept
  {
    krys_debug_assert(IsValid());
    krys_debug_assert(GetScriptType() == ScriptType::Under || GetScriptType() == ScriptType::UnderOver);
    return *FirstInFlowChildBox()->NextInFlowSiblingBox();
  }

  RenderBox &RenderMathMLUnderOver::Over() const noexcept
  {
    krys_debug_assert(IsValid());
    krys_debug_assert(GetScriptType() == ScriptType::Over || GetScriptType() == ScriptType::UnderOver);
    auto *secondChild = FirstInFlowChildBox()->NextInFlowSiblingBox();
    return GetScriptType() == ScriptType::Over ? *secondChild : *secondChild->NextInFlowSiblingBox();
  }

  void RenderMathMLUnderOver::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    if (!IsValid())
    {
      RenderMathMLRow::ComputePreferredLogicalWidths();
      return;
    }

    if (ShouldMoveLimits())
    {
      RenderMathMLScripts::ComputePreferredLogicalWidths();
      return;
    }

    LayoutUnit preferredWidth =
      Base().MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(Base());

    if (GetScriptType() == ScriptType::Under || GetScriptType() == ScriptType::UnderOver)
    {
      preferredWidth = std::max(preferredWidth, Under().MaxPreferredLogicalWidth()
                                                  + MarginIntrinsicLogicalWidthForChild(Under()));
    }

    if (GetScriptType() == ScriptType::Over || GetScriptType() == ScriptType::UnderOver)
    {
      preferredWidth = std::max(preferredWidth, Over().MaxPreferredLogicalWidth()
                                                  + MarginIntrinsicLogicalWidthForChild(Over()));
    }

    _minPreferredLogicalWidth = _maxPreferredLogicalWidth = preferredWidth;

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::CalculatePreferredLogicalWidth);
    ApplySizeToMathContent(LayoutPhase::CalculatePreferredLogicalWidth, sizes);

    AdjustPreferredLogicalWidthsForBorderAndPadding();

    ClearNeedsPreferredWidthsUpdate();
  }

  LayoutUnit RenderMathMLUnderOver::HorizontalOffset(const RenderBox &child) const noexcept
  {
    LayoutUnit contentBoxInlineSize = LogicalWidth();
    LayoutUnit childMarginBoxInlineSize = child.LogicalWidth() + child.MarginLogicalWidth();
    return (contentBoxInlineSize - childMarginBoxInlineSize) / 2 + child.MarginLeft();
  }

  bool RenderMathMLUnderOver::HasAccent(bool accentUnder) const noexcept
  {
    krys_debug_assert(GetScriptType() == ScriptType::UnderOver
                      || (accentUnder && GetScriptType() == ScriptType::Under)
                      || (!accentUnder && GetScriptType() == ScriptType::Over));

    auto attributeValue = accentUnder ? GetElement().AccentUnder() : GetElement().Accent();
    if (attributeValue == mathml::MathMLElement::BooleanValue::True)
    {
      return true;
    }
    if (attributeValue == mathml::MathMLElement::BooleanValue::False)
    {
      return false;
    }
    auto *script = DynamicDowncast<RenderMathMLBlock>(accentUnder ? Under() : Over());
    if (!script)
    {
      return false;
    }
    auto *scriptOperator = script->UnembellishedOperator();
    return scriptOperator && scriptOperator->HasOperatorFlag(MathMLOperatorDictionary::Accent);
  }

  RenderMathMLUnderOver::VerticalParameters RenderMathMLUnderOver::GetVerticalParameters() const noexcept
  {
    // By default, we set all values to zero.
    VerticalParameters parameters;

    auto primaryFont = Style().GetFontCascade().PrimaryFont();
    const auto *mathData = primaryFont->MathData();
    if (!mathData)
    {
      // The MATH table specification does not really provide any suggestions, except for some
      // underbar/overbar values and AccentBaseHeight.
      LayoutUnit defaultLineThickness = RuleThicknessFallback();
      parameters.UnderGapMin = 3 * defaultLineThickness;
      parameters.OverGapMin = 3 * defaultLineThickness;
      parameters.UnderExtraDescender = defaultLineThickness;
      parameters.OverExtraAscender = defaultLineThickness;
      parameters.AccentBaseHeight = LayoutUnit(Style().MetricsOfPrimaryFont().XHeight().value_or(0));
      parameters.UseUnderOverBarFallBack = true;
      return parameters;
    }

    auto constant = [&](MathConstant name)
    {
      return LayoutUnit(mathData->GetMathConstant(*primaryFont, name));
    };

    if (auto *mathMLBlock = DynamicDowncast<RenderMathMLBlock>(Base()))
    {
      if (auto *baseOperator = mathMLBlock->UnembellishedOperator())
      {
        if (baseOperator->HasOperatorFlag(MathMLOperatorDictionary::LargeOp))
        {
          // The base is a large operator so we read UpperLimit/LowerLimit constants from the MATH table.
          parameters.UnderGapMin = constant(MathConstant::LowerLimitGapMin);
          parameters.OverGapMin = constant(MathConstant::UpperLimitGapMin);
          parameters.UnderShiftMin = constant(MathConstant::LowerLimitBaselineDropMin);
          parameters.OverShiftMin = constant(MathConstant::UpperLimitBaselineRiseMin);
          parameters.UseUnderOverBarFallBack = false;
          return parameters;
        }
        if (baseOperator->IsStretchy() && !baseOperator->IsVertical())
        {
          // The base is a horizontal stretchy operator, so we read StretchStack constants from the MATH
          // table.
          parameters.UnderGapMin = constant(MathConstant::StretchStackGapBelowMin);
          parameters.OverGapMin = constant(MathConstant::StretchStackGapAboveMin);
          parameters.UnderShiftMin = constant(MathConstant::StretchStackBottomShiftDown);
          parameters.OverShiftMin = constant(MathConstant::StretchStackTopShiftUp);
          parameters.UseUnderOverBarFallBack = false;
          return parameters;
        }
      }
    }

    // By default, we just use the underbar/overbar constants.
    parameters.UnderGapMin = constant(MathConstant::UnderbarVerticalGap);
    parameters.OverGapMin = constant(MathConstant::OverbarVerticalGap);
    parameters.UnderExtraDescender = constant(MathConstant::UnderbarExtraDescender);
    parameters.OverExtraAscender = constant(MathConstant::OverbarExtraAscender);
    parameters.AccentBaseHeight = constant(MathConstant::AccentBaseHeight);
    parameters.UseUnderOverBarFallBack = true;
    return parameters;
  }

  void RenderMathMLUnderOver::LayoutBlock(RelayoutChildren relayoutChildren,
                                          LayoutUnit pageLogicalHeight) noexcept
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

    if (ShouldMoveLimits())
    {
      RenderMathMLScripts::LayoutBlock(relayoutChildren, pageLogicalHeight);
      return;
    }

    LayoutFloatingChildren();

    RecomputeLogicalWidth();
    ComputeAndSetBlockDirectionMarginsOfChildren();

    StretchHorizontalOperatorsAndLayoutChildren();

    krys_debug_assert(!Base().NeedsLayout());
    krys_debug_assert(GetScriptType() == ScriptType::Over || !Under().NeedsLayout());
    krys_debug_assert(GetScriptType() == ScriptType::Under || !Over().NeedsLayout());

    LayoutUnit logicalWidth = Base().LogicalWidth() + Base().MarginLogicalWidth();
    if (GetScriptType() == ScriptType::Under || GetScriptType() == ScriptType::UnderOver)
    {
      logicalWidth = std::max(logicalWidth, Under().LogicalWidth() + Under().MarginLogicalWidth());
    }
    if (GetScriptType() == ScriptType::Over || GetScriptType() == ScriptType::UnderOver)
    {
      logicalWidth = std::max(logicalWidth, Over().LogicalWidth() + Over().MarginLogicalWidth());
    }
    SetLogicalWidth(logicalWidth);

    VerticalParameters parameters = GetVerticalParameters();
    LayoutUnit verticalOffset = 0;
    if (GetScriptType() == ScriptType::Over || GetScriptType() == ScriptType::UnderOver)
    {
      verticalOffset += parameters.OverExtraAscender;
      verticalOffset += Over().MarginBefore();
      Over().SetLocation(LayoutPoint(HorizontalOffset(Over()), verticalOffset));
      if (parameters.UseUnderOverBarFallBack)
      {
        verticalOffset += Over().LogicalHeight();
        verticalOffset += Over().MarginAfter();
        if (HasAccent())
        {
          LayoutUnit baseAscent = AscentForChild(Base()) + Base().MarginBefore();
          if (baseAscent < parameters.AccentBaseHeight)
          {
            verticalOffset += parameters.AccentBaseHeight - baseAscent;
          }
        }
        else
        {
          verticalOffset += parameters.OverGapMin;
        }
      }
      else
      {
        LayoutUnit overAscent = AscentForChild(Over()) + Over().MarginBefore();
        verticalOffset += std::max(Over().LogicalHeight() + Over().MarginAfter() + parameters.OverGapMin,
                                   overAscent + parameters.OverShiftMin);
      }
    }
    verticalOffset += Base().MarginBefore();
    Base().SetLocation(LayoutPoint(HorizontalOffset(Base()), verticalOffset));
    verticalOffset += Base().LogicalHeight();
    verticalOffset += Base().MarginAfter();
    if (GetScriptType() == ScriptType::Under || GetScriptType() == ScriptType::UnderOver)
    {
      if (parameters.UseUnderOverBarFallBack)
      {
        if (!HasAccentUnder())
        {
          verticalOffset += parameters.UnderGapMin;
        }
      }
      else
      {
        LayoutUnit underAscent = AscentForChild(Under()) + Under().MarginBefore();
        verticalOffset += std::max(parameters.UnderGapMin, parameters.UnderShiftMin - underAscent);
      }
      verticalOffset += Under().MarginBefore();
      Under().SetLocation(LayoutPoint(HorizontalOffset(Under()), verticalOffset));
      verticalOffset += Under().LogicalHeight();
      verticalOffset += Under().MarginAfter();
      verticalOffset += parameters.UnderExtraDescender;
    }

    SetLogicalHeight(verticalOffset);

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::Layout);
    auto shift = ApplySizeToMathContent(LayoutPhase::Layout, sizes);
    ShiftInFlowChildren(shift, 0);

    AdjustLayoutForBorderAndPadding();

    LayoutOutOfFlowBoxes(relayoutChildren);
  }
}
