#include "Krystal.Booey/Graphics/ImageBuffers/ImageBufferUtilities.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/PixelBuffer.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Image/ImageEncoder.hpp"
#include "Krystal.Net/MimeTypeRegistry.hpp"
#include "Krystal.Text/Base64.hpp"

namespace krys::boo
{
  /// @brief The format `EncodeData` writes @p mimeType as, if it's one.
  KRYS_NODISCARD static Maybe<img::EncodedImageFormat> EncodedFormat(utf8_stringview mimeType) noexcept
  {
    if (net::MimeTypeRegistry::IsJPEGMimeType(mimeType))
    {
      return img::EncodedImageFormat::JPEG;
    }
    if (text::IsASCIICaselessEqual(mimeType, u8"image/webp"))
    {
      return img::EncodedImageFormat::WebP;
    }
    if (text::IsASCIICaselessEqual(mimeType, u8"image/png"))
    {
      return img::EncodedImageFormat::PNG;
    }
    return null;
  }

  bool SupportsEncodingMimeType(utf8_stringview mimeType) noexcept
  {
    return EncodedFormat(mimeType).has_value();
  }

  List<uint8> EncodeData(const PixelBuffer &pixelBuffer, utf8_stringview mimeType,
                         Maybe<double> quality) noexcept
  {
    krys_debug_assert(pixelBuffer.Format().Format == PixelFormat::RGBA8);
    auto format = EncodedFormat(mimeType);
    if (!format)
    {
      return {};
    }
    return img::EncodeImage(pixelBuffer.Bytes(), pixelBuffer.Size(), *format, quality);
  }

  utf8_string DataUrl(Span<const uint8> data, utf8_stringview mimeType) noexcept
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
