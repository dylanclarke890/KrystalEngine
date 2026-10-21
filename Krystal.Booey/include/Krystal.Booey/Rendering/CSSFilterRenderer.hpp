#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FilterOperation;
  class FilterOperations;
  class GraphicsContext;
  class RenderElement;

  namespace style
  {
    struct Filter;
  }

  /// @brief The filter a `filter` property (or a list of filter operations) builds: a filter effect per
  /// function, after the source graphic.
  // FIXME: Reference filters (`filter: url()`, SVGFilterRenderer), once there's SVG; applying the filter in
  // software (apply), once filters can be.
  class CSSFilterRenderer final : public Filter
  {
  private:
    List<Ref<FilterFunction>> _functions;
    bool _hasFilterThatMovesPixels {false};
    bool _hasFilterThatShouldBeRestrictedBySecurityOrigin {false};

  public:
    KRYS_NODISCARD static RefPtr<CSSFilterRenderer>
      Create(RenderElement &renderer, const style::Filter &filter, const FilterGeometry &geometry,
             EnumSet<FilterRenderingMode> preferredRenderingModes,
             const GraphicsContext &destinationContext) noexcept;
    KRYS_NODISCARD static RefPtr<CSSFilterRenderer>
      Create(RenderElement &renderer, const FilterOperations &operations, const FilterGeometry &geometry,
             EnumSet<FilterRenderingMode> preferredRenderingModes,
             const GraphicsContext &destinationContext) noexcept;

    KRYS_NODISCARD static Ref<CSSFilterRenderer>
      Create(List<Ref<FilterFunction>> &&functions, const FilterGeometry &geometry,
             EnumSet<FilterRenderingMode> preferredFilterRenderingModes) noexcept;

    KRYS_NODISCARD const List<Ref<FilterFunction>> &Functions() const noexcept
    {
      return _functions;
    }
    void SetFilterRegion(const FloatRect &filterRegion) noexcept;

    KRYS_NODISCARD bool HasFilterThatMovesPixels() const noexcept
    {
      return _hasFilterThatMovesPixels;
    }
    KRYS_NODISCARD bool HasFilterThatShouldBeRestrictedBySecurityOrigin() const noexcept
    {
      return _hasFilterThatShouldBeRestrictedBySecurityOrigin;
    }

    KRYS_NODISCARD FilterEffectVector EffectsOfType(FilterFunction::Type filterType) const noexcept final;

    KRYS_NODISCARD FilterStyleVector CreateFilterStyles(GraphicsContext &context,
                                                        const FilterStyle &sourceStyle) const noexcept final;

    KRYS_NODISCARD static bool IsIdentity(RenderElement &renderer, const style::Filter &filter) noexcept;
    KRYS_NODISCARD static bool IsIdentity(RenderElement &renderer,
                                          const FilterOperations &operations) noexcept;
    KRYS_NODISCARD static IntOutsets CalculateOutsets(RenderElement &renderer, const style::Filter &filter,
                                                      const FloatRect &targetBoundingBox) noexcept;
    KRYS_NODISCARD static IntOutsets CalculateOutsets(RenderElement &renderer,
                                                      const FilterOperations &operations,
                                                      const FloatRect &targetBoundingBox) noexcept;

  private:
    template <typename TFilter>
    KRYS_NODISCARD static RefPtr<CSSFilterRenderer>
      CreateGeneric(RenderElement &renderer, const TFilter &filter, const FilterGeometry &geometry,
                    EnumSet<FilterRenderingMode> preferredRenderingModes,
                    const GraphicsContext &destinationContext) noexcept;

    CSSFilterRenderer(const FilterGeometry &geometry, bool hasFilterThatMovesPixels,
                      bool hasFilterThatShouldBeRestrictedBySecurityOrigin) noexcept;
    CSSFilterRenderer(List<Ref<FilterFunction>> &&functions, const FilterGeometry &geometry) noexcept;

    KRYS_NODISCARD RefPtr<FilterFunction>
      BuildFilterFunction(RenderElement &renderer, const FilterOperation &operation,
                          EnumSet<FilterRenderingMode> preferredRenderingModes,
                          const GraphicsContext &destinationContext) noexcept;
    template <typename TFilter>
    bool BuildFilterFunctions(RenderElement &renderer, const TFilter &filter,
                              EnumSet<FilterRenderingMode> preferredRenderingModes,
                              const GraphicsContext &destinationContext) noexcept;

    KRYS_NODISCARD EnumSet<FilterRenderingMode> SupportedFilterRenderingModes(
      EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept final;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(CSSFilterRenderer)
