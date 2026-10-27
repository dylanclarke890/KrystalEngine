#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  enum class MorphologyOperatorType : uint8
  {
    Unknown,
    Erode,
    Dilate
  };

  /// @brief The morphology filter effect: its input thinned (eroded) or fattened (dilated).
  class FEMorphology final : public FilterEffect
  {
  private:
    MorphologyOperatorType _type;
    float _radiusX;
    float _radiusY;

  public:
    KRYS_NODISCARD static Ref<FEMorphology>
      Create(MorphologyOperatorType type, float radiusX, float radiusY,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEMorphology &other) const noexcept;

    KRYS_NODISCARD MorphologyOperatorType MorphologyOperator() const noexcept
    {
      return _type;
    }
    bool SetMorphologyOperator(MorphologyOperatorType type) noexcept;

    KRYS_NODISCARD float RadiusX() const noexcept
    {
      return _radiusX;
    }
    bool SetRadiusX(float radiusX) noexcept;

    KRYS_NODISCARD float RadiusY() const noexcept
    {
      return _radiusY;
    }
    bool SetRadiusY(float radiusY) noexcept;

  private:
    FEMorphology(MorphologyOperatorType type, float radiusX, float radiusY,
                 const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEMorphology>(*this, other);
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD bool ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEMorphology)
