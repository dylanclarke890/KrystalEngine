#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.IO/IStream.hpp"

namespace Krys::IO
{
  /// @brief Represents an image.
  struct Image
  {
    /// @brief Image data.
    List<byte> Data {};

    /// @brief Width of image (in pixels).
    int Width {};

    /// @brief Height of image (in pixels).
    int Height {};

    /// @brief Number of channels.
    int Channels {};
  };

  struct ImageLoadSettings
  {
    /// @brief Whether to flip the image vertically.
    bool FlipVertically = false;

    /// @brief The desired components of the image. Set to 0 to use the default format of the image.
    int DesiredComponents = 0;
  };

  /// @brief Reads an image from a stream.
  /// @param stream Stream to read from.
  /// @return Image read from the stream.
  NO_DISCARD Expected<Image> LoadImage(IStreamReader &stream, const ImageLoadSettings &settings = {}) noexcept;
}