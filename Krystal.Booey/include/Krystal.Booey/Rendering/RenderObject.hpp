#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImageClient.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/VisibleRectContext.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
  class Node;
  class Position;
  class PositionWithAffinity;
}

namespace krys::boo::layout
{
  class Box;
}

namespace krys::boo
{
  class FloatPoint;
  class HitTestLocation;
  class HitTestRequest;
  class HitTestResult;
  class FloatQuad;
  class IntRect;
  class LayoutPoint;
  class LayoutRect;
  class LayoutSize;
  class RenderBlock;
  class RenderBox;
  class RenderElement;
  class RenderLayer;
  class RenderLayerModelObject;
  class RenderTreeBuilder;
  class RenderView;
  class RenderGeometryMap;
  class TransformState;
  class TransformationMatrix;
  class VisiblePosition;
  enum class Affinity : bool;
  enum class HitTestSource : bool;

  // FIXME: SVG transforms and view transition captures come with their features.
  enum class RenderObjectStateFlag : uint8
  {
    IsBlock,
    IsBlockLevelReplacedOrAtomicInline,
    NeedsLayout,
    NeedsOutOfFlowMovementLayout,
    NormalChildNeedsLayout,
    OutOfFlowChildNeedsLayout,
    NeedsSimplifiedNormalFlowLayout,
    OutOfFlowChildNeedsStaticPositionLayout,
    EverHadLayout,
    IsExcludedFromNormalLayout,
    Floating,
    VerticalWritingMode,
    PreferredLogicalWidthsNeedUpdate,
    HasRareData,
    HasLayer,
    HasNonVisibleOverflow,
    HasTransformRelatedProperty,
    ChildrenInline,
    PaintContainmentApplies,
    WasSkippedDuringLastLayoutDueToContentVisibility
  };

  /// @brief How coordinates are mapped between a renderer and its ancestors.
  enum class MapCoordinatesMode : uint8
  {
    IsFixed,
    UseTransforms,
    ApplyContainerFlip,
    IgnoreStickyOffsets
  };

  /// @brief Which part of a renderer a hit test looks at: itself, its descendants or both.
  enum HitTestFilter
  {
    HitTestAll,
    HitTestSelf,
    HitTestDescendants
  };

  /// @brief The hit testing counterpart of a paint phase: what a renderer is tested for in one pass.
  enum HitTestAction
  {
    HitTestBlockBackground,
    HitTestChildBlockBackground,
    HitTestChildBlockBackgrounds,
    HitTestFloat,
    HitTestForeground
  };

  /// @brief Whether repaint rects include the outline bounds rect.
  enum class RepaintOutlineBounds : bool
  {
    No,
    Yes
  };

  /// @brief Whether a repaint after layout repaints the old and new rects whole, rather than what changed.
  enum class RequiresFullRepaint : bool
  {
    No,
    Yes
  };

  /// @brief Whether marking a renderer as needing layout also marks its containing blocks.
  enum class RenderObjectMarkingBehavior : bool
  {
    MarkOnlyThis,
    MarkContainingBlockChain
  };

  enum class RenderObjectTypeFlag : uint8
  {
    IsAnonymous,
    IsText,
    IsBox,
    IsBoxModelObject,
    IsLayerModelObject,
    IsRenderInline,
    IsRenderBlock,
    IsFlexibleBox
  };

  enum class RenderObjectBlockFlowFlag : uint8
  {
    IsFragmentContainer,
    IsFragmentedFlow,
    IsTextControl,
    IsSVGBlock,
    IsViewTransitionContainingBlock
  };

  enum class RenderObjectLineBreakFlag : uint8
  {
    IsWBR
  };

  enum class RenderObjectReplacedFlag : uint8
  {
    IsImage,
    IsMedia,
    IsWidget,
    IsViewTransitionCapture,
    UsesBoundaryCaching
  };

  enum class RenderObjectSVGModelObjectFlag : uint8
  {
    IsLegacy,
    IsContainer,
    IsHiddenContainer,
    IsResourceContainer,
    IsShape,
    UsesBoundaryCaching
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectStateFlag, 20uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectTypeFlag, 8uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectBlockFlowFlag, 5uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectLineBreakFlag, 1uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectReplacedFlag, 5uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectSVGModelObjectFlag, 6uz);

namespace krys::boo
{
  class RenderElement;
  class RenderStyle;

  /// @brief A node of the render tree: the box, text or other object a node (or, for anonymous objects, the
  /// document) is rendered as.
  /// @note FIXME: There's no render tree yet: renderers have no parent, children, layout or painting.
  /// @brief An opaque pointer identifying the image data behind a style image.
  using WrappedImagePtr = const void *;

  class RenderObject : public CanMakeWeakPtr<RenderObject>, public CanMakeCheckedPtr<RenderObject>
  {
    KRYS_NON_COPY_MOVABLE(RenderObject);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderObject);

  public:
    enum class Type : uint8
    {
      BlockFlow,
      Button,
      CombineText,
      Counter,
      DeprecatedFlexibleBox,
      DetailsMarker,
      EmbeddedObject,
      FileUploadControl,
      FlexibleBox,
      Frame,
      FrameSet,
      Grid,
      HTMLCanvas,
      IFrame,
      Image,
      Inline,
      LineBreak,
      ListBox,
      ListItem,
      ListMarker,
      Media,
      MenuList,
      Meter,
      MultiColumnFlow,
      MultiColumnSet,
      MultiColumnSpannerPlaceholder,
      Progress,
      Quote,
      Replica,
      ScrollbarPart,
      SearchField,
      Slider,
      SliderContainer,
      Table,
      TableCaption,
      TableCell,
      TableCol,
      TableRow,
      TableSection,
      Text,
      TextControlInnerBlock,
      TextControlInnerContainer,
      TextControlMultiLine,
      TextControlSingleLine,
      TextFragment,
      VTTCue,
      Video,
      View,
      ViewTransitionCapture,
      MathMLBlock,
      MathMLFenced,
      MathMLFencedOperator,
      MathMLFraction,
      MathMLMath,
      MathMLMenclose,
      MathMLOperator,
      MathMLPadded,
      MathMLRoot,
      MathMLRow,
      MathMLScripts,
      MathMLSpace,
      MathMLTable,
      MathMLToken,
      MathMLUnderOver,
      SVGEllipse,
      SVGForeignObject,
      SVGGradientStop,
      SVGHiddenContainer,
      SVGImage,
      SVGInline,
      SVGInlineText,
      SVGPath,
      SVGRect,
      SVGResourceClipper,
      SVGResourceFilter,
      SVGResourceFilterPrimitive,
      SVGResourceLinearGradient,
      SVGResourceMarker,
      SVGResourceMasker,
      SVGResourcePattern,
      SVGResourceRadialGradient,
      SVGRoot,
      SVGTSpan,
      SVGText,
      SVGTextPath,
      SVGTransformableContainer,
      SVGViewportContainer,
      LegacySVGEllipse,
      LegacySVGForeignObject,
      LegacySVGHiddenContainer,
      LegacySVGImage,
      LegacySVGPath,
      LegacySVGRect,
      LegacySVGResourceClipper,
      LegacySVGResourceFilter,
      LegacySVGResourceFilterPrimitive,
      LegacySVGResourceLinearGradient,
      LegacySVGResourceMarker,
      LegacySVGResourceMasker,
      LegacySVGResourcePattern,
      LegacySVGResourceRadialGradient,
      LegacySVGRoot,
      LegacySVGTransformableContainer,
      LegacySVGViewportContainer
    };

    using TypeFlag = RenderObjectTypeFlag;
    using StateFlag = RenderObjectStateFlag;
    using BlockFlowFlag = RenderObjectBlockFlowFlag;
    using LineBreakFlag = RenderObjectLineBreakFlag;
    using ReplacedFlag = RenderObjectReplacedFlag;
    using SVGModelObjectFlag = RenderObjectSVGModelObjectFlag;

    /// @brief The flags of one kind of renderer.
    class TypeSpecificFlags
    {
    public:
      enum class Kind : uint8
      {
        Invalid = 0,
        BlockFlow,
        LineBreak,
        Replaced,
        SVGModelObject
      };

    private:
      Kind _kind {Kind::Invalid};
      uint8 _flags {0};

    public:
      constexpr TypeSpecificFlags() noexcept = default;

      constexpr TypeSpecificFlags(EnumSet<BlockFlowFlag> flags) noexcept
          : _kind(Kind::BlockFlow), _flags(static_cast<uint8>(flags.ToRaw()))
      {
      }

      constexpr TypeSpecificFlags(EnumSet<LineBreakFlag> flags) noexcept
          : _kind(Kind::LineBreak), _flags(static_cast<uint8>(flags.ToRaw()))
      {
      }

      constexpr TypeSpecificFlags(EnumSet<ReplacedFlag> flags) noexcept
          : _kind(Kind::Replaced), _flags(static_cast<uint8>(flags.ToRaw()))
      {
      }

      constexpr TypeSpecificFlags(EnumSet<SVGModelObjectFlag> flags) noexcept
          : _kind(Kind::SVGModelObject), _flags(static_cast<uint8>(flags.ToRaw()))
      {
      }

      KRYS_NODISCARD constexpr Kind GetKind() const noexcept
      {
        return _kind;
      }

      KRYS_NODISCARD constexpr EnumSet<BlockFlowFlag> BlockFlowFlags() const noexcept
      {
        return EnumSet<BlockFlowFlag>::FromRaw(ValueForKind(Kind::BlockFlow));
      }
      KRYS_NODISCARD constexpr EnumSet<LineBreakFlag> LineBreakFlags() const noexcept
      {
        return EnumSet<LineBreakFlag>::FromRaw(ValueForKind(Kind::LineBreak));
      }
      KRYS_NODISCARD constexpr EnumSet<ReplacedFlag> ReplacedFlags() const noexcept
      {
        return EnumSet<ReplacedFlag>::FromRaw(ValueForKind(Kind::Replaced));
      }
      KRYS_NODISCARD constexpr EnumSet<SVGModelObjectFlag> SVGFlags() const noexcept
      {
        return EnumSet<SVGModelObjectFlag>::FromRaw(ValueForKind(Kind::SVGModelObject));
      }

    private:
      KRYS_NODISCARD constexpr uint8 ValueForKind(Kind kind) const noexcept
      {
        return _kind == kind ? _flags : 0;
      }
    };

    enum class BoxDecorationState : uint8
    {
      None,
      InvalidObscurationStatus,
      IsKnownToBeObscured,
      MayBeVisible,
    };

#pragma region CachedImageClient emulation

    virtual void NotifyFinished(CachedResource &) noexcept
    {
    }
    virtual void ImageChanged(WrappedImagePtr, const IntRect * = nullptr) noexcept
    {
    }
    KRYS_NODISCARD virtual bool AllowsAnimation() const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual bool CanDestroyDecodedData() const noexcept
    {
      return true;
    }
    virtual VisibleInViewportState ImageFrameAvailable(CachedImage &, ImageAnimatingState,
                                                       const IntRect *) noexcept;
    KRYS_NODISCARD virtual VisibleInViewportState ImageVisibleInViewport(const dom::Document &) const noexcept
    {
      return VisibleInViewportState::No;
    }
    virtual void DidRemoveCachedImageClient(CachedImage &) noexcept
    {
    }
    virtual void ImageContentChanged(CachedImage &) noexcept
    {
    }
    virtual void ScheduleRenderingUpdateForImage(CachedImage &) noexcept
    {
    }
    /// @brief The client the renderer is to the cached images it uses.
    KRYS_NODISCARD CachedImageClient &GetCachedImageClient() const noexcept;

#pragma endregion

#pragma region Repaint

    /// @brief The renderer whose coordinates repaint rects are mapped to, and whether a full repaint of it
    /// (or of a layer in between) is already scheduled.
    struct RepaintContainerStatus
    {
      bool FullRepaintIsScheduled {false};
      const RenderLayerModelObject *Renderer {nullptr};
    };
    /// @brief The renderer whose coordinates the renderer's repaints are in: null for the render view.
    // FIXME: Compositing layers, filter layers and fragmented flows, once there are any.
    KRYS_NODISCARD RepaintContainerStatus ContainerForRepaint() const noexcept;
    /// @brief Repaints @p rect, in @p repaintContainer's coordinates (the render view's if null).
    void RepaintUsingContainer(const RenderLayerModelObject *repaintContainer, const LayoutRect &rect,
                               bool shouldClipToLayer = true) const noexcept;

    enum class ForceRepaint : bool
    {
      No,
      Yes
    };
    /// @brief Repaints the renderer: its clipped overflow rect.
    void Repaint(ForceRepaint forceRepaint = ForceRepaint::No) const noexcept;

    enum class ClipRepaintToLayer : bool
    {
      No,
      Yes
    };
    /// @brief Repaints @p repaintRect, in the renderer's coordinates.
    void RepaintRectangle(const LayoutRect &repaintRect, bool shouldClipToLayer = true) const noexcept;
    /// @brief Repaints a renderer with a fixed background, which moves against the contents when they scroll.
    void RepaintSlowRepaintObject() const noexcept;
    void RepaintRectangle(const LayoutRect &repaintRect, ClipRepaintToLayer shouldClipToLayer,
                          ForceRepaint forceRepaint,
                          Maybe<LayoutBoxExtent> additionalRepaintOutsets = null) const noexcept;

    /// @brief The rects a renderer repaints, mapped up to a repaint container.
    struct RepaintRects
    {
      /// @brief Some rect (normally the visual overflow rect) mapped up to the repaint container, respecting
      /// clipping.
      LayoutRect ClippedOverflowRect;
      /// @brief A rect representing the extent of outlines and shadows, mapped to the repaint container, but
      /// not clipped.
      Maybe<LayoutRect> OutlineBoundsRect;

      RepaintRects() noexcept = default;
      explicit RepaintRects(const LayoutRect &rect, const Maybe<LayoutRect> &outlineBounds = null) noexcept
          : ClippedOverflowRect(rect), OutlineBoundsRect(outlineBounds)
      {
      }

      KRYS_NODISCARD bool operator==(const RepaintRects &) const noexcept = default;

      void Move(LayoutSize size) noexcept
      {
        ClippedOverflowRect.Move(size);
        if (OutlineBoundsRect)
        {
          OutlineBoundsRect->Move(size);
        }
      }

      void MoveBy(LayoutPoint point) noexcept
      {
        ClippedOverflowRect.MoveBy(point);
        if (OutlineBoundsRect)
        {
          OutlineBoundsRect->MoveBy(point);
        }
      }

      void Expand(LayoutSize size) noexcept
      {
        ClippedOverflowRect.Expand(size);
        if (OutlineBoundsRect)
        {
          OutlineBoundsRect->Expand(size);
        }
      }

      void EncloseToIntRects() noexcept;

      void Unite(const RepaintRects &other) noexcept
      {
        ClippedOverflowRect.Unite(other.ClippedOverflowRect);
        if (OutlineBoundsRect && other.OutlineBoundsRect)
        {
          OutlineBoundsRect->Unite(*other.OutlineBoundsRect);
        }
      }

      void FlipForWritingMode(LayoutSize containerSize, bool isHorizontalWritingMode) noexcept;

      /// @brief Intersects only the clipped overflow rect; returns whether it's still non-empty.
      bool Intersect(const LayoutRect &clipRect) noexcept
      {
        ClippedOverflowRect.Intersect(clipRect);
        return !ClippedOverflowRect.IsEmpty();
      }

      /// @brief Intersects only the clipped overflow rect, edges included; returns whether they intersect.
      bool EdgeInclusiveIntersect(const LayoutRect &clipRect) noexcept
      {
        return ClippedOverflowRect.EdgeInclusiveIntersect(clipRect);
      }

      void Transform(const TransformationMatrix &matrix) noexcept;
      void Transform(const TransformationMatrix &matrix, float deviceScaleFactor) noexcept;
    };

    /// @brief The rect to repaint whenever the renderer changes, in the view's coordinates: its overflow and
    /// outlines.
    KRYS_NODISCARD LayoutRect AbsoluteClippedOverflowRectForRepaint() const noexcept;
    KRYS_NODISCARD IntRect PixelSnappedAbsoluteClippedOverflowRect() const noexcept;

    KRYS_NODISCARD virtual LayoutRect ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                          VisibleRectContext context) const noexcept;
    KRYS_NODISCARD LayoutRect
      ClippedOverflowRectForRepaint(const RenderLayerModelObject *repaintContainer) const noexcept;
    KRYS_NODISCARD virtual LayoutRect
      RectWithOutlineForRepaint(const RenderLayerModelObject *repaintContainer,
                                LayoutUnit outlineWidth) const noexcept;
    KRYS_NODISCARD virtual LayoutRect
      OutlineBoundsForRepaint(const RenderLayerModelObject * /*repaintContainer*/) const noexcept;

    /// @brief Maps @p rects from the renderer's coordinates to @p repaintContainer's, as @p context says.
    KRYS_NODISCARD RepaintRects ComputeRects(const RepaintRects &rects,
                                             const RenderLayerModelObject *repaintContainer,
                                             VisibleRectContext context) const noexcept;
    KRYS_NODISCARD LayoutRect ComputeRectForRepaint(
      const LayoutRect &rect, const RenderLayerModelObject *repaintContainer) const noexcept;

    KRYS_NODISCARD virtual RepaintRects
      RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                    RepaintOutlineBounds repaintOutlineBounds) const noexcept;

    /// @brief Maps @p rects from the renderer's coordinates to @p container's, where they're visible, clipped
    /// and scrolled as @p context says. With an edge-inclusive intersection, null when they're clipped away
    /// entirely.
    KRYS_NODISCARD virtual Maybe<RepaintRects>
      ComputeVisibleRectsInContainer(const RepaintRects &rects, const RenderLayerModelObject *container,
                                     VisibleRectContext context) const noexcept;

    KRYS_NODISCARD static VisibleRectContext VisibleRectContextForRepaint() noexcept;

    /// @brief Whether the renderer has a layer that paints itself.
    KRYS_NODISCARD bool HasSelfPaintingLayer() const noexcept;

    KRYS_NODISCARD bool HasOutlineAutoAncestor() const noexcept;
    void SetHasOutlineAutoAncestor(bool hasOutlineAutoAncestor = true) noexcept;

    /// @brief Marks the enclosing layer as needing a full repaint after layout.
    void SetLayerNeedsFullRepaint() noexcept;
    void SetLayerNeedsFullRepaintForOutOfFlowMovementLayout() noexcept;

  protected:
    /// @brief The rects the renderer repaints, in its own coordinates.
    KRYS_NODISCARD virtual RepaintRects LocalRectsForRepaint(RepaintOutlineBounds) const noexcept;

    void IssueRepaint(Maybe<LayoutRect> partialRepaintRect = null,
                      ClipRepaintToLayer clipRepaintToLayer = ClipRepaintToLayer::No,
                      ForceRepaint forceRepaint = ForceRepaint::No,
                      Maybe<LayoutBoxExtent> additionalRepaintOutsets = null) const noexcept;

  private:
    void PropagateRepaintToParentWithOutlineAutoIfNeeded(const RenderLayerModelObject &repaintContainer,
                                                         const LayoutRect &repaintRect) const noexcept;

  public:
#pragma endregion

  private:
    friend class RenderElement;
    friend class RenderTreeBuilder;

    // This class is to avoid making RenderObject a client itself.
    class CachedImageListener final : public CachedImageClient
    {
    private:
      RenderObject &_renderer;

    public:
      explicit CachedImageListener(RenderObject &renderer) noexcept : _renderer(renderer)
      {
      }

    private:
      // CachedResourceClient.
      void NotifyFinished(CachedResource &) noexcept final;

      // CachedImageClient.
      void ImageChanged(CachedImage *, const IntRect * = nullptr) noexcept final;
      KRYS_NODISCARD bool AllowsAnimation() const noexcept final;
      KRYS_NODISCARD bool CanDestroyDecodedData() const noexcept final;
      VisibleInViewportState ImageFrameAvailable(CachedImage &, ImageAnimatingState,
                                                 const IntRect *) noexcept final;
      KRYS_NODISCARD VisibleInViewportState
        ImageVisibleInViewport(const dom::Document &) const noexcept final;
      void DidRemoveCachedImageClient(CachedImage &) noexcept final;
      void ImageContentChanged(CachedImage &) noexcept final;
      void ScheduleRenderingUpdateForImage(CachedImage &) noexcept final;
    };

    mutable UniquePtr<CachedImageListener> _cachedImageClient;

    enum class PositionedState : uint8
    {
      IsStaticallyPositioned = 0,
      IsRelativelyPositioned = 1,
      IsOutOfFlowPositioned = 2,
      IsStickilyPositioned = 3
    };

    dom::Node &_node;
    RenderElement *_parent {nullptr};
    RenderObject *_previous {nullptr};
    RenderObject *_next {nullptr};
    EnumSet<TypeFlag> _typeFlags;
    Type _type;
    TypeSpecificFlags _typeSpecificFlags;
    EnumSet<StateFlag> _stateFlags;
    mutable bool _setNeedsLayoutForbidden {false};
    PositionedState _positionedState {PositionedState::IsStaticallyPositioned};
    BoxDecorationState _boxDecorationState {BoxDecorationState::None};
    bool _isBeingDestroyed {false};
    CheckedPtr<layout::Box> _layoutBox;

  public:
    /// @brief Anonymous objects should pass the document as their node, and they will then automatically be
    /// marked as anonymous in the constructor.
    RenderObject(Type type, dom::Node &node, EnumSet<TypeFlag> typeFlags,
                 TypeSpecificFlags typeSpecificFlags) noexcept;
    virtual ~RenderObject() noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD virtual utf8_stringview RenderName() const noexcept = 0;

    KRYS_NODISCARD RenderElement *Parent() const noexcept
    {
      return _parent;
    }

    KRYS_NODISCARD bool IsDescendantOf(const RenderObject *ancestor) const noexcept;

    KRYS_NODISCARD RenderObject *PreviousSibling() const noexcept
    {
      return _previous;
    }
    KRYS_NODISCARD RenderObject *NextSibling() const noexcept
    {
      return _next;
    }

    KRYS_NODISCARD RenderObject *PreviousInFlowSibling() const noexcept;
    KRYS_NODISCARD RenderObject *NextInFlowSibling() const noexcept;

    // Use RenderElement versions instead.
    KRYS_NODISCARD virtual RenderObject *FirstChildSlow() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual RenderObject *LastChildSlow() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD RenderObject *NextInPreOrder() const noexcept;
    KRYS_NODISCARD RenderObject *NextInPreOrder(const RenderObject *stayWithin) const noexcept;
    KRYS_NODISCARD RenderObject *NextInPreOrderAfterChildren() const noexcept;
    KRYS_NODISCARD RenderObject *NextInPreOrderAfterChildren(const RenderObject *stayWithin) const noexcept;
    KRYS_NODISCARD RenderObject *PreviousInPreOrder() const noexcept;
    KRYS_NODISCARD RenderObject *PreviousInPreOrder(const RenderObject *stayWithin) const noexcept;
    KRYS_NODISCARD RenderObject *ChildAt(uint32 index) const noexcept;

    KRYS_NODISCARD RenderObject *FirstLeafChild() const noexcept;
    KRYS_NODISCARD RenderObject *LastLeafChild() const noexcept;

    KRYS_NODISCARD virtual bool CanHaveChildren() const noexcept = 0;
    KRYS_NODISCARD virtual bool CreatesAnonymousWrapper() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool CanHaveGeneratedChildren() const noexcept;

    /// @brief The render tree's root.
    KRYS_NODISCARD RenderView &View() const noexcept;

    KRYS_NODISCARD layout::Box *LayoutBox() noexcept;
    KRYS_NODISCARD const layout::Box *LayoutBox() const noexcept;
    void SetLayoutBox(layout::Box &box) noexcept;
    void ClearLayoutBox() noexcept;

    KRYS_NODISCARD RenderElement *FirstNonAnonymousAncestor() const noexcept;

    /// @brief Whether the object is in its document's render tree.
    KRYS_NODISCARD bool IsRooted() const noexcept;

    KRYS_NODISCARD bool BeingDestroyed() const noexcept
    {
      return _isBeingDestroyed;
    }

    KRYS_NODISCARD bool RenderTreeBeingDestroyed() const noexcept;

    /// @brief Destroys the object, which must have been detached from the render tree.
    void Destroy() noexcept;

    virtual void InsertedIntoTree() noexcept;
    virtual void WillBeRemovedFromTree() noexcept;

    KRYS_NODISCARD bool IsRenderElement() const noexcept
    {
      return !IsRenderText();
    }
    KRYS_NODISCARD bool IsRenderText() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsText);
    }
    KRYS_NODISCARD bool IsRenderBoxModelObject() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsBoxModelObject);
    }
    KRYS_NODISCARD bool IsRenderBlock() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsRenderBlock);
    }
    KRYS_NODISCARD bool IsRenderInline() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsRenderInline);
    }
    KRYS_NODISCARD bool IsRenderBox() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsBox);
    }
    KRYS_NODISCARD bool IsRenderGrid() const noexcept
    {
      return _type == Type::Grid;
    }
    KRYS_NODISCARD bool IsRenderButton() const noexcept
    {
      return _type == Type::Button;
    }
    KRYS_NODISCARD bool IsRenderFrameSet() const noexcept
    {
      return _type == Type::FrameSet;
    }
    KRYS_NODISCARD bool IsRenderLayerModelObject() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsLayerModelObject);
    }
    KRYS_NODISCARD bool IsRenderReplaced() const noexcept
    {
      return _typeSpecificFlags.GetKind() == TypeSpecificFlags::Kind::Replaced;
    }
    KRYS_NODISCARD bool IsRenderImage() const noexcept
    {
      return IsRenderReplaced() && _typeSpecificFlags.ReplacedFlags().Contains(ReplacedFlag::IsImage);
    }
    KRYS_NODISCARD bool IsRenderBlockFlow() const noexcept
    {
      return _typeSpecificFlags.GetKind() == TypeSpecificFlags::Kind::BlockFlow;
    }
    KRYS_NODISCARD bool IsRenderFlexibleBox() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsFlexibleBox);
    }
    KRYS_NODISCARD bool IsRenderDeprecatedFlexibleBox() const noexcept
    {
      return _type == Type::DeprecatedFlexibleBox;
    }
    KRYS_NODISCARD bool IsFlexibleBoxIncludingDeprecated() const noexcept
    {
      return IsRenderFlexibleBox() || IsRenderDeprecatedFlexibleBox();
    }
    KRYS_NODISCARD bool IsRenderListItem() const noexcept
    {
      return _type == Type::ListItem;
    }
    KRYS_NODISCARD bool IsRenderListMarker() const noexcept
    {
      return _type == Type::ListMarker;
    }
    KRYS_NODISCARD bool IsRenderLineBreak() const noexcept
    {
      return _type == Type::LineBreak;
    }
    /// @brief Whether this is the line break of a `<br>`.
    KRYS_NODISCARD bool IsBR() const noexcept
    {
      return IsRenderLineBreak() && !HasWBRLineBreakFlag();
    }
    /// @brief Whether this is the line break opportunity of a `<wbr>`.
    KRYS_NODISCARD bool IsWBR() const noexcept
    {
      return IsRenderLineBreak() && HasWBRLineBreakFlag();
    }
    KRYS_NODISCARD bool IsLineBreakOpportunity() const noexcept
    {
      return IsRenderLineBreak() && IsWBR();
    }
    KRYS_NODISCARD bool IsRenderTextOrLineBreak() const noexcept
    {
      return IsRenderText() || IsRenderLineBreak();
    }
    KRYS_NODISCARD bool IsRenderTable() const noexcept
    {
      return _type == Type::Table;
    }
    KRYS_NODISCARD bool IsRenderTableCell() const noexcept
    {
      return _type == Type::TableCell;
    }
    KRYS_NODISCARD bool IsRenderTableCol() const noexcept
    {
      return _type == Type::TableCol;
    }
    KRYS_NODISCARD bool IsRenderTableCaption() const noexcept
    {
      return _type == Type::TableCaption;
    }
    KRYS_NODISCARD bool IsRenderTableSection() const noexcept
    {
      return _type == Type::TableSection;
    }
    KRYS_NODISCARD bool IsRenderTableRow() const noexcept
    {
      return _type == Type::TableRow;
    }
    KRYS_NODISCARD bool IsTablePart() const noexcept
    {
      return IsRenderTableCell() || IsRenderTableCol() || IsRenderTableCaption() || IsRenderTableRow()
             || IsRenderTableSection();
    }
    KRYS_NODISCARD bool IsRenderFragmentedFlow() const noexcept
    {
      return IsRenderBlockFlow()
             && _typeSpecificFlags.BlockFlowFlags().Contains(BlockFlowFlag::IsFragmentedFlow);
    }
    KRYS_NODISCARD bool IsViewTransitionContainingBlock() const noexcept
    {
      return IsRenderBlockFlow()
             && _typeSpecificFlags.BlockFlowFlags().Contains(BlockFlowFlag::IsViewTransitionContainingBlock);
    }
    KRYS_NODISCARD bool IsRenderView() const noexcept
    {
      return _type == Type::View;
    }
    KRYS_NODISCARD bool IsRenderTextFragment() const noexcept
    {
      return _type == Type::TextFragment;
    }
    KRYS_NODISCARD bool IsRenderCounter() const noexcept
    {
      return _type == Type::Counter;
    }
    KRYS_NODISCARD bool IsRenderQuote() const noexcept
    {
      return _type == Type::Quote;
    }
    KRYS_NODISCARD bool IsLegacyRenderSVGModelObject() const noexcept
    {
      return _typeSpecificFlags.GetKind() == TypeSpecificFlags::Kind::SVGModelObject
             && _typeSpecificFlags.SVGFlags().Contains(SVGModelObjectFlag::IsLegacy);
    }
    KRYS_NODISCARD bool IsRenderSVGModelObject() const noexcept
    {
      return _typeSpecificFlags.GetKind() == TypeSpecificFlags::Kind::SVGModelObject
             && !_typeSpecificFlags.SVGFlags().Contains(SVGModelObjectFlag::IsLegacy);
    }
    KRYS_NODISCARD bool IsRenderOrLegacyRenderSVGModelObject() const noexcept
    {
      return IsRenderSVGModelObject() || IsLegacyRenderSVGModelObject();
    }

    KRYS_NODISCARD bool IsAnonymous() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsAnonymous);
    }

    KRYS_NODISCARD bool IsFloating() const noexcept
    {
      return _stateFlags.Contains(StateFlag::Floating);
    }

    KRYS_NODISCARD bool IsPositioned() const noexcept
    {
      return _positionedState != PositionedState::IsStaticallyPositioned;
    }
    KRYS_NODISCARD bool IsInFlowPositioned() const noexcept
    {
      return IsRelativelyPositioned() || IsStickilyPositioned();
    }
    // absolute or fixed positioning
    KRYS_NODISCARD bool IsOutOfFlowPositioned() const noexcept
    {
      return _positionedState == PositionedState::IsOutOfFlowPositioned;
    }
    KRYS_NODISCARD bool IsRelativelyPositioned() const noexcept
    {
      return _positionedState == PositionedState::IsRelativelyPositioned;
    }
    KRYS_NODISCARD bool IsStickilyPositioned() const noexcept
    {
      return _positionedState == PositionedState::IsStickilyPositioned;
    }

    KRYS_NODISCARD bool IsFloatingOrOutOfFlowPositioned() const noexcept
    {
      return IsFloating() || IsOutOfFlowPositioned();
    }
    KRYS_NODISCARD bool IsInFlow() const noexcept
    {
      return !IsFloatingOrOutOfFlowPositioned();
    }

    // inline object
    KRYS_NODISCARD bool IsInline() const noexcept
    {
      return !_stateFlags.Contains(StateFlag::IsBlock);
    }
    KRYS_NODISCARD bool IsBlockLevelReplacedOrAtomicInline() const noexcept
    {
      return _stateFlags.Contains(StateFlag::IsBlockLevelReplacedOrAtomicInline);
    }
    KRYS_NODISCARD bool IsHorizontalWritingMode() const noexcept
    {
      return !_stateFlags.Contains(StateFlag::VerticalWritingMode);
    }

    KRYS_NODISCARD bool IsAtomicInlineLevelBox() const noexcept;
    KRYS_NODISCARD bool IsNonReplacedAtomicInlineLevelBox() const noexcept;

    KRYS_NODISCARD bool IsExcludedFromNormalLayout() const noexcept
    {
      return _stateFlags.Contains(StateFlag::IsExcludedFromNormalLayout);
    }
    void SetIsExcludedFromNormalLayout(bool excluded) noexcept
    {
      _stateFlags.Set(StateFlag::IsExcludedFromNormalLayout, excluded);
    }

    KRYS_NODISCARD bool ChildrenInline() const noexcept
    {
      return _stateFlags.Contains(StateFlag::ChildrenInline);
    }
    virtual void SetChildrenInline(bool b) noexcept
    {
      _stateFlags.Set(StateFlag::ChildrenInline, b);
    }

    KRYS_NODISCARD bool IsComposited() const noexcept;

    KRYS_NODISCARD bool HasLayer() const noexcept
    {
      return _stateFlags.Contains(StateFlag::HasLayer);
    }

    KRYS_NODISCARD BoxDecorationState GetBoxDecorationState() const noexcept
    {
      return _boxDecorationState;
    }
    KRYS_NODISCARD bool HasVisibleBoxDecorations() const noexcept
    {
      return _boxDecorationState != BoxDecorationState::None;
    }

    KRYS_NODISCARD bool HasNonVisibleOverflow() const noexcept
    {
      return _stateFlags.Contains(StateFlag::HasNonVisibleOverflow);
    }

    // Transform, perspective or transform-style: preserve-3d.
    KRYS_NODISCARD bool HasTransformRelatedProperty() const noexcept
    {
      return _stateFlags.Contains(StateFlag::HasTransformRelatedProperty);
    }
    KRYS_NODISCARD bool IsTransformed() const noexcept;

    KRYS_NODISCARD RenderLayer *EnclosingLayer() const noexcept;
    /// @brief The nearest box that is this renderer or one of its ancestors.
    KRYS_NODISCARD RenderBox &EnclosingBox() const noexcept;

    KRYS_NODISCARD bool IsSkippedContent() const noexcept;

    /// @brief Hit tests the renderer in each of the hit test actions, foreground first, as @p hitTestFilter
    /// allows.
    bool HitTest(const HitTestRequest &request, HitTestResult &result,
                 const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                 HitTestFilter hitTestFilter = HitTestAll) noexcept;
    /// @brief Whether @p locationInContainer hits the renderer (or its descendants) in @p hitTestAction,
    /// setting
    /// @p result's inner node when it does.
    virtual bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                             const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                             HitTestAction hitTestAction) noexcept;
    /// @brief Sets @p result's inner node to this renderer's node (if it has none yet), @p point being the
    /// hit point in the renderer's coordinates.
    virtual void UpdateHitTestResult(HitTestResult &result, const LayoutPoint &point) const noexcept;
    /// @brief The node hit testing reports for the renderer: its node, or for anonymous renderers in
    /// generated content, the pseudo-element's.
    KRYS_NODISCARD virtual dom::Node *NodeForHitTest() const noexcept;
    KRYS_NODISCARD bool HasTransformOrPerspective() const noexcept;

    KRYS_NODISCARD bool PaintContainmentApplies() const noexcept
    {
      return _stateFlags.Contains(StateFlag::PaintContainmentApplies);
    }

    KRYS_NODISCARD bool IsDocumentElementRenderer() const noexcept;
    KRYS_NODISCARD bool IsFieldset() const noexcept;
    KRYS_NODISCARD bool IsLegend() const noexcept;
    KRYS_NODISCARD bool IsExcludedAndPlacedInBorder() const noexcept
    {
      return IsExcludedFromNormalLayout() && IsLegend();
    }

    void InvalidateBackgroundObscurationStatus() noexcept
    {
      if (!HasVisibleBoxDecorations())
      {
        return;
      }
      _boxDecorationState = BoxDecorationState::InvalidObscurationStatus;
    }

    KRYS_NODISCARD const RenderStyle &FirstLineStyle() const noexcept;
    KRYS_NODISCARD virtual const RenderStyle &OutlineStyleForRepaint() const noexcept;
    KRYS_NODISCARD bool IsBody() const noexcept;
    KRYS_NODISCARD bool IsPseudoElement() const noexcept;

    // Returns the object containing this one. Can be different from parent for positioned elements.
    KRYS_NODISCARD RenderElement *Container() const noexcept;
    /// @brief The object containing this one, and whether @p repaintContainer was skipped to reach it.
    KRYS_NODISCARD RenderElement *Container(const RenderLayerModelObject *repaintContainer,
                                            bool &repaintContainerSkipped) const noexcept;

#pragma region Geometry mapping

    /// @brief Converts a point in this renderer's coordinates to absolute (render view) coordinates.
    KRYS_NODISCARD FloatPoint LocalToAbsolute(const FloatPoint &localPoint,
                                              EnumSet<MapCoordinatesMode> mode = {},
                                              bool *wasFixed = nullptr) const noexcept;
    KRYS_NODISCARD FloatPoint LocalToAbsolute() const noexcept;
    /// @brief Converts an absolute point to this renderer's coordinates.
    KRYS_NODISCARD FloatPoint AbsoluteToLocal(const FloatPoint &containerPoint,
                                              EnumSet<MapCoordinatesMode> mode = {}) const noexcept;
    KRYS_NODISCARD FloatQuad AbsoluteToLocalQuad(const FloatQuad &quad,
                                                 EnumSet<MapCoordinatesMode> mode = {}) const noexcept;

    /// @brief Converts a quad in this renderer's coordinates to absolute (render view) coordinates.
    KRYS_NODISCARD FloatQuad LocalToAbsoluteQuad(
      const FloatQuad &quad, EnumSet<MapCoordinatesMode> mode = MapCoordinatesMode::UseTransforms,
      bool *wasFixed = nullptr) const noexcept;
    /// @brief Converts a quad in this renderer's coordinates to @p container's (absolute if null).
    KRYS_NODISCARD FloatQuad
      LocalToContainerQuad(const FloatQuad &localQuad, const RenderLayerModelObject *container,
                           EnumSet<MapCoordinatesMode> mode = MapCoordinatesMode::UseTransforms,
                           bool *wasFixed = nullptr) const noexcept;
    KRYS_NODISCARD FloatPoint
      LocalToContainerPoint(const FloatPoint &localPoint, const RenderLayerModelObject *container,
                            EnumSet<MapCoordinatesMode> mode = MapCoordinatesMode::UseTransforms,
                            bool *wasFixed = nullptr) const noexcept;

    /// @brief Maps the coordinates in @p transformState from this renderer's space to @p ancestorContainer's
    /// (absolute if null).
    virtual void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                     TransformState &transformState, EnumSet<MapCoordinatesMode> mode,
                                     bool *wasFixed) const noexcept;
    /// @brief Maps the coordinates in @p transformState from absolute coordinates to this renderer's.
    virtual void MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                         TransformState &transformState) const noexcept;

    /// @brief The offset from @p container (which must be this renderer's container) to this renderer.
    KRYS_NODISCARD virtual LayoutSize
      OffsetFromContainer(const RenderElement &container, const LayoutPoint &point,
                          bool *offsetDependsOnPoint = nullptr) const noexcept;
    /// @brief The offset from an ancestor @p container, without transforms, to this renderer.
    KRYS_NODISCARD LayoutSize OffsetFromAncestorContainer(const RenderElement &container) const noexcept;

    /// @brief Appends the renderer's border boxes, or line boxes, offset by @p offsetFromRoot.
    virtual void BoundingRects(List<LayoutRect> &, const LayoutPoint & /* offsetFromRoot */) const noexcept
    {
    }
    /// @brief Appends the renderer's border boxes, or line boxes, in absolute coordinates.
    virtual void AbsoluteQuads(List<FloatQuad> &, bool * /*wasFixed*/ = nullptr) const noexcept
    {
    }

    /// @brief The bounding box of the renderer in absolute coordinates.
    KRYS_NODISCARD IntRect AbsoluteBoundingBoxRect(bool useTransforms = true,
                                                   bool *wasFixed = nullptr) const noexcept;
    KRYS_NODISCARD IntRect AbsoluteBoundingBoxRectIgnoringTransforms() const noexcept;

    KRYS_NODISCARD bool ShouldUseTransformFromContainer(const RenderElement *containerObject) const noexcept;
    void GetTransformFromContainer(const LayoutSize &offsetInContainer,
                                   TransformationMatrix &transform) const noexcept;
    void PushOntoTransformState(TransformState &transformState, EnumSet<MapCoordinatesMode> mode,
                                const RenderLayerModelObject *repaintContainer,
                                const RenderElement *container, const LayoutSize &offsetInContainer,
                                bool containerSkipped) const noexcept;
    KRYS_NODISCARD bool ParticipatesInPreserve3D() const noexcept;

#pragma endregion

    KRYS_NODISCARD RenderBlock *ContainingBlock() const noexcept;
    KRYS_NODISCARD static RenderBlock *ContainingBlockForPositionType(PositionType positionType,
                                                                      const RenderObject &renderer) noexcept;

    /// @brief The node rendered, or null for an anonymous object.
    KRYS_NODISCARD dom::Node *GetNode() const noexcept
    {
      if (IsAnonymous())
      {
        return nullptr;
      }
      return &_node;
    }

    KRYS_NODISCARD dom::Node &NodeForNonAnonymous() const noexcept
    {
      krys_debug_assert(!IsAnonymous());
      return _node;
    }

    /// @brief The node rendered, or null for an anonymous object or a pseudo-element.
    KRYS_NODISCARD dom::Node *NonPseudoNode() const noexcept;
    KRYS_NODISCARD dom::Element *NonPseudoElement() const noexcept;

    KRYS_NODISCARD dom::Document &GetDocument() const noexcept;

    /// @brief The style the object is rendered with: an element's own, or a text's parent's.
    KRYS_NODISCARD const RenderStyle &Style() const noexcept;
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept;

#pragma region Editing

    /// @brief The position nearest @p point (in this renderer's coordinates), for hit testing to positions.
    KRYS_NODISCARD virtual dom::PositionWithAffinity PositionForPoint(const LayoutPoint &point,
                                                                      HitTestSource source) noexcept;
    KRYS_NODISCARD VisiblePosition VisiblePositionForPoint(const LayoutPoint &point,
                                                           HitTestSource source) noexcept;
    /// @brief The position at @p offset in this renderer's node, or near it if the renderer is anonymous; a
    /// visually equivalent editable position is preferred.
    KRYS_NODISCARD dom::PositionWithAffinity CreatePositionWithAffinity(int32 offset,
                                                                        Affinity affinity) const noexcept;
    KRYS_NODISCARD dom::PositionWithAffinity
      CreatePositionWithAffinity(const dom::Position &position) const noexcept;

    KRYS_NODISCARD bool IsHR() const noexcept;

    KRYS_NODISCARD virtual int32 CaretMinOffset() const noexcept;
    KRYS_NODISCARD virtual int32 CaretMaxOffset() const noexcept;

    KRYS_NODISCARD virtual int32 PreviousOffset(int32 current) const noexcept;
    KRYS_NODISCARD virtual int32 PreviousOffsetForBackwardDeletion(int32 current) const noexcept;
    KRYS_NODISCARD virtual int32 NextOffset(int32 current) const noexcept;

#pragma endregion

#pragma region Layout bits

    using MarkingBehavior = RenderObjectMarkingBehavior;
    enum class HadSkippedLayout : bool
    {
      No,
      Yes
    };

    KRYS_NODISCARD bool EverHadLayout() const noexcept
    {
      return _stateFlags.Contains(StateFlag::EverHadLayout);
    }
    KRYS_NODISCARD Maybe<bool> WasSkippedDuringLastLayoutDueToContentVisibility() const noexcept
    {
      return EverHadLayout() ? Maybe<bool> {_stateFlags.Contains(
                                 StateFlag::WasSkippedDuringLastLayoutDueToContentVisibility)}
                             : null;
    }

    KRYS_NODISCARD bool NeedsLayout() const noexcept
    {
      return SelfNeedsLayout() || NormalChildNeedsLayout() || OutOfFlowChildNeedsLayout()
             || NeedsSimplifiedNormalFlowLayout() || NeedsOutOfFlowMovementLayout();
    }
    KRYS_NODISCARD bool NeedsPreferredLogicalWidthsUpdate() const noexcept
    {
      return _stateFlags.Contains(StateFlag::PreferredLogicalWidthsNeedUpdate);
    }

    KRYS_NODISCARD bool SelfNeedsLayout() const noexcept
    {
      return _stateFlags.Contains(StateFlag::NeedsLayout);
    }
    KRYS_NODISCARD bool NeedsOutOfFlowMovementLayout() const noexcept
    {
      return _stateFlags.Contains(StateFlag::NeedsOutOfFlowMovementLayout);
    }
    KRYS_NODISCARD bool NeedsOutOfFlowMovementLayoutOnly() const noexcept
    {
      return NeedsOutOfFlowMovementLayout() && !SelfNeedsLayout() && !NormalChildNeedsLayout()
             && !OutOfFlowChildNeedsLayout() && !NeedsSimplifiedNormalFlowLayout();
    }

    KRYS_NODISCARD bool OutOfFlowChildNeedsLayout() const noexcept
    {
      return _stateFlags.Contains(StateFlag::OutOfFlowChildNeedsLayout);
    }
    KRYS_NODISCARD bool NeedsSimplifiedNormalFlowLayout() const noexcept
    {
      return _stateFlags.Contains(StateFlag::NeedsSimplifiedNormalFlowLayout);
    }
    KRYS_NODISCARD bool NeedsSimplifiedNormalFlowLayoutOnly() const noexcept
    {
      return NeedsSimplifiedNormalFlowLayout() && !SelfNeedsLayout() && !NormalChildNeedsLayout()
             && !OutOfFlowChildNeedsLayout() && !NeedsOutOfFlowMovementLayout();
    }
    KRYS_NODISCARD bool NeedsNormalChildOrSimplifiedLayoutOnly() const noexcept
    {
      return (NormalChildNeedsLayout() || NeedsSimplifiedNormalFlowLayout()) && !SelfNeedsLayout()
             && !OutOfFlowChildNeedsLayout() && !NeedsOutOfFlowMovementLayout();
    }
    KRYS_NODISCARD bool NormalChildNeedsLayout() const noexcept
    {
      return _stateFlags.Contains(StateFlag::NormalChildNeedsLayout);
    }
    KRYS_NODISCARD bool OutOfFlowChildNeedsStaticPositionLayout() const noexcept
    {
      return _stateFlags.Contains(StateFlag::OutOfFlowChildNeedsStaticPositionLayout);
    }

    /// @brief Marks the containing blocks of this renderer as having a child that needs layout, up to
    /// @p layoutRoot or the first layout boundary.
    /// @return The renderer to lay out from, or null if the chain was already marked or isn't rooted.
    RenderElement *MarkContainingBlocksForLayout(RenderElement *layoutRoot = nullptr) noexcept;
    void SetNeedsLayout(MarkingBehavior markParents = MarkingBehavior::MarkContainingBlockChain) noexcept;
    void ClearNeedsLayout(HadSkippedLayout hadSkippedLayout = HadSkippedLayout::No) noexcept;
    void SetNeedsPreferredWidthsUpdate(
      MarkingBehavior markParents = MarkingBehavior::MarkContainingBlockChain) noexcept;
    void ClearNeedsPreferredWidthsUpdate() noexcept
    {
      _stateFlags.Remove(StateFlag::PreferredLogicalWidthsNeedUpdate);
    }
    void SetNeedsLayoutAndPreferredWidthsUpdate() noexcept
    {
      SetNeedsLayout();
      SetNeedsPreferredWidthsUpdate();
    }

    /// @brief Asserts if the renderer is marked as needing layout while the scope lives.
    class SetLayoutNeededForbiddenScope
    {
      KRYS_NON_COPY_MOVABLE(SetLayoutNeededForbiddenScope);

    private:
      const RenderObject &_renderObject;
      bool _preexistingForbidden;

    public:
      explicit SetLayoutNeededForbiddenScope(const RenderObject &renderObject,
                                             bool isForbidden = true) noexcept;
      ~SetLayoutNeededForbiddenScope() noexcept;
    };
    KRYS_NODISCARD bool IsSetNeedsLayoutForbidden() const noexcept
    {
      return _setNeedsLayoutForbidden;
    }

#pragma endregion

  protected:
    static void ScheduleLayout(RenderElement *layoutRoot) noexcept;

    void SetNeedsOutOfFlowMovementLayoutBit(bool b) noexcept
    {
      _stateFlags.Set(StateFlag::NeedsOutOfFlowMovementLayout, b);
    }
    void SetNormalChildNeedsLayoutBit(bool b) noexcept
    {
      _stateFlags.Set(StateFlag::NormalChildNeedsLayout, b);
    }
    void SetOutOfFlowChildNeedsLayoutBit(bool b) noexcept
    {
      _stateFlags.Set(StateFlag::OutOfFlowChildNeedsLayout, b);
    }
    void SetNeedsSimplifiedNormalFlowLayoutBit(bool b) noexcept
    {
      _stateFlags.Set(StateFlag::NeedsSimplifiedNormalFlowLayout, b);
    }
    void SetOutOfFlowChildNeedsStaticPositionLayoutBit(bool b) noexcept
    {
      _stateFlags.Set(StateFlag::OutOfFlowChildNeedsStaticPositionLayout, b);
    }
    void SetEverHadLayout() noexcept
    {
      _stateFlags.Add(StateFlag::EverHadLayout);
    }
    void SetHadSkippedLayout(bool b) noexcept
    {
      _stateFlags.Set(StateFlag::WasSkippedDuringLastLayoutDueToContentVisibility, b);
    }
    void InvalidateContainerPreferredLogicalWidths() noexcept;

    void SetNeedsLayoutIsForbidden(bool flag) const noexcept
    {
      _setNeedsLayoutForbidden = flag;
    }

    /// @brief The data few renderers need, kept out of line.
    struct RareData
    {
      // Dirty bit was set with MarkingBehavior::MarkOnlyThis
      bool PreferredLogicalWidthsNeedUpdateIsMarkOnlyThis {false};
      EnumSet<style::MarginTrimSide> TrimmedMargins;
      bool HasOutlineAutoAncestor {false};
      // FIXME: Reflections, referenced SVG resources, the ::backdrop renderer and the
      // control part, once there are any.
    };
    KRYS_NODISCARD bool HasRareData() const noexcept
    {
      return _stateFlags.Contains(StateFlag::HasRareData);
    }
    KRYS_NODISCARD const RareData &GetRareData() const noexcept;
    KRYS_NODISCARD RareData &EnsureRareData() noexcept;
    void RemoveRareData() noexcept;

  private:
    // Entries are removed when their renderer is destroyed.
    using RareDataMap = HashMap<const RenderObject *, UniquePtr<RareData>>;
    KRYS_NODISCARD static RareDataMap &GetRareDataMap() noexcept;

  protected:
    void SetPreviousSibling(RenderObject *previous) noexcept
    {
      _previous = previous;
    }
    void SetNextSibling(RenderObject *next) noexcept
    {
      _next = next;
    }
    void SetParent(RenderElement *parent) noexcept;

    KRYS_NODISCARD bool HasWBRLineBreakFlag() const noexcept
    {
      return _typeSpecificFlags.LineBreakFlags().Contains(LineBreakFlag::IsWBR);
    }

    virtual void WillBeDestroyed() noexcept;

    void SetPositionState(PositionType position) noexcept;
    void ClearPositionedState() noexcept
    {
      _positionedState = PositionedState::IsStaticallyPositioned;
    }

    void SetFloating(bool b = true) noexcept
    {
      _stateFlags.Set(StateFlag::Floating, b);
    }
    void SetInline(bool b) noexcept
    {
      _stateFlags.Set(StateFlag::IsBlock, !b);
    }

    void SetHasVisibleBoxDecorations(bool b = true) noexcept;

    void SetBlockLevelReplacedOrAtomicInline(bool b = true) noexcept
    {
      _stateFlags.Set(StateFlag::IsBlockLevelReplacedOrAtomicInline, b);
    }
    void SetHorizontalWritingMode(bool b = true) noexcept
    {
      _stateFlags.Set(StateFlag::VerticalWritingMode, !b);
    }
    void SetHasNonVisibleOverflow(bool b = true) noexcept
    {
      _stateFlags.Set(StateFlag::HasNonVisibleOverflow, b);
    }
    void SetHasLayer(bool b = true) noexcept
    {
      _stateFlags.Set(StateFlag::HasLayer, b);
    }
    void SetHasTransformRelatedProperty(bool b = true) noexcept
    {
      _stateFlags.Set(StateFlag::HasTransformRelatedProperty, b);
    }
    void SetPaintContainmentApplies(bool value = true) noexcept
    {
      _stateFlags.Set(StateFlag::PaintContainmentApplies, value);
    }

    KRYS_NODISCARD EnumSet<TypeFlag> TypeFlags() const noexcept
    {
      return _typeFlags;
    }
    KRYS_NODISCARD TypeSpecificFlags GetTypeSpecificFlags() const noexcept
    {
      return _typeSpecificFlags;
    }
  };
}
