#include "Krystal.Booey/Graphics/ImageFrame.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  ImageFrame::ImageFrame(Ref<NativeImage> &&nativeImage) noexcept
  {
    _size = nativeImage->Size();
    _hasAlpha = nativeImage->HasAlpha();
    _source.Image = krys::move(nativeImage);
  }

  ImageFrame::~ImageFrame() noexcept
  {
    (void)ClearImage();
  }

  const ImageFrame &ImageFrame::DefaultFrame() noexcept
  {
    static NeverDestroyed<ImageFrame> sharedInstance;
    return sharedInstance;
  }

  void ImageFrame::SetDecodingStatus(DecodingStatus decodingStatus) noexcept
  {
    krys_debug_assert(decodingStatus != DecodingStatus::Decoding);
    _decodingStatus = decodingStatus;
  }

  DecodingStatus ImageFrame::GetDecodingStatus() const noexcept
  {
    krys_debug_assert(_decodingStatus != DecodingStatus::Decoding);
    return _decodingStatus;
  }

  uint32 ImageFrame::ClearImage() noexcept
  {
    if (!_source.HasNativeImage())
    {
      return 0;
    }

    _source.Clear();
    return SizeInBytes();
  }

  uint32 ImageFrame::Clear() noexcept
  {
    uint32 frameBytes = ClearImage();
    *this = ImageFrame();
    return frameBytes;
  }

  bool ImageFrame::HasNativeImage(SubsamplingLevel subsamplingLevel) const noexcept
  {
    return _source.HasNativeImage() && subsamplingLevel >= _subsamplingLevel;
  }

  bool ImageFrame::HasFullSizeNativeImage(SubsamplingLevel subsamplingLevel) const noexcept
  {
    return _source.HasFullSizeNativeImage() && subsamplingLevel >= _subsamplingLevel;
  }

  bool
    ImageFrame::HasDecodedNativeImageCompatibleWithOptions(const DecodingOptions &options,
                                                           SubsamplingLevel subsamplingLevel) const noexcept
  {
    return IsComplete() && _source.HasDecodedNativeImageCompatibleWithOptions(options)
           && subsamplingLevel >= _subsamplingLevel;
  }
}
