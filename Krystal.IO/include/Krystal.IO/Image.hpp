#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Types.hpp"

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

    /// @brief Whether the image is in HDR format.
    bool IsHDR {};
  };

  /// @brief Represents a cubemap image consisting of 6 faces.
  struct CubeMapImage
  {
    /// @brief Right face image data.
    List<byte> Right {};

    /// @brief Left face image data.
    List<byte> Left {};

    /// @brief Top face image data.
    List<byte> Top {};

    /// @brief Bottom face image data.
    List<byte> Bottom {};

    /// @brief Front face image data.
    List<byte> Front {};

    /// @brief Back face image data.
    List<byte> Back {};

    /// @brief Width of cubemap faces (in pixels).
    int Width {};

    /// @brief Height of cubemap faces (in pixels).
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

  /// @brief Checks if the image at the given path is a HDR image.
  NO_DISCARD extern bool IsHDRImage(const Path &path) noexcept;

  /// @brief Checks if the image from the stream is a HDR image.
  NO_DISCARD extern bool IsHDRImage(IStreamReader &stream) noexcept;

  /// @brief Reads an image from a stream.
  /// @param stream Source stream to read from.
  /// @return Image read from the stream.
  NO_DISCARD extern Expected<Image> LoadImage(IStreamReader &stream, bool hdr,
                                              const ImageLoadSettings &settings = {}) noexcept;

  /// @brief Reads an image from a file.
  /// @param path Path to the image file.
  /// @return Image read from the file.
  NO_DISCARD extern Expected<Image> LoadImage(const Path &path,
                                              const ImageLoadSettings &settings = {}) noexcept;

  /// @brief Reads a cubemap image from 6 separate files.
  /// @param left Path to the left face image file.
  /// @param right Path to the right face image file.
  /// @param top Path to the top face image file.
  /// @param bottom Path to the bottom face image file.
  /// @param front Path to the front face image file.
  /// @param back Path to the back face image file.
  /// @param settings Settings for loading each image.
  /// @return Cubemap image read from the files.
  NO_DISCARD extern Expected<CubeMapImage> LoadCubeMap(const Path &left, const Path &right, const Path &top,
                                                       const Path &bottom, const Path &front,
                                                       const Path &back,
                                                       const ImageLoadSettings &settings = {}) noexcept;
}