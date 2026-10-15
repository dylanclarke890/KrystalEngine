#include "Krystal.Booey/Rendering/Style/StyleRareInheritedData.hpp"

#include "Krystal.Booey/Rendering/Style/StyleAppleColorFilterData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo
{

  StyleRareInheritedData::StyleRareInheritedData() noexcept
      : UsedZoom(style::Evaluate<float>(style::ComputedStyle::InitialZoom())), DeviceScaleFactor(1.0f),
        TextFillColor(style::ComputedStyle::InitialTextFillColor()),
        TextEmphasisColor(style::ComputedStyle::InitialTextEmphasisColor()),
        VisitedLinkTextFillColor(style::ComputedStyle::InitialTextFillColor()),
        VisitedLinkTextEmphasisColor(style::ComputedStyle::InitialTextEmphasisColor()),
        CaretColor(style::Color::GetCurrentColor()), VisitedLinkCaretColor(style::Color::GetCurrentColor()),
        AccentColor(style::ComputedStyle::InitialAccentColor()),
        ScrollbarColor(style::ComputedStyle::InitialScrollbarColor()),
        TextEmphasisStyle(style::ComputedStyle::InitialTextEmphasisStyle()),
        Quotes(style::ComputedStyle::InitialQuotes()),
        StrokeColor(style::ComputedStyle::InitialStrokeColor()),
        VisitedLinkStrokeColor(style::Color::GetCurrentColor()),
        ColorScheme(style::ComputedStyle::InitialColorScheme()),
        CursorImages(style::ComputedStyle::InitialCursor().Images),
        ListStyleType(style::ComputedStyle::InitialListStyleType()),
        BlockEllipsis(style::ComputedStyle::InitialBlockEllipsis()),
        TextIndent(style::ComputedStyle::InitialTextIndent()),
        ListStyleImage(style::ComputedStyle::InitialListStyleImage()),
        DynamicRangeLimit(style::ComputedStyle::InitialDynamicRangeLimit()),
        TextShadow(style::ComputedStyle::InitialTextShadow()),
        HyphenateCharacter(style::ComputedStyle::InitialHyphenateCharacter()),
        CustomProperties(style::CustomPropertyData::Create()), EventListenerRegionTypes {},
        StrokeWidth(style::ComputedStyle::InitialStrokeWidth()),
        TextUnderlineOffset(style::ComputedStyle::InitialTextUnderlineOffset()),
        AppleColorFilter(StyleAppleColorFilterData::Create()),
        TabSize(style::ComputedStyle::InitialTabSize()),
        StrokeMiterLimit(style::ComputedStyle::InitialStrokeMiterLimit()),
        MathDepth(style::ComputedStyle::InitialMathDepth()),
        TextBoxEdge(style::ComputedStyle::InitialTextBoxEdge()),
        LineFitEdge(style::ComputedStyle::InitialLineFitEdge()),
        Widows(style::ComputedStyle::InitialWidows()), Orphans(style::ComputedStyle::InitialOrphans()),
        HyphenateLimitBefore(style::ComputedStyle::InitialHyphenateLimitBefore()),
        HyphenateLimitAfter(style::ComputedStyle::InitialHyphenateLimitAfter()),
        HyphenateLimitLines(style::ComputedStyle::InitialHyphenateLimitLines()),
        UsedTouchAction(style::ComputedStyle::InitialTouchAction()),
        TextSecurity(static_cast<unsigned>(style::ComputedStyle::InitialTextSecurity())),
        UserModify(static_cast<unsigned>(boo::UserModify::ReadOnly)),
        WordBreak(static_cast<unsigned>(style::ComputedStyle::InitialWordBreak())),
        OverflowWrap(static_cast<unsigned>(style::ComputedStyle::InitialOverflowWrap())),
        LineBreak(static_cast<unsigned>(boo::LineBreak::Auto)),
        UserSelect(static_cast<unsigned>(style::ComputedStyle::InitialUserSelect())),
        SpeakAs(static_cast<unsigned>(style::ComputedStyle::InitialSpeakAs().ToRaw())),
        Hyphens(static_cast<unsigned>(boo::Hyphens::Manual)),
        TextCombine(static_cast<unsigned>(style::ComputedStyle::InitialTextCombine())),
        TextEmphasisPosition(
          static_cast<unsigned>(style::ComputedStyle::InitialTextEmphasisPosition().ToRaw())),
        TextUnderlinePosition(
          static_cast<unsigned>(style::ComputedStyle::InitialTextUnderlinePosition().ToRaw())),
        LineBoxContain(static_cast<unsigned>(style::ComputedStyle::InitialLineBoxContain().ToRaw())),
        ImageOrientation(static_cast<unsigned>(style::ComputedStyle::InitialImageOrientation())),
        ImageRendering(static_cast<unsigned>(style::ComputedStyle::InitialImageRendering())),
        LineAlign(static_cast<unsigned>(style::ComputedStyle::InitialLineAlign())),
        TextAlignLast(static_cast<unsigned>(style::ComputedStyle::InitialTextAlignLast())),
        TextJustify(static_cast<unsigned>(style::ComputedStyle::InitialTextJustify())),
        TextDecorationSkipInk(static_cast<unsigned>(style::ComputedStyle::InitialTextDecorationSkipInk())),
        MathShift(static_cast<unsigned>(style::ComputedStyle::InitialMathShift())),
        MathStyle(static_cast<unsigned>(style::ComputedStyle::InitialMathStyle())),
        RubyPosition(static_cast<unsigned>(style::ComputedStyle::InitialRubyPosition())),
        RubyAlign(static_cast<unsigned>(style::ComputedStyle::InitialRubyAlign())),
        RubyOverhang(static_cast<unsigned>(style::ComputedStyle::InitialRubyOverhang())),
        HangingPunctuation(static_cast<unsigned>(style::ComputedStyle::InitialHangingPunctuation().ToRaw())),
        PaintOrder(static_cast<unsigned>(style::ComputedStyle::InitialPaintOrder().GetType())),
        CapStyle(static_cast<unsigned>(style::ComputedStyle::InitialCapStyle())),
        JoinStyle(static_cast<unsigned>(style::ComputedStyle::InitialJoinStyle())),
        HasExplicitlySetStrokeWidth(false), HasExplicitlySetStrokeColor(false), HasAutoCaretColor(true),
        HasVisitedLinkAutoCaretColor(true), EffectiveInert(false), EffectivelyTransparent(false),
        IsInSubtreeWithBlendMode(false), IsForceHidden(false),
        UsedContentVisibility(static_cast<unsigned>(ContentVisibility::Visible)), AutoRevealsWhenFound(false),
        InsideDefaultButton(false), InsideSubmitButton(false), EvaluationTimeZoomEnabled(false)
  {
  }

  inline StyleRareInheritedData::StyleRareInheritedData(const StyleRareInheritedData &o) noexcept
      : RefCounted<StyleRareInheritedData>(), UsedZoom(o.UsedZoom), DeviceScaleFactor(o.DeviceScaleFactor),
        TextFillColor(o.TextFillColor), TextEmphasisColor(o.TextEmphasisColor),
        VisitedLinkTextFillColor(o.VisitedLinkTextFillColor),
        VisitedLinkTextEmphasisColor(o.VisitedLinkTextEmphasisColor), CaretColor(o.CaretColor),
        VisitedLinkCaretColor(o.VisitedLinkCaretColor), AccentColor(o.AccentColor),
        ScrollbarColor(o.ScrollbarColor), TextEmphasisStyle(o.TextEmphasisStyle), Quotes(o.Quotes),
        StrokeColor(o.StrokeColor), VisitedLinkStrokeColor(o.VisitedLinkStrokeColor),
        ColorScheme(o.ColorScheme), CursorImages(o.CursorImages), ListStyleType(o.ListStyleType),
        BlockEllipsis(o.BlockEllipsis), TextIndent(o.TextIndent), ListStyleImage(o.ListStyleImage),
        DynamicRangeLimit(o.DynamicRangeLimit), TextShadow(o.TextShadow),
        HyphenateCharacter(o.HyphenateCharacter), CustomProperties(o.CustomProperties),
        EventListenerRegionTypes(o.EventListenerRegionTypes), StrokeWidth(o.StrokeWidth),
        TextUnderlineOffset(o.TextUnderlineOffset), AppleColorFilter(o.AppleColorFilter), TabSize(o.TabSize),
        StrokeMiterLimit(o.StrokeMiterLimit), MathDepth(o.MathDepth), TextBoxEdge(o.TextBoxEdge),
        LineFitEdge(o.LineFitEdge), Widows(o.Widows), Orphans(o.Orphans),
        HyphenateLimitBefore(o.HyphenateLimitBefore), HyphenateLimitAfter(o.HyphenateLimitAfter),
        HyphenateLimitLines(o.HyphenateLimitLines), UsedTouchAction(o.UsedTouchAction),
        TextSecurity(o.TextSecurity), UserModify(o.UserModify), WordBreak(o.WordBreak),
        OverflowWrap(o.OverflowWrap), LineBreak(o.LineBreak), UserSelect(o.UserSelect), SpeakAs(o.SpeakAs),
        Hyphens(o.Hyphens), TextCombine(o.TextCombine), TextEmphasisPosition(o.TextEmphasisPosition),
        TextUnderlinePosition(o.TextUnderlinePosition), LineBoxContain(o.LineBoxContain),
        ImageOrientation(o.ImageOrientation), ImageRendering(o.ImageRendering), LineAlign(o.LineAlign),
        TextAlignLast(o.TextAlignLast), TextJustify(o.TextJustify),
        TextDecorationSkipInk(o.TextDecorationSkipInk), MathShift(o.MathShift), MathStyle(o.MathStyle),
        RubyPosition(o.RubyPosition), RubyAlign(o.RubyAlign), RubyOverhang(o.RubyOverhang),
        HangingPunctuation(o.HangingPunctuation), PaintOrder(o.PaintOrder), CapStyle(o.CapStyle),
        JoinStyle(o.JoinStyle), HasExplicitlySetStrokeWidth(o.HasExplicitlySetStrokeWidth),
        HasExplicitlySetStrokeColor(o.HasExplicitlySetStrokeColor), HasAutoCaretColor(o.HasAutoCaretColor),
        HasVisitedLinkAutoCaretColor(o.HasVisitedLinkAutoCaretColor), EffectiveInert(o.EffectiveInert),
        EffectivelyTransparent(o.EffectivelyTransparent),
        IsInSubtreeWithBlendMode(o.IsInSubtreeWithBlendMode), IsForceHidden(o.IsForceHidden),
        UsedContentVisibility(o.UsedContentVisibility), AutoRevealsWhenFound(o.AutoRevealsWhenFound),
        InsideDefaultButton(o.InsideDefaultButton), InsideSubmitButton(o.InsideSubmitButton),
        EvaluationTimeZoomEnabled(o.EvaluationTimeZoomEnabled)
  {
    krys_debug_assert_msg(o == *this, "StyleRareInheritedData should be properly copied.");
  }

  Ref<StyleRareInheritedData> StyleRareInheritedData::Copy() const noexcept
  {
    return AdoptRef(*new StyleRareInheritedData(*this));
  }

  StyleRareInheritedData::~StyleRareInheritedData() noexcept = default;

  bool StyleRareInheritedData::operator==(const StyleRareInheritedData &o) const noexcept
  {
    return UsedZoom == o.UsedZoom && TextFillColor == o.TextFillColor
           && TextEmphasisColor == o.TextEmphasisColor
           && VisitedLinkTextFillColor == o.VisitedLinkTextFillColor
           && VisitedLinkTextEmphasisColor == o.VisitedLinkTextEmphasisColor && CaretColor == o.CaretColor
           && VisitedLinkCaretColor == o.VisitedLinkCaretColor && AccentColor == o.AccentColor
           && ScrollbarColor == o.ScrollbarColor && DynamicRangeLimit == o.DynamicRangeLimit
           && TextShadow == o.TextShadow && CursorImages == o.CursorImages
           && TextEmphasisStyle == o.TextEmphasisStyle && TextIndent == o.TextIndent
           && TextUnderlineOffset == o.TextUnderlineOffset && TextBoxEdge == o.TextBoxEdge
           && LineFitEdge == o.LineFitEdge && StrokeMiterLimit == o.StrokeMiterLimit && Widows == o.Widows
           && Orphans == o.Orphans && TextSecurity == o.TextSecurity && UserModify == o.UserModify
           && WordBreak == o.WordBreak && OverflowWrap == o.OverflowWrap && LineBreak == o.LineBreak
           && UserSelect == o.UserSelect && SpeakAs == o.SpeakAs && Hyphens == o.Hyphens
           && HyphenateLimitBefore == o.HyphenateLimitBefore && HyphenateLimitAfter == o.HyphenateLimitAfter
           && HyphenateLimitLines == o.HyphenateLimitLines && ColorScheme == o.ColorScheme
           && TextCombine == o.TextCombine && TextEmphasisPosition == o.TextEmphasisPosition
           && LineBoxContain == o.LineBoxContain && HyphenateCharacter == o.HyphenateCharacter
           && Quotes == o.Quotes && AppleColorFilter == o.AppleColorFilter && TabSize == o.TabSize
           && ImageOrientation == o.ImageOrientation && ImageRendering == o.ImageRendering
           && TextAlignLast == o.TextAlignLast && TextJustify == o.TextJustify
           && TextDecorationSkipInk == o.TextDecorationSkipInk
           && TextUnderlinePosition == o.TextUnderlinePosition && RubyPosition == o.RubyPosition
           && RubyAlign == o.RubyAlign && RubyOverhang == o.RubyOverhang && LineAlign == o.LineAlign
           && HangingPunctuation == o.HangingPunctuation && PaintOrder == o.PaintOrder
           && CapStyle == o.CapStyle && JoinStyle == o.JoinStyle
           && HasExplicitlySetStrokeWidth == o.HasExplicitlySetStrokeWidth
           && HasExplicitlySetStrokeColor == o.HasExplicitlySetStrokeColor && MathShift == o.MathShift
           && MathStyle == o.MathStyle && HasAutoCaretColor == o.HasAutoCaretColor
           && HasVisitedLinkAutoCaretColor == o.HasVisitedLinkAutoCaretColor
           && IsInSubtreeWithBlendMode == o.IsInSubtreeWithBlendMode && IsForceHidden == o.IsForceHidden
           && AutoRevealsWhenFound == o.AutoRevealsWhenFound && UsedTouchAction == o.UsedTouchAction
           && EventListenerRegionTypes == o.EventListenerRegionTypes && EffectiveInert == o.EffectiveInert
           && EffectivelyTransparent == o.EffectivelyTransparent
           && UsedContentVisibility == o.UsedContentVisibility && InsideDefaultButton == o.InsideDefaultButton
           && InsideSubmitButton == o.InsideSubmitButton && StrokeWidth == o.StrokeWidth
           && StrokeColor == o.StrokeColor && VisitedLinkStrokeColor == o.VisitedLinkStrokeColor
           && CustomProperties == o.CustomProperties && ListStyleImage == o.ListStyleImage
           && ListStyleType == o.ListStyleType && BlockEllipsis == o.BlockEllipsis
           && EvaluationTimeZoomEnabled == o.EvaluationTimeZoomEnabled
           && DeviceScaleFactor == o.DeviceScaleFactor && MathDepth == o.MathDepth;
  }

  bool StyleRareInheritedData::HasColorFilters() const noexcept
  {
    return !AppleColorFilter->AppleColorFilter.IsNone();
  }
}
