#pragma once

#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::IO
{
  enum class ImageDataType : uint8
  {
    UnsignedByte,
    Float
  };

  /// @brief Represents an image.
  struct Image
  {
    /// @brief Image data.
    List<byte> Data;

    /// @brief The type of data stored in the image.
    ImageDataType DataType {ImageDataType::UnsignedByte};

    /// @brief Width of image (in pixels).
    int Width {};

    /// @brief Height of image (in pixels).
    int Height {};

    /// @brief Number of channels.
    int Channels {};
  };
}