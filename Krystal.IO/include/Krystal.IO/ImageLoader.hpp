#pragma once

#include "Krystal.IO/Image.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"

namespace Krys::IO
{
  struct ImageLoadSettings
  {
    /// @brief Whether to flip the image vertically.
    bool FlipVertically = false;

    /// @brief The desired components of the image. Set to 0 to use the default format of the image.
    int DesiredComponents = 0;
  };

  class ImageLoader : NonCopyMovable<ImageLoader>
  {
  public:
    ImageLoader() = default;

    ~ImageLoader() = default;

    /// @brief Checks if the image from the stream is a HDR image.
    KRYS_NODISCARD bool IsHDRImage(IStreamReader &stream);

    /// @brief Loads an image from the given stream.
    KRYS_NODISCARD Expected<Image> Load(IStreamReader &stream, const ImageLoadSettings &settings = {});
  };
}