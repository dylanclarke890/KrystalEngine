#include "Krystal.Booey/Style/Values/Images/ImageOrientation.hpp"

#include "Krystal.Booey/Graphics/ImageOrientation.hpp"

namespace krys::boo::style
{
  // MARK: - Platform

  boo::ImageOrientation ToPlatform<ImageOrientation>::operator()(ImageOrientation value) noexcept
  {
    switch (value)
    {
      case ImageOrientation::FromImage: return boo::ImageOrientation::Orientation::FromImage;
      case ImageOrientation::None:      return boo::ImageOrientation::Orientation::None;
    }
    krys_unreachable();
  }
}
