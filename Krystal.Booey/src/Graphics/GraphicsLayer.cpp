#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFactory.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFilterAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerKeyframeValueList.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <format>

namespace krys::boo
{
  utf8_stringview AnimatedPropertyIDAsString(AnimatedProperty property) noexcept
  {
    switch (property)
    {
      case AnimatedProperty::Translate:
      case AnimatedProperty::Scale:
      case AnimatedProperty::Rotate:
      case AnimatedProperty::Transform:            return u8"transform";
      case AnimatedProperty::Opacity:              return u8"opacity";
      case AnimatedProperty::BackgroundColor:      return u8"background-color";
      case AnimatedProperty::Filter:               return u8"filter";
      case AnimatedProperty::WebkitBackdropFilter: return u8"backdrop-filter";
      case AnimatedProperty::Invalid:              return u8"invalid";
    }
    krys_debug_assert(false);
    return u8"";
  }

  namespace
  {
    using RepaintMap = HashMap<const GraphicsLayer *, List<FloatRect>>;
    RepaintMap &RepaintRectMap() noexcept
    {
      static NeverDestroyed<RepaintMap> map;
      return map;
    }

    // Singleton client used for layers on which clearClient has been called.
    class EmptyGraphicsLayerClient final : public GraphicsLayerClient
    {
    public:
      KRYS_NODISCARD static EmptyGraphicsLayerClient &Singleton() noexcept
      {
        static NeverDestroyed<EmptyGraphicsLayerClient> client;
        return client;
      }
    };
  }

  bool GraphicsLayer::SupportsLayerType(Type type) noexcept
  {
    switch (type)
    {
      case Type::Normal:
      case Type::Structural:
      case Type::PageTiledBacking:
      case Type::ScrollContainer:
      case Type::ScrolledContents:
      case Type::TiledBacking:     return true;
      case Type::Shape:            return false;
    }
    krys_debug_assert(false);
    return false;
  }

  bool GraphicsLayer::SupportsContentsTiling() noexcept
  {
    // FIXME: Enable the feature on different ports.
    return false;
  }

  Ref<GraphicsLayer> GraphicsLayer::Create(GraphicsLayerFactory *factory, GraphicsLayerClient &client,
                                           Type type) noexcept
  {
    krys_assert_msg(factory, "Graphics layers come from the host's factory.");
    return factory->CreateGraphicsLayer(type, client);
  }

  GraphicsLayer::GraphicsLayer(Type type, GraphicsLayerClient &layerClient) noexcept
      : _client(&layerClient), _type(type), _beingDestroyed(false), _contentsOpaque(false),
        _preserves3D(type == Type::Structural), _backfaceVisibility(true), _masksToBounds(false),
        _drawsContent(false), _contentsVisible(true), _contentsRectClipsDescendants(false),
        _acceleratesDrawing(false), _usesDisplayListDrawing(false), _allowsTiling(true),
        _appliesPageScale(false), _appliesDeviceScale(true), _showDebugBorder(false),
        _showRepaintCounter(false), _showFrameProcessBorders(false), _isMaskLayer(false),
        _isBackdropRoot(false), _isTrackingDisplayListReplay(false), _userInteractionEnabled(true),
        _canDetachBackingStore(true), _shouldPaintUsingCompositeCopy(false)
  {
#if KRYS_DEBUG
    Client().VerifyNotPainting();
#endif
  }

  GraphicsLayer::~GraphicsLayer() noexcept
  {
    ResetTrackedRepaints();
    krys_debug_assert(!_parent); // willBeDestroyed should have been called already.
  }

  void GraphicsLayer::UnparentAndClear(RefPtr<GraphicsLayer> &layer) noexcept
  {
    if (layer)
    {
      layer->RemoveFromParent();
      layer->ClearClient();
      layer = nullptr;
    }
  }

  void GraphicsLayer::Clear(RefPtr<GraphicsLayer> &layer) noexcept
  {
    if (layer)
    {
      layer->ClearClient();
      layer = nullptr;
    }
  }

  void GraphicsLayer::WillBeDestroyed() noexcept
  {
    _beingDestroyed = true;
#if KRYS_DEBUG
    Client().VerifyNotPainting();
#endif
    if (_replicaLayer)
    {
      _replicaLayer->SetReplicatedLayer(nullptr);
    }

    if (_replicatedLayer)
    {
      _replicatedLayer->SetReplicatedByLayer(nullptr);
    }

    if (_maskLayer)
    {
      _maskLayer->SetParent(nullptr);
      _maskLayer->SetIsMaskLayer(false);
    }

    RemoveAllChildren();
    RemoveFromParentInternal();
  }

  void GraphicsLayer::ClearClient() noexcept
  {
    _client = &EmptyGraphicsLayerClient::Singleton();
  }

  utf8_string GraphicsLayer::DebugName() const noexcept
  {
    return Name();
  }

  void GraphicsLayer::SetClient(GraphicsLayerClient &client) noexcept
  {
    _client = &client;
  }

  void GraphicsLayer::SetParent(GraphicsLayer *layer) noexcept
  {
    krys_debug_assert(!layer || !layer->HasAncestor(this));
    _parent = layer;
  }

  bool GraphicsLayer::HasAncestor(GraphicsLayer *ancestor) const noexcept
  {
    for (GraphicsLayer *curr = Parent(); curr; curr = curr->Parent())
    {
      if (curr == ancestor)
      {
        return true;
      }
    }

    return false;
  }

  bool GraphicsLayer::SetChildren(List<Ref<GraphicsLayer>> &&newChildren) noexcept
  {
    // If the contents of the arrays are the same, nothing to do.
    auto sameChildren = [&]
    {
      if (newChildren.size() != _children.size())
      {
        return false;
      }
      for (size_t i = 0; i < newChildren.size(); ++i)
      {
        if (newChildren[i].get() != _children[i].get())
        {
          return false;
        }
      }
      return true;
    };
    if (sameChildren())
    {
      return false;
    }

    RemoveAllChildren();

    size_t listSize = newChildren.size();
    for (size_t i = 0; i < listSize; ++i)
    {
      AddChild(krys::move(newChildren[i]));
    }

    return true;
  }

  void GraphicsLayer::AddChild(Ref<GraphicsLayer> &&childLayer) noexcept
  {
    krys_debug_assert(childLayer.get() != this);

    childLayer->RemoveFromParent();
    childLayer->SetParent(this);
    _children.push_back(krys::move(childLayer));
  }

  void GraphicsLayer::AddChildAtIndex(Ref<GraphicsLayer> &&childLayer, int32 index) noexcept
  {
    krys_debug_assert(childLayer.get() != this);

    childLayer->RemoveFromParent();
    childLayer->SetParent(this);
    _children.insert(_children.begin() + index, krys::move(childLayer));
  }

  void GraphicsLayer::AddChildBelow(Ref<GraphicsLayer> &&childLayer, GraphicsLayer *sibling) noexcept
  {
    krys_debug_assert(childLayer.get() != this);
    childLayer->RemoveFromParent();

    childLayer->SetParent(this);

    for (size_t i = 0; i < _children.size(); i++)
    {
      if (sibling == _children[i].get())
      {
        _children.insert(_children.begin() + static_cast<ptrdiff_t>(i), krys::move(childLayer));
        return;
      }
    }

    _children.push_back(krys::move(childLayer));
  }

  void GraphicsLayer::AddChildAbove(Ref<GraphicsLayer> &&childLayer, GraphicsLayer *sibling) noexcept
  {
    childLayer->RemoveFromParent();
    krys_debug_assert(childLayer.get() != this);

    childLayer->SetParent(this);

    for (size_t i = 0; i < _children.size(); i++)
    {
      if (sibling == _children[i].get())
      {
        _children.insert(_children.begin() + static_cast<ptrdiff_t>(i + 1), krys::move(childLayer));
        return;
      }
    }

    _children.push_back(krys::move(childLayer));
  }

  bool GraphicsLayer::ReplaceChild(GraphicsLayer *oldChild, Ref<GraphicsLayer> &&newChild) noexcept
  {
    krys_debug_assert(!newChild->Parent());

    GraphicsLayer *rawNewChild = newChild.get();

    bool found = false;
    for (size_t i = 0; i < _children.size(); i++)
    {
      if (oldChild == _children[i].get())
      {
        _children[i] = krys::move(newChild);
        found = true;
        break;
      }
    }
    if (found)
    {
      oldChild->SetParent(nullptr);

      rawNewChild->RemoveFromParent();
      rawNewChild->SetParent(this);
      return true;
    }
    return false;
  }

  void GraphicsLayer::RemoveAllChildren() noexcept
  {
    if (_children.empty())
    {
      return;
    }

    WillModifyChildren();

    for (auto &child : _children)
    {
      child->SetParent(nullptr);
    }

    _children.clear();
  }

  void GraphicsLayer::RemoveFromParentInternal() noexcept
  {
    if (_parent)
    {
      GraphicsLayer *parent = _parent;
      SetParent(nullptr);
      auto &siblings = parent->_children;
      for (auto it = siblings.begin(); it != siblings.end(); ++it)
      {
        if (it->get() == this)
        {
          siblings.erase(it);
          break;
        }
      }
      // |this| may be destroyed here.
    }
  }

  bool GraphicsLayer::NeedsBackdrop() const noexcept
  {
    return !_backdropFilters.IsEmpty();
  }

  void GraphicsLayer::SetPreserves3D(bool b) noexcept
  {
    krys_debug_assert(_type != Type::Structural || b);
    _preserves3D = b;
  }

  void GraphicsLayer::SetMasksToBounds(bool b) noexcept
  {
    krys_debug_assert(_type != Type::Structural);
    _masksToBounds = b;
  }

  void GraphicsLayer::SetDrawsContent(bool b) noexcept
  {
    krys_debug_assert(_type != Type::Structural);
    _drawsContent = b;
  }

  const TransformationMatrix &GraphicsLayer::Transform() const noexcept
  {
    return _transform ? *_transform : TransformationMatrix::Identity;
  }

  void GraphicsLayer::SetTransform(const TransformationMatrix &matrix) noexcept
  {
    if (_transform)
    {
      *_transform = matrix;
    }
    else
    {
      _transform = CreateUnique<TransformationMatrix>(matrix);
    }
  }

  const TransformationMatrix &GraphicsLayer::ChildrenTransform() const noexcept
  {
    return _childrenTransform ? *_childrenTransform : TransformationMatrix::Identity;
  }

  void GraphicsLayer::SetChildrenTransform(const TransformationMatrix &matrix) noexcept
  {
    if (_childrenTransform)
    {
      *_childrenTransform = matrix;
    }
    else
    {
      _childrenTransform = CreateUnique<TransformationMatrix>(matrix);
    }
  }

  bool GraphicsLayer::SetFilters(const FilterOperations &filters) noexcept
  {
    krys_debug_assert(_type != Type::Structural);
    _filters = filters;
    return true;
  }

  void GraphicsLayer::SetOpacity(float opacity) noexcept
  {
    krys_debug_assert(_type != Type::Structural);
    _opacity = opacity;
  }

  void GraphicsLayer::RemoveFromParent() noexcept
  {
    if (_parent)
    {
      _parent->WillModifyChildren();
    }

    // removeFromParentInternal is nonvirtual, for use in willBeDestroyed, which is called from destructors.
    RemoveFromParentInternal();
  }

  void GraphicsLayer::SetMaskLayer(RefPtr<GraphicsLayer> &&layer) noexcept
  {
    if (layer == _maskLayer)
    {
      return;
    }

    if (layer)
    {
      layer->RemoveFromParent();
      layer->SetParent(this);
      layer->SetIsMaskLayer(true);
    }
    else if (_maskLayer)
    {
      _maskLayer->SetParent(nullptr);
      _maskLayer->SetIsMaskLayer(false);
    }

    _maskLayer = krys::move(layer);
  }

  Path GraphicsLayer::ShapeLayerPath() const noexcept
  {
    return Path();
  }

  void GraphicsLayer::SetShapeLayerPath(const Path &) noexcept
  {
  }

  WindRule GraphicsLayer::ShapeLayerWindRule() const noexcept
  {
    return WindRule::NonZero;
  }

  void GraphicsLayer::SetShapeLayerWindRule(WindRule) noexcept
  {
  }

  void GraphicsLayer::NoteDeviceOrPageScaleFactorChangedIncludingDescendants() noexcept
  {
    DeviceOrPageScaleFactorChanged();

    if (_maskLayer)
    {
      _maskLayer->DeviceOrPageScaleFactorChanged();
    }

    if (_replicaLayer)
    {
      _replicaLayer->NoteDeviceOrPageScaleFactorChangedIncludingDescendants();
    }

    for (auto &layer : Children())
    {
      layer->NoteDeviceOrPageScaleFactorChangedIncludingDescendants();
    }
  }

  void GraphicsLayer::SetIsInWindow(bool) noexcept
  {
    // FIXME: Tiled backings (TiledBacking), which only Core Animation's layers have.
  }

  void GraphicsLayer::SetReplicatedByLayer(RefPtr<GraphicsLayer> &&layer) noexcept
  {
    if (_replicaLayer == layer)
    {
      return;
    }

    if (_replicaLayer)
    {
      _replicaLayer->SetReplicatedLayer(nullptr);
    }

    if (layer)
    {
      layer->SetReplicatedLayer(this);
    }

    _replicaLayer = krys::move(layer);
  }

  void GraphicsLayer::SetOffsetFromRenderer(const FloatSize &offset,
                                            ShouldSetNeedsDisplay shouldSetNeedsDisplay) noexcept
  {
    if (offset == _offsetFromRenderer)
    {
      return;
    }

    _offsetFromRenderer = offset;

    // If the compositing layer offset changes, we need to repaint.
    if (shouldSetNeedsDisplay == ShouldSetNeedsDisplay::Set)
    {
      SetNeedsDisplay();
    }
  }

  void GraphicsLayer::SetScrollOffset(const ScrollOffset &offset,
                                      ShouldSetNeedsDisplay shouldSetNeedsDisplay) noexcept
  {
    if (offset == _scrollOffset)
    {
      return;
    }

    _scrollOffset = offset;

    // If the compositing layer offset changes, we need to repaint.
    if (shouldSetNeedsDisplay == ShouldSetNeedsDisplay::Set)
    {
      SetNeedsDisplay();
    }
  }

  void GraphicsLayer::SetSize(const FloatSize &size) noexcept
  {
    if (size == _size)
    {
      return;
    }

    _size = size;

    if (ShouldRepaintOnSizeChange())
    {
      SetNeedsDisplay();
    }
  }

  void GraphicsLayer::SetBackgroundColor(const Color &color) noexcept
  {
    krys_debug_assert(_type != Type::Structural);
    _backgroundColor = color;
  }

  void GraphicsLayer::SetPaintingPhase(EnumSet<GraphicsLayerPaintingPhase> phase) noexcept
  {
    if (phase == _paintingPhase)
    {
      return;
    }

    SetNeedsDisplay();
    _paintingPhase = phase;
  }

  void GraphicsLayer::PaintGraphicsLayerContents(
    GraphicsContext &context, const FloatRect &clip,
    EnumSet<GraphicsLayerPaintBehavior> layerPaintBehavior) const noexcept
  {
    auto offset = OffsetFromRenderer() - ToFloatSize(FloatPoint(GetScrollOffset()));
    auto clipRect = clip;

    if (!offset.IsZero())
    {
      context.Translate(-offset);
      clipRect.Move(offset);
    }

    Client().PaintContents(*this, context, clipRect, layerPaintBehavior);
  }

  FloatRect GraphicsLayer::AdjustCoverageRectForMovement(const FloatRect &coverageRect,
                                                         const FloatRect &previousVisibleRect,
                                                         const FloatRect &currentVisibleRect) noexcept
  {
    // If the old visible rect is empty, we have no information about how the visible area is changing (maybe
    // the layer was just created), so don't attempt to expand. Also don't attempt to expand if the rects
    // don't overlap.
    if (previousVisibleRect.IsEmpty() || !currentVisibleRect.Intersects(previousVisibleRect))
    {
      return UnionRect(coverageRect, currentVisibleRect);
    }

    const float paddingMultiplier = 2;

    float leftEdgeDelta = paddingMultiplier * (currentVisibleRect.X() - previousVisibleRect.X());
    float rightEdgeDelta = paddingMultiplier * (currentVisibleRect.MaxX() - previousVisibleRect.MaxX());

    float topEdgeDelta = paddingMultiplier * (currentVisibleRect.Y() - previousVisibleRect.Y());
    float bottomEdgeDelta = paddingMultiplier * (currentVisibleRect.MaxY() - previousVisibleRect.MaxY());

    FloatRect expandedRect = currentVisibleRect;

    // More exposed on left side.
    if (leftEdgeDelta < 0)
    {
      float newLeft = expandedRect.X() + leftEdgeDelta;
      // Pad to the left, but don't reduce padding that's already in the backing store (since we're still
      // exposing to the left).
      if (newLeft < previousVisibleRect.X())
      {
        expandedRect.ShiftXEdgeTo(newLeft);
      }
      else
      {
        expandedRect.ShiftXEdgeTo(previousVisibleRect.X());
      }
    }

    // More exposed on right.
    if (rightEdgeDelta > 0)
    {
      float newRight = expandedRect.MaxX() + rightEdgeDelta;
      // Pad to the right, but don't reduce padding that's already in the backing store (since we're still
      // exposing to the right).
      if (newRight > previousVisibleRect.MaxX())
      {
        expandedRect.SetWidth(newRight - expandedRect.X());
      }
      else
      {
        expandedRect.SetWidth(previousVisibleRect.MaxX() - expandedRect.X());
      }
    }

    // More exposed at top.
    if (topEdgeDelta < 0)
    {
      float newTop = expandedRect.Y() + topEdgeDelta;
      if (newTop < previousVisibleRect.Y())
      {
        expandedRect.ShiftYEdgeTo(newTop);
      }
      else
      {
        expandedRect.ShiftYEdgeTo(previousVisibleRect.Y());
      }
    }

    // More exposed on bottom.
    if (bottomEdgeDelta > 0)
    {
      float newBottom = expandedRect.MaxY() + bottomEdgeDelta;
      if (newBottom > previousVisibleRect.MaxY())
      {
        expandedRect.SetHeight(newBottom - expandedRect.Y());
      }
      else
      {
        expandedRect.SetHeight(previousVisibleRect.MaxY() - expandedRect.Y());
      }
    }

    return UnionRect(coverageRect, expandedRect);
  }

  utf8_string GraphicsLayer::AnimationNameForTransition(AnimatedProperty property) noexcept
  {
    // | is not a valid identifier character in CSS, so this can never conflict with a keyframe identifier.
    auto name = std::format("-|transition{}-", static_cast<int32>(property));
    return utf8_string(name.begin(), name.end());
  }

  void GraphicsLayer::SuspendAnimations(MonotonicTime::duration) noexcept
  {
  }

  void GraphicsLayer::ResumeAnimations() noexcept
  {
  }

  void GraphicsLayer::GetDebugBorderInfo(Color &color, float &width) const noexcept
  {
    width = 2;

    if (NeedsBackdrop())
    {
      color = SRGBA<uint8> {255, 0, 255, 128}; // has backdrop: magenta
      width = 12;
      return;
    }

    if (DrawsContent())
    {
      color = SRGBA<uint8> {0, 128, 32, 128}; // normal layer: green
      return;
    }

    if (UsesContentsLayer())
    {
      color = SRGBA<uint8> {0, 64, 128, 150}; // non-painting layer with contents: blue
      width = 8;
      return;
    }

    if (MasksToBounds())
    {
      color = SRGBA<uint8> {128, 255, 255, 48}; // masking layer: pale blue
      width = 16;
      return;
    }

    // FIXME: Frame process borders, once frames can be in other processes.

    color = SRGBA<uint8> {255, 255, 0, 192}; // container: yellow
  }

  void GraphicsLayer::UpdateDebugIndicators() noexcept
  {
    if (!IsShowingDebugBorder() && !IsShowingFrameProcessBorders())
    {
      return;
    }

    Color borderColor;
    float width = 0;
    GetDebugBorderInfo(borderColor, width);
    SetDebugBorder(borderColor, width);
  }

  void GraphicsLayer::SetZPosition(float position) noexcept
  {
    _zPosition = position;
  }

  namespace
  {
    KRYS_NODISCARD inline const FilterOperations &
      FilterOperationsAt(const GraphicsLayerKeyframeValueList &valueList, size_t index) noexcept
    {
      return static_cast<const GraphicsLayerFilterAnimationValue &>(valueList.At(index)).Value();
    }
  }

  int32 GraphicsLayer::ValidateFilterOperations(const GraphicsLayerKeyframeValueList &valueList) noexcept
  {
    krys_debug_assert(valueList.Property() == AnimatedProperty::Filter
                      || valueList.Property() == AnimatedProperty::WebkitBackdropFilter);

    if (valueList.Size() < 2)
    {
      return -1;
    }

    // Empty filters match anything, so find the first non-empty entry as the reference
    size_t firstIndex = 0;
    for (; firstIndex < valueList.Size(); ++firstIndex)
    {
      if (!FilterOperationsAt(valueList, firstIndex).IsEmpty())
      {
        break;
      }
    }

    if (firstIndex >= valueList.Size())
    {
      return -1;
    }

    const FilterOperations &firstVal = FilterOperationsAt(valueList, firstIndex);

    for (size_t i = firstIndex + 1; i < valueList.Size(); ++i)
    {
      const FilterOperations &val = FilterOperationsAt(valueList, i);

      // An empty filter list matches anything.
      if (val.IsEmpty())
      {
        continue;
      }

      if (!firstVal.OperationsMatch(val))
      {
        return -1;
      }
    }

    return static_cast<int32>(firstIndex);
  }

  double GraphicsLayer::BackingStoreMemoryEstimate() const noexcept
  {
    if (!DrawsContent())
    {
      return 0;
    }

    // Effects of page and device scale are ignored; subclasses should override to take these into account.
    return static_cast<double>(4 * Size().Width()) * Size().Height();
  }

  void GraphicsLayer::ResetTrackedRepaints() noexcept
  {
    RepaintRectMap().erase(this);
  }

  void GraphicsLayer::AddRepaintRect(const FloatRect &repaintRect) noexcept
  {
    if (!Client().IsTrackingRepaints())
    {
      return;
    }

    FloatRect largestRepaintRect(FloatPoint(), _size);
    largestRepaintRect.Intersect(repaintRect);

    RepaintRectMap()[this].push_back(largestRepaintRect);
  }

  void GraphicsLayer::Traverse(GraphicsLayer &layer,
                               const Func<void(GraphicsLayer &)> &traversalFunc) noexcept
  {
    traversalFunc(layer);

    for (auto &childLayer : layer.Children())
    {
      Traverse(*childLayer, traversalFunc);
    }

    if (auto *replicaLayer = layer.ReplicaLayer())
    {
      Traverse(*replicaLayer, traversalFunc);
    }

    if (auto *maskLayer = layer.MaskLayer())
    {
      Traverse(*maskLayer, traversalFunc);
    }
  }

  void GraphicsLayer::SetTileCoverage(TileCoverage) noexcept
  {
    // FIXME: Tiled backings (TiledBacking), which only Core Animation's layers have.
  }

  void GraphicsLayer::DumpLayer(TextStream &ts, EnumSet<LayerTreeAsTextOptions> options) const noexcept
  {
    ts << Indent << '(' << "GraphicsLayer";

    if (options.Contains(LayerTreeAsTextOptions::Debug))
    {
      ts << ' ' << static_cast<const void *>(this);
      ts << " \"" << _name << '"';
    }

    ts << '\n';
    DumpProperties(ts, options);
    ts << Indent << ")\n";
  }

  namespace
  {
    void DumpChildren(TextStream &ts, const List<Ref<GraphicsLayer>> &children, uint32 &totalChildCount,
                      EnumSet<LayerTreeAsTextOptions> options) noexcept
    {
      totalChildCount += static_cast<uint32>(children.size());
      for (const auto &child : children)
      {
        if (options.Contains(LayerTreeAsTextOptions::Debug)
            || !child->Client().ShouldSkipLayerInDump(child.get(), options))
        {
          TextStream::IndentScope indentScope(ts);
          child->DumpLayer(ts, options);
          continue;
        }

        totalChildCount--;
        DumpChildren(ts, child->Children(), totalChildCount, options);
      }
    }

    void DumpMatrix(TextStream &ts, const TransformationMatrix &matrix) noexcept
    {
      ts << '[' << matrix.M11() << ' ' << matrix.M12() << ' ' << matrix.M13() << ' ' << matrix.M14() << "] ";
      ts << '[' << matrix.M21() << ' ' << matrix.M22() << ' ' << matrix.M23() << ' ' << matrix.M24() << "] ";
      ts << '[' << matrix.M31() << ' ' << matrix.M32() << ' ' << matrix.M33() << ' ' << matrix.M34() << "] ";
      ts << '[' << matrix.M41() << ' ' << matrix.M42() << ' ' << matrix.M43() << ' ' << matrix.M44()
         << "])\n";
    }
  }

  void GraphicsLayer::DumpProperties(TextStream &ts, EnumSet<LayerTreeAsTextOptions> options) const noexcept
  {
    TextStream::IndentScope indentScope(ts);
    if (!_offsetFromRenderer.IsZero())
    {
      ts << Indent << "(offsetFromRenderer " << _offsetFromRenderer << ")\n";
    }

    if (_scrollOffset != ScrollOffset())
    {
      ts << Indent << "(scrollOffset " << _scrollOffset << ")\n";
    }

    if (_position != FloatPoint())
    {
      ts << Indent << "(position " << _position.X() << ' ' << _position.Y() << ")\n";
    }

    if (_approximatePosition)
    {
      ts << Indent << "(approximate position " << _approximatePosition.value().X() << ' '
         << _approximatePosition.value().Y() << ")\n";
    }

    if (_boundsOrigin != FloatPoint())
    {
      ts << Indent << "(bounds origin " << _boundsOrigin.X() << ' ' << _boundsOrigin.Y() << ")\n";
    }

    if (Client().ShouldDumpPropertyForLayer(this, u8"anchorPoint", options))
    {
      ts << Indent << "(anchor " << _anchorPoint.X() << ' ' << _anchorPoint.Y();
      if (_anchorPoint.Z())
      {
        ts << ' ' << _anchorPoint.Z();
      }
      ts << ")\n";
    }

    if (_size != FloatSize())
    {
      ts << Indent << "(bounds " << _size.Width() << ' ' << _size.Height() << ")\n";
    }

    if (_opacity != 1)
    {
      ts << Indent << "(opacity " << _opacity << ")\n";
    }

    if (_blendMode != BlendMode::Normal)
    {
      ts << Indent << "(blendMode " << CompositeOperatorName(CompositeOperator::SourceOver, _blendMode)
         << ")\n";
    }

    if (_contentsOpaque)
    {
      ts << Indent << "(contentsOpaque " << _contentsOpaque << ")\n";
    }

    if (_masksToBounds && options.Contains(LayerTreeAsTextOptions::IncludeClipping))
    {
      ts << Indent << "(clips " << _masksToBounds << ")\n";
    }

    if (_preserves3D)
    {
      ts << Indent << "(preserves3D " << _preserves3D << ")\n";
    }

    if (_drawsContent && Client().ShouldDumpPropertyForLayer(this, u8"drawsContent", options))
    {
      ts << Indent << "(drawsContent " << _drawsContent << ")\n";
    }

    if (!_contentsVisible)
    {
      ts << Indent << "(contentsVisible " << _contentsVisible << ")\n";
    }

    if (!_backfaceVisibility)
    {
      ts << Indent << "(backfaceVisibility " << (_backfaceVisibility ? "visible" : "hidden") << ")\n";
    }

    if (_isBackdropRoot)
    {
      ts << Indent << "(backdropRoot " << _isBackdropRoot << ")\n";
    }

    if (options.Contains(LayerTreeAsTextOptions::Debug))
    {
      ts << Indent << "(primary-layer-id " << PrimaryLayerID() << ")\n";
    }

    if (_backgroundColor.IsValid() && Client().ShouldDumpPropertyForLayer(this, u8"backgroundColor", options))
    {
      ts << Indent << "(backgroundColor " << SerialisationForRenderTreeAsText(_backgroundColor) << ")\n";
    }

    if (options.Contains(LayerTreeAsTextOptions::IncludeAcceleratesDrawing) && _acceleratesDrawing)
    {
      ts << Indent << "(acceleratesDrawing " << _acceleratesDrawing << ")\n";
    }

    if (options.Contains(LayerTreeAsTextOptions::IncludeBackingStoreAttached))
    {
      ts << Indent << "(backingStoreAttached " << BackingStoreAttachedForTesting() << ")\n";
    }

    if (_transform && !_transform->IsIdentity())
    {
      ts << Indent << "(transform ";
      DumpMatrix(ts, *_transform);
    }

    // Avoid dumping the sublayer transform on the root layer, because it's used for geometry flipping, whose
    // behavior differs between platforms.
    if (Parent() && _childrenTransform && !_childrenTransform->IsIdentity())
    {
      ts << Indent << "(childrenTransform ";
      DumpMatrix(ts, *_childrenTransform);
    }

    if (_maskLayer)
    {
      ts << Indent << "(mask layer";
      if (options.Contains(LayerTreeAsTextOptions::Debug))
      {
        ts << ' ' << static_cast<const void *>(_maskLayer.get());
      }
      ts << ")\n";

      TextStream::IndentScope maskIndentScope(ts);
      _maskLayer->DumpLayer(ts, options);
    }

    if (_replicaLayer)
    {
      ts << Indent << "(replica layer";
      if (options.Contains(LayerTreeAsTextOptions::Debug))
      {
        ts << ' ' << static_cast<const void *>(_replicaLayer.get());
      }
      ts << ")\n";

      TextStream::IndentScope replicaIndentScope(ts);
      _replicaLayer->DumpLayer(ts, options);
    }

    if (_replicatedLayer)
    {
      ts << Indent << "(replicated layer";
      if (options.Contains(LayerTreeAsTextOptions::Debug))
      {
        ts << ' ' << static_cast<const void *>(_replicatedLayer);
      }
      ts << ")\n";
    }

    if (options.Contains(LayerTreeAsTextOptions::IncludeRepaintRects))
    {
      auto it = RepaintRectMap().find(this);
      if (it != RepaintRectMap().end() && !it->second.empty()
          && Client().ShouldDumpPropertyForLayer(this, u8"repaintRects", options))
      {
        ts << Indent << "(repaint rects\n";
        for (const auto &rect : it->second)
        {
          if (rect.IsEmpty())
          {
            continue;
          }

          TextStream::IndentScope rectIndentScope(ts);
          ts << Indent << "(rect ";
          ts << rect.X() << ' ';
          ts << rect.Y() << ' ';
          ts << rect.Width() << ' ';
          ts << rect.Height();
          ts << ")\n";
        }
        ts << Indent << ")\n";
      }
    }

    if (options.Contains(LayerTreeAsTextOptions::IncludePaintingPhases) && !PaintingPhase().IsEmpty())
    {
      ts << Indent << "(paintingPhases " << PaintingPhase() << ")\n";
    }

    DumpAdditionalProperties(ts, options);

    if (!_children.empty())
    {
      TextStream childrenStream;

      childrenStream.IncreaseIndent(ts.GetIndent());
      uint32 totalChildCount = 0;
      DumpChildren(childrenStream, _children, totalChildCount, options);

      if (totalChildCount)
      {
        ts << Indent << "(children " << totalChildCount << '\n';
        ts << childrenStream.Release();
        ts << Indent << ")\n";
      }
    }

    Client().DumpProperties(this, ts, options);
  }

  TextStream &operator<<(TextStream &ts, const List<PlatformLayerIdentifier> &layers) noexcept
  {
    for (size_t i = 0; i < layers.size(); ++i)
    {
      if (i)
      {
        ts << ' ';
      }
      ts << layers[i];
    }

    return ts;
  }

  TextStream &operator<<(TextStream &ts, GraphicsLayerPaintingPhase phase) noexcept
  {
    switch (phase)
    {
      case GraphicsLayerPaintingPhase::Background:        ts << "background"; break;
      case GraphicsLayerPaintingPhase::Foreground:        ts << "foreground"; break;
      case GraphicsLayerPaintingPhase::Mask:              ts << "mask"; break;
      case GraphicsLayerPaintingPhase::ClipPath:          ts << "clip-path"; break;
      case GraphicsLayerPaintingPhase::OverflowContents:  ts << "overflow-contents"; break;
      case GraphicsLayerPaintingPhase::CompositedScroll:  ts << "composited-scroll"; break;
      case GraphicsLayerPaintingPhase::ChildClippingMask: ts << "child-clipping-mask"; break;
    }

    return ts;
  }

  TextStream &operator<<(TextStream &ts, const GraphicsLayer::CustomAppearance &customAppearance) noexcept
  {
    switch (customAppearance)
    {
      case GraphicsLayer::CustomAppearance::None:            ts << "none"; break;
      case GraphicsLayer::CustomAppearance::ScrollingShadow: ts << "scrolling-shadow"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, PlatformLayerIdentifier identifier) noexcept
  {
    return ts << identifier.Value;
  }

  utf8_string GraphicsLayer::LayerTreeAsText(EnumSet<LayerTreeAsTextOptions> options,
                                             uint32 baseIndent) const noexcept
  {
    TextStream ts(TextStream::LineMode::MultipleLine, {TextStream::Formatting::SVGStyleRect});
    ts.SetIndent(static_cast<int32>(baseIndent));

    DumpLayer(ts, options);
    return ts.Release();
  }
}
