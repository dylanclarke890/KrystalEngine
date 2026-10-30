#include "Krystal.Booey/Graphics/ImageSource.hpp"

namespace krys::boo
{
  Expected<Ref<NativeImage>, DecodingStatus>
    ImageSource::PrimaryNativeImageForDrawing(SubsamplingLevel, const DecodingOptions &) noexcept
  {
    if (RefPtr<NativeImage> nativeImage = PrimaryNativeImage())
    {
      return Ref<NativeImage>(krys::move(nativeImage));
    }
    return Unexpected(DecodingStatus::Invalid);
  }

  Expected<Ref<NativeImage>, DecodingStatus>
    ImageSource::CurrentNativeImageForDrawing(SubsamplingLevel subsamplingLevel,
                                              const DecodingOptions &options) noexcept
  {
    return PrimaryNativeImageForDrawing(subsamplingLevel, options);
  }

  bool ImageSource::HasSolidColor() const noexcept
  {
    return Size() == IntSize(1, 1) && FrameCount() == 1;
  }
}
