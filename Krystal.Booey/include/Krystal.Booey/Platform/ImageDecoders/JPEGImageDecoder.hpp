#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class JPEGImageReader;

  // This class decodes the JPEG image format.
  // @note libjpeg reports errors with longjmp, which on Windows unwinds as an exception would, so the
  // functions between the reader's setjmp and libjpeg's calls (writing scanlines) can't be noexcept.
  // FIXME: ICC colour profiles (LCMS), once there's colour management.
  class JPEGImageDecoder final : public ScalableImageDecoder
  {
  private:
    UniquePtr<JPEGImageReader> _reader;

  public:
    KRYS_NODISCARD static Ref<ScalableImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new JPEGImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~JPEGImageDecoder() noexcept override;

    // ScalableImageDecoder
    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept override
    {
      return u8"jpg";
    }
    ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept override;
    // CAUTION: setFailed() deletes |m_reader|.  Be careful to avoid
    // accessing deleted memory, especially when calling this from inside
    // JPEGImageReader!
    bool SetFailed() noexcept override;

    bool OutputScanlines();
    void JpegComplete();

    void SetOrientation(ImageOrientation orientation) noexcept
    {
      _orientation = orientation;
    }

  private:
    JPEGImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;
    void TryDecodeSize(bool allDataReceived) noexcept override
    {
      Decode(true, allDataReceived);
    }

    // Decodes the image.  If |onlySize| is true, stops decoding after
    // calculating the image size.  If decoding fails but there is no more
    // data coming, sets the "decode failure" flag.
    void Decode(bool onlySize, bool allDataReceived) noexcept;

    template <int TColorSpace>
    bool OutputScanlines(ScalableImageDecoderFrame &buffer);

    void Clear() noexcept;
  };
}
