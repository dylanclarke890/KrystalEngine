#include "Krystal.IO/ImageLoader.hpp"
#include <stb_image.h>

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

  stbi_io_callbacks GetCallbacks() noexcept
  {
    stbi_io_callbacks callbacks {};
    callbacks.read = stbi_stream_read;
    callbacks.skip = stbi_stream_skip;
    callbacks.eof = stbi_stream_eof;
    return callbacks;
  }
}

namespace Krys::IO
{
  KRYS_NODISCARD bool ImageLoader::IsHDRImage(IStreamReader &stream)
  {
    if (!stream.IsOpen() && !stream.Open())
    {
      return false;
    }

    const stbi_io_callbacks callbacks = GetCallbacks();
    const bool result = stbi_is_hdr_from_callbacks(&callbacks, &stream) != 0;
    stream.Seek(0, SeekOrigin::Begin);

    return result;
  }

  KRYS_NODISCARD Expected<Image> ImageLoader::Load(IStreamReader &stream, const ImageLoadSettings &settings)
  {
    Image image {};
    image.DataType = IsHDRImage(stream) ? ImageDataType::Float : ImageDataType::UnsignedByte;

    stbi_set_flip_vertically_on_load(static_cast<int>(settings.FlipVertically));
    const stbi_io_callbacks callbacks = GetCallbacks();

    if (image.DataType == ImageDataType::Float)
    {
      float *data = stbi_loadf_from_callbacks(&callbacks, &stream, &image.Width, &image.Height,
                                              &image.Channels, settings.DesiredComponents);
      if (data == nullptr)
      {
        return Unexpected(stbi_failure_reason());
      }

      const size_t count = static_cast<size_t>(image.Width) * image.Height * image.Channels;
      image.Data.resize(count * sizeof(float));
      std::memcpy(image.Data.data(), data, image.Data.size());
      stbi_image_free(data);
    }
    else
    {
      stbi_uc *data = stbi_load_from_callbacks(&callbacks, &stream, &image.Width, &image.Height,
                                               &image.Channels, settings.DesiredComponents);
      if (data == nullptr)
      {
        return Unexpected(stbi_failure_reason());
      }

      const size_t count = static_cast<size_t>(image.Width) * image.Height * image.Channels;
      image.Data.resize(count);
      std::memcpy(image.Data.data(), data, image.Data.size());
      stbi_image_free(data);
    }

    return std::move(image);
  }
}