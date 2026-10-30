#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/BMPImageReader.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  // This class decodes the BMP image format.
  class BMPImageDecoder final : public ScalableImageDecoder
  {
  private:
    // An index into |m_data| representing how much we've already decoded.
    // Note that this only tracks data _this_ class decodes; once the
    // BMPImageReader takes over this will not be updated further.
    size_t _decodedOffset {0};

    // The reader used to do most of the BMP decoding.
    UniquePtr<BMPImageReader> _reader;

  public:
    KRYS_NODISCARD static Ref<ScalableImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new BMPImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    // ScalableImageDecoder
    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept final
    {
      return u8"bmp";
    }
    void SetData(const SharedBuffer &data, bool allDataReceived) noexcept final;
    ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept final;
    // CAUTION: setFailed() deletes |m_reader|. Be careful to avoid
    // accessing deleted memory, especially when calling this from inside
    // BMPImageReader!
    bool SetFailed() noexcept final;

  private:
    BMPImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;
    void TryDecodeSize(bool allDataReceived) noexcept final
    {
      Decode(true, allDataReceived);
    }

    KRYS_NODISCARD uint32 ReadUint32(size_t offset) const noexcept
    {
      return BMPImageReader::ReadUint32(*_data, _decodedOffset + offset);
    }

    // Decodes the image. If |onlySize| is true, stops decoding after
    // calculating the image size. If decoding fails but there is no more
    // data coming, sets the "decode failure" flag.
    void Decode(bool onlySize, bool allDataReceived) noexcept;

    // Decodes the image. If |onlySize| is true, stops decoding after
    // calculating the image size. Returns whether decoding succeeded.
    bool DecodeHelper(bool onlySize) noexcept;

    // Processes the file header at the beginning of the data. Sets
    // |*imgDataOffset| based on the header contents. Returns true if the
    // file header could be decoded.
    bool ProcessFileHeader(size_t *imgDataOffset) noexcept;
  };
}
