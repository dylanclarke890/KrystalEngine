/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/Rendering/Style/StyleRareInheritedData.hpp"

#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo
{

  StyleRareInheritedData::StyleRareInheritedData() noexcept
      : UsedZoom(style::Evaluate<float>(style::ComputedStyle::InitialZoom())), DeviceScaleFactor(1.0f),
        TextEmphasisColor(style::ComputedStyle::InitialTextEmphasisColor()),
        CaretColor(style::Color::GetCurrentColor()), AccentColor(style::ComputedStyle::InitialAccentColor()),
        ScrollbarColor(style::ComputedStyle::InitialScrollbarColor()),
        TextEmphasisStyle(style::ComputedStyle::InitialTextEmphasisStyle()),
        Quotes(style::ComputedStyle::InitialQuotes()),
        StrokeColor(style::ComputedStyle::InitialStrokeColor()),
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
        TabSize(style::ComputedStyle::InitialTabSize()),
        StrokeMiterLimit(style::ComputedStyle::InitialStrokeMiterLimit()),
        MathDepth(style::ComputedStyle::InitialMathDepth()),
        TextBoxEdge(style::ComputedStyle::InitialTextBoxEdge()),
        LineFitEdge(style::ComputedStyle::InitialLineFitEdge()),
        Widows(style::ComputedStyle::InitialWidows()), Orphans(style::ComputedStyle::InitialOrphans()),
        HyphenateLimitChars(style::ComputedStyle::InitialHyphenateLimitChars()),
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
        EffectiveInert(false), EffectivelyTransparent(false), IsInSubtreeWithBlendMode(false),
        IsForceHidden(false), UsedContentVisibility(static_cast<unsigned>(ContentVisibility::Visible)),
        AutoRevealsWhenFound(false), InsideDefaultButton(false), InsideSubmitButton(false),
        EvaluationTimeZoomEnabled(false)
  {
  }

  inline StyleRareInheritedData::StyleRareInheritedData(const StyleRareInheritedData &o) noexcept
      : RefCounted<StyleRareInheritedData>(), UsedZoom(o.UsedZoom), DeviceScaleFactor(o.DeviceScaleFactor),
        TextEmphasisColor(o.TextEmphasisColor), CaretColor(o.CaretColor), AccentColor(o.AccentColor),
        ScrollbarColor(o.ScrollbarColor), TextEmphasisStyle(o.TextEmphasisStyle), Quotes(o.Quotes),
        StrokeColor(o.StrokeColor), ColorScheme(o.ColorScheme), CursorImages(o.CursorImages),
        ListStyleType(o.ListStyleType), BlockEllipsis(o.BlockEllipsis), TextIndent(o.TextIndent),
        ListStyleImage(o.ListStyleImage), DynamicRangeLimit(o.DynamicRangeLimit), TextShadow(o.TextShadow),
        HyphenateCharacter(o.HyphenateCharacter), CustomProperties(o.CustomProperties),
        EventListenerRegionTypes(o.EventListenerRegionTypes), StrokeWidth(o.StrokeWidth),
        TextUnderlineOffset(o.TextUnderlineOffset), TabSize(o.TabSize), StrokeMiterLimit(o.StrokeMiterLimit),
        MathDepth(o.MathDepth), TextBoxEdge(o.TextBoxEdge), LineFitEdge(o.LineFitEdge), Widows(o.Widows),
        Orphans(o.Orphans), HyphenateLimitChars(o.HyphenateLimitChars),
        HyphenateLimitLines(o.HyphenateLimitLines), UsedTouchAction(o.UsedTouchAction),
        TextSecurity(o.TextSecurity), UserModify(o.UserModify), WordBreak(o.WordBreak),
        OverflowWrap(o.OverflowWrap), LineBreak(o.LineBreak), UserSelect(o.UserSelect), SpeakAs(o.SpeakAs),
        Hyphens(o.Hyphens), TextCombine(o.TextCombine), TextEmphasisPosition(o.TextEmphasisPosition),
        TextUnderlinePosition(o.TextUnderlinePosition), LineBoxContain(o.LineBoxContain),
        ImageOrientation(o.ImageOrientation), ImageRendering(o.ImageRendering),
        TextAlignLast(o.TextAlignLast), TextJustify(o.TextJustify),
        TextDecorationSkipInk(o.TextDecorationSkipInk), MathShift(o.MathShift), MathStyle(o.MathStyle),
        RubyPosition(o.RubyPosition), RubyAlign(o.RubyAlign), RubyOverhang(o.RubyOverhang),
        HangingPunctuation(o.HangingPunctuation), PaintOrder(o.PaintOrder), CapStyle(o.CapStyle),
        JoinStyle(o.JoinStyle), HasExplicitlySetStrokeWidth(o.HasExplicitlySetStrokeWidth),
        HasExplicitlySetStrokeColor(o.HasExplicitlySetStrokeColor), HasAutoCaretColor(o.HasAutoCaretColor),
        EffectiveInert(o.EffectiveInert), EffectivelyTransparent(o.EffectivelyTransparent),
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
    return UsedZoom == o.UsedZoom && TextEmphasisColor == o.TextEmphasisColor && CaretColor == o.CaretColor
           && AccentColor == o.AccentColor && ScrollbarColor == o.ScrollbarColor
           && DynamicRangeLimit == o.DynamicRangeLimit && TextShadow == o.TextShadow
           && CursorImages == o.CursorImages && TextEmphasisStyle == o.TextEmphasisStyle
           && TextIndent == o.TextIndent && TextUnderlineOffset == o.TextUnderlineOffset
           && TextBoxEdge == o.TextBoxEdge && LineFitEdge == o.LineFitEdge
           && StrokeMiterLimit == o.StrokeMiterLimit && Widows == o.Widows && Orphans == o.Orphans
           && TextSecurity == o.TextSecurity && UserModify == o.UserModify && WordBreak == o.WordBreak
           && OverflowWrap == o.OverflowWrap && LineBreak == o.LineBreak && UserSelect == o.UserSelect
           && SpeakAs == o.SpeakAs && Hyphens == o.Hyphens && HyphenateLimitChars == o.HyphenateLimitChars
           && HyphenateLimitLines == o.HyphenateLimitLines && ColorScheme == o.ColorScheme
           && TextCombine == o.TextCombine && TextEmphasisPosition == o.TextEmphasisPosition
           && LineBoxContain == o.LineBoxContain && HyphenateCharacter == o.HyphenateCharacter
           && Quotes == o.Quotes && TabSize == o.TabSize && ImageOrientation == o.ImageOrientation
           && ImageRendering == o.ImageRendering && TextAlignLast == o.TextAlignLast
           && TextJustify == o.TextJustify && TextDecorationSkipInk == o.TextDecorationSkipInk
           && TextUnderlinePosition == o.TextUnderlinePosition && RubyPosition == o.RubyPosition
           && RubyAlign == o.RubyAlign && RubyOverhang == o.RubyOverhang
           && HangingPunctuation == o.HangingPunctuation && PaintOrder == o.PaintOrder
           && CapStyle == o.CapStyle && JoinStyle == o.JoinStyle
           && HasExplicitlySetStrokeWidth == o.HasExplicitlySetStrokeWidth
           && HasExplicitlySetStrokeColor == o.HasExplicitlySetStrokeColor && MathShift == o.MathShift
           && MathStyle == o.MathStyle && HasAutoCaretColor == o.HasAutoCaretColor
           && IsInSubtreeWithBlendMode == o.IsInSubtreeWithBlendMode && IsForceHidden == o.IsForceHidden
           && AutoRevealsWhenFound == o.AutoRevealsWhenFound && UsedTouchAction == o.UsedTouchAction
           && EventListenerRegionTypes == o.EventListenerRegionTypes && EffectiveInert == o.EffectiveInert
           && EffectivelyTransparent == o.EffectivelyTransparent
           && UsedContentVisibility == o.UsedContentVisibility && InsideDefaultButton == o.InsideDefaultButton
           && InsideSubmitButton == o.InsideSubmitButton && StrokeWidth == o.StrokeWidth
           && StrokeColor == o.StrokeColor && CustomProperties == o.CustomProperties
           && ListStyleImage == o.ListStyleImage && ListStyleType == o.ListStyleType
           && BlockEllipsis == o.BlockEllipsis && EvaluationTimeZoomEnabled == o.EvaluationTimeZoomEnabled
           && DeviceScaleFactor == o.DeviceScaleFactor && MathDepth == o.MathDepth;
  }
}
