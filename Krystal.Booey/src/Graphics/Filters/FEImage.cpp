#include "Krystal.Booey/Graphics/Filters/FEImage.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEImage> FEImage::Create(SourceImage &&sourceImage, const FloatRect &sourceImageRect,
                               const svg::SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept
  {
    return AdoptRef(*new FEImage(krys::move(sourceImage), sourceImageRect, preserveAspectRatio));
  }

  FEImage::FEImage(SourceImage &&sourceImage, const FloatRect &sourceImageRect,
                   const svg::SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept
      : FilterEffect(Type::FEImage), _sourceImage(krys::move(sourceImage)), _sourceImageRect(sourceImageRect),
        _preserveAspectRatio(preserveAspectRatio)
  {
  }

  bool FEImage::operator==(const FEImage &other) const noexcept
  {
    return FilterEffect::operator==(other) && _sourceImage == other._sourceImage
           && _sourceImageRect == other._sourceImageRect
           && _preserveAspectRatio == other._preserveAspectRatio;
  }

  FloatRect FEImage::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                        const FloatRect &primitiveSubregion) const noexcept
  {
    if (_sourceImage.NativeImageIfExists())
    {
      auto imageRect = primitiveSubregion;
      auto srcRect = _sourceImageRect;
      _preserveAspectRatio.TransformRect(imageRect, srcRect);
      return filter.ClipToMaxEffectRect(imageRect, primitiveSubregion);
    }

    if (_sourceImage.ImageBufferIfExists())
    {
      return filter.MaxEffectRect(primitiveSubregion);
    }

    krys_debug_assert(false);
    return FloatRect();
  }

  TextStream &FEImage::ExternalRepresentation(TextStream &ts,
                                              FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feImage";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" image-size=\"" << _sourceImageRect.Width() << u8'x' << _sourceImageRect.Height() << u8'"';
    // FIXME: should this dump also object returned by FEImage::image() ?

    ts << u8"]\n";
    return ts;
  }
}
