#include "Krystal.IO/Image.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IStream.hpp"

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
  Result<Image> LoadImage(IStreamReader &stream, const ImageLoadSettings &settings) noexcept
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
      return Fail(stbi_failure_reason());
    }

    image.Width = width;
    image.Height = height;
    image.Channels = channels;

    auto *first = imageData;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *last = imageData + (static_cast<std::size_t>(width) * height * channels);
    image.Data.assign(reinterpret_cast<byte *>(first), reinterpret_cast<byte *>(last));

    stbi_image_free(imageData);

    return Result<Image>(std::move(image));
  }
}