#include "Krystal.Booey/Rendering/RenderImageResource.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderImageResource::RenderImageResource() noexcept = default;

  RenderImageResource::~RenderImageResource() noexcept = default;

  void RenderImageResource::Initialize(RenderElement &renderer, CachedImage *styleCachedImage) noexcept
  {
    krys_debug_assert(!_renderer);
    krys_debug_assert(!_cachedImage);
    _renderer = &renderer;
    _cachedImage = styleCachedImage;
    _cachedImageRemoveClientIsNeeded = !styleCachedImage;
  }

  void RenderImageResource::Shutdown() noexcept
  {
    GetImage()->StopAnimation();
    SetCachedImage(nullptr);
  }

  void RenderImageResource::SetCachedImage(CachedResourceHandle<CachedImage> &&newImage) noexcept
  {
    if (_cachedImage == newImage)
    {
      return;
    }

    if (_cachedImage && _renderer && _cachedImageRemoveClientIsNeeded)
    {
      _cachedImage->RemoveClient(_renderer->GetCachedImageClient());
    }
    if (!_renderer)
    {
      return;
    }
    _cachedImage = krys::move(newImage);
    _cachedImageRemoveClientIsNeeded = true;
    if (!_cachedImage)
    {
      return;
    }

    _cachedImage->AddClient(_renderer->GetCachedImageClient());
    if (_cachedImage->ErrorOccurred())
    {
      _renderer->ImageChanged(_cachedImage.get());
    }
  }

  void RenderImageResource::ResetAnimation() noexcept
  {
    if (!_cachedImage)
    {
      return;
    }

    GetImage()->ResetAnimation();

    if (_renderer && !_renderer->NeedsLayout())
    {
      _renderer->Repaint();
    }
  }

  RefPtr<Image> RenderImageResource::GetImage(const IntSize &) const noexcept
  {
    if (!_cachedImage)
    {
      return ShareRefPtr(&Image::NullImage());
    }
    if (auto *image = _cachedImage->ImageForRenderer(_renderer))
    {
      return ShareRefPtr(image);
    }
    return ShareRefPtr(&Image::NullImage());
  }

  bool RenderImageResource::CurrentFrameIsComplete() const noexcept
  {
    if (!_cachedImage)
    {
      return false;
    }

    return _cachedImage->CurrentFrameIsComplete(_renderer);
  }

  void RenderImageResource::SetContainerContext(const IntSize &imageContainerSize,
                                                const url::URL &imageURL) noexcept
  {
    if (!_cachedImage || !_renderer)
    {
      return;
    }
    _cachedImage->SetContainerContextForClient(_renderer->GetCachedImageClient(),
                                               LayoutSize(imageContainerSize), _renderer->Style().UsedZoom(),
                                               imageURL);
  }

  LayoutSize RenderImageResource::ImageSize(float multiplier, CachedImage::SizeType type) const noexcept
  {
    if (!_cachedImage)
    {
      return {};
    }
    LayoutSize size = _cachedImage->ImageSizeForRenderer(_renderer, multiplier, type);
    if (auto *renderImage = DynamicDowncast<RenderImage>(_renderer))
    {
      size.Scale(renderImage->ImageDevicePixelRatio());
    }
    return size;
  }
}
