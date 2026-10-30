#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include <array>
#include <cstring>

namespace krys::boo
{
  // This class decodes a BMP image. It is used in the BMP and ICO decoders,
  // which wrap it in the appropriate code to read file headers, etc.
  class BMPImageReader
  {
  public:
    // Read a value from |data[offset]|, converting from little to native
    // endianness.
    KRYS_NODISCARD static uint16 ReadUint16(const SharedBuffer &data, size_t offset) noexcept
    {
      uint16 result;
      std::memcpy(&result, data.Data() + offset, 2);
      return result;
    }

    KRYS_NODISCARD static uint32 ReadUint32(const SharedBuffer &data, size_t offset) noexcept
    {
      uint32 result;
      std::memcpy(&result, data.Data() + offset, 4);
      return result;
    }

  private:
    // The various BMP compression types. We don't currently decode all
    // these.
    enum CompressionType : uint8
    {
      // Universal types
      RGB = 0,
      RLE8 = 1,
      RLE4 = 2,
      // Windows V3+ only
      BITFIELDS = 3,
      JPEG = 4,
      PNG = 5,
      // OS/2 2.x-only
      HUFFMAN1D, // Stored in file as 3
      RLE24,     // Stored in file as 4
    };
    enum AndMaskState : uint8
    {
      None,
      NotYetDecoded,
      Decoding,
    };
    enum ProcessingResult : uint8
    {
      Success,
      Failure,
      InsufficientData,
    };

    // These are based on the Windows BITMAPINFOHEADER and RGBTRIPLE
    // structs, but with unnecessary entries removed.
    struct BitmapInfoHeader
    {
      uint32 BiSize;
      int32 BiWidth;
      int32 BiHeight;
      uint16 BiBitCount;
      CompressionType BiCompression;
      uint32 BiClrUsed;
    };
    struct RGBTriple
    {
      uint8 RgbBlue;
      uint8 RgbGreen;
      uint8 RgbRed;
    };

    // The decoder that owns us.
    ScalableImageDecoder *_parent;

    // The destination for the pixel data.
    ScalableImageDecoderFrame *_buffer {nullptr};

    // The file to decode.
    RefPtr<const SharedBuffer> _data;

    // An index into |m_data| representing how much we've already decoded.
    size_t _decodedOffset;

    // The file offset at which the BMP info header starts.
    size_t _headerOffset;

    // The file offset at which the actual image bits start. When decoding
    // ICO files, this is set to 0, since it's not stored anywhere in a
    // header; the reader functions expect the image data to start
    // immediately after the header and (if necessary) color table.
    size_t _imgDataOffset;

    // The BMP info header.
    BitmapInfoHeader _infoHeader {};

    // True if this is an OS/2 1.x (aka Windows 2.x) BMP. The struct
    // layouts for this type of BMP are slightly different from the later,
    // more common formats.
    bool _isOS21x {false};

    // True if this is an OS/2 2.x BMP. The meanings of compression types 3
    // and 4 for this type of BMP differ from Windows V3+ BMPs.
    //
    // This will be falsely negative in some cases, but only ones where the
    // way we misinterpret the data is irrelevant.
    bool _isOS22x {false};

    // True if the BMP is not vertically flipped, that is, the first line of
    // raster data in the file is the top line of the image.
    bool _isTopDown {false};

    // These flags get set to false as we finish each processing stage.
    bool _needToProcessBitmasks {false};
    bool _needToProcessColorTable {false};

    // Masks/offsets for the color values for non-palette formats. These
    // are bitwise, with array entries 0, 1, 2, 3 corresponding to R, G, B,
    // A.
    //
    // The right/left shift values are meant to be applied after the masks.
    // We need to right shift to compensate for the bitfields' offsets into
    // the 32 bits of pixel data, and left shift to scale the color values
    // up for fields with less than 8 bits of precision. Sadly, we can't
    // just combine these into one shift value because the net shift amount
    // could go either direction. (If only "<< -x" were equivalent to
    // ">> x"...)
    std::array<uint32, 4> _bitMasks {};
    std::array<int, 4> _bitShiftsRight {};
    std::array<int, 4> _bitShiftsLeft {};

    // The color palette, for paletted formats.
    List<RGBTriple> _colorTable;

    // The coordinate to which we've decoded the image.
    IntPoint _coord;

    // Variables that track whether we've seen pixels with alpha values != 0
    // and == 0, respectively. See comments in processNonRLEData() on how
    // these are used.
    bool _seenNonZeroAlphaPixel {false};
    bool _seenZeroAlphaPixel {false};

    // ICOs store a 1bpp "mask" immediately after the main bitmap image data
    // (and, confusingly, add its height to the biHeight value in the info
    // header, thus doubling it). This variable tracks whether we have such
    // a mask and if we've started decoding it yet.
    AndMaskState _andMaskState;

  public:
    // |parent| is the decoder that owns us.
    // |startOffset| points to the start of the BMP within the file.
    // |buffer| points at an empty ScalableImageDecoderFrame that we'll initialize and
    // fill with decoded data.
    BMPImageReader(ScalableImageDecoder *parent, size_t decodedAndHeaderOffset, size_t imgDataOffset,
                   bool usesAndMask) noexcept;

    void SetBuffer(ScalableImageDecoderFrame *buffer) noexcept
    {
      _buffer = buffer;
    }
    void SetData(const SharedBuffer &data) noexcept
    {
      _data = ShareRefPtr(&data);
    }

    // Does the actual decoding. If |onlySize| is true, decoding only
    // progresses as far as necessary to get the image size. Returns
    // whether decoding succeeded.
    bool DecodeBMP(bool onlySize) noexcept;

  private:
    KRYS_NODISCARD uint16 ReadUint16(size_t offset) const noexcept
    {
      return ReadUint16(*_data, _decodedOffset + offset);
    }

    KRYS_NODISCARD uint32 ReadUint32(size_t offset) const noexcept
    {
      return ReadUint32(*_data, _decodedOffset + offset);
    }

    // Determines the size of the BMP info header. Returns true if the size
    // is valid.
    bool ReadInfoHeaderSize() noexcept;

    // Processes the BMP info header. Returns true if the info header could
    // be decoded.
    bool ProcessInfoHeader() noexcept;

    // Helper function for processInfoHeader() which does the actual reading
    // of header values from the byte stream. Returns false on error.
    bool ReadInfoHeader() noexcept;

    // Returns true if this is a Windows V4+ BMP.
    KRYS_NODISCARD bool IsWindowsV4Plus() const noexcept
    {
      // Windows V4 info header is 108 bytes. V5 is 124 bytes.
      return (_infoHeader.BiSize == 108) || (_infoHeader.BiSize == 124);
    }

    // Returns false if consistency errors are found in the info header.
    KRYS_NODISCARD bool IsInfoHeaderValid() const noexcept;

    // For BI_BITFIELDS images, initializes the m_bitMasks[] and
    // m_bitOffsets[] arrays. processInfoHeader() will initialize these for
    // other compression types where needed.
    bool ProcessBitmasks() noexcept;

    // For paletted images, allocates and initializes the m_colorTable[]
    // array.
    bool ProcessColorTable() noexcept;

    // Processes an RLE-encoded image. Returns true if the entire image was
    // decoded.
    bool ProcessRLEData() noexcept;

    // Processes a set of non-RLE-compressed pixels. Two cases:
    //   * inRLE = true: the data is inside an RLE-encoded bitmap. Tries to
    //     process |numPixels| pixels on the current row.
    //   * inRLE = false: the data is inside a non-RLE-encoded bitmap.
    //     |numPixels| is ignored. Expects |m_coord| to point at the
    //     beginning of the next row to be decoded. Tries to process as
    //     many complete rows as possible. Returns InsufficientData if
    //     there wasn't enough data to decode the whole image.
    //
    // This function returns a ProcessingResult instead of a bool so that it
    // can avoid calling m_parent->setFailed(), which could lead to memory
    // corruption since that will delete |this| but some callers still want
    // to access member variables after this returns.
    ProcessingResult ProcessNonRLEData(bool inRLE, int numPixels) noexcept;

    // Returns true if the current y-coordinate plus |numRows| would be past
    // the end of the image. Here "plus" means "toward the end of the
    // image", so downwards for m_isTopDown images and upwards otherwise.
    KRYS_NODISCARD bool PastEndOfImage(int numRows) const noexcept
    {
      return _isTopDown ? ((_coord.Y() + numRows) >= _parent->Size().Height()) : ((_coord.Y() - numRows) < 0);
    }

    // Returns the pixel data for the current X coordinate in a uint32_t.
    // Assumes m_decodedOffset has been set to the beginning of the current
    // row.
    // NOTE: Only as many bytes of the return value as are needed to hold
    // the pixel data will actually be set.
    KRYS_NODISCARD uint32 ReadCurrentPixel(size_t bytesPerPixel) const noexcept
    {
      const size_t offset = static_cast<size_t>(_coord.X()) * bytesPerPixel;
      switch (bytesPerPixel)
      {
        case 2: return ReadUint16(offset);

        case 3:
        {
          // It doesn't matter that we never set the most significant byte
          // of the return value here in little-endian mode, the caller
          // won't read it.
          uint32 pixel = 0;
          std::memcpy(&pixel, _data->Data() + _decodedOffset + offset, 3);
          return pixel;
        }

        case 4:  return ReadUint32(offset);

        default: krys_debug_assert(false); return 0;
      }
    }

    // Returns the value of the desired component (0, 1, 2, 3 == R, G, B, A)
    // in the given pixel data.
    KRYS_NODISCARD unsigned GetComponent(uint32 pixel, size_t component) const noexcept
    {
      return ((pixel & _bitMasks[component]) >> _bitShiftsRight[component]) << _bitShiftsLeft[component];
    }

    KRYS_NODISCARD unsigned GetAlpha(uint32 pixel) const noexcept
    {
      // For images without alpha, return alpha of 0xff.
      return _bitMasks[3] ? GetComponent(pixel, 3) : 0xff;
    }

    // Sets the current pixel to the color given by |colorIndex|. This also
    // increments the relevant local variables to move the current pixel
    // right by one.
    void SetI(size_t colorIndex) noexcept
    {
      SetPixel(_colorTable[colorIndex].RgbRed, _colorTable[colorIndex].RgbGreen,
               _colorTable[colorIndex].RgbBlue, 0xff);
    }

    // Like setI(), but with the individual component values specified.
    void SetPixel(unsigned red, unsigned green, unsigned blue, unsigned alpha) noexcept
    {
      _buffer->BackingStore()->SetPixel(_coord.X(), _coord.Y(), static_cast<uint8>(red),
                                        static_cast<uint8>(green), static_cast<uint8>(blue),
                                        static_cast<uint8>(alpha));
      _coord.Move(1, 0);
    }

    // Fills pixels from the current X-coordinate up to, but not including,
    // |endCoord| with the color given by the individual components. This
    // also increments the relevant local variables to move the current
    // pixel right to |endCoord|.
    void FillRGBA(int endCoord, unsigned red, unsigned green, unsigned blue, unsigned alpha) noexcept
    {
      if (endCoord <= _coord.X())
      {
        return;
      }
      _buffer->BackingStore()->FillRect(IntRect(_coord.X(), _coord.Y(), endCoord - _coord.X(), 1),
                                        static_cast<uint8>(red), static_cast<uint8>(green),
                                        static_cast<uint8>(blue), static_cast<uint8>(alpha));
      _coord.SetX(endCoord);
    }

    // Resets the relevant local variables to start drawing at the left edge
    // of the "next" row, where "next" is above or below the current row
    // depending on the value of |m_isTopDown|.
    void MoveBufferToNextRow() noexcept;
  };
}
