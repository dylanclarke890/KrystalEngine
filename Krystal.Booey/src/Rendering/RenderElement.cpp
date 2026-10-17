#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/HTMLMarqueeElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "Krystal.Booey/Rendering/RenderDeprecatedFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCaption.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoElementRequest.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"

namespace krys::boo
{
  RenderElement::RenderElement(Type type, dom::ContainerNode &elementOrDocument, RenderStyle &&style,
                               EnumSet<TypeFlag> flags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderObject(type, elementOrDocument, flags, typeSpecificFlags), _style(krys::move(style))
  {
  }

  RenderElement::RenderElement(Type type, dom::Element &element, RenderStyle &&style,
                               EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderElement(type, static_cast<dom::ContainerNode &>(element), krys::move(style), baseTypeFlags,
                      typeSpecificFlags)
  {
  }

  RenderElement::RenderElement(Type type, dom::Document &document, RenderStyle &&style,
                               EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderElement(type, static_cast<dom::ContainerNode &>(document), krys::move(style), baseTypeFlags,
                      typeSpecificFlags)
  {
  }

  RenderElement::~RenderElement() noexcept
  {
    // Do not add any code here. Add it to willBeDestroyed() instead.
    krys_debug_assert(!_firstChild);
  }

  RenderPtr<RenderElement>
    RenderElement::CreateFor(dom::Element &element, RenderStyle &&style,
                             EnumSet<ConstructBlockLevelRendererFor> rendererTypeOverride) noexcept
  {
    // FIXME: Generated content images (`content: url(...)`) when there's no override, once there are images.
    switch (style.Display())
    {
      case DisplayType::None:
      case DisplayType::Contents: return nullptr;
      case DisplayType::Inline:
        if (rendererTypeOverride.Contains(ConstructBlockLevelRendererFor::Inline))
        {
          return CreateRenderer<RenderBlockFlow>(Type::BlockFlow, element, krys::move(style));
        }
        return CreateRenderer<RenderInline>(Type::Inline, element, krys::move(style));
      case DisplayType::Block:
      case DisplayType::FlowRoot:
      case DisplayType::InlineBlock:
        return CreateRenderer<RenderBlockFlow>(Type::BlockFlow, element, krys::move(style));
      case DisplayType::ListItem:
        if (rendererTypeOverride.Contains(ConstructBlockLevelRendererFor::ListItem))
        {
          return CreateRenderer<RenderBlockFlow>(Type::BlockFlow, element, krys::move(style));
        }
        return CreateRenderer<RenderListItem>(element, krys::move(style));
      case DisplayType::Flex:
      case DisplayType::InlineFlex:
        return CreateRenderer<RenderFlexibleBox>(Type::FlexibleBox, element, krys::move(style));
      case DisplayType::Grid:
      case DisplayType::InlineGrid:
      case DisplayType::GridLanes:
      case DisplayType::InlineGridLanes: return CreateRenderer<RenderGrid>(element, krys::move(style));
      case DisplayType::Box:
      case DisplayType::InlineBox:
        return CreateRenderer<RenderDeprecatedFlexibleBox>(element, krys::move(style));
      case DisplayType::RubyBase:
        return CreateRenderer<RenderInline>(Type::Inline, element, krys::move(style));
      case DisplayType::RubyAnnotation:
        return CreateRenderer<RenderBlockFlow>(Type::BlockFlow, element, krys::move(style));
      case DisplayType::Ruby: return CreateRenderer<RenderInline>(Type::Inline, element, krys::move(style));
      case DisplayType::RubyBlock:
        return CreateRenderer<RenderBlockFlow>(Type::BlockFlow, element, krys::move(style));
      default: break;
    }

    if (style.IsDisplayTableOrTablePart()
        && rendererTypeOverride.Contains(ConstructBlockLevelRendererFor::TableOrTablePart))
    {
      return CreateRenderer<RenderBlockFlow>(Type::BlockFlow, element, krys::move(style));
    }

    switch (style.Display())
    {
      case DisplayType::Table:
      case DisplayType::InlineTable:
        return CreateRenderer<RenderTable>(Type::Table, element, krys::move(style));
      case DisplayType::TableCell:    return CreateRenderer<RenderTableCell>(element, krys::move(style));
      case DisplayType::TableCaption: return CreateRenderer<RenderTableCaption>(element, krys::move(style));
      case DisplayType::TableRowGroup:
      case DisplayType::TableHeaderGroup:
      case DisplayType::TableFooterGroup:
        return CreateRenderer<RenderTableSection>(element, krys::move(style));
      case DisplayType::TableRow:         return CreateRenderer<RenderTableRow>(element, krys::move(style));
      case DisplayType::TableColumnGroup:
      case DisplayType::TableColumn:      return CreateRenderer<RenderTableCol>(element, krys::move(style));
      default:                            break;
    }
    krys_debug_assert(false);
    return nullptr;
  }

  style::Difference RenderElement::AdjustStyleDifference(style::Difference diff) const noexcept
  {
    // FIXME: Renderers have no layers yet (hasLayer() is false), so the composited cases and the layer status
    // changes of plug-ins, iframes and canvas don't apply.

    // If transform changed, and we are not composited, need to do a layout.
    if (diff.ContextSensitiveProperties.Contains(style::DifferenceContextSensitiveProperty::Transform))
    {
      // FIXME: when transforms are taken into account for overflow, we will need to do a layout.
      diff.Result = std::max(diff.Result, style::DifferenceResult::Layout);
    }

    if (diff.ContextSensitiveProperties.Contains(style::DifferenceContextSensitiveProperty::Opacity))
    {
      diff.Result = std::max(diff.Result, style::DifferenceResult::RepaintLayer);
    }

    if (diff.ContextSensitiveProperties.Contains(style::DifferenceContextSensitiveProperty::ClipPath))
    {
      diff.Result = std::max(diff.Result, style::DifferenceResult::Repaint);
    }

    if (diff.ContextSensitiveProperties.Contains(style::DifferenceContextSensitiveProperty::WillChange))
    {
      if (Style().GetWillChange().CanTriggerCompositing())
      {
        diff.Result = std::max(diff.Result, style::DifferenceResult::RecompositeLayer);
      }
    }

    if (IsHTMLMarquee())
    {
      diff.Result = std::max(diff.Result, style::DifferenceResult::Layout);
    }

    // If we have no layer(), just treat a RepaintLayer hint as a normal Repaint.
    if (diff == style::DifferenceResult::RepaintLayer)
    {
      diff.Result = style::DifferenceResult::Repaint;
    }

    return diff;
  }

  bool RenderElement::IsHTMLMarquee() const noexcept
  {
    auto *element = GetElement();
    if (!element || element->Renderer() != this)
    {
      return false;
    }
    auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element);
    return htmlElement && Is<html::HTMLMarqueeElement>(*htmlElement);
  }

  void RenderElement::InitializeStyle() noexcept
  {
    // FIXME: Pending resources (Style::loadPendingResources), once there's resource loading.

    StyleWillChange(style::DifferenceResult::NewStyle, Style());
    _hasInitializedStyle = true;
    StyleDidChange(style::DifferenceResult::NewStyle, nullptr);

    // FIXME: We shouldn't have any text children that would need styleDidChange at this point, once there
    // are text renderers; the view transition capture state, once there are view transitions.
  }

  void RenderElement::SetStyle(RenderStyle &&style, style::DifferenceResult minimalStyleDifference) noexcept
  {
    // FIXME: Should change RenderView so it can use initializeStyle too.
    // If we do that, we can assert m_hasInitializedStyle unconditionally,
    // and remove the check of m_hasInitializedStyle below too.
    krys_debug_assert(_hasInitializedStyle || IsRenderView());

    auto diff = style::Difference {};
    if (_hasInitializedStyle)
    {
      diff = style::GetDifference(_style, style);
    }

    diff.Result = std::max(diff.Result, minimalStyleDifference);
    diff = AdjustStyleDifference(diff);

    // FIXME: Pending resources and the repaint before the change, once there are resource loading and
    // painting.
    StyleWillChange(diff, style);
    auto oldStyle = krys::move(_style);
    _style = krys::move(style);
    bool detachedFromParent = !Parent();

    // FIXME: The fragmented flow state when the containing block changes, once there are fragmented flows.

    StyleDidChange(diff, &oldStyle);

    // Text renderers use their parent style. Notify them about the change.
    for (auto &child : ChildrenOfType<RenderText>(*this))
    {
      child.StyleDidChange(diff, &oldStyle);
    }

    // FIXME: |this| might be destroyed here. This can currently happen for a RenderTextFragment when
    // its first-letter block gets an update in RenderTextFragment::styleDidChange. For RenderTextFragment(s),
    // we will safely bail out with the detachedFromParent flag. We might want to broaden this condition
    // in the future as we move renderer changes out of layout and into style changes.
    if (detachedFromParent)
    {
      return;
    }

    // Now that the layer (if any) has been updated, we need to adjust the diff again,
    // check whether we should layout now, and decide if we need to repaint.
    auto updatedDiff = AdjustStyleDifference(diff);

    if (diff <= style::DifferenceResult::LayoutOutOfFlowMovementOnly)
    {
      SetNeedsLayoutForStyleDifference(updatedDiff, &oldStyle);
    }

    // FIXME: The repaint the difference asks for, once there's painting.
  }

  void RenderElement::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    auto *oldStyle = HasInitializedStyle() ? &Style() : nullptr;
    if (oldStyle)
    {
      // FIXME: Line layout, the stacking context's z-order lists, rendering-dependent regions, the
      // accessibility cache, the enclosing layer's visible content status and event region, the floating and
      // positioned object lists and the fragmented flow, once there are any.

      // reset style flags
      if (diff == style::DifferenceResult::Layout
          || diff == style::DifferenceResult::LayoutOutOfFlowMovementOnly)
      {
        SetFloating(false);
        ClearPositionedState();
      }
      SetHorizontalWritingMode(true);
      SetHasVisibleBoxDecorations(false);
      SetHasNonVisibleOverflow(false);
      SetHasTransformRelatedProperty(false);
    }

    // FIXME: Content visibility observation, outline counting, slow repaint objects and the extended
    // background, once there are any.
    (void)newStyle;
  }

  const RenderStyle *
    RenderElement::GetCachedPseudoStyle(const style::PseudoElementIdentifier &pseudoElementIdentifier,
                                        const RenderStyle *parentStyle) const noexcept
  {
    if (AllPublicPseudoElementTypes.Contains(pseudoElementIdentifier.Type)
        && !Style().HasPseudoStyle(pseudoElementIdentifier.Type))
    {
      return nullptr;
    }

    auto *cachedStyle = Style().GetCachedPseudoStyle(pseudoElementIdentifier);
    if (cachedStyle)
    {
      return cachedStyle;
    }

    auto result = GetUncachedPseudoStyle(style::PseudoElementRequest(pseudoElementIdentifier), parentStyle);
    if (result)
    {
      return const_cast<RenderStyle &>(_style).AddCachedPseudoStyle(krys::move(result));
    }
    return nullptr;
  }

  UniquePtr<RenderStyle>
    RenderElement::GetUncachedPseudoStyle(const style::PseudoElementRequest &pseudoElementRequest,
                                          const RenderStyle *parentStyle,
                                          const RenderStyle *ownStyle) const noexcept
  {
    if (AllPublicPseudoElementTypes.Contains(pseudoElementRequest.GetType()) && !ownStyle
        && !Style().HasPseudoStyle(pseudoElementRequest.GetType()))
    {
      return nullptr;
    }

    if (!parentStyle)
    {
      krys_debug_assert(!ownStyle);
      parentStyle = &Style();
    }

    if (IsAnonymous())
    {
      return nullptr;
    }

    auto element = ShareRef(*GetElement());
    auto &styleResolver = element->StyleResolver();

    auto resolvedStyle = styleResolver.StyleForPseudoElement(*element, pseudoElementRequest, {parentStyle});
    if (!resolvedStyle)
    {
      return nullptr;
    }

    // FIXME: Load the style's pending resources, once there's resource loading.

    return krys::move(resolvedStyle->Style);
  }

  void RenderElement::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    // FIXME: Images and SVG resources, once there are any.

    if (!Parent())
    {
      return;
    }

    if (diff == style::DifferenceResult::Layout || diff == style::DifferenceResult::Overflow)
    {
      RenderCounter::RendererStyleChanged(*this, oldStyle, _style);

      // If the object already needs layout, then setNeedsLayout won't do
      // any work. But if the containing block has changed, then we may need
      // to mark the new containing blocks for layout. The change that can
      // directly affect the containing block of this object is a change to
      // the position style.
      if (NeedsLayout() && oldStyle && oldStyle->GetPosition() != _style.GetPosition())
      {
        ScheduleLayout(MarkContainingBlocksForLayout());
      }
    }

    SetNeedsLayoutForStyleDifference(diff, oldStyle);

    // FIXME: An out-of-flow box whose original display changed between block and inline marks its static
    // position's containing block, once there's out-of-flow layout.

    // FIXME: Out-of-flow static positions, cursors, outline-auto ancestors, scroll anchoring and line layout,
    // once there are any.
  }

  void RenderElement::PropagateStyleToAnonymousChildren(StylePropagationType propagationType) noexcept
  {
    // FIXME: We could save this call when the change only affected non-inherited properties.
    for (auto &elementChild : ChildrenOfType<RenderElement>(*this))
    {
      if (!elementChild.IsAnonymous() || elementChild.Style().GetPseudoElementType()
          || elementChild.IsViewTransitionContainingBlock())
      {
        continue;
      }

      bool isBlockOrRuby =
        Is<RenderBlock>(elementChild) || elementChild.Style().Display() == DisplayType::Ruby;
      if (propagationType == StylePropagationType::BlockAndRubyChildren && !isBlockOrRuby)
      {
        continue;
      }

      // RenderFragmentedFlows are updated through the RenderView::styleDidChange function.
      if (elementChild.IsRenderFragmentedFlow())
      {
        continue;
      }

      // FIXME: Anonymous ruby boxes take the ruby style (createAnonymousStyleForRuby), once there's ruby.
      auto newStyle = RenderStyle::CreateAnonymousStyleWithDisplay(Style(), elementChild.Style().Display());

      if (Style().SpecifiesColumns())
      {
        if (elementChild.Style().SpecifiesColumns())
        {
          newStyle.InheritColumnPropertiesFrom(Style());
        }
        if (elementChild.Style().GetColumnSpan() == ColumnSpan::All)
        {
          newStyle.SetColumnSpan(ColumnSpan::All);
        }
      }

      // FIXME: Preserve the position of anonymous block continuations, which can be relative or sticky
      // positioned when they contain block descendants of positioned inlines, once there are continuations.

      UpdateAnonymousChildStyle(newStyle);

      elementChild.SetStyle(krys::move(newStyle));
    }
  }

  const RenderStyle &RenderElement::FirstLineStyle() const noexcept
  {
    // FIXME: It would be better to just set anonymous block first-line styles correctly.
    if (IsAnonymousBlock())
    {
      if (!PreviousInFlowSibling())
      {
        if (auto *firstLineStyle = Parent()->Style().GetCachedPseudoStyle({PseudoElementType::FirstLine}))
        {
          return *firstLineStyle;
        }
      }
      return Style();
    }

    if (auto *firstLineStyle = Style().GetCachedPseudoStyle({PseudoElementType::FirstLine}))
    {
      return *firstLineStyle;
    }

    return Style();
  }

  RenderBoxModelObject *RenderElement::OffsetParent() const noexcept
  {
    // If any of the following holds true return null and stop this algorithm:
    // A is the root element.
    // A is the HTML body element.
    // The computed value of the position property for element A is fixed.
    if (IsDocumentElementRenderer() || IsBody() || (IsFixedPositioned() && Is<RenderView>(Container())))
    {
      return nullptr;
    }

    // If A is an area HTML element which has a map HTML element somewhere in the ancestor
    // chain return the nearest ancestor map HTML element and stop this algorithm.
    // FIXME: Implement!

    // Return the nearest ancestor element of A for which at least one of the following is
    // true and stop this algorithm if such an ancestor is found:
    //     * The element is a containing block of absolutely-positioned descendants (regardless
    //       of whether there are any absolutely-positioned descendants).
    //     * The element is a containing block of fixed-positioned descendants.
    //     * It is the HTML body element.
    //     * The computed value of the position property of A is static and the ancestor
    //       is one of the following HTML elements: td, th, or table.
    //     * Our own extension: if there is a difference in the effective zoom

    bool skipTables = IsPositioned();
    float currZoom = Style().UsedZoom();
    auto *current = Parent();
    while (current
           && (!current->GetElement()
               || (!current->IsBody()
                   && !(IsFixedPositioned() ? current->CanContainFixedPositionObjects()
                                            : current->CanContainAbsolutelyPositionedObjects()))))
    {
      auto *element = DynamicDowncast<html::HTMLElement>(current->GetElement());
      if (!skipTables && element
          && (Is<html::HTMLTableElement>(*element) || Is<html::HTMLTableCellElement>(*element)))
      {
        break;
      }

      float newZoom = current->Style().UsedZoom();
      if (currZoom != newZoom)
      {
        break;
      }
      currZoom = newZoom;
      current = current->Parent();
    }

    return DynamicDowncast<RenderBoxModelObject>(current);
  }

  bool RenderElement::IsWritingModeRoot() const noexcept
  {
    return !Parent()
           || Parent()->Style().GetWritingMode().ComputedWritingMode()
                != Style().GetWritingMode().ComputedWritingMode();
  }

  bool RenderElement::IsFixedPositioned() const noexcept
  {
    return IsOutOfFlowPositioned() && Style().GetPosition() == PositionType::Fixed;
  }

  bool RenderElement::IsAbsolutelyPositioned() const noexcept
  {
    return IsOutOfFlowPositioned() && Style().GetPosition() == PositionType::Absolute;
  }

  bool RenderElement::IsBlockLevelBox() const noexcept
  {
    // block-level boxes are boxes that participate in a block formatting context.
    auto *renderBox = DynamicDowncast<RenderBox>(*this);
    if (!renderBox)
    {
      return false;
    }
    if (renderBox->IsFlexItem() || renderBox->IsGridItem() || renderBox->IsRenderTableCell())
    {
      return false;
    }
    return Style().IsDisplayBlockLevel();
  }

  bool RenderElement::IsAnonymousBlock() const noexcept
  {
    // FIXME: MathML blocks, multi-column sets, once there are any.
    return IsAnonymous() && (Style().Display() == DisplayType::Block || Style().Display() == DisplayType::Box)
           && !Style().GetPseudoElementType() && IsRenderBlock() && !IsRenderListMarker()
           && !IsRenderFragmentedFlow() && !IsRenderView() && !IsViewTransitionContainingBlock();
  }

  bool RenderElement::IsBlockContainer() const noexcept
  {
    auto display = Style().Display();
    return (display == DisplayType::Block || display == DisplayType::InlineBlock
            || display == DisplayType::FlowRoot || display == DisplayType::ListItem
            || display == DisplayType::TableCell || display == DisplayType::TableCaption)
           && !IsRenderReplaced();
  }

  bool RenderElement::IsBlockBox() const noexcept
  {
    // A block-level box that is also a block container.
    return IsBlockLevelBox() && IsBlockContainer();
  }

  bool RenderElement::IsBeforeContent() const noexcept
  {
    // Text nodes don't have their own styles, so ignore the style on a text node.
    // if (isRenderText())
    //     return false;
    if (Style().GetPseudoElementType() != PseudoElementType::Before)
    {
      return false;
    }
    return true;
  }

  bool RenderElement::IsAfterContent() const noexcept
  {
    // Text nodes don't have their own styles, so ignore the style on a text node.
    // if (isRenderText())
    //     return false;
    if (Style().GetPseudoElementType() != PseudoElementType::After)
    {
      return false;
    }
    return true;
  }

  bool RenderElement::IsBeforeContent(const RenderObject *renderer) noexcept
  {
    auto *element = DynamicDowncast<RenderElement>(renderer);
    return element && element->IsBeforeContent();
  }

  bool RenderElement::IsAfterContent(const RenderObject *renderer) noexcept
  {
    auto *element = DynamicDowncast<RenderElement>(renderer);
    return element && element->IsAfterContent();
  }

  RenderObject *RenderElement::FirstInFlowChild() const noexcept
  {
    if (auto *firstChild = FirstChild())
    {
      if (firstChild->IsInFlow())
      {
        return firstChild;
      }
      return firstChild->NextInFlowSibling();
    }
    return nullptr;
  }

  RenderObject *RenderElement::LastInFlowChild() const noexcept
  {
    if (auto *lastChild = LastChild())
    {
      if (lastChild->IsInFlow())
      {
        return lastChild;
      }
      return lastChild->PreviousInFlowSibling();
    }
    return nullptr;
  }

  bool RenderElement::IsBeforeOrAfterContent() const noexcept
  {
    return IsBeforeContent() || IsAfterContent();
  }

  dom::Element *RenderElement::GeneratingElement() const noexcept
  {
    return IsPseudoElement() ? static_cast<dom::PseudoElement &>(*GetElement()).HostElement() : GetElement();
  }

  Overflow RenderElement::EffectiveOverflowX() const noexcept
  {
    auto overflowX = Style().OverflowX();
    if (PaintContainmentApplies() && overflowX == Overflow::Visible)
    {
      return Overflow::Clip;
    }
    return overflowX;
  }

  Overflow RenderElement::EffectiveOverflowY() const noexcept
  {
    auto overflowY = Style().OverflowY();
    if (PaintContainmentApplies() && overflowY == Overflow::Visible)
    {
      return Overflow::Clip;
    }
    return overflowY;
  }

  Overflow RenderElement::EffectiveOverflowBlockDirection() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? EffectiveOverflowY() : EffectiveOverflowX();
  }

  Overflow RenderElement::EffectiveOverflowInlineDirection() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? EffectiveOverflowX() : EffectiveOverflowY();
  }

  bool RenderElement::HasPotentiallyScrollableOverflow() const noexcept
  {
    // We only need to test one overflow dimension since 'visible' and 'clip' always get accompanied
    // with 'clip' or 'visible' in the other dimension (see Style::Adjuster::adjust).
    return HasNonVisibleOverflow() && Style().OverflowX() != Overflow::Clip
           && Style().OverflowX() != Overflow::Visible;
  }

  bool RenderElement::HasClip() const noexcept
  {
    return IsOutOfFlowPositioned() && Style().HasClip();
  }

  bool RenderElement::HasClipOrNonVisibleOverflow() const noexcept
  {
    return HasClip() || HasNonVisibleOverflow();
  }

  bool RenderElement::CanEstablishContainingBlockWithTransform() const noexcept
  {
    return IsRenderBlock() || (IsTablePart() && !IsRenderTableCol());
  }

  bool RenderElement::MayContainOutOfFlowPositionedObjects(const RenderStyle *styleToUse) const noexcept
  {
    auto &style = styleToUse ? *styleToUse : Style();
    // FIXME: SVG foreign objects, once there are SVG renderers.
    return IsRenderView()
           || (CanEstablishContainingBlockWithTransform()
               && (styleToUse ? styleToUse->HasTransformRelatedProperty() : HasTransformRelatedProperty()))
           || (style.HasBackdropFilter() && !IsDocumentElementRenderer())
           || (style.HasFilter() && !IsDocumentElementRenderer()) || ShouldApplyLayoutContainment(styleToUse)
           || ShouldApplyPaintContainment(styleToUse) || IsViewTransitionContainingBlock();
  }

  bool RenderElement::CanContainAbsolutelyPositionedObjects(const RenderStyle *styleToUse) const noexcept
  {
    auto &style = styleToUse ? *styleToUse : Style();
    return MayContainOutOfFlowPositionedObjects(styleToUse) || style.GetPosition() != PositionType::Static
           || (IsRenderBlock()
               && style.GetWillChange().CreatesContainingBlockForAbsolutelyPositioned(
                 IsDocumentElementRenderer()));
  }

  bool RenderElement::CanContainFixedPositionObjects(const RenderStyle *styleToUse) const noexcept
  {
    auto &style = styleToUse ? *styleToUse : Style();
    return MayContainOutOfFlowPositionedObjects(styleToUse)
           || (IsRenderBlock()
               && style.GetWillChange().CreatesContainingBlockForOutOfFlowPositioned(
                 IsDocumentElementRenderer()));
  }

  bool RenderElement::ShouldApplyAnyContainment() const noexcept
  {
    return ShouldApplyLayoutContainment() || ShouldApplySizeContainment()
           || ShouldApplyInlineSizeContainment() || ShouldApplyStyleContainment()
           || ShouldApplyPaintContainment();
  }

  bool RenderElement::ShouldApplySizeOrInlineSizeContainment() const noexcept
  {
    return ShouldApplySizeContainment() || ShouldApplyInlineSizeContainment();
  }

  bool RenderElement::ShouldApplyLayoutContainment(const RenderStyle *styleToUse) const noexcept
  {
    return GetElement()
           && boo::ShouldApplyLayoutContainment(styleToUse ? *styleToUse : Style(), *GetElement());
  }

  bool RenderElement::ShouldApplySizeContainment() const noexcept
  {
    return GetElement() && boo::ShouldApplySizeContainment(Style(), *GetElement());
  }

  bool RenderElement::ShouldApplyInlineSizeContainment() const noexcept
  {
    return GetElement() && boo::ShouldApplyInlineSizeContainment(Style(), *GetElement());
  }

  bool RenderElement::ShouldApplyStyleContainment() const noexcept
  {
    return GetElement() && boo::ShouldApplyStyleContainment(Style(), *GetElement());
  }

  bool RenderElement::ShouldApplyPaintContainment(const RenderStyle *styleToUse) const noexcept
  {
    return GetElement()
           && boo::ShouldApplyPaintContainment(styleToUse ? *styleToUse : Style(), *GetElement());
  }

  bool RenderElement::HasEligibleContainmentForSizeQuery() const noexcept
  {
    switch (Style().GetContainerType())
    {
      case ContainerType::InlineSize: return ShouldApplyInlineSizeContainment();
      case ContainerType::Size:       return ShouldApplySizeContainment();
      case ContainerType::Normal:     return true;
    }
    krys_debug_assert(false);
    return false;
  }

  dom::Element *RenderElement::GetElement() const noexcept
  {
    return static_cast<dom::Element *>(GetNode());
  }

  void RenderElement::DidAttachChild(RenderObject &child, RenderObject *) noexcept
  {
    if (auto *textRenderer = DynamicDowncast<RenderText>(child))
    {
      textRenderer->StyleDidChange(style::DifferenceResult::Equal, nullptr);
    }

    // FIXME: Remove the layers created in hidden SVG subtrees, once there are layers.
  }

  RenderObject *RenderElement::AttachRendererInternal(RenderPtr<RenderObject> child,
                                                      RenderObject *beforeChild) noexcept
  {
    child->SetParent(this);

    if (_firstChild == beforeChild)
    {
      _firstChild = child.get();
    }

    if (beforeChild)
    {
      auto *previousSibling = beforeChild->PreviousSibling();
      if (previousSibling)
      {
        previousSibling->SetNextSibling(child.get());
      }
      child->SetPreviousSibling(previousSibling);
      child->SetNextSibling(beforeChild);
      beforeChild->SetPreviousSibling(child.get());
      return child.release();
    }
    {
      auto *lastChild = _lastChild;
      if (lastChild)
      {
        lastChild->SetNextSibling(child.get());
      }
      child->SetPreviousSibling(lastChild);
    }
    _lastChild = child.get();
    return child.release();
  }

  RenderPtr<RenderObject> RenderElement::DetachRendererInternal(RenderObject &renderer) noexcept
  {
    auto *parent = renderer.Parent();
    krys_debug_assert(parent);
    auto *nextSibling = renderer.NextSibling();

    if (auto *previousSibling = renderer.PreviousSibling())
    {
      previousSibling->SetNextSibling(nextSibling);
    }
    if (nextSibling)
    {
      nextSibling->SetPreviousSibling(renderer.PreviousSibling());
    }

    if (parent->FirstChild() == &renderer)
    {
      parent->_firstChild = nextSibling;
    }
    if (parent->LastChild() == &renderer)
    {
      parent->_lastChild = renderer.PreviousSibling();
    }

    renderer.SetPreviousSibling(nullptr);
    renderer.SetNextSibling(nullptr);
    renderer.SetParent(nullptr);
    return RenderPtr<RenderObject>(&renderer);
  }

  bool RenderElement::HasClipPath() const noexcept
  {
    return Style().HasClipPath();
  }

  bool RenderElement::HasFilter() const noexcept
  {
    return Style().HasFilter();
  }

  bool RenderElement::HasMask() const noexcept
  {
    return Style().HasMask();
  }

  bool RenderElement::HasBackdropFilter() const noexcept
  {
    return Style().HasBackdropFilter();
  }

  bool RenderElement::HasBlendMode() const noexcept
  {
    return Style().HasBlendMode();
  }

  bool RenderElement::IsTransparent() const noexcept
  {
    return Style().HasOpacity();
  }

  bool RenderElement::HasHiddenBackface() const noexcept
  {
    return Style().GetBackfaceVisibility() == BackfaceVisibility::Hidden;
  }

  bool RenderElement::CreatesGroupForStyle(const RenderStyle &style) noexcept
  {
    return style.HasOpacity() || style.HasMask() || style.HasClipPath() || style.HasFilter()
           || style.HasBackdropFilter() || style.HasBlendMode();
  }

  bool RenderElement::HasViewTransitionName() const noexcept
  {
    return !Style().GetViewTransitionName().IsNone();
  }

  bool RenderElement::IsViewTransitionRoot() const noexcept
  {
    return Style().GetPseudoElementType() == PseudoElementType::ViewTransition;
  }

  bool RenderElement::RequiresRenderingConsolidationForViewTransition() const noexcept
  {
    // FIXME: Renderers captured in a view transition too, once there are view transitions.
    return HasViewTransitionName();
  }

  FloatRect RenderElement::TransformReferenceBoxRect() const noexcept
  {
    return TransformReferenceBoxRect(Style());
  }

  FloatRect RenderElement::TransformReferenceBoxRect(const RenderStyle &style) const noexcept
  {
    return ReferenceBoxRect(TransformBoxToCSSBoxType(style.GetTransformBox()));
  }

  FloatRect RenderElement::ReferenceBoxRect(CSSBoxType) const noexcept
  {
    // CSS box model code is implemented in RenderBox::referenceBoxRect().
    // FIXME: The reference boxes of SVG renderers, once there's SVG.
    return {};
  }

  namespace
  {
    KRYS_NODISCARD RenderLayer *FindNextLayer(const RenderElement &currRenderer,
                                              const RenderLayer &parentLayer,
                                              const RenderObject *siblingToTraverseFrom,
                                              bool checkParent = true) noexcept
    {
      // Step 1: If our layer is a child of the desired parent, then return our layer.
      auto *ourLayer =
        currRenderer.HasLayer() ? Downcast<RenderLayerModelObject>(currRenderer).Layer() : nullptr;
      if (ourLayer && ourLayer->Parent() == &parentLayer)
      {
        return ourLayer;
      }

      // Step 2: If we don't have a layer, or our layer is the desired parent, then descend
      // into our siblings trying to find the next layer whose parent is the desired parent.
      if (!ourLayer || ourLayer == &parentLayer)
      {
        for (auto *child = siblingToTraverseFrom ? siblingToTraverseFrom->NextSibling()
                                                 : currRenderer.FirstChild();
             child; child = child->NextSibling())
        {
          auto *element = DynamicDowncast<RenderElement>(*child);
          if (!element)
          {
            continue;
          }
          if (auto *nextLayer = FindNextLayer(*element, parentLayer, nullptr, false))
          {
            return nextLayer;
          }
        }
      }

      // Step 3: If our layer is the desired parent layer, then we're finished. We didn't
      // find anything.
      if (ourLayer == &parentLayer)
      {
        return nullptr;
      }

      // Step 4: If |checkParent| is set, climb up to our parent and check its siblings that
      // follow us to see if we can locate a layer.
      if (checkParent && currRenderer.Parent())
      {
        return FindNextLayer(*currRenderer.Parent(), parentLayer, &currRenderer, true);
      }

      return nullptr;
    }

    KRYS_NODISCARD RenderLayer *LayerNextSiblingRespectingTopLayer(const RenderElement &renderer,
                                                                   const RenderLayer &parentLayer) noexcept
    {
      // FIXME: Top layer renderers follow the top layer's order (topLayerRenderLayers), once there's a top
      // layer.
      return FindNextLayer(*renderer.Parent(), parentLayer, &renderer);
    }

    void AddLayers(const RenderElement &insertedRenderer, RenderElement &currentRenderer,
                   RenderLayer &parentLayer) noexcept
    {
      if (currentRenderer.HasLayer())
      {
        // FIXME: Top layer and backdrop renderers go in the view's layer, once there's a top layer.
        auto *beforeChild = LayerNextSiblingRespectingTopLayer(insertedRenderer, parentLayer);
        parentLayer.AddChild(*Downcast<RenderLayerModelObject>(currentRenderer).Layer(), beforeChild);
        return;
      }

      for (auto &child : ChildrenOfType<RenderElement>(currentRenderer))
      {
        AddLayers(insertedRenderer, child, parentLayer);
      }
    }
  }

  void RenderElement::RemoveLayers() noexcept
  {
    auto *parentLayer = LayerParent();
    if (!parentLayer)
    {
      return;
    }

    if (HasLayer())
    {
      parentLayer->RemoveChild(*Downcast<RenderLayerModelObject>(*this).Layer());
      return;
    }

    for (auto &child : ChildrenOfType<RenderElement>(*this))
    {
      child.RemoveLayers();
    }
  }

  void RenderElement::MoveLayers(RenderLayer &newParent) noexcept
  {
    if (HasLayer())
    {
      // FIXME: Top layer and backdrop renderers stay in the view's layer, once there's a top layer.
      auto *layer = Downcast<RenderLayerModelObject>(*this).Layer();
      if (auto *layerParent = layer->Parent())
      {
        layerParent->RemoveChild(*layer);
      }
      newParent.AddChild(*layer);
      return;
    }

    for (auto &child : ChildrenOfType<RenderElement>(*this))
    {
      child.MoveLayers(newParent);
    }
  }

  RenderLayer *RenderElement::LayerParent() const noexcept
  {
    // FIXME: Top layer and backdrop renderers' layers are children of the view's layer, once there's a top
    // layer.
    return Parent()->EnclosingLayer();
  }

  // This answers the question "if this renderer had a layer, what would its next sibling layer be".
  RenderLayer *RenderElement::LayerNextSibling(RenderLayer &parentLayer) const noexcept
  {
    return LayerNextSiblingRespectingTopLayer(*this, parentLayer);
  }

  void RenderElement::InsertedIntoTree() noexcept
  {
    // Keep our layer hierarchy updated. Optimize for the common case where we don't have any children
    // and don't have a layer attached to ourselves.
    if (FirstChild() || HasLayer())
    {
      if (auto *parentLayer = LayerParent())
      {
        AddLayers(*this, *this, *parentLayer);
      }
    }

    // If |this| is visible but this object was not, tell the layer it has some visible content
    // that needs to be drawn and layer visibility optimization can't be used
    if (Parent()->Style().UsedVisibility() != Visibility::Visible
        && Style().UsedVisibility() == Visibility::Visible && !HasLayer())
    {
      if (auto *parentLayer = LayerParent())
      {
        parentLayer->DirtyVisibleContentStatus();
      }
    }

    RenderObject::InsertedIntoTree();
  }

  void RenderElement::WillBeRemovedFromTree() noexcept
  {
    // If we remove a visible child from an invisible parent, we don't know the layer visibility any more.
    if (Parent()->Style().UsedVisibility() != Visibility::Visible
        && Style().UsedVisibility() == Visibility::Visible && !HasLayer())
    {
      // FIXME: should get parent layer. Necessary?
      if (auto *enclosingLayer = Parent()->EnclosingLayer())
      {
        enclosingLayer->DirtyVisibleContentStatus();
      }
    }
    // Keep our layer hierarchy updated.
    if (FirstChild() || HasLayer())
    {
      RemoveLayers();
    }

    RenderObject::WillBeRemovedFromTree();
  }

  void RenderElement::WillBeDestroyed() noexcept
  {
    // FIXME: Slow repaint objects and visibility callbacks, once there are any.

    if (HasCounterNodeMap())
    {
      RenderCounter::DestroyCounterNodes(*this);
    }

    // FIXME: The subtree layout root, image clients, outlines, paused image animations and the other upstream
    // clean-up, once there are any.
    RenderObject::WillBeDestroyed();
  }

#pragma region Layout

  void RenderElement::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      if (child->NeedsLayout())
      {
        Downcast<RenderElement>(*child).Layout();
      }
      krys_debug_assert(!child->NeedsLayout());
    }
    ClearNeedsLayout();
  }

  void RenderElement::SetChildNeedsLayout(MarkingBehavior markParents) noexcept
  {
    krys_debug_assert(!IsSetNeedsLayoutForbidden());
    if (NormalChildNeedsLayout())
    {
      return;
    }
    SetNormalChildNeedsLayoutBit(true);
    if (markParents == MarkingBehavior::MarkContainingBlockChain)
    {
      ScheduleLayout(MarkContainingBlocksForLayout());
    }
  }

  void RenderElement::SetNeedsOutOfFlowMovementLayout(const RenderStyle *) noexcept
  {
    krys_debug_assert(!IsSetNeedsLayoutForbidden());
    if (NeedsOutOfFlowMovementLayout())
    {
      return;
    }
    SetNeedsOutOfFlowMovementLayoutBit(true);
    ScheduleLayout(MarkContainingBlocksForLayout());
    // FIXME: Mark the layer as needing a full repaint, once there are layers.
  }

  void RenderElement::ClearChildNeedsLayout() noexcept
  {
    SetNormalChildNeedsLayoutBit(false);
    SetOutOfFlowChildNeedsLayoutBit(false);
    SetNeedsSimplifiedNormalFlowLayoutBit(false);
    SetNeedsOutOfFlowMovementLayoutBit(false);
    SetOutOfFlowChildNeedsStaticPositionLayoutBit(false);
  }

  void RenderElement::SetNeedsLayoutForStyleDifference(style::Difference diff,
                                                       const RenderStyle *oldStyle) noexcept
  {
    if (diff == style::DifferenceResult::Layout)
    {
      SetNeedsLayoutAndPreferredWidthsUpdate();
    }
    else if (diff == style::DifferenceResult::LayoutOutOfFlowMovementOnly)
    {
      SetNeedsOutOfFlowMovementLayout(oldStyle);
    }
    else if (diff == style::DifferenceResult::OverflowAndOutOfFlowMovement)
    {
      SetNeedsOutOfFlowMovementLayout(oldStyle);
      SetNeedsLayoutForOverflowChange();
    }
    else if (diff == style::DifferenceResult::Overflow)
    {
      SetNeedsLayoutForOverflowChange();
    }
  }

  void RenderElement::SetNeedsLayoutForOverflowChange() noexcept
  {
    krys_debug_assert(!IsSetNeedsLayoutForbidden());
    // FIXME: Eagerly preventing simplified layout due to the (unlikely) possibility of a size change
    // is possibly wasteful. We could in theory detect an actual change during layout, and
    // unwind back to restart proper layout.
    if (OverflowChangesMayAffectLayout())
    {
      SetNeedsLayout();
      return;
    }
    if (NeedsSimplifiedNormalFlowLayout())
    {
      return;
    }
    SetNeedsSimplifiedNormalFlowLayoutBit(true);
    ScheduleLayout(MarkContainingBlocksForLayout());
    // FIXME: Mark the layer as needing a full repaint, once there are layers.
  }

  void RenderElement::SetOutOfFlowChildNeedsStaticPositionLayout() noexcept
  {
    // FIXME: Currently this dirty bit has a very limited useage but should be expanded to
    // optimize all kinds of out-of-flow cases.
    // It's also assumed that regular, positioned child related bits are already set.
    krys_debug_assert(!IsSetNeedsLayoutForbidden());
    krys_debug_assert(OutOfFlowChildNeedsLayout() || SelfNeedsLayout() || NeedsSimplifiedNormalFlowLayout()
                      || !Parent());
    SetOutOfFlowChildNeedsStaticPositionLayoutBit(true);
  }

#pragma endregion
}
