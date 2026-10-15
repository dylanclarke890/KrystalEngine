#include "Krystal.Booey/Style/Values/Masking/MaskLayer.hpp"

namespace krys::boo::style
{
  MaskLayer::MaskLayer() noexcept
      : _image(MaskLayer::InitialImage()), _positionX(MaskLayer::InitialPositionX()),
        _positionY(MaskLayer::InitialPositionY()), _size(MaskLayer::InitialSize()),
        _repeat(MaskLayer::InitialRepeat()), _clip(static_cast<uint8>(MaskLayer::InitialClip())),
        _origin(static_cast<uint8>(MaskLayer::InitialOrigin())),
        _composite(static_cast<uint8>(MaskLayer::InitialComposite())),
        _maskMode(static_cast<uint8>(MaskLayer::InitialMaskMode())),
        _clipMax(static_cast<uint8>(MaskLayer::InitialClip()))
  {
  }

  MaskLayer::MaskLayer(ImageOrNone &&image) noexcept : MaskLayer {}
  {
    SetImage(krys::move(image));
  }

  MaskLayer::MaskLayer(css::keywords::None keyword) noexcept : MaskLayer {ImageOrNone {keyword}}
  {
  }

  MaskLayer::MaskLayer(RefPtr<StyleImage> &&image) noexcept : MaskLayer {ImageOrNone {krys::move(image)}}
  {
  }

  bool MaskLayer::operator==(const MaskLayer &other) const noexcept
  {
    // NOTE: Default operator== is not used due to exclusion of _clipMax.
    return AllOfCoordinatedValueListProperties<MaskLayer>(
      [this, &other]<auto propertyID>()
      {
        using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyID>;
        return PropertyAccessor {*this} == PropertyAccessor {other};
      });
  }

  // MARK: - Blending

  bool Blending<MaskLayer>::CanBlend(const MaskLayer &a, const MaskLayer &b) noexcept
  {
    return a.GetSize().HasSameType(b.GetSize());
  }
}
