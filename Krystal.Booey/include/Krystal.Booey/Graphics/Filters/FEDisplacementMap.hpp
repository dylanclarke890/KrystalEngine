#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  enum class ChannelSelectorType : uint8
  {
    CHANNEL_UNKNOWN = 0,
    CHANNEL_R = 1,
    CHANNEL_G = 2,
    CHANNEL_B = 3,
    CHANNEL_A = 4
  };

  /// @brief The displacement map filter effect: its first input's pixels moved by its second's channels.
  class FEDisplacementMap final : public FilterEffect
  {
  private:
    ChannelSelectorType _xChannelSelector;
    ChannelSelectorType _yChannelSelector;
    float _scale;

  public:
    KRYS_NODISCARD static Ref<FEDisplacementMap>
      Create(ChannelSelectorType xChannelSelector, ChannelSelectorType yChannelSelector, float scale,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEDisplacementMap &other) const noexcept;

    KRYS_NODISCARD ChannelSelectorType XChannelSelector() const noexcept
    {
      return _xChannelSelector;
    }
    bool SetXChannelSelector(ChannelSelectorType xChannelSelector) noexcept;

    KRYS_NODISCARD ChannelSelectorType YChannelSelector() const noexcept
    {
      return _yChannelSelector;
    }
    bool SetYChannelSelector(ChannelSelectorType yChannelSelector) noexcept;

    KRYS_NODISCARD float Scale() const noexcept
    {
      return _scale;
    }
    bool SetScale(float scale) noexcept;

    KRYS_NODISCARD static IntOutsets CalculateOutsets(const FloatSize &maxDisplacement) noexcept;

  private:
    FEDisplacementMap(ChannelSelectorType xChannelSelector, ChannelSelectorType yChannelSelector, float scale,
                      const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEDisplacementMap>(*this, other);
    }

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return 2u;
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD const DestinationColorSpace &
      ResultColorSpace(Span<const Ref<FilterImage>> inputs) const noexcept override;
    void TransformInputsColorSpace(Span<const Ref<FilterImage>> inputs) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEDisplacementMap)
