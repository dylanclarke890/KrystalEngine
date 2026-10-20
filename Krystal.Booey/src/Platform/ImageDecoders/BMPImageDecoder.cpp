#include "Krystal.Booey/Platform/ImageDecoders/BMPImageDecoder.hpp"

namespace krys::boo
{
  namespace
  {
    // Number of bits in .BMP used to store the file header (doesn't match
    // "sizeof(BMPImageDecoder::BitmapFileHeader)" since we omit some fields and
    // don't pack).
    constexpr size_t SizeOfFileHeader = 14;
  }

  BMPImageDecoder::BMPImageDecoder(AlphaOption alphaOption,
                                   GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ScalableImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  void BMPImageDecoder::SetData(const SharedBuffer &data, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    ScalableImageDecoder::SetData(data, allDataReceived);
    if (_reader)
    {
      _reader->SetData(*_data);
    }
  }

  ScalableImageDecoderFrame *BMPImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (index)
    {
      return nullptr;
    }

    if (_frameBufferCache.empty())
    {
      _frameBufferCache.resize(1);
    }

    auto *buffer = &_frameBufferCache.front();
    if (!buffer->IsComplete())
    {
      Decode(false, IsAllDataReceived());
    }
    return buffer;
  }

  bool BMPImageDecoder::SetFailed() noexcept
  {
    _reader = nullptr;
    return ScalableImageDecoder::SetFailed();
  }

  void BMPImageDecoder::Decode(bool onlySize, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    // If we couldn't decode the image but we've received all the data, decoding
    // has failed.
    if (!DecodeHelper(onlySize) && allDataReceived)
    {
      (void)SetFailed();
    }
    // If we're done decoding the image, we don't need the BMPImageReader
    // anymore.  (If we failed, |m_reader| has already been cleared.)
    else if (!_frameBufferCache.empty() && _frameBufferCache.front().IsComplete())
    {
      _reader = nullptr;
    }
  }

  bool BMPImageDecoder::DecodeHelper(bool onlySize) noexcept
  {
    size_t imgDataOffset = 0;
    if ((_decodedOffset < SizeOfFileHeader) && !ProcessFileHeader(&imgDataOffset))
    {
      return false;
    }

    if (!_reader)
    {
      _reader = CreateUnique<BMPImageReader>(this, _decodedOffset, imgDataOffset, false);
      _reader->SetData(*_data);
    }

    if (!_frameBufferCache.empty())
    {
      _reader->SetBuffer(&_frameBufferCache.front());
    }

    return _reader->DecodeBMP(onlySize);
  }

  bool BMPImageDecoder::ProcessFileHeader(size_t *imgDataOffset) noexcept
  {
    krys_debug_assert(imgDataOffset);

    // Read file header.
    krys_debug_assert(!_decodedOffset);
    if (_data->Size() < SizeOfFileHeader)
    {
      return false;
    }
    const uint16 fileType = static_cast<uint16>(((*_data)[0] << 8) | (*_data)[1]);
    *imgDataOffset = ReadUint32(10);
    _decodedOffset = SizeOfFileHeader;

    // See if this is a bitmap filetype we understand.
    enum : uint16
    {
      BMAP = 0x424D, // "BM"
      // The following additional OS/2 2.x header values (see
      // http://www.fileformat.info/format/os2bmp/egff.htm ) aren't widely
      // decoded, and are unlikely to be in much use.
      /*
      ICON = 0x4943,  // "IC"
      POINTER = 0x5054,  // "PT"
      COLORICON = 0x4349,  // "CI"
      COLORPOINTER = 0x4350,  // "CP"
      BITMAPARRAY = 0x4241,  // "BA"
      */
    };
    return (fileType == BMAP) || SetFailed();
  }
}
