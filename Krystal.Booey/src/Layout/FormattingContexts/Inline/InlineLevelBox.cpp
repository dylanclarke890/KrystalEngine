#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLevelBox.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Inline/VerticalAlign.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::layout
{
  namespace
  {
    template <typename PreferredLineHeightFunctor>
    KRYS_NODISCARD InlineLevelBox::VerticalAlignment
      ToInlineBoxLevelVerticalAlign(const style::VerticalAlign &verticalAlign,
                                    PreferredLineHeightFunctor &&preferredLineHeightFunctor) noexcept
    {
      return verticalAlign.SwitchOn(
        [&](const style::VerticalAlign::Length &length) -> InlineLevelBox::VerticalAlignment
        {
          return style::Evaluate<InlineLayoutUnit>(
            length, std::forward<PreferredLineHeightFunctor>(preferredLineHeightFunctor),
            style::ZoomNeeded {});
        },
        [](const auto &keyword) -> InlineLevelBox::VerticalAlignment { return keyword; });
    }
  }

  InlineLevelBox::InlineLevelBox(const Box &layoutBox, const RenderStyle &style, InlineLayoutUnit logicalLeft,
                                 InlineLayoutSize logicalSize, Type type,
                                 EnumSet<PositionWithinLayoutBox> positionWithinLayoutBox) noexcept
      : _layoutBox(&layoutBox), _logicalRect({}, logicalLeft, logicalSize.Width(), logicalSize.Height()),
        // Normally we set inline box's has-content state as we come across child content, but ruby
        // annotations are not visible to inline layout.
        _hasContent(layoutBox.IsRubyBase() && layoutBox.AssociatedRubyAnnotationBox()),
        _isFirstWithinLayoutBox(positionWithinLayoutBox.Contains(PositionWithinLayoutBox::First)),
        _isLastWithinLayoutBox(positionWithinLayoutBox.Contains(PositionWithinLayoutBox::Last)), _type(type),
        _style({&style.GetFontCascade().MetricsOfPrimaryFont(), &style.GetLineHeight(),
                style.GetTextBoxTrim(), style.GetTextBoxEdge(), style.GetLineFitEdge(),
                style.UsedZoomForLength(), style.LineBoxContain(),
                InlineLayoutUnit(style.GetFontCascade().GetFontDescription().ComputedSize()),
                css::keywords::Baseline {}})
  {
    _style.VerticalAlignmentValue =
      ToInlineBoxLevelVerticalAlign(style.GetVerticalAlign(), [this] { return PreferredLineHeight(); });
  }

  InlineLevelBox InlineLevelBox::CreateAtomicInlineBox(const Box &layoutBox, const RenderStyle &style,
                                                       InlineLayoutUnit logicalLeft,
                                                       InlineLayoutUnit logicalWidth) noexcept
  {
    return {layoutBox, style, logicalLeft, {logicalWidth, {}}, Type::AtomicInlineBox};
  }

  InlineLevelBox InlineLevelBox::CreateGenericInlineLevelBox(const Box &layoutBox, const RenderStyle &style,
                                                             InlineLayoutUnit logicalLeft) noexcept
  {
    return {layoutBox, style, logicalLeft, {}, Type::GenericInlineLevelBox};
  }

  InlineLevelBox InlineLevelBox::CreateInlineBox(const Box &layoutBox, const RenderStyle &style,
                                                 InlineLayoutUnit logicalLeft, InlineLayoutUnit logicalWidth,
                                                 LineSpanningInlineBox isLineSpanning) noexcept
  {
    return {layoutBox,
            style,
            logicalLeft,
            {logicalWidth, {}},
            isLineSpanning == LineSpanningInlineBox::Yes ? Type::LineSpanningInlineBox : Type::InlineBox,
            {}};
  }

  InlineLevelBox InlineLevelBox::CreateLineBreakBox(const Box &layoutBox, const RenderStyle &style,
                                                    InlineLayoutUnit logicalLeft) noexcept
  {
    return {layoutBox, style, logicalLeft, {}, Type::LineBreakBox};
  }

  InlineLevelBox InlineLevelBox::CreateRootInlineBox(const Box &layoutBox, const RenderStyle &style,
                                                     InlineLayoutUnit logicalLeft,
                                                     InlineLayoutUnit logicalWidth) noexcept
  {
    return {layoutBox, style, logicalLeft, {logicalWidth, {}}, Type::RootInlineBox, {}};
  }

  InlineLayoutUnit InlineLevelBox::PreferredLineHeight() const noexcept
  {
    if (IsPreferredLineHeightFontMetricsBased())
    {
      return PrimarymetricsOfPrimaryFont().LineSpacing();
    }

    return krys::SwitchOn(
      *_style.LineHeight, [&](const css::keywords::Normal &) -> InlineLayoutUnit { return 0; },
      [&](const style::LineHeight::Fixed &fixed) -> InlineLayoutUnit
      { return style::Evaluate<InlineLayoutUnit>(fixed, _style.ZoomFactor); },
      [&](const style::LineHeight::Percentage &percentage) -> InlineLayoutUnit
      { return style::Evaluate<LayoutUnit>(percentage, LayoutUnit {FontSize()}).ToFloat(); },
      [&](const style::LineHeight::Calc &calc) -> InlineLayoutUnit
      { return style::Evaluate<LayoutUnit>(calc, LayoutUnit {FontSize()}, _style.ZoomFactor).ToFloat(); });
  }

  bool InlineLevelBox::MayStretchLineBox() const noexcept
  {
    using Value = style::WebkitLineBoxContainValue;
    auto &lineBoxContain = _style.LineBoxContain;
    if (IsRootInlineBox())
    {
      return lineBoxContain.ContainsAny({Value::Block, Value::Inline})
             || (HasContent()
                 && lineBoxContain.ContainsAny({Value::InitialLetter, Value::Font, Value::Glyphs}));
    }

    if (IsAtomicInlineBox())
    {
      return lineBoxContain.Contains(Value::Replaced);
    }

    if (IsInlineBox())
    {
      // Either the inline box itself is included or its text content through Glyph and Font.
      return lineBoxContain.ContainsAny({Value::Inline, Value::InlineBox})
             || (HasContent() && lineBoxContain.ContainsAny({Value::Font, Value::Glyphs}));
    }

    if (IsLineBreakBox())
    {
      return lineBoxContain.ContainsAny({Value::Inline, Value::InlineBox})
             || (HasContent() && lineBoxContain.ContainsAny({Value::Font, Value::Glyphs}));
    }

    return true;
  }
}
