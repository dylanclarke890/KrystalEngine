#pragma once

#include "Krystal.Booey/HTML/Canvas/PredefinedColorSpace.hpp"
#include "Krystal.Booey/HTML/ImageDataPixelFormat.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#imagedatasettings
  struct ImageDataSettings
  {
    Maybe<PredefinedColorSpace> ColorSpace;
    ImageDataPixelFormat Format {ImageDataPixelFormat::RgbaUnorm8};
  };
}
