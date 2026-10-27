#include "Krystal.Booey/Graphics/Filters/FETile.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FETile> FETile::Create(const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FETile(colorSpace));
  }

  FETile::FETile(const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FETile, colorSpace)
  {
  }

  FloatRect FETile::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                       const FloatRect &primitiveSubregion) const noexcept
  {
    return filter.MaxEffectRect(primitiveSubregion);
  }

  bool FETile::ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept
  {
    return inputs[0]->IsAlphaImage();
  }

  TextStream &FETile::ExternalRepresentation(TextStream &ts,
                                             FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feTile";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8"]\n";
    return ts;
  }
}
