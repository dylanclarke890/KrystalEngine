#include "Krystal.Booey/Graphics/GeneratedImage.hpp"

namespace krys::boo
{
  void GeneratedImage::ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                                  FloatSize &intrinsicRatio) noexcept
  {
    Image::ComputeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    intrinsicRatio = FloatSize();
  }
}
