#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCustomPropertyData.hpp"
#include "Krystal.Booey/Style/Values/Break/Orphans.hpp"
#include "Krystal.Booey/Style/Values/Break/Widows.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/Style/Values/Content/Quotes.hpp"
#include "Krystal.Booey/Style/Values/FillStroke/StrokeMiterlimit.hpp"
#include "Krystal.Booey/Style/Values/FillStroke/StrokeWidth.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrientation.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrNone.hpp"
#include "Krystal.Booey/Style/Values/Inline/LineFitEdge.hpp"
#include "Krystal.Booey/Style/Values/Inline/TextBoxEdge.hpp"
#include "Krystal.Booey/Style/Values/Lists/ListStyleType.hpp"
#include "Krystal.Booey/Style/Values/Math/MathDepth.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLineBoxContain.hpp"
#include "Krystal.Booey/Style/Values/Overflow/BlockEllipsis.hpp"
#include "Krystal.Booey/Style/Values/PointerEvents/TouchAction.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarColor.hpp"
#include "Krystal.Booey/Style/Values/Speech/SpeakAs.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGPaintOrder.hpp"
#include "Krystal.Booey/Style/Values/Text/HangingPunctuation.hpp"
#include "Krystal.Booey/Style/Values/Text/HyphenateCharacter.hpp"
#include "Krystal.Booey/Style/Values/Text/HyphenateLimitEdge.hpp"
#include "Krystal.Booey/Style/Values/Text/HyphenateLimitLines.hpp"
#include "Krystal.Booey/Style/Values/Text/TabSize.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlignLast.hpp"
#include "Krystal.Booey/Style/Values/Text/TextIndent.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextEmphasisPosition.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextEmphasisStyle.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlineOffset.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlinePosition.hpp"
#include "Krystal.Booey/Style/Values/UI/AccentColor.hpp"
#include "Krystal.Booey/Style/Values/UI/Cursor.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/DataRef.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"

#include "Krystal.Booey/Style/Values/ColorAdjust/ColorScheme.hpp"

namespace krys::boo
{
  class StyleAppleColorFilterData;
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

    KRYS_NODISCARD bool HasColorFilters() const noexcept;

    float UsedZoom;
    float DeviceScaleFactor {1.0f};

    style::Color TextFillColor;
    style::Color TextEmphasisColor;
    style::Color VisitedLinkTextFillColor;
    style::Color VisitedLinkTextEmphasisColor;
    style::Color CaretColor;
    style::Color VisitedLinkCaretColor;

    style::AccentColor AccentColor;

    style::ScrollbarColor ScrollbarColor;

    style::TextEmphasisStyle TextEmphasisStyle;

    style::Quotes Quotes;

    style::Color StrokeColor;
    style::Color VisitedLinkStrokeColor;

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
    DataRef<StyleAppleColorFilterData> AppleColorFilter;
    style::TabSize TabSize;

    style::StrokeMiterlimit StrokeMiterLimit;

    style::MathDepth MathDepth;

    style::TextBoxEdge TextBoxEdge;
    style::LineFitEdge LineFitEdge;

    style::Widows Widows;
    style::Orphans Orphans;
    style::HyphenateLimitEdge HyphenateLimitBefore;
    style::HyphenateLimitEdge HyphenateLimitAfter;
    style::HyphenateLimitLines HyphenateLimitLines;

    style::TouchAction UsedTouchAction;

    unsigned TextSecurity : 2;
    unsigned UserModify : 2;
    unsigned WordBreak : 3;
    unsigned OverflowWrap : 2;
    unsigned LineBreak : 3;
    unsigned UserSelect : 2;
    unsigned SpeakAs : 4;
    unsigned Hyphens : 2;
    unsigned TextCombine : 1;
    unsigned TextEmphasisPosition : 4;
    unsigned TextUnderlinePosition : 4;
    unsigned LineBoxContain : 7;
    unsigned ImageOrientation : 1;
    unsigned ImageRendering : 3;
    unsigned LineAlign : 1;
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
    unsigned HasExplicitlySetStrokeWidth : 1;
    unsigned HasExplicitlySetStrokeColor : 1;
    unsigned HasAutoCaretColor : 1;
    unsigned HasVisitedLinkAutoCaretColor : 1;
    unsigned EffectiveInert : 1;
    unsigned EffectivelyTransparent : 1;
    unsigned IsInSubtreeWithBlendMode : 1;
    unsigned IsForceHidden : 1;
    unsigned UsedContentVisibility : 2;
    unsigned AutoRevealsWhenFound : 1;
    unsigned InsideDefaultButton : 1;
    unsigned InsideSubmitButton : 1;
    unsigned EvaluationTimeZoomEnabled : 1;

  private:
    StyleRareInheritedData() noexcept;
    StyleRareInheritedData(const StyleRareInheritedData &) noexcept;
  };
}
