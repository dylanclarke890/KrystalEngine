#include "Krystal.Booey/Graphics/Filters/FEBlend.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEBlend> FEBlend::Create(BlendMode mode, const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEBlend(mode, colorSpace));
  }

  FEBlend::FEBlend(BlendMode mode, const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEBlend, colorSpace), _mode(mode)
  {
  }

  bool FEBlend::operator==(const FEBlend &other) const noexcept
  {
    return FilterEffect::operator==(other) && _mode == other._mode;
  }

  bool FEBlend::SetBlendMode(BlendMode mode) noexcept
  {
    if (_mode == mode)
    {
      return false;
    }
    _mode = mode;
    return true;
  }

  TextStream &FEBlend::ExternalRepresentation(TextStream &ts,
                                              FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feBlend";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" mode=\"";
    if (_mode == BlendMode::Normal)
    {
      ts << u8"normal";
    }
    else
    {
      ts << CompositeOperatorName(CompositeOperator::SourceOver, _mode);
    }
    ts << u8"\"]\n";
    return ts;
  }
}
