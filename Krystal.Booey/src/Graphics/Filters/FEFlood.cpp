#include "Krystal.Booey/Graphics/Filters/FEFlood.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEFlood> FEFlood::Create(const Color &floodColor, float floodOpacity,
                               const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEFlood(floodColor, floodOpacity, colorSpace));
  }

  FEFlood::FEFlood(const Color &floodColor, float floodOpacity,
                   const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEFlood, colorSpace), _floodColor(floodColor),
        _floodOpacity(floodOpacity)
  {
  }

  bool FEFlood::operator==(const FEFlood &other) const noexcept
  {
    return FilterEffect::operator==(other) && _floodColor == other._floodColor
           && _floodOpacity == other._floodOpacity;
  }

  bool FEFlood::SetFloodColor(const Color &color) noexcept
  {
    if (_floodColor == color)
    {
      return false;
    }
    _floodColor = color;
    return true;
  }

  bool FEFlood::SetFloodOpacity(float floodOpacity) noexcept
  {
    if (_floodOpacity == floodOpacity)
    {
      return false;
    }
    _floodOpacity = floodOpacity;
    return true;
  }

  FloatRect FEFlood::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                        const FloatRect &primitiveSubregion) const noexcept
  {
    return filter.MaxEffectRect(primitiveSubregion);
  }

  TextStream &FEFlood::ExternalRepresentation(TextStream &ts,
                                              FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feFlood";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" flood-color=\"" << SerialisationForRenderTreeAsText(FloodColor()) << u8'"';
    ts << u8" flood-opacity=\"" << FloodOpacity() << u8'"';
    ts << u8"]\n";
    return ts;
  }
}
