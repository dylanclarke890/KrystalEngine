#pragma once

#include "Krystal.IO/Image.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::IO
{
  struct ImageLoadSettings
  {
    /// @brief Whether to flip the image vertically.
    bool FlipVertically = false;

    /// @brief The desired components of the image. Set to 0 to use the default format of the image.
    int DesiredComponents = 0;
  };

  class ImageLoader
  {
    NO_COPY_MOVE(ImageLoader)

  public:
    ImageLoader() = default;

    ~ImageLoader() = default;

    /// @brief Checks if the image from the stream is a HDR image.
    NO_DISCARD bool IsHDRImage(IStreamReader &stream);

    /// @brief Loads an image from the given stream.
    NO_DISCARD Expected<Image> Load(IStreamReader &stream, const ImageLoadSettings &settings = {});
  };
}