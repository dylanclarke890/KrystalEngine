#include "Krystal.Booey/Graphics/ImageDecoder.hpp"
#include "Krystal.Booey/Graphics/ImageFrame.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"

namespace krys::boo
{
  RefPtr<ImageDecoder> ImageDecoder::Create(const SharedBuffer &data, utf8_stringview mimeType,
                                            AlphaOption alphaOption,
                                            GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
  {
    (void)mimeType;
    return ScalableImageDecoder::Create(data, alphaOption, gammaAndColorProfileOption);
  }

  ImageDecoder::ImageDecoder() noexcept = default;

  ImageDecoder::~ImageDecoder() noexcept = default;

  bool ImageDecoder::SupportsMediaType(MediaType type) noexcept
  {
    return ScalableImageDecoder::SupportsMediaType(type);
  }

  bool ImageDecoder::FetchFrameMetaDataAtIndex(size_t index, SubsamplingLevel subsamplingLevel,
                                               const DecodingOptions &options,
                                               ImageFrame &frame) const noexcept
  {
    if (options.HasSizeForDrawing())
    {
      krys_debug_assert(frame.HasNativeImage());
      frame._size = frame.GetNativeImage()->Size();
    }
    else
    {
      frame._size = FrameSizeAtIndex(index, subsamplingLevel);
    }

    frame._densityCorrectedSize = FrameDensityCorrectedSizeAtIndex(index);
    frame._subsamplingLevel = subsamplingLevel;
    frame._hasAlpha = FrameHasAlphaAtIndex(index);
    frame._orientation = FrameOrientationAtIndex(index);
    frame._decodingStatus =
      FrameIsCompleteAtIndex(index) ? DecodingStatus::Complete : DecodingStatus::Partial;
    return true;
  }
}
