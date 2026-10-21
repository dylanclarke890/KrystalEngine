#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A filter's input: what's drawn, as it is.
  class SourceGraphic final : public FilterEffect
  {
  public:
    KRYS_NODISCARD static Ref<SourceGraphic> Create() noexcept;

    KRYS_NODISCARD static utf8_stringview EffectName() noexcept
    {
      return FilterEffect::SourceGraphicName();
    }

  private:
    SourceGraphic() noexcept;

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return 0u;
    }

    KRYS_NODISCARD EnumSet<FilterRenderingMode> SupportedFilterRenderingModes(
      EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(SourceGraphic)
