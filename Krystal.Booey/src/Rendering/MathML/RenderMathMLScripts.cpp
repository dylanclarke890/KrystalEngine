#include "Krystal.Booey/Rendering/MathML/RenderMathMLScripts.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  using MathConstant = OpenTypeMathData::MathConstant;
  using ScriptType = mathml::MathMLScriptsElement::ScriptType;

  namespace
  {
    KRYS_NODISCARD bool IsPrescriptDelimiter(const RenderObject &renderObject) noexcept
    {
      auto *element = DynamicDowncast<mathml::MathMLElement>(renderObject.GetNode());
      return element && element->HasTagName(html::TagName::mprescripts);
    }
  }

  RenderMathMLScripts::RenderMathMLScripts(Type type, mathml::MathMLScriptsElement &element,
                                           RenderStyle &&style) noexcept
      : RenderMathMLRow(type, element, krys::move(style))
  {
  }

  RenderMathMLScripts::~RenderMathMLScripts() noexcept = default;

  mathml::MathMLScriptsElement &RenderMathMLScripts::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLScriptsElement &>(NodeForNonAnonymous());
  }

  ScriptType RenderMathMLScripts::GetScriptType() const noexcept
  {
    return GetElement().GetScriptType();
  }

  RenderMathMLOperator *RenderMathMLScripts::UnembellishedOperator() const noexcept
  {
    auto possibleReference = ValidateAndGetReferenceChildren();
    if (!possibleReference)
    {
      return RenderMathMLRow::UnembellishedOperator();
    }

    auto *base = DynamicDowncast<RenderMathMLBlock>(possibleReference->Base);
    return base ? base->UnembellishedOperator() : nullptr;
  }

  auto RenderMathMLScripts::ValidateAndGetReferenceChildren() const noexcept -> Maybe<ReferenceChildren>
  {
    // All scripted elements must have at least one in-flow child.
    // The first child is the base.
    auto *base = FirstInFlowChildBox();
    if (!base)
    {
      return {};
    }

    ReferenceChildren reference;
    reference.Base = base;

    switch (GetScriptType())
    {
      case ScriptType::Sub:
      case ScriptType::Super:
      case ScriptType::Under:
      case ScriptType::Over:
      {
        // These elements must have exactly two in-flow children.
        // The second child is a postscript and there are no prescripts.
        // <msub> base subscript </msub>
        // <msup> base superscript </msup>
        // <munder> base underscript </munder>
        // <mover> base overscript </mover>
        auto *script = base->NextInFlowSiblingBox();
        if (!script || IsPrescriptDelimiter(*script) || script->NextInFlowSiblingBox())
        {
          return {};
        }
        reference.FirstPostScript = script;
        return reference;
      }
      case ScriptType::SubSup:
      case ScriptType::UnderOver:
      {
        // These elements must have exactly three in-flow children.
        // The second and third children are postscripts and there are no prescripts.
        // <msubsup> base subscript superscript </msubsup>
        // <munderover> base subscript superscript </munderover>
        auto *subScript = base->NextInFlowSiblingBox();
        if (!subScript || IsPrescriptDelimiter(*subScript))
        {
          return {};
        }
        auto *superScript = subScript->NextInFlowSiblingBox();
        if (!superScript || IsPrescriptDelimiter(*superScript) || superScript->NextInFlowSiblingBox())
        {
          return {};
        }
        reference.FirstPostScript = subScript;
        return reference;
      }
      case ScriptType::Multiscripts:
      {
        // This element accepts the following syntax:
        //
        // <mmultiscripts>
        //   base
        //   (subscript superscript)*
        //   [ <mprescripts/> (presubscript presuperscript)* ]
        // </mmultiscripts>
        //
        // https://www.w3.org/TR/MathML3/chapter3.html#presm.mmultiscripts
        //
        // The base cannot be an <mprescripts> element.
        if (IsPrescriptDelimiter(*base))
        {
          return {};
        }

        // We set the first postscript, unless (subscript superscript)* is empty.
        if (base->NextInFlowSiblingBox() && !IsPrescriptDelimiter(*base->NextInFlowSiblingBox()))
        {
          reference.FirstPostScript = base->NextInFlowSiblingBox();
        }

        // We browse the children in order to
        // 1) Set the first prescript, unless (presubscript presuperscript)* is empty.
        // 2) Ensure the validity of the element i.e.
        //   a) That the list of postscripts can be grouped into pairs of subscript/superscript.
        //   b) That the list of prescripts can be grouped into pairs of subscript/superscript.
        //   c) That there is at most one <mprescripts/>.
        bool numberOfScriptIsEven = true;
        for (auto *script = base->NextInFlowSiblingBox(); script; script = script->NextInFlowSiblingBox())
        {
          if (IsPrescriptDelimiter(*script))
          {
            // This is a <mprescripts/>. Let's check 2a) and 2c).
            if (!numberOfScriptIsEven || reference.FirstPreScript)
            {
              return {};
            }
            reference.FirstPreScript = script->NextInFlowSiblingBox(); // We do 1).
            reference.PrescriptDelimiter = script;
            continue;
          }
          numberOfScriptIsEven = !numberOfScriptIsEven;
        }
        if (!numberOfScriptIsEven)
        {
          return {}; // We verify 2b).
        }
        return reference;
      }
    }

    krys_debug_assert(false);
    return {};
  }

  LayoutUnit RenderMathMLScripts::SpaceAfterScript() const noexcept
  {
    auto primaryFont = Style().GetFontCascade().PrimaryFont();
    if (const auto *mathData = primaryFont->MathData())
    {
      return LayoutUnit(mathData->GetMathConstant(*primaryFont, MathConstant::SpaceAfterScript));
    }
    return LayoutUnit(Style().GetFontCascade().Size() / 5);
  }

  LayoutUnit RenderMathMLScripts::ItalicCorrection(const ReferenceChildren &reference) noexcept
  {
    if (auto *mathMLBlock = DynamicDowncast<RenderMathMLBlock>(*reference.Base))
    {
      if (auto *renderOperator = mathMLBlock->UnembellishedOperator())
      {
        return renderOperator->ItalicCorrection();
      }
    }
    return 0;
  }

  void RenderMathMLScripts::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    _minPreferredLogicalWidth = 0;
    _maxPreferredLogicalWidth = 0;

    auto possibleReference = ValidateAndGetReferenceChildren();
    if (!possibleReference)
    {
      RenderMathMLRow::ComputePreferredLogicalWidths();
      return;
    }
    auto &reference = *possibleReference;

    LayoutUnit baseItalicCorrection = std::min(reference.Base->MaxPreferredLogicalWidth()
                                                 + MarginIntrinsicLogicalWidthForChild(*reference.Base),
                                               ItalicCorrection(reference));
    LayoutUnit space = SpaceAfterScript();

    switch (GetScriptType())
    {
      case ScriptType::Sub:
      case ScriptType::Under:
        _maxPreferredLogicalWidth +=
          reference.Base->MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(*reference.Base);
        _maxPreferredLogicalWidth +=
          std::max(0_lu, reference.FirstPostScript->MaxPreferredLogicalWidth()
                           + MarginIntrinsicLogicalWidthForChild(*reference.FirstPostScript)
                           - baseItalicCorrection + space);
        break;
      case ScriptType::Super:
      case ScriptType::Over:
        _maxPreferredLogicalWidth +=
          reference.Base->MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(*reference.Base);
        _maxPreferredLogicalWidth +=
          std::max(0_lu, reference.FirstPostScript->MaxPreferredLogicalWidth()
                           + MarginIntrinsicLogicalWidthForChild(*reference.FirstPostScript) + space);
        break;
      case ScriptType::SubSup:
      case ScriptType::UnderOver:
      case ScriptType::Multiscripts:
      {
        auto *subScript = reference.FirstPreScript;
        while (subScript)
        {
          auto *supScript = subScript->NextInFlowSiblingBox();
          krys_debug_assert(supScript);
          LayoutUnit subSupPairWidth =
            std::max(subScript->MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(*subScript),
                     supScript->MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(*supScript));
          _maxPreferredLogicalWidth += subSupPairWidth + space;
          subScript = supScript->NextInFlowSiblingBox();
        }
        _maxPreferredLogicalWidth +=
          reference.Base->MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(*reference.Base);
        subScript = reference.FirstPostScript;
        while (subScript && subScript != reference.PrescriptDelimiter)
        {
          auto *supScript = subScript->NextInFlowSiblingBox();
          krys_debug_assert(supScript);
          LayoutUnit subSupPairWidth = std::max(
            std::max(0_lu, subScript->MaxPreferredLogicalWidth()
                             + MarginIntrinsicLogicalWidthForChild(*subScript) - baseItalicCorrection),
            supScript->MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(*supScript));
          _maxPreferredLogicalWidth += subSupPairWidth + space;
          subScript = supScript->NextInFlowSiblingBox();
        }
      }
    }

    _minPreferredLogicalWidth = _maxPreferredLogicalWidth;

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::CalculatePreferredLogicalWidth);
    ApplySizeToMathContent(LayoutPhase::CalculatePreferredLogicalWidth, sizes);

    AdjustPreferredLogicalWidthsForBorderAndPadding();

    ClearNeedsPreferredWidthsUpdate();
  }

  auto RenderMathMLScripts::GetVerticalParameters() const noexcept -> VerticalParameters
  {
    VerticalParameters parameters;
    auto primaryFont = Style().GetFontCascade().PrimaryFont();
    if (const auto *mathData = primaryFont->MathData())
    {
      auto constant = [&](MathConstant name)
      {
        return LayoutUnit(mathData->GetMathConstant(*primaryFont, name));
      };
      parameters.SubscriptShiftDown = constant(MathConstant::SubscriptShiftDown);
      parameters.SuperscriptShiftUp =
        constant(Style().GetMathShift() == MathShift::Compact ? MathConstant::SuperscriptShiftUpCramped
                                                              : MathConstant::SuperscriptShiftUp);
      parameters.SubscriptBaselineDropMin = constant(MathConstant::SubscriptBaselineDropMin);
      parameters.SuperScriptBaselineDropMax = constant(MathConstant::SuperscriptBaselineDropMax);
      parameters.SubSuperscriptGapMin = constant(MathConstant::SubSuperscriptGapMin);
      parameters.SuperscriptBottomMin = constant(MathConstant::SuperscriptBottomMin);
      parameters.SubscriptTopMax = constant(MathConstant::SubscriptTopMax);
      parameters.SuperscriptBottomMaxWithSubscript =
        constant(MathConstant::SuperscriptBottomMaxWithSubscript);
    }
    else
    {
      // Default heuristic values when you do not have a font.
      float xHeight = Style().MetricsOfPrimaryFont().XHeight().value_or(0);
      parameters.SubscriptShiftDown = LayoutUnit(xHeight / 3);
      parameters.SuperscriptShiftUp = LayoutUnit(xHeight);
      parameters.SubscriptBaselineDropMin = LayoutUnit(xHeight / 2);
      parameters.SuperScriptBaselineDropMax = LayoutUnit(xHeight / 2);
      parameters.SubSuperscriptGapMin = LayoutUnit(Style().GetFontCascade().Size() / 5);
      parameters.SuperscriptBottomMin = LayoutUnit(xHeight / 4);
      parameters.SubscriptTopMax = LayoutUnit(4 * xHeight / 5);
      parameters.SuperscriptBottomMaxWithSubscript = LayoutUnit(4 * xHeight / 5);
    }
    return parameters;
  }

  auto RenderMathMLScripts::GetVerticalMetrics(const ReferenceChildren &reference) const noexcept
    -> VerticalMetrics
  {
    VerticalParameters parameters = GetVerticalParameters();
    VerticalMetrics metrics = {0, 0, 0, 0};

    LayoutUnit baseAscent = AscentForChild(*reference.Base) + reference.Base->MarginBefore();
    LayoutUnit baseDescent =
      reference.Base->LogicalHeight() + reference.Base->MarginLogicalHeight() - baseAscent;
    auto scriptType = GetScriptType();
    if (scriptType == ScriptType::Sub || scriptType == ScriptType::SubSup
        || scriptType == ScriptType::Multiscripts || scriptType == ScriptType::Under
        || scriptType == ScriptType::UnderOver)
    {
      metrics.SubShift =
        std::max(parameters.SubscriptShiftDown, baseDescent + parameters.SubscriptBaselineDropMin);
    }
    if (scriptType == ScriptType::Super || scriptType == ScriptType::SubSup
        || scriptType == ScriptType::Multiscripts || scriptType == ScriptType::Over
        || scriptType == ScriptType::UnderOver)
    {
      metrics.SupShift =
        std::max(parameters.SuperscriptShiftUp, baseAscent - parameters.SuperScriptBaselineDropMax);
    }

    switch (scriptType)
    {
      case ScriptType::Sub:
      case ScriptType::Under:
      {
        LayoutUnit subAscent =
          AscentForChild(*reference.FirstPostScript) + reference.FirstPostScript->MarginBefore();
        LayoutUnit subDescent = reference.FirstPostScript->LogicalHeight()
                                + reference.FirstPostScript->MarginLogicalHeight() - subAscent;
        metrics.Descent = subDescent;
        metrics.SubShift = std::max(metrics.SubShift, subAscent - parameters.SubscriptTopMax);
      }
      break;
      case ScriptType::Super:
      case ScriptType::Over:
      {
        LayoutUnit supAscent =
          AscentForChild(*reference.FirstPostScript) + reference.FirstPostScript->MarginBefore();
        LayoutUnit supDescent = reference.FirstPostScript->LogicalHeight()
                                + reference.FirstPostScript->MarginLogicalHeight() - supAscent;
        metrics.Ascent = supAscent;
        metrics.SupShift = std::max(metrics.SupShift, parameters.SuperscriptBottomMin + supDescent);
      }
      break;
      case ScriptType::SubSup:
      case ScriptType::UnderOver:
      case ScriptType::Multiscripts:
      {
        // FIXME: We should move the code updating VerticalMetrics for each sub/sup pair in a helper
        // function. That way, SubSup/UnderOver can just make one call and the loop for Multiscripts
        // can be rewritten in a more readable.
        auto *subScript = reference.FirstPostScript ? reference.FirstPostScript : reference.FirstPreScript;
        while (subScript)
        {
          auto *supScript = subScript->NextInFlowSiblingBox();
          krys_debug_assert(supScript);
          LayoutUnit subAscent = AscentForChild(*subScript) + subScript->MarginBefore();
          LayoutUnit subDescent = subScript->LogicalHeight() + subScript->MarginLogicalHeight() - subAscent;
          LayoutUnit supAscent = AscentForChild(*supScript) + supScript->MarginBefore();
          LayoutUnit supDescent = supScript->LogicalHeight() + supScript->MarginLogicalHeight() - supAscent;
          metrics.Ascent = std::max(metrics.Ascent, supAscent);
          metrics.Descent = std::max(metrics.Descent, subDescent);
          LayoutUnit subScriptShift =
            std::max(parameters.SubscriptShiftDown, baseDescent + parameters.SubscriptBaselineDropMin);
          subScriptShift = std::max(subScriptShift, subAscent - parameters.SubscriptTopMax);
          LayoutUnit supScriptShift =
            std::max(parameters.SuperscriptShiftUp, baseAscent - parameters.SuperScriptBaselineDropMax);
          supScriptShift = std::max(supScriptShift, parameters.SuperscriptBottomMin + supDescent);

          LayoutUnit subSuperscriptGap = (subScriptShift - subAscent) + (supScriptShift - supDescent);
          if (subSuperscriptGap < parameters.SubSuperscriptGapMin)
          {
            // First, we try and push the superscript up.
            LayoutUnit delta = parameters.SuperscriptBottomMaxWithSubscript - (supScriptShift - supDescent);
            if (delta > 0)
            {
              delta = std::min(delta, parameters.SubSuperscriptGapMin - subSuperscriptGap);
              supScriptShift += delta;
              subSuperscriptGap += delta;
            }
            // If that is not enough, we push the subscript down.
            if (subSuperscriptGap < parameters.SubSuperscriptGapMin)
            {
              subScriptShift += parameters.SubSuperscriptGapMin - subSuperscriptGap;
            }
          }

          metrics.SubShift = std::max(metrics.SubShift, subScriptShift);
          metrics.SupShift = std::max(metrics.SupShift, supScriptShift);

          subScript = supScript->NextInFlowSiblingBox();
          if (subScript == reference.PrescriptDelimiter)
          {
            subScript = reference.FirstPreScript;
          }
        }
      }
    }

    return metrics;
  }

  void RenderMathMLScripts::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    auto possibleReference = ValidateAndGetReferenceChildren();
    if (!possibleReference)
    {
      RenderMathMLRow::LayoutBlock(relayoutChildren);
      return;
    }
    auto &reference = *possibleReference;

    LayoutFloatingChildren();

    RecomputeLogicalWidth();
    ComputeAndSetBlockDirectionMarginsOfChildren();

    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      child->LayoutIfNeeded();
    }

    LayoutUnit space = SpaceAfterScript();

    // We determine the minimal shift/size of each script and take the maximum of the values.
    VerticalMetrics metrics = GetVerticalMetrics(reference);

    LayoutUnit baseAscent = AscentForChild(*reference.Base) + reference.Base->MarginBefore();
    LayoutUnit baseDescent =
      reference.Base->LogicalHeight() + reference.Base->MarginLogicalHeight() - baseAscent;
    LayoutUnit baseItalicCorrection = std::min(
      reference.Base->LogicalWidth() + reference.Base->MarginLogicalWidth(), ItalicCorrection(reference));
    LayoutUnit horizontalOffset = 0;

    LayoutUnit ascent = std::max(baseAscent, metrics.Ascent + metrics.SupShift);
    LayoutUnit descent = std::max(baseDescent, metrics.Descent + metrics.SubShift);
    SetLogicalHeight(ascent + descent);

    switch (GetScriptType())
    {
      case ScriptType::Sub:
      case ScriptType::Under:
      {
        LayoutUnit baseWidth = reference.Base->LogicalWidth() + reference.Base->MarginLogicalWidth();
        LayoutUnit contentWidth = baseWidth
                                  + std::max(0_lu, reference.FirstPostScript->LogicalWidth()
                                                     + reference.FirstPostScript->MarginLogicalWidth()
                                                     - baseItalicCorrection + space);
        SetLogicalWidth(contentWidth);
        LayoutPoint baseLocation(
          MirrorIfNeeded(horizontalOffset + reference.Base->MarginStart(), *reference.Base),
          ascent - baseAscent + reference.Base->MarginBefore());
        reference.Base->SetLocation(baseLocation);
        horizontalOffset += baseWidth;
        LayoutUnit scriptAscent = AscentForChild(*reference.FirstPostScript);
        LayoutPoint scriptLocation(
          MirrorIfNeeded(horizontalOffset - baseItalicCorrection + reference.FirstPostScript->MarginStart(),
                         *reference.FirstPostScript),
          ascent + metrics.SubShift - scriptAscent);
        reference.FirstPostScript->SetLocation(scriptLocation);
      }
      break;
      case ScriptType::Super:
      case ScriptType::Over:
      {
        LayoutUnit baseWidth = reference.Base->LogicalWidth() + reference.Base->MarginLogicalWidth();
        LayoutUnit contentWidth =
          baseWidth
          + std::max(0_lu, reference.FirstPostScript->LogicalWidth()
                             + reference.FirstPostScript->MarginLogicalWidth() + space);
        SetLogicalWidth(contentWidth);
        LayoutPoint baseLocation(
          MirrorIfNeeded(horizontalOffset + reference.Base->MarginStart(), *reference.Base),
          ascent - baseAscent + reference.Base->MarginBefore());
        reference.Base->SetLocation(baseLocation);
        horizontalOffset += baseWidth;
        LayoutUnit scriptAscent = AscentForChild(*reference.FirstPostScript);
        LayoutPoint scriptLocation(MirrorIfNeeded(horizontalOffset + reference.FirstPostScript->MarginStart(),
                                                  *reference.FirstPostScript),
                                   ascent - metrics.SupShift - scriptAscent);
        reference.FirstPostScript->SetLocation(scriptLocation);
      }
      break;
      case ScriptType::SubSup:
      case ScriptType::UnderOver:
      case ScriptType::Multiscripts:
      {
        // Calculate the logical width.
        LayoutUnit logicalWidth = 0;
        auto *subScript = reference.FirstPreScript;
        while (subScript)
        {
          auto *supScript = subScript->NextInFlowSiblingBox();
          krys_debug_assert(supScript);
          LayoutUnit subSupPairWidth = std::max(subScript->LogicalWidth() + subScript->MarginLogicalWidth(),
                                                supScript->LogicalWidth() + supScript->MarginLogicalWidth());
          logicalWidth += subSupPairWidth + space;
          subScript = supScript->NextInFlowSiblingBox();
        }
        logicalWidth += reference.Base->LogicalWidth() + reference.Base->MarginLogicalWidth();
        subScript = reference.FirstPostScript;
        while (subScript && subScript != reference.PrescriptDelimiter)
        {
          auto *supScript = subScript->NextInFlowSiblingBox();
          krys_debug_assert(supScript);
          LayoutUnit subSupPairWidth =
            std::max(std::max(0_lu, subScript->LogicalWidth() + subScript->MarginLogicalWidth()
                                      - baseItalicCorrection),
                     supScript->LogicalWidth() + supScript->MarginLogicalWidth());
          logicalWidth += subSupPairWidth + space;
          subScript = supScript->NextInFlowSiblingBox();
        }
        SetLogicalWidth(logicalWidth);

        subScript = reference.FirstPreScript;
        while (subScript)
        {
          auto *supScript = subScript->NextInFlowSiblingBox();
          krys_debug_assert(supScript);
          LayoutUnit subSupPairWidth = std::max(subScript->LogicalWidth() + subScript->MarginLogicalWidth(),
                                                supScript->LogicalWidth() + supScript->MarginLogicalWidth());
          horizontalOffset += space + subSupPairWidth;
          LayoutUnit subAscent = AscentForChild(*subScript);
          LayoutPoint subScriptLocation(
            MirrorIfNeeded(horizontalOffset - subScript->MarginEnd() - subScript->LogicalWidth(), *subScript),
            ascent + metrics.SubShift - subAscent);
          subScript->SetLocation(subScriptLocation);
          LayoutUnit supAscent = AscentForChild(*supScript);
          LayoutPoint supScriptLocation(
            MirrorIfNeeded(horizontalOffset - supScript->MarginEnd() - supScript->LogicalWidth(), *supScript),
            ascent - metrics.SupShift - supAscent);
          supScript->SetLocation(supScriptLocation);
          subScript = supScript->NextInFlowSiblingBox();
        }
        LayoutPoint baseLocation(
          MirrorIfNeeded(horizontalOffset + reference.Base->MarginStart(), *reference.Base),
          ascent - baseAscent + reference.Base->MarginBefore());
        reference.Base->SetLocation(baseLocation);
        horizontalOffset += reference.Base->LogicalWidth() + reference.Base->MarginLogicalWidth();
        subScript = reference.FirstPostScript;
        while (subScript && subScript != reference.PrescriptDelimiter)
        {
          auto *supScript = subScript->NextInFlowSiblingBox();
          krys_debug_assert(supScript);
          LayoutUnit subAscent = AscentForChild(*subScript);
          LayoutPoint subScriptLocation(
            MirrorIfNeeded(horizontalOffset - baseItalicCorrection + subScript->MarginStart(), *subScript),
            ascent + metrics.SubShift - subAscent);
          subScript->SetLocation(subScriptLocation);
          LayoutUnit supAscent = AscentForChild(*supScript);
          LayoutPoint supScriptLocation(
            MirrorIfNeeded(horizontalOffset + supScript->MarginStart(), *supScript),
            ascent - metrics.SupShift - supAscent);
          supScript->SetLocation(supScriptLocation);

          LayoutUnit subSupPairWidth = std::max(subScript->LogicalWidth() + subScript->MarginLogicalWidth(),
                                                supScript->LogicalWidth() + supScript->MarginLogicalWidth());
          horizontalOffset += subSupPairWidth + space;
          subScript = supScript->NextInFlowSiblingBox();
        }
      }
    }

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::Layout);
    auto shift = ApplySizeToMathContent(LayoutPhase::Layout, sizes);
    ShiftInFlowChildren(shift, 0);

    AdjustLayoutForBorderAndPadding();

    LayoutOutOfFlowBoxes(relayoutChildren);
  }

  Maybe<LayoutUnit> RenderMathMLScripts::FirstLineBaseline() const noexcept
  {
    auto possibleReference = ValidateAndGetReferenceChildren();
    if (!possibleReference)
    {
      return RenderMathMLRow::FirstLineBaseline();
    }

    auto &base = *possibleReference->Base;
    // Subpixel inline layout is off, so the baseline is rounded.
    return LayoutUnit(std::round((base.MarginBefore() + base.LogicalTop() + AscentForChild(base)).ToFloat()));
  }
}
