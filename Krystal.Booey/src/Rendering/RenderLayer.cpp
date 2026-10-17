#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <algorithm>

namespace krys::boo
{
  /// @brief The clip rects a layer passes to its child layers: the overflow clip, and the clips of fixed and
  /// absolutely positioned descendants.
  class ClipRects : public RefCounted<ClipRects>
  {
  private:
    bool _fixed {false};
    ClipRect _overflowClipRect;
    ClipRect _fixedClipRect;
    ClipRect _posClipRect;

  public:
    ClipRects() noexcept = default;

    ClipRects(const ClipRects &other) noexcept
        : RefCounted(), _fixed(other.Fixed()), _overflowClipRect(other.OverflowClipRect()),
          _fixedClipRect(other.FixedClipRect()), _posClipRect(other.PosClipRect())
    {
    }

    KRYS_NODISCARD static Ref<ClipRects> Create() noexcept
    {
      return AdoptRef(*new ClipRects);
    }

    void Reset() noexcept
    {
      _overflowClipRect.Reset();
      _fixedClipRect.Reset();
      _posClipRect.Reset();
      _fixed = false;
    }

    KRYS_NODISCARD const ClipRect &OverflowClipRect() const noexcept
    {
      return _overflowClipRect;
    }
    void SetOverflowClipRect(const ClipRect &clipRect) noexcept
    {
      _overflowClipRect = clipRect;
    }

    KRYS_NODISCARD const ClipRect &FixedClipRect() const noexcept
    {
      return _fixedClipRect;
    }
    void SetFixedClipRect(const ClipRect &clipRect) noexcept
    {
      _fixedClipRect = clipRect;
    }

    KRYS_NODISCARD const ClipRect &PosClipRect() const noexcept
    {
      return _posClipRect;
    }
    void SetPosClipRect(const ClipRect &clipRect) noexcept
    {
      _posClipRect = clipRect;
    }

    KRYS_NODISCARD bool Fixed() const noexcept
    {
      return _fixed;
    }
    void SetFixed(bool fixed) noexcept
    {
      _fixed = fixed;
    }

    void SetOverflowClipRectAffectedByRadius() noexcept
    {
      _overflowClipRect.SetAffectedByRadius(true);
    }

    KRYS_NODISCARD bool operator==(const ClipRects &other) const noexcept
    {
      return _overflowClipRect == other.OverflowClipRect() && _fixedClipRect == other.FixedClipRect()
             && _posClipRect == other.PosClipRect() && _fixed == other.Fixed();
    }

    ClipRects &operator=(const ClipRects &other) noexcept
    {
      _overflowClipRect = other.OverflowClipRect();
      _fixedClipRect = other.FixedClipRect();
      _posClipRect = other.PosClipRect();
      _fixed = other.Fixed();
      return *this;
    }
  };

  /// @brief A layer's cached clip rects, per clip rects type and whether the overflow clip is respected.
  class ClipRectsCache
  {
  private:
    Array<RefPtr<ClipRects>, NumCachedClipRectsTypes * 2> _clipRects;

  public:
#if KRYS_DEBUG
    Array<const RenderLayer *, NumCachedClipRectsTypes> ClipRectsRoot {};
#endif

    KRYS_NODISCARD ClipRects *GetClipRects(const RenderLayer::ClipRectsContext &context) const noexcept
    {
      return _clipRects[GetIndex(context.Type, context.RespectOverflowClip())].get();
    }

    void SetClipRects(ClipRectsType clipRectsType, bool respectOverflowClip,
                      RefPtr<ClipRects> &&clipRects) noexcept
    {
      _clipRects[GetIndex(clipRectsType, respectOverflowClip)] = krys::move(clipRects);
    }

  private:
    KRYS_NODISCARD static uint32 GetIndex(ClipRectsType clipRectsType, bool respectOverflowClip) noexcept
    {
      auto index = static_cast<uint32>(clipRectsType);
      if (respectOverflowClip)
      {
        index += static_cast<uint32>(NumCachedClipRectsTypes);
      }
      krys_assert(index < NumCachedClipRectsTypes * 2);
      return index;
    }
  };

  void MakeMatrixRenderable(TransformationMatrix &matrix, bool has3DRendering) noexcept
  {
    if (!has3DRendering)
    {
      matrix.MakeAffine();
    }
  }

  RenderLayer::RenderLayer(RenderLayerModelObject &renderer) noexcept
      : _isRenderViewLayer(renderer.IsRenderView()),
        // FIXME: Media renderers force a stacking context, once there are any.
        _forcedStackingContext(false), _renderer(renderer)
  {
    SetIsNormalFlowOnly(ShouldBeNormalFlowOnly());
    SetIsCSSStackingContext(ShouldBeCSSStackingContext());
    SetNeedsPositionUpdate();

    _isSelfPaintingLayer = ShouldBeSelfPaintingLayer();

    auto needsVisibleContentStatusUpdate = [&]()
    {
      if (renderer.FirstChild())
      {
        return false;
      }

      // Leave m_visibleContentStatusDirty = true in any case. The associated renderer needs to be inserted
      // into the render tree, before we can determine the visible content status.

      // We need the parent to know if we have skipped content or content-visibility root.
      if (renderer.Style().IsSkippedRootOrSkippedContent() && !renderer.Parent())
      {
        return false;
      }
      return true;
    }();

    if (needsVisibleContentStatusUpdate)
    {
      _visibleContentStatusDirty = false;
      _hasVisibleContent = renderer.Style().UsedVisibility() == Visibility::Visible;
    }
  }

  RenderLayer::~RenderLayer() noexcept
  {
    // FIXME: The scrollable area, filters, clip path and backing, once there are any.

    // Child layers will be deleted by their corresponding render objects, so
    // we don't need to delete them ourselves.

    // Layer and all its children should be removed from the tree before destruction.
    krys_assert(Renderer().RenderTreeBeingDestroyed() || !Parent());
    krys_assert(Renderer().RenderTreeBeingDestroyed() || !FirstChild());
  }

  UniquePtr<RenderLayer> RenderLayer::Create(RenderLayerModelObject &renderer) noexcept
  {
    return CreateUnique<RenderLayer>(renderer);
  }

  RenderBox *RenderLayer::GetRenderBox() const noexcept
  {
    return DynamicDowncast<RenderBox>(Renderer());
  }

  void RenderLayer::SetNextSibling(RenderLayer *next) noexcept
  {
    _next = next ? CreateWeakPtr(next) : nullptr;
  }

  void RenderLayer::SetPreviousSibling(RenderLayer *prev) noexcept
  {
    _previous = prev ? CreateWeakPtr(prev) : nullptr;
  }

  void RenderLayer::SetFirstChild(RenderLayer *first) noexcept
  {
    _first = first ? CreateWeakPtr(first) : nullptr;
  }

  void RenderLayer::SetLastChild(RenderLayer *last) noexcept
  {
    _last = last ? CreateWeakPtr(last) : nullptr;
  }

#pragma region Tree

  void RenderLayer::AddChild(RenderLayer &child, RenderLayer *beforeChild) noexcept
  {
    RenderLayer *prevSibling = beforeChild ? beforeChild->PreviousSibling() : LastChild();
    if (prevSibling)
    {
      child.SetPreviousSibling(prevSibling);
      prevSibling->SetNextSibling(&child);
      krys_debug_assert(prevSibling != &child);
    }
    else
    {
      SetFirstChild(&child);
    }

    if (beforeChild)
    {
      beforeChild->SetPreviousSibling(&child);
      child.SetNextSibling(beforeChild);
      krys_debug_assert(beforeChild != &child);
    }
    else
    {
      SetLastChild(&child);
    }

    child._parent = CreateWeakPtr(this);
    child.SetSelfAndDescendantsNeedPositionUpdate();

    DirtyPaintOrderListsOnChildChange(child);

    child.UpdateAncestorDependentState();
    DirtyAncestorChainVisibleDescendantStatus();
    child.UpdateDescendantDependentFlags();

    if (child.IsSelfPaintingLayer() || child.HasSelfPaintingLayerDescendant())
    {
      SetAncestorChainHasSelfPaintingLayerDescendant();
    }

    if (child.IsViewportConstrained() || child._hasViewportConstrainedDescendant)
    {
      SetAncestorChainHasViewportConstrainedDescendant();
    }

    // FIXME: The compositing dirty bits, once there's compositing.

    if (child.HasBlendMode() || (child.HasNotIsolatedBlendingDescendants() && !child.IsolatesBlending()))
    {
      UpdateAncestorChainHasBlendingDescendants(); // Why not just dirty?
    }
  }

  void RenderLayer::RemoveChild(RenderLayer &oldChild) noexcept
  {
    // FIXME: Tell the compositor, once there is one.

    // remove the child
    if (oldChild.PreviousSibling())
    {
      oldChild.PreviousSibling()->SetNextSibling(oldChild.NextSibling());
    }
    if (oldChild.NextSibling())
    {
      oldChild.NextSibling()->SetPreviousSibling(oldChild.PreviousSibling());
    }

    if (_first.get() == &oldChild)
    {
      SetFirstChild(oldChild.NextSibling());
    }
    if (_last.get() == &oldChild)
    {
      SetLastChild(oldChild.PreviousSibling());
    }

    DirtyPaintOrderListsOnChildChange(oldChild);

    oldChild.SetPreviousSibling(nullptr);
    oldChild.SetNextSibling(nullptr);
    oldChild._parent = nullptr;

    oldChild.UpdateDescendantDependentFlags();
    if (oldChild._hasVisibleContent || oldChild._hasVisibleDescendant)
    {
      DirtyAncestorChainVisibleDescendantStatus();
    }

    if (oldChild.IsSelfPaintingLayer() || oldChild.HasSelfPaintingLayerDescendant())
    {
      DirtyAncestorChainHasSelfPaintingLayerDescendantStatus();
    }

    if (oldChild.IsViewportConstrained() || oldChild._hasViewportConstrainedDescendant)
    {
      DirtyAncestorChainHasViewportConstrainedDescendantStatus();
    }

    if (oldChild.HasBlendMode()
        || (oldChild.HasNotIsolatedBlendingDescendants() && !oldChild.IsolatesBlending()))
    {
      DirtyAncestorChainHasBlendingDescendants();
    }
    if (Renderer().Style().UsedVisibility() != Visibility::Visible)
    {
      DirtyVisibleContentStatus();
    }
  }

  void RenderLayer::DirtyPaintOrderListsOnChildChange(RenderLayer &child) noexcept
  {
    if (child.IsNormalFlowOnly())
    {
      DirtyNormalFlowList();
    }

    if (!child.IsNormalFlowOnly() || child.FirstChild())
    {
      // Dirty the z-order list in which we are contained. The stackingContext() can be null in the
      // case where we're building up generated content layers. This is ok, since the lists will start
      // off dirty in that case anyway.
      child.DirtyStackingContextZOrderLists();
    }
  }

  void RenderLayer::InsertOnlyThisLayer() noexcept
  {
    if (!_parent && Renderer().Parent())
    {
      // We need to connect ourselves when our renderer() has a parent.
      // Find our enclosingLayer and add ourselves.
      auto *parentLayer = Renderer().LayerParent();
      if (!parentLayer)
      {
        return;
      }

      auto *beforeChild = Renderer().LayerNextSibling(*parentLayer);
      parentLayer->AddChild(*this, beforeChild);
    }

    // Remove all descendant layers from the hierarchy and add them to the new position.
    for (auto &child : ChildrenOfType<RenderElement>(Renderer()))
    {
      child.MoveLayers(*this);
    }

    // Clear out all the clip rects.
    ClearClipRectsIncludingDescendants();
  }

  void RenderLayer::RemoveOnlyThisLayer() noexcept
  {
    if (!_parent)
    {
      return;
    }

    // FIXME: Tell the compositor, once there is one.

    // Dirty the clip rects.
    ClearClipRectsIncludingDescendants();

    RenderLayer *nextSib = NextSibling();

    // Now walk our kids and reattach them to our parent.
    RenderLayer *current = _first.get();
    while (current)
    {
      RenderLayer *next = current->NextSibling();
      RemoveChild(*current);
      _parent->AddChild(*current, nextSib);
      current->SetRepaintStatus(RepaintStatus::NeedsFullRepaint);
      current = next;
    }

    // Remove us from the parent.
    _parent->RemoveChild(*this);
    Renderer().DestroyLayer();
  }

  bool RenderLayer::IsDescendantOf(const RenderLayer &layer) const noexcept
  {
    for (const auto *ancestor = this; ancestor; ancestor = ancestor->Parent())
    {
      if (&layer == ancestor)
      {
        return true;
      }
    }
    return false;
  }

  RenderLayer *RenderLayer::CommonAncestorWithLayer(const RenderLayer &layer) const noexcept
  {
    if (this == &layer)
    {
      return const_cast<RenderLayer *>(this);
    }

    HashSet<const RenderLayer *> ancestorChain;
    for (const auto *currLayer = this; currLayer; currLayer = currLayer->Parent())
    {
      ancestorChain.insert(currLayer);
    }

    for (const auto *currLayer = &layer; currLayer; currLayer = currLayer->Parent())
    {
      if (ancestorChain.contains(currLayer))
      {
        return const_cast<RenderLayer *>(currLayer);
      }
    }
    return nullptr;
  }

#pragma endregion

#pragma region Stacking contexts and z-order lists

  static bool CanCreateStackingContext(const RenderLayer &layer) noexcept
  {
    const auto &renderer = layer.Renderer();
    // FIXME: View transition captures and the top layer, once there are any.
    return renderer.HasTransformRelatedProperty() || renderer.HasClipPath() || renderer.HasFilter()
           || renderer.HasMask() || renderer.HasBackdropFilter() || renderer.HasBlendMode()
           || renderer.IsTransparent() || renderer.RequiresRenderingConsolidationForViewTransition()
           || renderer.IsPositioned() // Note that this only creates stacking context in conjunction with
                                      // explicit z-index.
           || renderer.Style().HasIsolation() || renderer.ShouldApplyPaintContainment()
           || !renderer.Style().UsedZIndex().IsAuto()
           || renderer.Style().GetWillChange().CanCreateStackingContext();
  }

  bool RenderLayer::ShouldBeNormalFlowOnly() const noexcept
  {
    if (CanCreateStackingContext(*this))
    {
      return false;
    }

    // FIXME: Canvases, video, embedded objects and iframes too, once there are any.
    return Renderer().HasNonVisibleOverflow()
           || (Renderer().Style().SpecifiesColumns() && !IsRenderViewLayer())
           || Renderer().IsRenderFragmentedFlow();
  }

  bool RenderLayer::ShouldBeCSSStackingContext() const noexcept
  {
    // FIXME: View transition captures and containing blocks, once there are view transitions.
    return !Renderer().Style().UsedZIndex().IsAuto() || Renderer().ShouldApplyLayoutContainment()
           || Renderer().ShouldApplyPaintContainment()
           || Renderer().RequiresRenderingConsolidationForViewTransition()
           || Renderer().IsViewTransitionRoot() || Renderer().IsViewTransitionContainingBlock()
           || IsRenderViewLayer();
  }

  bool RenderLayer::SetIsNormalFlowOnly(bool isNormalFlowOnly) noexcept
  {
    if (isNormalFlowOnly == _isNormalFlowOnly)
    {
      return false;
    }

    _isNormalFlowOnly = isNormalFlowOnly;

    if (auto *p = Parent())
    {
      p->DirtyNormalFlowList();
    }
    DirtyStackingContextZOrderLists();
    return true;
  }

  void RenderLayer::IsStackingContextChanged() noexcept
  {
    DirtyStackingContextZOrderLists();
    SetSelfAndDescendantsNeedPositionUpdate();
    if (IsStackingContext())
    {
      DirtyZOrderLists();
    }
    else
    {
      ClearZOrderLists();
    }
  }

  bool RenderLayer::SetIsOpportunisticStackingContext(bool isStacking) noexcept
  {
    bool wasStacking = IsStackingContext();
    _isOpportunisticStackingContext = isStacking;
    if (wasStacking == IsStackingContext())
    {
      return false;
    }

    IsStackingContextChanged();
    return true;
  }

  bool RenderLayer::SetIsCSSStackingContext(bool isCSSStackingContext) noexcept
  {
    bool wasStacking = IsStackingContext();
    _isCSSStackingContext = isCSSStackingContext;
    if (wasStacking == IsStackingContext())
    {
      return false;
    }

    IsStackingContextChanged();
    return true;
  }

  RenderLayer *RenderLayer::StackingContext() const noexcept
  {
    auto *layer = Parent();
    while (layer && !layer->IsStackingContext())
    {
      layer = layer->Parent();
    }

    krys_debug_assert(!layer || layer->IsStackingContext());
    return layer;
  }

  void RenderLayer::DirtyZOrderLists() noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif
    krys_debug_assert(IsStackingContext());

    if (_posZOrderList)
    {
      _posZOrderList->clear();
    }
    if (_negZOrderList)
    {
      _negZOrderList->clear();
    }
    _zOrderListsDirty = true;

    // FIXME: The compositing paint order children update, once there's compositing.
  }

  void RenderLayer::DirtyStackingContextZOrderLists() noexcept
  {
    if (auto *sc = StackingContext())
    {
      sc->DirtyZOrderLists();
    }
  }

  void RenderLayer::DirtyHiddenStackingContextAncestorZOrderLists() noexcept
  {
    for (auto *sc = StackingContext(); sc; sc = sc->StackingContext())
    {
      sc->DirtyZOrderLists();
      if (sc->HasVisibleContent())
      {
        break;
      }
    }
  }

  void RenderLayer::DirtyNormalFlowList() noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif

    if (_normalFlowList)
    {
      _normalFlowList->clear();
    }
    _normalFlowListDirty = true;

    // FIXME: The compositing paint order children update, once there's compositing.
  }

  void RenderLayer::UpdateNormalFlowList() noexcept
  {
    if (!_normalFlowListDirty)
    {
      return;
    }

#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif

    for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
    {
      // Ignore non-overflow layers.
      if (child->IsNormalFlowOnly())
      {
        if (!_normalFlowList)
        {
          _normalFlowList = CreateUnique<List<RenderLayer *>>();
        }
        _normalFlowList->push_back(child);
      }
    }

    if (_normalFlowList)
    {
      _normalFlowList->shrink_to_fit();
    }

    _normalFlowListDirty = false;
  }

  void RenderLayer::ClearZOrderLists() noexcept
  {
    krys_debug_assert(!IsStackingContext());
#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif

    _posZOrderList = nullptr;
    _negZOrderList = nullptr;
  }

  void RenderLayer::UpdateZOrderLists() noexcept
  {
    if (!_zOrderListsDirty)
    {
      return;
    }

    if (!IsStackingContext())
    {
      ClearZOrderLists();
      _zOrderListsDirty = false;
      return;
    }

    RebuildZOrderLists();
  }

  void RenderLayer::RebuildZOrderLists() noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif
    krys_debug_assert(IsDirtyStackingContext());

    RebuildZOrderLists(_posZOrderList, _negZOrderList);
    _zOrderListsDirty = false;

    bool hasNegativeZOrderList = _negZOrderList && _negZOrderList->size();
    // Having negative z-order lists affect whether a compositing layer needs a foreground layer.
    if (hasNegativeZOrderList != _hadNegativeZOrderList)
    {
      _hadNegativeZOrderList = hasNegativeZOrderList;
      // FIXME: The compositing configuration update, once there's compositing.
    }
  }

  void RenderLayer::RebuildZOrderLists(UniquePtr<List<RenderLayer *>> &posZOrderList,
                                       UniquePtr<List<RenderLayer *>> &negZOrderList) noexcept
  {
    for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
    {
      child->CollectLayers(posZOrderList, negZOrderList);
    }

    auto compareZIndex = [](const RenderLayer *first, const RenderLayer *second) -> bool
    {
      return first->ZIndex() < second->ZIndex();
    };

    // Sort the two lists.
    if (posZOrderList)
    {
      std::stable_sort(posZOrderList->begin(), posZOrderList->end(), compareZIndex);
      posZOrderList->shrink_to_fit();
    }

    if (negZOrderList)
    {
      std::stable_sort(negZOrderList->begin(), negZOrderList->end(), compareZIndex);
      negZOrderList->shrink_to_fit();
    }

    // FIXME: The top layer's layers and the view transition containing block go last in the view's list, once
    // there's a top layer and view transitions.
  }

  void RenderLayer::CollectLayers(UniquePtr<List<RenderLayer *>> &positiveZOrderList,
                                  UniquePtr<List<RenderLayer *>> &negativeZOrderList) noexcept
  {
    krys_debug_assert(!DescendantDependentFlagsAreDirty());
    // FIXME: Layers that establish the top layer and the view transition containing block are collected by
    // the view, once there are any.

    bool isStacking = IsStackingContext();
    bool layerOrDescendantsAreVisible = _hasVisibleContent || _hasVisibleDescendant;
    // Normal flow layers are just painted by their enclosing layers, so they don't get put in zorder lists.
    if (!IsNormalFlowOnly())
    {
      if (layerOrDescendantsAreVisible)
      {
        auto &layerList = (ZIndex() >= 0) ? positiveZOrderList : negativeZOrderList;
        if (!layerList)
        {
          layerList = CreateUnique<List<RenderLayer *>>();
        }
        layerList->push_back(this);
      }
      // FIXME: Omitted layers leave the compositor (setWasOmittedFromZOrderTree), once there's compositing.
    }

    // Recur into our children to collect more layers, but only if we don't establish
    // a stacking context/container.
    if (!isStacking)
    {
      for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
      {
        child->CollectLayers(positiveZOrderList, negativeZOrderList);
      }
    }
  }

  void RenderLayer::UpdateLayerListsIfNeeded() noexcept
  {
    UpdateDescendantDependentFlags();
    UpdateZOrderLists();
    UpdateNormalFlowList();
  }

  void RenderLayer::ForceStackingContextIfNeeded() noexcept
  {
    if (SetIsCSSStackingContext(true))
    {
      SetIsNormalFlowOnly(false);
      if (Parent())
      {
        if (!HasNotIsolatedBlendingDescendantsStatusDirty() && HasNotIsolatedBlendingDescendants())
        {
          Parent()->DirtyAncestorChainHasBlendingDescendants();
        }
      }
    }
  }

  int32 RenderLayer::ZIndex() const noexcept
  {
    auto value = Renderer().Style().UsedZIndex().TryValue();
    return value ? static_cast<int32>(value->Value) : 0;
  }

#pragma endregion

#pragma region Position updates

  void RenderLayer::SetNeedsPositionUpdate() noexcept
  {
    _layerPositionDirtyBits.Add(LayerPositionUpdates::NeedsPositionUpdate);
    for (auto *layer = Parent(); layer; layer = layer->Parent())
    {
      if (layer->_layerPositionDirtyBits.Contains(LayerPositionUpdates::DescendantNeedsPositionUpdate))
      {
        break;
      }
      layer->_layerPositionDirtyBits.Add(LayerPositionUpdates::DescendantNeedsPositionUpdate);
    }
  }

  bool RenderLayer::NeedsPositionUpdate() const noexcept
  {
    if (_layerPositionDirtyBits.ContainsAny(
          {LayerPositionUpdates::NeedsPositionUpdate, LayerPositionUpdates::DescendantNeedsPositionUpdate}))
    {
      return true;
    }
    if (Parent()
        && Parent()->_layerPositionDirtyBits.Contains(LayerPositionUpdates::AllChildrenNeedPositionUpdate))
    {
      return true;
    }
    return false;
  }

  void RenderLayer::SetSelfAndChildrenNeedPositionUpdate() noexcept
  {
    SetNeedsPositionUpdate();
    _layerPositionDirtyBits.Add({LayerPositionUpdates::DescendantNeedsPositionUpdate,
                                 LayerPositionUpdates::AllChildrenNeedPositionUpdate});
  }

  void RenderLayer::SetSelfAndDescendantsNeedPositionUpdate() noexcept
  {
    SetNeedsPositionUpdate();
    _layerPositionDirtyBits.Add({LayerPositionUpdates::DescendantNeedsPositionUpdate,
                                 LayerPositionUpdates::AllDescendantsNeedPositionUpdate});
  }

  EnumSet<RenderLayer::UpdateLayerPositionsFlag>
    RenderLayer::FlagsForUpdateLayerPositions(RenderLayer &startingLayer) noexcept
  {
    EnumSet<UpdateLayerPositionsFlag> flags = {UpdateLayerPositionsFlag::CheckForRepaint};

    if (auto *parent = startingLayer.Parent())
    {
      if (parent->HasFixedContainingBlockAncestor()
          || (!parent->IsRenderViewLayer() && parent->Renderer().CanContainFixedPositionObjects()))
      {
        flags.Add(UpdateLayerPositionsFlag::SeenFixedContainingBlockLayer);
      }

      if (parent->HasTransformedAncestor() || parent->Transform())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenTransformedLayer);
      }

      if (parent->_has3DTransformedAncestor || (parent->Transform() && !parent->Transform()->IsAffine()))
      {
        flags.Add(UpdateLayerPositionsFlag::Seen3DTransformedLayer);
      }

      if (parent->BehavesAsFixed()
          || (parent->Renderer().IsFixedPositioned() && !parent->HasFixedContainingBlockAncestor()))
      {
        flags.Add(UpdateLayerPositionsFlag::SeenFixedLayer);
      }

      if (parent->Renderer().IsStickilyPositioned())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenStickyLayer);
      }

      // FIXME: Composited scrolling ancestors, once there's composited scrolling.
    }

    return flags;
  }

  void RenderLayer::UpdateLayerPositionsAfterStyleChange(bool environmentChanged) noexcept
  {
    auto updateLayerPositionFlags = [&](bool environmentChanged)
    {
      auto flags = FlagsForUpdateLayerPositions(*this);
      if (environmentChanged)
      {
        flags.Add(UpdateLayerPositionsFlag::EnvironmentChanged);
      }
      return flags;
    };

    if (environmentChanged)
    {
      SetSelfAndDescendantsNeedPositionUpdate();
    }

    // FIXME: Invalidate the document markers' rects (willUpdateLayerPositions), once there are document
    // markers.
    RecursiveUpdateLayerPositions(updateLayerPositionFlags(environmentChanged));
  }

  void RenderLayer::UpdateLayerPositionsAfterLayout(bool didFullRepaint, bool environmentChanged) noexcept
  {
    krys_debug_assert(IsRenderViewLayer());

    auto updateLayerPositionFlags = [&](bool didFullRepaint, bool environmentChanged)
    {
      auto flags = FlagsForUpdateLayerPositions(*this);
      if (didFullRepaint)
      {
        flags.Remove(UpdateLayerPositionsFlag::CheckForRepaint);
        flags.Add(UpdateLayerPositionsFlag::NeedsFullRepaintInBacking);
      }
      if (environmentChanged)
      {
        flags.Add(UpdateLayerPositionsFlag::EnvironmentChanged);
      }
      return flags;
    };

    if (environmentChanged)
    {
      SetSelfAndDescendantsNeedPositionUpdate();
    }

    // FIXME: Invalidate the document markers' rects (willUpdateLayerPositions), once there are document
    // markers.
    RecursiveUpdateLayerPositions(updateLayerPositionFlags(didFullRepaint, environmentChanged));
  }

  bool RenderLayer::AncestorLayerPositionStateChanged(EnumSet<UpdateLayerPositionsFlag> flags) const noexcept
  {
    return _hasFixedContainingBlockAncestor
             != flags.Contains(UpdateLayerPositionsFlag::SeenFixedContainingBlockLayer)
           || _hasTransformedAncestor != flags.Contains(UpdateLayerPositionsFlag::SeenTransformedLayer)
           || _has3DTransformedAncestor != flags.Contains(UpdateLayerPositionsFlag::Seen3DTransformedLayer)
           || _hasFixedAncestor != flags.Contains(UpdateLayerPositionsFlag::SeenFixedLayer)
           || _hasPaginatedAncestor != flags.Contains(UpdateLayerPositionsFlag::UpdatePagination);
  }

  void RenderLayer::RecursiveUpdateLayerPositions(EnumSet<UpdateLayerPositionsFlag> flags) noexcept
  {
    if (AncestorLayerPositionStateChanged(flags))
    {
      flags.Add(UpdateLayerPositionsFlag::SubtreeNeedsUpdate);
    }

    if (!NeedsPositionUpdate() && !flags.ContainsAny(InvalidationLayerPositionsFlags()))
    {
      return;
    }

    if (_layerPositionDirtyBits.Contains(LayerPositionUpdates::AllDescendantsNeedPositionUpdate))
    {
      flags.Add(UpdateLayerPositionsFlag::SubtreeNeedsUpdate);
    }

    if (UpdateLayerPosition(&flags))
    {
      flags.Add(UpdateLayerPositionsFlag::SubtreeNeedsUpdate);
    }

    // FIXME: Apply the post-layout scroll position, and position the overflow controls, once there's a
    // scrollable area.

    // Clear our cached clip rect information.
    ClearClipRects();

    UpdateDescendantDependentFlags();

    if (flags.Contains(UpdateLayerPositionsFlag::UpdatePagination))
    {
      UpdatePagination();
    }
    else if (Renderer().IsRenderFragmentedFlow())
    {
      _enclosingPaginationLayer = CreateWeakPtr(this);
      flags.Add(UpdateLayerPositionsFlag::UpdatePagination);
    }
    else
    {
      _enclosingPaginationLayer = nullptr;
    }

    // FIXME: Compute the repaint rects and repaint after layout, once there's painting.

    _repaintStatus = RepaintStatus::NeedsNormalRepaint;
    _hasFixedContainingBlockAncestor =
      flags.Contains(UpdateLayerPositionsFlag::SeenFixedContainingBlockLayer);
    _hasTransformedAncestor = flags.Contains(UpdateLayerPositionsFlag::SeenTransformedLayer);
    _has3DTransformedAncestor = flags.Contains(UpdateLayerPositionsFlag::Seen3DTransformedLayer);
    _hasFixedAncestor = flags.Contains(UpdateLayerPositionsFlag::SeenFixedLayer);
    _hasStickyAncestor = flags.Contains(UpdateLayerPositionsFlag::SeenStickyLayer);
    _hasPaginatedAncestor = flags.Contains(UpdateLayerPositionsFlag::UpdatePagination);

    if (!IsRenderViewLayer())
    {
      if (Renderer().CanContainFixedPositionObjects())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenFixedContainingBlockLayer);
      }

      if (Transform())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenTransformedLayer);
        if (!Transform()->IsAffine())
        {
          flags.Add(UpdateLayerPositionsFlag::Seen3DTransformedLayer);
        }
      }
    }

    // Fixed inside transform behaves like absolute (per spec).
    if (_hasFixedAncestor || (Renderer().IsFixedPositioned() && !_hasFixedContainingBlockAncestor))
    {
      SetBehavesAsFixed(true);
      flags.Add(UpdateLayerPositionsFlag::SeenFixedLayer);
    }

    if (_hasStickyAncestor || Renderer().IsStickilyPositioned())
    {
      flags.Add(UpdateLayerPositionsFlag::SeenStickyLayer);
    }

    if (flags.ContainsAny(InvalidationLayerPositionsFlags())
        || _layerPositionDirtyBits.Contains(LayerPositionUpdates::DescendantNeedsPositionUpdate))
    {
      for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
      {
        child->RecursiveUpdateLayerPositions(flags);
      }
    }

    // FIXME: The marquee position, the fixed layer's intersection with the viewport and the backing, once
    // there are marquees and compositing.

    ClearLayerPositionDirtyBits();
  }

  static LayoutRect ComputeLayerPositionAndIntegralSize(const RenderLayerModelObject &renderer) noexcept
  {
    if (auto *inlineRenderer = DynamicDowncast<RenderInline>(renderer);
        inlineRenderer && inlineRenderer->IsInline())
    {
      return {LayoutPoint(), LayoutSize(inlineRenderer->LinesBoundingBox().Size())};
    }

    if (auto *boxRenderer = DynamicDowncast<RenderBox>(renderer))
    {
      const auto &frameRect = boxRenderer->FrameRect();
      return {boxRenderer->TopLeftLocation(),
              LayoutSize(SnappedIntSize(frameRect.Size(), frameRect.Location()))};
    }

    krys_debug_assert(false);
    return {};
  }

  bool RenderLayer::UpdateLayerPosition(EnumSet<UpdateLayerPositionsFlag> *flags) noexcept
  {
    auto layerRect = ComputeLayerPositionAndIntegralSize(Renderer());
    auto localPoint = layerRect.Location();

    bool geometryChanged = false;
    if (IntSize newSize(layerRect.Width().ToInt(), layerRect.Height().ToInt()); newSize != Size())
    {
      geometryChanged = true;
      SetSize(newSize);

      if (flags && Renderer().HasNonVisibleOverflow())
      {
        flags->Add(UpdateLayerPositionsFlag::ContainingClippingLayerChangedSize);
      }
    }

    if (!Renderer().IsOutOfFlowPositioned())
    {
      auto *ancestor = Renderer().Parent();
      // We must adjust our position by walking up the render tree looking for the
      // nearest enclosing object with a layer.
      while (ancestor && !ancestor->HasLayer())
      {
        if (auto *boxRenderer = DynamicDowncast<RenderBox>(ancestor))
        {
          // Rows and cells share the same coordinate space (that of the section).
          // Omit them when computing our xpos/ypos.
          if (!Is<RenderTableRow>(*boxRenderer))
          {
            localPoint += boxRenderer->TopLeftLocationOffset();
          }
        }
        ancestor = ancestor->Parent();
      }
      if (auto *tableRow = DynamicDowncast<RenderTableRow>(ancestor))
      {
        // Put ourselves into the row coordinate space.
        localPoint -= tableRow->TopLeftLocationOffset();
      }
    }

    // FIXME: Subtract the enclosing positioned (or parent) layer's scroll position, and move out-of-flow
    // boxes inside an in-flow positioned inline with it (offsetForInFlowPositionedInline), once there's a
    // scrollable area and inline layout. Assign the scrolling scopes, once there's composited scrolling.

    if (Renderer().IsInFlowPositioned())
    {
      if (auto *boxModelObject = DynamicDowncast<RenderBoxModelObject>(Renderer()))
      {
        auto newOffset = boxModelObject->OffsetForInFlowPosition();
        geometryChanged |= newOffset != _offsetForPosition;
        _offsetForPosition = newOffset;
        localPoint.Move(_offsetForPosition);
      }
    }

    geometryChanged |= Location() != localPoint;
    SetLocation(localPoint);

    // FIXME: The compositing geometry updates, once there's compositing.

    return geometryChanged;
  }

  void RenderLayer::SetRepaintStatus(RepaintStatus status) noexcept
  {
    if (status != _repaintStatus)
    {
      _repaintStatus = status;
      SetNeedsPositionUpdate();
    }
  }

  void RenderLayer::UpdatePagination() noexcept
  {
    _enclosingPaginationLayer = nullptr;

    if (!Parent())
    {
      return;
    }

    // Each layer that is inside a multicolumn flow thread has to be checked individually and
    // genuinely know if it is going to have to split itself up when painting only its contents (and not any
    // other descendant layers). We track an enclosingPaginationLayer instead of using a simple bit, since we
    // want to be able to get back to that layer easily.
    if (Renderer().IsRenderFragmentedFlow())
    {
      _enclosingPaginationLayer = CreateWeakPtr(this);
      return;
    }

    auto paginationLayerOf = [](const RenderLayer &layer) -> WeakPtr<RenderLayer>
    {
      // Content inside a transform is not considered to be paginated, since we simply
      // paint the transform multiple times in each column, so we don't have to use
      // fragments for the transformed content.
      if (layer.IsTransformed())
      {
        return nullptr;
      }
      auto *paginationLayer = layer.EnclosingPaginationLayer(IncludeCompositedPaginatedLayers);
      return paginationLayer ? CreateWeakPtr(paginationLayer) : nullptr;
    };

    if (IsNormalFlowOnly())
    {
      _enclosingPaginationLayer = paginationLayerOf(*Parent());
      return;
    }

    // For the new columns code, we want to walk up our containing block chain looking for an enclosing layer.
    // Once we find one, then we just check its pagination status.
    for (const auto *containingBlock = Renderer().ContainingBlock();
         containingBlock && !Is<RenderView>(*containingBlock);
         containingBlock = containingBlock->ContainingBlock())
    {
      if (containingBlock->HasLayer())
      {
        _enclosingPaginationLayer = paginationLayerOf(*containingBlock->Layer());
        return;
      }
    }
  }

#pragma endregion

#pragma region Descendant dependent flags

  void RenderLayer::SetAncestorChainHasSelfPaintingLayerDescendant() noexcept
  {
    for (RenderLayer *layer = this; layer; layer = layer->Parent())
    {
      if (Renderer().ShouldApplyPaintContainment())
      {
        _hasSelfPaintingLayerDescendant = true;
        _hasSelfPaintingLayerDescendantDirty = false;
        break;
      }
      if (!layer->_hasSelfPaintingLayerDescendantDirty && layer->HasSelfPaintingLayerDescendant())
      {
        break;
      }

      layer->_hasSelfPaintingLayerDescendantDirty = false;
      layer->_hasSelfPaintingLayerDescendant = true;
    }
  }

  void RenderLayer::DirtyAncestorChainHasSelfPaintingLayerDescendantStatus() noexcept
  {
    for (RenderLayer *layer = this; layer; layer = layer->Parent())
    {
      if (layer->_hasSelfPaintingLayerDescendantDirty)
      {
        break;
      }

      layer->_hasSelfPaintingLayerDescendantDirty = true;
      layer->SetNeedsPositionUpdate();
    }
  }

  void RenderLayer::SetAncestorChainHasViewportConstrainedDescendant() noexcept
  {
    for (RenderLayer *layer = this; layer; layer = layer->Parent())
    {
      if (!layer->_hasViewportConstrainedDescendantStatusDirty && layer->_hasViewportConstrainedDescendant)
      {
        break;
      }

      layer->_hasViewportConstrainedDescendant = true;
      layer->_hasViewportConstrainedDescendantStatusDirty = false;
    }
  }

  void RenderLayer::DirtyAncestorChainHasViewportConstrainedDescendantStatus() noexcept
  {
    for (RenderLayer *layer = this; layer; layer = layer->Parent())
    {
      if (layer->_hasViewportConstrainedDescendantStatusDirty)
      {
        break;
      }

      layer->_hasViewportConstrainedDescendantStatusDirty = true;
    }
  }

  void RenderLayer::UpdateBlendMode() noexcept
  {
    bool hadBlendMode = _blendMode != BlendMode::Normal;
    if (Parent() && hadBlendMode != HasBlendMode())
    {
      if (HasBlendMode())
      {
        Parent()->UpdateAncestorChainHasBlendingDescendants();
      }
      else
      {
        Parent()->DirtyAncestorChainHasBlendingDescendants();
      }
    }

    BlendMode newBlendMode = Renderer().Style().GetBlendMode();
    if (newBlendMode != _blendMode)
    {
      _blendMode = newBlendMode;
    }
  }

  void RenderLayer::WillRemoveChildWithBlendMode() noexcept
  {
    Parent()->DirtyAncestorChainHasBlendingDescendants();
  }

  void RenderLayer::UpdateAncestorChainHasBlendingDescendants() noexcept
  {
    for (auto *layer = this; layer; layer = layer->Parent())
    {
      if (!layer->HasNotIsolatedBlendingDescendantsStatusDirty()
          && layer->HasNotIsolatedBlendingDescendants())
      {
        break;
      }
      layer->_hasNotIsolatedBlendingDescendants = true;
      layer->_hasNotIsolatedBlendingDescendantsStatusDirty = false;

      layer->UpdateSelfPaintingLayer();

      if (layer->IsCSSStackingContext())
      {
        break;
      }
    }
  }

  void RenderLayer::DirtyAncestorChainHasBlendingDescendants() noexcept
  {
    for (auto *layer = this; layer; layer = layer->Parent())
    {
      if (layer->HasNotIsolatedBlendingDescendantsStatusDirty())
      {
        break;
      }

      layer->_hasNotIsolatedBlendingDescendantsStatusDirty = true;
      layer->SetNeedsPositionUpdate();
    }
  }

  void RenderLayer::SetHasVisibleContent() noexcept
  {
    if (_hasVisibleContent && !_visibleContentStatusDirty)
    {
      krys_debug_assert(!Parent() || Parent()->_visibleDescendantStatusDirty
                        || Parent()->HasVisibleDescendant());
      return;
    }

    _visibleContentStatusDirty = false;
    _hasVisibleContent = true;
    // FIXME: Compute the repaint rects, once there's painting.
    SetNeedsPositionUpdate();
    if (!IsNormalFlowOnly())
    {
      // We don't collect invisible layers in z-order lists if they are not composited.
      // As we became visible, we need to dirty our stacking containers ancestors to be properly
      // collected.
      DirtyHiddenStackingContextAncestorZOrderLists();
    }

    if (Parent())
    {
      Parent()->DirtyAncestorChainVisibleDescendantStatus();
    }
  }

  void RenderLayer::DirtyVisibleContentStatus() noexcept
  {
    _visibleContentStatusDirty = true;
    SetNeedsPositionUpdate();
    if (Parent())
    {
      Parent()->DirtyAncestorChainVisibleDescendantStatus();
    }
  }

  void RenderLayer::DirtyAncestorChainVisibleDescendantStatus() noexcept
  {
    SetNeedsPositionUpdate();
    for (auto *layer = this; layer; layer = layer->Parent())
    {
      if (layer->_visibleDescendantStatusDirty)
      {
        break;
      }

      layer->_visibleDescendantStatusDirty = true;
    }
  }

  void RenderLayer::UpdateAncestorDependentState() noexcept
  {
    // FIXME: The enclosing SVG hidden or resource container, and whether the layer is inside an SVG foreign
    // object (which makes it self-painting), once there's SVG.
  }

  void RenderLayer::UpdateDescendantDependentFlags() noexcept
  {
    if (_visibleDescendantStatusDirty || _hasSelfPaintingLayerDescendantDirty
        || HasNotIsolatedBlendingDescendantsStatusDirty() || _hasViewportConstrainedDescendantStatusDirty)
    {
      bool hasVisibleDescendant = false;
      bool hasSelfPaintingLayerDescendant = false;
      bool hasNotIsolatedBlendingDescendants = false;
      bool hasViewportConstrainedDescendant = false;

      if (_hasNotIsolatedBlendingDescendantsStatusDirty)
      {
        _hasNotIsolatedBlendingDescendantsStatusDirty = false;
        UpdateSelfPaintingLayer();
      }

      for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
      {
        child->UpdateDescendantDependentFlags();

        hasVisibleDescendant |= child->_hasVisibleContent || child->_hasVisibleDescendant;
        hasSelfPaintingLayerDescendant |=
          child->IsSelfPaintingLayer() || child->HasSelfPaintingLayerDescendant();
        hasNotIsolatedBlendingDescendants |=
          child->HasBlendMode() || (child->HasNotIsolatedBlendingDescendants() && !child->IsolatesBlending());
        hasViewportConstrainedDescendant |=
          child->_hasViewportConstrainedDescendant || child->IsViewportConstrained();
      }

      _hasVisibleDescendant = hasVisibleDescendant;
      _visibleDescendantStatusDirty = false;
      _hasSelfPaintingLayerDescendant = hasSelfPaintingLayerDescendant;
      _hasSelfPaintingLayerDescendantDirty = false;
      _hasViewportConstrainedDescendant = hasViewportConstrainedDescendant;
      _hasViewportConstrainedDescendantStatusDirty = false;

      _hasNotIsolatedBlendingDescendants = hasNotIsolatedBlendingDescendants;
    }

    if (_visibleContentStatusDirty)
    {
      // We need the parent to know if we have skipped content or content-visibility root.
      if (Renderer().Style().IsSkippedRootOrSkippedContent() && !Renderer().Parent())
      {
        return;
      }
      bool hasVisibleContent = ComputeHasVisibleContent();
      if (hasVisibleContent != _hasVisibleContent)
      {
        _hasVisibleContent = hasVisibleContent;
        if (!IsNormalFlowOnly())
        {
          // We don't collect invisible layers in z-order lists if they are not composited.
          // As we change visibility, we need to dirty our stacking containers ancestors to be properly
          // collected.
          DirtyHiddenStackingContextAncestorZOrderLists();
        }
      }
      _visibleContentStatusDirty = false;
    }

    krys_debug_assert(!DescendantDependentFlagsAreDirty());
  }

  bool RenderLayer::ComputeHasVisibleContent() const noexcept
  {
    if (_isHiddenByOverflowTruncation)
    {
      return false;
    }

    if (Renderer().IsSkippedContent())
    {
      return false;
    }

    if (Renderer().Style().UsedVisibility() == Visibility::Visible)
    {
      return true;
    }

    // Layer's renderer has visibility:hidden, but some non-layer child may have visibility:visible.
    auto nextRenderer = [&](const RenderObject &renderer) -> const RenderObject *
    {
      for (const RenderObject *ancestor = &renderer; ancestor && ancestor != &this->Renderer();
           ancestor = ancestor->Parent())
      {
        if (auto *sibling = ancestor->NextSibling())
        {
          return sibling;
        }
      }
      return {};
    };
    const RenderObject *renderer = this->Renderer().FirstChild();
    while (renderer)
    {
      if (auto *renderElement = DynamicDowncast<RenderElement>(renderer);
          renderElement && !renderElement->HasLayer())
      {
        if (renderElement->Style().UsedVisibility() == Visibility::Visible)
        {
          return true;
        }
        if (auto *firstChild = renderElement->FirstChild())
        {
          renderer = firstChild;
          continue;
        }
      }
      renderer = nextRenderer(*renderer);
    }
    return false;
  }

  void RenderLayer::SetIsHiddenByOverflowTruncation(bool isHidden) noexcept
  {
    if (_isHiddenByOverflowTruncation == isHidden)
    {
      return;
    }
    _isHiddenByOverflowTruncation = isHidden;
    DirtyVisibleContentStatus();
  }

  void RenderLayer::Dirty3DTransformedDescendantStatus() noexcept
  {
    RenderLayer *curr = StackingContext();
    if (curr)
    {
      curr->_3DTransformedDescendantStatusDirty = true;
    }

    // This propagates up through preserve-3d hierarchies to the enclosing flattening layer.
    // Note that preserves3D() creates stacking context, so we can just run up the stacking containers.
    while (curr && curr->Preserves3D())
    {
      curr->_3DTransformedDescendantStatusDirty = true;
      curr = curr->StackingContext();
    }
  }

  // Return true if this layer or any preserve-3d descendants have 3d.
  bool RenderLayer::Update3DTransformedDescendantStatus() noexcept
  {
    if (_3DTransformedDescendantStatusDirty)
    {
      _has3DTransformedDescendant = false;

      UpdateZOrderLists();

      // Transformed or preserve-3d descendants can only be in the z-order lists, not
      // in the normal flow list, so we only need to check those.
      for (auto *layer : PositiveZOrderLayers())
      {
        _has3DTransformedDescendant |= layer->Update3DTransformedDescendantStatus();
      }

      // Now check our negative z-index children.
      for (auto *layer : NegativeZOrderLayers())
      {
        _has3DTransformedDescendant |= layer->Update3DTransformedDescendantStatus();
      }

      _3DTransformedDescendantStatusDirty = false;
    }

    // If we live in a 3d hierarchy, then the layer at the root of that hierarchy needs
    // the m_has3DTransformedDescendant set.
    if (Preserves3D())
    {
      return Has3DTransform() || _has3DTransformedDescendant;
    }

    return Has3DTransform();
  }

  bool RenderLayer::ShouldBeSelfPaintingLayer() const noexcept
  {
    if (!IsNormalFlowOnly())
    {
      return true;
    }

    // FIXME: Overlay scrollbars, composited scrollable overflow, canvases, video, embedded objects and
    // iframes too, once there are any.
    return Renderer().IsRenderTableRow() || Renderer().IsRenderFragmentedFlow();
  }

  void RenderLayer::UpdateSelfPaintingLayer() noexcept
  {
    bool isSelfPaintingLayer = ShouldBeSelfPaintingLayer();
    if (_isSelfPaintingLayer == isSelfPaintingLayer)
    {
      return;
    }

    _isSelfPaintingLayer = isSelfPaintingLayer;
    SetNeedsPositionUpdate();

    if (!Parent())
    {
      return;
    }

    if (isSelfPaintingLayer)
    {
      Parent()->SetAncestorChainHasSelfPaintingLayerDescendant();
    }
    else
    {
      Parent()->DirtyAncestorChainHasSelfPaintingLayerDescendantStatus();
      // FIXME: Clear the repaint rects, and update the float painter of a floating box, once there's painting
      // and floats.
    }
  }

  bool RenderLayer::HasVisibleBoxDecorationsOrBackground() const noexcept
  {
    return Renderer().HasVisibleBoxDecorations() || Renderer().Style().HasOutline();
  }

  bool RenderLayer::HasVisibleBoxDecorations() const noexcept
  {
    if (!HasVisibleContent())
    {
      return false;
    }

    // FIXME: Overflow controls, once there's a scrollable area.
    return HasVisibleBoxDecorationsOrBackground();
  }

  bool RenderLayer::IsVisibilityHiddenOrOpacityZero() const noexcept
  {
    return !HasVisibleContent() || Renderer().Style().GetOpacity().IsTransparent();
  }

  bool RenderLayer::IsSubtreeVisibilityHiddenOrOpacityZero() const noexcept
  {
    return (!HasVisibleContent() && !HasVisibleDescendant())
           || Renderer().Style().GetOpacity().IsTransparent();
  }

#pragma endregion

#pragma region Style

  void RenderLayer::StyleChanged(style::Difference, const RenderStyle *oldStyle) noexcept
  {
    SetIsNormalFlowOnly(ShouldBeNormalFlowOnly());

    if (SetIsCSSStackingContext(ShouldBeCSSStackingContext()))
    {
      if (Parent())
      {
        if (IsCSSStackingContext())
        {
          if (!HasNotIsolatedBlendingDescendantsStatusDirty() && HasNotIsolatedBlendingDescendants())
          {
            Parent()->DirtyAncestorChainHasBlendingDescendants();
          }
        }
        else
        {
          if (HasNotIsolatedBlendingDescendantsStatusDirty())
          {
            Parent()->DirtyAncestorChainHasBlendingDescendants();
          }
          else if (HasNotIsolatedBlendingDescendants())
          {
            Parent()->UpdateAncestorChainHasBlendingDescendants();
          }
        }
      }
    }

    // FIXME: Create or destroy the scrollable area (updateLayerScrollableArea), once there is one.

    // FIXME: RenderLayer already handles visibility changes through our visibility dirty bits. This logic
    // could likely be folded along with the rest.
    if (oldStyle)
    {
      bool visibilityChanged = oldStyle->UsedVisibility() != Renderer().Style().UsedVisibility();
      if (oldStyle->UsedZIndex() != Renderer().Style().UsedZIndex()
          || oldStyle->UsedContentVisibility() != Renderer().Style().UsedContentVisibility()
          || visibilityChanged)
      {
        DirtyStackingContextZOrderLists();
        if (IsStackingContext())
        {
          DirtyZOrderLists();
        }
      }

      // FIXME: The view transition name and the scrollable area's scroll corner and composited scrollable
      // overflow, once there are view transitions and a scrollable area.

      if (oldStyle->IsOverflowVisible() != Renderer().Style().IsOverflowVisible())
      {
        SetSelfAndDescendantsNeedPositionUpdate();
      }

      if (oldStyle->GetOpacity().IsTransparent() != Renderer().Style().GetOpacity().IsTransparent())
      {
        SetNeedsPositionUpdate();
      }

      if (oldStyle->Preserves3D() != Preserves3D())
      {
        Dirty3DTransformedDescendantStatus();
      }
    }

    // FIXME: The marquee and the scrollbars, once there's a scrollable area.

    // Overlay scrollbars can make this layer self-painting so we need
    // to recompute the bit once scrollbars have been updated.
    UpdateSelfPaintingLayer();

    UpdateDescendantDependentFlags();
    UpdateBlendMode();
    // FIXME: Filters, once there's painting.
    ClearClipRects();

    // FIXME: Tell the compositor, once there is one.

    UpdateTransform();

    if (oldStyle && oldStyle->HasViewportConstrainedPosition() != IsViewportConstrained())
    {
      DirtyAncestorChainHasViewportConstrainedDescendantStatus();
    }
  }

#pragma endregion

#pragma region Transforms

  void RenderLayer::UpdateTransformFromStyle(TransformationMatrix &transform, const RenderStyle &style,
                                             EnumSet<style::TransformResolverOption> options) const noexcept
  {
    // FIXME: The default scroll shift of anchor-positioned boxes goes first
    // (https://drafts.csswg.org/css-anchor-position-1/#default-scroll-shift), once there's anchor
    // positioning.

    // FIXME: Snap the reference box to device pixels when the renderer needs it
    // (snapRectToDevicePixelsIfNeeded), once there's painting.
    auto referenceBoxRect = Renderer().TransformReferenceBoxRect(style);
    Renderer().ApplyTransform(transform, style, referenceBoxRect, options);

    // FIXME: 3D rendering (canRender3DTransforms), once there's compositing.
    MakeMatrixRenderable(transform, false);
  }

  void RenderLayer::UpdateTransform() noexcept
  {
    bool hasTransform = IsTransformed();
    bool had3DTransform = Has3DTransform();

    UniquePtr<TransformationMatrix> oldTransform;
    if (_transform && hasTransform)
    {
      oldTransform = CreateUnique<TransformationMatrix>(*_transform);
    }
    if (hasTransform != static_cast<bool>(_transform))
    {
      if (hasTransform)
      {
        _transform = CreateUnique<TransformationMatrix>();
      }
      else
      {
        _transform = nullptr;
      }

      // Layers with transforms act as clip rects roots, so clear the cached clip rects here.
      ClearClipRectsIncludingDescendants();
      SetSelfAndDescendantsNeedPositionUpdate();
    }

    if (hasTransform)
    {
      _transform->MakeIdentity();
      UpdateTransformFromStyle(*_transform, Renderer().Style(),
                               style::TransformResolver::AllTransformOperations);
    }

    if (had3DTransform != Has3DTransform())
    {
      Dirty3DTransformedDescendantStatus();
    }

    if (oldTransform && _transform && *oldTransform != *_transform)
    {
      SetSelfAndDescendantsNeedPositionUpdate();
    }
  }

  TransformationMatrix
    RenderLayer::CurrentTransform(EnumSet<style::TransformResolverOption> options) const noexcept
  {
    if (!_transform)
    {
      return {};
    }

    // m_transform includes transform-origin and is affected by the choice of the transform-box.
    // Therefore we can only use the cached m_transform, if the animation doesn't alter transform-box or
    // excludes transform-origin.
    // FIXME: Running accelerated transform animations use the animated style, once there are animations.
    if (!options.Contains(style::TransformResolverOption::TransformOrigin))
    {
      TransformationMatrix transform;
      UpdateTransformFromStyle(transform, Renderer().Style(), options);
      return transform;
    }

    return *_transform;
  }

  TransformationMatrix RenderLayer::CurrentTransform() const noexcept
  {
    return CurrentTransform(style::TransformResolver::AllTransformOperations);
  }

  TransformationMatrix RenderLayer::PerspectiveTransform() const noexcept
  {
    if (!Renderer().HasTransformRelatedProperty())
    {
      return {};
    }

    const auto &style = Renderer().Style();
    if (!style.HasPerspective())
    {
      return {};
    }

    auto transformReferenceBoxRect = Renderer().TransformReferenceBoxRect(style);

    TransformationMatrix transform;
    style::TransformResolver transformResolver {transform, style};

    auto perspectiveOrigin = transformResolver.ComputePerspectiveOrigin(transformReferenceBoxRect);

    // In the regular case of a non-clipped, non-scrolled GraphicsLayer, all transformations
    // (via CSS 'transform' / 'perspective') are applied with respect to a predefined anchor point,
    // which depends on the chosen CSS 'transform-box' / 'transform-origin' properties.
    //
    // A transformation given by the CSS 'transform' property is applied, by translating
    // to the 'transform origin', applying the transformation, and translating back.
    // When an element specifies a CSS 'perspective' property, the perspective transformation matrix
    // that's computed here is propagated to the GraphicsLayer by calling setChildrenTransform().
    //
    // However the GraphicsLayer platform implementations (e.g. CA on macOS) apply the children transform,
    // defined on the parent, with respect to the anchor point of the parent, when rendering child elements.
    // This is wrong, as the perspective transformation (applied to a child of the element defining the
    // 3d effect), must be independent of the chosen transform-origin (the parents transform origin
    // must not affect its children).
    //
    // To circumvent this, explicitly remove the transform-origin dependency in the perspective matrix.
    auto transformOrigin = TransformOriginPixelSnappedIfNeeded();

    transformResolver.UnapplyTransformOrigin(transformOrigin);
    transformResolver.ApplyPerspective(perspectiveOrigin);
    transformResolver.ApplyTransformOrigin(transformOrigin);

    return transform;
  }

  FloatPoint3D RenderLayer::TransformOriginPixelSnappedIfNeeded() const noexcept
  {
    if (!Renderer().HasTransformRelatedProperty())
    {
      return {};
    }

    const auto &style = Renderer().Style();
    auto referenceBoxRect = Renderer().TransformReferenceBoxRect(style);

    // FIXME: Snap the origin to device pixels when the renderer needs it (rendererNeedsPixelSnapping), once
    // there's painting.
    return style::TransformResolver::ComputeTransformOrigin(style, referenceBoxRect);
  }

  FloatPoint RenderLayer::PerspectiveOrigin() const noexcept
  {
    if (!Renderer().HasTransformRelatedProperty())
    {
      return {};
    }
    return style::TransformResolver::ComputePerspectiveOrigin(
      Renderer().Style(), Renderer().TransformReferenceBoxRect(Renderer().Style()));
  }

  bool RenderLayer::IsTransformed() const noexcept
  {
    // FIXME: The anchor scroll adjustment applies a transform too, once there's anchor positioning.
    return Renderer().IsTransformed();
  }

  bool RenderLayer::Preserves3D() const noexcept
  {
    return Renderer().Style().Preserves3D();
  }

  bool RenderLayer::HasPerspective() const noexcept
  {
    return Renderer().Style().HasPerspective();
  }

  bool RenderLayer::PaintsWithTransform() const noexcept
  {
    // FIXME: Composited layers paint their transforms in the backing, once there's compositing.
    return Transform();
  }

  RenderLayer *RenderLayer::EnclosingTransformedAncestor() const noexcept
  {
    RenderLayer *curr = Parent();
    while (curr && !curr->IsRenderViewLayer() && !curr->Transform())
    {
      curr = curr->Parent();
    }

    return curr;
  }

#pragma endregion

#pragma region Queries

  bool RenderLayer::IsTransparent() const noexcept
  {
    return Renderer().IsTransparent() || Renderer().HasMask();
  }

  bool RenderLayer::HasFilter() const noexcept
  {
    return Renderer().HasFilter();
  }

  bool RenderLayer::HasBackdropFilter() const noexcept
  {
    return Renderer().HasBackdropFilter();
  }

  // FIXME: Why ask the renderer this given we have m_blendMode?
  bool RenderLayer::HasBlendMode() const noexcept
  {
    return Renderer().HasBlendMode();
  }

  bool RenderLayer::IsViewportConstrained() const noexcept
  {
    return Renderer().IsFixedPositioned() || Renderer().IsStickilyPositioned();
  }

  bool RenderLayer::IsRenderFragmentedFlow() const noexcept
  {
    return Renderer().IsRenderFragmentedFlow();
  }

  bool RenderLayer::CanUseOffsetFromAncestor() const noexcept
  {
    // FIXME: This really needs to know if there are transforms on this layer and any of the layers between it
    // and the ancestor in question.
    // FIXME: SVG roots too, once there's SVG.
    return !IsTransformed();
  }

  bool RenderLayer::CanUseOffsetFromAncestor(const RenderLayer &ancestor) const noexcept
  {
    for (const auto *layer = this; layer && layer != &ancestor; layer = layer->Parent())
    {
      if (!layer->CanUseOffsetFromAncestor())
      {
        return false;
      }
    }
    return true;
  }

  static bool IsContainerForPositioned(RenderLayer &layer, PositionType position) noexcept
  {
    // FIXME: The top layer's container is the view's layer, once there's a top layer.
    switch (position)
    {
      case PositionType::Fixed:    return layer.Renderer().CanContainFixedPositionObjects();

      case PositionType::Absolute: return layer.Renderer().CanContainAbsolutelyPositionedObjects();

      default:                     krys_debug_assert(false); return false;
    }
  }

  bool RenderLayer::AncestorLayerIsInContainingBlockChain(const RenderLayer &ancestor,
                                                          const RenderLayer *checkLimit) const noexcept
  {
    if (&ancestor == this)
    {
      return true;
    }

    for (const auto *currentBlock = Renderer().ContainingBlock();
         currentBlock && !Is<RenderView>(*currentBlock); currentBlock = currentBlock->ContainingBlock())
    {
      auto *currLayer = currentBlock->Layer();
      if (currLayer == &ancestor)
      {
        return true;
      }

      if (currLayer && currLayer == checkLimit)
      {
        return false;
      }
    }

    return false;
  }

  RenderLayer *RenderLayer::EnclosingAncestorForPosition(PositionType position) const noexcept
  {
    auto *curr = Parent();
    while (curr && !IsContainerForPositioned(*curr, position))
    {
      curr = curr->Parent();
    }

    return curr;
  }

  RenderLayer *RenderLayer::EnclosingLayerInContainingBlockOrder() const noexcept
  {
    for (const auto *currentBlock = Renderer().ContainingBlock(); currentBlock;
         currentBlock = currentBlock->ContainingBlock())
    {
      if (auto *layer = currentBlock->Layer())
      {
        return layer;
      }
    }

    return nullptr;
  }

  RenderLayer *RenderLayer::EnclosingContainingBlockLayer(CrossFrameBoundaries) const noexcept
  {
    // FIXME: Cross into the owner frame's layer (enclosingFrameRenderLayer), once there are frames.
    return EnclosingLayerInContainingBlockOrder();
  }

  RenderLayer *RenderLayer::EnclosingOverflowClipLayer(IncludeSelfOrNot includeSelf) const noexcept
  {
    const RenderLayer *layer = (includeSelf == IncludeSelf) ? this : Parent();
    while (layer)
    {
      if (layer->Renderer().HasPotentiallyScrollableOverflow())
      {
        return const_cast<RenderLayer *>(layer);
      }

      layer = layer->Parent();
    }
    return nullptr;
  }

  RenderLayer *RenderLayer::ClippingRootForPainting() const noexcept
  {
    // FIXME: Composited layers, layers that paint into a provided backing and hidden backfaces in preserve-3d
    // contexts, once there's compositing.
    const RenderLayer *current = this;
    while (current)
    {
      if (current->IsRenderViewLayer())
      {
        return const_cast<RenderLayer *>(current);
      }

      current = current->PaintOrderParent();
      krys_debug_assert(current);
      if (current->Transform())
      {
        return const_cast<RenderLayer *>(current);
      }
    }

    krys_debug_assert(false);
    return nullptr;
  }

#pragma endregion

#pragma region Coordinates

  void RenderLayer::ConvertToPixelSnappedLayerCoords(const RenderLayer *ancestorLayer,
                                                     IntPoint &roundedLocation,
                                                     ColumnOffsetAdjustment adjustForColumns) const noexcept
  {
    LayoutPoint location =
      ConvertToLayerCoords(ancestorLayer, LayoutPoint(roundedLocation), adjustForColumns);
    roundedLocation = RoundedIntPoint(location);
  }

  // Returns the layer reached on the walk up towards the ancestor.
  static const RenderLayer *AccumulateOffsetTowardsAncestor(const RenderLayer *layer,
                                                            const RenderLayer *ancestorLayer,
                                                            LayoutPoint &location,
                                                            RenderLayer::ColumnOffsetAdjustment) noexcept
  {
    krys_debug_assert(ancestorLayer != layer);

    const auto &renderer = layer->Renderer();
    auto position = renderer.Style().GetPosition();

    // FIXME: Fixed positioned boxes whose container is the root add the view's offset (localToAbsolute), once
    // there's geometry mapping.
    if (position == PositionType::Fixed && (!ancestorLayer || ancestorLayer == renderer.View().Layer()))
    {
      location.MoveBy(layer->Location());
      return ancestorLayer;
    }

    if (position == PositionType::Fixed)
    {
      // For a fixed layers, we need to walk up to the root to see if there's a fixed position container
      // (e.g. a transformed layer). It's an error to call offsetFromAncestor() across a layer with a
      // transform, so we should always find the ancestor at or before we find the fixed position container,
      // if the container is transformed.
      RenderLayer *fixedPositionContainerLayer = nullptr;
      bool foundAncestor = false;
      for (auto *currLayer = layer->Parent(); currLayer; currLayer = currLayer->Parent())
      {
        if (currLayer == ancestorLayer)
        {
          foundAncestor = true;
        }

        if (IsContainerForPositioned(*currLayer, PositionType::Fixed))
        {
          fixedPositionContainerLayer = currLayer;
          // A layer that has a transform-related property but not a
          // transform still acts as a fixed-position container.
          // Accumulating offsets across such layers is allowed.
          if (currLayer->Transform())
          {
            krys_debug_assert(foundAncestor);
          }
          break;
        }
      }

      krys_debug_assert(fixedPositionContainerLayer); // We should have hit the RenderView's layer at least.

      if (fixedPositionContainerLayer != ancestorLayer)
      {
        auto fixedContainerCoords = layer->OffsetFromAncestor(fixedPositionContainerLayer);
        auto ancestorCoords =
          foundAncestor ? ancestorLayer->OffsetFromAncestor(fixedPositionContainerLayer) : LayoutSize {};
        location.Move(fixedContainerCoords - ancestorCoords);
        return foundAncestor ? ancestorLayer : fixedPositionContainerLayer;
      }
    }

    RenderLayer *parentLayer;
    if (position == PositionType::Absolute || position == PositionType::Fixed)
    {
      // Do what enclosingAncestorForPosition() does, but check for ancestorLayer along the way.
      parentLayer = layer->Parent();
      bool foundAncestorFirst = false;
      while (parentLayer)
      {
        // RenderFragmentedFlow is a positioned container, child of RenderView, positioned at (0,0).
        // This implies that, for out-of-flow positioned elements inside a RenderFragmentedFlow,
        // we are bailing out before reaching root layer.
        if (IsContainerForPositioned(*parentLayer, position))
        {
          break;
        }

        if (parentLayer == ancestorLayer)
        {
          foundAncestorFirst = true;
          break;
        }

        parentLayer = parentLayer->Parent();
      }

      if (foundAncestorFirst)
      {
        // Found ancestorLayer before the abs. positioned container, so compute offset of both relative
        // to enclosingAncestorForPosition and subtract.
        auto *positionedAncestor = parentLayer->EnclosingAncestorForPosition(position);
        auto thisCoords = layer->OffsetFromAncestor(positionedAncestor);
        auto ancestorCoords = ancestorLayer->OffsetFromAncestor(positionedAncestor);
        location.Move(thisCoords - ancestorCoords);
        return ancestorLayer;
      }
    }
    else
    {
      parentLayer = layer->Parent();
    }

    if (!parentLayer)
    {
      return nullptr;
    }

    location.MoveBy(layer->Location());

    // FIXME: Translate from the multi-column flow into its column (AdjustForColumns), once there are columns.

    return parentLayer;
  }

  LayoutPoint RenderLayer::ConvertToLayerCoords(const RenderLayer *ancestorLayer, const LayoutPoint &location,
                                                ColumnOffsetAdjustment adjustForColumns) const noexcept
  {
    if (ancestorLayer == this)
    {
      return location;
    }

    const auto *currLayer = this;
    auto locationInLayerCoords = location;
    while (currLayer && currLayer != ancestorLayer)
    {
      currLayer =
        AccumulateOffsetTowardsAncestor(currLayer, ancestorLayer, locationInLayerCoords, adjustForColumns);
    }

    // FIXME: Snap the whole SVG subtree as one block, once there's SVG.
    return locationInLayerCoords;
  }

  LayoutSize RenderLayer::OffsetFromAncestor(const RenderLayer *ancestorLayer,
                                             ColumnOffsetAdjustment adjustForColumns) const noexcept
  {
    return ToLayoutSize(ConvertToLayerCoords(ancestorLayer, LayoutPoint(), adjustForColumns));
  }

#pragma endregion

#pragma region Renderer geometry

  LayoutPoint RenderLayer::RendererLocation() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->Location();
    }
    return {};
  }

  LayoutRect RenderLayer::RendererBorderBoxRect() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->BorderBoxRect();
    }
    return {};
  }

  LayoutRect RenderLayer::RendererVisualOverflowRect() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->VisualOverflowRect();
    }
    return {};
  }

  LayoutRect RenderLayer::RendererOverflowClipRect(const LayoutPoint &location,
                                                   OverlayScrollbarSizeRelevancy relevancy) const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->OverflowClipRect(location, relevancy);
    }
    return {};
  }

  LayoutRect RenderLayer::RendererOverflowClipRectForChildLayers(
    const LayoutPoint &location, OverlayScrollbarSizeRelevancy relevancy) const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->OverflowClipRectForChildLayers(location, relevancy);
    }
    return {};
  }

  bool RenderLayer::RendererHasVisualOverflow() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->HasVisualOverflow();
    }
    return false;
  }

#pragma endregion

#pragma region Clip rects

  Ref<ClipRects> RenderLayer::UpdateClipRects(const ClipRectsContext &clipRectsContext) noexcept
  {
    ClipRectsType clipRectsType = clipRectsContext.Type;
    krys_debug_assert(clipRectsType < NumCachedClipRectsTypes);
    krys_debug_assert(!clipRectsContext.Options.Contains(ClipRectsOption::Temporary));
    krys_debug_assert(!clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter));
    if (_clipRectsCache)
    {
      if (auto *clipRects = _clipRectsCache->GetClipRects(clipRectsContext))
      {
#if KRYS_DEBUG
        krys_debug_assert(clipRectsContext.RootLayer == _clipRectsCache->ClipRectsRoot[clipRectsType]);
#endif
        return ShareRef(*clipRects); // We have the correct cached value.
      }
    }

    if (!_clipRectsCache)
    {
      _clipRectsCache = CreateUnique<ClipRectsCache>();
    }

#if KRYS_DEBUG
    _clipRectsCache->ClipRectsRoot[clipRectsType] = clipRectsContext.RootLayer;
#endif
    krys_debug_assert((clipRectsContext.GetOverlayScrollbarSizeRelevancy()
                       == OverlayScrollbarSizeRelevancy::IncludeOverlayScrollbarSize)
                      == (clipRectsContext.Type == RootRelativeClipRects));

    RefPtr<ClipRects> parentClipRects = this->ParentClipRects(clipRectsContext);

    auto clipRects = ClipRects::Create();
    CalculateClipRects(clipRectsContext, *clipRects);

    if (parentClipRects && *parentClipRects == *clipRects)
    {
      _clipRectsCache->SetClipRects(clipRectsType, clipRectsContext.RespectOverflowClip(),
                                    RefPtr<ClipRects> {parentClipRects});
      Ref<ClipRects> sharedClipRects = parentClipRects;
      return sharedClipRects;
    }

    _clipRectsCache->SetClipRects(clipRectsType, clipRectsContext.RespectOverflowClip(),
                                  RefPtr<ClipRects> {clipRects});
    return clipRects;
  }

  bool RenderLayer::ClipCrossesPaintingBoundary() const noexcept
  {
    // FIXME: The enclosing compositing layers differ too, once there's compositing.
    return Parent()->EnclosingPaginationLayer(IncludeCompositedPaginatedLayers)
           != EnclosingPaginationLayer(IncludeCompositedPaginatedLayers);
  }

  void RenderLayer::CalculateClipRects(const ClipRectsContext &clipRectsContext,
                                       ClipRects &clipRects) const noexcept
  {
    if (!Parent())
    {
      // The root layer's clip rect is always infinite.
      clipRects.Reset();
      return;
    }

    if (auto parentClipRects = this->ParentClipRects(clipRectsContext))
    {
      clipRects = *parentClipRects;
    }
    else
    {
      clipRects.Reset();
    }

    // A fixed object is essentially the root of its containing block hierarchy, so when
    // we encounter such an object, we reset our clip rects to the fixedClipRect.
    if (Renderer().IsFixedPositioned())
    {
      clipRects.SetPosClipRect(clipRects.FixedClipRect());
      clipRects.SetOverflowClipRect(clipRects.FixedClipRect());
      clipRects.SetFixed(true);
    }
    else if (Renderer().IsInFlowPositioned())
    {
      clipRects.SetPosClipRect(clipRects.OverflowClipRect());
    }
    else if (Renderer().ShouldUsePositionedClipping())
    {
      clipRects.SetOverflowClipRect(clipRects.PosClipRect());
    }

    // Update the clip rects that will be passed to child layers.
    if ((Renderer().HasNonVisibleOverflow()
         && (clipRectsContext.RespectOverflowClip() || this != clipRectsContext.RootLayer))
        || Renderer().HasClip())
    {
      // This layer establishes a clip of some kind.

      // FIXME: Transforming a clip doesn't make a whole lot of sense, since it we have to round out to the
      // bounding box of the transformed quad.
      // It would be better for callers to transform rects into the coordinate space of the nearest clipped
      // layer, apply the clip in local space, and then repeat until the required coordinate space is reached.
      // FIXME: Transform the clips into the root's coordinates (localToContainerQuad) when the offset can't
      // be used, once there's geometry mapping.

      auto offset = ToLayoutPoint(OffsetFromAncestor(clipRectsContext.RootLayer, AdjustForColumns));

      // FIXME: Fixed clips relative to the view move by the view's scroll position for fixed positioning,
      // once there's scrolling.

      if (Renderer().HasNonVisibleOverflow())
      {
        ClipRect newOverflowClip =
          RendererOverflowClipRectForChildLayers({}, clipRectsContext.GetOverlayScrollbarSizeRelevancy());
        newOverflowClip.MoveBy(offset);
        newOverflowClip.SetAffectedByRadius(Renderer().Style().HasBorderRadius());
        clipRects.SetOverflowClipRect(Intersection(newOverflowClip, clipRects.OverflowClipRect()));
        if (Renderer().CanContainAbsolutelyPositionedObjects())
        {
          clipRects.SetPosClipRect(Intersection(newOverflowClip, clipRects.PosClipRect()));
        }
        if (Renderer().CanContainFixedPositionObjects())
        {
          clipRects.SetFixedClipRect(Intersection(newOverflowClip, clipRects.FixedClipRect()));
        }
      }
      if (Renderer().HasClip())
      {
        if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
        {
          LayoutRect newPosClip = box->ClipRect({});
          newPosClip.MoveBy(offset);
          clipRects.SetPosClipRect(Intersection(newPosClip, clipRects.PosClipRect()));
          clipRects.SetOverflowClipRect(Intersection(newPosClip, clipRects.OverflowClipRect()));
          clipRects.SetFixedClipRect(Intersection(newPosClip, clipRects.FixedClipRect()));
        }
      }
    }
    else if (Renderer().HasNonVisibleOverflow() && Transform() && Renderer().Style().HasBorderRadius())
    {
      clipRects.SetOverflowClipRectAffectedByRadius();
    }
  }

  RefPtr<ClipRects> RenderLayer::ParentClipRects(const ClipRectsContext &clipRectsContext) const noexcept
  {
    auto *containerLayer = Parent();
    if (clipRectsContext.RootLayer == this || !Parent())
    {
      return nullptr;
    }

    // FIXME: Ancestor clipping inside filters is suppressed (m_suppressAncestorClippingInsideFilter), once
    // there are filters.

    auto temporaryParentClipRects = [&](const ClipRectsContext &clipContext)
    {
      auto parentClipRects = ClipRects::Create();
      containerLayer->CalculateClipRects(clipContext, *parentClipRects);
      RefPtr<ClipRects> result = parentClipRects;
      return result;
    };

    if (clipRectsContext.Options.Contains(ClipRectsOption::Temporary)
        || clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter))
    {
      return temporaryParentClipRects(clipRectsContext);
    }

    if (clipRectsContext.Type != AbsoluteClipRects && ClipCrossesPaintingBoundary())
    {
      ClipRectsContext tempClipRectsContext(clipRectsContext);
      tempClipRectsContext.Options.Add(ClipRectsOption::Temporary);
      return temporaryParentClipRects(tempClipRectsContext);
    }

    return containerLayer->UpdateClipRects(clipRectsContext);
  }

  static ClipRect BackgroundClipRectForPosition(const ClipRects &parentRects, PositionType position) noexcept
  {
    if (position == PositionType::Fixed)
    {
      return parentRects.FixedClipRect();
    }

    if (position == PositionType::Absolute)
    {
      return parentRects.PosClipRect();
    }

    return parentRects.OverflowClipRect();
  }

  ClipRect RenderLayer::BackgroundClipRect(const ClipRectsContext &clipRectsContext) const noexcept
  {
    krys_debug_assert(Parent());
    ClipRect backgroundClipRect;
    auto parentRects = ParentClipRects(clipRectsContext);
    if (!parentRects)
    {
      backgroundClipRect.Reset();
      return backgroundClipRect;
    }
    backgroundClipRect = BackgroundClipRectForPosition(*parentRects, Renderer().Style().GetPosition());
    // FIXME: Fixed clips relative to the view move by the view's scroll position for fixed positioning, once
    // there's scrolling.
    return backgroundClipRect;
  }

  ClipRect RenderLayer::CalculateBackgroundClipRect(const ClipRectsContext &clipRectsContext,
                                                    const LayoutSize &offsetFromRoot) const noexcept
  {
    ClipRect backgroundClipRect = LayoutRect::InfiniteRect();

    if (clipRectsContext.RootLayer != this && Parent())
    {
      backgroundClipRect = this->BackgroundClipRect(clipRectsContext);
    }

    if (Renderer().HasClip())
    {
      if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
      {
        // Clip applies to *us* as well, so update the damageRect.
        auto newPosClip = box->ClipRect(ToLayoutPoint(offsetFromRoot));
        backgroundClipRect.Intersect(newPosClip);
      }
    }

    return backgroundClipRect;
  }

  ClipRect RenderLayer::CalculateBackgroundRect(const ClipRectsContext &clipRectsContext,
                                                const LayoutSize &offsetFromRoot) const noexcept
  {
    auto backgroundRect = CalculateBackgroundClipRect(clipRectsContext, offsetFromRoot);

    if (clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter)
        || !Renderer().HasClipOrNonVisibleOverflow())
    {
      return backgroundRect;
    }

    if (this == clipRectsContext.RootLayer && !clipRectsContext.RespectOverflowClip())
    {
      return backgroundRect;
    }

    // If we establish a clip at all, then make sure our background rect is intersected with our layer's
    // bounds including our visual overflow, since any visual overflow like box-shadow or border-outset is not
    // clipped by overflow:auto/hidden.
    if (RendererHasVisualOverflow())
    {
      // FIXME: Does not do the right thing with CSS regions yet, since we don't yet factor in the
      // individual region boxes as overflow.
      auto layerBoundsWithVisualOverflow = RendererVisualOverflowRect();
      if (auto *box = GetRenderBox())
      {
        // Layers are in physical coordinates, so the overflow has to be flipped.
        box->FlipForWritingMode(layerBoundsWithVisualOverflow);
      }

      layerBoundsWithVisualOverflow.Move(offsetFromRoot);
      backgroundRect.Intersect(layerBoundsWithVisualOverflow);
      return backgroundRect;
    }

    auto bounds = RendererBorderBoxRect();

    // Shift the bounds to be for our region only.
    bounds.Move(offsetFromRoot);
    backgroundRect.Intersect(bounds);
    return backgroundRect;
  }

  ClipRect RenderLayer::CalculateForegroundRect(const ClipRectsContext &clipRectsContext,
                                                const LayoutSize &offsetFromRoot) const noexcept
  {
    auto foregroundRect = CalculateBackgroundClipRect(clipRectsContext, offsetFromRoot);

    if (clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter)
        || !Renderer().HasNonVisibleOverflow())
    {
      return foregroundRect;
    }

    // This layer establishes a clip of some kind.
    if (this != clipRectsContext.RootLayer || clipRectsContext.RespectOverflowClip())
    {
      auto overflowClipRect = RendererOverflowClipRect(ToLayoutPoint(offsetFromRoot),
                                                       clipRectsContext.GetOverlayScrollbarSizeRelevancy());
      foregroundRect.Intersect(overflowClipRect);
      foregroundRect.SetAffectedByRadius(true);
      return foregroundRect;
    }

    if (Transform() && Renderer().Style().HasBorderRadius())
    {
      foregroundRect.SetAffectedByRadius(true);
    }

    return foregroundRect;
  }

  LayerFragment::Rects RenderLayer::CalculateRects(const ClipRectsContext &clipRectsContext,
                                                   const LayoutSize &offsetFromRoot,
                                                   const LayoutRect &paintDirtyRect) const noexcept
  {
    auto layerBounds = LayoutRect(ToLayoutPoint(offsetFromRoot), LayoutSize(Size()));
    auto backgroundRect = CalculateBackgroundRect(clipRectsContext, offsetFromRoot);
    auto foregroundRect = CalculateForegroundRect(clipRectsContext, offsetFromRoot);
    return {layerBounds, paintDirtyRect, backgroundRect, foregroundRect};
  }

  LayoutRect RenderLayer::ClipRectRelativeToAncestor(const RenderLayer *ancestor,
                                                     LayoutSize offsetFromAncestor,
                                                     const LayoutRect &constrainingRect,
                                                     bool temporaryClipRects) const noexcept
  {
    auto options = ClipRectDefaultOptions;
    if ((_enclosingPaginationLayer && _enclosingPaginationLayer.get() != ancestor) || temporaryClipRects)
    {
      options.Add(ClipRectsOption::Temporary);
    }
    ClipRectsContext clipRectsContext(ancestor, PaintingClipRects, options);
    auto backgroundRect = CalculateBackgroundRect(clipRectsContext, offsetFromAncestor);
    return Intersection(backgroundRect.Rect(), constrainingRect);
  }

  LayoutRect RenderLayer::LocalClipRect(bool &clipExceedsBounds, LocalClipRectMode mode) const noexcept
  {
    clipExceedsBounds = false;
    // FIXME: border-radius not accounted for.
    // FIXME: Regions not accounted for.
    const RenderLayer *clippingRootLayer =
      mode == LocalClipRectMode::ExcludeCompositingState ? this : ClippingRootForPainting();
    LayoutSize offsetFromRoot = OffsetFromAncestor(clippingRootLayer);
    LayoutRect clipRect =
      ClipRectRelativeToAncestor(clippingRootLayer, offsetFromRoot, LayoutRect::InfiniteRect());
    if (clipRect.IsInfinite())
    {
      return clipRect;
    }

    if (Renderer().HasClip())
    {
      if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
      {
        // CSS clip may be larger than our border box.
        LayoutRect cssClipRect = box->ClipRect({});
        clipExceedsBounds =
          !cssClipRect.IsEmpty()
          && (clipRect.Width() < cssClipRect.Width() || clipRect.Height() < cssClipRect.Height());
      }
    }

    clipRect.Move(-offsetFromRoot);
    return clipRect;
  }

  void RenderLayer::ClearClipRectsIncludingDescendants(ClipRectsType typeToClear) noexcept
  {
    // FIXME: it's not clear how this layer not having clip rects guarantees that no descendants have any.
    if (!_clipRectsCache)
    {
      return;
    }

    ClearClipRects(typeToClear);

    for (RenderLayer *l = FirstChild(); l; l = l->NextSibling())
    {
      l->ClearClipRectsIncludingDescendants(typeToClear);
    }
  }

  void RenderLayer::ClearClipRects(ClipRectsType typeToClear) noexcept
  {
    if (typeToClear == AllClipRectTypes)
    {
      _clipRectsCache = nullptr;
    }
    else if (_clipRectsCache)
    {
      krys_debug_assert(typeToClear < NumCachedClipRectsTypes);
      _clipRectsCache->SetClipRects(typeToClear, RespectOverflowClip, nullptr);
      _clipRectsCache->SetClipRects(typeToClear, IgnoreOverflowClip, nullptr);
    }
  }

#pragma endregion

#pragma region Bounds

  bool RenderLayer::IntersectsDamageRect(const LayoutRect &layerBounds, const LayoutRect &damageRect,
                                         const RenderLayer *rootLayer, const LayoutSize &offsetFromRoot,
                                         const Maybe<LayoutRect> &cachedBoundingBox) const noexcept
  {
    // Always examine the canvas and the root.
    // FIXME: Could eliminate the isDocumentElementRenderer() check if we fix background painting so that the
    // RenderView paints the root's background.
    if (IsRenderViewLayer() || Renderer().IsDocumentElementRenderer())
    {
      return true;
    }

    if (damageRect.IsInfinite())
    {
      return true;
    }

    if (damageRect.IsEmpty())
    {
      return false;
    }

    // If we aren't an inline flow, and our layer bounds do intersect the damage rect, then we can return
    // true.
    if (!Renderer().IsRenderInline() && layerBounds.Intersects(damageRect))
    {
      return true;
    }

    // Otherwise we need to compute the bounding box of this single layer and see if it intersects
    // the damage rect. It's possible the fragment computed the bounding box already, in which case we
    // can use the cached value.
    if (cachedBoundingBox)
    {
      return cachedBoundingBox->Intersects(damageRect);
    }

    return BoundingBox(rootLayer, offsetFromRoot).Intersects(damageRect);
  }

  LayoutRect RenderLayer::LocalBoundingBox(EnumSet<CalculateLayerBoundsFlag> flags) const noexcept
  {
    // There are three special cases we need to consider.
    // (1) Inline Flows.  For inline flows we will create a bounding box that fully encompasses all of the
    // lines occupied by the inline.  In other words, if some <span> wraps to three lines, we'll create a
    // bounding box that fully encloses the line boxes of all three lines (including overflow on those lines).
    // (2) Left/Top Overflow.  The width/height of layers already includes right/bottom overflow.  However, in
    // the case of left/top overflow, we have to create a bounding box that will extend to include this
    // overflow. (3) Floats.  When a layer has overhanging floats that it paints, we need to make sure to
    // include these overhanging floats as part of our bounding box.  We do this because we are the
    // responsible layer for both hit testing and painting those floats.
    LayoutRect result;
    if (auto *renderInline = DynamicDowncast<RenderInline>(Renderer()); renderInline && Renderer().IsInline())
    {
      // FIXME: The visual overflow of the inline's lines (linesVisualOverflowBoundingBox), once there's
      // inline layout.
      result = LayoutRect(LayoutPoint(), LayoutSize(renderInline->LinesBoundingBox().Size()));
    }
    else if (auto *tableRow = DynamicDowncast<RenderTableRow>(Renderer()))
    {
      // FIXME: Our bounding box is just the union of all of our cells' border/overflow rects, once there's
      // table layout.
      result = tableRow->VisualOverflowRect();
    }
    else
    {
      RenderBox *box = GetRenderBox();
      krys_debug_assert(box);
      // FIXME: The mask clip rect (maskClipRect) of boxes with a mask, unless DontConstrainForMask, once
      // there's painting.
      result = box->VisualOverflowRect();

      // FIXME: The root's background painting area (IncludeRootBackgroundPaintingArea), once the frame view
      // has contents sizes.
    }
    (void)flags;
    return result;
  }

  LayoutRect RenderLayer::BoundingBox(const RenderLayer *ancestorLayer, const LayoutSize &offsetFromRoot,
                                      EnumSet<CalculateLayerBoundsFlag> flags) const noexcept
  {
    LayoutRect result = LocalBoundingBox(flags);
    if (Renderer().View().FrameView().HasFlippedBlockRenderers())
    {
      if (auto *box = GetRenderBox())
      {
        box->FlipForWritingMode(result);
      }
      else
      {
        Renderer().ContainingBlock()->FlipForWritingMode(result);
      }
    }

    // FIXME: Split the box into the fragment boxes of its pagination layers (fragmentsBoundingBox), once
    // there's fragmentation.
    (void)ancestorLayer;

    result.Move(offsetFromRoot);
    return result;
  }

  IntRect RenderLayer::AbsoluteBoundingBox() const noexcept
  {
    const RenderLayer *rootLayer = Root();
    return SnappedIntRect(BoundingBox(rootLayer, OffsetFromAncestor(rootLayer)));
  }

  LayoutRect RenderLayer::CalculateLayerBounds(const RenderLayer *ancestorLayer,
                                               const LayoutSize &offsetFromRoot,
                                               EnumSet<CalculateLayerBoundsFlag> flags) const noexcept
  {
    if (!IsSelfPaintingLayer())
    {
      return LayoutRect();
    }

    // FIXME: This could be improved to do a check like hasVisibleNonCompositingDescendantLayers() (bug
    // 92580).
    if (flags.Contains(ExcludeHiddenDescendants) && this != ancestorLayer && !HasVisibleContent()
        && !HasVisibleDescendant())
    {
      return LayoutRect();
    }

    // FIXME: Descendants captured in a view transition (ExcludeViewTransitionCapturedDescendants), once there
    // are view transitions.

    if (IsRenderViewLayer())
    {
      // The root layer is always just the size of the document.
      // FIXME: The unscaled document rect, once the view knows it.
      return Renderer().View().LayoutOverflowRect();
    }

    LayoutRect boundingBoxRect = LocalBoundingBox(flags | IncludeRootBackgroundPaintingArea);
    if (Renderer().View().FrameView().HasFlippedBlockRenderers())
    {
      if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
      {
        box->FlipForWritingMode(boundingBoxRect);
      }
      else
      {
        Renderer().ContainingBlock()->FlipForWritingMode(boundingBoxRect);
      }
    }

    LayoutRect unionBounds = boundingBoxRect;

    if (flags.ContainsAny({UseLocalClipRectIfPossible, UseLocalClipRectExcludingCompositingIfPossible}))
    {
      bool clipExceedsBounds = false;
      LayoutRect localClipRect =
        this->LocalClipRect(clipExceedsBounds, flags.Contains(UseLocalClipRectExcludingCompositingIfPossible)
                                                 ? LocalClipRectMode::ExcludeCompositingState
                                                 : LocalClipRectMode::IncludeCompositingState);
      if (!localClipRect.IsInfinite() && !clipExceedsBounds)
      {
        if (flags.Contains(IncludeSelfTransform) && PaintsWithTransform())
        {
          localClipRect = Transform()->MapRect(localClipRect);
        }

        localClipRect.Move(OffsetFromAncestor(ancestorLayer));
        return localClipRect;
      }
    }

    // FIXME: should probably just pass 'flags' down to descendants.
    EnumSet<CalculateLayerBoundsFlag> descendantFlags = DefaultCalculateLayerBoundsFlags();
    if (flags.Contains(PreserveAncestorFlags))
    {
      descendantFlags = flags;
    }
    else
    {
      if (flags.Contains(ExcludeHiddenDescendants))
      {
        descendantFlags.Add(ExcludeHiddenDescendants);
      }
      if (flags.Contains(IncludeCompositedDescendants))
      {
        descendantFlags.Add(IncludeCompositedDescendants);
      }
    }

    const_cast<RenderLayer *>(this)->UpdateLayerListsIfNeeded();

    krys_debug_assert(IsStackingContext() || !PositiveZOrderLayers().size());

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(const_cast<RenderLayer &>(*this));
#endif

    auto computeLayersUnion = [this, &unionBounds, descendantFlags](const RenderLayer &childLayer)
    {
      // FIXME: Skip composited descendants unless IncludeCompositedDescendants, once there's compositing.
      LayoutRect childBounds =
        childLayer.CalculateLayerBounds(this, childLayer.OffsetFromAncestor(this), descendantFlags);
      // Ignore child layer (and behave as if we had overflow: hidden) when it is positioned off the parent
      // layer so much that we hit the max LayoutUnit value.
      (void)unionBounds.CheckedUnite(childBounds);
    };

    for (auto *childLayer : NegativeZOrderLayers())
    {
      computeLayersUnion(*childLayer);
    }

    for (auto *childLayer : PositiveZOrderLayers())
    {
      computeLayersUnion(*childLayer);
    }

    for (auto *childLayer : NormalFlowLayers())
    {
      computeLayersUnion(*childLayer);
    }

    // FIXME: The filter outsets (IncludeFilterOutsets, IncludePaintedFilterOutsets), once there are filters.

    if (flags.Contains(IncludeSelfTransform) && PaintsWithTransform())
    {
      TransformationMatrix *affineTrans = Transform();
      boundingBoxRect = affineTrans->MapRect(boundingBoxRect);
      unionBounds = affineTrans->MapRect(unionBounds);
    }
    unionBounds.Move(offsetFromRoot);
    return unionBounds;
  }

#pragma endregion
}
