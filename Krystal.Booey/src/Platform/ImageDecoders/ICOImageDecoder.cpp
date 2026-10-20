#include "Krystal.Booey/Platform/ImageDecoders/ICOImageDecoder.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/PNGImageDecoder.hpp"
#include <algorithm>
#include <cstring>

namespace krys::boo
{
  namespace
  {
    // Number of bits in .ICO/.CUR used to store the directory and its entries,
    // respectively (doesn't match sizeof values for member structs since we omit
    // some fields).
    constexpr size_t SizeOfDirectory = 6;
    constexpr size_t SizeOfDirEntry = 16;
  }

  ICOImageDecoder::ICOImageDecoder(AlphaOption alphaOption,
                                   GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ScalableImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  ICOImageDecoder::~ICOImageDecoder() noexcept = default;

  void ICOImageDecoder::SetData(const SharedBuffer &data, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    ScalableImageDecoder::SetData(data, allDataReceived);

    for (auto &reader : _bmpReaders)
    {
      if (reader)
      {
        reader->SetData(*_data);
      }
    }

    for (size_t i = 0; i < _pngDecoders.size(); ++i)
    {
      SetDataForPNGDecoderAtIndex(i);
    }
  }

  IntSize ICOImageDecoder::Size() const noexcept
  {
    return _frameSize.IsEmpty() ? ScalableImageDecoder::Size() : _frameSize;
  }

  IntSize ICOImageDecoder::FrameSizeAtIndex(size_t index, SubsamplingLevel) const noexcept
  {
    return (index && (index < _dirEntries.size())) ? _dirEntries[index].Size : Size();
  }

  bool ICOImageDecoder::SetSize(const IntSize &size) noexcept
  {
    // The size calculated inside the BMPImageReader had better match the one in
    // the icon directory.
    return _frameSize.IsEmpty() ? ScalableImageDecoder::SetSize(size) : ((size == _frameSize) || SetFailed());
  }

  size_t ICOImageDecoder::FrameCount() const noexcept
  {
    const_cast<ICOImageDecoder *>(this)->Decode(0, true, IsAllDataReceived());
    return _frameBufferCache.size();
  }

  ScalableImageDecoderFrame *ICOImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    // Ensure |index| is valid.
    if (index >= FrameCount())
    {
      return nullptr;
    }

    auto *buffer = &_frameBufferCache[index];
    if (!buffer->IsComplete())
    {
      Decode(index, false, IsAllDataReceived());
    }
    return buffer;
  }

  bool ICOImageDecoder::SetFailed() noexcept
  {
    _bmpReaders.clear();
    _pngDecoders.clear();
    return ScalableImageDecoder::SetFailed();
  }

  Maybe<IntPoint> ICOImageDecoder::HotSpot() const noexcept
  {
    // When unspecified, the default frame is always frame 0. This is consistent with
    // BitmapImage where currentFrame() starts at 0 and only increases when animation is
    // requested.
    return HotSpotAtIndex(0);
  }

  Maybe<IntPoint> ICOImageDecoder::HotSpotAtIndex(size_t index) const noexcept
  {
    if (index >= _dirEntries.size() || _fileType != CURSOR)
    {
      return null;
    }

    return _dirEntries[index].HotSpot;
  }

  bool ICOImageDecoder::CompareEntries(const IconDirectoryEntry &a, const IconDirectoryEntry &b) noexcept
  {
    // Larger icons are better.  After that, higher bit-depth icons are better.
    const int aEntryArea = a.Size.Width() * a.Size.Height();
    const int bEntryArea = b.Size.Width() * b.Size.Height();
    return (aEntryArea == bEntryArea) ? (a.BitCount > b.BitCount) : (aEntryArea > bEntryArea);
  }

  void ICOImageDecoder::SetDataForPNGDecoderAtIndex(size_t index) noexcept
  {
    if (!_pngDecoders[index])
    {
      return;
    }

    const IconDirectoryEntry &dirEntry = _dirEntries[index];
    // Copy out PNG data to a separate vector and send to the PNG decoder.
    // FIXME: Save this copy by making the PNG decoder able to take an
    // optional offset.
    auto pngData = SharedBuffer::Create(_data->GetSpan().subspan(dirEntry.ImageOffset));
    _pngDecoders[index]->SetData(*pngData, IsAllDataReceived());
  }

  void ICOImageDecoder::Decode(size_t index, bool onlySize, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    // If we couldn't decode the image but we've received all the data, decoding
    // has failed.
    if ((!DecodeDirectory() || (!onlySize && !DecodeAtIndex(index))) && allDataReceived)
    {
      (void)SetFailed();
    }
    // If we're done decoding this frame, we don't need the BMPImageReader or
    // PNGImageDecoder anymore.  (If we failed, these have already been
    // cleared.)
    else if ((_frameBufferCache.size() > index) && _frameBufferCache[index].IsComplete())
    {
      _bmpReaders[index] = nullptr;
      _pngDecoders[index] = nullptr;
    }

    if (_frameBufferCache.empty())
    {
      _frameBufferCache.resize(_dirEntries.size());
    }
    // CAUTION: We must not resize m_frameBufferCache again after this, as
    // decodeAtIndex() may give a BMPImageReader a pointer to one of the
    // entries.
  }

  bool ICOImageDecoder::DecodeDirectory() noexcept
  {
    // Read and process directory.
    if ((_decodedOffset < SizeOfDirectory) && !ProcessDirectory())
    {
      return false;
    }

    // Read and process directory entries.
    return (_decodedOffset >= (SizeOfDirectory + (_dirEntries.size() * SizeOfDirEntry)))
           || ProcessDirectoryEntries();
  }

  bool ICOImageDecoder::DecodeAtIndex(size_t index) noexcept
  {
    krys_assert(index < _dirEntries.size());
    const IconDirectoryEntry &dirEntry = _dirEntries[index];
    const ImageType imageType = ImageTypeAtIndex(index);
    if (imageType == Unknown)
    {
      return false; // Not enough data to determine image type yet.
    }

    if (imageType == BMP)
    {
      if (!_bmpReaders[index])
      {
        // We need to have already sized m_frameBufferCache before this, and
        // we must not resize it again later (see caution in frameCount()).
        krys_debug_assert(_frameBufferCache.size() == _dirEntries.size());
        _bmpReaders[index] = CreateUnique<BMPImageReader>(this, dirEntry.ImageOffset, 0, true);
        _bmpReaders[index]->SetData(*_data);
        _bmpReaders[index]->SetBuffer(&_frameBufferCache[index]);
      }
      _frameSize = dirEntry.Size;
      bool result = _bmpReaders[index]->DecodeBMP(false);
      _frameSize = IntSize();
      return result;
    }

    if (!_pngDecoders[index])
    {
      _pngDecoders[index] = PNGImageDecoder::Create(
        _premultiplyAlpha ? AlphaOption::Premultiplied : AlphaOption::NotPremultiplied,
        _ignoreGammaAndColorProfile ? GammaAndColorProfileOption::Ignored
                                    : GammaAndColorProfileOption::Applied);
      SetDataForPNGDecoderAtIndex(index);
    }
    // Fail if the size the PNGImageDecoder calculated does not match the size
    // in the directory.
    if (_pngDecoders[index]->GetEncodedDataStatus() >= EncodedDataStatus::SizeAvailable
        && (_pngDecoders[index]->Size() != dirEntry.Size))
    {
      return SetFailed();
    }
    // The PNG decoder has no frame until it has its size.
    auto *pngFrame = _pngDecoders[index]->FrameBufferAtIndex(0);
    if (!pngFrame)
    {
      return !_pngDecoders[index]->Failed() || SetFailed();
    }
    _frameBufferCache[index] = *pngFrame;
    return !_pngDecoders[index]->Failed() || SetFailed();
  }

  bool ICOImageDecoder::ProcessDirectory() noexcept
  {
    // Read directory.
    krys_debug_assert(!_decodedOffset);
    if (_data->Size() < SizeOfDirectory)
    {
      return false;
    }
    const uint16 fileType = ReadUint16(2);
    const uint16 idCount = ReadUint16(4);
    _decodedOffset = SizeOfDirectory;

    // See if this is an icon filetype we understand, and make sure we have at
    // least one entry in the directory.
    if (((fileType != ICON) && (fileType != CURSOR)) || (!idCount))
    {
      return SetFailed();
    }

    _fileType = static_cast<FileType>(fileType);

    // Enlarge member vectors to hold all the entries.
    _dirEntries.resize(idCount);
    _bmpReaders.resize(idCount);
    _pngDecoders.resize(idCount);
    return true;
  }

  bool ICOImageDecoder::ProcessDirectoryEntries() noexcept
  {
    // Read directory entries.
    krys_debug_assert(_decodedOffset == SizeOfDirectory);
    if ((_decodedOffset > _data->Size())
        || ((_data->Size() - _decodedOffset) < (_dirEntries.size() * SizeOfDirEntry)))
    {
      return false;
    }
    for (auto &entry : _dirEntries)
    {
      entry = ReadDirectoryEntry(); // Updates m_decodedOffset.
    }

    // Make sure the specified image offsets are past the end of the directory
    // entries.
    for (auto &entry : _dirEntries)
    {
      if (entry.ImageOffset < _decodedOffset)
      {
        return SetFailed();
      }
    }

    // Arrange frames in decreasing quality order.
    std::sort(_dirEntries.begin(), _dirEntries.end(), CompareEntries);

    // The image size is the size of the largest entry.
    const IconDirectoryEntry &dirEntry = _dirEntries.front();
    // Technically, this next call shouldn't be able to fail, since the width
    // and height here are each <= 256, and |m_frameSize| is empty.
    return SetSize(dirEntry.Size);
  }

  ICOImageDecoder::IconDirectoryEntry ICOImageDecoder::ReadDirectoryEntry() noexcept
  {
    // Read icon data.
    // The casts to uint8_t in the next few lines are because that's the on-disk
    // type of the width and height values.  Storing them in ints (instead of
    // matching uint8_ts) is so we can record dimensions of size 256 (which is
    // what a zero byte really means).
    int width = (*_data)[_decodedOffset];
    if (!width)
    {
      width = 256;
    }
    int height = (*_data)[_decodedOffset + 1];
    if (!height)
    {
      height = 256;
    }
    IconDirectoryEntry entry;
    entry.Size = IntSize(width, height);
    if (_fileType == CURSOR)
    {
      entry.BitCount = 0;
      entry.HotSpot = IntPoint(ReadUint16(4), ReadUint16(6));
    }
    else
    {
      entry.BitCount = ReadUint16(6);
      entry.HotSpot = IntPoint();
    }
    entry.ImageOffset = ReadUint32(12);

    // Some icons don't have a bit depth, only a color count.  Convert the
    // color count to the minimum necessary bit depth.  It doesn't matter if
    // this isn't quite what the bitmap info header says later, as we only use
    // this value to determine which icon entry is best.
    if (!entry.BitCount)
    {
      int colorCount = (*_data)[_decodedOffset + 2];
      if (!colorCount)
      {
        colorCount = 256; // Vague in the spec, needed by real-world icons.
      }
      for (--colorCount; colorCount; colorCount >>= 1)
      {
        ++entry.BitCount;
      }
    }

    _decodedOffset += SizeOfDirEntry;
    return entry;
  }

  ICOImageDecoder::ImageType ICOImageDecoder::ImageTypeAtIndex(size_t index) noexcept
  {
    // Check if this entry is a BMP or a PNG; we need 4 bytes to check the magic
    // number.
    krys_assert(index < _dirEntries.size());
    const uint32 imageOffset = _dirEntries[index].ImageOffset;
    if ((imageOffset > _data->Size()) || ((_data->Size() - imageOffset) < 4))
    {
      return Unknown;
    }
    return !std::memcmp(_data->Data() + imageOffset, "\x89PNG", 4) ? PNG : BMP;
  }
}
