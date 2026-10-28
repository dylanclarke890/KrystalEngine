#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/MathML/MathMLOperatorElement.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  using mathml::MathMLOperatorDictionary;

  namespace
  {
    constexpr char32 MinusSign = 0x2212;
  }

  RenderMathMLOperator::RenderMathMLOperator(mathml::MathMLOperatorElement &element,
                                             RenderStyle &&style) noexcept
      : RenderMathMLToken(Type::MathMLOperator, element, krys::move(style))
  {
    UpdateTokenContent();
  }

  RenderMathMLOperator::~RenderMathMLOperator() noexcept = default;

  mathml::MathMLOperatorElement &RenderMathMLOperator::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLOperatorElement &>(NodeForNonAnonymous());
  }

  char32 RenderMathMLOperator::TextContent() const noexcept
  {
    return GetElement().GetOperatorChar().Character;
  }

  bool RenderMathMLOperator::IsInvisibleOperator() const noexcept
  {
    // The following operators are invisible: U+2061 FUNCTION APPLICATION, U+2062 INVISIBLE TIMES, U+2063
    // INVISIBLE SEPARATOR, U+2064 INVISIBLE PLUS.
    char32 character = TextContent();
    return 0x2061 <= character && character <= 0x2064;
  }

  bool RenderMathMLOperator::HasOperatorFlag(MathMLOperatorDictionary::Flag flag) const noexcept
  {
    return GetElement().HasProperty(flag);
  }

  bool RenderMathMLOperator::IsLargeOperatorInDisplayStyle() const noexcept
  {
    return !HasOperatorFlag(MathMLOperatorDictionary::Stretchy)
           && HasOperatorFlag(MathMLOperatorDictionary::LargeOp)
           && Style().GetMathStyle() == MathStyle::Normal;
  }

  LayoutUnit RenderMathMLOperator::LeadingSpace() const noexcept
  {
    // FIXME: Negative leading spaces must be implemented (https://webkit.org/b/124830).
    LayoutUnit leadingSpace = ToUserUnits(GetElement().DefaultLeadingSpace(), Style(), 0);
    leadingSpace = ToUserUnits(GetElement().LeadingSpace(), Style(), leadingSpace);
    return std::max<LayoutUnit>(0, leadingSpace);
  }

  LayoutUnit RenderMathMLOperator::TrailingSpace() const noexcept
  {
    // FIXME: Negative trailing spaces must be implemented (https://webkit.org/b/124830).
    LayoutUnit trailingSpace = ToUserUnits(GetElement().DefaultTrailingSpace(), Style(), 0);
    trailingSpace = ToUserUnits(GetElement().TrailingSpace(), Style(), trailingSpace);
    return std::max<LayoutUnit>(0, trailingSpace);
  }

  LayoutUnit RenderMathMLOperator::MinSize() const noexcept
  {
    LayoutUnit minSize {Style().GetFontCascade().Size()}; // Default minsize is "1em".
    minSize = ToUserUnits(GetElement().MinSize(), Style(), minSize);
    return std::max<LayoutUnit>(0, minSize);
  }

  LayoutUnit RenderMathMLOperator::MaxSize() const noexcept
  {
    LayoutUnit maxSize {IntMaxForLayoutUnit}; // Default maxsize is ∞.
    maxSize = ToUserUnits(GetElement().MaxSize(), Style(), maxSize);
    return std::max<LayoutUnit>(0, maxSize);
  }

  bool RenderMathMLOperator::IsVertical() const noexcept
  {
    return GetElement().GetOperatorChar().IsVertical;
  }

  void RenderMathMLOperator::StretchTo(LayoutUnit heightAboveBaseline, LayoutUnit depthBelowBaseline) noexcept
  {
    krys_debug_assert(IsStretchy());
    krys_debug_assert(IsVertical());
    krys_debug_assert(!IsStretchWidthLocked());

    if (!IsVertical()
        || (heightAboveBaseline == _stretchHeightAboveBaseline
            && depthBelowBaseline == _stretchDepthBelowBaseline))
    {
      return;
    }

    _stretchHeightAboveBaseline = heightAboveBaseline;
    _stretchDepthBelowBaseline = depthBelowBaseline;

    if (HasOperatorFlag(MathMLOperatorDictionary::Symmetric))
    {
      // We make the operator stretch symmetrically above and below the axis.
      LayoutUnit axis = MathAxisHeight();
      LayoutUnit halfStretchSize =
        std::max(_stretchHeightAboveBaseline - axis, _stretchDepthBelowBaseline + axis);
      _stretchHeightAboveBaseline = halfStretchSize + axis;
      _stretchDepthBelowBaseline = halfStretchSize - axis;
    }
    // We try to honor the minsize/maxsize condition by increasing or decreasing both height and depth
    // proportionately. The MathML specification does not indicate what to do when maxsize < minsize, so we
    // follow Gecko and make minsize take precedence.
    LayoutUnit size = StretchSize();
    float aspect = 1.0;
    if (size > 0)
    {
      LayoutUnit minSizeValue = MinSize();
      if (size < minSizeValue)
      {
        aspect = minSizeValue.ToFloat() / size.ToFloat();
      }
      else
      {
        LayoutUnit maxSizeValue = MaxSize();
        if (maxSizeValue < size)
        {
          aspect = maxSizeValue.ToFloat() / size.ToFloat();
        }
      }
    }
    _stretchHeightAboveBaseline *= aspect;
    _stretchDepthBelowBaseline *= aspect;

    _mathOperator.StretchTo(Style(), _stretchHeightAboveBaseline + _stretchDepthBelowBaseline);

    SetLogicalHeight(_mathOperator.Ascent() + _mathOperator.Descent() + BorderAndPaddingLogicalHeight());
  }

  void RenderMathMLOperator::StretchTo(LayoutUnit width) noexcept
  {
    krys_debug_assert(IsStretchy());
    krys_debug_assert(!IsVertical());
    krys_debug_assert(!IsStretchWidthLocked());

    if (IsVertical() || _stretchWidth == width)
    {
      return;
    }

    _stretchWidth = width;
    _mathOperator.StretchTo(Style(), width);

    SetLogicalWidth(LeadingSpace() + width + TrailingSpace() + BorderAndPaddingLogicalWidth());
    SetLogicalHeight(_mathOperator.Ascent() + _mathOperator.Descent() + BorderAndPaddingLogicalHeight());
  }

  void RenderMathMLOperator::ResetStretchSize() noexcept
  {
    krys_debug_assert(!IsStretchWidthLocked());

    if (IsVertical())
    {
      _stretchHeightAboveBaseline = 0;
      _stretchDepthBelowBaseline = 0;
    }
    else
    {
      _stretchWidth = 0;
    }
  }

  void RenderMathMLOperator::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    LayoutUnit preferredWidth;

    if (!UseMathOperator())
    {
      // No need to include padding/border/margin here, RenderMathMLToken::ComputePreferredLogicalWidths takes
      // care of them.
      RenderMathMLToken::ComputePreferredLogicalWidths();
      preferredWidth = _maxPreferredLogicalWidth;
      if (IsInvisibleOperator())
      {
        // In some fonts, glyphs for invisible operators have nonzero width. Consequently, we subtract that
        // width here to avoid wide gaps.
        GlyphData data = Style().GetFontCascade().GlyphDataForCharacter(TextContent(), false);
        float glyphWidth = data.Font ? data.Font->WidthForGlyph(data.Glyph) : 0;
        preferredWidth -= std::min(LayoutUnit(glyphWidth), preferredWidth);
      }
    }
    else
    {
      preferredWidth = _mathOperator.MaxPreferredWidth() + BorderAndPaddingLogicalWidth();
    }

    // FIXME: The spacing should be added to the whole embellished operator (https://webkit.org/b/124831).
    // FIXME: The spacing should only be added inside (perhaps inferred) mrow
    // (http://www.w3.org/TR/MathML/chapter3.html#presm.opspacing).
    preferredWidth = LeadingSpace() + preferredWidth + TrailingSpace();

    _maxPreferredLogicalWidth = _minPreferredLogicalWidth = preferredWidth;

    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderMathMLOperator::LayoutBlock(RelayoutChildren relayoutChildren,
                                         LayoutUnit pageLogicalHeight) noexcept
  {
    krys_debug_assert(NeedsLayout());

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    LayoutFloatingChildren();

    LayoutUnit leadingSpaceValue = LeadingSpace();
    LayoutUnit trailingSpaceValue = TrailingSpace();

    if (UseMathOperator())
    {
      RecomputeLogicalWidth();
      for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
      {
        child->LayoutIfNeeded();
      }
      SetLogicalWidth(leadingSpaceValue + _mathOperator.Width() + trailingSpaceValue
                      + BorderAndPaddingLogicalWidth());
      SetLogicalHeight(_mathOperator.Ascent() + _mathOperator.Descent() + BorderAndPaddingLogicalHeight());

      LayoutOutOfFlowBoxes(relayoutChildren);
    }
    else
    {
      // We first do the normal layout without spacing.
      // No need to handle padding/border/margin here, RenderMathMLToken::LayoutBlock takes care of them.
      RecomputeLogicalWidth();
      LayoutUnit width = LogicalWidth();
      SetLogicalWidth(width - leadingSpaceValue - trailingSpaceValue);
      RenderMathMLToken::LayoutBlock(relayoutChildren, pageLogicalHeight);
      SetLogicalWidth(width);

      // We then move the children to take spacing into account.
      ShiftInFlowChildren(GetWritingMode().IsBidiLTR() ? leadingSpaceValue : -leadingSpaceValue, 0_lu);
    }
  }

  void RenderMathMLOperator::UpdateMathOperator() noexcept
  {
    krys_debug_assert(UseMathOperator());
    MathOperator::Type type;
    if (IsStretchy())
    {
      type = IsVertical() ? MathOperator::Type::VerticalOperator : MathOperator::Type::HorizontalOperator;
    }
    else if (TextContent() && IsLargeOperatorInDisplayStyle())
    {
      type = MathOperator::Type::DisplayOperator;
    }
    else
    {
      type = MathOperator::Type::NormalOperator;
    }

    _mathOperator.SetOperator(Style(), TextContent(), type);
  }

  void RenderMathMLOperator::UpdateTokenContent() noexcept
  {
    krys_debug_assert(!IsAnonymous());
    RenderMathMLToken::UpdateTokenContent();
    if (UseMathOperator())
    {
      UpdateMathOperator();
    }
  }

  void RenderMathMLOperator::UpdateFromElement() noexcept
  {
    UpdateTokenContent();
  }

  bool RenderMathMLOperator::UseMathOperator() const noexcept
  {
    // We use the MathOperator class to handle the following cases:
    // 1) Stretchy and large operators, since they require special painting.
    // 2) The minus sign, since it can be obtained from a hyphen in the DOM.
    return IsStretchy() || (TextContent() && IsLargeOperatorInDisplayStyle()) || TextContent() == MinusSign;
  }

  void RenderMathMLOperator::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderMathMLBlock::StyleDidChange(diff, oldStyle);
    _mathOperator.Reset(Style());
    ResetStretchSize();

    // MathML displaystyle can affect IsLargeOperatorInDisplayStyle()
    if (oldStyle && Style().GetMathStyle() != oldStyle->GetMathStyle() && !IsAnonymous())
    {
      UpdateTokenContent();
    }
  }

  LayoutUnit RenderMathMLOperator::VerticalStretchedOperatorShift() const noexcept
  {
    if (!IsVertical() || !StretchSize())
    {
      return 0;
    }

    return (_stretchDepthBelowBaseline - _stretchHeightAboveBaseline - _mathOperator.Descent()
            + _mathOperator.Ascent())
           / 2;
  }

  Maybe<LayoutUnit> RenderMathMLOperator::FirstLineBaseline() const noexcept
  {
    if (UseMathOperator())
    {
      auto baseline =
        LayoutUnit(std::round((_mathOperator.Ascent() - VerticalStretchedOperatorShift()).ToFloat()));
      return BorderAndPaddingBefore() + baseline;
    }
    return RenderMathMLToken::FirstLineBaseline();
  }

  void RenderMathMLOperator::Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept
  {
    RenderMathMLToken::Paint(info, paintOffset);
    if (!UseMathOperator())
    {
      return;
    }

    auto operatorTopLeft = paintOffset + Location();
    operatorTopLeft.Move((GetWritingMode().IsBidiLTR() ? LeadingSpace() : TrailingSpace()) + BorderLeft()
                           + PaddingLeft(),
                         BorderAndPaddingBefore());

    _mathOperator.Paint(Style(), info, operatorTopLeft, GetDocument().DeviceScaleFactor());
  }

  void RenderMathMLOperator::PaintChildren(PaintInfo &forSelf, const LayoutPoint &paintOffset,
                                           PaintInfo &forChild) noexcept
  {
    // We skip painting for invisible operators too to avoid some "missing character" glyph to appear if
    // appropriate math fonts are not available.
    if (UseMathOperator() || IsInvisibleOperator())
    {
      return;
    }
    RenderMathMLToken::PaintChildren(forSelf, paintOffset, forChild);
  }
}
