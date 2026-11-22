#pragma once

#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Image/ImageDecoder.hpp"

namespace krys::img
{
  /// @brief Decodes JPEG images with libjpeg-turbo, as their data arrives: progressive JPEGs show each scan
  /// as it completes. Embedded ICC profiles (including CMYK ones) convert the pixels to sRGB, and EXIF gives
  /// the orientation and density-corrected size.
  class JPEGImageDecoder final : public ImageDecoder
  {
    class Reader;

  private:
    // libjpeg's state while the image decodes; released once it's complete or has failed.
    UniquePtr<Reader> _reader;

  public:
    KRYS_NODISCARD static Ref<ImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new JPEGImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~JPEGImageDecoder() noexcept override;

    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept override
    {
      return u8"jpg";
    }

    ImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept override;

  private:
    JPEGImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;

    void TryDecodeSize(bool allDataReceived) noexcept override
    {
      Decode(true, allDataReceived);
    }

    /// @brief Decodes what's arrived: only up to the size if @p sizeOnly, or else the whole image.
    void Decode(bool sizeOnly, bool allDataReceived) noexcept;
  };
}
