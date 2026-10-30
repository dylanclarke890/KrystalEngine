#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/BMPImageReader.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  // This class decodes the ICO and CUR image formats.
  class ICOImageDecoder final : public ScalableImageDecoder
  {
  private:
    enum ImageType : uint8
    {
      Unknown,
      BMP,
      PNG,
    };

    enum FileType : uint8
    {
      ICON = 1,
      CURSOR = 2,
    };

    struct IconDirectoryEntry
    {
      IntSize Size;
      uint16 BitCount {0};
      IntPoint HotSpot;
      uint32 ImageOffset {0};
    };

    // An index into |m_data| representing how much we've already decoded.
    // Note that this only tracks data _this_ class decodes; once the
    // BMPImageReader takes over this will not be updated further.
    size_t _decodedOffset {0};

    // Which type of file (ICO/CUR) this is.
    FileType _fileType {ICON};

    // The headers for the ICO.
    List<IconDirectoryEntry> _dirEntries;

    // The image decoders for the various frames.
    List<UniquePtr<BMPImageReader>> _bmpReaders;
    List<RefPtr<ScalableImageDecoder>> _pngDecoders;

    // Valid only while a BMPImageReader is decoding, this holds the size
    // for the particular entry being decoded.
    IntSize _frameSize;

  public:
    KRYS_NODISCARD static Ref<ScalableImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new ICOImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~ICOImageDecoder() noexcept override;

    // ScalableImageDecoder
    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept final
    {
      return u8"ico";
    }
    void SetData(const SharedBuffer &data, bool allDataReceived) noexcept final;
    KRYS_NODISCARD IntSize Size() const noexcept final;
    KRYS_NODISCARD IntSize FrameSizeAtIndex(size_t index, SubsamplingLevel) const noexcept final;
    bool SetSize(const IntSize &size) noexcept final;
    KRYS_NODISCARD size_t FrameCount() const noexcept final;
    ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept final;
    // CAUTION: setFailed() deletes all readers and decoders. Be careful to
    // avoid accessing deleted memory, especially when calling this from
    // inside BMPImageReader!
    bool SetFailed() noexcept final;
    KRYS_NODISCARD Maybe<IntPoint> HotSpot() const noexcept final;

  private:
    ICOImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;
    void TryDecodeSize(bool allDataReceived) noexcept final
    {
      Decode(0, true, allDataReceived);
    }

    // Returns true if |a| is a preferable icon entry to |b|.
    // Larger sizes, or greater bitdepths at the same size, are preferable.
    KRYS_NODISCARD static bool CompareEntries(const IconDirectoryEntry &a,
                                              const IconDirectoryEntry &b) noexcept;

    KRYS_NODISCARD uint16 ReadUint16(size_t offset) const noexcept
    {
      return BMPImageReader::ReadUint16(*_data, _decodedOffset + offset);
    }

    KRYS_NODISCARD uint32 ReadUint32(size_t offset) const noexcept
    {
      return BMPImageReader::ReadUint32(*_data, _decodedOffset + offset);
    }

    // If the desired PNGImageDecoder exists, gives it the appropriate data.
    void SetDataForPNGDecoderAtIndex(size_t index) noexcept;

    // Decodes the entry at |index|. If |onlySize| is true, stops decoding
    // after calculating the image size. If decoding fails but there is no
    // more data coming, sets the "decode failure" flag.
    void Decode(size_t index, bool onlySize, bool allDataReceived) noexcept;

    // Decodes the directory and directory entries at the beginning of the
    // data, and initializes members. Returns true if all decoding
    // succeeded. Once this returns true, all entries' sizes are known.
    bool DecodeDirectory() noexcept;

    // Decodes the specified entry.
    bool DecodeAtIndex(size_t index) noexcept;

    // Processes the ICONDIR at the beginning of the data. Returns true if
    // the directory could be decoded.
    bool ProcessDirectory() noexcept;

    // Processes the ICONDIRENTRY records after the directory. Keeps the
    // "best" entry as the one we'll decode. Returns true if the entries
    // could be decoded.
    bool ProcessDirectoryEntries() noexcept;

    // Returns the hot-spot for |index|, returns std::nullopt if there is none.
    KRYS_NODISCARD Maybe<IntPoint> HotSpotAtIndex(size_t index) const noexcept;

    // Reads and returns a directory entry from the current offset into
    // |data|.
    IconDirectoryEntry ReadDirectoryEntry() noexcept;

    // Determines whether the desired entry is a BMP or PNG. Returns true
    // if the type could be determined.
    KRYS_NODISCARD ImageType ImageTypeAtIndex(size_t index) noexcept;
  };
}
