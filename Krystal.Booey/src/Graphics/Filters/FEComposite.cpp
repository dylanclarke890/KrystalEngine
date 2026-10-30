#include "Krystal.Booey/Graphics/Filters/FEComposite.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEComposite> FEComposite::Create(const CompositeOperationType &type, float k1, float k2, float k3,
                                       float k4, const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEComposite(type, k1, k2, k3, k4, colorSpace));
  }

  FEComposite::FEComposite(const CompositeOperationType &type, float k1, float k2, float k3, float k4,
                           const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEComposite, colorSpace), _type(type), _k1(k1), _k2(k2), _k3(k3),
        _k4(k4)
  {
  }

  bool FEComposite::operator==(const FEComposite &other) const noexcept
  {
    return FilterEffect::operator==(other) && _type == other._type && _k1 == other._k1 && _k2 == other._k2
           && _k3 == other._k3 && _k4 == other._k4;
  }

  bool FEComposite::SetOperation(CompositeOperationType type) noexcept
  {
    if (_type == type)
    {
      return false;
    }
    _type = type;
    return true;
  }

  bool FEComposite::SetK1(float k1) noexcept
  {
    if (_k1 == k1)
    {
      return false;
    }
    _k1 = k1;
    return true;
  }

  bool FEComposite::SetK2(float k2) noexcept
  {
    if (_k2 == k2)
    {
      return false;
    }
    _k2 = k2;
    return true;
  }

  bool FEComposite::SetK3(float k3) noexcept
  {
    if (_k3 == k3)
    {
      return false;
    }
    _k3 = k3;
    return true;
  }

  bool FEComposite::SetK4(float k4) noexcept
  {
    if (_k4 == k4)
    {
      return false;
    }
    _k4 = k4;
    return true;
  }

  FloatRect FEComposite::CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                            const FloatRect &primitiveSubregion) const noexcept
  {
    switch (_type)
    {
      case CompositeOperationType::FECOMPOSITE_OPERATOR_IN:
      case CompositeOperationType::FECOMPOSITE_OPERATOR_ATOP:
        // For In and Atop the first FilterImage just influences the result of the
        // second FilterImage. So just use the rect of the second FilterImage here.
        return filter.ClipToMaxEffectRect(inputImageRects[1], primitiveSubregion);

      case CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC:
        // Arithmetic may influnce the entire filter primitive region. So we can't
        // optimize the paint region here.
        return filter.MaxEffectRect(primitiveSubregion);

      default:
        // Take the union of both input effects.
        return FilterEffect::CalculateImageRect(filter, inputImageRects, primitiveSubregion);
    }
  }

  namespace
  {
    TextStream &operator<<(TextStream &ts, const CompositeOperationType &type) noexcept
    {
      switch (type)
      {
        case CompositeOperationType::FECOMPOSITE_OPERATOR_UNKNOWN:    ts << u8"UNKNOWN"; break;
        case CompositeOperationType::FECOMPOSITE_OPERATOR_OVER:       ts << u8"OVER"; break;
        case CompositeOperationType::FECOMPOSITE_OPERATOR_IN:         ts << u8"IN"; break;
        case CompositeOperationType::FECOMPOSITE_OPERATOR_OUT:        ts << u8"OUT"; break;
        case CompositeOperationType::FECOMPOSITE_OPERATOR_ATOP:       ts << u8"ATOP"; break;
        case CompositeOperationType::FECOMPOSITE_OPERATOR_XOR:        ts << u8"XOR"; break;
        case CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC: ts << u8"ARITHMETIC"; break;
        case CompositeOperationType::FECOMPOSITE_OPERATOR_LIGHTER:    ts << u8"LIGHTER"; break;
      }
      return ts;
    }
  }

  TextStream &FEComposite::ExternalRepresentation(TextStream &ts,
                                                  FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feComposite";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" operation=\"" << _type << u8'"';
    if (_type == CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC)
    {
      ts << u8" k1=\"" << _k1 << u8"\" k2=\"" << _k2 << u8"\" k3=\"" << _k3 << u8"\" k4=\"" << _k4 << u8'"';
    }
    ts << u8"]\n";
    return ts;
  }
}
