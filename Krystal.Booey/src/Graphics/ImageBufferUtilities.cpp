#include "Krystal.Booey/Graphics/ImageBufferUtilities.hpp"
#include "Krystal.Booey/Graphics/PixelBuffer.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/Base64.hpp"
#include <csetjmp>
#include <cstdio>
#include <cstdlib>

#include <jpeglib.h>
#include <png.h>
#include <webp/encode.h>

// libpng and libjpeg report errors with longjmp to the setjmp in the encoder, which owns no objects with
// destructors between the two.
#pragma warning(disable : 4'611) // interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4'324) // structure was padded due to alignment specifier (jmp_buf)

namespace krys::boo
{
  /// @brief The encoders' quality, 0 to 100, from @p quality, 0 to 1; 100 if it's empty or out of range.
  KRYS_NODISCARD static int32 EncoderQuality(Maybe<double> quality) noexcept
  {
    if (quality && *quality >= 0.0 && *quality <= 1.0)
    {
      return static_cast<int32>(*quality * 100.0 + 0.5);
    }
    return 100;
  }

  // MARK: - PNG

  static void WritePNGData(png_structp png, png_bytep data, png_size_t length) noexcept
  {
    auto &output = *static_cast<List<uint8> *>(png_get_io_ptr(png));
    output.insert(output.end(), data, data + length);
  }

  static void FlushPNGData(png_structp) noexcept
  {
  }

  static void PNGError(png_structp png, png_const_charp) noexcept
  {
    longjmp(png_jmpbuf(png), 1);
  }

  static void PNGWarning(png_structp, png_const_charp) noexcept
  {
  }

  // NOTE: not noexcept: libpng's errors longjmp back to the setjmp here, through its callbacks.
  KRYS_NODISCARD static bool EncodePNG(const PixelBuffer &pixelBuffer, List<uint8> &output)
  {
    const auto width = static_cast<uint32>(pixelBuffer.Size().Width());
    const auto height = static_cast<uint32>(pixelBuffer.Size().Height());
    const auto bytes = pixelBuffer.Bytes();

    auto png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, PNGError, PNGWarning);
    if (!png)
    {
      return false;
    }
    auto info = png_create_info_struct(png);
    if (!info)
    {
      png_destroy_write_struct(&png, nullptr);
      return false;
    }

    if (setjmp(png_jmpbuf(png)))
    {
      png_destroy_write_struct(&png, &info);
      return false;
    }

    png_set_write_fn(png, &output, WritePNGData, FlushPNGData);
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    for (uint32 y = 0; y < height; ++y)
    {
      png_write_row(png, bytes.data() + static_cast<size_t>(y) * width * 4u);
    }
    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    return true;
  }

  // MARK: - JPEG

  namespace
  {
    struct JPEGErrorManager
    {
      jpeg_error_mgr Pub;
      jmp_buf SetjmpBuffer;
    };
  }

  static void JPEGErrorExit(j_common_ptr info) noexcept
  {
    auto *error = reinterpret_cast<JPEGErrorManager *>(info->err);
    longjmp(error->SetjmpBuffer, 1);
  }

  static void JPEGOutputMessage(j_common_ptr) noexcept
  {
  }

  // NOTE: not noexcept: libjpeg's errors longjmp back to the setjmp here.
  KRYS_NODISCARD static bool EncodeJPEG(const PixelBuffer &pixelBuffer, int32 quality, List<uint8> &output)
  {
    const auto width = static_cast<uint32>(pixelBuffer.Size().Width());
    const auto height = static_cast<uint32>(pixelBuffer.Size().Height());
    const auto bytes = pixelBuffer.Bytes();

    // The rows without their alpha.
    List<uint8> row(static_cast<size_t>(width) * 3u);

    jpeg_compress_struct info {};
    JPEGErrorManager error {};
    info.err = jpeg_std_error(&error.Pub);
    error.Pub.error_exit = JPEGErrorExit;
    error.Pub.output_message = JPEGOutputMessage;

    unsigned char *buffer = nullptr;
    unsigned long size = 0;
    if (setjmp(error.SetjmpBuffer))
    {
      jpeg_destroy_compress(&info);
      std::free(buffer);
      return false;
    }

    jpeg_create_compress(&info);
    jpeg_mem_dest(&info, &buffer, &size);
    info.image_width = width;
    info.image_height = height;
    info.input_components = 3;
    info.in_color_space = JCS_RGB;
    jpeg_set_defaults(&info);
    jpeg_set_quality(&info, quality, TRUE);
    jpeg_start_compress(&info, TRUE);
    while (info.next_scanline < height)
    {
      const auto *source = bytes.data() + static_cast<size_t>(info.next_scanline) * width * 4u;
      for (uint32 x = 0; x < width; ++x)
      {
        row[x * 3u] = source[x * 4u];
        row[x * 3u + 1u] = source[x * 4u + 1u];
        row[x * 3u + 2u] = source[x * 4u + 2u];
      }
      JSAMPROW rows[1] = {row.data()};
      jpeg_write_scanlines(&info, rows, 1);
    }
    jpeg_finish_compress(&info);
    output.assign(buffer, buffer + size);
    jpeg_destroy_compress(&info);
    std::free(buffer);
    return true;
  }

  // MARK: - WebP

  KRYS_NODISCARD static bool EncodeWebP(const PixelBuffer &pixelBuffer, int32 quality,
                                        List<uint8> &output) noexcept
  {
    const auto width = pixelBuffer.Size().Width();
    const auto height = pixelBuffer.Size().Height();
    uint8_t *encoded = nullptr;
    auto size = WebPEncodeRGBA(pixelBuffer.Bytes().data(), width, height, width * 4,
                               static_cast<float>(quality), &encoded);
    if (!size)
    {
      return false;
    }
    output.assign(encoded, encoded + size);
    WebPFree(encoded);
    return true;
  }

  List<uint8> EncodeData(const PixelBuffer &pixelBuffer, utf8_stringview mimeType,
                         Maybe<double> quality) noexcept
  {
    krys_debug_assert(pixelBuffer.Format().Format == PixelFormat::RGBA8);
    if (pixelBuffer.Size().IsEmpty())
    {
      return {};
    }

    List<uint8> output;
    bool encoded = false;
    if (MIMETypeRegistry::IsJPEGMIMEType(mimeType))
    {
      encoded = EncodeJPEG(pixelBuffer, EncoderQuality(quality), output);
    }
    else if (text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"image/webp")))
    {
      encoded = EncodeWebP(pixelBuffer, EncoderQuality(quality), output);
    }
    else if (text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"image/png")))
    {
      encoded = EncodePNG(pixelBuffer, output);
    }

    if (!encoded)
    {
      return {};
    }
    return output;
  }

  utf8_string DataURL(Span<const uint8> data, utf8_stringview mimeType) noexcept
  {
    if (data.empty())
    {
      return u8"data:,";
    }

    utf8_string url = u8"data:";
    url += mimeType;
    url += u8";base64,";
    url += Base64Encode(data);
    return url;
  }
}
