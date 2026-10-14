#include "Krystal.Booey/Style/PropertyAllowlist.hpp"

namespace krys::boo::style
{
  using css::PropertyId;

  PropertyAllowlist PropertyAllowlistForPseudoElement(PseudoElementType type) noexcept
  {
    if (type == PseudoElementType::Marker)
    {
      return PropertyAllowlist::Marker;
    }
    return PropertyAllowlist::None;
  }

  // https://drafts.csswg.org/css-lists-3/#marker-properties (Editor's Draft, 14 July 2021)
  // FIXME: this is outdated, see https://bugs.webkit.org/show_bug.cgi?id=218791.
  bool IsValidMarkerStyleProperty(PropertyId id) noexcept
  {
    switch (id)
    {
      case PropertyId::Color:
      case PropertyId::Content:
      case PropertyId::Custom:
      case PropertyId::Cursor:
      case PropertyId::Direction:
      case PropertyId::Font:
      case PropertyId::FontFamily:
      case PropertyId::FontFeatureSettings:
      case PropertyId::FontKerning:
      case PropertyId::FontSize:
      case PropertyId::FontSizeAdjust:
      case PropertyId::FontWidth:
      case PropertyId::FontStyle:
      case PropertyId::FontSynthesis:
      case PropertyId::FontSynthesisWeight:
      case PropertyId::FontSynthesisStyle:
      case PropertyId::FontSynthesisSmallCaps:
      case PropertyId::FontVariantAlternates:
      case PropertyId::FontVariantCaps:
      case PropertyId::FontVariantEastAsian:
      case PropertyId::FontVariantLigatures:
      case PropertyId::FontVariantNumeric:
      case PropertyId::FontVariantPosition:
      case PropertyId::FontWeight:
      case PropertyId::FontOpticalSizing:
      case PropertyId::FontVariationSettings:
      case PropertyId::Hyphens:
      case PropertyId::LetterSpacing:
      case PropertyId::LineBreak:
      case PropertyId::LineHeight:
      case PropertyId::ListStyle:
      case PropertyId::OverflowWrap:
      case PropertyId::TabSize:
      case PropertyId::TextCombineUpright:
      case PropertyId::TextDecorationSkipInk:
      case PropertyId::TextEmphasis:
      case PropertyId::TextEmphasisColor:
      case PropertyId::TextEmphasisPosition:
      case PropertyId::TextEmphasisStyle:
      case PropertyId::TextIndent:
      case PropertyId::TextOrientation:
      case PropertyId::TextShadow:
      case PropertyId::TextTransform:
      case PropertyId::TextWrapMode:
      case PropertyId::TextWrapStyle:
      case PropertyId::UnicodeBidi:
      case PropertyId::WordBreak:
      case PropertyId::WordSpacing:
      case PropertyId::WhiteSpace:
      case PropertyId::WhiteSpaceCollapse:
      case PropertyId::AnimationDuration:
      case PropertyId::AnimationTimingFunction:
      case PropertyId::AnimationDelay:
      case PropertyId::AnimationIterationCount:
      case PropertyId::AnimationDirection:
      case PropertyId::AnimationFillMode:
      case PropertyId::AnimationPlayState:
      case PropertyId::AnimationComposition:
      case PropertyId::AnimationName:
      case PropertyId::AnimationRangeEnd:
      case PropertyId::AnimationRangeStart:
      case PropertyId::AnimationTimeline:
      case PropertyId::TransitionBehavior:
      case PropertyId::TransitionDuration:
      case PropertyId::TransitionTimingFunction:
      case PropertyId::TransitionDelay:
      case PropertyId::TransitionProperty:       return true;
      default:                                   break;
    }
    return false;
  }

  bool IsValidCueStyleProperty(PropertyId id) noexcept
  {
    switch (id)
    {
      case PropertyId::Color:
      case PropertyId::Custom:
      case PropertyId::Font:
      case PropertyId::FontFamily:
      case PropertyId::FontSize:
      case PropertyId::FontStyle:
      case PropertyId::FontVariantCaps:
      case PropertyId::FontWeight:
      case PropertyId::LineHeight:
      case PropertyId::Opacity:
      case PropertyId::Outline:
      case PropertyId::OutlineColor:
      case PropertyId::OutlineOffset:
      case PropertyId::OutlineStyle:
      case PropertyId::OutlineWidth:
      case PropertyId::Visibility:
      case PropertyId::WhiteSpace:
      case PropertyId::WhiteSpaceCollapse:
      case PropertyId::TextCombineUpright:
      case PropertyId::TextDecorationLine:
      case PropertyId::TextShadow:
      case PropertyId::TextWrapMode:
      case PropertyId::TextWrapStyle:
      case PropertyId::BorderStyle:
      case PropertyId::PaintOrder:
      case PropertyId::StrokeLinejoin:
      case PropertyId::StrokeLinecap:
      case PropertyId::StrokeColor:
      case PropertyId::StrokeWidth:        return true;
      default:                             break;
    }
    return false;
  }

  bool IsValidCueSelectorStyleProperty(PropertyId id) noexcept
  {
    switch (id)
    {
      case PropertyId::Background:
      case PropertyId::BackgroundAttachment:
      case PropertyId::BackgroundClip:
      case PropertyId::BackgroundColor:
      case PropertyId::BackgroundImage:
      case PropertyId::BackgroundOrigin:
      case PropertyId::BackgroundPosition:
      case PropertyId::BackgroundPositionX:
      case PropertyId::BackgroundPositionY:
      case PropertyId::BackgroundRepeat:
      case PropertyId::BackgroundSize:
      case PropertyId::Color:
      case PropertyId::Custom:
      case PropertyId::Font:
      case PropertyId::FontFamily:
      case PropertyId::FontSize:
      case PropertyId::FontStyle:
      case PropertyId::FontVariantCaps:
      case PropertyId::FontWeight:
      case PropertyId::LineHeight:
      case PropertyId::Opacity:
      case PropertyId::Outline:
      case PropertyId::OutlineColor:
      case PropertyId::OutlineOffset:
      case PropertyId::OutlineStyle:
      case PropertyId::OutlineWidth:
      case PropertyId::Visibility:
      case PropertyId::WhiteSpace:
      case PropertyId::WhiteSpaceCollapse:
      case PropertyId::TextCombineUpright:
      case PropertyId::TextDecorationLine:
      case PropertyId::TextShadow:
      case PropertyId::TextWrapMode:
      case PropertyId::TextWrapStyle:
      case PropertyId::BorderStyle:
      case PropertyId::PaintOrder:
      case PropertyId::StrokeLinejoin:
      case PropertyId::StrokeLinecap:
      case PropertyId::StrokeColor:
      case PropertyId::StrokeWidth:          return true;
      default:                               break;
    }
    return false;
  }

  bool IsValidCueBackgroundStyleProperty(PropertyId id) noexcept
  {
    switch (id)
    {
      case PropertyId::Background:
      case PropertyId::BackgroundAttachment:
      case PropertyId::BackgroundClip:
      case PropertyId::BackgroundColor:
      case PropertyId::BackgroundImage:
      case PropertyId::BackgroundOrigin:
      case PropertyId::BackgroundPosition:
      case PropertyId::BackgroundPositionX:
      case PropertyId::BackgroundPositionY:
      case PropertyId::BackgroundRepeat:
      case PropertyId::BackgroundSize:       return true;
      default:                               break;
    }
    return false;
  }
}
