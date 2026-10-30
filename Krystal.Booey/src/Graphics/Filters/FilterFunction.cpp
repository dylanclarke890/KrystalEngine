#include "Krystal.Booey/Graphics/Filters/FilterFunction.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  RefPtr<FilterImage> FilterFunction::Apply(const Filter &, FilterImage &, FilterResults &) noexcept
  {
    return nullptr;
  }

  utf8_stringview FilterFunction::FilterName(Type type) noexcept
  {
    switch (type)
    {
      case Type::CSSFilterRenderer:   return u8"CSSFilterRenderer";
      case Type::SVGFilterRenderer:   return u8"SVGFilterRenderer";

      case Type::FEBlend:             return u8"FEBlend";
      case Type::FEColorMatrix:       return u8"FEColorMatrix";
      case Type::FEComponentTransfer: return u8"FEComponentTransfer";
      case Type::FEComposite:         return u8"FEComposite";
      case Type::FEConvolveMatrix:    return u8"FEConvolveMatrix";
      case Type::FEDiffuseLighting:   return u8"FEDiffuseLighting";
      case Type::FEDisplacementMap:   return u8"FEDisplacementMap";
      case Type::FEDropShadow:        return u8"FEDropShadow";
      case Type::FEFlood:             return u8"FEFlood";
      case Type::FEGaussianBlur:      return u8"FEGaussianBlur";
      case Type::FEImage:             return u8"FEImage";
      case Type::FEMerge:             return u8"FEMerge";
      case Type::FEMorphology:        return u8"FEMorphology";
      case Type::FEOffset:            return u8"FEOffset";
      case Type::FESpecularLighting:  return u8"FESpecularLighting";
      case Type::FETile:              return u8"FETile";
      case Type::FETurbulence:        return u8"FETurbulence";

      case Type::SourceAlpha:         return u8"SourceAlpha";
      case Type::SourceGraphic:       return u8"SourceGraphic";
    }
    krys_debug_assert(false);
    return u8"";
  }

  TextStream &operator<<(TextStream &ts, const FilterFunction &filterFunction) noexcept
  {
    return filterFunction.ExternalRepresentation(ts, FilterRepresentation::Debugging);
  }
}
