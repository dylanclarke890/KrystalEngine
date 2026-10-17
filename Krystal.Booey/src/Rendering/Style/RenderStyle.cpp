#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"
#include "Krystal.Booey/Graphics/Transforms/ScaleTransformOperation.hpp"
#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleBase+ConstructionInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Rendering/Style/SVGRenderStyle.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/ScaleTransformFunction.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>

namespace krys::boo
{
  RenderStyle::RenderStyle(RenderStyle &&) noexcept = default;
  RenderStyle &RenderStyle::operator=(RenderStyle &&) noexcept = default;

  RenderStyle::RenderStyle(CreateDefaultStyleTag tag) noexcept : RenderStyleProperties {tag}
  {
  }

  RenderStyle::RenderStyle(const RenderStyle &other, CloneTag tag) noexcept
      : RenderStyleProperties {other, tag}
  {
  }

  RenderStyle::RenderStyle(RenderStyle &a, RenderStyle &&b) noexcept
      : RenderStyleProperties {a, krys::move(b)}
  {
  }

  RenderStyle &RenderStyle::DefaultStyleSingleton() noexcept
  {
    static NeverDestroyed<RenderStyle> style {CreateDefaultStyle};
    return style.get();
  }

  RenderStyle RenderStyle::Create() noexcept
  {
    return Clone(DefaultStyleSingleton());
  }

  UniquePtr<RenderStyle> RenderStyle::CreatePtr() noexcept
  {
    return ClonePtr(DefaultStyleSingleton());
  }

  UniquePtr<RenderStyle>
    RenderStyle::CreatePtrWithRegisteredInitialValues(const style::CustomPropertyRegistry &registry) noexcept
  {
    return ClonePtr(registry.InitialValuePrototypeStyle());
  }

  RenderStyle RenderStyle::Clone(const RenderStyle &style) noexcept
  {
    return RenderStyle(style, CloneTag::Clone);
  }

  RenderStyle RenderStyle::CloneIncludingPseudoElements(const RenderStyle &style) noexcept
  {
    auto newStyle = RenderStyle(style, CloneTag::Clone);
    newStyle.CopyPseudoElementsFrom(style);
    return newStyle;
  }

  UniquePtr<RenderStyle> RenderStyle::ClonePtr(const RenderStyle &style) noexcept
  {
    return CreateUnique<RenderStyle>(style, CloneTag::Clone);
  }

  RenderStyle RenderStyle::CreateAnonymousStyleWithDisplay(const RenderStyle &parentStyle,
                                                           DisplayType display) noexcept
  {
    auto newStyle = Create();
    newStyle.InheritFrom(parentStyle);
    newStyle.InheritUnicodeBidiFrom(parentStyle);
    newStyle.SetDisplay(display);
    return newStyle;
  }

  RenderStyle RenderStyle::CreateStyleInheritingFromPseudoStyle(const RenderStyle &pseudoStyle) noexcept
  {
    krys_debug_assert(pseudoStyle.GetPseudoElementType() == PseudoElementType::Before
                      || pseudoStyle.GetPseudoElementType() == PseudoElementType::After);

    auto style = Create();
    style.InheritFrom(pseudoStyle);
    return style;
  }

  RenderStyle RenderStyle::Replace(RenderStyle &&newStyle) noexcept
  {
    return RenderStyle {*this, krys::move(newStyle)};
  }

  void RenderStyle::CopyPseudoElementsFrom(const RenderStyle &other) noexcept
  {
    if (!other._computedStyle._cachedPseudoStyles)
    {
      return;
    }

    for (auto &[key, pseudoElementStyle] : other._computedStyle._cachedPseudoStyles->Styles)
    {
      if (!pseudoElementStyle)
      {
        krys_debug_assert_msg(false, "Unreachable");
        continue;
      }
      AddCachedPseudoStyle(CreateUnique<RenderStyle>(CloneIncludingPseudoElements(*pseudoElementStyle)));
    }
  }

  // MARK: - Specific style change queries

  bool RenderStyle::ScrollAnchoringSuppressionStyleDidChange(const RenderStyle *other) const noexcept
  {
    // https://drafts.csswg.org/css-scroll-anchoring/#suppression-triggers
    // Determine if there are any style changes that should result in an scroll anchoring suppression
    if (!other)
    {
      return false;
    }

    if (_computedStyle._nonInheritedData->BoxData.Ptr()
        != other->_computedStyle._nonInheritedData->BoxData.Ptr())
    {
      if (_computedStyle._nonInheritedData->BoxData->Width
            != other->_computedStyle._nonInheritedData->BoxData->Width
          || _computedStyle._nonInheritedData->BoxData->MinWidth
               != other->_computedStyle._nonInheritedData->BoxData->MinWidth
          || _computedStyle._nonInheritedData->BoxData->MaxWidth
               != other->_computedStyle._nonInheritedData->BoxData->MaxWidth
          || _computedStyle._nonInheritedData->BoxData->Height
               != other->_computedStyle._nonInheritedData->BoxData->Height
          || _computedStyle._nonInheritedData->BoxData->MinHeight
               != other->_computedStyle._nonInheritedData->BoxData->MinHeight
          || _computedStyle._nonInheritedData->BoxData->MaxHeight
               != other->_computedStyle._nonInheritedData->BoxData->MaxHeight)
      {
        return true;
      }
    }

    if (GetOverflowAnchor() != other->GetOverflowAnchor() && GetOverflowAnchor() == OverflowAnchor::None)
    {
      return true;
    }

    if (GetPosition() != other->GetPosition())
    {
      return true;
    }

    if (_computedStyle._nonInheritedData->SurroundData.Ptr()
        && other->_computedStyle._nonInheritedData->SurroundData.Ptr()
        && _computedStyle._nonInheritedData->SurroundData
             != other->_computedStyle._nonInheritedData->SurroundData)
    {
      if (_computedStyle._nonInheritedData->SurroundData->Margin
          != other->_computedStyle._nonInheritedData->SurroundData->Margin)
      {
        return true;
      }

      if (_computedStyle._nonInheritedData->SurroundData->Padding
          != other->_computedStyle._nonInheritedData->SurroundData->Padding)
      {
        return true;
      }
    }

    if (GetPosition() != PositionType::Static)
    {
      if (_computedStyle._nonInheritedData->SurroundData->Inset
          != other->_computedStyle._nonInheritedData->SurroundData->Inset)
      {
        return true;
      }
    }

    if (HasTransformRelatedProperty() != other->HasTransformRelatedProperty()
        || GetTransform() != other->GetTransform())
    {
      return true;
    }

    return false;
  }

  bool RenderStyle::OutOfFlowPositionStyleDidChange(const RenderStyle *other) const noexcept
  {
    // https://drafts.csswg.org/css-scroll-anchoring/#suppression-triggers
    // Determine if there is a style change that causes an element to become or stop
    // being absolutely or fixed positioned
    return other && HasOutOfFlowPosition() != other->HasOutOfFlowPosition();
  }

  // MARK: - Color

  const style::Color &RenderStyle::UnresolvedColorForProperty(css::PropertyId colorProperty,
                                                              bool visitedLink) const noexcept
  {
    switch (colorProperty)
    {
      case css::PropertyId::AccentColor: return GetAccentColor().ColorOrCurrentColor();
      case css::PropertyId::BackgroundColor:
        return visitedLink ? VisitedLinkBackgroundColor() : BackgroundColor();
      case css::PropertyId::BorderBottomColor:
        return visitedLink ? VisitedLinkBorderBottomColor() : BorderBottomColor();
      case css::PropertyId::BorderLeftColor:
        return visitedLink ? VisitedLinkBorderLeftColor() : BorderLeftColor();
      case css::PropertyId::BorderRightColor:
        return visitedLink ? VisitedLinkBorderRightColor() : BorderRightColor();
      case css::PropertyId::BorderTopColor:
        return visitedLink ? VisitedLinkBorderTopColor() : BorderTopColor();
      case css::PropertyId::Fill:          return Fill().ColorDisregardingType();
      case css::PropertyId::FloodColor:    return FloodColor();
      case css::PropertyId::LightingColor: return LightingColor();
      case css::PropertyId::OutlineColor:  return visitedLink ? VisitedLinkOutlineColor() : OutlineColor();
      case css::PropertyId::StopColor:     return StopColor();
      case css::PropertyId::Stroke:        return Stroke().ColorDisregardingType();
      case css::PropertyId::StrokeColor:   return visitedLink ? VisitedLinkStrokeColor() : StrokeColor();
      case css::PropertyId::BorderBlockEndColor:
      case css::PropertyId::BorderBlockStartColor:
      case css::PropertyId::BorderInlineEndColor:
      case css::PropertyId::BorderInlineStartColor:
        return UnresolvedColorForProperty(
          css::Property::ResolveDirectionAwareProperty(colorProperty, GetWritingMode()));
      case css::PropertyId::ColumnRuleColor:
        return visitedLink ? VisitedLinkColumnRuleColor() : ColumnRuleColor();
      case css::PropertyId::TextEmphasisColor:
        return visitedLink ? VisitedLinkTextEmphasisColor() : TextEmphasisColor();
      case css::PropertyId::WebkitTextFillColor:
        return visitedLink ? VisitedLinkTextFillColor() : TextFillColor();
      case css::PropertyId::TextDecorationColor:
        return visitedLink ? VisitedLinkTextDecorationColor() : TextDecorationColor();
      case css::PropertyId::CaretColor: return visitedLink ? VisitedLinkCaretColor() : CaretColor();
      default:                          krys_debug_assert_msg(false, "Unreachable"); break;
    }

    static NeverDestroyed<style::Color> defaultColor {};
    return defaultColor;
  }

  Color RenderStyle::ColorResolvingCurrentColor(css::PropertyId colorProperty,
                                                bool visitedLink) const noexcept
  {
    if (colorProperty == css::PropertyId::Color)
    {
      return visitedLink ? VisitedLinkColor() : GetColor();
    }

    auto &result = UnresolvedColorForProperty(colorProperty, visitedLink);
    if (result.IsCurrentColor())
    {
      if (colorProperty == css::PropertyId::TextDecorationColor)
      {
        if (HasPositiveStrokeWidth())
        {
          // Prefer stroke color if possible but not if it's fully transparent.
          auto strokeColor = ColorResolvingCurrentColor(UsedStrokeColorProperty(), visitedLink);
          if (strokeColor.IsVisible())
          {
            return strokeColor;
          }
        }

        return ColorResolvingCurrentColor(css::PropertyId::WebkitTextFillColor, visitedLink);
      }

      return visitedLink ? VisitedLinkColor() : GetColor();
    }

    return ColorResolvingCurrentColor(result, visitedLink);
  }

  Color RenderStyle::ColorResolvingCurrentColor(const style::Color &color, bool visitedLink) const noexcept
  {
    return color.ResolveColor(visitedLink ? VisitedLinkColor() : this->GetColor());
  }

  Color RenderStyle::VisitedDependentColor(css::PropertyId colorProperty,
                                           EnumSet<PaintBehavior> paintBehavior) const noexcept
  {
    auto unvisitedColor = ColorResolvingCurrentColor(colorProperty, false);
    if (GetInsideLink() != InsideLink::InsideVisited)
    {
      return unvisitedColor;
    }

    if (paintBehavior.Contains(PaintBehavior::DontShowVisitedLinks))
    {
      return unvisitedColor;
    }

    if (IsInSubtreeWithBlendMode())
    {
      return unvisitedColor;
    }

    auto visitedColor = ColorResolvingCurrentColor(colorProperty, true);

    // FIXME: Technically someone could explicitly specify the color transparent, but for now we'll just
    // assume that if the background color is transparent that it wasn't set. Note that it's weird that
    // we're returning unvisited info for a visited link, but given our restriction that the alpha values
    // have to match, it makes more sense to return the unvisited background color if specified than it
    // does to return black. This behavior matches what Firefox 4 does as well.
    if (colorProperty == css::PropertyId::BackgroundColor && visitedColor == Color::transparentBlack)
    {
      return unvisitedColor;
    }

    // Take the alpha from the unvisited color, but get the RGB values from the visited color.
    return visitedColor.ColorWithAlpha(unvisitedColor.AlphaAsFloat());
  }

  Color RenderStyle::VisitedDependentColorWithColorFilter(css::PropertyId colorProperty,
                                                          EnumSet<PaintBehavior> paintBehavior) const noexcept
  {
    if (!HasAppleColorFilter())
    {
      return VisitedDependentColor(colorProperty, paintBehavior);
    }

    return ColorByApplyingColorFilter(VisitedDependentColor(colorProperty, paintBehavior));
  }

  Color RenderStyle::ColorByApplyingColorFilter(const Color &color) const noexcept
  {
    auto transformedColor = color;
    std::ignore = GetAppleColorFilter().TransformColor(transformedColor);
    return transformedColor;
  }

  Color RenderStyle::ColorWithColorFilter(const style::Color &color) const noexcept
  {
    return ColorByApplyingColorFilter(ColorResolvingCurrentColor(color));
  }

  // MARK: - Used Values

  float RenderStyle::OutlineSize() const noexcept
  {
    return std::max(0.0f, style::Evaluate<float>(OutlineWidth(), style::ZoomNeeded {})
                            + style::Evaluate<float>(OutlineOffset(), style::ZoomNeeded {}));
  }

  css::CSSOMString RenderStyle::AltFromContent() const noexcept
  {
    if (auto *contentData = GetContent().TryData())
    {
      return contentData->AltText.value_or(css::CSSOMString {});
    }
    return {};
  }

  const css::CSSOMStringAtom &RenderStyle::HyphenString() const noexcept
  {
    krys_debug_assert(GetHyphens() != Hyphens::None);

    return krys::SwitchOn(
      GetHyphenateCharacter(),
      [&](const css::keywords::Auto &) -> const css::CSSOMStringAtom &
      {
        // FIXME: This should depend on locale, and use U+2010 HYPHEN when the primary font has a glyph for it
        // (fonts have no glyph data yet).
        static const NeverDestroyed<const css::CSSOMStringAtom> hyphenMinusString {u8"-"};
        return hyphenMinusString.get();
      },
      [](const css::CSSOMStringAtom &string) -> const css::CSSOMStringAtom & { return string; });
  }

  float RenderStyle::UsedStrokeWidth(const IntSize &viewportSize) const noexcept
  {
    // Use the stroke-width and stroke-color value combination only if stroke-color has been explicitly
    // specified. Since there will be no visible stroke when stroke-color is not specified (transparent by
    // default), we fall back to the legacy text stroke combination in that case, whose initial width is 0.
    if (!HasExplicitlySetStrokeColor())
    {
      return 0;
    }

    return krys::SwitchOn(
      GetStrokeWidth(), [&](const style::StrokeWidth::Fixed &fixedStrokeWidth) -> float
      { return style::Evaluate<float>(fixedStrokeWidth, style::ZoomNeeded {}); },
      [&](const style::StrokeWidth::Percentage &percentageStrokeWidth) -> float
      {
        // According to the spec, https://drafts.fxtf.org/paint/#stroke-width, the percentage is relative to
        // the scaled viewport size. The scaled viewport size is the geometric mean of the viewport width and
        // height.
        return percentageStrokeWidth.Value * (viewportSize.Width() + viewportSize.Height()) / 200.0f;
      },
      [&](const style::StrokeWidth::Calc &calcStrokeWidth) -> float
      {
        // FIXME: It is almost certainly wrong that calc and percentage are being handled differently -
        // https://bugs.webkit.org/show_bug.cgi?id=296482
        return style::Evaluate<float>(calcStrokeWidth, static_cast<float>(viewportSize.Width()),
                                      style::ZoomNeeded {});
      });
  }

  Color RenderStyle::UsedStrokeColor() const noexcept
  {
    return VisitedDependentColor(UsedStrokeColorProperty());
  }

  css::PropertyId RenderStyle::UsedStrokeColorProperty() const noexcept
  {
    // The legacy text stroke colour's initial value is `currentcolor`.
    return HasExplicitlySetStrokeColor() ? css::PropertyId::StrokeColor : css::PropertyId::Color;
  }

  style::Contain RenderStyle::UsedContain() const noexcept
  {
    auto result = GetContain();

    switch (GetContainerType())
    {
      case ContainerType::Normal: break;
      case ContainerType::Size:   result.Add({style::ContainValue::Style, style::ContainValue::Size}); break;
      case ContainerType::InlineSize:
        result.Add({style::ContainValue::Style, style::ContainValue::InlineSize});
        break;
    };

    return result;
  }

  UserSelect RenderStyle::UsedUserSelect() const noexcept
  {
    if (EffectiveInert())
    {
      return UserSelect::None;
    }

    auto value = GetUserSelect();
    if (GetUserModify() != UserModify::ReadOnly && GetUserDrag() != UserDrag::Element)
    {
      return value == UserSelect::None ? UserSelect::Text : value;
    }

    return value;
  }

  Color RenderStyle::UsedScrollbarThumbColor() const noexcept
  {
    return krys::SwitchOn(
      GetScrollbarColor(), [&](const css::keywords::Auto &) -> Color { return {}; },
      [&](const auto &parts) -> Color
      {
        if (HasAppleColorFilter())
        {
          return ColorByApplyingColorFilter(ColorResolvingCurrentColor(parts.Thumb));
        }
        return ColorResolvingCurrentColor(parts.Thumb);
      });
  }

  Color RenderStyle::UsedScrollbarTrackColor() const noexcept
  {
    return krys::SwitchOn(
      GetScrollbarColor(), [&](const css::keywords::Auto &) -> Color { return {}; },
      [&](const auto &parts) -> Color
      {
        if (HasAppleColorFilter())
        {
          return ColorByApplyingColorFilter(ColorResolvingCurrentColor(parts.Track));
        }
        return ColorResolvingCurrentColor(parts.Track);
      });
  }

  Color RenderStyle::UsedAccentColor(style::ColorOptions styleColorOptions) const noexcept
  {
    return krys::SwitchOn(
      GetAccentColor(), [](const css::keywords::Auto &) -> Color { return {}; },
      [&](const style::Color &color) -> Color
      {
        auto resolvedAccentColor = ColorResolvingCurrentColor(color);

        if (!resolvedAccentColor.IsOpaque())
        {
          auto computedCanvasColor = css::ColorFromKeyword(css::ValueId::Canvas, styleColorOptions);
          resolvedAccentColor = BlendSourceOver(computedCanvasColor, resolvedAccentColor);
        }

        if (HasAppleColorFilter())
        {
          return ColorByApplyingColorFilter(resolvedAccentColor);
        }

        return resolvedAccentColor;
      });
  }

  // MARK: - Derived Values

  template <typename OutsetValue>
  KRYS_NODISCARD static LayoutUnit ComputeOutset(const OutsetValue &outsetValue,
                                                 LayoutUnit borderWidth) noexcept
  {
    return krys::SwitchOn(
      outsetValue, [&](const typename OutsetValue::Number &number)
      { return LayoutUnit(number.Value * borderWidth); }, [&](const typename OutsetValue::Length &length)
      { return LayoutUnit(length.ResolveZoom(style::ZoomNeeded {})); });
  }

  LayoutBoxExtent RenderStyle::ImageOutsets(const style::BorderImage &image) const noexcept
  {
    return {
      ComputeOutset(image.Outset.Values.Top(),
                    style::Evaluate<LayoutUnit>(BorderTopWidth(), style::ZoomNeeded {})),
      ComputeOutset(image.Outset.Values.Right(),
                    style::Evaluate<LayoutUnit>(BorderRightWidth(), style::ZoomNeeded {})),
      ComputeOutset(image.Outset.Values.Bottom(),
                    style::Evaluate<LayoutUnit>(BorderBottomWidth(), style::ZoomNeeded {})),
      ComputeOutset(image.Outset.Values.Left(),
                    style::Evaluate<LayoutUnit>(BorderLeftWidth(), style::ZoomNeeded {})),
    };
  }

  LayoutBoxExtent RenderStyle::ImageOutsets(const style::MaskBorder &image) const noexcept
  {
    return {
      ComputeOutset(image.Outset.Values.Top(),
                    style::Evaluate<LayoutUnit>(BorderTopWidth(), style::ZoomNeeded {})),
      ComputeOutset(image.Outset.Values.Right(),
                    style::Evaluate<LayoutUnit>(BorderRightWidth(), style::ZoomNeeded {})),
      ComputeOutset(image.Outset.Values.Bottom(),
                    style::Evaluate<LayoutUnit>(BorderBottomWidth(), style::ZoomNeeded {})),
      ComputeOutset(image.Outset.Values.Left(),
                    style::Evaluate<LayoutUnit>(BorderLeftWidth(), style::ZoomNeeded {})),
    };
  }

  LayoutBoxExtent RenderStyle::BorderImageOutsets() const noexcept
  {
    return ImageOutsets(GetBorderImage());
  }

  LayoutBoxExtent RenderStyle::MaskBorderOutsets() const noexcept
  {
    return ImageOutsets(GetMaskBorder());
  }

  // MARK: - Logical

  const BorderValue &RenderStyle::BorderBefore(const WritingMode writingMode) const noexcept
  {
    switch (writingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom: return BorderTop();
      case FlowDirection::BottomToTop: return BorderBottom();
      case FlowDirection::LeftToRight: return BorderLeft();
      case FlowDirection::RightToLeft: return BorderRight();
    }
    krys_debug_assert_msg(false, "Unreachable");
    return BorderTop();
  }

  const BorderValue &RenderStyle::BorderAfter(const WritingMode writingMode) const noexcept
  {
    switch (writingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom: return BorderBottom();
      case FlowDirection::BottomToTop: return BorderTop();
      case FlowDirection::LeftToRight: return BorderRight();
      case FlowDirection::RightToLeft: return BorderLeft();
    }
    krys_debug_assert_msg(false, "Unreachable");
    return BorderBottom();
  }

  const BorderValue &RenderStyle::BorderStart(const WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? BorderLeft() : BorderRight();
    }
    return writingMode.IsInlineTopToBottom() ? BorderTop() : BorderBottom();
  }

  const BorderValue &RenderStyle::BorderEnd(const WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? BorderRight() : BorderLeft();
    }
    return writingMode.IsInlineTopToBottom() ? BorderBottom() : BorderTop();
  }

  UsedClear RenderStyle::GetUsedClear(const RenderElement &renderer) noexcept
  {
    auto computedClear = renderer.Style().GetClear();
    auto writingMode = renderer.ContainingBlock()->Style().GetWritingMode();
    switch (computedClear)
    {
      case Clear::None:  return UsedClear::None;
      case Clear::Both:  return UsedClear::Both;
      case Clear::Left:  return writingMode.IsLogicalLeftLineLeft() ? UsedClear::Left : UsedClear::Right;
      case Clear::Right: return writingMode.IsLogicalLeftLineLeft() ? UsedClear::Right : UsedClear::Left;
      case Clear::InlineStart:
        return writingMode.IsLogicalLeftInlineStart() ? UsedClear::Left : UsedClear::Right;
      case Clear::InlineEnd:
        return writingMode.IsLogicalLeftInlineStart() ? UsedClear::Right : UsedClear::Left;
    }

    krys_unreachable();
  }

  UsedFloat RenderStyle::GetUsedFloat(const RenderElement &renderer) noexcept
  {
    auto computedFloat = renderer.Style().Floating();
    auto writingMode = renderer.ContainingBlock()->Style().GetWritingMode();
    switch (computedFloat)
    {
      case Float::None:  return UsedFloat::None;
      case Float::Left:  return writingMode.IsLogicalLeftLineLeft() ? UsedFloat::Left : UsedFloat::Right;
      case Float::Right: return writingMode.IsLogicalLeftLineLeft() ? UsedFloat::Right : UsedFloat::Left;
      case Float::InlineStart:
        return writingMode.IsLogicalLeftInlineStart() ? UsedFloat::Left : UsedFloat::Right;
      case Float::InlineEnd:
        return writingMode.IsLogicalLeftInlineStart() ? UsedFloat::Right : UsedFloat::Left;
    }

    krys_unreachable();
  }
}
