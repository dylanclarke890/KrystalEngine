#include "Krystal.Booey/Graphics/Filters/FEDisplacementMap.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEDisplacementMap> FEDisplacementMap::Create(ChannelSelectorType xChannelSelector,
                                                   ChannelSelectorType yChannelSelector, float scale,
                                                   const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEDisplacementMap(xChannelSelector, yChannelSelector, scale, colorSpace));
  }

  FEDisplacementMap::FEDisplacementMap(ChannelSelectorType xChannelSelector,
                                       ChannelSelectorType yChannelSelector, float scale,
                                       const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEDisplacementMap, colorSpace), _xChannelSelector(xChannelSelector),
        _yChannelSelector(yChannelSelector), _scale(scale)
  {
  }

  bool FEDisplacementMap::operator==(const FEDisplacementMap &other) const noexcept
  {
    return FilterEffect::operator==(other) && _xChannelSelector == other._xChannelSelector
           && _yChannelSelector == other._yChannelSelector && _scale == other._scale;
  }

  bool FEDisplacementMap::SetXChannelSelector(ChannelSelectorType xChannelSelector) noexcept
  {
    if (_xChannelSelector == xChannelSelector)
    {
      return false;
    }
    _xChannelSelector = xChannelSelector;
    return true;
  }

  bool FEDisplacementMap::SetYChannelSelector(ChannelSelectorType yChannelSelector) noexcept
  {
    if (_yChannelSelector == yChannelSelector)
    {
      return false;
    }
    _yChannelSelector = yChannelSelector;
    return true;
  }

  bool FEDisplacementMap::SetScale(float scale) noexcept
  {
    if (_scale == scale)
    {
      return false;
    }
    _scale = scale;
    return true;
  }

  FloatRect FEDisplacementMap::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                                  const FloatRect &primitiveSubregion) const noexcept
  {
    return filter.MaxEffectRect(primitiveSubregion);
  }

  IntOutsets FEDisplacementMap::CalculateOutsets(const FloatSize &maxDisplacement) noexcept
  {
    auto intDisplacement = ExpandedIntSize(maxDisplacement);
    return {intDisplacement.Height(), intDisplacement.Width(), intDisplacement.Height(),
            intDisplacement.Width()};
  }

  const DestinationColorSpace &
    FEDisplacementMap::ResultColorSpace(Span<const Ref<FilterImage>> inputs) const noexcept
  {
    // Spec: The 'color-interpolation-filters' property only applies to the 'in2' source image
    // and does not apply to the 'in' source image. The 'in' source image must remain in its
    // current color space.
    // The result is in that same color space because it is a displacement of the 'in' image.
    return inputs[0]->ColorSpace();
  }

  void FEDisplacementMap::TransformInputsColorSpace(Span<const Ref<FilterImage>> inputs) const noexcept
  {
    // Do not transform the first primitive input, as per the spec.
    krys_debug_assert(inputs.size() == 2);
    inputs[1]->TransformToColorSpace(OperatingColorSpace());
  }

  namespace
  {
    TextStream &operator<<(TextStream &ts, const ChannelSelectorType &type) noexcept
    {
      switch (type)
      {
        case ChannelSelectorType::CHANNEL_UNKNOWN: ts << u8"UNKNOWN"; break;
        case ChannelSelectorType::CHANNEL_R:       ts << u8"RED"; break;
        case ChannelSelectorType::CHANNEL_G:       ts << u8"GREEN"; break;
        case ChannelSelectorType::CHANNEL_B:       ts << u8"BLUE"; break;
        case ChannelSelectorType::CHANNEL_A:       ts << u8"ALPHA"; break;
      }
      return ts;
    }
  }

  TextStream &FEDisplacementMap::ExternalRepresentation(TextStream &ts,
                                                        FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feDisplacementMap";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" scale=\"" << _scale << u8'"';
    ts << u8" xChannelSelector=\"" << _xChannelSelector << u8'"';
    ts << u8" yChannelSelector=\"" << _yChannelSelector << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
