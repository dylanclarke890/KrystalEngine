#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerEnums.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Page/ScrollBehavior.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "Krystal.Booey/Rendering/ClipRect.hpp"
#include "Krystal.Booey/Rendering/LayerFragment.hpp"
#include "Krystal.Booey/Rendering/PaintPhase.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Debug.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  namespace style
  {
    struct Difference;
    enum class TransformResolverOption : uint8;
  }

  class ClipRects;
  class HitTestingTransformState;
  class HitTestRequest;
  class HitTestResult;
  class ClipRectsCache;
  class GraphicsContext;
  class GraphicsContextStateSaver;
  class Path;
  class RenderObject;
  class RenderBox;
  class RenderLayerBacking;
  class RenderLayerCompositor;
  enum class ContentChangeType : uint8;
  class RenderLayerFilters;
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

  /// @brief How to scroll a rect into view.
  // FIXME: Cross-origin scrolling (shouldAllowCrossOriginScrolling) and forward-only scrolling
  // (onlyAllowForwardScrolling), once there are frames and find-in-page.
  struct ScrollRectToVisibleOptions
  {
    SelectionRevealMode RevealMode {SelectionRevealMode::Reveal};
    ScrollAlignment AlignX {ScrollAlignment::AlignCenterIfNeeded};
    ScrollAlignment AlignY {ScrollAlignment::AlignCenterIfNeeded};
    ScrollBehavior Behavior {ScrollBehavior::Auto};
    AllowScrollingOverflowHidden AllowOverflowHidden {AllowScrollingOverflowHidden::Yes};
    Maybe<LayoutRect> VisibilityCheckRect;
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

  enum class RequestState : uint8
  {
    Unknown,
    DontCare,
    False,
    True,
    Undetermined
  };

  enum class IndirectCompositingReason : uint8
  {
    None,
    Clipping,
    Stacking,
    OverflowScrollPositioning,
    Overlap,
    BackgroundLayer,
    GraphicalEffect, // opacity, mask, filter, transform etc.
    Perspective,
    Preserve3D
  };

  enum class UpdateBackingSharingFlags : uint8
  {
    DuringCompositingUpdate,
  };

  using ScrollingScope = uint64;

  /// @brief The layer of a renderer that needs one: it positions, clips and orders (in z-order) the
  /// renderer's subtree.
  // FIXME: Masks, SVG clip paths, reflections, resizing, scrollbars, event regions and fragmentation
  // (paginated collectFragments) come with effects and fragmentation.
  class RenderLayer final : public CanMakeWeakPtr<RenderLayer>, public CanMakeCheckedPtr<RenderLayer>
  {
    KRYS_NON_COPY_MOVABLE(RenderLayer);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLayer);

    friend class RenderLayerBacking;
    friend class RenderLayerCompositor;
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
      RespectOverflowClip,
      IncludeOverlayScrollbarSize,
      Temporary,
      OutsideFilter,
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
      IncludeSelfTransform,
      UseLocalClipRectIfPossible,
      IncludeFilterOutsets,
      IncludePaintedFilterOutsets,
      ExcludeHiddenDescendants,
      DontConstrainForMask,
      IncludeCompositedDescendants,
      UseFragmentBoxesExcludingCompositing,
      UseFragmentBoxesIncludingCompositing,
      IncludeRootBackgroundPaintingArea,
      PreserveAncestorFlags,
      UseLocalClipRectExcludingCompositingIfPossible,
      ExcludeViewTransitionCapturedDescendants,
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
      NeedsPositionUpdate,
      DescendantNeedsPositionUpdate,
      AllChildrenNeedPositionUpdate,
      AllDescendantsNeedPositionUpdate,
    };

    enum class UpdateLayerPositionsFlag : uint16
    {
      CheckForRepaint,
      NeedsFullRepaintInBacking,
      ContainingClippingLayerChangedSize,
      UpdatePagination,
      SeenFixedLayer,
      SeenFixedContainingBlockLayer,
      SeenTransformedLayer,
      Seen3DTransformedLayer,
      SeenCompositedScrollingLayer,
      SubtreeNeedsUpdate,
      EnvironmentChanged,
      SeenStickyLayer,
    };

    // UpdateLayerPositionsFlags that describe changes to the layer tree
    static constexpr EnumSet<UpdateLayerPositionsFlag> InvalidationLayerPositionsFlags()
    {
      return {UpdateLayerPositionsFlag::NeedsFullRepaintInBacking,
              UpdateLayerPositionsFlag::ContainingClippingLayerChangedSize,
              UpdateLayerPositionsFlag::SubtreeNeedsUpdate, UpdateLayerPositionsFlag::EnvironmentChanged};
    }

    // These flags propagate in paint order (z-order tree).
    enum class Compositing : uint8
    {
      // Need to do the overlap-testing tree walk because hierarchy or geometry changed.
      HasDescendantNeedingRequirementsTraversal,
      // Need to update geometry, configuration and update the GraphicsLayer tree.
      HasDescendantNeedingBackingOrHierarchyTraversal,

      // Things that trigger HasDescendantNeedingRequirementsTraversal
      // The paint order children of this layer changed (gained/lost child, order change).
      NeedsPaintOrderChildrenUpdate,
      // Needs compositing to be re-evaluated after layout (it depends on geometry).
      NeedsPostLayoutUpdate,
      // Something changed that forces computeCompositingRequirements to traverse all descendant layers.
      DescendantsNeedRequirementsTraversal,
      // Something changed that forces computeCompositingRequirements to traverse all layers later in paint
      // order.
      SubsequentLayersNeedRequirementsTraversal,

      // Things that trigger HasDescendantNeedingBackingOrHierarchyTraversal
      // This layer needs a geometry update.
      NeedsGeometryUpdate,
      // This layer needs a configuration update (updating its internal compositing hierarchy).
      NeedsConfigurationUpdate,
      // Something changed that requires this layer's scrolling tree node to be updated.
      NeedsScrollingTreeUpdate,
      // This layer needs hookup with its parents or children.
      NeedsLayerConnection,
      // This layer's composited children need a geometry update.
      ChildrenNeedGeometryUpdate,
      // Something changed that forces us to traverse all descendant layers in updateBackingAndHierarchy.
      DescendantsNeedBackingAndHierarchyTraversal,
    };

    static constexpr EnumSet<Compositing> ComputeCompositingRequirementsFlags()
    {
      return {Compositing::NeedsPaintOrderChildrenUpdate, Compositing::NeedsPostLayoutUpdate,
              Compositing::DescendantsNeedRequirementsTraversal,
              Compositing::SubsequentLayersNeedRequirementsTraversal};
    }

    static constexpr EnumSet<Compositing> UpdateBackingOrHierarchyFlags()
    {
      return {
        Compositing::NeedsLayerConnection,       Compositing::NeedsGeometryUpdate,
        Compositing::NeedsConfigurationUpdate,   Compositing::NeedsScrollingTreeUpdate,
        Compositing::ChildrenNeedGeometryUpdate, Compositing::DescendantsNeedBackingAndHierarchyTraversal};
    }

  public:
    void SetAncestorsHaveCompositingDirtyFlag(Compositing flag) noexcept;

    KRYS_NODISCARD bool HasDescendantNeedingCompositingRequirementsTraversal() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::HasDescendantNeedingRequirementsTraversal);
    }
    KRYS_NODISCARD bool HasDescendantNeedingUpdateBackingOrHierarchyTraversal() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::HasDescendantNeedingBackingOrHierarchyTraversal);
    }

    KRYS_NODISCARD bool NeedsCompositingPaintOrderChildrenUpdate() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::NeedsPaintOrderChildrenUpdate);
    }
    KRYS_NODISCARD bool NeedsPostLayoutCompositingUpdate() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::NeedsPostLayoutUpdate);
    }
    KRYS_NODISCARD bool DescendantsNeedCompositingRequirementsTraversal() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::DescendantsNeedRequirementsTraversal);
    }
    KRYS_NODISCARD bool SubsequentLayersNeedCompositingRequirementsTraversal() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::SubsequentLayersNeedRequirementsTraversal);
    }

    KRYS_NODISCARD bool NeedsCompositingLayerConnection() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::NeedsLayerConnection);
    }
    KRYS_NODISCARD bool NeedsCompositingGeometryUpdate() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::NeedsGeometryUpdate);
    }
    KRYS_NODISCARD bool NeedsCompositingConfigurationUpdate() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::NeedsConfigurationUpdate);
    }
    KRYS_NODISCARD bool NeedsScrollingTreeUpdate() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::NeedsScrollingTreeUpdate);
    }
    KRYS_NODISCARD bool ChildrenNeedCompositingGeometryUpdate() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::ChildrenNeedGeometryUpdate);
    }
    KRYS_NODISCARD bool DescendantsNeedUpdateBackingAndHierarchyTraversal() const noexcept
    {
      return _compositingDirtyBits.Contains(Compositing::DescendantsNeedBackingAndHierarchyTraversal);
    }

    template <Compositing V>
    void SetRequirementsTraversalDirtyBit() noexcept
    {
      _compositingDirtyBits.Add(V);
      SetAncestorsHaveCompositingDirtyFlag(Compositing::HasDescendantNeedingRequirementsTraversal);
    }

    void SetNeedsCompositingPaintOrderChildrenUpdate() noexcept
    {
      SetRequirementsTraversalDirtyBit<Compositing::NeedsPaintOrderChildrenUpdate>();
    }
    void SetNeedsPostLayoutCompositingUpdate() noexcept
    {
      SetRequirementsTraversalDirtyBit<Compositing::NeedsPostLayoutUpdate>();
    }
    void SetDescendantsNeedCompositingRequirementsTraversal() noexcept
    {
      SetRequirementsTraversalDirtyBit<Compositing::DescendantsNeedRequirementsTraversal>();
    }
    void SetSubsequentLayersNeedCompositingRequirementsTraversal() noexcept
    {
      SetRequirementsTraversalDirtyBit<Compositing::SubsequentLayersNeedRequirementsTraversal>();
    }

    void SetNeedsPostLayoutCompositingUpdateOnAncestors() noexcept
    {
      SetAncestorsHaveCompositingDirtyFlag(Compositing::NeedsPostLayoutUpdate);
    }

    template <Compositing V>
    void SetBackingAndHierarchyTraversalDirtyBit() noexcept
    {
      _compositingDirtyBits.Add(V);
      SetAncestorsHaveCompositingDirtyFlag(Compositing::HasDescendantNeedingBackingOrHierarchyTraversal);
    }

    void SetNeedsCompositingLayerConnection() noexcept
    {
      SetBackingAndHierarchyTraversalDirtyBit<Compositing::NeedsLayerConnection>();
    }
    void SetNeedsCompositingGeometryUpdate() noexcept
    {
      SetBackingAndHierarchyTraversalDirtyBit<Compositing::NeedsGeometryUpdate>();
    }
    void SetNeedsCompositingConfigurationUpdate() noexcept
    {
      SetBackingAndHierarchyTraversalDirtyBit<Compositing::NeedsConfigurationUpdate>();
    }
    void SetNeedsScrollingTreeUpdate() noexcept
    {
      SetBackingAndHierarchyTraversalDirtyBit<Compositing::NeedsScrollingTreeUpdate>();
    }
    void SetChildrenNeedCompositingGeometryUpdate() noexcept
    {
      SetBackingAndHierarchyTraversalDirtyBit<Compositing::ChildrenNeedGeometryUpdate>();
    }
    void SetDescendantsNeedUpdateBackingAndHierarchyTraversal() noexcept
    {
      SetBackingAndHierarchyTraversalDirtyBit<Compositing::DescendantsNeedBackingAndHierarchyTraversal>();
    }

    void SetNeedsCompositingGeometryUpdateOnAncestors() noexcept
    {
      SetAncestorsHaveCompositingDirtyFlag(Compositing::NeedsGeometryUpdate);
    }

    KRYS_NODISCARD bool NeedsCompositingRequirementsTraversal() const noexcept
    {
      return _compositingDirtyBits.ContainsAny(ComputeCompositingRequirementsFlags());
    }
    void ClearCompositingRequirementsTraversalState() noexcept
    {
      _compositingDirtyBits.Remove(Compositing::HasDescendantNeedingRequirementsTraversal);
      _compositingDirtyBits.Remove(ComputeCompositingRequirementsFlags());
    }

    KRYS_NODISCARD bool NeedsUpdateBackingOrHierarchyTraversal() const noexcept
    {
      return _compositingDirtyBits.ContainsAny(UpdateBackingOrHierarchyFlags());
    }
    void ClearUpdateBackingOrHierarchyTraversalState() noexcept
    {
      _compositingDirtyBits.Remove(Compositing::HasDescendantNeedingBackingOrHierarchyTraversal);
      _compositingDirtyBits.Remove(UpdateBackingOrHierarchyFlags());
    }

    KRYS_NODISCARD bool NeedsAnyCompositingTraversal() const noexcept
    {
      return !_compositingDirtyBits.IsEmpty();
    }
    void ClearCompositingPaintOrderState() noexcept
    {
      _compositingDirtyBits = {};
    }

    enum ViewportConstrainedNotCompositedReason : uint8
    {
      NoNotCompositedReason,
      NotCompositedForBoundsOutOfView,
      NotCompositedForNonViewContainer,
      NotCompositedForNoVisibleContent,
    };

    struct PaintedContentRequest
    {
      RequestState HasPaintedContent {RequestState::Unknown};

      PaintedContentRequest() noexcept = default;

      void SetHasPaintedContent() noexcept
      {
        HasPaintedContent = RequestState::True;
      }
      void MakePaintedContentUndetermined() noexcept
      {
        HasPaintedContent = RequestState::Undetermined;
      }
      KRYS_NODISCARD bool ProbablyHasPaintedContent() const noexcept
      {
        return HasPaintedContent == RequestState::True || HasPaintedContent == RequestState::Undetermined;
      }
      KRYS_NODISCARD bool IsPaintedContentSatisfied() const noexcept
      {
        return HasPaintedContent != RequestState::Unknown;
      }

      // FIXME: HDR content, once there's HDR.
      KRYS_NODISCARD bool IsSatisfied() const noexcept
      {
        return IsPaintedContentSatisfied();
      }
    };

    struct EnclosingCompositingLayerStatus
    {
      bool FullRepaintAlreadyScheduled {false};
      RenderLayer *Layer {nullptr};
    };

  private:
    EnumSet<Compositing> _compositingDirtyBits;

    EnumSet<LayerPositionUpdates> _layerPositionDirtyBits;

    const bool _isRenderViewLayer : 1;
    const bool _forcedStackingContext : 1;

    bool _isNormalFlowOnly : 1 {false};
    bool _inResizeMode : 1 {false};
    bool _canBeBackdropRoot : 1 {false};
    bool _hasBackdropFilterDescendantsWithoutRoot : 1 {false};
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

    bool _hasCompositingDescendant : 1 {false}; // In the z-order tree.
    // Set when a layer has a composited descendant in z-order which is not a descendant in containing block
    // order (e.g. opacity layer with an abspos descendant).
    bool _hasCompositedNonContainedDescendants : 1 {false};

    bool _hasCompositedScrollingAncestor : 1 {false}; // In the layer-order tree.

    bool _hasStickyAncestor : 1 {false};
    bool _hasFixedAncestor : 1 {false};
    bool _hasPaginatedAncestor : 1 {false};

    bool _isHiddenByOverflowTruncation : 1 {false};

    bool _repaintRectsValid : 1 {false};

    bool _suppressAncestorClippingInsideFilter : 1 {false};

#if KRYS_DEBUG
    bool _layerListMutationAllowed : 1 {true};
#endif

    IndirectCompositingReason _indirectCompositingReason {IndirectCompositingReason::None};
    ViewportConstrainedNotCompositedReason _viewportConstrainedNotCompositedReason {NoNotCompositedReason};

    BlendMode _blendMode {BlendMode::Normal};
    bool _hasNotIsolatedCompositedBlendingDescendants : 1 {false};
    bool _hasNotIsolatedBlendingDescendants : 1 {false};
    bool _hasNotIsolatedBlendingDescendantsStatusDirty : 1 {false};

    bool _intrinsicallyComposited : 1 {false};
    bool _alwaysIncludedInZOrderLists : 1 {false};
    bool _hasAlwaysIncludedInZOrderListsDescendants : 1 {false};
    bool _hasAlwaysIncludedInZOrderListsDescendantsStatusDirty : 1 {true};

    bool _wasOmittedFromZOrderTree : 1 {false};

    bool _isFixedIntersectingViewport : 1 {false};

    // Whether this layer has begun its transparency layer during the current paint.
    bool _usedTransparency : 1 {false};
    Maybe<float> _savedAlphaForTransparency;

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

    // If the RenderLayer contains an anchor-positioned box, this is the "default scroll shift"
    // for scroll compensation purpose. This offset aligns the anchor-positioned box with the anchor
    // after scroll, and is applied as a transform.
    Maybe<LayoutSize> _anchorScrollAdjustment;

    UniquePtr<RenderLayerScrollableArea> _scrollableArea;

    RefPtr<RenderLayerFilters> _filters;

    IntRect _blockSelectionGapsBounds;

    // Pointer to the enclosing RenderLayer that caused us to be paginated. It is 0 if we are not paginated.
    WeakPtr<RenderLayer> _enclosingPaginationLayer;

    WeakPtr<RenderLayer> _backingProviderLayer;
    WeakPtr<RenderLayer> _backingProviderLayerAtEndOfCompositingUpdate;

    // Cached repaint rects. Used by layout.
    RenderObject::RepaintRects _repaintRects;
    WeakPtr<const RenderLayerModelObject> _repaintContainer;

    Maybe<ScrollingScope> _boxScrollingScope;
    Maybe<ScrollingScope> _contentsScrollingScope;

    UniquePtr<RenderLayerBacking> _backing;

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
             || _hasNotIsolatedBlendingDescendantsStatusDirty
             || _hasAlwaysIncludedInZOrderListsDescendantsStatusDirty;
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

    KRYS_NODISCARD bool HasCompositedLayerInEnclosingPaginationChain() const noexcept;
    KRYS_NODISCARD RenderLayer *EnclosingPaginationLayer(PaginationInclusionMode mode) const noexcept
    {
      if (mode == ExcludeCompositedPaginatedLayers && HasCompositedLayerInEnclosingPaginationChain())
      {
        return nullptr;
      }
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

    KRYS_NODISCARD bool BackgroundIsKnownToBeOpaqueInRect(const LayoutRect &localRect) const noexcept;
    KRYS_NODISCARD bool ListBackgroundIsKnownToBeOpaqueInRect(const LayerList &list,
                                                              const LayoutRect &localRect) const noexcept;
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

    // Returns true if this layer has visible content (ignoring any child layers).
    KRYS_NODISCARD bool IsVisuallyNonEmpty(PaintedContentRequest *request = nullptr) const noexcept;
    // True if this layer container renderers that paint.
    void DetermineNonLayerDescendantsPaintedContent(PaintedContentRequest &request) const noexcept;

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

    // Enclosing compositing layer; if includeSelf is true, may return this.
    KRYS_NODISCARD RenderLayer *
      EnclosingCompositingLayer(IncludeSelfOrNot includeSelf = IncludeSelf) const noexcept;
    KRYS_NODISCARD EnclosingCompositingLayerStatus
      EnclosingCompositingLayerForRepaint(IncludeSelfOrNot includeSelf = IncludeSelf) const noexcept;
    // Ancestor compositing layer, excluding this.
    KRYS_NODISCARD RenderLayer *AncestorCompositingLayer() const noexcept
    {
      return EnclosingCompositingLayer(ExcludeSelf);
    }

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

    // Bounds used for layer overlap testing in RenderLayerCompositor.
    KRYS_NODISCARD LayoutRect OverlapBounds() const noexcept;

    // Takes transform animations into account, returning true if they could be cheaply computed.
    // Unlike overlapBounds, these bounds include descendant layers.
    KRYS_NODISCARD bool GetOverlapBoundsIncludingChildrenAccountingForTransformAnimations(
      LayoutRect &bounds, EnumSet<CalculateLayerBoundsFlag> additionalFlags = {}) const noexcept;

    // If true, this layer's children are included in its bounds for overlap testing.
    // We can't rely on the children's positions if this layer has a filter that could have moved the
    // children's pixels around.
    KRYS_NODISCARD bool OverlapBoundsIncludeChildren() const noexcept;

    // Can pass offsetFromRoot if known.
    KRYS_NODISCARD LayoutRect CalculateLayerBounds(
      const RenderLayer *ancestorLayer, const LayoutSize &offsetFromRoot,
      EnumSet<CalculateLayerBoundsFlag> flags = DefaultCalculateLayerBoundsFlags()) const noexcept;

#pragma region Hit testing

    /// @brief Hit tests the layers under the result's location from front to back, starting at this layer,
    /// and sets the result's link element.
    /// @return Whether a layer was hit: always for the view's layer on a move over the document, or a press
    /// or release.
    bool HitTest(const HitTestRequest &request, HitTestResult &result) noexcept;

    /// @brief Scrolls to bring @p positionInWindow into view, as a mouse drag selects past the layer's edge.
    void Autoscroll(const IntPoint &positionInWindow) noexcept;
    bool HitTest(const HitTestRequest &request, const HitTestLocation &hitTestLocation,
                 HitTestResult &result) noexcept;

    /// @brief The nearest element of the layer's renderer or its ancestors.
    KRYS_NODISCARD dom::Element *EnclosingElement() const noexcept;

    /// @brief The layers of the top layer's elements and their `::backdrop`s, bottom to top, which the view's
    /// layer paints above everything else.
    KRYS_NODISCARD static List<RenderLayer *> TopLayerRenderLayers(const RenderView &renderView) noexcept;

    /// @brief Whether the layer's renderer is in the top layer or is a `::backdrop`; its layer is then a
    /// child of the view's layer.
    KRYS_NODISCARD bool EstablishesTopLayer() const noexcept;
    void EstablishesTopLayerWillChange() noexcept;
    void EstablishesTopLayerDidChange() noexcept;

#pragma endregion

#pragma region Painting

    enum class PaintLayerFlag : uint8
    {
      HaveTransparency,
      AppliedTransform,
      TemporaryClipRects,
      PaintingReflection,
      PaintingOverlayScrollbars,
      PaintingCompositingBackgroundPhase,
      PaintingCompositingForegroundPhase,
      PaintingCompositingMaskPhase,
      PaintingCompositingClipPathPhase,
      PaintingOverflowContainer,
      PaintingOverflowContentsRoot,
      PaintingOverflowContents,
      PaintingRootBackgroundOnly,
      PaintingSkipRootBackground,
      PaintingChildClippingMaskPhase,
      PaintingSVGClippingMask,
      CollectingEventRegion,
      PaintingSkipDescendantViewTransition,
    };
    static constexpr EnumSet<PaintLayerFlag> PaintLayerPaintingCompositingAllPhasesFlags()
    {
      return {PaintLayerFlag::PaintingCompositingBackgroundPhase,
              PaintLayerFlag::PaintingCompositingForegroundPhase};
    }

    /// @brief Paints the layers that intersect @p damageRect from back to front, starting at this layer.
    /// @param subtreePaintRoot If set, only this renderer's subtree is painted.
    void Paint(GraphicsContext &, const LayoutRect &damageRect,
               const LayoutSize &subpixelOffset = LayoutSize(), EnumSet<PaintBehavior> paintBehavior = {},
               RenderObject *subtreePaintRoot = nullptr, EnumSet<PaintLayerFlag> paintFlags = {}) noexcept;

    KRYS_NODISCARD bool PaintsWithTransparency(EnumSet<PaintBehavior>) const noexcept;
    KRYS_NODISCARD bool HasNonOpacityTransparency() const noexcept;
    /// @brief Whether the layer's opacity can be applied as an alpha rather than a transparency layer.
    KRYS_NODISCARD bool CanPaintTransparencyWithSetOpacity() const noexcept
    {
      return IsBitmapOnly() && !HasNonOpacityTransparency() && !HasFilter();
    }
    /// @brief Whether the layer paints nothing but a canvas or a bitmap image.
    KRYS_NODISCARD bool IsBitmapOnly() const noexcept;

    /// @brief The transform the layer paints with, flattened to 2D when flattening compositing layers.
    KRYS_NODISCARD TransformationMatrix RenderableTransform(EnumSet<PaintBehavior>) const noexcept;

    KRYS_NODISCARD bool ShouldPaintMask(EnumSet<PaintBehavior>, EnumSet<PaintLayerFlag>) const noexcept;
    KRYS_NODISCARD bool ShouldApplyClipPath(EnumSet<PaintBehavior>, EnumSet<PaintLayerFlag>) const noexcept;

    /// @brief The reference box a `clip-path` resolves against, relative to the painting root.
    KRYS_NODISCARD FloatRect ReferenceBoxRectForClipPath(CSSBoxType boxType, const LayoutSize &offsetFromRoot,
                                                         const LayoutRect &rootRelativeBounds) const noexcept;

#pragma endregion

    void SetRepaintStatus(RepaintStatus status) noexcept;
    /// @brief Whether the layer's renderer repaints after a layout moves or resizes it.
    KRYS_NODISCARD bool ShouldRepaintAfterLayout() const noexcept;
    KRYS_NODISCARD RepaintStatus GetRepaintStatus() const noexcept
    {
      return _repaintStatus;
    }
    KRYS_NODISCARD bool NeedsFullRepaint() const noexcept
    {
      return _repaintStatus == RepaintStatus::NeedsFullRepaint
             || _repaintStatus == RepaintStatus::NeedsFullRepaintForOutOfFlowMovementLayout;
    }

    /// @brief The renderer's repaint rects, as of the last layer position update; null when the layer doesn't
    /// paint itself or paints nothing.
    KRYS_NODISCARD Maybe<RenderObject::RepaintRects> RepaintRects() const noexcept
    {
      if (_repaintRectsValid)
      {
        return _repaintRects;
      }
      return null;
    }
    KRYS_NODISCARD Maybe<LayoutRect> CachedClippedOverflowRect() const noexcept;
    /// @brief Works out the renderer's repaint rects in @p repaintContainer's coordinates.
    void ComputeRepaintRects(const RenderLayerModelObject *repaintContainer) noexcept;
    void ComputeRepaintRectsIncludingDescendants() noexcept;
    void ClearRepaintRects() noexcept
    {
      _repaintRectsValid = false;
    }

    /// @brief The layer's repaint rect, with those of its descendant layers that aren't composited.
    KRYS_NODISCARD LayoutRect RepaintRectIncludingNonCompositingDescendants() const noexcept;
    /// @brief Repaints the layer's renderer and those of its descendant layers.
    void RepaintIncludingDescendants() noexcept;

    /// @brief Adds to the bounds of the selection gaps the layer's blocks painted, in its coordinates.
    void AddBlockSelectionGapsBounds(const LayoutRect &bounds) noexcept;
    void ClearBlockSelectionGapsBounds() noexcept;
    /// @brief Repaints the selection gaps the layer and its descendant layers painted.
    void RepaintBlockSelectionGaps() noexcept;

    // Indicate that the layer contents need to be repainted. Only has an effect if layer compositing is being
    // used.
    void SetBackingNeedsRepaint(
      GraphicsLayerShouldClipToLayer shouldClip = GraphicsLayerShouldClipToLayer::Clip) noexcept;

    // The rect is in the coordinate space of the layer's render object.
    void SetBackingNeedsRepaintInRect(const LayoutRect &r, GraphicsLayerShouldClipToLayer shouldClip =
                                                             GraphicsLayerShouldClipToLayer::Clip) noexcept;
    void RepaintIncludingNonCompositingDescendants(const RenderLayerModelObject *repaintContainer) noexcept;

    KRYS_NODISCARD const RenderLayerModelObject *RepaintContainer() const noexcept;
    void ClearRepaintContainer() noexcept
    {
      _repaintContainer = nullptr;
    }

    void CompositingStatusChanged(LayoutUpToDate layoutUpToDate) noexcept;

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
    KRYS_NODISCARD bool Has3DTransformedDescendant() const noexcept
    {
      krys_debug_assert(!_3DTransformedDescendantStatusDirty);
      return _has3DTransformedDescendant;
    }
    /// @brief Whether this layer is part of its parent's 3D rendering context.
    KRYS_NODISCARD bool ParticipatesInPreserve3D() const noexcept;

    KRYS_NODISCARD Maybe<LayoutSize> AnchorScrollAdjustment() const noexcept
    {
      return _anchorScrollAdjustment;
    }
    bool SetAnchorScrollAdjustment(LayoutSize) noexcept; // Returns true if changed.
    void ClearAnchorScrollAdjustment() noexcept;
    KRYS_NODISCARD bool AncestorLayerIsDOMParent(const RenderLayer *ancestor) const noexcept;
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

    KRYS_NODISCARD bool ShouldPaintWithFilters(EnumSet<PaintBehavior> = {}) const noexcept;
    KRYS_NODISCARD bool WillCompositeClipPath() const noexcept;
    KRYS_NODISCARD bool RequiresFullLayerImageForFilters() const noexcept;
    KRYS_NODISCARD RenderLayerFilters *Filters() const noexcept;

    KRYS_NODISCARD IntOutsets FilterOutsets() const noexcept;
    KRYS_NODISCARD bool HasFilterOutsets() const noexcept
    {
      return !FilterOutsets().IsZero();
    }
    KRYS_NODISCARD bool HasAncestorWithFilterOutsets() const noexcept;
    /// @brief Whether the layer paints with a filter that moves pixels (a blur, a drop shadow).
    KRYS_NODISCARD bool FilterMovesPixels() const noexcept;
    /// @brief What the layer's filter paints, in absolute coordinates: the layer's bounds with its
    /// descendants', grown by the filter's outsets.
    KRYS_NODISCARD LayoutRect AbsoluteFilterRepaintRect() const noexcept;

    // Gets the nearest enclosing layer that has a filter which needs the whole layer's image.
    KRYS_NODISCARD RenderLayer *EnclosingFilterLayer(IncludeSelfOrNot = IncludeSelf) const noexcept;
    KRYS_NODISCARD RenderLayer *EnclosingFilterRepaintLayer() const noexcept;
    void SetFilterBackendNeedsRepaintingInRect(const LayoutRect &) noexcept;

    KRYS_NODISCARD bool HasBlendMode() const noexcept;
    KRYS_NODISCARD BlendMode GetBlendMode() const noexcept
    {
      return _blendMode;
    }

    KRYS_NODISCARD bool HasBackdropFilterDescendantsWithoutRoot() const noexcept
    {
      return _hasBackdropFilterDescendantsWithoutRoot;
    }
    void SetHasBackdropFilterDescendantsWithoutRoot(bool hasBackdropFilterDescendantsWithoutRoot) noexcept
    {
      _hasBackdropFilterDescendantsWithoutRoot = hasBackdropFilterDescendantsWithoutRoot;
    }

    KRYS_NODISCARD bool CanBeBackdropRoot() const noexcept
    {
      return _canBeBackdropRoot;
    }
    KRYS_NODISCARD bool IsBackdropRoot() const noexcept
    {
      return HasBackdropFilterDescendantsWithoutRoot() && CanBeBackdropRoot();
    }

    KRYS_NODISCARD bool IsolatesCompositedBlending() const noexcept
    {
      return _hasNotIsolatedCompositedBlendingDescendants && IsCSSStackingContext();
    }
    KRYS_NODISCARD bool HasNotIsolatedCompositedBlendingDescendants() const noexcept
    {
      return _hasNotIsolatedCompositedBlendingDescendants;
    }
    void SetHasNotIsolatedCompositedBlendingDescendants(
      bool hasNotIsolatedCompositedBlendingDescendants) noexcept
    {
      _hasNotIsolatedCompositedBlendingDescendants = hasNotIsolatedCompositedBlendingDescendants;
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

    KRYS_NODISCARD bool PaintsWithTransform(EnumSet<PaintBehavior> paintBehavior) const noexcept;

#pragma region Compositing

    KRYS_NODISCARD RenderLayerCompositor &Compositor() const noexcept;

    /// @brief A name for the layer in layer tree dumps: its renderer's.
    KRYS_NODISCARD utf8_string Name() const noexcept;

    void ContentChanged(ContentChangeType changeType, const Maybe<FloatRect> &dirtyRect = null) noexcept;

    KRYS_NODISCARD bool IsComposited() const noexcept
    {
      return _backing != nullptr;
    }
    KRYS_NODISCARD bool HasCompositingDescendant() const noexcept
    {
      return _hasCompositingDescendant;
    }
    KRYS_NODISCARD bool HasCompositedMask() const noexcept;
    KRYS_NODISCARD bool HasCompositedNonContainedDescendants() const noexcept
    {
      return _hasCompositedNonContainedDescendants;
    }

    // If non-null, a non-ancestor composited layer that this layer paints into (it is sharing its backing
    // store with this layer).
    KRYS_NODISCARD RenderLayer *BackingProviderLayer() const noexcept
    {
      return _backingProviderLayer.get();
    }
    void SetBackingProviderLayer(RenderLayer *backingProvider,
                                 EnumSet<UpdateBackingSharingFlags> flags) noexcept;
    void DisconnectFromBackingProviderLayer(EnumSet<UpdateBackingSharingFlags> flags) noexcept;

    KRYS_NODISCARD bool PaintsIntoProvidedBacking() const noexcept
    {
      return _backingProviderLayer.get() != nullptr;
    }

    KRYS_NODISCARD RenderLayer *BackingProviderLayerAtEndOfCompositingUpdate() const noexcept
    {
      return _backingProviderLayerAtEndOfCompositingUpdate.get();
    }
    void SetBackingProviderLayerAtEndOfCompositingUpdate(RenderLayer *provider) noexcept
    {
      _backingProviderLayerAtEndOfCompositingUpdate = CreateWeakPtr(provider);
    }

    KRYS_NODISCARD RenderLayerBacking *Backing() const noexcept
    {
      return _backing.get();
    }
    RenderLayerBacking *EnsureBacking() noexcept;
    void ClearBacking(EnumSet<UpdateBackingSharingFlags> flags, bool layerBeingDestroyed = false) noexcept;

    KRYS_NODISCARD bool HasCompositedScrollingAncestor() const noexcept
    {
      return _hasCompositedScrollingAncestor;
    }
    void SetHasCompositedScrollingAncestor(bool hasCompositedScrollingAncestor) noexcept
    {
      _hasCompositedScrollingAncestor = hasCompositedScrollingAncestor;
    }

    // Layers with the same ScrollingScope are scrolled by some common ancestor scroller. Used for async
    // scrolling.
    KRYS_NODISCARD Maybe<ScrollingScope> BoxScrollingScope() const noexcept
    {
      return _boxScrollingScope;
    }
    KRYS_NODISCARD Maybe<ScrollingScope> ContentsScrollingScope() const noexcept
    {
      return _contentsScrollingScope;
    }

    void SetViewportConstrainedNotCompositedReason(ViewportConstrainedNotCompositedReason reason) noexcept
    {
      _viewportConstrainedNotCompositedReason = reason;
    }
    KRYS_NODISCARD ViewportConstrainedNotCompositedReason
      GetViewportConstrainedNotCompositedReason() const noexcept
    {
      return _viewportConstrainedNotCompositedReason;
    }

    KRYS_NODISCARD IndirectCompositingReason GetIndirectCompositingReason() const noexcept
    {
      return _indirectCompositingReason;
    }

    KRYS_NODISCARD bool Has3DTransformedAncestor() const noexcept
    {
      return _has3DTransformedAncestor;
    }

    KRYS_NODISCARD bool AlwaysIncludedInZOrderLists() const noexcept
    {
      return _alwaysIncludedInZOrderLists;
    }
    KRYS_NODISCARD bool HasAlwaysIncludedInZOrderListsDescendants() const noexcept
    {
      return _hasAlwaysIncludedInZOrderListsDescendants;
    }
    void SetIntrinsicallyComposited(bool composited) noexcept;
    void UpdateAlwaysIncludedInZOrderLists() noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsRenderFragmentedFlow() const noexcept;

    void SetIsHiddenByOverflowTruncation(bool isHidden) noexcept;

    KRYS_NODISCARD RenderLayerScrollableArea *GetScrollableArea() const noexcept;
    /// @brief Whether scrolling a rect into view may scroll this layer.
    KRYS_NODISCARD bool
      ShouldTryToScrollForScrollIntoView(const ScrollRectToVisibleOptions &options) const noexcept;
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
    /// @brief The smallest the user can resize the box to: its minimum size, and at least the resizer.
    KRYS_NODISCARD LayoutSize MinimumSizeForResizing(float zoomFactor) const noexcept;
    /// @brief Resizes the box to follow the mouse as the user drags its resizer, setting its inline `width`
    /// and `height`.
    /// @param oldOffset The mouse's offset from the resize corner when the drag began.
    void Resize(const PlatformMouseEvent &event, const LayoutSize &oldOffset) noexcept;

    KRYS_NODISCARD bool InResizeMode() const noexcept
    {
      return _inResizeMode;
    }

    void SetInResizeMode(bool inResizeMode) noexcept
    {
      _inResizeMode = inResizeMode;
    }

    /// @brief Whether @p localPoint is over the box's resizer.
    KRYS_NODISCARD bool IsPointInResizeControl(IntPoint localPoint) const noexcept;
    KRYS_NODISCARD IntSize OffsetFromResizeCorner(const IntPoint &localPoint) const noexcept;
    KRYS_NODISCARD LayoutPoint AbsoluteToContents(const LayoutPoint &absolutePoint) const noexcept;

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

    KRYS_NODISCARD bool ComputeCanBeBackdropRoot() const noexcept;
    bool SetCanBeBackdropRoot(bool canBeBackdropRoot) noexcept;

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
                            UniquePtr<List<RenderLayer *>> &negZOrderList,
                            EnumSet<Compositing> &accumulatedDirtyFlags) noexcept;
    void CollectLayers(UniquePtr<List<RenderLayer *>> &positiveZOrderList,
                       UniquePtr<List<RenderLayer *>> &negativeZOrderList,
                       EnumSet<Compositing> &accumulatedDirtyFlags) noexcept;
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
      IsOverflowScroll,
      HasSeenViewportConstrainedAncestor,
      HasSeenAncestorWithOverflowClip,
      HasChangedAncestor,
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

    void UpdateAncestorChainHasAlwaysIncludedInZOrderListsDescendants() noexcept;
    void DirtyAncestorChainHasAlwaysIncludedInZOrderListsDescendants() noexcept;

    void SetWasOmittedFromZOrderTree() noexcept;
    void SetWasIncludedInZOrderTree() noexcept
    {
      _wasOmittedFromZOrderTree = false;
    }
    void RemoveSelfFromCompositor() noexcept;
    void RemoveDescendantsFromCompositor() noexcept;

    void SetHasCompositingDescendant(bool b) noexcept
    {
      _hasCompositingDescendant = b;
    }
    void SetHasCompositedNonContainedDescendants(bool value) noexcept
    {
      _hasCompositedNonContainedDescendants = value;
    }

    void SetIndirectCompositingReason(IndirectCompositingReason reason) noexcept
    {
      _indirectCompositingReason = reason;
    }
    KRYS_NODISCARD bool MustCompositeForIndirectReasons() const noexcept
    {
      return _indirectCompositingReason != IndirectCompositingReason::None;
    }

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

#pragma region Hit testing

    struct HitLayer
    {
      RenderLayer *Layer {nullptr};
      double ZOffset {0};
    };
    HitLayer HitTestLayer(RenderLayer *rootLayer, RenderLayer *containerLayer, const HitTestRequest &,
                          HitTestResult &, const LayoutRect &hitTestRect, const HitTestLocation &,
                          bool appliedTransform, const HitTestingTransformState * = nullptr,
                          double *zOffset = nullptr) noexcept;
    HitLayer HitTestLayerByApplyingTransform(RenderLayer *rootLayer, RenderLayer *containerLayer,
                                             const HitTestRequest &, HitTestResult &,
                                             const LayoutRect &hitTestRect, const HitTestLocation &,
                                             const HitTestingTransformState * = nullptr,
                                             double *zOffset = nullptr,
                                             const LayoutSize &translationOffset = LayoutSize()) noexcept;
    HitLayer HitTestList(LayerList, RenderLayer *rootLayer, const HitTestRequest &, HitTestResult &,
                         const LayoutRect &hitTestRect, const HitTestLocation &,
                         const HitTestingTransformState *, double *zOffsetForDescendants,
                         bool depthSortDescendants) noexcept;

    KRYS_NODISCARD Ref<HitTestingTransformState>
      CreateLocalTransformState(RenderLayer *rootLayer, RenderLayer *containerLayer,
                                const LayoutRect &hitTestRect, const HitTestLocation &,
                                const HitTestingTransformState *containerTransformState,
                                const LayoutSize &translationOffset = LayoutSize()) const noexcept;

    bool HitTestContents(const HitTestRequest &, HitTestResult &, const LayoutRect &layerBounds,
                         const HitTestLocation &, HitTestFilter) const noexcept;
    bool HitTestContentsForFragments(const LayerFragments &, const HitTestRequest &, HitTestResult &,
                                     const HitTestLocation &, HitTestFilter,
                                     bool &insideClipRect) const noexcept;

#pragma endregion

#pragma region Painting

    struct LayerPaintingInfo
    {
      RenderLayer *RootLayer;
      RenderObject *SubtreePaintRoot; // Only paint descendants of this object.
      LayoutRect PaintDirtyRect;      // Relative to rootLayer;
      LayoutSize SubpixelOffset;
      EnumSet<boo::PaintBehavior> PaintBehavior;
      bool RequireSecurityOriginAccessForWidgets;

      LayerPaintingInfo(RenderLayer *inRootLayer, const LayoutRect &inDirtyRect,
                        EnumSet<boo::PaintBehavior> inPaintBehavior, const LayoutSize &inSubpixelOffset,
                        RenderObject *inSubtreePaintRoot = nullptr,
                        bool inRequireSecurityOriginAccessForWidgets = false) noexcept
          : RootLayer(inRootLayer), SubtreePaintRoot(inSubtreePaintRoot), PaintDirtyRect(inDirtyRect),
            SubpixelOffset(inSubpixelOffset), PaintBehavior(inPaintBehavior),
            RequireSecurityOriginAccessForWidgets(inRequireSecurityOriginAccessForWidgets)
      {
      }
    };

    enum BorderRadiusClippingRule
    {
      IncludeSelfForBorderRadius,
      DoNotIncludeSelfForBorderRadius
    };

    KRYS_NODISCARD LayoutPoint PaintOffsetForRenderer(const LayerFragment &fragment,
                                                      const LayerPaintingInfo &paintingInfo) const noexcept
    {
      return ToLayoutPoint(fragment.LayerBounds().Location() - RendererLocation()
                           + paintingInfo.SubpixelOffset);
    }

    void PaintLayer(GraphicsContext &, const LayerPaintingInfo &, EnumSet<PaintLayerFlag>) noexcept;
    void PaintLayerWithEffects(GraphicsContext &, const LayerPaintingInfo &,
                               EnumSet<PaintLayerFlag>) noexcept;
    void PaintLayerContentsAndReflection(GraphicsContext &, const LayerPaintingInfo &,
                                         EnumSet<PaintLayerFlag>) noexcept;
    void PaintLayerContents(GraphicsContext &, const LayerPaintingInfo &, EnumSet<PaintLayerFlag>) noexcept;
    void PaintLayerByApplyingTransform(GraphicsContext &, const LayerPaintingInfo &, EnumSet<PaintLayerFlag>,
                                       const LayoutSize &translationOffset = LayoutSize()) noexcept;
    void PaintList(LayerList, GraphicsContext &, const LayerPaintingInfo &, EnumSet<PaintLayerFlag>) noexcept;

    void CollectFragments(LayerFragments &, const RenderLayer *rootLayer, const LayoutRect &dirtyRect,
                          ClipRectsType, EnumSet<ClipRectsOption>,
                          const LayoutSize &offsetFromRoot) const noexcept;
    void UpdatePaintingInfoForFragments(LayerFragments &, const LayerPaintingInfo &, EnumSet<PaintLayerFlag>,
                                        bool shouldPaintContent,
                                        const LayoutSize &offsetFromRoot) const noexcept;

    void PaintBackgroundForFragments(const LayerFragments &, GraphicsContext &,
                                     GraphicsContext &contextForTransparencyLayer,
                                     const LayoutRect &transparencyPaintDirtyRect, bool haveTransparency,
                                     const LayerPaintingInfo &, EnumSet<boo::PaintBehavior>,
                                     RenderObject *subtreePaintRootForRenderer) noexcept;
    void PaintForegroundForFragments(const LayerFragments &, GraphicsContext &,
                                     GraphicsContext &contextForTransparencyLayer,
                                     const LayoutRect &transparencyPaintDirtyRect, bool haveTransparency,
                                     const LayerPaintingInfo &, EnumSet<boo::PaintBehavior>,
                                     RenderObject *subtreePaintRootForRenderer) noexcept;
    void PaintForegroundForFragmentsWithPhase(PaintPhase, const LayerFragments &, GraphicsContext &,
                                              const LayerPaintingInfo &, EnumSet<boo::PaintBehavior>,
                                              RenderObject *subtreePaintRootForRenderer) noexcept;
    void PaintOutlineForFragments(const LayerFragments &, GraphicsContext &, const LayerPaintingInfo &,
                                  EnumSet<boo::PaintBehavior>,
                                  RenderObject *subtreePaintRootForRenderer) noexcept;
    void PaintMaskForFragments(const LayerFragments &, GraphicsContext &, const LayerPaintingInfo &,
                               EnumSet<boo::PaintBehavior>,
                               RenderObject *subtreePaintRootForRenderer) noexcept;
    void PaintChildClippingMaskForFragments(const LayerFragments &, GraphicsContext &,
                                            const LayerPaintingInfo &, EnumSet<boo::PaintBehavior>,
                                            RenderObject *subtreePaintRootForRenderer) noexcept;

    void ClipToRect(GraphicsContext &, GraphicsContextStateSaver &, const LayerPaintingInfo &,
                    EnumSet<boo::PaintBehavior>, const ClipRect &,
                    BorderRadiusClippingRule = IncludeSelfForBorderRadius) noexcept;

    void SetupClipPath(GraphicsContext &, GraphicsContextStateSaver &, const LayerPaintingInfo &,
                       EnumSet<PaintLayerFlag> &, const LayoutSize &offsetFromRoot) noexcept;
    KRYS_NODISCARD Pair<Path, WindRule>
      ComputeClipPath(const LayoutSize &offsetFromRoot,
                      const LayoutRect &rootRelativeBoundsForNonBoxes) const noexcept;

    KRYS_NODISCARD RenderLayer *TransparentPaintingAncestor(const LayerPaintingInfo &) noexcept;
    void BeginTransparencyLayers(GraphicsContext &, const LayerPaintingInfo &,
                                 const LayoutRect &dirtyRect) noexcept;

    KRYS_NODISCARD bool ShouldHaveFiltersForPainting(GraphicsContext &, EnumSet<PaintLayerFlag>,
                                                     EnumSet<PaintBehavior>) const noexcept;
    RenderLayerFilters *FiltersForPainting(GraphicsContext &, EnumSet<PaintLayerFlag>,
                                           EnumSet<PaintBehavior>) noexcept;
    GraphicsContext *SetupFilters(GraphicsContext &destinationContext, LayerPaintingInfo &,
                                  EnumSet<PaintLayerFlag> &, const LayoutSize &offsetFromRoot,
                                  const ClipRect &backgroundRect) noexcept;
    void ApplyFilters(GraphicsContext &originalContext, const LayerPaintingInfo &, EnumSet<PaintBehavior>,
                      const ClipRect &backgroundRect) noexcept;

    RenderLayerFilters &EnsureLayerFilters() noexcept;
    void ClearLayerFilters() noexcept;
    void ClearLayerClipPath() noexcept;
    void UpdateFiltersAfterStyleChange(style::Difference, const RenderStyle *oldStyle) noexcept;
    void UpdateFilterPaintingStrategy() noexcept;

#pragma endregion
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

  KRYS_NODISCARD bool CompositedWithOwnBackingStore(const RenderLayer &layer) noexcept;

  TextStream &operator<<(TextStream &ts, IndirectCompositingReason reason) noexcept;
}
