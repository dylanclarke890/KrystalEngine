#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A filter's input: the alpha of what's drawn, as black.
  class SourceAlpha final : public FilterEffect
  {
  public:
    KRYS_NODISCARD static Ref<SourceAlpha>
      Create(const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD static utf8_stringview EffectName() noexcept
    {
      return FilterEffect::SourceAlphaName();
    }

  private:
    explicit SourceAlpha(const DestinationColorSpace &colorSpace) noexcept;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(SourceAlpha)
