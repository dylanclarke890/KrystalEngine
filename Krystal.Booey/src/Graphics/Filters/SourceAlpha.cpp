#include "Krystal.Booey/Graphics/Filters/SourceAlpha.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<SourceAlpha> SourceAlpha::Create(const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new SourceAlpha(colorSpace));
  }

  SourceAlpha::SourceAlpha(const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::SourceAlpha, colorSpace)
  {
  }

  TextStream &SourceAlpha::ExternalRepresentation(TextStream &ts, FilterRepresentation) const noexcept
  {
    ts << Indent << u8"[SourceAlpha]\n";
    return ts;
  }
}
