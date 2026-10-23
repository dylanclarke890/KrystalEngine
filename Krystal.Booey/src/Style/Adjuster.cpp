#include "Krystal.Booey/Style/Adjuster.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/Update.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD bool IsHTMLElement(const dom::Element *element, html::TagName tagName) noexcept
    {
      return element && Is<html::HTMLElement>(*element) && element->TagQualifiedName().TagName() == tagName;
    }

    KRYS_NODISCARD bool HasLocalName(const dom::Element *element, html::TagName tagName) noexcept
    {
      return element && element->TagQualifiedName().TagName() == tagName;
    }

    // https://www.w3.org/TR/css-display-3/#transformations
    KRYS_NODISCARD DisplayType EquivalentBlockDisplay(const RenderStyle &style) noexcept
    {
      switch (auto display = style.Display())
      {
        case DisplayType::Block:
        case DisplayType::Table:
        case DisplayType::Box:
        case DisplayType::Flex:
        case DisplayType::Grid:
        case DisplayType::GridLanes:
        case DisplayType::FlowRoot:
        case DisplayType::ListItem:
        case DisplayType::RubyBlock:        return display;
        case DisplayType::InlineTable:      return DisplayType::Table;
        case DisplayType::InlineBox:        return DisplayType::Box;
        case DisplayType::InlineFlex:       return DisplayType::Flex;
        case DisplayType::InlineGrid:       return DisplayType::Grid;
        case DisplayType::InlineGridLanes:  return DisplayType::GridLanes;
        case DisplayType::Ruby:             return DisplayType::RubyBlock;

        case DisplayType::Inline:
        case DisplayType::InlineBlock:
        case DisplayType::TableRowGroup:
        case DisplayType::TableHeaderGroup:
        case DisplayType::TableFooterGroup:
        case DisplayType::TableRow:
        case DisplayType::TableColumnGroup:
        case DisplayType::TableColumn:
        case DisplayType::TableCell:
        case DisplayType::TableCaption:
        case DisplayType::RubyBase:
        case DisplayType::RubyAnnotation:   return DisplayType::Block;

        case DisplayType::Contents:         krys_debug_assert(false); return DisplayType::Contents;
        case DisplayType::None:             krys_debug_assert(false); return DisplayType::None;
      }
      krys_debug_assert(false);
      return DisplayType::Block;
    }

    // https://www.w3.org/TR/css-display-3/#transformations
    KRYS_NODISCARD DisplayType EquivalentInlineDisplay(const RenderStyle &style) noexcept
    {
      switch (auto display = style.Display())
      {
        case DisplayType::Block:            return DisplayType::InlineBlock;
        case DisplayType::Table:            return DisplayType::InlineTable;
        case DisplayType::Box:              return DisplayType::InlineBox;
        case DisplayType::Flex:             return DisplayType::InlineFlex;
        case DisplayType::Grid:             return DisplayType::InlineGrid;
        case DisplayType::GridLanes:        return DisplayType::InlineGridLanes;
        case DisplayType::RubyBlock:        return DisplayType::Ruby;

        case DisplayType::Inline:
        case DisplayType::InlineBlock:
        case DisplayType::InlineTable:
        case DisplayType::InlineBox:
        case DisplayType::InlineFlex:
        case DisplayType::InlineGrid:
        case DisplayType::InlineGridLanes:
        case DisplayType::Ruby:
        case DisplayType::RubyBase:
        case DisplayType::RubyAnnotation:   return display;

        case DisplayType::FlowRoot:
        case DisplayType::ListItem:
        case DisplayType::TableRowGroup:
        case DisplayType::TableHeaderGroup:
        case DisplayType::TableFooterGroup:
        case DisplayType::TableRow:
        case DisplayType::TableColumnGroup:
        case DisplayType::TableColumn:
        case DisplayType::TableCell:
        case DisplayType::TableCaption:     return DisplayType::Inline;

        case DisplayType::Contents:         krys_debug_assert(false); return DisplayType::Contents;
        case DisplayType::None:             krys_debug_assert(false); return DisplayType::None;
      }
      krys_debug_assert(false);
      return DisplayType::Inline;
    }

    KRYS_NODISCARD bool ShouldInheritTextDecorationsInEffect(const RenderStyle &style,
                                                             const dom::Element *element) noexcept
    {
      if (style.IsFloating() || style.HasOutOfFlowPosition())
      {
        return false;
      }

      // FIXME: Media elements' user agent shadow roots (the media controls) stop text decorations, with
      // media.

      // Outermost <svg> roots are considered to be atomic inline-level.
      if (auto *svgElement = DynamicDowncast<svg::SVGElement>(element);
          svgElement && svgElement->IsOutermostSVGSVGElement())
      {
        return false;
      }

      switch (style.Display())
      {
        case DisplayType::InlineTable:
        case DisplayType::InlineBlock:
        case DisplayType::InlineGrid:
        case DisplayType::InlineGridLanes:
        case DisplayType::InlineFlex:
        case DisplayType::InlineBox:       return false;
        default:                           break;
      };

      return true;
    }

    KRYS_NODISCARD bool IsScrollableOverflow(Overflow overflow) noexcept
    {
      return overflow == Overflow::Scroll || overflow == Overflow::Auto;
    }

    KRYS_NODISCARD TouchAction ComputeUsedTouchAction(const RenderStyle &style,
                                                      TouchAction usedTouchAction) noexcept
    {
      // https://w3c.github.io/pointerevents/#determining-supported-touch-behavior
      // "A touch behavior is supported if it conforms to the touch-action property of each element between
      // the hit tested element and its nearest ancestor with the default touch behavior (including both the
      // hit tested element and the element with the default touch behavior)."

      bool hasDefaultTouchBehavior =
        IsScrollableOverflow(style.OverflowX()) || IsScrollableOverflow(style.OverflowY());
      if (hasDefaultTouchBehavior)
      {
        usedTouchAction = ComputedStyle::InitialTouchAction();
      }

      auto touchAction = style.GetTouchAction();
      if (touchAction == ComputedStyle::InitialTouchAction())
      {
        return usedTouchAction;
      }

      if (usedTouchAction.IsNone() || touchAction.IsNone())
      {
        return css::keywords::None {};
      }

      auto usedTouchActionEnumSet = usedTouchAction.TryEnumSet();
      if (!usedTouchActionEnumSet)
      {
        return touchAction;
      }

      auto touchActionEnumSet = touchAction.TryEnumSet();
      if (!touchActionEnumSet)
      {
        return usedTouchAction;
      }

      auto sharedTouchActions = *usedTouchActionEnumSet & *touchActionEnumSet;
      if (sharedTouchActions.IsEmpty())
      {
        return css::keywords::None {};
      }

      return sharedTouchActions;
    }

    KRYS_NODISCARD bool IsOverflowClipOrVisible(Overflow overflow) noexcept
    {
      return overflow == Overflow::Clip || overflow == Overflow::Visible;
    }

    KRYS_NODISCARD bool ShouldInlinifyForRuby(const RenderStyle &style,
                                              const RenderStyle &parentBoxStyle) noexcept
    {
      auto parentDisplay = parentBoxStyle.Display();
      auto hasRubyParent = parentDisplay == DisplayType::Ruby || parentDisplay == DisplayType::RubyBlock
                           || parentDisplay == DisplayType::RubyAnnotation
                           || parentDisplay == DisplayType::RubyBase;

      return hasRubyParent && !style.HasOutOfFlowPosition() && !style.IsFloating();
    }

    KRYS_NODISCARD bool HasUnsupportedRubyDisplay(DisplayType display, const dom::Element *element) noexcept
    {
      // Only allow ruby elements to have ruby display types for now.
      switch (display)
      {
        case DisplayType::Ruby:
        case DisplayType::RubyBlock:
          // Test for localName so this also allows WebVTT ruby elements.
          return !HasLocalName(element, html::TagName::ruby);
        case DisplayType::RubyAnnotation: return !HasLocalName(element, html::TagName::rt);
        case DisplayType::RubyBase:       krys_debug_assert(false); return false;
        default:                          return false;
      }
    }

    // https://drafts.csswg.org/css-ruby-1/#bidi
    KRYS_NODISCARD UnicodeBidi ForceBidiIsolationForRuby(UnicodeBidi unicodeBidi) noexcept
    {
      switch (unicodeBidi)
      {
        case UnicodeBidi::Normal:
        case UnicodeBidi::Embed:
        case UnicodeBidi::Isolate:         return UnicodeBidi::Isolate;
        case UnicodeBidi::Override:
        case UnicodeBidi::IsolateOverride: return UnicodeBidi::IsolateOverride;
        case UnicodeBidi::Plaintext:       return UnicodeBidi::Plaintext;
      }
      krys_debug_assert(false);
      return UnicodeBidi::Isolate;
    }

    KRYS_NODISCARD bool ShouldTreatAutoZIndexAsZero(const RenderStyle &style) noexcept
    {
      return style.HasOpacity() || style.HasTransformRelatedProperty() || style.HasMask()
             || style.HasClipPath() || style.HasFilter() || style.HasBackdropFilter() || style.HasBlendMode()
             || style.HasIsolation() || style.GetPosition() == PositionType::Sticky
             || style.GetPosition() == PositionType::Fixed
             || style.GetWillChange().CanCreateStackingContext();
    }

    KRYS_NODISCARD bool HasEffectiveDisplayNoneForDisplayContents(const dom::Element &element) noexcept
    {
      // https://drafts.csswg.org/css-display-3/#unbox-svg
      // FIXME: <g>, <use> and <tspan> have special (?) behavior for display:contents in the current draft
      // spec.
      if (Is<svg::SVGElement>(element))
      {
        return true;
      }
      // Not sure MathML code can handle it.
      if (Is<mathml::MathMLElement>(element))
      {
        return true;
      }
      if (!Is<html::HTMLElement>(element))
      {
        return false;
      }

      // https://drafts.csswg.org/css-display-3/#unbox-html
      switch (element.TagQualifiedName().TagName())
      {
        case html::TagName::br:
        case html::TagName::wbr:
        case html::TagName::meter:
        case html::TagName::applet:
        case html::TagName::progress:
        case html::TagName::canvas:
        case html::TagName::embed:
        case html::TagName::object:
        case html::TagName::audio:
        case html::TagName::iframe:
        case html::TagName::img:
        case html::TagName::video:
        case html::TagName::frame:
        case html::TagName::frameset:
        case html::TagName::input:
        case html::TagName::textarea:
        case html::TagName::select:   return true;
        default:                      break;
      }

      return false;
    }
  }

  Adjuster::Adjuster(const dom::Document &document, const RenderStyle &parentStyle,
                     const RenderStyle *parentBoxStyle, dom::Element *element) noexcept
      : _document(ShareCheckedRef(document)), _parentStyle(parentStyle),
        _parentBoxStyle(parentBoxStyle ? *parentBoxStyle : _parentStyle), _element(ShareRefPtr(element))
  {
  }

  bool Adjuster::AdjustEventListenerRegionTypesForRootStyle(RenderStyle &rootStyle,
                                                            const dom::Document &document) noexcept
  {
    auto regionTypes = ComputeEventListenerRegionTypes(document, rootStyle, document, {});
    // FIXME: Add the window's event listener region types (there is no window).

    bool changed = regionTypes != rootStyle.EventListenerRegionTypes();
    rootStyle.SetEventListenerRegionTypes(regionTypes);
    return changed;
  }

  EnumSet<EventListenerRegionType>
    Adjuster::ComputeEventListenerRegionTypes(const dom::Document &, const RenderStyle &,
                                              const dom::EventTarget &,
                                              EnumSet<EventListenerRegionType> parentTypes) noexcept
  {
    // Event listener regions (wheel, touch and interaction regions) are platform features that aren't
    // enabled, so the types are the parent's.
    return parentTypes;
  }

  void Adjuster::AdjustFromBuilder(RenderStyle &style) noexcept
  {
    // Do some adjustments that don't depend on element or parent style and are safe to cache.
    // This allows copy-on-write to trigger before caching.

    if (style.SpecifiedZIndex().IsAuto())
    {
      if (ShouldTreatAutoZIndexAsZero(style))
      {
        style.SetUsedZIndex(0);
      }
    }
    else if (style.GetPosition() != PositionType::Static)
    {
      style.SetUsedZIndex(style.SpecifiedZIndex());
    }

    // Adjust any coordinated value lists.
    style.AdjustAnimations();
    style.AdjustTransitions();
    style.AdjustBackgroundLayers();
    style.AdjustMaskLayers();

    // Do the same for scroll-timeline and view-timeline longhands.
    style.AdjustScrollTimelines();
    style.AdjustViewTimelines();
  }

  void Adjuster::AdjustFirstLetterStyle(RenderStyle &style) noexcept
  {
    if (style.GetPseudoElementType() != PseudoElementType::FirstLetter)
    {
      return;
    }

    // Force inline display (except for floating first-letters).
    style.SetEffectiveDisplay(style.IsFloating() ? DisplayType::Block : DisplayType::Inline);
  }

  void Adjuster::AdjustFirstLineStyle(RenderStyle &style) noexcept
  {
    if (style.GetPseudoElementType() != PseudoElementType::FirstLine)
    {
      return;
    }

    // Force inline display.
    style.SetEffectiveDisplay(DisplayType::Inline);
  }

  void Adjuster::AdjustAnimatedStyle(RenderStyle &style, EnumSet<AnimationImpact> impact) const noexcept
  {
    Adjust(style);

    // Set an explicit used z-index in two cases:
    // 1. When the element respects z-index, and the style has an explicit z-index set (for example, the
    // animation
    //    itself may animate z-index).
    // 2. When we want the stacking context side-effets of explicit z-index, via forceStackingContext.
    // It's important to not clobber an existing used z-index, since an earlier animation may have set it, but
    // we may still need to update the used z-index value from the specified value.

    if (style.UsedZIndex().IsAuto() && impact.Contains(AnimationImpact::ForcesStackingContext))
    {
      style.SetUsedZIndex(0);
    }
  }

  void Adjuster::Adjust(RenderStyle &style) const noexcept
  {
    if (style.Display() == DisplayType::Contents)
    {
      AdjustDisplayContentsStyle(style);
    }

    if (IsHTMLElement(_element.get(), html::TagName::frame)
        || IsHTMLElement(_element.get(), html::TagName::frameset))
    {
      // Framesets ignore display, position and float properties.
      style.SetPosition(PositionType::Static);
      style.SetEffectiveDisplay(DisplayType::Block);
      style.SetFloating(Float::None);
    }

    auto documentElement = _document->DocumentElement();

    if (style.Display() != DisplayType::None && style.Display() != DisplayType::Contents)
    {
      if (auto *element = _element.get())
      {
        // Tables never support the -webkit-* values for text-align and will reset back to the default.
        if (IsHTMLElement(element, html::TagName::table)
            && (style.GetTextAlign() == TextAlign::WebKitLeft
                || style.GetTextAlign() == TextAlign::WebKitCenter
                || style.GetTextAlign() == TextAlign::WebKitRight))
        {
          style.SetTextAlign(TextAlign::Start);
        }

        // Ruby text does not support float or position. This might change with evolution of the
        // specification.
        if (IsHTMLElement(element, html::TagName::rt))
        {
          style.SetPosition(PositionType::Static);
          style.SetFloating(Float::None);
        }

        if (IsHTMLElement(element, html::TagName::legend))
        {
          style.SetEffectiveDisplay(EquivalentBlockDisplay(style));
        }
      }

      if (HasUnsupportedRubyDisplay(style.Display(), _element.get()))
      {
        style.SetEffectiveDisplay(style.Display() == DisplayType::RubyBlock ? DisplayType::Block
                                                                            : DisplayType::Inline);
      }

      // Top layer elements are always position: absolute; unless the position is set to fixed.
      // https://fullscreen.spec.whatwg.org/#new-stacking-layer
      if (_element.get() != documentElement.get() && style.GetPosition() != PositionType::Absolute
          && style.GetPosition() != PositionType::Fixed && dom::IsInTopLayerOrBackdrop(style, _element.get()))
      {
        style.SetPosition(PositionType::Absolute);
      }

      // Absolute/fixed positioned elements, floating elements and the document element need block-like
      // outside display.
      if (style.HasOutOfFlowPosition() || style.IsFloating()
          || (_element && documentElement.get() == _element.get()))
      {
        style.SetEffectiveDisplay(EquivalentBlockDisplay(style));
      }

      AdjustFirstLetterStyle(style);
      AdjustFirstLineStyle(style);

      // FIXME: Don't support this mutation for pseudo styles like first-letter or first-line, since it's not
      // completely clear how that should work.
      if (style.Display() == DisplayType::Inline && !style.GetPseudoElementType()
          && style.GetWritingMode().ComputedWritingMode()
               != _parentStyle.GetWritingMode().ComputedWritingMode())
      {
        style.SetEffectiveDisplay(DisplayType::InlineBlock);
      }

      // After performing the display mutation, check table rows. We do not honor position:relative or
      // position:sticky on table rows or cells. This has been established for position:relative in CSS2.1
      // (and caused a crash in containingBlock() on some sites).
      if ((style.Display() == DisplayType::TableHeaderGroup || style.Display() == DisplayType::TableRowGroup
           || style.Display() == DisplayType::TableFooterGroup || style.Display() == DisplayType::TableRow)
          && style.GetPosition() == PositionType::Relative)
      {
        style.SetPosition(PositionType::Static);
      }

      // writing-mode does not apply to table row groups, table column groups, table rows, and table columns.
      if (style.Display() == DisplayType::TableColumn || style.Display() == DisplayType::TableColumnGroup
          || style.Display() == DisplayType::TableFooterGroup
          || style.Display() == DisplayType::TableHeaderGroup || style.Display() == DisplayType::TableRow
          || style.Display() == DisplayType::TableRowGroup)
      {
        style.SetWritingMode(_parentStyle.GetWritingMode().ComputedWritingMode());
      }

      // FIXME: Adjust this once CSSWG clarifies exactly how the initial value should compute on other display
      // types. For now, this gives mostly backwards-compatible behavior.
      if (style.Display() == DisplayType::Grid || style.Display() == DisplayType::InlineGrid)
      {
        if (style.GetGridAutoFlow().GetDirection() == GridAutoFlow::Direction::Normal)
        {
          style.SetGridAutoFlowDirection(GridAutoFlow::Direction::Row);
        }
      }
      else if (style.Display() == DisplayType::GridLanes || style.Display() == DisplayType::InlineGridLanes)
      {
        if (style.GetGridAutoFlow().GetDirection() == GridAutoFlow::Direction::Normal)
        {
          if (!style.GridTemplateRows().IsNone() && style.GridTemplateColumns().IsNone())
          {
            style.SetGridAutoFlowDirection(GridAutoFlow::Direction::Column);
          }
          else
          {
            style.SetGridAutoFlowDirection(GridAutoFlow::Direction::Row);
          }
        }
      }

      if (style.IsDisplayDeprecatedFlexibleBox())
      {
        // FIXME: Since we don't support block-flow on flexible boxes yet, disallow setting
        // of block-flow to anything other than StyleWritingMode::HorizontalTb.
        // https://bugs.webkit.org/show_bug.cgi?id=46418 - Flexible box support.
        style.SetWritingMode(StyleWritingMode::HorizontalTb);
      }

      if (_parentBoxStyle.IsDisplayDeprecatedFlexibleBox())
      {
        style.SetFloating(Float::None);
      }

      // https://www.w3.org/TR/css-display/#transformations
      // "A parent with a grid or flex display value blockifies the box’s display type."
      if (_parentBoxStyle.IsDisplayFlexibleOrGridFormattingContextBox())
      {
        style.SetFloating(Float::None);
        style.SetEffectiveDisplay(EquivalentBlockDisplay(style));
      }

      // https://www.w3.org/TR/css-ruby-1/#anon-gen-inlinize
      if (ShouldInlinifyForRuby(style, _parentBoxStyle))
      {
        style.SetEffectiveDisplay(EquivalentInlineDisplay(style));
      }
      // https://drafts.csswg.org/css-ruby-1/#bidi
      if (style.IsRubyContainerOrInternalRubyBox())
      {
        style.SetUnicodeBidi(ForceBidiIsolationForRuby(style.GetUnicodeBidi()));
      }
    }

    auto hasAutoZIndex = [](const RenderStyle &style, const RenderStyle &parentBoxStyle)
    {
      if (style.SpecifiedZIndex().IsAuto())
      {
        return true;
      }

      // FIXME: With the layer-based SVG engine (off by default), the z-index property applies to all SVG
      // elements but outermost <svg> elements that aren't positioned.

      // Make sure our z-index value is only applied if the object is positioned.
      return style.GetPosition() == PositionType::Static
             && !parentBoxStyle.IsDisplayFlexibleOrGridFormattingContextBox();
    };

    bool hasAutoSpecifiedZIndex = hasAutoZIndex(style, _parentBoxStyle);

    // FIXME: SVG transforms that aren't stored on the style ('animatedLocalTransform', <use> x/y, the
    // viewBox) count as transforms with the layer-based SVG engine (off by default), and so does a hidden
    // backface in a 3D context with the backface visibility interoperability setting (off by default).
    auto hasTransformRelatedProperty = [](const RenderStyle &style)
    {
      return style.HasTransformRelatedProperty();
    };

    // Auto z-index becomes 0 for the root element and transparent objects. This prevents
    // cases where objects that should be blended as a single unit end up with a non-transparent
    // object wedged in between them. Auto z-index also becomes 0 for objects that specify
    // transforms/masks/reflections.
    if (hasAutoSpecifiedZIndex)
    {
      if ((_element && documentElement.get() == _element.get()) || hasTransformRelatedProperty(style)
          || ShouldTreatAutoZIndexAsZero(style) || dom::IsInTopLayerOrBackdrop(style, _element.get()))
      {
        style.SetUsedZIndex(0);
      }
      else
      {
        style.SetUsedZIndex(css::keywords::Auto {});
      }
    }
    else
    {
      style.SetUsedZIndex(style.SpecifiedZIndex());
    }

    if (auto *element = _element.get())
    {
      // Textarea considers overflow visible as auto.
      if (IsHTMLElement(element, html::TagName::textarea))
      {
        style.SetOverflowX(style.OverflowX() == Overflow::Visible ? Overflow::Auto : style.OverflowX());
        style.SetOverflowY(style.OverflowY() == Overflow::Visible ? Overflow::Auto : style.OverflowY());
      }

      if (auto *input = DynamicDowncast<html::HTMLInputElement>(DynamicDowncast<html::HTMLElement>(element));
          input && input->IsPasswordField())
      {
        style.SetTextSecurity(style.GetInputSecurity() == InputSecurity::Auto ? TextSecurity::Disc
                                                                              : TextSecurity::None);
      }

      // Disallow -webkit-user-modify on ::pseudo elements, except if that pseudo-element targets a slot,
      // in which case we want the editability to be passed onto the slotted contents.
      if (element->IsInUserAgentShadowTree() && element->UserAgentPart() != dom::DOMStringAtom::Null()
          && !Is<html::HTMLSlotElement>(*element))
      {
        style.SetUserModify(UserModify::ReadOnly);
      }

      if (IsHTMLElement(element, html::TagName::marquee))
      {
        // There is no marquee direction property, so marquees are horizontal.
        // Make horizontal marquees not wrap.
        style.SetWhiteSpaceCollapse(WhiteSpaceCollapse::Collapse);
        style.SetTextWrapMode(TextWrapMode::NoWrap);
        style.SetTextAlign(TextAlign::Start);
      }

      // FIXME: Force-hide the element for its visibility adjustment (`VisibilityAdjustment::Subtree`), and
      // mark popover invokers (`setIsPopoverInvoker`); elements have no visibility adjustment or invoked
      // popover.

      // FIXME: `<details>` content auto-reveals in its user agent shadow tree (`detailsAutoExpandEnabled`),
      // once `<details>` has one.

      if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
          htmlElement && htmlElement->IsHiddenUntilFound())
      {
        style.SetAutoRevealsWhenFound();
      }
    }

    if (ShouldInheritTextDecorationsInEffect(style, _element.get()))
    {
      style.AddToTextDecorationLineInEffect(style.GetTextDecorationLine());
    }
    else
    {
      style.SetTextDecorationLineInEffect(TextDecorationLine {style.GetTextDecorationLine()});
    }

    bool overflowIsClipOrVisible =
      IsOverflowClipOrVisible(style.OverflowY()) && IsOverflowClipOrVisible(style.OverflowX());

    if (!overflowIsClipOrVisible
        && (style.Display() == DisplayType::Table || style.Display() == DisplayType::InlineTable))
    {
      // Tables only support overflow:hidden and overflow:visible and ignore anything else,
      // see https://drafts.csswg.org/css2/#overflow. As a table is not a block
      // container box the rules for resolving conflicting x and y values in CSS Overflow Module
      // Level 3 do not apply. Arguably overflow-x and overflow-y aren't allowed on tables but
      // all UAs allow it.
      if (style.OverflowX() != Overflow::Hidden)
      {
        style.SetOverflowX(Overflow::Visible);
      }
      if (style.OverflowY() != Overflow::Hidden)
      {
        style.SetOverflowY(Overflow::Visible);
      }
      // If we are left with conflicting overflow values for the x and y axes on a table then resolve
      // both to Overflow::Visible. This is interoperable behaviour but is not specced anywhere.
      if (style.OverflowX() == Overflow::Visible)
      {
        style.SetOverflowY(Overflow::Visible);
      }
      else if (style.OverflowY() == Overflow::Visible)
      {
        style.SetOverflowX(Overflow::Visible);
      }
    }
    else if (!IsOverflowClipOrVisible(style.OverflowY()))
    {
      // FIXME: Once we implement pagination controls, overflow-x should default to hidden
      // if overflow-y is set to -webkit-paged-x or -webkit-page-y. For now, we'll let it
      // default to auto so we can at least scroll through the pages.
      // Values of 'clip' and 'visible' can only be used with 'clip' and 'visible'.
      // If they aren't, 'clip' and 'visible' is reset.
      if (style.OverflowX() == Overflow::Visible)
      {
        style.SetOverflowX(Overflow::Auto);
      }
      else if (style.OverflowX() == Overflow::Clip)
      {
        style.SetOverflowX(Overflow::Hidden);
      }
    }
    else if (!IsOverflowClipOrVisible(style.OverflowX()))
    {
      // Values of 'clip' and 'visible' can only be used with 'clip' and 'visible'.
      // If they aren't, 'clip' and 'visible' is reset.
      if (style.OverflowY() == Overflow::Visible)
      {
        style.SetOverflowY(Overflow::Auto);
      }
      else if (style.OverflowY() == Overflow::Clip)
      {
        style.SetOverflowY(Overflow::Hidden);
      }
    }

    // FIXME: Paged overflow (`-webkit-paged-x`/`-webkit-paged-y`) on elements other than <html> and <body>
    // sets the column styles from the pagination mode (`setColumnStylesFromPaginationMode`); there is no
    // frame view.

    // Let the theme also have a crack at adjusting the style.
    if (style.HasAppearance())
    {
      AdjustThemeStyle(style, _parentStyle);
    }

    // This should be kept in sync with requiresRenderingConsolidationForViewTransition
    if (style.Preserves3D())
    {
      bool forceToFlat = style.OverflowX() != Overflow::Visible || style.HasOpacity()
                         || style.OverflowY() != Overflow::Visible || style.HasClip() || style.HasClipPath()
                         || style.HasFilter() || style.HasIsolation() || style.HasMask()
                         || style.HasBackdropFilter() || style.HasBlendMode()
                         || !style.GetViewTransitionName().IsNone();
      // FIXME: An element captured in an active view transition is forced to flat; there are no view
      // transitions.
      style.SetTransformStyleForcedToFlat(forceToFlat);
    }

    style.SetIsEffectivelyTransparent(style.GetOpacity().IsTransparent()
                                      || _parentStyle.IsEffectivelyTransparent());

    if (auto *element = DynamicDowncast<svg::SVGElement>(_element.get()))
    {
      AdjustSVGElementStyle(style, *element);
    }

    // If the inherited value of justify-items includes the 'legacy' keyword (plus 'left', 'right' or
    // 'center'), 'legacy' computes to the the inherited value. Otherwise, 'auto' computes to 'normal'.
    if (_parentBoxStyle.GetJustifyItems().Resolve().PositionType() == ItemPositionType::Legacy
        && style.GetJustifyItems().Resolve().Position() == ItemPosition::Legacy)
    {
      style.SetJustifyItems(_parentBoxStyle.GetJustifyItems());
    }

    style.SetUsedTouchAction(ComputeUsedTouchAction(style, _parentStyle.UsedTouchAction()));

    // Counterpart in Element::addToTopLayer/removeFromTopLayer!
    auto hasInertAttribute = [](const dom::Element *element) -> bool
    {
      return element && Is<html::HTMLElement>(*element) && element->HasAttribute(u8"inert");
    };
    // FIXME: The document element is an inert subtree root while there is an active modal dialog or a
    // fullscreen element, and the active modal dialog and the fullscreen element aren't inert; there is no
    // top layer.
    auto isInertSubtreeRoot = [&](const dom::Element *element) -> bool
    {
      return hasInertAttribute(element);
    };
    if (isInertSubtreeRoot(_element.get()))
    {
      style.SetEffectiveInert(true);
    }

    if (auto *element = _element.get())
    {
      style.SetEventListenerRegionTypes(ComputeEventListenerRegionTypes(
        *_document, style, *element, _parentStyle.EventListenerRegionTypes()));
    }

    if (_parentStyle.GetContentVisibility() != ContentVisibility::Hidden)
    {
      if (_element && IsSkippedContentRoot(style, *_element))
      {
        style.SetUsedContentVisibility(style.GetContentVisibility());
      }
    }
    if (style.GetContentVisibility() == ContentVisibility::Auto)
    {
      style.ContainIntrinsicWidthAddAuto();
      style.ContainIntrinsicHeightAddAuto();
    }
  }

  void Adjuster::AdjustDisplayContentsStyle(RenderStyle &style) const noexcept
  {
    bool isInTopLayer = dom::IsInTopLayerOrBackdrop(style, _element.get());
    if (isInTopLayer || (_element && _document->DocumentElement().get() == _element.get()))
    {
      style.SetEffectiveDisplay(DisplayType::Block);
      return;
    }

    if (!_element && style.GetPseudoElementType() != PseudoElementType::Before
        && style.GetPseudoElementType() != PseudoElementType::After)
    {
      style.SetEffectiveDisplay(DisplayType::None);
      return;
    }

    if (_element && HasEffectiveDisplayNoneForDisplayContents(*_element))
    {
      style.SetEffectiveDisplay(DisplayType::None);
    }
  }

  void Adjuster::AdjustSVGElementStyle(RenderStyle &style, const svg::SVGElement &svgElement) noexcept
  {
    // Only the root <svg> element in an SVG document fragment tree honors css position
    if (!svgElement.IsOutermostSVGSVGElement())
    {
      style.SetPosition(ComputedStyle::InitialPosition());
    }

    // FIXME: With the layer-based SVG engine (off by default), SVG2's stacking context rules give some SVG
    // elements a used z-index of 0.

    // (Legacy)RenderSVGRoot handles zooming for the whole SVG subtree, so foreignObject content should
    // not be scaled again.
    if (Is<svg::SVGForeignObjectElement>(svgElement))
    {
      std::ignore = style.SetUsedZoom(Evaluate<float>(ComputedStyle::InitialZoom()));
    }

    // SVG text layout code expects us to be a block-level style element.
    // While in theory any block level element would work (flex, grid etc), since we construct RenderBlockFlow
    // for both foreign object and svg text, in practice only block layout happens here.
    if ((Is<svg::SVGForeignObjectElement>(svgElement)
         || svgElement.TagQualifiedName().LocalName().View() == u8"text")
        && GeneratesBox(style))
    {
      style.SetEffectiveDisplay(DisplayType::Block);
    }
  }

  void Adjuster::AdjustThemeStyle(RenderStyle &style, const RenderStyle &) const noexcept
  {
    krys_debug_assert(style.HasAppearance());
    auto isOldWidthAuto = style.Width().IsAuto();
    auto isOldMinWidthAuto = style.MinWidth().IsAuto();
    auto isOldHeightAuto = style.Height().IsAuto();
    auto isOldMinHeightAuto = style.MinHeight().IsAuto();

    // FIXME: The render theme adjusts the style of elements with an appearance (`RenderTheme::adjustStyle`);
    // there is no render theme.

    if (style.UsedContain().Contains(ContainValue::Size))
    {
      if (!style.ContainIntrinsicWidth().IsNone())
      {
        if (isOldWidthAuto)
        {
          style.SetWidth(css::keywords::Auto {});
        }
        if (isOldMinWidthAuto)
        {
          style.SetMinWidth(css::keywords::Auto {});
        }
      }
      if (!style.ContainIntrinsicHeight().IsNone())
      {
        if (isOldHeightAuto)
        {
          style.SetHeight(css::keywords::Auto {});
        }
        if (isOldMinHeightAuto)
        {
          style.SetMinHeight(css::keywords::Auto {});
        }
      }
    }
  }

  void Adjuster::PropagateToDocumentElementAndInitialContainingBlock(Update &update,
                                                                     const dom::Document &document) noexcept
  {
    auto body = document.Body();
    auto *bodyStyle = body ? update.ElementStyle(*body) : nullptr;
    auto *documentElementStyle = update.ElementStyle(*document.DocumentElement());

    if (!documentElementStyle)
    {
      return;
    }

    // https://drafts.csswg.org/css-contain-2/#contain-property
    // "Additionally, when any containments are active on either the HTML html or body elements, propagation
    // of properties from the body element to the initial containing block, the viewport, or the canvas
    // background, is disabled."
    auto shouldPropagateFromBody = [&]
    {
      if (bodyStyle && !bodyStyle->UsedContain().IsNone())
      {
        return false;
      }
      return documentElementStyle->UsedContain().IsNone();
    }();

    auto writingMode = [&]
    {
      if (shouldPropagateFromBody && bodyStyle && bodyStyle->HasExplicitlySetWritingMode())
      {
        return bodyStyle->GetWritingMode().ComputedWritingMode();
      }
      if (documentElementStyle->HasExplicitlySetWritingMode())
      {
        return documentElementStyle->GetWritingMode().ComputedWritingMode();
      }
      return ComputedStyle::InitialWritingMode();
    }();

    auto direction = [&]
    {
      if (documentElementStyle->HasExplicitlySetDirection())
      {
        return documentElementStyle->GetWritingMode().ComputedTextDirection();
      }
      if (shouldPropagateFromBody && bodyStyle && bodyStyle->HasExplicitlySetDirection())
      {
        return bodyStyle->GetWritingMode().ComputedTextDirection();
      }
      return ComputedStyle::InitialDirection();
    }();

    // https://drafts.csswg.org/css-writing-modes-3/#icb
    auto &renderViewStyle = document.RenderViewStyle();
    auto viewWritingMode = renderViewStyle.GetWritingMode();
    if (writingMode != viewWritingMode.ComputedWritingMode()
        || direction != viewWritingMode.ComputedTextDirection())
    {
      auto newRootStyle = RenderStyle::ClonePtr(renderViewStyle);
      newRootStyle->SetWritingMode(writingMode);
      newRootStyle->SetDirection(direction);
      // FIXME: setColumnStylesFromPaginationMode, once documents have a view with a pagination mode.
      update.AddInitialContainingBlockUpdate(krys::move(newRootStyle));
    }

    // https://drafts.csswg.org/css-writing-modes-3/#principal-flow
    if (writingMode != documentElementStyle->GetWritingMode().ComputedWritingMode()
        || direction != documentElementStyle->GetWritingMode().ComputedTextDirection())
    {
      auto documentElement = const_cast<dom::Document &>(document).DocumentElement();
      auto *documentElementUpdate = update.GetElementUpdate(*documentElement);
      if (!documentElementUpdate)
      {
        update.AddElement(*documentElement, nullptr, {RenderStyle::ClonePtr(*documentElementStyle)});
        documentElementUpdate = update.GetElementUpdate(*documentElement);
      }
      documentElementUpdate->Style->SetWritingMode(writingMode);
      documentElementUpdate->Style->SetDirection(direction);
      documentElementUpdate->Changes.Add(Change::Inherited);
    }
  }

  UniquePtr<RenderStyle>
    Adjuster::RestoreUsedDocumentElementStyleToComputed(const RenderStyle &style) noexcept
  {
    if (style.GetWritingMode().ComputedWritingMode() == ComputedStyle::InitialWritingMode()
        && style.GetWritingMode().ComputedTextDirection() == ComputedStyle::InitialDirection())
    {
      return {};
    }

    auto adjusted = RenderStyle::ClonePtr(style);
    if (!style.HasExplicitlySetWritingMode())
    {
      adjusted->SetWritingMode(ComputedStyle::InitialWritingMode());
    }
    if (!style.HasExplicitlySetDirection())
    {
      adjusted->SetDirection(ComputedStyle::InitialDirection());
    }

    return adjusted;
  }
}
