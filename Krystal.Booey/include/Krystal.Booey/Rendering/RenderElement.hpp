#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;
}

namespace krys::boo::layout
{
  class ElementBox;
}

namespace krys::boo::style
{
  class PseudoElementRequest;
  struct PseudoElementIdentifier;
}

namespace krys::boo
{
  enum class RenderElementConstructBlockLevelRendererFor : uint8
  {
    Inline,
    ListItem,
    TableOrTablePart
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderElementConstructBlockLevelRendererFor, 3uz);

namespace krys::boo
{
  class BlendingKeyframes;
  class GraphicsLayerAnimation;
  class RenderBlock;
  class RenderBoxModelObject;
  class RenderLayer;
  struct PaintInfo;

  /// @brief A renderer with its own style: an element's, or an anonymous box's.
  class RenderElement : public RenderObject
  {
    friend class RenderTreeBuilder;
    KRYS_NON_COPY_MOVABLE(RenderElement);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderElement);

  private:
    RenderStyle _style;
    RenderObject *_firstChild {nullptr};
    RenderObject *_lastChild {nullptr};
    bool _hasInitializedStyle {false};
    bool _hasCounterNodeMap {false};
    bool _isFirstLetter {false};
    bool _renderBlockHasMarginBeforeQuirk {false};
    bool _renderBlockHasMarginAfterQuirk {false};
    bool _renderBlockShouldForceRelayoutChildren {false};
    bool _renderBoxHasShapeOutsideInfo {false};
    bool _hasPausedImageAnimations {false};

  public:
    /// @brief Displays that get a block flow instead of their usual renderer.
    using ConstructBlockLevelRendererFor = RenderElementConstructBlockLevelRendererFor;

    ~RenderElement() noexcept override;

    /// @brief The renderer @p element gets with @p style, or null if @p style's display doesn't generate a
    /// box.
    /// @param rendererTypeOverride The displays that get a block flow instead.
    KRYS_NODISCARD static RenderPtr<RenderElement>
      CreateFor(dom::Element &element, RenderStyle &&style,
                EnumSet<ConstructBlockLevelRendererFor> rendererTypeOverride = {}) noexcept;

    /// @brief Whether @p content replaces an element's content with an image: it's exactly one image.
    KRYS_NODISCARD static bool IsContentDataSupported(const style::Content &content) noexcept;

    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return _style;
    }

    KRYS_NODISCARD RenderStyle &MutableStyle() noexcept
    {
      return _style;
    }

    KRYS_NODISCARD bool HasInitializedStyle() const noexcept
    {
      return _hasInitializedStyle;
    }

    KRYS_NODISCARD bool HasCounterNodeMap() const noexcept
    {
      return _hasCounterNodeMap;
    }
    void SetHasCounterNodeMap(bool f) noexcept
    {
      _hasCounterNodeMap = f;
    }

    /// @brief Whether this renders the `::first-letter` of a block.
    KRYS_NODISCARD bool IsFirstLetter() const noexcept
    {
      return _isFirstLetter;
    }
    void SetIsFirstLetter() noexcept
    {
      _isFirstLetter = true;
    }

    /// @brief Takes the style the renderer was created with as its first style.
    void InitializeStyle() noexcept;

    /// @brief Replaces the renderer's style, doing at least @p minimalStyleDifference of the work a style
    /// change needs.
    /// @note FIXME: The difference doesn't invalidate layout or painting yet (no layout, painting or layers).
    void SetStyle(RenderStyle &&style,
                  style::DifferenceResult minimalStyleDifference = style::DifferenceResult::Equal) noexcept;

    /// @brief The element rendered, or null for an anonymous box.
    KRYS_NODISCARD dom::Element *GetElement() const noexcept;

    KRYS_NODISCARD layout::ElementBox *LayoutBox() noexcept;
    KRYS_NODISCARD const layout::ElementBox *LayoutBox() const noexcept;

    KRYS_NODISCARD RenderObject *FirstChild() const noexcept
    {
      return _firstChild;
    }
    KRYS_NODISCARD RenderObject *LastChild() const noexcept
    {
      return _lastChild;
    }

    KRYS_NODISCARD RenderObject *FirstInFlowChild() const noexcept;
    KRYS_NODISCARD RenderObject *LastInFlowChild() const noexcept;

    KRYS_NODISCARD RenderObject *FirstChildSlow() const noexcept final
    {
      return FirstChild();
    }
    KRYS_NODISCARD RenderObject *LastChildSlow() const noexcept final
    {
      return LastChild();
    }

    /// @brief Whether @p child, with @p style, may be a child of this renderer.
    KRYS_NODISCARD virtual bool IsChildAllowed(const RenderObject &, const RenderStyle &) const noexcept
    {
      return true;
    }

    void DidAttachChild(RenderObject &child, RenderObject *beforeChild) noexcept;

    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    KRYS_NODISCARD bool IsFixedPositioned() const noexcept;
    KRYS_NODISCARD bool IsAbsolutelyPositioned() const noexcept;

    KRYS_NODISCARD bool IsWritingModeRoot() const noexcept;

    KRYS_NODISCARD bool IsAnonymousForPercentageResolution() const noexcept
    {
      // FIXME: View transition pseudo-elements, once there are view transitions.
      return IsAnonymous();
    }

    /// @see https://drafts.csswg.org/cssom-view/#dom-htmlelement-offsetparent
    KRYS_NODISCARD RenderBoxModelObject *OffsetParent() const noexcept;

    /// @brief Pushes the mapping from this renderer to its container (or @p ancestorToStopAt) onto
    /// @p geometryMap.
    /// @return The renderer the mapping is to, or null at the render view.
    virtual const RenderElement *PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                        RenderGeometryMap &geometryMap) const noexcept;

    KRYS_NODISCARD bool IsHTMLMarquee() const noexcept;

    /// @brief How images drawn for the renderer are oriented: its `image-orientation`.
    // FIXME: An image element that doesn't allow the orientation to be overridden draws its image's
    // (allowsOrientationOverride, G7c).
    KRYS_NODISCARD boo::ImageOrientation GetImageOrientation() const noexcept;

    /// @brief Whether the renderer can be seen at all: it's visible and its document's frame is on screen.
    KRYS_NODISCARD bool IsVisibleIgnoringGeometry() const noexcept;
    /// @brief Whether the renderer can be seen in @p documentRect, in document coordinates.
    KRYS_NODISCARD bool IsVisibleInDocumentRect(const IntRect &documentRect) const noexcept;
    /// @brief Whether the renderer can be seen in its frame's visible content.
    KRYS_NODISCARD bool IsVisibleInViewport() const noexcept;

    /// @brief Whether the images of the renderer may animate.
    KRYS_NODISCARD bool AllowsAnimation() const noexcept;

    /// @brief The renderer's style with its animations applied at their current time.
    KRYS_NODISCARD UniquePtr<RenderStyle> AnimatedStyle() const noexcept;

    /// @brief Starts @p keyframes' animation on the renderer's composited layer, @p timeOffset seconds in.
    /// @return Whether the layer runs it.
    virtual bool StartAnimation(double /*timeOffset*/, const GraphicsLayerAnimation &,
                                const BlendingKeyframes &) noexcept
    {
      return false;
    }
    virtual void AnimationPaused(double /*timeOffset*/, const BlendingKeyframes &) noexcept
    {
    }
    virtual void AnimationFinished(const BlendingKeyframes &) noexcept
    {
    }
    virtual void TransformRelatedPropertyDidChange() noexcept
    {
    }
    virtual void SuspendAnimations(MonotonicTime::duration = {}) noexcept
    {
    }

    /// @brief Repaints the renderer and starts its paused @p cachedImage's animation if it's now visible in
    /// @p visibleRect.
    /// @return Whether it resumed.
    bool RepaintForPausedImageAnimationsIfNeeded(const IntRect &visibleRect,
                                                 CachedImage &cachedImage) noexcept;
    KRYS_NODISCARD bool HasPausedImageAnimations() const noexcept
    {
      return _hasPausedImageAnimations;
    }
    void SetHasPausedImageAnimations(bool b) noexcept
    {
      _hasPausedImageAnimations = b;
    }

    /// @brief Repaints only if the old and new rects differ: what changed between them, or both whole.
    /// @return Whether it repainted (or nothing needed it).
    bool RepaintAfterLayoutIfNeeded(const RenderLayerModelObject *repaintContainer,
                                    RequiresFullRepaint requiresFullRepaint, const RepaintRects &oldRects,
                                    const RepaintRects &newRects) noexcept;
    /// @brief Whether layout checks if the renderer needs repainting (a `LayoutRepainter`), rather than
    /// leaving it to its layer or a full repaint.
    KRYS_NODISCARD bool CheckForRepaintDuringLayout() const noexcept;
    /// @brief Whether the renderer is invisible and so is everything in its enclosing layer.
    KRYS_NODISCARD bool IsInsideEntirelyHiddenLayer() const noexcept;

    VisibleInViewportState ImageFrameAvailable(CachedImage &, ImageAnimatingState,
                                               const IntRect *) noexcept override;
    KRYS_NODISCARD VisibleInViewportState
      ImageVisibleInViewport(const dom::Document &) const noexcept override;
    void DidRemoveCachedImageClient(CachedImage &) noexcept override;

    KRYS_NODISCARD const RenderStyle &FirstLineStyle() const noexcept;

    /// @brief Paints this renderer's @p paintInfo phase, at @p paintOffset (its container's position in the
    /// painting coordinates).
    virtual void Paint(PaintInfo &, const LayoutPoint &paintOffset) noexcept = 0;

    /// @brief Paints every phase at once, as though this renderer established its own stacking context (as
    /// inline blocks, flex items and grid items paint).
    void PaintOutline(PaintInfo &, const LayoutRect &) noexcept;
    void PaintAsInlineBlock(PaintInfo &, const LayoutPoint &childPoint) noexcept;

    /// @brief Lays out this renderer and its descendants that need layout.
    virtual void Layout() noexcept;
    void LayoutIfNeeded() noexcept
    {
      if (NeedsLayout())
      {
        Layout();
      }
    }

    void
      SetChildNeedsLayout(MarkingBehavior markParents = MarkingBehavior::MarkContainingBlockChain) noexcept;
    void ClearChildNeedsLayout() noexcept;
    void SetNeedsOutOfFlowMovementLayout(const RenderStyle *oldStyle) noexcept;
    void SetNeedsLayoutForStyleDifference(style::Difference diff, const RenderStyle *oldStyle) noexcept;
    void SetNeedsLayoutForOverflowChange() noexcept;
    void SetOutOfFlowChildNeedsStaticPositionLayout() noexcept;

    KRYS_NODISCARD virtual bool OverflowChangesMayAffectLayout() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool IsDeprecatedFlexItem() const noexcept
    {
      return !IsInline() && !IsFloatingOrOutOfFlowPositioned() && Parent()
             && Parent()->IsRenderDeprecatedFlexibleBox();
    }

    KRYS_NODISCARD bool IsFlexItemIncludingDeprecated() const noexcept
    {
      return !IsInline() && !IsFloatingOrOutOfFlowPositioned() && Parent()
             && Parent()->IsFlexibleBoxIncludingDeprecated();
    }

    // block-level boxes are boxes that participate in a block formatting context.
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept;
    KRYS_NODISCARD bool IsAnonymousBlock() const noexcept;
    KRYS_NODISCARD bool IsBlockContainer() const noexcept;
    // A block-level box that is also a block container.
    KRYS_NODISCARD bool IsBlockBox() const noexcept;
    KRYS_NODISCARD bool IsBeforeContent() const noexcept;
    KRYS_NODISCARD bool IsAfterContent() const noexcept;
    KRYS_NODISCARD bool IsBeforeOrAfterContent() const noexcept;
    KRYS_NODISCARD static bool IsBeforeContent(const RenderObject *renderer) noexcept;
    KRYS_NODISCARD static bool IsAfterContent(const RenderObject *renderer) noexcept;

    /// @brief The style of the pseudo-element @p pseudoElementIdentifier of this renderer's element,
    /// resolving and caching it if it isn't cached yet.
    KRYS_NODISCARD const RenderStyle *
      GetCachedPseudoStyle(const style::PseudoElementIdentifier &pseudoElementIdentifier,
                           const RenderStyle *parentStyle = nullptr) const noexcept;
    KRYS_NODISCARD UniquePtr<RenderStyle>
      GetUncachedPseudoStyle(const style::PseudoElementRequest &pseudoElementRequest,
                             const RenderStyle *parentStyle = nullptr,
                             const RenderStyle *ownStyle = nullptr) const noexcept;

    /// @brief The element whose content this renderer renders: the host of a pseudo-element.
    KRYS_NODISCARD dom::Element *GeneratingElement() const noexcept;

    KRYS_NODISCARD Overflow EffectiveOverflowX() const noexcept;
    KRYS_NODISCARD Overflow EffectiveOverflowY() const noexcept;
    KRYS_NODISCARD Overflow EffectiveOverflowBlockDirection() const noexcept;
    KRYS_NODISCARD Overflow EffectiveOverflowInlineDirection() const noexcept;
    KRYS_NODISCARD bool HasPotentiallyScrollableOverflow() const noexcept;

    /// @brief The absolute rect a fragment link or scrollIntoView() scrolls to: from the leading corner of
    /// the renderer to its trailing corner.
    KRYS_NODISCARD LayoutRect AbsoluteAnchorRect(bool *insideFixed = nullptr) const noexcept;
    /// @brief The anchor rect expanded by the scroll margin, and the anchor rect.
    KRYS_NODISCARD Pair<LayoutRect, LayoutRect>
      AbsoluteAnchorRectWithScrollMargin(bool *insideFixed = nullptr) const noexcept;

    KRYS_NODISCARD bool HasBackground() const noexcept;
    KRYS_NODISCARD bool BorderImageIsLoadedAndCanBeRendered() const noexcept;
    KRYS_NODISCARD bool HasOutline() const noexcept;
    /// @brief Whether hit tests (@p request's, or user-triggered ones) can hit the renderer: visible, not
    /// skipped, and taking pointer events.
    KRYS_NODISCARD bool VisibleToHitTesting(const HitTestRequest *request = nullptr) const noexcept;
    /// @brief Whether a link's outline carries its URL as an annotation (painting PDFs).
    KRYS_NODISCARD bool HasOutlineAnnotation() const noexcept;

    /// @brief The rect cut out of the border when painting it: a fieldset's legend.
    /// FIXME: Fieldsets (RenderBlock's override, around the legend) come with fieldset layout.
    KRYS_NODISCARD virtual LayoutRect PaintRectToClipOutFromBorder(const LayoutRect &) noexcept
    {
      return {};
    }
    KRYS_NODISCARD bool HasClip() const noexcept;
    KRYS_NODISCARD bool HasClipOrNonVisibleOverflow() const noexcept;

    KRYS_NODISCARD bool HasClipPath() const noexcept;
    KRYS_NODISCARD bool HasFilter() const noexcept;
    KRYS_NODISCARD bool HasMask() const noexcept;
    KRYS_NODISCARD bool HasShapeOutside() const noexcept;
    KRYS_NODISCARD bool HasBackdropFilter() const noexcept;
    KRYS_NODISCARD bool HasBlendMode() const noexcept;
    KRYS_NODISCARD bool IsTransparent() const noexcept;
    KRYS_NODISCARD bool HasHiddenBackface() const noexcept;
    KRYS_NODISCARD static bool CreatesGroupForStyle(const RenderStyle &style) noexcept;
    KRYS_NODISCARD bool CreatesGroup() const noexcept
    {
      return CreatesGroupForStyle(Style());
    }
    KRYS_NODISCARD bool HasViewTransitionName() const noexcept;
    KRYS_NODISCARD bool IsViewTransitionRoot() const noexcept;
    KRYS_NODISCARD bool RequiresRenderingConsolidationForViewTransition() const noexcept;

    KRYS_NODISCARD FloatRect TransformReferenceBoxRect() const noexcept;
    KRYS_NODISCARD FloatRect TransformReferenceBoxRect(const RenderStyle &style) const noexcept;
    KRYS_NODISCARD virtual FloatRect ReferenceBoxRect(CSSBoxType boxType) const noexcept;

    KRYS_NODISCARD RenderLayer *LayerParent() const noexcept;
    KRYS_NODISCARD RenderLayer *LayerNextSibling(RenderLayer &parentLayer) const noexcept;
    void RemoveLayers() noexcept;
    void MoveLayers(RenderLayer &newParent) noexcept;

    KRYS_NODISCARD bool CanEstablishContainingBlockWithTransform() const noexcept;
    KRYS_NODISCARD bool
      MayContainOutOfFlowPositionedObjects(const RenderStyle *styleToUse = nullptr) const noexcept;
    KRYS_NODISCARD bool
      CanContainFixedPositionObjects(const RenderStyle *styleToUse = nullptr) const noexcept;
    KRYS_NODISCARD bool
      CanContainAbsolutelyPositionedObjects(const RenderStyle *styleToUse = nullptr) const noexcept;

    KRYS_NODISCARD bool ShouldApplyAnyContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplySizeOrInlineSizeContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplyLayoutContainment(const RenderStyle *styleToUse = nullptr) const noexcept;
    KRYS_NODISCARD bool ShouldApplySizeContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplyInlineSizeContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplyStyleContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplyPaintContainment(const RenderStyle *styleToUse = nullptr) const noexcept;
    KRYS_NODISCARD bool HasEligibleContainmentForSizeQuery() const noexcept;

  protected:
    void PushOntoGeometryMap(RenderGeometryMap &geometryMap, const RenderLayerModelObject *repaintContainer,
                             RenderElement *container, bool containerSkipped) const noexcept;

    RenderElement(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags,
                  TypeSpecificFlags typeSpecificFlags) noexcept;
    RenderElement(Type type, dom::Document &document, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags,
                  TypeSpecificFlags typeSpecificFlags) noexcept;

    void WillBeDestroyed() noexcept override;

    // FIXME: Scroll anchoring, once there is any.
    virtual void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept;
    virtual void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept;

    template <typename TFillLayers>
    void UpdateFillImages(const TFillLayers *oldLayers, const TFillLayers *newLayers) noexcept;
    void UpdateImage(StyleImage *oldImage, StyleImage *newImage) noexcept;
    void UpdateShapeImage(const style::ShapeOutside *oldShapeValue,
                          const style::ShapeOutside *newShapeValue) noexcept;

    enum class StylePropagationType
    {
      AllChildren,
      BlockAndRubyChildren
    };
    void PropagateStyleToAnonymousChildren(StylePropagationType propagationType) noexcept;

    virtual void UpdateAnonymousChildStyle(RenderStyle &) const noexcept
    {
    }

    void SetRenderBlockHasMarginBeforeQuirk(bool b) noexcept
    {
      _renderBlockHasMarginBeforeQuirk = b;
    }
    void SetRenderBlockHasMarginAfterQuirk(bool b) noexcept
    {
      _renderBlockHasMarginAfterQuirk = b;
    }
    void SetRenderBlockShouldForceRelayoutChildren(bool b) noexcept
    {
      _renderBlockShouldForceRelayoutChildren = b;
    }
    KRYS_NODISCARD bool RenderBlockHasMarginBeforeQuirk() const noexcept
    {
      return _renderBlockHasMarginBeforeQuirk;
    }
    KRYS_NODISCARD bool RenderBlockHasMarginAfterQuirk() const noexcept
    {
      return _renderBlockHasMarginAfterQuirk;
    }
    KRYS_NODISCARD bool RenderBlockShouldForceRelayoutChildren() const noexcept
    {
      return _renderBlockShouldForceRelayoutChildren;
    }
    void SetRenderBoxHasShapeOutsideInfo(bool b) noexcept
    {
      _renderBoxHasShapeOutsideInfo = b;
    }
    KRYS_NODISCARD bool RenderBoxHasShapeOutsideInfo() const noexcept
    {
      return _renderBoxHasShapeOutsideInfo;
    }

  private:
    RenderElement(Type type, dom::ContainerNode &elementOrDocument, RenderStyle &&style,
                  EnumSet<TypeFlag> flags, TypeSpecificFlags typeSpecificFlags) noexcept;

    // Called by RenderTreeBuilder: they take and give up ownership of the child.
    RenderObject *AttachRendererInternal(RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> DetachRendererInternal(RenderObject &renderer) noexcept;

    KRYS_NODISCARD style::Difference AdjustStyleDifference(style::Difference diff) const noexcept;

    /// @brief Repaints, before @p newStyle replaces @p oldStyle, what the change will move or hide.
    /// @return Whether it repainted.
    bool RepaintBeforeStyleChange(style::Difference diff, const RenderStyle &oldStyle,
                                  const RenderStyle &newStyle) noexcept;
    KRYS_NODISCARD bool ShouldRepaintForStyleDifference(style::Difference diff) const noexcept;
    void UpdateOutlineAutoAncestor(bool hasOutlineAuto) noexcept;
    void IssueRepaintForOutlineAuto(float outlineSize) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
