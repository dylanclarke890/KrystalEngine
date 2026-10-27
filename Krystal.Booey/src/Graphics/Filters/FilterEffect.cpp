#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  FilterEffect::FilterEffect(Type type, const DestinationColorSpace &colorSpace) noexcept
      : FilterFunction(type), _operatingColorSpace(colorSpace)
  {
  }

  bool FilterEffect::operator==(const FilterEffect &other) const noexcept
  {
    if (FilterType() != other.FilterType())
    {
      return false;
    }
    return _operatingColorSpace == other._operatingColorSpace;
  }

  FilterImageVector FilterEffect::TakeImageInputs(FilterImageVector &stack) const noexcept
  {
    uint32 inputsSize = NumberOfImageInputs();
    if (stack.size() < inputsSize)
    {
      return {};
    }

    size_t index = stack.size();
    FilterImageVector result;
    result.reserve(inputsSize);
    for (uint32 i = 0u; i < inputsSize; ++i)
    {
      result.push_back(krys::move(stack[--index]));
    }
    stack.erase(stack.end() - inputsSize, stack.end());
    return result;
  }

  namespace
  {
    KRYS_NODISCARD List<FloatRect> InputPrimitiveSubregions(Span<const Ref<FilterImage>> inputs) noexcept
    {
      List<FloatRect> result;
      result.reserve(inputs.size());
      for (const auto &input : inputs)
      {
        result.push_back(input->PrimitiveSubregion());
      }
      return result;
    }

    KRYS_NODISCARD List<FloatRect> InputImageRects(Span<const Ref<FilterImage>> inputs) noexcept
    {
      List<FloatRect> result;
      result.reserve(inputs.size());
      for (const auto &input : inputs)
      {
        result.push_back(input->ImageRect());
      }
      return result;
    }
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

  UniquePtr<FilterEffectApplier> FilterEffect::CreateApplier(const Filter &filter,
                                                             const FilterResults &results) const noexcept
  {
    if (filter.FilterRenderingModes().Contains(FilterRenderingMode::Accelerated))
    {
      return results.CreateAcceleratedApplier(*this);
    }
    // There's no software rendering.
    return nullptr;
  }

  void FilterEffect::TransformInputsColorSpace(Span<const Ref<FilterImage>> inputs) const noexcept
  {
    for (const auto &input : inputs)
    {
      input->TransformToColorSpace(OperatingColorSpace());
    }
  }

  RefPtr<FilterImage> FilterEffect::Apply(const Filter &filter, FilterImage &input,
                                          FilterResults &results) noexcept
  {
    Ref protectedInput = ShareRef(input);
    return Apply(filter, Span<const Ref<FilterImage>>(&protectedInput, 1u), results);
  }

  RefPtr<FilterImage> FilterEffect::Apply(const Filter &filter, Span<const Ref<FilterImage>> inputs,
                                          FilterResults &results,
                                          const Maybe<FilterEffectGeometry> &geometry) noexcept
  {
    if (inputs.size() != NumberOfImageInputs())
    {
      return nullptr;
    }

    if (auto *result = results.EffectResult(*this))
    {
      return ShareRefPtr(result);
    }

    auto primitiveSubregion = CalculatePrimitiveSubregion(filter, InputPrimitiveSubregions(inputs), geometry);
    auto imageRect = CalculateImageRect(filter, InputImageRects(inputs), primitiveSubregion);
    auto absoluteImageRect = EnclosingIntRect(filter.ScaledByFilterScale(imageRect));

    if (absoluteImageRect.IsEmpty() || ImageBuffer::SizeNeedsClamping(FloatSize(absoluteImageRect.Size())))
    {
      return nullptr;
    }

    auto applier = CreateApplier(filter, results);
    if (!applier)
    {
      return nullptr;
    }

    auto isAlphaImage = ResultIsAlphaImage(inputs);
    auto isValidPremultiplied = ResultIsValidPremultiplied();
    const auto &imageColorSpace = ResultColorSpace(inputs);

    auto result = FilterImage::Create(primitiveSubregion, imageRect, absoluteImageRect, isAlphaImage,
                                      isValidPremultiplied, imageColorSpace);

    TransformInputsColorSpace(inputs);

    if (!applier->Apply(filter, inputs, *result))
    {
      return nullptr;
    }

    results.SetEffectResult(*this, inputs, Ref(result));
    return result;
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

  TextStream &FilterEffect::ExternalRepresentation(TextStream &ts,
                                                   FilterRepresentation representation) const noexcept
  {
    // FIXME: We should dump the subRegions of the filter primitives here later. This isn't
    // possible at the moment, because we need more detailed informations from the target object.

    if (representation == FilterRepresentation::Debugging)
    {
      TextStream::IndentScope indentScope(ts);
      ts.DumpProperty(u8"operating colorspace", OperatingColorSpace());
    }
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
