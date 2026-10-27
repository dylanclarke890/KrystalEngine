#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  enum class ColorMatrixType : uint8
  {
    FECOLORMATRIX_TYPE_UNKNOWN = 0,
    FECOLORMATRIX_TYPE_MATRIX = 1,
    FECOLORMATRIX_TYPE_SATURATE = 2,
    FECOLORMATRIX_TYPE_HUEROTATE = 3,
    FECOLORMATRIX_TYPE_LUMINANCETOALPHA = 4
  };

  /// @brief The colour matrix filter effect: a matrix applied to each pixel's straight-alpha colour.
  class FEColorMatrix final : public FilterEffect
  {
  private:
    ColorMatrixType _type;
    List<float> _values;

  public:
    KRYS_NODISCARD static Ref<FEColorMatrix> Create(ColorMatrixType type, List<float> &&values) noexcept;

    KRYS_NODISCARD bool operator==(const FEColorMatrix &other) const noexcept;

    KRYS_NODISCARD ColorMatrixType GetType() const noexcept
    {
      return _type;
    }
    bool SetType(ColorMatrixType type) noexcept;

    KRYS_NODISCARD const List<float> &Values() const noexcept
    {
      return _values;
    }
    bool SetValues(const List<float> &values) noexcept;

    static void CalculateSaturateComponents(Span<float, 9> components, float value) noexcept;
    static void CalculateHueRotateComponents(Span<float, 9> components, float value) noexcept;
    KRYS_NODISCARD static List<float> NormalizedFloats(const List<float> &values) noexcept;

  private:
    FEColorMatrix(ColorMatrixType type, List<float> &&values) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEColorMatrix>(*this, other);
    }

    KRYS_NODISCARD bool ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept override;

    KRYS_NODISCARD EnumSet<FilterRenderingMode> SupportedFilterRenderingModes(
      EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept override;

    KRYS_NODISCARD Maybe<GraphicsStyle> CreateGraphicsStyle(GraphicsContext &context,
                                                            const Filter &filter) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEColorMatrix)
