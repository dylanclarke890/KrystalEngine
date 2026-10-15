#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/CanvasValue.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/Assert.hpp"

// FIXME: Placeholder definitions; see `Style/Seam.hpp`. Only `BuilderState` can be constructed, so only its
// members and the document functions can be reached.

namespace krys::boo
{
  const dom::Document &RenderView::GetDocument() const noexcept
  {
    krys_unreachable();
  }

  float RenderView::ZoomFactor() const noexcept
  {
    krys_unreachable();
  }

  FloatSize RenderView::SizeForCSSSmallViewportUnits() const noexcept
  {
    krys_unreachable();
  }

  FloatSize RenderView::SizeForCSSLargeViewportUnits() const noexcept
  {
    krys_unreachable();
  }

  FloatSize RenderView::SizeForCSSDynamicViewportUnits() const noexcept
  {
    krys_unreachable();
  }

  FloatSize RenderView::SizeForCSSDefaultViewportUnits() const noexcept
  {
    krys_unreachable();
  }

  bool RenderBox::HasEligibleContainmentForSizeQuery() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContentBoxWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContentBoxHeight() const noexcept
  {
    krys_unreachable();
  }

  bool RenderTheme::HasAppearanceForElementTypeFromUAStyle(const dom::Element &element) noexcept
  {
    if (!Is<html::HTMLElement>(element))
    {
      return false;
    }
    switch (element.TagQualifiedName().TagName())
    {
      case html::TagName::input:
      case html::TagName::textarea:
      case html::TagName::button:
      case html::TagName::progress:
      case html::TagName::select:
      case html::TagName::meter:    return true;
      default:                      return false;
    }
  }

  const SettingsValues &DocumentSettingsValues(const dom::Document &) noexcept
  {
    static constexpr SettingsValues settings {};
    return settings;
  }

  int32 AdjustForAbsoluteZoom(int32, const RenderView &) noexcept
  {
    krys_unreachable();
  }

  const RenderView *RenderViewOf(const dom::Document &) noexcept
  {
    return nullptr;
  }

  const RenderStyle *RenderStyleOf(const dom::Element &) noexcept
  {
    krys_unreachable();
  }

  const RenderBox *RenderBoxOf(const dom::Element &) noexcept
  {
    krys_unreachable();
  }

  bool DocumentUseDarkAppearance(const dom::Document &, const RenderStyle *style) noexcept
  {
    // Collapses `Document::useDarkAppearance`: there's no document colour scheme and the page is light, so
    // only a style that supports dark and not light uses the dark appearance.
    if (!style)
    {
      return false;
    }
    auto colorScheme = style->GetColorScheme().GetColorScheme();
    return colorScheme.Contains(ColorScheme::Dark) && !colorScheme.Contains(ColorScheme::Light);
  }

  style::ColorOptions DocumentStyleColorOptions(const dom::Document &document,
                                                const RenderStyle *style) noexcept
  {
    auto options = style::ColorOptions::None;
    if (DocumentUseDarkAppearance(document, style))
    {
      options |= style::ColorOptions::UseDarkAppearance;
    }
    return options;
  }

  bool DocumentCanRequest(const dom::Document &, const url::URL &) noexcept
  {
    return true;
  }

  void DocumentKeyframesRuleDidChange(dom::Document &, const css::CSSOMString &) noexcept
  {
  }
}

namespace krys::boo::style
{
  Maybe<double> AnchorPositionEvaluator::Evaluate(BuilderState &, Maybe<ScopedName>,
                                                  AnchorPositionEvaluator::Side) noexcept
  {
    krys_unreachable();
  }

  Maybe<double> AnchorPositionEvaluator::EvaluateSize(BuilderState &, Maybe<ScopedName>,
                                                      Maybe<AnchorSizeDimension>) noexcept
  {
    krys_unreachable();
  }

  RefPtr<const dom::Element>
    ContainerQueryEvaluator::SelectContainer(css::cq::Axis, const css::CSSOMString &, const dom::Element &,
                                             ContainerQueryEvaluator::SelectionMode) noexcept
  {
    krys_unreachable();
  }

  css::ValueId AnchorPositionEvaluator::ResolvePositionTryFallbackValueForSelfPosition(
    css::PropertyId propertyID, css::ValueId position, WritingMode writingMode,
    const BuilderPositionTryFallback &fallback) noexcept
  {
    // Implements the bullet starting "For the self-alignment properties" from step 4 of
    // https://drafts.csswg.org/css-anchor-position-1/#swap-due-to-a-try-tactic.

    krys_debug_assert(propertyID == css::PropertyId::AlignSelf || propertyID == css::PropertyId::JustifySelf);

    auto flipSidedPosition = [](css::ValueId position) -> css::ValueId
    {
      // Swap to the "opposite" position if the current position is "sided".
      // If there is no opposite value, nothing is changed.
      switch (position)
      {
        case css::ValueId::Start:     return css::ValueId::End;
        case css::ValueId::End:       return css::ValueId::Start;
        case css::ValueId::SelfStart: return css::ValueId::SelfEnd;
        case css::ValueId::SelfEnd:   return css::ValueId::SelfStart;
        case css::ValueId::FlexStart: return css::ValueId::FlexEnd;
        case css::ValueId::FlexEnd:   return css::ValueId::FlexStart;
        case css::ValueId::Left:      return css::ValueId::Right;
        case css::ValueId::Right:     return css::ValueId::Left;
        default:                      return position;
      }
    };

    auto flipStart = [](WritingMode writingMode, css::ValueId position) -> css::ValueId
    {
      // `justify-self` additionally takes `left`/`right`, `align-self` doesn't. When applying `flip-start`,
      // `justify-self` gets swapped with `align-self`. So if we're resolving `justify-self` (which later gets
      // swapped with `align-self`), and the position is `left`/`right`, resolve it to
      // `self-start`/`self-end`.
      switch (position)
      {
        case css::ValueId::Left:
          return writingMode.IsBidiLTR() ? css::ValueId::SelfStart : css::ValueId::SelfEnd;
        case css::ValueId::Right:
          return writingMode.IsBidiLTR() ? css::ValueId::SelfEnd : css::ValueId::SelfStart;
        default: return position;
      }
    };

    for (auto tactic : fallback.Tactics)
    {
      switch (tactic)
      {
        case PositionTryFallbackTactic::FlipBlock:
          if (propertyID == css::PropertyId::AlignSelf)
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipInline:
          if (propertyID == css::PropertyId::JustifySelf)
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipX:
          if (propertyID
              == (writingMode.IsHorizontal() ? css::PropertyId::JustifySelf : css::PropertyId::AlignSelf))
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipY:
          if (propertyID
              == (writingMode.IsHorizontal() ? css::PropertyId::AlignSelf : css::PropertyId::JustifySelf))
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipStart:
          if (propertyID == css::PropertyId::JustifySelf)
          {
            position = flipStart(writingMode, position);
          }
          break;
      }
    }

    return position;
  }
}
