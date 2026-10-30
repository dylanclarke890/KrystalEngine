#include "Krystal.Booey/Rendering/RenderGeometryMap.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  RenderGeometryMap::RenderGeometryMap(EnumSet<MapCoordinatesMode> flags) noexcept
      : _mapCoordinatesFlags(flags)
  {
  }

  RenderGeometryMap::~RenderGeometryMap() noexcept = default;

  void RenderGeometryMap::MapToContainer(TransformState &transformState,
                                         const RenderLayerModelObject *container) const noexcept
  {
    // If the mapping includes something like columns, we have to go via renderers.
    if (HasNonUniformStep())
    {
      auto mode = _mapCoordinatesFlags;
      mode.Add(MapCoordinatesMode::ApplyContainerFlip);
      _mapping.back().Renderer->MapLocalToContainer(container, transformState, mode, nullptr);
      return;
    }

    bool inFixed = false;

    for (int32 i = static_cast<int32>(_mapping.size()) - 1; i >= 0; --i)
    {
      const RenderGeometryMapStep &currentStep = _mapping[static_cast<size_t>(i)];

      // If container is the RenderView (step 0) we want to apply its scroll offset.
      if (i > 0 && currentStep.Renderer == container)
      {
        break;
      }

      // If this box has a transform, it acts as a fixed position container for fixed descendants, which
      // prevents the propagation of 'fixed' unless the layer itself is also fixed position.
      if (i && currentStep.HasTransform && !currentStep.IsFixedPosition)
      {
        inFixed = false;
      }
      else if (currentStep.IsFixedPosition)
      {
        inFixed = true;
      }

      if (!i)
      {
        // The root gets special treatment for fixed position
        if (inFixed)
        {
          transformState.Move(currentStep.Offset.Width(), currentStep.Offset.Height());
        }

        // A null container indicates mapping through the RenderView, so including its transform (the page
        // scale).
        if (!container && currentStep.Transform)
        {
          transformState.ApplyTransform(*currentStep.Transform);
        }
      }
      else
      {
        auto accumulate = currentStep.AccumulatingTransform
                            ? TransformState::TransformAccumulation::AccumulateTransform
                            : TransformState::TransformAccumulation::FlattenTransform;
        if (currentStep.Transform)
        {
          transformState.ApplyTransform(*currentStep.Transform, accumulate);
        }
        else
        {
          transformState.Move(currentStep.Offset.Width(), currentStep.Offset.Height(), accumulate);
        }
      }
    }
  }

  FloatPoint RenderGeometryMap::MapToContainer(const FloatPoint &p,
                                               const RenderLayerModelObject *container) const noexcept
  {
    FloatPoint result;

    if (!HasFixedPositionStep() && !HasTransformStep() && !HasNonUniformStep()
        && (!container || (!_mapping.empty() && container == _mapping[0].Renderer)))
    {
      result = p;
      result.Move(_accumulatedOffset);
    }
    else
    {
      TransformState transformState(TransformState::TransformDirection::ApplyTransform, p);
      MapToContainer(transformState, container);
      result = transformState.MappedPoint();
    }

    return result;
  }

  FloatQuad RenderGeometryMap::MapToContainer(const FloatRect &rect,
                                              const RenderLayerModelObject *container) const noexcept
  {
    FloatQuad result;

    if (!HasFixedPositionStep() && !HasTransformStep() && !HasNonUniformStep()
        && (!container || (!_mapping.empty() && container == _mapping[0].Renderer)))
    {
      result = FloatQuad(rect);
      result.Move(_accumulatedOffset);
    }
    else
    {
      TransformState transformState(TransformState::TransformDirection::ApplyTransform, rect.Center(),
                                    FloatQuad(rect));
      MapToContainer(transformState, container);
      result = transformState.MappedQuad();
    }

    return result;
  }

  void RenderGeometryMap::PushMappingsToAncestor(const RenderElement *renderer,
                                                 const RenderLayerModelObject *ancestorRenderer) noexcept
  {
    // We need to push mappings in reverse order here, so do insertions rather than appends.
    SetForScope positionChange(_insertionPosition, _mapping.size());
    do
    {
      renderer = renderer->PushMappingToContainer(ancestorRenderer, *this);
    } while (renderer && renderer != ancestorRenderer);

    krys_debug_assert(_mapping.empty() || _mapping[0].Renderer->IsRenderView());
  }

  namespace
  {
    KRYS_NODISCARD bool CanMapBetweenRenderersViaLayers(const RenderLayerModelObject &renderer,
                                                        const RenderLayerModelObject &ancestor) noexcept
    {
      for (const RenderElement *current = &renderer;; current = current->Parent())
      {
        const RenderStyle &style = current->Style();
        if (current->IsFixedPositioned() || style.GetWritingMode().IsBlockFlipped())
        {
          return false;
        }

        if (current->HasTransformOrPerspective())
        {
          return false;
        }

        // FIXME: Fragmented flows, once there are any.

        if (current->IsLegacyRenderSVGRoot())
        {
          return false;
        }

        if (current == &ancestor)
        {
          break;
        }
      }

      return true;
    }
  }

  void RenderGeometryMap::PushMappingsToAncestor(const RenderLayer *layer, const RenderLayer *ancestorLayer,
                                                 bool respectTransforms) noexcept
  {
    if (!ancestorLayer)
    {
      krys_debug_assert(_mapping.empty());
      PushMappingsToAncestor(&layer->Renderer().View(), nullptr);

      // Ancestors are pushed first: each push appends to the mappings.
      List<const RenderLayer *> layers;
      for (; layer->Parent(); layer = layer->Parent())
      {
        layers.push_back(layer);
      }
      for (auto it = layers.rbegin(); it != layers.rend(); ++it)
      {
        PushMappingsToAncestor(*it, (*it)->Parent(), respectTransforms);
      }
      krys_debug_assert(_mapping[0].Renderer->IsRenderView());
      return;
    }

    auto newFlags = _mapCoordinatesFlags;
    if (!respectTransforms)
    {
      newFlags.Remove(MapCoordinatesMode::UseTransforms);
    }

    SetForScope flagsChange(_mapCoordinatesFlags, newFlags);

    const RenderLayerModelObject &renderer = layer->Renderer();

    // We have to visit all the renderers to detect flipped blocks. This might defeat the gains from mapping
    // via layers.
    if (CanMapBetweenRenderersViaLayers(renderer, ancestorLayer->Renderer()))
    {
      LayoutSize layerOffset = layer->OffsetFromAncestor(ancestorLayer);

      // The RenderView must be pushed first.
      if (_mapping.empty())
      {
        krys_debug_assert(ancestorLayer->Renderer().IsRenderView());
        PushMappingsToAncestor(&ancestorLayer->Renderer(), nullptr);
      }

      SetForScope positionChange(_insertionPosition, _mapping.size());
      Push(&renderer, layerOffset, /*accumulatingTransform*/ true, /*isNonUniform*/ false,
           /*isFixedPosition*/ false,
           /*hasTransform*/ false);
      return;
    }
    const RenderLayerModelObject *ancestorRenderer = &ancestorLayer->Renderer();
    PushMappingsToAncestor(&renderer, ancestorRenderer);
  }

  void RenderGeometryMap::Push(const RenderObject *renderer, const LayoutSize &offsetFromContainer,
                               bool accumulatingTransform, bool isNonUniform, bool isFixedPosition,
                               bool hasTransform) noexcept
  {
    krys_debug_assert(_insertionPosition != notFound);

    auto it = _mapping.emplace(_mapping.begin() + static_cast<ptrdiff_t>(_insertionPosition), renderer,
                               accumulatingTransform, isNonUniform, isFixedPosition, hasTransform);
    it->Offset = offsetFromContainer;

    StepInserted(*it);
  }

  void RenderGeometryMap::Push(const RenderObject *renderer, const TransformationMatrix &t,
                               bool accumulatingTransform, bool isNonUniform, bool isFixedPosition,
                               bool hasTransform) noexcept
  {
    krys_debug_assert(_insertionPosition != notFound);

    auto it = _mapping.emplace(_mapping.begin() + static_cast<ptrdiff_t>(_insertionPosition), renderer,
                               accumulatingTransform, isNonUniform, isFixedPosition, hasTransform);
    if (!t.IsIntegerTranslation())
    {
      it->Transform = CreateUnique<TransformationMatrix>(t);
    }
    else
    {
      it->Offset = LayoutSize(LayoutUnit(t.E()), LayoutUnit(t.F()));
    }

    StepInserted(*it);
  }

  void RenderGeometryMap::PushView(const RenderView *view, const LayoutSize &scrollOffset,
                                   const TransformationMatrix *t) noexcept
  {
    krys_debug_assert(_insertionPosition != notFound);
    krys_debug_assert(!_insertionPosition); // The view should always be the first step.

    auto it = _mapping.emplace(_mapping.begin() + static_cast<ptrdiff_t>(_insertionPosition), view, false,
                               false, false, t != nullptr);
    it->Offset = scrollOffset;
    if (t)
    {
      it->Transform = CreateUnique<TransformationMatrix>(*t);
    }

    StepInserted(*it);
  }

  void RenderGeometryMap::PopMappingsToAncestor(const RenderLayerModelObject *ancestorRenderer) noexcept
  {
    krys_debug_assert(!_mapping.empty());

    while (!_mapping.empty() && _mapping.back().Renderer != ancestorRenderer)
    {
      StepRemoved(_mapping.back());
      _mapping.pop_back();
    }
  }

  void RenderGeometryMap::PopMappingsToAncestor(const RenderLayer *ancestorLayer) noexcept
  {
    const RenderLayerModelObject *ancestorRenderer = ancestorLayer ? &ancestorLayer->Renderer() : nullptr;
    PopMappingsToAncestor(ancestorRenderer);
  }

  void RenderGeometryMap::StepInserted(const RenderGeometryMapStep &step) noexcept
  {
    // RenderView's offset, is only applied when we have fixed-positions.
    if (!step.Renderer->IsRenderView())
    {
      _accumulatedOffset += step.Offset;
    }

    if (step.IsNonUniform)
    {
      ++_nonUniformStepsCount;
    }

    if (step.Transform)
    {
      ++_transformedStepsCount;
    }

    if (step.IsFixedPosition)
    {
      ++_fixedStepsCount;
    }
  }

  void RenderGeometryMap::StepRemoved(const RenderGeometryMapStep &step) noexcept
  {
    // RenderView's offset, is only applied when we have fixed-positions.
    if (!step.Renderer->IsRenderView())
    {
      _accumulatedOffset -= step.Offset;
    }

    if (step.IsNonUniform)
    {
      krys_debug_assert(_nonUniformStepsCount);
      --_nonUniformStepsCount;
    }

    if (step.Transform)
    {
      krys_debug_assert(_transformedStepsCount);
      --_transformedStepsCount;
    }

    if (step.IsFixedPosition)
    {
      krys_debug_assert(_fixedStepsCount);
      --_fixedStepsCount;
    }
  }
}
