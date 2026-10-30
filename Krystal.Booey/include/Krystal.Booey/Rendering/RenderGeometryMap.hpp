#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderElement;
  class RenderLayer;
  class RenderLayerModelObject;
  class RenderView;
  class TransformState;

  /// @brief Stores data about how to map from one renderer to its container.
  struct RenderGeometryMapStep
  {
    const RenderObject *Renderer;
    LayoutSize Offset;
    UniquePtr<TransformationMatrix> Transform; // Includes offset if non-null.
    bool AccumulatingTransform;
    bool IsNonUniform; // Mapping depends on the input point, e.g. because of CSS columns.
    bool IsFixedPosition;
    bool HasTransform;

    RenderGeometryMapStep(const RenderObject *renderer, bool accumulatingTransform, bool isNonUniform,
                          bool isFixedPosition, bool hasTransform) noexcept
        : Renderer(renderer), AccumulatingTransform(accumulatingTransform), IsNonUniform(isNonUniform),
          IsFixedPosition(isFixedPosition), HasTransform(hasTransform)
    {
    }
  };

  /// @brief Can be used while walking the renderer tree to cache data about offsets and transforms.
  class RenderGeometryMap
  {
    KRYS_NON_COPY_MOVABLE(RenderGeometryMap);

  private:
    static constexpr size_t notFound = static_cast<size_t>(-1);

    size_t _insertionPosition {notFound};
    int32 _nonUniformStepsCount {0};
    int32 _transformedStepsCount {0};
    int32 _fixedStepsCount {0};
    List<RenderGeometryMapStep> _mapping;
    LayoutSize _accumulatedOffset;
    EnumSet<MapCoordinatesMode> _mapCoordinatesFlags;

  public:
    explicit RenderGeometryMap(EnumSet<MapCoordinatesMode> flags = {
                                 MapCoordinatesMode::UseTransforms}) noexcept;
    ~RenderGeometryMap() noexcept;

    KRYS_NODISCARD EnumSet<MapCoordinatesMode> MapCoordinatesFlags() const noexcept
    {
      return _mapCoordinatesFlags;
    }

    KRYS_NODISCARD FloatRect AbsoluteRect(const FloatRect &rect) const noexcept
    {
      return MapToContainer(rect, nullptr).BoundingBox();
    }

    // Map to a container. Will assert that the container has been pushed onto this map. A null container maps
    // through the RenderView (including its scale transform, if any). If the container is the RenderView, the
    // scroll offset is applied, but not the scale.
    KRYS_NODISCARD FloatPoint MapToContainer(const FloatPoint &p,
                                             const RenderLayerModelObject *container) const noexcept;
    KRYS_NODISCARD FloatQuad MapToContainer(const FloatRect &rect,
                                            const RenderLayerModelObject *container) const noexcept;

    // Called by code walking the renderer or layer trees.
    void PushMappingsToAncestor(const RenderLayer *layer, const RenderLayer *ancestorLayer,
                                bool respectTransforms = true) noexcept;
    void PopMappingsToAncestor(const RenderLayer *ancestorLayer) noexcept;
    void PushMappingsToAncestor(const RenderElement *renderer,
                                const RenderLayerModelObject *ancestorRenderer) noexcept;
    void PopMappingsToAncestor(const RenderLayerModelObject *ancestorRenderer) noexcept;

    // The following methods should only be called by renderers inside a call to pushMappingsToAncestor().

    // Push geometry info between this renderer and some ancestor. The ancestor must be its container() or
    // some stacking context between the renderer and its container.
    void Push(const RenderObject *renderer, const LayoutSize &offsetFromContainer,
              bool accumulatingTransform = false, bool isNonUniform = false, bool isFixedPosition = false,
              bool hasTransform = false) noexcept;
    void Push(const RenderObject *renderer, const TransformationMatrix &t, bool accumulatingTransform = false,
              bool isNonUniform = false, bool isFixedPosition = false, bool hasTransform = false) noexcept;

    // RenderView gets special treatment, because it applies the scroll offset only for elements inside in
    // fixed position.
    void PushView(const RenderView *view, const LayoutSize &scrollOffset,
                  const TransformationMatrix *t = nullptr) noexcept;

  private:
    void MapToContainer(TransformState &transformState,
                        const RenderLayerModelObject *container = nullptr) const noexcept;
    void StepInserted(const RenderGeometryMapStep &step) noexcept;
    void StepRemoved(const RenderGeometryMapStep &step) noexcept;

    KRYS_NODISCARD bool HasNonUniformStep() const noexcept
    {
      return _nonUniformStepsCount;
    }
    KRYS_NODISCARD bool HasTransformStep() const noexcept
    {
      return _transformedStepsCount;
    }
    KRYS_NODISCARD bool HasFixedPositionStep() const noexcept
    {
      return _fixedStepsCount;
    }
  };
}
