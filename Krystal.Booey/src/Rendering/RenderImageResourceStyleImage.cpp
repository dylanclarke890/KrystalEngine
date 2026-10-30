#include "Krystal.Booey/Rendering/RenderImageResourceStyleImage.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/NullGraphicsContext.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderImageResourceStyleImage::RenderImageResourceStyleImage(StyleImage &styleImage) noexcept
      : _styleImage(ShareRef(styleImage))
  {
  }

  void RenderImageResourceStyleImage::Initialize(RenderElement &renderer) noexcept
  {
    RenderImageResource::Initialize(renderer,
                                    _styleImage->HasCachedImage() ? _styleImage->GetCachedImage() : nullptr);
    _styleImage->AddClient(renderer);
  }

  void RenderImageResourceStyleImage::Shutdown() noexcept
  {
    RenderImageResource::Shutdown();
    if (auto *renderer = Renderer())
    {
      _styleImage->RemoveClient(*renderer);
    }
  }

  RefPtr<Image> RenderImageResourceStyleImage::GetImage(const IntSize &size) const noexcept
  {
    // Generated content may trigger calls to image() while we're still pending, don't assert but gracefully
    // exit.
    if (_styleImage->IsPending())
    {
      return ShareRefPtr(&Image::NullImage());
    }
    if (auto image = _styleImage->GetImage(Renderer(), FloatSize(size), NullGraphicsContext()))
    {
      return image;
    }
    return ShareRefPtr(&Image::NullImage());
  }

  void RenderImageResourceStyleImage::SetContainerContext(const IntSize &size, const url::URL &) noexcept
  {
    if (auto *renderer = Renderer())
    {
      _styleImage->SetContainerContextForRenderer(*renderer, FloatSize(size), renderer->Style().UsedZoom());
    }
  }
}
