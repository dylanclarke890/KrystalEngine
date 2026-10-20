#pragma once

#include "Krystal.Booey/Graphics/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Image;
  class RenderElement;

  /// @brief The image a `RenderImage` shows: a cached image its element loaded, or (in a subclass) a style
  /// image from generated content.
  class RenderImageResource
  {
    KRYS_NON_COPY_MOVABLE(RenderImageResource);

  private:
    RenderElement *_renderer {nullptr};
    CachedResourceHandle<CachedImage> _cachedImage;
    bool _cachedImageRemoveClientIsNeeded {true};

  public:
    RenderImageResource() noexcept;
    virtual ~RenderImageResource() noexcept;

    virtual void Initialize(RenderElement &renderer) noexcept
    {
      Initialize(renderer, nullptr);
    }
    virtual void Shutdown() noexcept;

    void SetCachedImage(CachedResourceHandle<CachedImage> &&newImage) noexcept;
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept
    {
      return _cachedImage.get();
    }

    void ResetAnimation() noexcept;

    KRYS_NODISCARD virtual RefPtr<Image> GetImage(const IntSize &size = {}) const noexcept;
    KRYS_NODISCARD virtual bool CurrentFrameIsComplete() const noexcept;
    KRYS_NODISCARD virtual bool ErrorOccurred() const noexcept
    {
      return _cachedImage && _cachedImage->ErrorOccurred();
    }

    virtual void SetContainerContext(const IntSize &, const url::URL &) noexcept;

    KRYS_NODISCARD virtual bool ImageHasRelativeWidth() const noexcept
    {
      return _cachedImage && _cachedImage->ImageHasRelativeWidth();
    }
    KRYS_NODISCARD virtual bool ImageHasRelativeHeight() const noexcept
    {
      return _cachedImage && _cachedImage->ImageHasRelativeHeight();
    }

    KRYS_NODISCARD LayoutSize ImageSize(float multiplier) const noexcept
    {
      return ImageSize(multiplier, CachedImage::UsedSize);
    }
    KRYS_NODISCARD LayoutSize IntrinsicSize(float multiplier) const noexcept
    {
      return ImageSize(multiplier, CachedImage::IntrinsicSize);
    }

    KRYS_NODISCARD virtual WrappedImagePtr ImagePtr() const noexcept
    {
      return _cachedImage.get();
    }

  protected:
    KRYS_NODISCARD RenderElement *Renderer() const noexcept
    {
      return _renderer;
    }
    void Initialize(RenderElement &renderer, CachedImage *styleCachedImage) noexcept;

  private:
    KRYS_NODISCARD virtual LayoutSize ImageSize(float multiplier, CachedImage::SizeType type) const noexcept;
  };
}
