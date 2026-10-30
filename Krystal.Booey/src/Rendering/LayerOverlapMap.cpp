#include "Krystal.Booey/Rendering/LayerOverlapMap.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  namespace
  {
    struct RectList
    {
      List<LayoutRect> Rects;
      LayoutRect BoundingRect;

      void Append(const LayoutRect &rect) noexcept
      {
        Rects.push_back(rect);
        BoundingRect.Unite(rect);
      }

      void Append(const RectList &rectList) noexcept
      {
        Rects.insert(Rects.end(), rectList.Rects.begin(), rectList.Rects.end());
        BoundingRect.Unite(rectList.BoundingRect);
      }

      KRYS_NODISCARD bool Intersects(const LayoutRect &rect) const noexcept
      {
        if (Rects.empty() || !rect.Intersects(BoundingRect))
        {
          return false;
        }

        for (const auto &currentRect : Rects)
        {
          if (currentRect.Intersects(rect))
          {
            return true;
          }
        }
        return false;
      }
    };

    TextStream &operator<<(TextStream &ts, const RectList &rectList) noexcept
    {
      ts << "bounds " << rectList.BoundingRect << " (" << rectList.Rects << " rects)";
      return ts;
    }
  }

  /// @brief Used to store overlap rects in a way that takes overflow into account. It stores a tree whose
  /// nodes are layers with composited scrolling. The tree is built lazily as layers are added whose
  /// containing block chains contain composited scrollers. The tree always starts at the root layer. Checking
  /// for overlap involves finding the node for the clipping layer enclosing the given layer (or the root),
  /// and comparing against the bounds of earlier siblings.
  class OverlapMapContainer
  {
  private:
    struct ClippingScope
    {
      const RenderLayer *Layer;
      LayoutRect Bounds; // Bounds of the composited clip.
      List<ClippingScope> Children;
      RectList Rects;

      explicit ClippingScope(const RenderLayer &inLayer) noexcept : Layer(&inLayer)
      {
      }

      explicit ClippingScope(const LayerOverlapMap::LayerAndBounds &layerAndBounds) noexcept
          : Layer(layerAndBounds.Layer), Bounds(layerAndBounds.Bounds)
      {
      }

      KRYS_NODISCARD ClippingScope *ChildWithLayer(const RenderLayer &layer) const noexcept
      {
        for (const auto &child : Children)
        {
          if (child.Layer == &layer)
          {
            return const_cast<ClippingScope *>(&child);
          }
        }
        return nullptr;
      }

      ClippingScope *
        AddChildWithLayerAndBounds(const LayerOverlapMap::LayerAndBounds &layerAndBounds) noexcept
      {
        Children.push_back(ClippingScope {layerAndBounds});
        return &Children.back();
      }

      ClippingScope *AddChild(const ClippingScope &child) noexcept
      {
        krys_debug_assert(Layer != child.Layer);
        Children.push_back(child);
        return &Children.back();
      }

      void AppendRect(const LayoutRect &bounds) noexcept
      {
        Rects.Append(bounds);
      }
    };

    ClippingScope _rootScope;
    const RenderLayer &_scopeLayer;

  public:
    OverlapMapContainer(const RenderLayer &rootLayer, const RenderLayer &scopeLayer) noexcept
        : _rootScope(rootLayer), _scopeLayer(scopeLayer)
    {
    }

    OverlapMapContainer(const OverlapMapContainer &) noexcept = default;

    // Layers are added in z-order, lazily creating clipping scopes as necessary.
    void Add(const RenderLayer &, const LayoutRect &bounds,
             const LayerOverlapMap::LayerAndBoundsVector &enclosingClippingLayers) noexcept
    {
      auto *layerScope = EnsureClippingScopeForLayers(enclosingClippingLayers);
      layerScope->AppendRect(bounds);
    }

    KRYS_NODISCARD bool
      OverlapsLayers(const RenderLayer &, const LayoutRect &bounds,
                     const LayerOverlapMap::LayerAndBoundsVector &enclosingClippingLayers) const noexcept
    {
      if (_rootScope.Rects.Intersects(bounds))
      {
        return true;
      }

      if (_rootScope.Children.empty())
      {
        return false;
      }

      // Find the ClippingScope for which this layer is a child.
      auto *clippingScope = FindClippingScopeForLayers(enclosingClippingLayers);
      if (!clippingScope)
      {
        return false;
      }

      if (clippingScope->Rects.Intersects(bounds))
      {
        return true;
      }

      // FIXME: In some cases do we have to walk up the ancestor clipping scope chain?
      return false;
    }

    void Append(UniquePtr<OverlapMapContainer> &&otherContainer) noexcept
    {
      MergeClippingScopesRecursive(otherContainer->_rootScope, _rootScope);
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _rootScope.Rects.Rects.empty() && _rootScope.Children.empty();
    }

    KRYS_NODISCARD utf8_string Dump(uint32 indent) const noexcept
    {
      TextStream multilineStream;
      multilineStream.IncreaseIndent(static_cast<int32>(indent));
      multilineStream << "overlap container - root layer " << static_cast<const void *>(_rootScope.Layer)
                      << " scope layer " << static_cast<const void *>(&_scopeLayer) << " rects "
                      << _rootScope.Rects;

      for (const auto &childScope : _rootScope.Children)
      {
        RecursiveOutputToStream(multilineStream, childScope, 1);
      }

      return multilineStream.Release();
    }

    KRYS_NODISCARD const RenderLayer &ScopeLayer() const noexcept
    {
      return _scopeLayer;
    }

  private:
    static void MergeClippingScopesRecursive(const ClippingScope &sourceScope,
                                             ClippingScope &destScope) noexcept
    {
      krys_debug_assert(sourceScope.Layer == destScope.Layer);
      destScope.Rects.Append(sourceScope.Rects);

      for (const auto &sourceChildScope : sourceScope.Children)
      {
        ClippingScope *destChild = destScope.ChildWithLayer(*sourceChildScope.Layer);
        if (destChild)
        {
          MergeClippingScopesRecursive(sourceChildScope, *destChild);
        }
        else
        {
          // New child, just copy the whole subtree.
          destScope.AddChild(sourceChildScope);
        }
      }
    }

    ClippingScope *EnsureClippingScopeForLayers(
      const LayerOverlapMap::LayerAndBoundsVector &enclosingClippingLayers) noexcept
    {
      krys_debug_assert(!enclosingClippingLayers.empty());
      krys_debug_assert(enclosingClippingLayers[0].Layer->IsRenderViewLayer());

      auto *currScope = &_rootScope;
      for (size_t i = 1; i < enclosingClippingLayers.size(); ++i)
      {
        const auto &scopeLayerAndBounds = enclosingClippingLayers[i];
        auto *childScope = currScope->ChildWithLayer(*scopeLayerAndBounds.Layer);
        if (!childScope)
        {
          currScope = currScope->AddChildWithLayerAndBounds(scopeLayerAndBounds);
          break;
        }

        currScope = childScope;
      }

      return currScope;
    }

    KRYS_NODISCARD ClippingScope *FindClippingScopeForLayers(
      const LayerOverlapMap::LayerAndBoundsVector &enclosingClippingLayers) const noexcept
    {
      krys_debug_assert(!enclosingClippingLayers.empty());
      krys_debug_assert(enclosingClippingLayers[0].Layer->IsRenderViewLayer());

      const auto *currScope = &_rootScope;
      for (size_t i = 1; i < enclosingClippingLayers.size(); ++i)
      {
        const auto &scopeLayerAndBounds = enclosingClippingLayers[i];
        auto *childScope = currScope->ChildWithLayer(*scopeLayerAndBounds.Layer);
        if (!childScope)
        {
          return nullptr;
        }

        currScope = childScope;
      }

      return const_cast<ClippingScope *>(currScope);
    }

    void RecursiveOutputToStream(TextStream &ts, const ClippingScope &scope, uint32 depth) const noexcept
    {
      ts << '\n'
         << Indent << TextStream::Repeat {2 * depth, ' '} << " scope for layer "
         << static_cast<const void *>(scope.Layer) << " rects " << scope.Rects;
      for (const auto &childScope : scope.Children)
      {
        RecursiveOutputToStream(ts, childScope, depth + 1);
      }
    }
  };

  LayerOverlapMap::LayerOverlapMap(const RenderLayer &rootLayer) noexcept
      : _geometryMap({MapCoordinatesMode::UseTransforms}), _rootLayer(rootLayer)
  {
    // Begin assuming the root layer will be composited so that there is something on the stack. The root
    // layer should also never get an popCompositingContainer call.
    PushCompositingContainer(rootLayer);
  }

  LayerOverlapMap::~LayerOverlapMap() noexcept = default;

  void LayerOverlapMap::Add(const RenderLayer &layer, const LayoutRect &bounds,
                            const LayerAndBoundsVector &enclosingClippingLayers) noexcept
  {
    // Layers do not contribute to overlap immediately--instead, they will contribute to overlap as soon as
    // their composited ancestor has been recursively processed and popped off the stack.
    krys_debug_assert(_overlapStack.size() >= 2);
    auto &container = _overlapStack[_overlapStack.size() - 2];
    container->Add(layer, bounds, enclosingClippingLayers);
    if (!_speculativeOverlapStack.empty())
    {
      krys_debug_assert(_speculativeOverlapStack.size() >= 2);
      _speculativeOverlapStack[_speculativeOverlapStack.size() - 2]->Add(layer, bounds,
                                                                         enclosingClippingLayers);
    }

    _isEmpty = false;
  }

  bool LayerOverlapMap::OverlapsLayers(const RenderLayer &layer, const LayoutRect &bounds,
                                       const LayerAndBoundsVector &enclosingClippingLayers) const noexcept
  {
    if (_speculativeOverlapStack.empty())
    {
      return _overlapStack.back()->OverlapsLayers(layer, bounds, enclosingClippingLayers);
    }
    krys_debug_assert(_speculativeOverlapStack.back()->IsEmpty());
    return false;
  }

  void LayerOverlapMap::PushCompositingContainer(const RenderLayer &layer) noexcept
  {
    ConfirmSpeculativeCompositingContainer();
    _overlapStack.push_back(CreateUnique<OverlapMapContainer>(_rootLayer, layer));
  }

  void LayerOverlapMap::PopCompositingContainer(const RenderLayer &layer) noexcept
  {
    krys_debug_assert(&_overlapStack.back()->ScopeLayer() == &layer);
    (void)layer;
    _overlapStack[_overlapStack.size() - 2]->Append(krys::move(_overlapStack.back()));
    _overlapStack.pop_back();
  }

  void LayerOverlapMap::PushSpeculativeCompositingContainer(const RenderLayer &layer) noexcept
  {
    // Create a duplicate copy of the overlap stack, push the container to one, and make all future add calls
    // apply to both stacks. If we end up needing the compositing container, then we copy across the
    // speculative stack to replace the main, otherwise we throw the speculative stack away. If we already
    // have a speculative stack (and we've recursed into this), then just force a real compositing container
    // on the outer, since otherwise we'd have to start tracking four possible outcomes.
    ConfirmSpeculativeCompositingContainer();
    for (const auto &container : _overlapStack)
    {
      _speculativeOverlapStack.push_back(CreateUnique<OverlapMapContainer>(*container));
    }
    _speculativeOverlapStack.push_back(CreateUnique<OverlapMapContainer>(_rootLayer, layer));
  }

  void LayerOverlapMap::ConfirmSpeculativeCompositingContainer() noexcept
  {
    if (!_speculativeOverlapStack.empty())
    {
      _overlapStack.clear();
      _overlapStack.swap(_speculativeOverlapStack);
    }
  }

  bool LayerOverlapMap::MaybePopSpeculativeCompositingContainer() noexcept
  {
    if (!_speculativeOverlapStack.empty())
    {
      _speculativeOverlapStack.clear();
      return true;
    }
    return false;
  }

  TextStream &operator<<(TextStream &ts, const LayerOverlapMap &overlapMap) noexcept
  {
    TextStream multilineStream;

    TextStream::GroupScope scope(ts);
    multilineStream << "LayerOverlapMap\n";
    multilineStream.IncreaseIndent(2);

    bool needNewline = false;
    for (const auto &container : overlapMap.OverlapStack())
    {
      if (needNewline)
      {
        multilineStream << "\n";
      }
      else
      {
        needNewline = true;
      }
      multilineStream << Indent << container->Dump(static_cast<uint32>(multilineStream.GetIndent()));
    }

    ts << multilineStream.Release();
    return ts;
  }
}
