#include "Krystal.Booey/Graphics/Filters/FEOffset.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEOffset> FEOffset::Create(float dx, float dy, const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEOffset(dx, dy, colorSpace));
  }

  FEOffset::FEOffset(float dx, float dy, const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEOffset, colorSpace), _dx(dx), _dy(dy)
  {
  }

  bool FEOffset::operator==(const FEOffset &other) const noexcept
  {
    return FilterEffect::operator==(other) && _dx == other._dx && _dy == other._dy;
  }

  bool FEOffset::SetDx(float dx) noexcept
  {
    if (_dx == dx)
    {
      return false;
    }
    _dx = dx;
    return true;
  }

  bool FEOffset::SetDy(float dy) noexcept
  {
    if (_dy == dy)
    {
      return false;
    }
    _dy = dy;
    return true;
  }

  FloatRect FEOffset::CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                         const FloatRect &primitiveSubregion) const noexcept
  {
    auto imageRect = inputImageRects[0];
    imageRect.Move(filter.ResolvedSize({_dx, _dy}));
    return filter.ClipToMaxEffectRect(imageRect, primitiveSubregion);
  }

  IntOutsets FEOffset::CalculateOutsets(const FloatSize &offset) noexcept
  {
    auto adjustedOffset = ExpandedIntSize(offset);

    IntOutsets outsets;
    if (adjustedOffset.Height() < 0)
    {
      outsets.SetTop(-adjustedOffset.Height());
    }
    else
    {
      outsets.SetBottom(adjustedOffset.Height());
    }
    if (adjustedOffset.Width() < 0)
    {
      outsets.SetLeft(-adjustedOffset.Width());
    }
    else
    {
      outsets.SetRight(adjustedOffset.Width());
    }

    return outsets;
  }

  bool FEOffset::ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept
  {
    return inputs[0]->IsAlphaImage();
  }

  TextStream &FEOffset::ExternalRepresentation(TextStream &ts,
                                               FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feOffset";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" dx=\"" << Dx() << u8"\" dy=\"" << Dy() << u8'"';
    ts << u8"]\n";
    return ts;
  }
}
