#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffectGeometry.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterFunction.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsStyle.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  class Filter;

  /// @brief One filter primitive.
  // FIXME: The operating colour space (filters operate in sRGB; SVG filters can ask for linear RGB), image
  // inputs and appliers (takeImageInputs, apply, createApplier and the result's colour space and
  // premultiplication), once there's SVG and a rendering mode other than the graphics context.
  class FilterEffect : public FilterFunction
  {
  public:
    KRYS_NODISCARD virtual bool operator==(const FilterEffect &other) const noexcept;

    KRYS_NODISCARD uint32 NumberOfImageInputs() const noexcept
    {
      return FilterType() == FilterEffect::Type::SourceGraphic ? 1u : NumberOfEffectInputs();
    }

    /// @brief The style that applies the effect in the graphics context, following @p input, and where its
    /// result goes.
    KRYS_NODISCARD FilterStyle
      CreateFilterStyle(GraphicsContext &context, const Filter &filter, const FilterStyle &input,
                        const Maybe<FilterEffectGeometry> &geometry = null) const noexcept;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;

  protected:
    explicit FilterEffect(Type type) noexcept;

    template <typename FilterEffectType>
    KRYS_NODISCARD static bool AreEqual(const FilterEffectType &a, const FilterEffect &b) noexcept
    {
      auto *bType = DynamicDowncast<FilterEffectType>(b);
      return bType && a.operator==(*bType);
    }

    KRYS_NODISCARD virtual uint32 NumberOfEffectInputs() const noexcept
    {
      return 1u;
    }

    KRYS_NODISCARD FloatRect
      CalculatePrimitiveSubregion(const Filter &filter, Span<const FloatRect> inputPrimitiveSubregions,
                                  const Maybe<FilterEffectGeometry> &geometry) const noexcept;

    KRYS_NODISCARD virtual FloatRect CalculateImageRect(const Filter &filter,
                                                        Span<const FloatRect> inputImageRects,
                                                        const FloatRect &primitiveSubregion) const noexcept;

    KRYS_NODISCARD virtual Maybe<GraphicsStyle> CreateGraphicsStyle(GraphicsContext &,
                                                                    const Filter &) const noexcept
    {
      return null;
    }

    KRYS_NODISCARD FilterStyleVector CreateFilterStyles(GraphicsContext &context, const Filter &filter,
                                                        const FilterStyle &input) const noexcept override;
  };

  TextStream &operator<<(TextStream &ts, const FilterEffect &filterEffect) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::FilterEffect)
  KRYS_NODISCARD static bool IsType(const krys::boo::FilterFunction &function) noexcept
  {
    return function.IsFilterEffect();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
