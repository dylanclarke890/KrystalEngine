#include "Krystal.Booey/Graphics/Filters/FEMerge.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEMerge> FEMerge::Create(uint32 numberOfEffectInputs, const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEMerge(numberOfEffectInputs, colorSpace));
  }

  FEMerge::FEMerge(uint32 numberOfEffectInputs, const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEMerge, colorSpace), _numberOfEffectInputs(numberOfEffectInputs)
  {
  }

  bool FEMerge::operator==(const FEMerge &other) const noexcept
  {
    return FilterEffect::operator==(other) && _numberOfEffectInputs == other._numberOfEffectInputs;
  }

  TextStream &FEMerge::ExternalRepresentation(TextStream &ts,
                                              FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feMerge";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" mergeNodes=\"" << _numberOfEffectInputs << u8'"';
    ts << u8"]\n";
    return ts;
  }
}
