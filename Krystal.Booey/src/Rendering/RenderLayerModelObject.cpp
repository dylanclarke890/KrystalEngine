#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Animation/BlendingKeyframes.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/ReferencedSVGResources.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"

namespace krys::boo
{
  RenderLayerModelObject::RenderLayerModelObject(Type type, dom::Element &element, RenderStyle &&style,
                                                 EnumSet<TypeFlag> baseTypeFlags,
                                                 TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderElement(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsLayerModelObject,
                      typeSpecificFlags)
  {
    krys_debug_assert(IsRenderLayerModelObject());
  }

  RenderLayerModelObject::RenderLayerModelObject(Type type, dom::Document &document, RenderStyle &&style,
                                                 EnumSet<TypeFlag> baseTypeFlags,
                                                 TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderElement(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsLayerModelObject,
                      typeSpecificFlags)
  {
    krys_debug_assert(IsRenderLayerModelObject());
  }

  namespace
  {
    // Used to store state between styleWillChange and styleDidChange
    bool s_wasFloating = false;
    bool s_hadLayer = false;
    bool s_wasTransformed = false;
    bool s_layerWasSelfPainting = false;
    bool s_establishesTopLayerChanged = false;
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderLayerModelObject::~RenderLayerModelObject() noexcept = default;

  void RenderLayerModelObject::WillBeDestroyed() noexcept
  {
    if (IsPositioned())
    {
      if (Style().HasViewportConstrainedPosition())
      {
        View().FrameView().RemoveViewportConstrainedObject(*this);
      }
    }

    DestroyLayer();

    RenderElement::WillBeDestroyed();
  }

  void RenderLayerModelObject::DestroyLayer() noexcept
  {
    SetHasLayer(false);
    _layer = nullptr;
  }

  void RenderLayerModelObject::CreateLayer() noexcept
  {
    krys_debug_assert(!_layer);
    _layer = RenderLayer::Create(*this);
    SetHasLayer(true);
    _layer->InsertOnlyThisLayer();
  }

  void RenderLayerModelObject::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    s_wasFloating = IsFloating();
    s_hadLayer = HasLayer();
    s_wasTransformed = IsTransformed();
    if (s_hadLayer)
    {
      s_layerWasSelfPainting = Layer()->IsSelfPaintingLayer();
    }

    const auto *oldStyle = HasInitializedStyle() ? &Style() : nullptr;

    // Whether the element is rendered in the top layer follows its overlay, so its layer moves to or from the
    // view's layer as its style changes.
    s_establishesTopLayerChanged = s_hadLayer && oldStyle && Layer()->Parent()
                                   && dom::IsInTopLayerOrBackdrop(*oldStyle, GetElement())
                                        != dom::IsInTopLayerOrBackdrop(newStyle, GetElement());
    if (s_establishesTopLayerChanged)
    {
      Layer()->EstablishesTopLayerWillChange();
    }
    if (diff == style::DifferenceResult::RepaintLayer && Parent() && oldStyle
        && oldStyle->GetClip() != newStyle.GetClip())
    {
      Layer()->ClearClipRectsIncludingDescendants();
    }
    // What a filter that moves pixels paints reaches past the layer's repaint rects, so when the filter
    // changes, all it painted is repainted, and all it paints after.
    if (s_hadLayer && Parent() && oldStyle && oldStyle->GetFilter() != newStyle.GetFilter()
        && Layer()->FilterMovesPixels())
    {
      View().RepaintViewRectangle(Layer()->AbsoluteFilterRepaintRect());
    }
    RenderElement::StyleWillChange(diff, newStyle);
  }

  void RenderLayerModelObject::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    UpdateFromStyle();
    RenderElement::StyleDidChange(diff, oldStyle);

    if (HasLayer() && Parent() && oldStyle && oldStyle->GetFilter() != Style().GetFilter()
        && Layer()->FilterMovesPixels())
    {
      View().RepaintViewRectangle(Layer()->AbsoluteFilterRepaintRect());
    }

    // When an out-of-flow-positioned element changes its display between block and inline-block,
    // then an incremental layout on the element's containing block lays out the element through
    // LayoutPositionedObjects, which skips laying out the element's parent.
    // The element's parent needs to relayout so that it calls
    // RenderBlockFlow::setStaticInlinePositionForChild with the out-of-flow-positioned child, so
    // that when it's laid out, its RenderBox::computeOutOfFlowPositionedLogicalWidth/Height takes into
    // account its new inline/block position rather than its old block/inline position.
    // Position changes and other types of display changes are handled elsewhere.
    if ((oldStyle && IsOutOfFlowPositioned() && Parent() && (Parent() != ContainingBlock()))
        && (Style().GetPosition() == oldStyle->GetPosition())
        && (Style().IsOriginalDisplayInlineType() != oldStyle->IsOriginalDisplayInlineType())
        && ((Style().IsOriginalDisplayBlockType()) || (Style().IsOriginalDisplayInlineType()))
        && ((oldStyle->IsOriginalDisplayBlockType()) || (oldStyle->IsOriginalDisplayInlineType())))
    {
      Parent()->SetChildNeedsLayout();
    }

    if (s_establishesTopLayerChanged && HasLayer() && !Layer()->Parent())
    {
      Layer()->EstablishesTopLayerDidChange();
    }

    if (RequiresLayer())
    {
      // FIXME: No layers inside hidden SVG subtrees (layerCreationAllowedForSubtree).
      if (!Layer())
      {
        if (s_wasFloating && IsFloating())
        {
          SetChildNeedsLayout();
        }
        CreateLayer();
        if (Parent() && !NeedsLayout())
        {
          Layer()->SetRepaintStatus(RepaintStatus::NeedsFullRepaint);
        }
      }
    }
    else if (Layer() && Layer()->Parent())
    {
      if (oldStyle && oldStyle->HasBlendMode())
      {
        Layer()->WillRemoveChildWithBlendMode();
      }
      // All transform-related properties force layers, so we know we don't have one or the object doesn't
      // support them.
      SetHasTransformRelatedProperty(false);

      // Repaint the about to be destroyed self-painting layer when style change also triggers repaint.
      if (Layer()->IsSelfPaintingLayer() && Layer()->GetRepaintStatus() == RepaintStatus::NeedsFullRepaint
          && Layer()->CachedClippedOverflowRect())
      {
        RepaintUsingContainer(ContainerForRepaint().Renderer, *Layer()->CachedClippedOverflowRect());
      }

      Layer()->RemoveOnlyThisLayer(); // calls destroyLayer() which clears m_layer
      if (s_wasFloating && IsFloating())
      {
        SetChildNeedsLayout();
      }
      if (s_wasTransformed)
      {
        SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }

    if (Layer())
    {
      Layer()->StyleChanged(diff, oldStyle);
      if (s_hadLayer && Layer()->IsSelfPaintingLayer() != s_layerWasSelfPainting)
      {
        SetChildNeedsLayout();
      }
    }

    bool newStyleIsViewportConstrained = Style().HasViewportConstrainedPosition();
    bool oldStyleIsViewportConstrained = oldStyle && oldStyle->HasViewportConstrainedPosition();
    if (newStyleIsViewportConstrained != oldStyleIsViewportConstrained)
    {
      if (newStyleIsViewportConstrained && Layer())
      {
        View().FrameView().AddViewportConstrainedObject(*this);
      }
      else
      {
        View().FrameView().RemoveViewportConstrainedObject(*this);
      }
    }

    // FIXME: Scrollbar steps, and scroll snap containers once there's scroll snapping.
  }

  bool RenderLayerModelObject::ApplyCachedClipAndScrollPosition(RepaintRects &,
                                                                const RenderLayerModelObject *,
                                                                VisibleRectContext) const noexcept
  {
    return false;
  }

  TransformationMatrix *RenderLayerModelObject::LayerTransform() const noexcept
  {
    if (HasLayer())
    {
      return Layer()->Transform();
    }
    return nullptr;
  }

  void RenderLayerModelObject::UpdateLayerTransform() noexcept
  {
    // FIXME: Boxes with an offset path that depends on the containing block's layout update after it
    // (setBoxNeedsTransformUpdateAfterContainerLayout).
    // Transform-origin depends on box size, so we need to update the layer transform after layout.
    if (HasLayer())
    {
      Layer()->UpdateTransform();
    }
  }

  void RenderLayerModelObject::ApplyTransform(TransformationMatrix &transform, const RenderStyle &style,
                                              const FloatRect &boundingBox) const noexcept
  {
    // FIXME: Shouldn't this be using Style::TransformResolver::individualTransformOperations?
    ApplyTransform(transform, style, boundingBox, style::TransformResolver::AllTransformOperations);
  }

  // Every renderer is pixel snapped: there are no layer-aware SVG renderers.
  bool RenderLayerModelObject::StartAnimation(double timeOffset, const GraphicsLayerAnimation &animation,
                                              const BlendingKeyframes &keyframes) noexcept
  {
    if (!Layer() || !Layer()->Backing())
    {
      return false;
    }
    return Layer()->Backing()->StartAnimation(timeOffset, animation, keyframes);
  }

  void RenderLayerModelObject::AnimationPaused(double timeOffset, const BlendingKeyframes &keyframes) noexcept
  {
    if (!Layer() || !Layer()->Backing())
    {
      return;
    }
    Layer()->Backing()->AnimationPaused(timeOffset, keyframes.AcceleratedAnimationName());
  }

  void RenderLayerModelObject::AnimationFinished(const BlendingKeyframes &keyframes) noexcept
  {
    if (!Layer() || !Layer()->Backing())
    {
      return;
    }
    Layer()->Backing()->AnimationFinished(keyframes.AcceleratedAnimationName());
  }

  void RenderLayerModelObject::TransformRelatedPropertyDidChange() noexcept
  {
    if (!Layer() || !Layer()->Backing())
    {
      return;
    }
    Layer()->Backing()->TransformRelatedPropertyDidChange();
  }

  void RenderLayerModelObject::SuspendAnimations(MonotonicTime::duration time) noexcept
  {
    if (!Layer() || !Layer()->Backing())
    {
      return;
    }
    Layer()->Backing()->SuspendAnimations(time);
  }

  FloatRect SnapRectToDevicePixelsIfNeeded(const LayoutRect &rect,
                                           const RenderLayerModelObject &renderer) noexcept
  {
    return SnapRectToDevicePixels(rect, renderer.GetDocument().DeviceScaleFactor());
  }

  FloatRect SnapRectToDevicePixelsIfNeeded(const FloatRect &rect,
                                           const RenderLayerModelObject &renderer) noexcept
  {
    return SnapRectToDevicePixels(LayoutRect(rect), renderer.GetDocument().DeviceScaleFactor());
  }

  Maybe<LayoutRect> RenderLayerModelObject::CachedLayerClippedOverflowRect() const noexcept
  {
    return HasLayer() ? Layer()->CachedClippedOverflowRect() : null;
  }

  LegacyRenderSVGResourceClipper *RenderLayerModelObject::LegacySVGClipperResourceFromStyle() const noexcept
  {
    if (auto clipPath = Style().GetClipPath().TryReference())
    {
      return ReferencedSVGResources::ReferencedClipperRenderer(TreeScopeForSVGReferences(), *clipPath);
    }
    return nullptr;
  }
}
