#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  FilterEffect::FilterEffect(Type type) noexcept : FilterFunction(type)
  {
  }

  bool FilterEffect::operator==(const FilterEffect &other) const noexcept
  {
    return FilterType() == other.FilterType();
  }

  FloatRect
    FilterEffect::CalculatePrimitiveSubregion(const Filter &filter,
                                              Span<const FloatRect> inputPrimitiveSubregions,
                                              const Maybe<FilterEffectGeometry> &geometry) const noexcept
  {
    // This function implements https://www.w3.org/TR/filter-effects-1/#FilterPrimitiveSubRegion.
    FloatRect primitiveSubregion;

    // If there is no input effects, take the effect boundaries as unite rect. Don't use the input's subregion
    // for FETile.
    if (!inputPrimitiveSubregions.empty() && FilterType() != FilterEffect::Type::FETile)
    {
      for (const auto &inputPrimitiveSubregion : inputPrimitiveSubregions)
      {
        primitiveSubregion.Unite(inputPrimitiveSubregion);
      }
    }
    else
    {
      primitiveSubregion = filter.FilterRegion();
    }

    // Clip the primitive subregion to the effect geometry.
    if (geometry)
    {
      if (auto x = geometry->X())
      {
        primitiveSubregion.SetX(*x);
      }
      if (auto y = geometry->Y())
      {
        primitiveSubregion.SetY(*y);
      }
      if (auto width = geometry->Width())
      {
        primitiveSubregion.SetWidth(*width);
      }
      if (auto height = geometry->Height())
      {
        primitiveSubregion.SetHeight(*height);
      }
    }

    return primitiveSubregion;
  }

  FloatRect FilterEffect::CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                             const FloatRect &primitiveSubregion) const noexcept
  {
    if (inputImageRects.empty())
    {
      return filter.MaxEffectRect(primitiveSubregion);
    }

    FloatRect imageRect;
    for (const auto &inputImageRect : inputImageRects)
    {
      imageRect.Unite(inputImageRect);
    }

    return filter.ClipToMaxEffectRect(imageRect, primitiveSubregion);
  }

  FilterStyleVector FilterEffect::CreateFilterStyles(GraphicsContext &context, const Filter &filter,
                                                     const FilterStyle &input) const noexcept
  {
    return {CreateFilterStyle(context, filter, input)};
  }

  FilterStyle FilterEffect::CreateFilterStyle(GraphicsContext &context, const Filter &filter,
                                              const FilterStyle &input,
                                              const Maybe<FilterEffectGeometry> &geometry) const noexcept
  {
    krys_debug_assert(filter.FilterRenderingModes().Contains(FilterRenderingMode::GraphicsContext));

    auto primitiveSubregion =
      CalculatePrimitiveSubregion(filter, Span<const FloatRect> {&input.PrimitiveSubregion, 1u}, geometry);
    auto imageRect =
      CalculateImageRect(filter, Span<const FloatRect> {&input.ImageRect, 1u}, primitiveSubregion);

    auto style = CreateGraphicsStyle(context, filter);
    return FilterStyle {style, primitiveSubregion, imageRect};
  }

  TextStream &FilterEffect::ExternalRepresentation(TextStream &ts, FilterRepresentation) const noexcept
  {
    // FIXME: We should dump the subRegions of the filter primitives here later. This isn't
    // possible at the moment, because we need more detailed informations from the target object.
    // FIXME: The operating colour space, once effects have one.
    return ts;
  }

  TextStream &operator<<(TextStream &ts, const FilterEffect &filterEffect) noexcept
  {
    // Use a new stream because we want multiline mode for logging filters.
    TextStream filterStream;
    filterEffect.ExternalRepresentation(filterStream, FilterRepresentation::Debugging);

    return ts << filterStream.Release();
  }
}
