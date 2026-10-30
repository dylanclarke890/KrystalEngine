#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class CachedImageClient;
  class LayoutSize;
  class RenderObject;
}

namespace krys::boo::svg
{
  class SVGImage;
  class SVGImageForContainer;

  /// @brief The images a cached SVG image is drawn as, one per client, each at its client's container size.
  class SVGImageCache
  {
  private:
    using ImageForContainerMap = HashMap<const CachedImageClient *, RefPtr<SVGImageForContainer>>;

    WeakPtr<SVGImage> _svgImage;
    ImageForContainerMap _imageForContainerMap;

  public:
    explicit SVGImageCache(SVGImage *) noexcept;
    ~SVGImageCache() noexcept;

    void RemoveClientFromCache(const CachedImageClient *) noexcept;

    void SetContainerContextForClient(const CachedImageClient &, const LayoutSize &, float,
                                      const url::URL &) noexcept;
    KRYS_NODISCARD FloatSize ImageSizeForRenderer(const RenderObject *) const noexcept;

    KRYS_NODISCARD Image *ImageForRenderer(const RenderObject *) const noexcept;

  private:
    KRYS_NODISCARD Image *FindImageForRenderer(const RenderObject *) const noexcept;
  };
}
