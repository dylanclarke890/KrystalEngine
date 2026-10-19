#include "Krystal.Booey/Style/Values/Backgrounds/BackgroundLayer.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"

namespace krys::boo::style
{
  bool BackgroundLayer::HasOpaqueImage(const RenderElement &renderer) const noexcept
  {
    auto image = _image.TryStyleImage();
    if (!image)
    {
      return false;
    }

    if (GetComposite() == CompositeOperator::Clear || GetComposite() == CompositeOperator::Copy)
    {
      return true;
    }

    return GetBlendMode() == BlendMode::Normal && GetComposite() == CompositeOperator::SourceOver
           && image->KnownToBeOpaque(renderer);
  }

  BackgroundLayer::BackgroundLayer() noexcept
      : _image(BackgroundLayer::InitialImage()), _positionX(BackgroundLayer::InitialPositionX()),
        _positionY(BackgroundLayer::InitialPositionY()), _size(BackgroundLayer::InitialSize()),
        _repeat(BackgroundLayer::InitialRepeat()),
        _attachment(static_cast<uint8>(BackgroundLayer::InitialAttachment())),
        _clip(static_cast<uint8>(BackgroundLayer::InitialClip())),
        _origin(static_cast<uint8>(BackgroundLayer::InitialOrigin())),
        _blendMode(static_cast<uint8>(BackgroundLayer::InitialBlendMode())),
        _clipMax(static_cast<uint8>(BackgroundLayer::InitialClip()))
  {
  }

  BackgroundLayer::BackgroundLayer(ImageOrNone &&image) noexcept : BackgroundLayer {}
  {
    SetImage(krys::move(image));
  }

  BackgroundLayer::BackgroundLayer(css::keywords::None keyword) noexcept
      : BackgroundLayer {ImageOrNone {keyword}}
  {
  }

  BackgroundLayer::BackgroundLayer(RefPtr<StyleImage> &&image) noexcept
      : BackgroundLayer {ImageOrNone {krys::move(image)}}
  {
  }

  bool BackgroundLayer::operator==(const BackgroundLayer &other) const noexcept
  {
    // NOTE: Default operator== is not used due to exclusion of _clipMax.
    return AllOfCoordinatedValueListProperties<BackgroundLayer>(
      [this, &other]<auto propertyID>()
      {
        using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyID>;
        return PropertyAccessor {*this} == PropertyAccessor {other};
      });
  }

  // MARK: - Blending

  bool Blending<BackgroundLayer>::CanBlend(const BackgroundLayer &a, const BackgroundLayer &b) noexcept
  {
    return a.GetSize().HasSameType(b.GetSize());
  }
}
