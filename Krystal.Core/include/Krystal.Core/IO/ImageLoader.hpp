#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/IO/Image.hpp"
#include "Krystal.Core/IO/Streams/Stream.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Expected.hpp"

namespace krys::io
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
    KRYS_NON_COPY_MOVABLE(ImageLoader);

  public:
    ImageLoader() = default;

    ~ImageLoader() = default;

    /// @brief Checks if the image from the stream is a HDR image.
    KRYS_NODISCARD bool IsHDRImage(IStreamReader &stream);

    /// @brief Loads an image from the given stream.
    KRYS_NODISCARD Expected<Image> Load(IStreamReader &stream, const ImageLoadSettings &settings = {});
  };
}