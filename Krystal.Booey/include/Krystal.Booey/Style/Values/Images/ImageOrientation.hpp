#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo
{
  struct ImageOrientation;

  namespace style
  {
    // <'image-orientation'> = from-image | none | [ <angle> || flip ]
    // FIXME: Add support for `[ <angle> || flip ]`.
    // https://drafts.csswg.org/css-images-3/#propdef-image-orientation
    enum class ImageOrientation : bool
    {
      FromImage,
      None,
    };

    // MARK: - Platform

    template <>
    struct ToPlatform<ImageOrientation>
    {
      KRYS_NODISCARD boo::ImageOrientation operator()(ImageOrientation) noexcept;
    };

  }
}
