#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/RenderingResourceIdentifier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class ImageBuffer;
  class NativeImage;

  /// @brief An image a pattern tiles: a native image or an image buffer.
  class SourceImage
  {
  public:
    using ImageVariant = Variant<Ref<NativeImage>, Ref<ImageBuffer>>;

  private:
    ImageVariant _imageVariant;
    mutable Maybe<ImageVariant> _transformedImageVariant;

  public:
    SourceImage(ImageVariant &&) noexcept;

    SourceImage(const SourceImage &) noexcept;
    SourceImage(SourceImage &&) noexcept;
    SourceImage &operator=(const SourceImage &) noexcept;
    SourceImage &operator=(SourceImage &&) noexcept;
    ~SourceImage() noexcept;

    KRYS_NODISCARD bool operator==(const SourceImage &) const noexcept;

    KRYS_NODISCARD NativeImage *NativeImageIfExists() const noexcept;
    KRYS_NODISCARD NativeImage *GetNativeImage() const noexcept;

    KRYS_NODISCARD ImageBuffer *ImageBufferIfExists() const noexcept;
    KRYS_NODISCARD ImageBuffer *GetImageBuffer() const noexcept;

    KRYS_NODISCARD RenderingResourceIdentifier ImageIdentifier() const noexcept;
    KRYS_NODISCARD IntSize Size() const noexcept;
  };
}
