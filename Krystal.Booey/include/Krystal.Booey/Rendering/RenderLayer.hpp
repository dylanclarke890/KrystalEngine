#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/ClipRect.hpp"
#include "Krystal.Booey/Rendering/LayerFragment.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Debug.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  namespace style
  {
    struct Difference;
    enum class TransformResolverOption : uint8;
  }

  class ClipRects;
  class ClipRectsCache;
  class RenderBox;
  class RenderLayerModelObject;
  class RenderLayerScrollableArea;
  class RenderStyle;

  enum IncludeSelfOrNot
  {
    IncludeSelf,
    ExcludeSelf
  };
  enum class CrossFrameBoundaries : bool
  {
    No,
    Yes
  };

  enum class RepaintStatus : uint8
  {
    NeedsNormalRepaint,
    NeedsFullRepaint,
    NeedsFullRepaintForOutOfFlowMovementLayout
  };

  enum ClipRectsType
  {
    PaintingClipRects,     // Relative to painting ancestor. Used for painting.
    RootRelativeClipRects, // Relative to the ancestor treated as the root (e.g. transformed layer). Used for
                           // hit testing.
    AbsoluteClipRects,     // Relative to the RenderView's layer. Used for compositing overlap testing.
    NumCachedClipRectsTypes,
    AllClipRectTypes,
  };

  enum ShouldRespectOverflowClip
  {
    IgnoreOverflowClip,
    RespectOverflowClip
  };

  enum class LayoutUpToDate : bool
  {
    No,
    Yes
  };

  /// @brief The layer of a renderer that needs one: it positions, clips and orders (in z-order) the
  /// renderer's subtree.
  // FIXME: Painting, hit testing, repaint, compositing, filters, clip paths, reflections, resizing, selection
  // gaps, event regions and fragmentation (collectFragments) come with painting, hit testing and
  // fragmentation.
  class RenderLayer final : public CanMakeWeakPtr<RenderLayer>, public CanMakeCheckedPtr<RenderLayer>
  {
    KRYS_NON_COPY_MOVABLE(RenderLayer);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLayer);

    friend class RenderLayerScrollableArea;

  public:
    struct OverflowControlRects
    {
      IntRect HorizontalScrollbar;
      IntRect VerticalScrollbar;
      IntRect ScrollCorner;
      IntRect Resizer;

      KRYS_NODISCARD IntRect ScrollCornerOrResizerRect() const noexcept
      {
        return !ScrollCorner.IsEmpty() ? ScrollCorner : Resizer;
      }
    };

    class LayerList
    {
      friend class RenderLayer;

    private:
      const List<RenderLayer *> *_layerList;

      explicit LayerList(const List<RenderLayer *> *layerList) noexcept : _layerList(layerList)
      {
      }

    public:
      using const_iterator = RenderLayer *const *;

      KRYS_NODISCARD const_iterator begin() const noexcept
      {
        return _layerList ? _layerList->data() : nullptr;
      }
      KRYS_NODISCARD const_iterator end() const noexcept
      {
        return _layerList ? _layerList->data() + _layerList->size() : nullptr;
      }

      KRYS_NODISCARD size_t size() const noexcept
      {
        return _layerList ? _layerList->size() : 0;
      }
    };

    enum ColumnOffsetAdjustment
    {
      DontAdjustForColumns,
      AdjustForColumns
    };

    enum PaginationInclusionMode
    {
      ExcludeCompositedPaginatedLayers,
      IncludeCompositedPaginatedLayers
    };

    enum class ClipRectsOption : uint8
    {
      RespectOverflowClip = 1 << 0,
      IncludeOverlayScrollbarSize = 1 << 1,
      Temporary = 1 << 2,
      OutsideFilter = 1 << 3,
    };

    static constexpr EnumSet<ClipRectsOption> ClipRectOptionsForPaintingOverflowContents {};
    static constexpr EnumSet<ClipRectsOption> ClipRectDefaultOptions {ClipRectsOption::RespectOverflowClip};
    static constexpr EnumSet<ClipRectsOption> ClipRectTemporaryOptions {ClipRectsOption::RespectOverflowClip,
                                                                        ClipRectsOption::Temporary};

    struct ClipRectsContext
    {
      const RenderLayer *RootLayer;
      ClipRectsType Type;
      EnumSet<ClipRectsOption> Options;

      ClipRectsContext(const RenderLayer *rootLayer, ClipRectsType clipRectsType,
                       EnumSet<ClipRectsOption> options = ClipRectDefaultOptions) noexcept
          : RootLayer(rootLayer), Type(clipRectsType), Options(options)
      {
        if (clipRectsType == RootRelativeClipRects)
        {
          Options.Add(ClipRectsOption::IncludeOverlayScrollbarSize);
        }
      }

      KRYS_NODISCARD bool RespectOverflowClip() const noexcept
      {
        return Options.Contains(ClipRectsOption::RespectOverflowClip);
      }
      KRYS_NODISCARD OverlayScrollbarSizeRelevancy GetOverlayScrollbarSizeRelevancy() const noexcept
      {
        return Options.Contains(ClipRectsOption::IncludeOverlayScrollbarSize)
                 ? OverlayScrollbarSizeRelevancy::IncludeOverlayScrollbarSize
                 : OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize;
      }
    };

    enum CalculateLayerBoundsFlag : uint16
    {
      IncludeSelfTransform = 1 << 0,
      UseLocalClipRectIfPossible = 1 << 1,
      IncludeFilterOutsets = 1 << 2,
      IncludePaintedFilterOutsets = 1 << 3,
      ExcludeHiddenDescendants = 1 << 4,
      DontConstrainForMask = 1 << 5,
      IncludeCompositedDescendants = 1 << 6,
      UseFragmentBoxesExcludingCompositing = 1 << 7,
      UseFragmentBoxesIncludingCompositing = 1 << 8,
      IncludeRootBackgroundPaintingArea = 1 << 9,
      PreserveAncestorFlags = 1 << 10,
      UseLocalClipRectExcludingCompositingIfPossible = 1 << 11,
      ExcludeViewTransitionCapturedDescendants = 1 << 12,
    };
    static constexpr EnumSet<CalculateLayerBoundsFlag> DefaultCalculateLayerBoundsFlags()
    {
      return {IncludeSelfTransform, UseLocalClipRectIfPossible, IncludePaintedFilterOutsets,
              UseFragmentBoxesExcludingCompositing};
    }

    enum class LocalClipRectMode : uint8
    {
      IncludeCompositingState,
      ExcludeCompositingState,
    };

  private:
    enum class LayerPositionUpdates : uint8
    {
      NeedsPositionUpdate = 1 << 0,
      DescendantNeedsPositionUpdate = 1 << 1,
      AllChildrenNeedPositionUpdate = 1 << 2,
      AllDescendantsNeedPositionUpdate = 1 << 3,
    };

    enum class UpdateLayerPositionsFlag : uint16
    {
      CheckForRepaint = 1 << 0,
      NeedsFullRepaintInBacking = 1 << 1,
      ContainingClippingLayerChangedSize = 1 << 2,
      UpdatePagination = 1 << 3,
      SeenFixedLayer = 1 << 4,
      SeenFixedContainingBlockLayer = 1 << 5,
      SeenTransformedLayer = 1 << 6,
      Seen3DTransformedLayer = 1 << 7,
      SeenCompositedScrollingLayer = 1 << 8,
      SubtreeNeedsUpdate = 1 << 9,
      EnvironmentChanged = 1 << 10,
      SeenStickyLayer = 1 << 11,
    };

    // UpdateLayerPositionsFlags that describe changes to the layer tree
    static constexpr EnumSet<UpdateLayerPositionsFlag> InvalidationLayerPositionsFlags()
    {
      return {UpdateLayerPositionsFlag::NeedsFullRepaintInBacking,
              UpdateLayerPositionsFlag::ContainingClippingLayerChangedSize,
              UpdateLayerPositionsFlag::SubtreeNeedsUpdate, UpdateLayerPositionsFlag::EnvironmentChanged};
    }

    EnumSet<LayerPositionUpdates> _layerPositionDirtyBits;

    const bool _isRenderViewLayer : 1;
    const bool _forcedStackingContext : 1;

    bool _isNormalFlowOnly : 1 {false};
    bool _isCSSStackingContext : 1 {false};
    bool _isOpportunisticStackingContext : 1 {false};

    bool _zOrderListsDirty : 1 {false};
    bool _normalFlowListDirty : 1 {true};
    bool _hadNegativeZOrderList : 1 {false};

    bool _isSelfPaintingLayer : 1 {false};

    // If have no self-painting descendants, we don't have to walk our children during painting. This can lead
    // to significant savings, especially if the tree has lots of non-self-painting layers grouped together
    // (e.g. table cells).
    bool _hasSelfPaintingLayerDescendant : 1 {false};
    bool _hasSelfPaintingLayerDescendantDirty : 1 {false};

    bool _hasViewportConstrainedDescendant : 1 {false};
    bool _hasViewportConstrainedDescendantStatusDirty : 1 {false};

    RepaintStatus _repaintStatus : 2 {RepaintStatus::NeedsNormalRepaint};

    bool _visibleContentStatusDirty : 1 {true};
    bool _hasVisibleContent : 1 {false};
    bool _visibleDescendantStatusDirty : 1 {false};
    bool _hasVisibleDescendant : 1 {false};
    bool _behavesAsFixed : 1 {false};

    bool _3DTransformedDescendantStatusDirty : 1 {true};
    // Set on a stacking context layer that has 3D descendants anywhere in a preserves3D hierarchy. Hint to do
    // 3D-aware hit testing.
    bool _has3DTransformedDescendant : 1 {false};

    bool _hasFixedContainingBlockAncestor : 1 {false};
    bool _hasTransformedAncestor : 1 {false};
    bool _has3DTransformedAncestor : 1 {false};

    bool _hasCompositedScrollingAncestor : 1 {false}; // In the layer-order tree.

    bool _hasStickyAncestor : 1 {false};
    bool _hasFixedAncestor : 1 {false};
    bool _hasPaginatedAncestor : 1 {false};

    bool _isHiddenByOverflowTruncation : 1 {false};

#if KRYS_DEBUG
    bool _layerListMutationAllowed : 1 {true};
#endif

    BlendMode _blendMode {BlendMode::Normal};
    bool _hasNotIsolatedBlendingDescendants : 1 {false};
    bool _hasNotIsolatedBlendingDescendantsStatusDirty : 1 {false};

    RenderLayerModelObject &_renderer;

    WeakPtr<RenderLayer> _parent;
    WeakPtr<RenderLayer> _previous;
    WeakPtr<RenderLayer> _next;
    WeakPtr<RenderLayer> _first;
    WeakPtr<RenderLayer> _last;

    // For layers that establish stacking contexts, _posZOrderList holds a sorted list of all the
    // descendant layers within the stacking context that have z-indices of 0 or greater
    // (auto will count as 0). _negZOrderList holds descendants within our stacking context with negative
    // z-indices.
    UniquePtr<List<RenderLayer *>> _posZOrderList;
    UniquePtr<List<RenderLayer *>> _negZOrderList;

    // This list contains child layers that cannot create stacking contexts and appear in normal flow order.
    UniquePtr<List<RenderLayer *>> _normalFlowList;

    // Our current relative or absolute position offset.
    LayoutSize _offsetForPosition;

    // Our (x,y) coordinates are in our parent layer's coordinate space.
    LayoutPoint _topLeft;

    // The layer's width/height
    IntSize _layerSize;

    UniquePtr<ClipRectsCache> _clipRectsCache;

    UniquePtr<TransformationMatrix> _transform;

    UniquePtr<RenderLayerScrollableArea> _scrollableArea;

    // Pointer to the enclosing RenderLayer that caused us to be paginated. It is 0 if we are not paginated.
    WeakPtr<RenderLayer> _enclosingPaginationLayer;

  public:
    explicit RenderLayer(RenderLayerModelObject &renderer) noexcept;
    ~RenderLayer() noexcept;

    KRYS_NODISCARD static UniquePtr<RenderLayer> Create(RenderLayerModelObject &renderer) noexcept;

    KRYS_NODISCARD RenderLayerModelObject &Renderer() const noexcept
    {
      return _renderer;
    }
    KRYS_NODISCARD RenderBox *GetRenderBox() const noexcept;

    KRYS_NODISCARD RenderLayer *Parent() const noexcept
    {
      return _parent.get();
    }
    KRYS_NODISCARD RenderLayer *PreviousSibling() const noexcept
    {
      return _previous.get();
    }
    KRYS_NODISCARD RenderLayer *NextSibling() const noexcept
    {
      return _next.get();
    }
    KRYS_NODISCARD RenderLayer *FirstChild() const noexcept
    {
      return _first.get();
    }
    KRYS_NODISCARD RenderLayer *LastChild() const noexcept
    {
      return _last.get();
    }
    KRYS_NODISCARD bool IsDescendantOf(const RenderLayer &layer) const noexcept;
    KRYS_NODISCARD RenderLayer *CommonAncestorWithLayer(const RenderLayer &layer) const noexcept;

    // This does an ancestor tree walk. Avoid it!
    KRYS_NODISCARD const RenderLayer *Root() const noexcept
    {
      const RenderLayer *curr = this;
      while (curr->Parent())
      {
        curr = curr->Parent();
      }
      return curr;
    }

    void AddChild(RenderLayer &newChild, RenderLayer *beforeChild = nullptr) noexcept;
    void RemoveChild(RenderLayer &oldChild) noexcept;

    void InsertOnlyThisLayer() noexcept;
    void RemoveOnlyThisLayer() noexcept;

    KRYS_NODISCARD bool IsNormalFlowOnly() const noexcept
    {
      return _isNormalFlowOnly;
    }

    // isStackingContext is true for layers that we've determined should be stacking contexts for painting.
    // Not all stacking contexts are CSS stacking contexts.
    KRYS_NODISCARD bool IsStackingContext() const noexcept
    {
      return IsCSSStackingContext() || _isOpportunisticStackingContext;
    }

    // isCSSStackingContext is true for layers that are stacking contexts from a CSS perspective.
    // isCSSStackingContext() => isStackingContext().
    // FIXME: m_forcedStackingContext should affect isStackingContext(), not isCSSStackingContext(), but doing
    // so breaks media control mix-blend-mode.
    KRYS_NODISCARD bool IsCSSStackingContext() const noexcept
    {
      return _isCSSStackingContext || _forcedStackingContext;
    }

    // Gets the enclosing stacking context for this layer, excluding this layer itself.
    KRYS_NODISCARD RenderLayer *StackingContext() const noexcept;

    // Gets the enclosing stacking container for this layer, possibly the layer
    // itself, if it is a stacking container.
    KRYS_NODISCARD RenderLayer *EnclosingStackingContext() noexcept
    {
      return IsStackingContext() ? this : StackingContext();
    }

    void ForceStackingContextIfNeeded() noexcept;

    KRYS_NODISCARD RenderLayer *PaintOrderParent() const noexcept
    {
      return _isNormalFlowOnly ? _parent.get() : StackingContext();
    }

    void DirtyNormalFlowList() noexcept;
    void DirtyZOrderLists() noexcept;
    void DirtyStackingContextZOrderLists() noexcept;
    void DirtyHiddenStackingContextAncestorZOrderLists() noexcept;

    KRYS_NODISCARD bool NormalFlowListDirty() const noexcept
    {
      return _normalFlowListDirty;
    }
    KRYS_NODISCARD bool ZOrderListsDirty() const noexcept
    {
      return _zOrderListsDirty;
    }

#if KRYS_DEBUG
    KRYS_NODISCARD bool LayerListMutationAllowed() const noexcept
    {
      return _layerListMutationAllowed;
    }
    void SetLayerListMutationAllowed(bool flag) noexcept
    {
      _layerListMutationAllowed = flag;
    }
#endif

    void SetNeedsPositionUpdate() noexcept;
    void SetSelfAndChildrenNeedPositionUpdate() noexcept;
    void SetSelfAndDescendantsNeedPositionUpdate() noexcept;

    KRYS_NODISCARD LayerList NormalFlowLayers() const noexcept
    {
      krys_debug_assert(!_normalFlowListDirty);
      return LayerList(_normalFlowList.get());
    }

    KRYS_NODISCARD LayerList PositiveZOrderLayers() const noexcept
    {
      krys_debug_assert(!_zOrderListsDirty);
      krys_debug_assert(IsStackingContext() || !_posZOrderList);
      return LayerList(_posZOrderList.get());
    }

    KRYS_NODISCARD bool HasNegativeZOrderLayers() const noexcept
    {
      return _negZOrderList && _negZOrderList->size();
    }

    KRYS_NODISCARD LayerList NegativeZOrderLayers() const noexcept
    {
      krys_debug_assert(!_zOrderListsDirty);
      krys_debug_assert(IsStackingContext() || !_negZOrderList);
      return LayerList(_negZOrderList.get());
    }

    // Update our normal and z-index lists.
    void UpdateLayerListsIfNeeded() noexcept;
    void UpdateDescendantDependentFlags() noexcept;
    KRYS_NODISCARD bool DescendantDependentFlagsAreDirty() const noexcept
    {
      return _visibleDescendantStatusDirty || _visibleContentStatusDirty
             || _hasSelfPaintingLayerDescendantDirty || _hasViewportConstrainedDescendantStatusDirty
             || _hasNotIsolatedBlendingDescendantsStatusDirty;
    }

    void StyleChanged(style::Difference diff, const RenderStyle *oldStyle) noexcept;

    KRYS_NODISCARD bool IsSelfPaintingLayer() const noexcept
    {
      return _isSelfPaintingLayer;
    }

    // FIXME: This function is incorrectly named. It's isNotOpaque, sometimes called hasOpacity, not
    // isEntirelyTransparent.
    KRYS_NODISCARD bool IsTransparent() const noexcept;

    KRYS_NODISCARD const LayoutPoint &Location() const noexcept
    {
      return _topLeft;
    }
    void SetLocation(const LayoutPoint &p) noexcept
    {
      _topLeft = p;
    }

    KRYS_NODISCARD const IntSize &Size() const noexcept
    {
      return _layerSize;
    }
    // Only public for RenderTreeAsText.
    void SetSize(const IntSize &size) noexcept
    {
      _layerSize = size;
    }

    KRYS_NODISCARD LayoutRect Rect() const noexcept
    {
      return LayoutRect(Location(), LayoutSize(Size()));
    }

    KRYS_NODISCARD bool IsRenderViewLayer() const noexcept
    {
      return _isRenderViewLayer;
    }
    KRYS_NODISCARD bool IsForcedStackingContext() const noexcept
    {
      return _forcedStackingContext;
    }
    KRYS_NODISCARD bool IsOpportunisticStackingContext() const noexcept
    {
      return _isOpportunisticStackingContext;
    }
    bool SetIsOpportunisticStackingContext(bool isStacking) noexcept;

    void UpdateLayerPositionsAfterStyleChange(bool environmentChanged = false) noexcept;
    void UpdateLayerPositionsAfterLayout(bool didFullRepaint, bool environmentChanged) noexcept;

    KRYS_NODISCARD RenderLayer *EnclosingPaginationLayer(PaginationInclusionMode) const noexcept
    {
      // FIXME: Composited layers in the pagination chain (hasCompositedLayerInEnclosingPaginationChain), once
      // there's compositing.
      return _enclosingPaginationLayer.get();
    }

    void UpdateTransform() noexcept;

    void UpdateBlendMode() noexcept;
    void WillRemoveChildWithBlendMode() noexcept;

    KRYS_NODISCARD const LayoutSize &OffsetForInFlowPosition() const noexcept
    {
      return _offsetForPosition;
    }

    void ClearClipRectsIncludingDescendants(ClipRectsType typeToClear = AllClipRectTypes) noexcept;
    void ClearClipRects(ClipRectsType typeToClear = AllClipRectTypes) noexcept;

    // FIXME: We should ASSERT(!m_visibleContentStatusDirty) here, but see
    // https://bugs.webkit.org/show_bug.cgi?id=71044 ditto for hasVisibleDescendant(), see
    // https://bugs.webkit.org/show_bug.cgi?id=71277
    KRYS_NODISCARD bool HasVisibleContent() const noexcept
    {
      return _hasVisibleContent;
    }
    KRYS_NODISCARD bool HasVisibleDescendant() const noexcept
    {
      return _hasVisibleDescendant;
    }

    void SetHasVisibleContent() noexcept;
    void DirtyVisibleContentStatus() noexcept;

    KRYS_NODISCARD bool HasVisibleBoxDecorationsOrBackground() const noexcept;
    KRYS_NODISCARD bool HasVisibleBoxDecorations() const noexcept;

    void SetBehavesAsFixed(bool behavesAsFixed) noexcept
    {
      _behavesAsFixed = behavesAsFixed;
    }
    KRYS_NODISCARD bool BehavesAsFixed() const noexcept
    {
      return _behavesAsFixed;
    }

    KRYS_NODISCARD bool IsVisibilityHiddenOrOpacityZero() const noexcept;
    KRYS_NODISCARD bool IsSubtreeVisibilityHiddenOrOpacityZero() const noexcept;

    KRYS_NODISCARD bool IsViewportConstrained() const noexcept;

    // FIXME: We should ASSERT(!m_hasSelfPaintingLayerDescendantDirty); here but we hit the same bugs as
    // visible content above. Part of the issue is with subtree relayout: we don't check if our ancestors have
    // some descendant flags dirty, missing some updates.
    KRYS_NODISCARD bool HasSelfPaintingLayerDescendant() const noexcept
    {
      return _hasSelfPaintingLayerDescendant;
    }

    KRYS_NODISCARD bool
      AncestorLayerIsInContainingBlockChain(const RenderLayer &ancestor,
                                            const RenderLayer *checkLimit = nullptr) const noexcept;

    // Gets the nearest enclosing positioned ancestor layer (also includes
    // the <html> layer and the root layer).
    KRYS_NODISCARD RenderLayer *EnclosingAncestorForPosition(PositionType position) const noexcept;

    KRYS_NODISCARD RenderLayer *EnclosingLayerInContainingBlockOrder() const noexcept;
    KRYS_NODISCARD RenderLayer *
      EnclosingContainingBlockLayer(CrossFrameBoundaries crossFrameBoundaries) const noexcept;

    // The layer relative to which clipping rects for this layer are computed.
    KRYS_NODISCARD RenderLayer *ClippingRootForPainting() const noexcept;

    KRYS_NODISCARD RenderLayer *EnclosingOverflowClipLayer(IncludeSelfOrNot includeSelf) const noexcept;

    KRYS_NODISCARD bool CanUseOffsetFromAncestor() const noexcept;
    KRYS_NODISCARD bool CanUseOffsetFromAncestor(const RenderLayer &ancestor) const noexcept;

    // FIXME: adjustForColumns allows us to position compositing layers in columns correctly, but eventually
    // they need to be split across columns too.
    void ConvertToPixelSnappedLayerCoords(
      const RenderLayer *ancestorLayer, IntPoint &location,
      ColumnOffsetAdjustment adjustForColumns = DontAdjustForColumns) const noexcept;
    KRYS_NODISCARD LayoutPoint
      ConvertToLayerCoords(const RenderLayer *ancestorLayer, const LayoutPoint &location,
                           ColumnOffsetAdjustment adjustForColumns = DontAdjustForColumns) const noexcept;
    KRYS_NODISCARD LayoutSize
      OffsetFromAncestor(const RenderLayer *ancestorLayer,
                         ColumnOffsetAdjustment adjustForColumns = DontAdjustForColumns) const noexcept;

    KRYS_NODISCARD int32 ZIndex() const noexcept;

    // This method figures out our layerBounds in coordinates relative to
    // |rootLayer|. It also computes our background and foreground clip rects
    // for painting/event handling.
    // Pass offsetFromRoot if known.
    KRYS_NODISCARD LayerFragment::Rects
      CalculateRects(const ClipRectsContext &clipRectsContext, const LayoutSize &offsetFromRoot,
                     const LayoutRect &paintDirtyRect = LayoutRect::InfiniteRect()) const noexcept;

    // Returns the background clip rect of the layer in the local coordinate space.
    KRYS_NODISCARD LayoutRect
      LocalClipRect(bool &clipExceedsBounds,
                    LocalClipRectMode mode = LocalClipRectMode::IncludeCompositingState) const noexcept;

    KRYS_NODISCARD bool ClipCrossesPaintingBoundary() const noexcept;

    // Pass offsetFromRoot if known.
    KRYS_NODISCARD bool
      IntersectsDamageRect(const LayoutRect &layerBounds, const LayoutRect &damageRect,
                           const RenderLayer *rootLayer, const LayoutSize &offsetFromRoot,
                           const Maybe<LayoutRect> &cachedBoundingBox = null) const noexcept;

    // Bounding box relative to some ancestor layer. Pass offsetFromRoot if known.
    KRYS_NODISCARD LayoutRect BoundingBox(const RenderLayer *rootLayer,
                                          const LayoutSize &offsetFromRoot = LayoutSize(),
                                          EnumSet<CalculateLayerBoundsFlag> flags = {}) const noexcept;
    // Bounding box in the coordinates of this layer.
    KRYS_NODISCARD LayoutRect LocalBoundingBox(EnumSet<CalculateLayerBoundsFlag> flags = {}) const noexcept;
    // Deprecated: Pixel snapped bounding box relative to the root.
    KRYS_NODISCARD IntRect AbsoluteBoundingBox() const noexcept;

    // Can pass offsetFromRoot if known.
    KRYS_NODISCARD LayoutRect CalculateLayerBounds(
      const RenderLayer *ancestorLayer, const LayoutSize &offsetFromRoot,
      EnumSet<CalculateLayerBoundsFlag> flags = DefaultCalculateLayerBoundsFlags()) const noexcept;

    void SetRepaintStatus(RepaintStatus status) noexcept;
    KRYS_NODISCARD RepaintStatus GetRepaintStatus() const noexcept
    {
      return _repaintStatus;
    }
    KRYS_NODISCARD bool NeedsFullRepaint() const noexcept
    {
      return _repaintStatus == RepaintStatus::NeedsFullRepaint
             || _repaintStatus == RepaintStatus::NeedsFullRepaintForOutOfFlowMovementLayout;
    }

    KRYS_NODISCARD LayoutUnit StaticInlinePosition() const noexcept
    {
      return _offsetForPosition.Width();
    }
    KRYS_NODISCARD LayoutUnit StaticBlockPosition() const noexcept
    {
      return _offsetForPosition.Height();
    }

    void SetStaticInlinePosition(LayoutUnit position) noexcept
    {
      _offsetForPosition.SetWidth(position);
    }
    void SetStaticBlockPosition(LayoutUnit position) noexcept
    {
      _offsetForPosition.SetHeight(position);
    }

    KRYS_NODISCARD bool IsTransformed() const noexcept;
    // Note that this transform has the transform-origin baked in.
    KRYS_NODISCARD TransformationMatrix *Transform() const noexcept
    {
      return _transform.get();
    }
    // updateTransformFromStyle computes a transform according to the passed options (e.g. transform-origin
    // baked in or excluded) and the given style.
    void UpdateTransformFromStyle(TransformationMatrix &transform, const RenderStyle &style,
                                  EnumSet<style::TransformResolverOption> options) const noexcept;
    // currentTransform computes a transform which takes accelerated animations into account. The
    // resulting transform has transform-origin baked in, unless non-default options are given. If
    // the layer does not have a transform, the identity matrix is returned.
    KRYS_NODISCARD TransformationMatrix
      CurrentTransform(EnumSet<style::TransformResolverOption> options) const noexcept;
    KRYS_NODISCARD TransformationMatrix CurrentTransform() const noexcept;

    // Get the children transform (to apply a perspective on children), which is applied to transformed
    // sublayers, but not this layer. Returns true if the layer has a perspective. Note that this transform
    // has the perspective-origin baked in.
    KRYS_NODISCARD TransformationMatrix PerspectiveTransform() const noexcept;
    KRYS_NODISCARD FloatPoint PerspectiveOrigin() const noexcept;
    KRYS_NODISCARD FloatPoint3D TransformOriginPixelSnappedIfNeeded() const noexcept;
    KRYS_NODISCARD bool Preserves3D() const noexcept;
    KRYS_NODISCARD bool HasPerspective() const noexcept;
    KRYS_NODISCARD bool Has3DTransform() const noexcept
    {
      return _transform && !_transform->IsAffine();
    }
    KRYS_NODISCARD bool HasTransformedAncestor() const noexcept
    {
      return _hasTransformedAncestor;
    }

    KRYS_NODISCARD bool HasFixedContainingBlockAncestor() const noexcept
    {
      return _hasFixedContainingBlockAncestor;
    }

    KRYS_NODISCARD bool HasFilter() const noexcept;
    KRYS_NODISCARD bool HasBackdropFilter() const noexcept;

    KRYS_NODISCARD bool HasBlendMode() const noexcept;
    KRYS_NODISCARD BlendMode GetBlendMode() const noexcept
    {
      return _blendMode;
    }

    KRYS_NODISCARD bool IsolatesBlending() const noexcept
    {
      return HasNotIsolatedBlendingDescendants() && IsCSSStackingContext();
    }

    // FIXME: We should ASSERT(!m_hasNotIsolatedBlendingDescendantsStatusDirty); here but we hit the same bugs
    // as visible content above.
    KRYS_NODISCARD bool HasNotIsolatedBlendingDescendants() const noexcept
    {
      return _hasNotIsolatedBlendingDescendants;
    }
    KRYS_NODISCARD bool HasNotIsolatedBlendingDescendantsStatusDirty() const noexcept
    {
      return _hasNotIsolatedBlendingDescendantsStatusDirty;
    }

    KRYS_NODISCARD bool PaintsWithTransform() const noexcept;

    KRYS_NODISCARD bool IsRenderFragmentedFlow() const noexcept;

    void SetIsHiddenByOverflowTruncation(bool isHidden) noexcept;

    KRYS_NODISCARD RenderLayerScrollableArea *GetScrollableArea() const noexcept;
    RenderLayerScrollableArea *EnsureLayerScrollableArea() noexcept;

    KRYS_NODISCARD IntSize VisibleSize() const noexcept;

    KRYS_NODISCARD int32 ScrollWidth() const noexcept;
    KRYS_NODISCARD int32 ScrollHeight() const noexcept;

    // Returns the nearest enclosing layer that is scrollable.
    // FIXME: This can return the RenderView's layer when callers probably want the FrameView as a
    // ScrollableArea.
    KRYS_NODISCARD RenderLayer *
      EnclosingScrollableLayer(IncludeSelfOrNot includeSelf,
                               CrossFrameBoundaries crossFrameBoundaries) const noexcept;

    // Returns true when the layer could do touch scrolling, but doesn't look at whether there is actually
    // scrollable overflow.
    KRYS_NODISCARD bool CanUseCompositedScrolling() const noexcept;
    // Returns true when there is actually scrollable overflow (requires layout to be up-to-date).
    KRYS_NODISCARD bool HasCompositedScrollableOverflow() const noexcept;
    void ComputeHasCompositedScrollableOverflow(LayoutUpToDate layoutUpToDate) noexcept;

    KRYS_NODISCARD bool HasOverlayScrollbars() const noexcept;
    KRYS_NODISCARD bool UsesCompositedScrolling() const noexcept;

    void UpdateScrollInfoAfterLayout() noexcept;
    void UpdateScrollbarSteps() noexcept;

    KRYS_NODISCARD bool CanResize() const noexcept;

    KRYS_NODISCARD OverflowControlRects OverflowControlsRects() const noexcept;

    void UpdateLayerPositionsAfterOverflowScroll() noexcept;
    void UpdateLayerPositionsAfterDocumentScroll() noexcept;

  private:
    void SetNextSibling(RenderLayer *next) noexcept;
    void SetPreviousSibling(RenderLayer *prev) noexcept;
    void SetFirstChild(RenderLayer *first) noexcept;
    void SetLastChild(RenderLayer *last) noexcept;

    void UpdateAncestorDependentState() noexcept;

    void DirtyPaintOrderListsOnChildChange(RenderLayer &child) noexcept;

    KRYS_NODISCARD bool ShouldBeNormalFlowOnly() const noexcept;
    KRYS_NODISCARD bool ShouldBeCSSStackingContext() const noexcept;

    // Return true if changed.
    bool SetIsNormalFlowOnly(bool isNormalFlowOnly) noexcept;

    bool SetIsCSSStackingContext(bool isCSSStackingContext) noexcept;

    void IsStackingContextChanged() noexcept;

    KRYS_NODISCARD bool IsDirtyStackingContext() const noexcept
    {
      return _zOrderListsDirty && IsStackingContext();
    }

    void UpdateZOrderLists() noexcept;
    void RebuildZOrderLists() noexcept;
    void RebuildZOrderLists(UniquePtr<List<RenderLayer *>> &posZOrderList,
                            UniquePtr<List<RenderLayer *>> &negZOrderList) noexcept;
    void CollectLayers(UniquePtr<List<RenderLayer *>> &positiveZOrderList,
                       UniquePtr<List<RenderLayer *>> &negativeZOrderList) noexcept;
    void ClearZOrderLists() noexcept;

    void UpdateNormalFlowList() noexcept;

    KRYS_NODISCARD bool NeedsPositionUpdate() const noexcept;
    void ClearLayerPositionDirtyBits() noexcept
    {
      _layerPositionDirtyBits = {};
    }

    // Compute, cache and return clip rects computed with the given layer as the root.
    KRYS_NODISCARD Ref<ClipRects> UpdateClipRects(const ClipRectsContext &clipRectsContext) noexcept;
    // Compute and return the clip rects. If useCached is true, will used previously computed clip rects on
    // ancestors (rather than computing them all from scratch up the parent chain).
    void CalculateClipRects(const ClipRectsContext &clipRectsContext, ClipRects &clipRects) const noexcept;

    void SetAncestorChainHasSelfPaintingLayerDescendant() noexcept;
    void DirtyAncestorChainHasSelfPaintingLayerDescendantStatus() noexcept;

    KRYS_NODISCARD LayoutRect ClipRectRelativeToAncestor(const RenderLayer *ancestor,
                                                         LayoutSize offsetFromAncestor,
                                                         const LayoutRect &constrainingRect,
                                                         bool temporaryClipRects = false) const noexcept;

    void UpdateSelfPaintingLayer() noexcept;

    void UpdateLayerScrollableArea() noexcept;
    void ClearLayerScrollableArea() noexcept;

    enum class UpdateLayerPositionsAfterScrollFlag : uint8
    {
      IsOverflowScroll = 1 << 0,
      HasSeenViewportConstrainedAncestor = 1 << 1,
      HasSeenAncestorWithOverflowClip = 1 << 2,
      HasChangedAncestor = 1 << 3,
    };
    void RecursiveUpdateLayerPositionsAfterScroll(
      EnumSet<UpdateLayerPositionsAfterScrollFlag> flags = {}) noexcept;

    KRYS_NODISCARD static EnumSet<UpdateLayerPositionsFlag>
      FlagsForUpdateLayerPositions(RenderLayer &startingLayer) noexcept;

    // Returns true if the position changed.
    bool UpdateLayerPosition(EnumSet<UpdateLayerPositionsFlag> *flags = nullptr) noexcept;

    void RecursiveUpdateLayerPositions(EnumSet<UpdateLayerPositionsFlag> flags) noexcept;
    KRYS_NODISCARD bool
      AncestorLayerPositionStateChanged(EnumSet<UpdateLayerPositionsFlag> flags) const noexcept;

    KRYS_NODISCARD LayoutPoint RendererLocation() const noexcept;
    KRYS_NODISCARD LayoutRect RendererBorderBoxRect() const noexcept;
    KRYS_NODISCARD LayoutRect RendererVisualOverflowRect() const noexcept;
    KRYS_NODISCARD LayoutRect RendererOverflowClipRect(
      const LayoutPoint &location, OverlayScrollbarSizeRelevancy relevancy) const noexcept;
    KRYS_NODISCARD LayoutRect RendererOverflowClipRectForChildLayers(
      const LayoutPoint &location, OverlayScrollbarSizeRelevancy relevancy) const noexcept;
    KRYS_NODISCARD bool RendererHasVisualOverflow() const noexcept;

    KRYS_NODISCARD bool ShouldBeSelfPaintingLayer() const noexcept;

    void DirtyAncestorChainVisibleDescendantStatus() noexcept;

    KRYS_NODISCARD bool ComputeHasVisibleContent() const noexcept;

    void SetAncestorChainHasViewportConstrainedDescendant() noexcept;
    void DirtyAncestorChainHasViewportConstrainedDescendantStatus() noexcept;

    void Dirty3DTransformedDescendantStatus() noexcept;
    // Both updates the status, and returns true if descendants of this have 3d.
    bool Update3DTransformedDescendantStatus() noexcept;

    void UpdateAncestorChainHasBlendingDescendants() noexcept;
    void DirtyAncestorChainHasBlendingDescendants() noexcept;

    KRYS_NODISCARD RefPtr<ClipRects> ParentClipRects(const ClipRectsContext &clipRectsContext) const noexcept;
    KRYS_NODISCARD ClipRect BackgroundClipRect(const ClipRectsContext &clipRectsContext) const noexcept;
    KRYS_NODISCARD ClipRect CalculateBackgroundClipRect(const ClipRectsContext &clipRectsContext,
                                                        const LayoutSize &offsetFromRoot) const noexcept;
    KRYS_NODISCARD ClipRect CalculateBackgroundRect(const ClipRectsContext &clipRectsContext,
                                                    const LayoutSize &offsetFromRoot) const noexcept;
    KRYS_NODISCARD ClipRect CalculateForegroundRect(const ClipRectsContext &clipRectsContext,
                                                    const LayoutSize &offsetFromRoot) const noexcept;

    KRYS_NODISCARD RenderLayer *EnclosingTransformedAncestor() const noexcept;

    void UpdatePagination() noexcept;
  };

#if KRYS_DEBUG
  /// @brief Asserts that a layer's lists aren't mutated while it's alive.
  class LayerListMutationDetector
  {
  private:
    RenderLayer &_layer;
    bool _previousMutationAllowedState;

  public:
    explicit LayerListMutationDetector(RenderLayer &layer) noexcept
        : _layer(layer), _previousMutationAllowedState(layer.LayerListMutationAllowed())
    {
      _layer.SetLayerListMutationAllowed(false);
    }

    ~LayerListMutationDetector() noexcept
    {
      _layer.SetLayerListMutationAllowed(_previousMutationAllowedState);
    }
  };
#endif

  void MakeMatrixRenderable(TransformationMatrix &matrix, bool has3DRendering) noexcept;
}
