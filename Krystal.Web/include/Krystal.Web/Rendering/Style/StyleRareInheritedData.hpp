/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Memory/DataRef.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Web/Rendering/Style/StyleCustomPropertyData.hpp"
#include "Krystal.Web/Style/Values/Break/Orphans.hpp"
#include "Krystal.Web/Style/Values/Break/Widows.hpp"
#include "Krystal.Web/Style/Values/Color/Color.hpp"
#include "Krystal.Web/Style/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Web/Style/Values/Content/Quotes.hpp"
#include "Krystal.Web/Style/Values/FillStroke/StrokeMiterlimit.hpp"
#include "Krystal.Web/Style/Values/FillStroke/StrokeWidth.hpp"
#include "Krystal.Web/Style/Values/Images/ImageOrientation.hpp"
#include "Krystal.Web/Style/Values/Images/ImageOrNone.hpp"
#include "Krystal.Web/Style/Values/Inline/LineFitEdge.hpp"
#include "Krystal.Web/Style/Values/Inline/TextBoxEdge.hpp"
#include "Krystal.Web/Style/Values/Lists/ListStyleType.hpp"
#include "Krystal.Web/Style/Values/Math/MathDepth.hpp"
#include "Krystal.Web/Style/Values/NonStandard/WebKitLineBoxContain.hpp"
#include "Krystal.Web/Style/Values/Overflow/BlockEllipsis.hpp"
#include "Krystal.Web/Style/Values/PointerEvents/TouchAction.hpp"
#include "Krystal.Web/Style/Values/Scrollbars/ScrollbarColor.hpp"
#include "Krystal.Web/Style/Values/Speech/SpeakAs.hpp"
#include "Krystal.Web/Style/Values/SVG/SVGPaintOrder.hpp"
#include "Krystal.Web/Style/Values/Text/HangingPunctuation.hpp"
#include "Krystal.Web/Style/Values/Text/HyphenateCharacter.hpp"
#include "Krystal.Web/Style/Values/Text/HyphenateLimitChars.hpp"
#include "Krystal.Web/Style/Values/Text/HyphenateLimitLines.hpp"
#include "Krystal.Web/Style/Values/Text/TabSize.hpp"
#include "Krystal.Web/Style/Values/Text/TextAlignLast.hpp"
#include "Krystal.Web/Style/Values/Text/TextIndent.hpp"
#include "Krystal.Web/Style/Values/TextDecoration/TextEmphasisPosition.hpp"
#include "Krystal.Web/Style/Values/TextDecoration/TextEmphasisStyle.hpp"
#include "Krystal.Web/Style/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Web/Style/Values/TextDecoration/TextUnderlineOffset.hpp"
#include "Krystal.Web/Style/Values/TextDecoration/TextUnderlinePosition.hpp"
#include "Krystal.Web/Style/Values/UI/AccentColor.hpp"
#include "Krystal.Web/Style/Values/UI/Cursor.hpp"

#include "Krystal.Web/Style/Values/ColorAdjust/ColorScheme.hpp"

namespace krys::web
{
  class StyleImage;

  // This struct is for rarely used inherited CSS3, CSS2 and non-standard properties.
  // By grouping them together, we save space, and only allocate this object when someone
  // actually uses one of these properties.
  class StyleRareInheritedData : public RefCounted<StyleRareInheritedData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleRareInheritedData> Create() noexcept
    {
      return AdoptRef(*new StyleRareInheritedData);
    }

    KRYS_NODISCARD Ref<StyleRareInheritedData> Copy() const noexcept;

    ~StyleRareInheritedData() noexcept;

    bool operator==(const StyleRareInheritedData &) const noexcept;

    float UsedZoom;
    float DeviceScaleFactor {1.0f};

    style::Color TextEmphasisColor;
    style::Color CaretColor;

    style::AccentColor AccentColor;

    style::ScrollbarColor ScrollbarColor;

    style::TextEmphasisStyle TextEmphasisStyle;

    style::Quotes Quotes;

    style::Color StrokeColor;

    style::ColorScheme ColorScheme;

    style::Cursor::ImagesType CursorImages;

    style::ListStyleType ListStyleType;
    style::BlockEllipsis BlockEllipsis;

    style::TextIndent TextIndent;

    style::ImageOrNone ListStyleImage;
    style::DynamicRangeLimit DynamicRangeLimit;
    style::TextShadows TextShadow;
    style::HyphenateCharacter HyphenateCharacter;
    DataRef<style::CustomPropertyData> CustomProperties;
    EnumSet<EventListenerRegionType> EventListenerRegionTypes;
    style::StrokeWidth StrokeWidth;
    style::TextUnderlineOffset TextUnderlineOffset;
    style::TabSize TabSize;

    style::StrokeMiterlimit StrokeMiterLimit;

    style::MathDepth MathDepth;

    style::TextBoxEdge TextBoxEdge;
    style::LineFitEdge LineFitEdge;

    style::Widows Widows;
    style::Orphans Orphans;
    style::HyphenateLimitChars HyphenateLimitChars;
    style::HyphenateLimitLines HyphenateLimitLines;

    style::TouchAction UsedTouchAction;

    unsigned TextSecurity : 2;
    unsigned UserModify : 2 {static_cast<unsigned>(web::UserModify::ReadOnly)};
    unsigned WordBreak : 3;
    unsigned OverflowWrap : 2;
    unsigned LineBreak : 3 {static_cast<unsigned>(web::LineBreak::Auto)};
    unsigned UserSelect : 2;
    unsigned SpeakAs : 4;
    unsigned Hyphens : 2 {static_cast<unsigned>(web::Hyphens::Manual)};
    unsigned TextCombine : 1;
    unsigned TextEmphasisPosition : 4;
    unsigned TextUnderlinePosition : 4;
    unsigned LineBoxContain : 7;
    unsigned ImageOrientation : 1;
    unsigned ImageRendering : 3;
    unsigned TextAlignLast : 3;
    unsigned TextJustify : 2;
    unsigned TextDecorationSkipInk : 2;
    unsigned MathShift : 1;
    unsigned MathStyle : 1;
    unsigned RubyPosition : 2;
    unsigned RubyAlign : 2;
    unsigned RubyOverhang : 1;
    unsigned HangingPunctuation : 4;
    unsigned PaintOrder : 3;
    unsigned CapStyle : 2;
    unsigned JoinStyle : 2;
    unsigned HasExplicitlySetStrokeWidth : 1 {false};
    unsigned HasExplicitlySetStrokeColor : 1 {false};
    unsigned HasAutoCaretColor : 1 {true};
    unsigned EffectiveInert : 1 {false};
    unsigned EffectivelyTransparent : 1 {false};
    unsigned IsInSubtreeWithBlendMode : 1 {false};
    unsigned IsForceHidden : 1 {false};
    unsigned UsedContentVisibility : 2 {static_cast<unsigned>(ContentVisibility::Visible)};
    unsigned AutoRevealsWhenFound : 1 {false};
    unsigned InsideDefaultButton : 1 {false};
    unsigned InsideSubmitButton : 1 {false};
    unsigned EvaluationTimeZoomEnabled : 1 {false};

  private:
    StyleRareInheritedData() noexcept;

    StyleRareInheritedData(const StyleRareInheritedData &) noexcept;
  };
}
