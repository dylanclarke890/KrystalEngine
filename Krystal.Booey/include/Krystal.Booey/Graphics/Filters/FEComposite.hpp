#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  enum class CompositeOperationType : uint8
  {
    FECOMPOSITE_OPERATOR_UNKNOWN = 0,
    FECOMPOSITE_OPERATOR_OVER = 1,
    FECOMPOSITE_OPERATOR_IN = 2,
    FECOMPOSITE_OPERATOR_OUT = 3,
    FECOMPOSITE_OPERATOR_ATOP = 4,
    FECOMPOSITE_OPERATOR_XOR = 5,
    FECOMPOSITE_OPERATOR_ARITHMETIC = 6,
    FECOMPOSITE_OPERATOR_LIGHTER = 7
  };

  /// @brief The composite filter effect: its inputs combined by a Porter-Duff operator, or arithmetically.
  class FEComposite final : public FilterEffect
  {
  private:
    CompositeOperationType _type;
    float _k1;
    float _k2;
    float _k3;
    float _k4;

  public:
    KRYS_NODISCARD static Ref<FEComposite>
      Create(const CompositeOperationType &type, float k1, float k2, float k3, float k4,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEComposite &other) const noexcept;

    KRYS_NODISCARD CompositeOperationType Operation() const noexcept
    {
      return _type;
    }
    bool SetOperation(CompositeOperationType type) noexcept;

    KRYS_NODISCARD float K1() const noexcept
    {
      return _k1;
    }
    bool SetK1(float k1) noexcept;

    KRYS_NODISCARD float K2() const noexcept
    {
      return _k2;
    }
    bool SetK2(float k2) noexcept;

    KRYS_NODISCARD float K3() const noexcept
    {
      return _k3;
    }
    bool SetK3(float k3) noexcept;

    KRYS_NODISCARD float K4() const noexcept
    {
      return _k4;
    }
    bool SetK4(float k4) noexcept;

  private:
    FEComposite(const CompositeOperationType &type, float k1, float k2, float k3, float k4,
                const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEComposite>(*this, other);
    }

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return 2u;
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD bool ResultIsValidPremultiplied() const noexcept override
    {
      return _type != CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC;
    }

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEComposite)
