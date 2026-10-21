#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/HTML/HTMLMarqueeElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/InlineWalker.hpp"
#include "Krystal.Booey/Rendering/OutlinePainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "Krystal.Booey/Rendering/RenderDeprecatedFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
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
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderRuby.hpp"
#include "Krystal.Booey/Style/PendingResources.hpp"
#include "Krystal.Booey/Style/PseudoElementRequest.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrientation.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollMargin.hpp"

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

  static RefPtr<StyleImage> MinimallySupportedContentDataImage(const style::Content &content) noexcept
  {
    // Minimal support for content properties replacing an entire element.
    // Works only if we have exactly one piece of content and it's a URL.
    // Otherwise acts as if we didn't support this feature.
    auto *data = content.TryData();
    if (!data)
    {
      return nullptr;
    }
    if (data->List.size() != 1)
    {
      return nullptr;
    }
    auto *image = std::get_if<style::Content::Image>(&data->List[0]);
    if (!image)
    {
      return nullptr;
    }
    return ShareRefPtr(image->Value.Value.get());
  }

  bool RenderElement::IsContentDataSupported(const style::Content &content) noexcept
  {
    return MinimallySupportedContentDataImage(content) != nullptr;
  }

  RenderPtr<RenderElement>
    RenderElement::CreateFor(dom::Element &element, RenderStyle &&style,
                             EnumSet<ConstructBlockLevelRendererFor> rendererTypeOverride) noexcept
  {
    if (rendererTypeOverride.IsEmpty())
    {
      if (RefPtr styleImage = MinimallySupportedContentDataImage(style.GetContent());
          styleImage && !Is<dom::PseudoElement>(element))
      {
        style::LoadPendingResources(style, element.NodeDocument(), &element);
        auto image = CreateRenderer<RenderImage>(Type::Image, element, krys::move(style), styleImage.get());
        image->SetIsGeneratedContent();
        image->UpdateAltText();
        return image;
      }
    }

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

  boo::ImageOrientation RenderElement::GetImageOrientation() const noexcept
  {
    return style::ToPlatform<style::ImageOrientation> {}(Style().GetImageOrientation());
  }

  void RenderElement::InitializeStyle() noexcept
  {
    style::LoadPendingResources(_style, GetDocument(), GetElement());

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

    style::LoadPendingResources(style, GetDocument(), GetElement());

    auto didRepaint = RepaintBeforeStyleChange(diff, _style, style);
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

    if (!didRepaint
        && (updatedDiff == style::DifferenceResult::RepaintLayer
            || ShouldRepaintForStyleDifference(updatedDiff)))
    {
      // Do a repaint with the new style now, e.g., for example if we go from
      // not having an outline to having an outline.
      Repaint();
    }
  }

  namespace
  {
    bool HasNonWhitespaceTextContent(const RenderElement &renderer) noexcept
    {
      if (!renderer.ChildrenInline())
      {
        return false;
      }

      if (auto *blockContainer = DynamicDowncast<RenderBlockFlow>(renderer))
      {
        for (InlineWalker walker(*blockContainer); !walker.AtEnd(); walker.Advance())
        {
          if (auto *textRenderer = DynamicDowncast<RenderText>(*walker.Current());
              textRenderer && !textRenderer->ContainsOnlyCollapsibleWhitespace())
          {
            return true;
          }
        }
        return false;
      }

      for (auto &textRenderer : ChildrenOfType<RenderText>(renderer))
      {
        if (!textRenderer.ContainsOnlyCollapsibleWhitespace())
        {
          return true;
        }
      }
      return false;
    }

    template <typename FillLayers>
    bool MustRepaintFillLayers(const RenderElement &renderer, const FillLayers &layers) noexcept
    {
      // Nobody will use multiple layers without wanting fancy positioning.
      if (layers.UsedLength() > 1)
      {
        return true;
      }

      auto &layer = layers.UsedFirst();

      // Make sure we have a valid image.
      auto image = layer.GetImage().TryStyleImage();
      if (!image || !image->CanRender(&renderer, renderer.Style().UsedZoom()))
      {
        return false;
      }

      if (!layer.GetPositionX().IsKnownZero() || !layer.GetPositionY().IsKnownZero())
      {
        return true;
      }

      return krys::SwitchOn(
        layer.GetSize(), [](const css::keywords::Contain &) { return true; },
        [](const css::keywords::Cover &) { return true; },
        [&](const style::BackgroundSize::LengthSize &size)
        {
          if (size.Width().IsPercentOrCalculated() || size.Height().IsPercentOrCalculated())
          {
            return true;
          }
          // If the image has neither an intrinsic width nor an intrinsic height, its size is determined as
          // for 'contain'.
          if ((size.Width().IsAuto() || size.Height().IsAuto()) && image->IsGeneratedImage())
          {
            return true;
          }
          return false;
        });
    }
  }

  bool RenderElement::ShouldRepaintForStyleDifference(style::Difference diff) const noexcept
  {
    if (diff == style::DifferenceResult::Repaint)
    {
      return true;
    }

    if (diff == style::DifferenceResult::RepaintIfText)
    {
      if (HasNonWhitespaceTextContent(*this))
      {
        return true;
      }
      for (auto &blockChild : ChildrenOfType<RenderBlock>(*this))
      {
        if (blockChild.IsAnonymousBlock() && HasNonWhitespaceTextContent(blockChild))
        {
          return true;
        }
      }
    }

    return false;
  }

  bool RenderElement::RepaintBeforeStyleChange(style::Difference diff, const RenderStyle &oldStyle,
                                               const RenderStyle &newStyle) noexcept
  {
    enum class RequiredRepaint
    {
      None,
      RendererOnly,
      RendererAndDescendantsRenderersWithLayers
    };
    auto shouldRepaintBeforeStyleChange = [&]() -> RequiredRepaint
    {
      if (!Parent())
      {
        // Can't resolve absolute coordinates.
        return RequiredRepaint::None;
      }

      if (Is<RenderLayerModelObject>(*this) && HasLayer())
      {
        if (diff == style::DifferenceResult::RepaintLayer)
        {
          return RequiredRepaint::RendererAndDescendantsRenderersWithLayers;
        }

        if (diff == style::DifferenceResult::Layout || diff == style::DifferenceResult::Overflow)
        {
          // Certain style changes require layer repaint, since the layer could end up being destroyed.
          auto layerMayGetDestroyed =
            oldStyle.GetPosition() != newStyle.GetPosition() || oldStyle.UsedZIndex() != newStyle.UsedZIndex()
            || oldStyle.GetClip() != newStyle.GetClip() || oldStyle.HasClip() != newStyle.HasClip()
            || oldStyle.HasOpacity() != newStyle.HasOpacity()
            || oldStyle.HasTransform() != newStyle.HasTransform()
            || oldStyle.HasFilter() != newStyle.HasFilter();
          if (layerMayGetDestroyed)
          {
            return RequiredRepaint::RendererAndDescendantsRenderersWithLayers;
          }
        }
      }

      if (ShouldRepaintForStyleDifference(diff))
      {
        return RequiredRepaint::RendererOnly;
      }

      if (newStyle.OutlineSize() < oldStyle.OutlineSize())
      {
        return RequiredRepaint::RendererOnly;
      }

      if (auto *modelObject = DynamicDowncast<RenderLayerModelObject>(*this))
      {
        // If we don't have a layer yet, but we are going to get one because of transform or opacity, then we
        // need to repaint the old position of the object.
        bool hasLayer = modelObject->HasLayer();
        bool willHaveLayer = newStyle.AffectsTransform() || newStyle.HasOpacity() || newStyle.HasFilter()
                             || newStyle.HasBackdropFilter();
        if (!hasLayer && willHaveLayer)
        {
          return RequiredRepaint::RendererOnly;
        }
      }

      // Note that RenderObject::setNeedsLayout issues setLayerNeedsFullRepaint on renderers with layers.
      if (Is<RenderBox>(*this))
      {
        if (oldStyle.GetPosition() != newStyle.GetPosition()
            && oldStyle.GetPosition() == PositionType::Static)
        {
          krys_debug_assert(diff == style::DifferenceResult::Layout);
          return RequiredRepaint::RendererOnly;
        }

        auto willBecomeHiddenSkippedContent = newStyle.UsedContentVisibility() == ContentVisibility::Hidden
                                              && oldStyle.UsedContentVisibility() != ContentVisibility::Hidden
                                              && oldStyle.UsedVisibility() == Visibility::Visible;
        if (willBecomeHiddenSkippedContent)
        {
          krys_debug_assert(diff == style::DifferenceResult::Layout);
          return RequiredRepaint::RendererOnly;
        }
      }

      if (diff > style::DifferenceResult::RepaintLayer
          && oldStyle.UsedVisibility() != newStyle.UsedVisibility())
      {
        if (auto *enclosingLayer = EnclosingLayer())
        {
          bool rendererWillBeHidden = newStyle.UsedVisibility() != Visibility::Visible;
          if (rendererWillBeHidden && enclosingLayer->HasVisibleContent()
              && (this == &enclosingLayer->Renderer()
                  || enclosingLayer->Renderer().Style().UsedVisibility() != Visibility::Visible))
          {
            return RequiredRepaint::RendererOnly;
          }
        }
      }

      if (diff == style::DifferenceResult::Layout && Parent()->GetWritingMode().IsBlockFlipped())
      {
        // FIXME: Repaint during (after) layout is currently broken for flipped writing modes in block
        // direction (mostly affecting vertical-rl) (see webkit.org/b/70762) This repaint call here ensures we
        // invalidate at least the current rect which should cover the non-moving type of cases.
        return RequiredRepaint::RendererOnly;
      }

      return RequiredRepaint::None;
    }();

    if (shouldRepaintBeforeStyleChange == RequiredRepaint::RendererAndDescendantsRenderersWithLayers)
    {
      krys_debug_assert(HasLayer());
      Downcast<RenderLayerModelObject>(*this).Layer()->RepaintIncludingDescendants();
      return true;
    }

    if (shouldRepaintBeforeStyleChange == RequiredRepaint::RendererOnly)
    {
      if (IsOutOfFlowPositioned() && Downcast<RenderLayerModelObject>(*this).Layer()->IsSelfPaintingLayer())
      {
        if (oldStyle.UsedVisibility() == Visibility::Hidden)
        {
          // Repaint on hidden renderer is a no-op.
          return false;
        }
        if (auto cachedClippedOverflowRect =
              Downcast<RenderLayerModelObject>(*this).Layer()->CachedClippedOverflowRect())
        {
          RepaintUsingContainer(ContainerForRepaint().Renderer, *cachedClippedOverflowRect);
          return true;
        }
      }
      Repaint();
      return true;
    }

    return false;
  }

  bool RenderElement::RepaintAfterLayoutIfNeeded(const RenderLayerModelObject *repaintContainer,
                                                 RequiresFullRepaint requiresFullRepaint,
                                                 const RepaintRects &oldRects,
                                                 const RepaintRects &newRects) noexcept
  {
    // FIXME: Nothing repaints while printing, once there's printing.

    auto oldClippedOverflowRect = oldRects.ClippedOverflowRect;
    auto newClippedOverflowRect = newRects.ClippedOverflowRect;
    bool haveOutlinesBoundsRects = oldRects.OutlineBoundsRect && newRects.OutlineBoundsRect;

    if (oldClippedOverflowRect.IsEmpty() && newClippedOverflowRect.IsEmpty())
    {
      return true;
    }

    auto mustRepaintBackgroundOrBorderOnSizeChange =
      [&](LayoutRect oldOutlineBounds, LayoutRect newOutlineBounds)
    {
      if (HasMask() && MustRepaintFillLayers(*this, Style().GetMaskLayers()))
      {
        return true;
      }

      if (Style().HasBorderRadius())
      {
        // If the border radius changed, repaints at style change time will take care of that.
        // This code is attempting to detect whether border-radius constraining based on box size
        // affects the radii, using the outlineBoundsRect as a proxy for the border box.
        auto oldShapeApproximation = BorderShape::ShapeForBorderRect(Style(), oldOutlineBounds);
        auto newShapeApproximation = BorderShape::ShapeForBorderRect(Style(), newOutlineBounds);
        if (oldShapeApproximation.Radii() != newShapeApproximation.Radii())
        {
          return true;
        }
      }

      // If we don't have a background/border/mask, then nothing to do.
      if (!HasVisibleBoxDecorations())
      {
        return false;
      }

      if (MustRepaintFillLayers(*this, Style().GetBackgroundLayers()))
      {
        return true;
      }

      // Our fill layers are ok. Let's check border.
      if (Style().HasBorder() && BorderImageIsLoadedAndCanBeRendered())
      {
        return true;
      }

      return false;
    };

    auto fullRepaint = [&]()
    {
      if (requiresFullRepaint == RequiresFullRepaint::Yes)
      {
        return true;
      }

      if (oldClippedOverflowRect.IsEmpty() || newClippedOverflowRect.IsEmpty())
      {
        return true;
      }

      if (!oldClippedOverflowRect.Intersects(newClippedOverflowRect))
      {
        return true;
      }

      if (!haveOutlinesBoundsRects)
      {
        return false;
      }

      // If our outline bounds rect moved, we have to repaint everything.
      if (oldRects.OutlineBoundsRect->Location() != newRects.OutlineBoundsRect->Location())
      {
        return true;
      }

      // If our outline bounds rect resized (as a proxy for a border box resize),
      // we have to repaint if we paint content that scales with the size.
      if (oldRects.OutlineBoundsRect->Size() != newRects.OutlineBoundsRect->Size()
          && mustRepaintBackgroundOrBorderOnSizeChange(*oldRects.OutlineBoundsRect,
                                                       *newRects.OutlineBoundsRect))
      {
        return true;
      }

      return false;
    }();

    if (!repaintContainer)
    {
      repaintContainer = &View();
    }

    if (fullRepaint)
    {
      if (newClippedOverflowRect.Contains(oldClippedOverflowRect))
      {
        RepaintUsingContainer(repaintContainer, newClippedOverflowRect);
      }
      else if (oldClippedOverflowRect.Contains(newClippedOverflowRect))
      {
        RepaintUsingContainer(repaintContainer, oldClippedOverflowRect);
      }
      else
      {
        RepaintUsingContainer(repaintContainer, oldClippedOverflowRect);
        RepaintUsingContainer(repaintContainer, newClippedOverflowRect);
      }
      return true;
    }

    if (oldRects == newRects)
    {
      return false;
    }

    LayoutUnit deltaLeft = newClippedOverflowRect.X() - oldClippedOverflowRect.X();
    if (deltaLeft > 0)
    {
      RepaintUsingContainer(repaintContainer,
                            LayoutRect(oldClippedOverflowRect.X(), oldClippedOverflowRect.Y(), deltaLeft,
                                       oldClippedOverflowRect.Height()));
    }
    else if (deltaLeft < 0)
    {
      RepaintUsingContainer(repaintContainer,
                            LayoutRect(newClippedOverflowRect.X(), newClippedOverflowRect.Y(), -deltaLeft,
                                       newClippedOverflowRect.Height()));
    }

    LayoutUnit deltaRight = newClippedOverflowRect.MaxX() - oldClippedOverflowRect.MaxX();
    if (deltaRight > 0)
    {
      RepaintUsingContainer(repaintContainer,
                            LayoutRect(oldClippedOverflowRect.MaxX(), newClippedOverflowRect.Y(), deltaRight,
                                       newClippedOverflowRect.Height()));
    }
    else if (deltaRight < 0)
    {
      RepaintUsingContainer(repaintContainer,
                            LayoutRect(newClippedOverflowRect.MaxX(), oldClippedOverflowRect.Y(), -deltaRight,
                                       oldClippedOverflowRect.Height()));
    }

    LayoutUnit deltaTop = newClippedOverflowRect.Y() - oldClippedOverflowRect.Y();
    if (deltaTop > 0)
    {
      RepaintUsingContainer(repaintContainer,
                            LayoutRect(oldClippedOverflowRect.X(), oldClippedOverflowRect.Y(),
                                       oldClippedOverflowRect.Width(), deltaTop));
    }
    else if (deltaTop < 0)
    {
      RepaintUsingContainer(repaintContainer,
                            LayoutRect(newClippedOverflowRect.X(), newClippedOverflowRect.Y(),
                                       newClippedOverflowRect.Width(), -deltaTop));
    }

    LayoutUnit deltaBottom = newClippedOverflowRect.MaxY() - oldClippedOverflowRect.MaxY();
    if (deltaBottom > 0)
    {
      RepaintUsingContainer(repaintContainer,
                            LayoutRect(newClippedOverflowRect.X(), oldClippedOverflowRect.MaxY(),
                                       newClippedOverflowRect.Width(), deltaBottom));
    }
    else if (deltaBottom < 0)
    {
      RepaintUsingContainer(repaintContainer,
                            LayoutRect(oldClippedOverflowRect.X(), newClippedOverflowRect.MaxY(),
                                       oldClippedOverflowRect.Width(), -deltaBottom));
    }

    if (!haveOutlinesBoundsRects || *oldRects.OutlineBoundsRect == *newRects.OutlineBoundsRect)
    {
      return false;
    }

    auto oldOutlineBoundsRect = *oldRects.OutlineBoundsRect;
    auto newOutlineBoundsRect = *newRects.OutlineBoundsRect;

    // Repainting the delta of the old and new clipped overflow rects is not sufficient when the box has
    // outlines border and shadows, because a size change has to repaint those areas affected by such
    // decorations. It's not really correct to do math here with oldOutlineBoundsRect/newOutlineBoundsRect and
    // local shadow/radius values, since oldOutlineBoundsRect/newOutlineBoundsRect are in the coordinate space
    // of the repaint container, and have been mapped through ancestor transforms.

    const RenderStyle &outlineStyle = OutlineStyleForRepaint();
    const auto &style = Style();
    auto outlineWidth = LayoutUnit {outlineStyle.OutlineSize()};
    auto insetShadowExtent = style::ShadowInsetExtent(style.GetBoxShadow(), style.UsedZoomForLength());
    auto sizeDelta =
      LayoutSize {AbsoluteValue(newOutlineBoundsRect.Width() - oldOutlineBoundsRect.Width()),
                  AbsoluteValue(newOutlineBoundsRect.Height() - oldOutlineBoundsRect.Height())};
    if (sizeDelta.Width())
    {
      auto [shadowLeft, shadowRight] =
        style::ShadowHorizontalExtent(style.GetBoxShadow(), style.UsedZoomForLength());

      auto insetExtent = [&]
      {
        // Inset "content" is inside the border box (e.g. border, negative outline and box shadow).
        auto borderRightExtent = [&]() -> LayoutUnit
        {
          auto *renderBox = DynamicDowncast<RenderBox>(*this);
          if (!renderBox)
          {
            return {};
          }
          auto borderBoxWidth = renderBox->Width();
          return std::max({
            renderBox->BorderRight(),
            style::Evaluate<LayoutUnit>(style.BorderRadii().TopRight().Width(), borderBoxWidth,
                                        style::ZoomNeeded {}),
            style::Evaluate<LayoutUnit>(style.BorderRadii().BottomRight().Width(), borderBoxWidth,
                                        style::ZoomNeeded {}),
          });
        };
        auto outlineRightInsetExtent = [&]() -> LayoutUnit
        {
          auto offset = style::Evaluate<LayoutUnit>(outlineStyle.OutlineOffset(), style::ZoomNeeded {});
          return offset < 0 ? -offset : 0_lu;
        };
        auto boxShadowRightInsetExtent = [&]() -> LayoutUnit
        {
          // Turn negative box shadow offset into inset.
          auto inset = std::min(insetShadowExtent.Right(), shadowLeft);
          // Clip inset shadow at the clipped overflow rect. We would never paint outside.
          return inset < 0 ? std::min(-inset, std::min(newClippedOverflowRect.Width(),
                                                       oldClippedOverflowRect.Width()))
                           : 0_lu;
        };
        // Outline starts at the border box while box shadow starts at the padding box.
        return std::max(outlineRightInsetExtent(), borderRightExtent() + boxShadowRightInsetExtent());
      };
      auto outsetExtent = [&]
      {
        // Outset "content" is outside of the border box (e.g. regular outline and box shadow).
        return std::max(outlineWidth, shadowRight);
      };
      auto decorationRightExtent = insetExtent() + outsetExtent();
      // Both inset and outset "decorations" are within the "outline and box shadow" box.
      auto decorationLeft = newOutlineBoundsRect.X()
                            + std::min(newOutlineBoundsRect.Width(), oldOutlineBoundsRect.Width())
                            - decorationRightExtent;
      auto clippedBoundsRight = std::min(newClippedOverflowRect.MaxX(), oldClippedOverflowRect.MaxX());
      auto damageExtentWithinClippedOverflow = clippedBoundsRight - decorationLeft;
      if (damageExtentWithinClippedOverflow > 0)
      {
        damageExtentWithinClippedOverflow =
          std::min<LayoutUnit>(sizeDelta.Width() + decorationRightExtent, damageExtentWithinClippedOverflow);
        auto damagedRect =
          LayoutRect {decorationLeft, newOutlineBoundsRect.Y(), damageExtentWithinClippedOverflow,
                      std::max(newOutlineBoundsRect.Height(), oldOutlineBoundsRect.Height())};
        RepaintUsingContainer(repaintContainer, damagedRect);
      }
    }
    if (sizeDelta.Height())
    {
      auto [shadowTop, shadowBottom] =
        style::ShadowVerticalExtent(style.GetBoxShadow(), style.UsedZoomForLength());

      auto insetExtent = [&]
      {
        // Inset "content" is inside the border box (e.g. border, negative outline and box shadow).
        auto borderBottomExtent = [&]() -> LayoutUnit
        {
          auto *renderBox = DynamicDowncast<RenderBox>(*this);
          if (!renderBox)
          {
            return {};
          }
          auto borderBoxHeight = renderBox->Height();
          return std::max({
            renderBox->BorderBottom(),
            style::Evaluate<LayoutUnit>(style.BorderRadii().BottomLeft().Height(), borderBoxHeight,
                                        style::ZoomNeeded {}),
            style::Evaluate<LayoutUnit>(style.BorderRadii().BottomRight().Height(), borderBoxHeight,
                                        style::ZoomNeeded {}),
          });
        };
        auto outlineBottomInsetExtent = [&]() -> LayoutUnit
        {
          auto offset = style::Evaluate<LayoutUnit>(outlineStyle.OutlineOffset(), style::ZoomNeeded {});
          return offset < 0 ? -offset : 0_lu;
        };
        auto boxShadowBottomInsetExtent = [&]() -> LayoutUnit
        {
          // Turn negative box shadow offset into inset.
          auto inset = std::min(insetShadowExtent.Bottom(), shadowTop);
          // Clip inset shadow at the clipped overflow rect. We would never paint outside.
          return inset < 0 ? std::min(-inset, std::min(newClippedOverflowRect.Height(),
                                                       oldClippedOverflowRect.Height()))
                           : 0_lu;
        };
        // Outline starts at the border box while box shadow starts at the padding box.
        return std::max(outlineBottomInsetExtent(), borderBottomExtent() + boxShadowBottomInsetExtent());
      };
      auto outsetExtent = [&]
      {
        // Outset "content" is outside of the border box (e.g. regular outline and box shadow).
        return std::max(outlineWidth, shadowBottom);
      };
      auto decorationBottomExtent = insetExtent() + outsetExtent();
      // Both inset and outset "decorations" are within the "outline and box shadow" box.
      auto decorationTop =
        std::min(newOutlineBoundsRect.MaxY(), oldOutlineBoundsRect.MaxY()) - decorationBottomExtent;
      auto clippedBoundsBottom = std::min(newClippedOverflowRect.MaxY(), oldClippedOverflowRect.MaxY());
      auto damageExtentWithinClippedOverflow = clippedBoundsBottom - decorationTop;
      if (damageExtentWithinClippedOverflow > 0)
      {
        damageExtentWithinClippedOverflow = std::min<LayoutUnit>(sizeDelta.Height() + decorationBottomExtent,
                                                                 damageExtentWithinClippedOverflow);
        auto damagedRect = LayoutRect {newOutlineBoundsRect.X(), decorationTop,
                                       std::max(newOutlineBoundsRect.Width(), oldOutlineBoundsRect.Width()),
                                       damageExtentWithinClippedOverflow};
        RepaintUsingContainer(repaintContainer, damagedRect);
      }
    }
    return false;
  }

  bool RenderElement::IsInsideEntirelyHiddenLayer() const noexcept
  {
    // FIXME: Layers inside hidden SVG containers and resources, once there's SVG.
    return Style().UsedVisibility() != Visibility::Visible && !EnclosingLayer()->HasVisibleContent();
  }

  bool RenderElement::CheckForRepaintDuringLayout() const noexcept
  {
    return EverHadLayout() && !HasSelfPaintingLayer()
           && !GetDocument().View()->LayoutContext().NeedsFullRepaint();
  }

  void RenderElement::IssueRepaintForOutlineAuto(float outlineSize) noexcept
  {
    auto focusRingRects =
      OutlinePainter::CollectFocusRingRects(*this, LayoutPoint(), ContainerForRepaint().Renderer);

    LayoutRect repaintRect;
    for (auto rect : focusRingRects)
    {
      rect.Inflate(LayoutUnit {outlineSize});
      repaintRect.Unite(rect);
    }
    RepaintRectangle(repaintRect);
  }

  void RenderElement::UpdateOutlineAutoAncestor(bool hasOutlineAuto) noexcept
  {
    // FIXME: Column spanner placeholders, once there are multi-column flows.

    for (auto &child : ChildrenOfType<RenderObject>(*this))
    {
      if (hasOutlineAuto == child.HasOutlineAutoAncestor())
      {
        continue;
      }
      child.SetHasOutlineAutoAncestor(hasOutlineAuto);
      bool childHasOutlineAuto = child.OutlineStyleForRepaint().GetOutlineStyle() == OutlineStyle::Auto;
      if (childHasOutlineAuto)
      {
        continue;
      }
      if (auto *element = DynamicDowncast<RenderElement>(child))
      {
        element->UpdateOutlineAutoAncestor(hasOutlineAuto);
      }
    }
    // FIXME: Continuations, once there are any.
  }

  void RenderElement::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    auto *oldStyle = HasInitializedStyle() ? &Style() : nullptr;
    if (oldStyle)
    {
      if (diff >= style::DifferenceResult::Repaint && LayoutBox())
      {
        // FIXME: It is highly unlikely that a style mutation has effect on both the formatting context the
        // box lives in and the one it establishes but calling only one would require to come up with a list
        // of properties that only affects one or the other.
        if (auto *inlineFormattingContextRoot = DynamicDowncast<RenderBlockFlow>(*this);
            inlineFormattingContextRoot && inlineFormattingContextRoot->InlineLayout())
        {
          (void)inlineFormattingContextRoot->InlineLayout()->RootStyleWillChange(*inlineFormattingContextRoot,
                                                                                 newStyle);
        }
        if (auto *lineLayout = layout_integration::LineLayout::Containing(*this))
        {
          (void)lineLayout->StyleWillChange(*this, newStyle, diff);
        }
      }
      // FIXME: The stacking context's z-order lists, rendering-dependent regions, the accessibility cache,
      // the enclosing layer's visible content status and event region and the fragmented flow, once there are
      // any.

      if (IsFloating() && _style.Floating() != newStyle.Floating())
      {
        // For changes in float styles, we need to conceivably remove ourselves
        // from the floating objects list.
        Downcast<RenderBox>(*this).RemoveFloatingOrOutOfFlowChildFromBlockLists();
      }
      else if (IsOutOfFlowPositioned() && _style.GetPosition() != newStyle.GetPosition())
      {
        // For changes in positioning styles, we need to conceivably remove ourselves
        // from the positioned objects list.
        Downcast<RenderBox>(*this).RemoveFloatingOrOutOfFlowChildFromBlockLists();
      }

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

    style::LoadPendingResources(*resolvedStyle->Style, GetDocument(), element.get());

    return krys::move(resolvedStyle->Style);
  }

  template <typename TFillLayers>
  void RenderElement::UpdateFillImages(const TFillLayers *oldLayers, const TFillLayers *newLayers) noexcept
  {
    auto fillImagesAreIdentical = [](const TFillLayers *layers1, const TFillLayers *layers2) -> bool
    {
      if (layers1 == layers2)
      {
        return true;
      }
      if (!layers1 || !layers2)
      {
        return false;
      }
      if (layers1->UsedLength() != layers2->UsedLength())
      {
        return false;
      }

      auto values1 = layers1->UsedValues();
      auto values2 = layers2->UsedValues();
      auto it2 = values2.begin();
      for (auto it1 = values1.begin(); it1 != values1.end(); ++it1, ++it2)
      {
        if (it1->GetImage() != it2->GetImage())
        {
          return false;
        }
        if (auto image = it1->GetImage().TryStyleImage();
            image && (image->ErrorOccurred() || !image->HasImage() || image->UsesDataProtocol()))
        {
          return false;
        }
      }
      return true;
    };

    auto isRegisteredWithNewFillImages = [&]() -> bool
    {
      if (!newLayers)
      {
        return true;
      }
      for (auto &layer : newLayers->UsedValues())
      {
        if (auto image = layer.GetImage().TryStyleImage(); image && !image->HasClient(*this))
        {
          return false;
        }
      }
      return true;
    };

    // If images have the same characteristics and this element is already registered as a
    // client to the new images, there is nothing to do.
    if (fillImagesAreIdentical(oldLayers, newLayers) && isRegisteredWithNewFillImages())
    {
      return;
    }

    // Add before removing, to avoid removing all clients of an image that is in both sets.
    if (newLayers)
    {
      for (auto &layer : newLayers->UsedValues())
      {
        if (auto image = layer.GetImage().TryStyleImage())
        {
          image->AddClient(*this);
        }
      }
    }
    if (oldLayers)
    {
      for (auto &layer : oldLayers->UsedValues())
      {
        if (auto image = layer.GetImage().TryStyleImage())
        {
          image->RemoveClient(*this);
        }
      }
    }
  }

  void RenderElement::UpdateImage(StyleImage *oldImage, StyleImage *newImage) noexcept
  {
    if (oldImage == newImage)
    {
      return;
    }
    if (oldImage)
    {
      oldImage->RemoveClient(*this);
    }
    if (newImage)
    {
      newImage->AddClient(*this);
    }
  }

  void RenderElement::UpdateShapeImage(const style::ShapeOutside *oldShapeValue,
                                       const style::ShapeOutside *newShapeValue) noexcept
  {
    if (oldShapeValue || newShapeValue)
    {
      UpdateImage(oldShapeValue ? oldShapeValue->GetImage().get() : nullptr,
                  newShapeValue ? newShapeValue->GetImage().get() : nullptr);
    }
  }

  void RenderElement::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    auto registerImages = [this](const RenderStyle *style, const RenderStyle *oldStyle)
    {
      if (!style && !oldStyle)
      {
        return;
      }
      if ((style && style::HasImageInAnyLayer(style->GetBackgroundLayers()))
          || (oldStyle && style::HasImageInAnyLayer(oldStyle->GetBackgroundLayers())))
      {
        UpdateFillImages(oldStyle ? &oldStyle->GetBackgroundLayers() : nullptr,
                         style ? &style->GetBackgroundLayers() : nullptr);
      }
      if ((style && style::HasImageInAnyLayer(style->GetMaskLayers()))
          || (oldStyle && style::HasImageInAnyLayer(oldStyle->GetMaskLayers())))
      {
        UpdateFillImages(oldStyle ? &oldStyle->GetMaskLayers() : nullptr,
                         style ? &style->GetMaskLayers() : nullptr);
      }
      UpdateImage(oldStyle ? oldStyle->GetBorderImageSource().TryStyleImage().get() : nullptr,
                  style ? style->GetBorderImageSource().TryStyleImage().get() : nullptr);
      UpdateImage(oldStyle ? oldStyle->GetMaskBorderSource().TryStyleImage().get() : nullptr,
                  style ? style->GetMaskBorderSource().TryStyleImage().get() : nullptr);
      UpdateShapeImage(oldStyle ? &oldStyle->GetShapeOutside() : nullptr,
                       style ? &style->GetShapeOutside() : nullptr);
    };

    registerImages(&Style(), oldStyle);

    // Are there other pseudo-elements that need the resources to be registered?
    registerImages(Style().GetCachedPseudoStyle({PseudoElementType::FirstLine}),
                   oldStyle ? oldStyle->GetCachedPseudoStyle({PseudoElementType::FirstLine}) : nullptr);

    // FIXME: SVG resources, once there are any.

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

    if (IsOutOfFlowPositioned() && oldStyle
        && oldStyle->IsOriginalDisplayBlockType() != Style().IsOriginalDisplayBlockType())
    {
      if (auto *ancestor = RenderObject::ContainingBlockForPositionType(PositionType::Static, *this))
      {
        ancestor->SetNeedsLayout();
        ancestor->SetOutOfFlowChildNeedsStaticPositionLayout();
      }
    }

    if (oldStyle && oldStyle->GetCursor() != _style.GetCursor())
    {
      if (auto *frame = GetDocument().Frame())
      {
        frame->GetEventHandler().ScheduleCursorUpdate();
      }
    }

    bool hadOutlineAuto = oldStyle && oldStyle->GetOutlineStyle() == OutlineStyle::Auto;
    bool hasOutlineAuto = OutlineStyleForRepaint().GetOutlineStyle() == OutlineStyle::Auto;
    if (hasOutlineAuto != hadOutlineAuto)
    {
      UpdateOutlineAutoAncestor(hasOutlineAuto);
      IssueRepaintForOutlineAuto(hasOutlineAuto ? OutlineStyleForRepaint().OutlineSize()
                                                : oldStyle->OutlineSize());
    }

    // FIXME: Scroll anchoring, once there is any.

    // FIXME: First line change on the block comes in as equal on inline boxes.
    auto needsLayoutBoxStyleUpdate =
      (diff >= style::DifferenceResult::Repaint || (Is<RenderInline>(*this) && &Style() != &FirstLineStyle()))
      && LayoutBox();
    if (needsLayoutBoxStyleUpdate)
    {
      layout_integration::LineLayout::UpdateStyle(*this);
    }
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

      auto newStyle = [&]
      {
        auto display = elementChild.Style().Display();
        if (display == DisplayType::RubyBase || display == DisplayType::Ruby)
        {
          return CreateAnonymousStyleForRuby(Style(), display);
        }
        return RenderStyle::CreateAnonymousStyleWithDisplay(Style(), display);
      }();

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

  bool RenderElement::HasBackground() const noexcept
  {
    return Style().HasBackground();
  }

  bool RenderElement::BorderImageIsLoadedAndCanBeRendered() const noexcept
  {
    krys_debug_assert(Style().HasBorder());

    auto borderImage = Style().GetBorderImage().Source.TryStyleImage();
    return borderImage && borderImage->CanRender(this, Style().UsedZoom()) && borderImage->IsLoaded(this);
  }

  bool RenderElement::HasOutline() const noexcept
  {
    return Style().HasOutline() || HasOutlineAnnotation();
  }

  bool RenderElement::HasOutlineAnnotation() const noexcept
  {
    // Nothing prints: only an explicit request annotates links.
    const auto *element = GetElement();
    return element && css::IsLink(*element)
           && View().FrameView().GetPaintBehavior().Contains(PaintBehavior::AnnotateLinks);
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

  layout::ElementBox *RenderElement::LayoutBox() noexcept
  {
    return Downcast<layout::ElementBox>(RenderObject::LayoutBox());
  }

  const layout::ElementBox *RenderElement::LayoutBox() const noexcept
  {
    return Downcast<layout::ElementBox>(RenderObject::LayoutBox());
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

  bool RenderElement::IsVisibleIgnoringGeometry() const noexcept
  {
    // FIXME: Documents whose active DOM objects are suspended, once they can be.
    if (Style().UsedVisibility() != Visibility::Visible)
    {
      return false;
    }
    // FIXME: Off-screen frame views (isOffscreen), once a view can be off screen.
    return true;
  }

  bool RenderElement::IsVisibleInDocumentRect(const IntRect &documentRect) const noexcept
  {
    if (!IsVisibleIgnoringGeometry())
    {
      return false;
    }

    // Use background rect if we are the root or if we are the body and the background is propagated to the
    // root.
    // FIXME: This is overly conservative as the image may not be a background-image, in which case it will
    // not be propagated to the root. At this point, we unfortunately don't have access to the image anymore
    // so we can no longer check if it is a background image.
    auto rendererHasBackground = [](const RenderElement *renderer)
    {
      return renderer && renderer->HasBackground();
    };
    auto *documentElement = GetDocument().DocumentElement().get();
    auto backgroundIsPaintedByRoot =
      IsDocumentElementRenderer()
      || (IsBody() && !rendererHasBackground(documentElement ? documentElement->Renderer() : nullptr));

    LayoutRect backgroundPaintingRect;
    if (backgroundIsPaintedByRoot)
    {
      backgroundPaintingRect = View().BackgroundRect();
    }
    else if (auto *box = DynamicDowncast<RenderBox>(*this))
    {
      backgroundPaintingRect = box->AbsoluteClippedOverflowRectForRepaint();
    }
    else
    {
      backgroundPaintingRect = LayoutRect(AbsoluteBoundingBoxRect());
    }
    return documentRect.Intersects(EnclosingIntRect(backgroundPaintingRect));
  }

  bool RenderElement::IsVisibleInViewport() const noexcept
  {
    return IsVisibleInDocumentRect(View().FrameView().VisibleContentRect());
  }

  VisibleInViewportState RenderElement::ImageFrameAvailable(CachedImage &image,
                                                            ImageAnimatingState animatingState,
                                                            const IntRect *changeRect) noexcept
  {
    bool isVisible = IsVisibleInViewport();

    if (!isVisible && animatingState == ImageAnimatingState::Yes)
    {
      View().AddRendererWithPausedImageAnimations(*this, image);
    }

    // Static images should repaint even if they are outside the viewport rectangle
    // because they should be inside the TileCoverageRect.
    if (isVisible || animatingState == ImageAnimatingState::No)
    {
      ImageChanged(&image, changeRect);
    }

    return isVisible ? VisibleInViewportState::Yes : VisibleInViewportState::No;
  }

  VisibleInViewportState RenderElement::ImageVisibleInViewport(const dom::Document &document) const noexcept
  {
    if (&GetDocument() != &document)
    {
      return VisibleInViewportState::No;
    }

    return IsVisibleInViewport() ? VisibleInViewportState::Yes : VisibleInViewportState::No;
  }

  bool RenderElement::AllowsAnimation() const noexcept
  {
    // FIXME: An image element's own animation control (HTMLImageElement::allowsAnimation) and the page's
    // (imageAnimationEnabled), once there are animation controls.
    return true;
  }

  void RenderElement::DidRemoveCachedImageClient(CachedImage &cachedImage) noexcept
  {
    if (HasPausedImageAnimations())
    {
      View().RemoveRendererWithPausedImageAnimations(*this, cachedImage);
    }
  }

  bool RenderElement::RepaintForPausedImageAnimationsIfNeeded(const IntRect &visibleRect,
                                                              CachedImage &cachedImage) noexcept
  {
    krys_debug_assert(_hasPausedImageAnimations);
    if (!AllowsAnimation() || !IsVisibleInDocumentRect(visibleRect))
    {
      return false;
    }

    Repaint();

    // FIXME: SVG images schedule their start (scheduleStartAnimation), once there are SVG images.
    if (auto *image = cachedImage.GetImage())
    {
      image->StartAnimation();
    }

    // FIXME: Tell compositing layers the image changed (contentChanged), once there are layers.
    return true;
  }

  bool RenderElement::HasShapeOutside() const noexcept
  {
    return !Style().GetShapeOutside().IsNone();
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

    RenderObject::WillBeDestroyed();

    // FIXME: The subtree layout root, outlines and the other upstream clean-up, once there are any.

    auto unregisterImage = [this](StyleImage *image)
    {
      if (image)
      {
        image->RemoveClient(*this);
      }
    };

    auto unregisterImages = [&](const RenderStyle &style)
    {
      for (auto &backgroundLayer : style.GetBackgroundLayers().UsedValues())
      {
        unregisterImage(backgroundLayer.GetImage().TryStyleImage().get());
      }
      for (auto &maskLayer : style.GetMaskLayers().UsedValues())
      {
        unregisterImage(maskLayer.GetImage().TryStyleImage().get());
      }
      unregisterImage(style.GetBorderImageSource().TryStyleImage().get());
      unregisterImage(style.GetMaskBorderSource().TryStyleImage().get());
      unregisterImage(style.GetShapeOutside().GetImage().get());
    };

    if (HasInitializedStyle())
    {
      unregisterImages(_style);

      if (auto *firstLineStyle = Style().GetCachedPseudoStyle({PseudoElementType::FirstLine}))
      {
        unregisterImages(*firstLineStyle);
      }
    }

    if (_hasPausedImageAnimations)
    {
      View().RemoveRendererWithPausedImageAnimations(*this);
    }
  }

#pragma region Painting

  namespace
  {
    void PaintPhaseOf(RenderElement &element, PaintPhase phase, PaintInfo &paintInfo,
                      const LayoutPoint &childPoint) noexcept
    {
      paintInfo.Phase = phase;
      element.Paint(paintInfo, childPoint);
    }
  }

  void RenderElement::PaintOutline(PaintInfo &paintInfo, const LayoutRect &paintRect) noexcept
  {
    if (paintInfo.Context().PaintingDisabled())
    {
      return;
    }

    if (!HasOutline())
    {
      return;
    }

    OutlinePainter {paintInfo}.PaintOutline(*this, paintRect);
  }

  void RenderElement::PaintAsInlineBlock(PaintInfo &paintInfo, const LayoutPoint &childPoint) noexcept
  {
    // Paint all phases atomically, as though the element established its own stacking context.
    // (See Appendix E.2, section 6.4 on inline block/table/replaced elements in the CSS2.1 specification.)
    // This is also used by other elements (e.g. flex items and grid items).
    PaintPhase paintPhaseToUse = IsExcludedAndPlacedInBorder() ? paintInfo.Phase : PaintPhase::Foreground;
    if (paintInfo.Phase == PaintPhase::Selection || paintInfo.Phase == PaintPhase::EventRegion
        || paintInfo.Phase == PaintPhase::TextClip || paintInfo.Phase == PaintPhase::Accessibility)
    {
      Paint(paintInfo, childPoint);
    }
    else if (paintInfo.Phase == paintPhaseToUse)
    {
      PaintPhaseOf(*this, PaintPhase::BlockBackground, paintInfo, childPoint);
      PaintPhaseOf(*this, PaintPhase::ChildBlockBackgrounds, paintInfo, childPoint);
      PaintPhaseOf(*this, PaintPhase::Float, paintInfo, childPoint);
      PaintPhaseOf(*this, PaintPhase::Foreground, paintInfo, childPoint);
      PaintPhaseOf(*this, PaintPhase::Outline, paintInfo, childPoint);

      // Reset |paintInfo| to the original phase.
      paintInfo.Phase = paintPhaseToUse;
    }
  }

#pragma endregion

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

  void RenderElement::SetNeedsOutOfFlowMovementLayout(const RenderStyle *oldStyle) noexcept
  {
    krys_debug_assert(!IsSetNeedsLayoutForbidden());
    if (NeedsOutOfFlowMovementLayout())
    {
      return;
    }
    SetNeedsOutOfFlowMovementLayoutBit(true);
    ScheduleLayout(MarkContainingBlocksForLayout());
    if (HasLayer())
    {
      if (oldStyle && style::DifferenceRequiresLayerRepaint(Style(), *oldStyle, false))
      {
        SetLayerNeedsFullRepaint();
      }
      else
      {
        SetLayerNeedsFullRepaintForOutOfFlowMovementLayout();
      }
    }
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
    if (HasLayer())
    {
      SetLayerNeedsFullRepaint();
    }
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

  namespace
  {
    KRYS_NODISCARD bool GetLeadingCorner(const RenderElement &renderer, FloatPoint &point,
                                         bool &insideFixed) noexcept
    {
      // FIXME: SVG renderers' stroke bounding boxes, once there's SVG.
      if (!renderer.IsInline() || renderer.IsBlockLevelReplacedOrAtomicInline())
      {
        point = renderer.LocalToAbsolute(FloatPoint(), MapCoordinatesMode::UseTransforms, &insideFixed);
        return true;
      }

      // find the next text/image child, to get a position
      const RenderObject *o = &renderer;
      while (o)
      {
        const auto *p = o;
        if (auto *child = o->FirstChildSlow())
        {
          o = child;
        }
        else if (o->NextSibling())
        {
          o = o->NextSibling();
        }
        else
        {
          const RenderObject *next = nullptr;
          while (!next && o->Parent())
          {
            o = o->Parent();
            next = o->NextSibling();
          }
          o = next;

          if (!o)
          {
            break;
          }
        }
        krys_debug_assert(o);

        if (!o->IsInline() || o->IsBlockLevelReplacedOrAtomicInline())
        {
          point = o->LocalToAbsolute(FloatPoint(), MapCoordinatesMode::UseTransforms, &insideFixed);
          return true;
        }

        auto *textRenderer = DynamicDowncast<RenderText>(*o);
        if (p->GetNode() && p->GetNode() == renderer.GetElement() && textRenderer
            && !inline_iterator::LineLeftmostTextBoxFor(*textRenderer))
        {
          // do nothing - skip unrendered whitespace that is a child or next sibling of the anchor
        }
        else if (textRenderer || o->IsBlockLevelReplacedOrAtomicInline())
        {
          point = FloatPoint();
          if (textRenderer)
          {
            if (auto run = inline_iterator::LineLeftmostTextBoxFor(*textRenderer))
            {
              point.Move(static_cast<float>(textRenderer->LinesBoundingBox().X()),
                         run->GetLineBox()->ContentLogicalTop());
            }
          }
          else if (auto *box = DynamicDowncast<RenderBox>(*o))
          {
            point.MoveBy(FloatPoint(box->Location()));
          }
          point = o->Container()->LocalToAbsolute(point, MapCoordinatesMode::UseTransforms, &insideFixed);
          return true;
        }
      }

      // If the target doesn't have any children or siblings that could be used to calculate the scroll
      // position, we must be at the end of the document. Scroll to the bottom.
      // FIXME: who said anything about scrolling?
      if (!o && renderer.GetDocument().View())
      {
        point = FloatPoint(0, static_cast<float>(renderer.View().LayoutOverflowRect().MaxY().ToInt()));
        return true;
      }
      return false;
    }

    KRYS_NODISCARD bool GetTrailingCorner(const RenderElement &renderer, FloatPoint &point,
                                          bool &insideFixed) noexcept
    {
      // FIXME: SVG renderers' stroke bounding boxes, once there's SVG.
      if (!renderer.IsInline() || renderer.IsBlockLevelReplacedOrAtomicInline())
      {
        point = renderer.LocalToAbsolute(FloatPoint(Downcast<RenderBox>(renderer).Size()),
                                         MapCoordinatesMode::UseTransforms, &insideFixed);
        return true;
      }

      // find the last text/image child, to get a position
      const RenderObject *o = &renderer;
      while (o)
      {
        if (auto *child = o->LastChildSlow())
        {
          o = child;
        }
        else if (o->PreviousSibling())
        {
          o = o->PreviousSibling();
        }
        else
        {
          const RenderObject *prev = nullptr;
          while (!prev)
          {
            o = o->Parent();
            if (!o)
            {
              return false;
            }
            prev = o->PreviousSibling();
          }
          o = prev;
        }
        krys_debug_assert(o);
        if (auto *textRenderer = DynamicDowncast<RenderText>(*o);
            textRenderer || o->IsBlockLevelReplacedOrAtomicInline())
        {
          point = FloatPoint();
          if (textRenderer)
          {
            auto linesBox = LayoutRect(textRenderer->LinesBoundingBox());
            if (!linesBox.MaxX() && !linesBox.MaxY())
            {
              continue;
            }
            point.MoveBy(FloatPoint(linesBox.MaxXMaxYCorner()));
          }
          else
          {
            point.MoveBy(FloatPoint(Downcast<RenderBox>(*o).FrameRect().MaxXMaxYCorner()));
          }
          point = o->Container()->LocalToAbsolute(point, MapCoordinatesMode::UseTransforms, &insideFixed);
          return true;
        }
      }
      return true;
    }
  }

  LayoutRect RenderElement::AbsoluteAnchorRect(bool *insideFixed) const noexcept
  {
    FloatPoint leading;
    FloatPoint trailing;
    bool leadingInFixed = false;
    bool trailingInFixed = false;
    (void)GetLeadingCorner(*this, leading, leadingInFixed);
    (void)GetTrailingCorner(*this, trailing, trailingInFixed);

    auto upperLeft = leading;
    auto lowerRight = trailing;

    // Vertical writing modes might mean the leading point is not in the top left
    if (!IsInline() || IsBlockLevelReplacedOrAtomicInline())
    {
      upperLeft = FloatPoint(std::min(leading.X(), trailing.X()), std::min(leading.Y(), trailing.Y()));
      lowerRight = FloatPoint(std::max(leading.X(), trailing.X()), std::max(leading.Y(), trailing.Y()));
    } // Otherwise, it's not obvious what to do.

    if (insideFixed)
    {
      // For now, just look at the leading corner. Handling one inside fixed and one not would be tricky.
      *insideFixed = leadingInFixed;
    }

    return EnclosingLayoutRect(FloatRect(upperLeft, lowerRight.ExpandedTo(upperLeft) - upperLeft));
  }

  Pair<LayoutRect, LayoutRect>
    RenderElement::AbsoluteAnchorRectWithScrollMargin(bool *insideFixed) const noexcept
  {
    auto anchorRect = AbsoluteAnchorRect(insideFixed);

    const auto &scrollMarginBox = Style().GetScrollMarginBox();
    // The scroll snap specification says that the scroll-margin should be applied in the
    // coordinate system of the scroll container and applied to the rectangular bounding
    // box of the transformed border box of the target element.
    // See https://www.w3.org/TR/css-scroll-snap-1/#scroll-margin.
    auto marginRect = anchorRect;
    marginRect.Expand(style::ExtentForRect(scrollMarginBox, anchorRect));
    return {marginRect, anchorRect};
  }

  bool RenderElement::VisibleToHitTesting(const HitTestRequest *request) const noexcept
  {
    auto visibility =
      !request || request->UserTriggered() ? Style().UsedVisibility() : Style().GetVisibility();
    return visibility == Visibility::Visible && !IsSkippedContent()
           && ((request && request->IgnoreCSSPointerEventsProperty())
               || Style().UsedPointerEvents() != PointerEvents::None);
  }
}
