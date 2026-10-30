#include "Krystal.Booey/Rendering/Style/StyleMultiImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCachedImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  StyleMultiImage::StyleMultiImage(Type type) noexcept : StyleImage {type}
  {
  }

  StyleMultiImage::~StyleMultiImage() noexcept = default;

  bool StyleMultiImage::Equals(const StyleMultiImage &other) const noexcept
  {
    return !_isPending && !other._isPending && ArePointingToEqualData(_selectedImage, other._selectedImage);
  }

  void StyleMultiImage::Load(CachedResourceLoader &loader, const ResourceLoaderOptions &options) noexcept
  {
    krys_debug_assert(_isPending);
    krys_debug_assert(loader.GetDocument());

    _isPending = false;

    auto bestFitImage = SelectBestFitImage(*loader.GetDocument());

    krys_debug_assert(Is<StyleCachedImage>(*bestFitImage.Image)
                      || Is<StyleGeneratedImage>(*bestFitImage.Image));

    if (Is<StyleGeneratedImage>(*bestFitImage.Image))
    {
      _selectedImage = bestFitImage.Image;
      _selectedImage->Load(loader, options);
      return;
    }

    if (auto *styleCachedImage = DynamicDowncast<StyleCachedImage>(bestFitImage.Image.get()))
    {
      if (styleCachedImage->ImageScaleFactor() == bestFitImage.ScaleFactor)
      {
        _selectedImage = bestFitImage.Image;
      }
      else
      {
        _selectedImage =
          StyleCachedImage::CopyOverridingScaleFactor(*styleCachedImage, bestFitImage.ScaleFactor);
      }

      if (_selectedImage->IsPending())
      {
        _selectedImage->Load(loader, options);
      }
      return;
    }
  }

  CachedImage *StyleMultiImage::GetCachedImage() const noexcept
  {
    if (!_selectedImage)
    {
      return nullptr;
    }
    return _selectedImage->GetCachedImage();
  }

  WrappedImagePtr StyleMultiImage::Data() const noexcept
  {
    if (!_selectedImage)
    {
      return nullptr;
    }
    return _selectedImage->Data();
  }

  bool StyleMultiImage::CanRender(const RenderElement *renderer, float multiplier) const noexcept
  {
    return _selectedImage && _selectedImage->CanRender(renderer, multiplier);
  }

  bool StyleMultiImage::IsPending() const noexcept
  {
    return _isPending;
  }

  bool StyleMultiImage::IsLoaded(const RenderElement *renderer) const noexcept
  {
    return _selectedImage && _selectedImage->IsLoaded(renderer);
  }

  FloatSize StyleMultiImage::ImageSize(const RenderElement *renderer, float multiplier) const noexcept
  {
    if (!_selectedImage)
    {
      return {};
    }
    return _selectedImage->ImageSize(renderer, multiplier);
  }

  void StyleMultiImage::ComputeIntrinsicDimensions(const RenderElement *element, float &intrinsicWidth,
                                                   float &intrinsicHeight, FloatSize &intrinsicRatio) noexcept
  {
    if (!_selectedImage)
    {
      return;
    }
    _selectedImage->ComputeIntrinsicDimensions(element, intrinsicWidth, intrinsicHeight, intrinsicRatio);
  }

  void StyleMultiImage::SetContainerContextForRenderer(const RenderElement &renderer,
                                                       const FloatSize &containerSize,
                                                       float containerZoom) noexcept
  {
    if (!_selectedImage)
    {
      return;
    }
    _selectedImage->SetContainerContextForRenderer(renderer, containerSize, containerZoom);
  }

  void StyleMultiImage::AddClient(RenderElement &renderer) noexcept
  {
    if (!_selectedImage)
    {
      return;
    }
    _selectedImage->AddClient(renderer);
  }

  void StyleMultiImage::RemoveClient(RenderElement &renderer) noexcept
  {
    if (!_selectedImage)
    {
      return;
    }
    _selectedImage->RemoveClient(renderer);
  }

  bool StyleMultiImage::HasClient(RenderElement &renderer) const noexcept
  {
    if (!_selectedImage)
    {
      return false;
    }
    return _selectedImage->HasClient(renderer);
  }

  RefPtr<Image> StyleMultiImage::GetImage(const RenderElement *renderer, const FloatSize &size,
                                          const GraphicsContext &destinationContext,
                                          bool isForFirstLine) const noexcept
  {
    if (!_selectedImage)
    {
      return nullptr;
    }
    return _selectedImage->GetImage(renderer, size, destinationContext, isForFirstLine);
  }

  bool StyleMultiImage::KnownToBeOpaque(const RenderElement &renderer) const noexcept
  {
    return _selectedImage && _selectedImage->KnownToBeOpaque(renderer);
  }

  bool StyleMultiImage::ErrorOccurred() const noexcept
  {
    return _selectedImage && _selectedImage->ErrorOccurred();
  }

  bool StyleMultiImage::ImageHasRelativeWidth() const noexcept
  {
    return _selectedImage && _selectedImage->ImageHasRelativeWidth();
  }

  bool StyleMultiImage::ImageHasRelativeHeight() const noexcept
  {
    return _selectedImage && _selectedImage->ImageHasRelativeHeight();
  }

  bool StyleMultiImage::UsesImageContainerSize() const noexcept
  {
    return _selectedImage && _selectedImage->UsesImageContainerSize();
  }

  float StyleMultiImage::ImageScaleFactor() const noexcept
  {
    if (!_selectedImage)
    {
      return 1;
    }
    return _selectedImage->ImageScaleFactor();
  }
}
