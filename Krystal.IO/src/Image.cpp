#include "Krystal.IO/Image.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"

#include <stb_image.h>
#include <utility>

namespace
{
  int stbi_stream_read(void *user, char *data, int size)
  {
    auto *stream = static_cast<Krys::IO::IStreamReader *>(user);
    auto *byteData = static_cast<Krys::byte *>(static_cast<void *>(data));
    return static_cast<int>(stream->Read(byteData, size));
  }

  void stbi_stream_skip(void *user, int n)
  {
    auto *stream = static_cast<Krys::IO::IStreamReader *>(user);
    stream->Seek(n, Krys::IO::SeekOrigin::Current);
  }

  int stbi_stream_eof(void *user)
  {
    auto *stream = static_cast<Krys::IO::IStreamReader *>(user);
    return static_cast<int>(stream->EndOfStream());
  }
}

namespace Krys::IO
{
  // NOLINTNEXTLINE(misc-use-internal-linkage)
  Expected<Image> LoadImage(IStreamReader &stream, const ImageLoadSettings &settings) noexcept
  {
    stbi_io_callbacks callbacks {};
    callbacks.read = stbi_stream_read;
    callbacks.skip = stbi_stream_skip;
    callbacks.eof = stbi_stream_eof;

    Image image;
    stbi_set_flip_vertically_on_load(static_cast<int>(settings.FlipVertically));

    int width {};
    int height {};
    int channels {};
    auto *imageData =
      stbi_load_from_callbacks(&callbacks, &stream, &width, &height, &channels, settings.DesiredComponents);

    if (imageData == nullptr)
    {
      return Unexpected(stbi_failure_reason());
    }

    image.Width = width;
    image.Height = height;
    image.Channels = channels;

    auto *first = imageData;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *last = imageData + (static_cast<std::size_t>(width) * height * channels);
    image.Data.assign(reinterpret_cast<byte *>(first), reinterpret_cast<byte *>(last));

    stbi_image_free(imageData);

    return Expected<Image>(std::move(image));
  }

  // NOLINTNEXTLINE(misc-use-internal-linkage)
  Expected<Image> LoadImage(const Path &path, const ImageLoadSettings &settings) noexcept
  {
    // NOLINTNEXTLINE(misc-const-correctness)
    NativeFileReader fileReader {path};
    return LoadImage(fileReader, settings);
  }

  // NOLINTNEXTLINE(misc-use-internal-linkage, bugprone-easily-swappable-parameters)
  Expected<CubeMapImage> LoadCubeMap(const Path &left, const Path &right, const Path &top, const Path &bottom,
                                     const Path &front, const Path &back,
                                     const ImageLoadSettings &settings) noexcept
  {
    CubeMapImage cubeMapImage {};

    auto leftFace = LoadImage(left, settings);
    if (!leftFace.has_value())
    {
      return Unexpected(leftFace.error());
    }
    if (leftFace->Width <= 0 || leftFace->Height <= 0 || leftFace->Channels <= 0)
    {
      return Unexpected("Cubemap face has invalid dimensions (left face).");
    }
    cubeMapImage.Left = std::move(leftFace->Data);

    cubeMapImage.Width = leftFace->Width;
    cubeMapImage.Height = leftFace->Height;
    cubeMapImage.Channels = leftFace->Channels;

    auto rightFace = LoadImage(right, settings);
    if (!rightFace.has_value())
    {
      return Unexpected(rightFace.error());
    }
    if (rightFace->Width != cubeMapImage.Width || rightFace->Height != cubeMapImage.Height
        || rightFace->Channels != cubeMapImage.Channels)
    {
      return Unexpected("Cubemap face dimensions do not match (left face).");
    }
    cubeMapImage.Right = std::move(rightFace->Data);

    auto topFace = LoadImage(top, settings);
    if (!topFace.has_value())
    {
      return Unexpected(topFace.error());
    }
    if (topFace->Width != cubeMapImage.Width || topFace->Height != cubeMapImage.Height
        || topFace->Channels != cubeMapImage.Channels)
    {
      return Unexpected("Cubemap face dimensions do not match (top face).");
    }
    cubeMapImage.Top = std::move(topFace->Data);

    auto bottomFace = LoadImage(bottom, settings);
    if (!bottomFace.has_value())
    {
      return Unexpected(bottomFace.error());
    }
    if (bottomFace->Width != cubeMapImage.Width || bottomFace->Height != cubeMapImage.Height
        || bottomFace->Channels != cubeMapImage.Channels)
    {
      return Unexpected("Cubemap face dimensions do not match (bottom face).");
    }
    cubeMapImage.Bottom = std::move(bottomFace->Data);

    auto frontFace = LoadImage(front, settings);
    if (!frontFace.has_value())
    {
      return Unexpected(frontFace.error());
    }
    if (frontFace->Width != cubeMapImage.Width || frontFace->Height != cubeMapImage.Height
        || frontFace->Channels != cubeMapImage.Channels)
    {
      return Unexpected("Cubemap face dimensions do not match (front face).");
    }
    cubeMapImage.Front = std::move(frontFace->Data);

    auto backFace = LoadImage(back, settings);
    if (!backFace.has_value())
    {
      return Unexpected(backFace.error());
    }
    if (backFace->Width != cubeMapImage.Width || backFace->Height != cubeMapImage.Height
        || backFace->Channels != cubeMapImage.Channels)
    {
      return Unexpected("Cubemap face dimensions do not match (back face).");
    }
    cubeMapImage.Back = std::move(backFace->Data);

    return Expected<CubeMapImage>(std::move(cubeMapImage));
  }
}