#include "Krystal.Booey/Platform/ImageDecoders/BMPImageReader.hpp"
#include <algorithm>
#include <climits>

namespace krys::boo
{
  BMPImageReader::BMPImageReader(ScalableImageDecoder *parent, size_t decodedAndHeaderOffset,
                                 size_t imgDataOffset, bool usesAndMask) noexcept
      : _parent(parent), _decodedOffset(decodedAndHeaderOffset), _headerOffset(decodedAndHeaderOffset),
        _imgDataOffset(imgDataOffset), _andMaskState(usesAndMask ? NotYetDecoded : None)
  {
    // Clue-in decodeBMP() that we need to detect the correct info header size.
  }

  bool BMPImageReader::DecodeBMP(bool onlySize) noexcept
  {
    // Calculate size of info header.
    if (!_infoHeader.BiSize && !ReadInfoHeaderSize())
    {
      return false;
    }

    const size_t headerEnd = _headerOffset + _infoHeader.BiSize;
    // Read and process info header.
    if ((_decodedOffset < headerEnd) && !ProcessInfoHeader())
    {
      return false;
    }

    // processInfoHeader() set the size, so if that's all we needed, we're done.
    if (onlySize)
    {
      return true;
    }

    // Read and process the bitmasks, if needed.
    if (_needToProcessBitmasks && !ProcessBitmasks())
    {
      return false;
    }

    // Read and process the color table, if needed.
    if (_needToProcessColorTable && !ProcessColorTable())
    {
      return false;
    }

    // Initialize the framebuffer if needed.
    krys_debug_assert(_buffer); // Parent should set this before asking us to decode!
    if (_buffer->IsInvalid())
    {
      if (!_buffer->Initialize(_parent->Size(), _parent->PremultiplyAlpha()))
      {
        return _parent->SetFailed(); // Unable to allocate.
      }

      _buffer->SetDecodingStatus(DecodingStatus::Partial);
      _buffer->SetHasAlpha(false);

      if (!_isTopDown)
      {
        _coord.SetY(_parent->Size().Height() - 1);
      }
    }

    // Decode the data.
    if ((_andMaskState != Decoding) && !PastEndOfImage(0))
    {
      if ((_infoHeader.BiCompression != RLE4) && (_infoHeader.BiCompression != RLE8)
          && (_infoHeader.BiCompression != RLE24))
      {
        const ProcessingResult result = ProcessNonRLEData(false, 0);
        if (result != Success)
        {
          return (result == Failure) ? _parent->SetFailed() : false;
        }
      }
      else if (!ProcessRLEData())
      {
        return false;
      }
    }

    // If the image has an AND mask and there was no alpha data, process the
    // mask.
    if ((_andMaskState == NotYetDecoded) && !_buffer->HasAlpha())
    {
      // Reset decoding coordinates to start of image.
      _coord.SetX(0);
      _coord.SetY(_isTopDown ? 0 : (_parent->Size().Height() - 1));

      // The AND mask is stored as 1-bit data.
      _infoHeader.BiBitCount = 1;

      _andMaskState = Decoding;
    }
    if (_andMaskState == Decoding)
    {
      const ProcessingResult result = ProcessNonRLEData(false, 0);
      if (result != Success)
      {
        return (result == Failure) ? _parent->SetFailed() : false;
      }
    }

    // Done!
    _buffer->SetDecodingStatus(DecodingStatus::Complete);
    return true;
  }

  bool BMPImageReader::ReadInfoHeaderSize() noexcept
  {
    // Get size of info header.
    krys_debug_assert(_decodedOffset == _headerOffset);
    if ((_decodedOffset > _data->Size()) || ((_data->Size() - _decodedOffset) < 4))
    {
      return false;
    }
    _infoHeader.BiSize = ReadUint32(0);
    // Don't increment m_decodedOffset here, it just makes the code in
    // processInfoHeader() more confusing.

    // Don't allow the header to overflow (which would be harmless here, but
    // problematic or at least confusing in other places), or to overrun the
    // image data.
    const size_t headerEnd = _headerOffset + _infoHeader.BiSize;
    if ((headerEnd < _headerOffset) || (_imgDataOffset && (_imgDataOffset < headerEnd)))
    {
      return _parent->SetFailed();
    }

    // See if this is a header size we understand:
    // OS/2 1.x: 12
    if (_infoHeader.BiSize == 12)
    {
      _isOS21x = true;
    }
    // Windows V3: 40
    else if ((_infoHeader.BiSize == 40) || IsWindowsV4Plus())
    {
    }
    // OS/2 2.x: any multiple of 4 between 16 and 64, inclusive, or 42 or 46
    else if ((_infoHeader.BiSize >= 16) && (_infoHeader.BiSize <= 64)
             && (!(_infoHeader.BiSize & 3) || (_infoHeader.BiSize == 42) || (_infoHeader.BiSize == 46)))
    {
      _isOS22x = true;
    }
    else
    {
      return _parent->SetFailed();
    }

    return true;
  }

  bool BMPImageReader::ProcessInfoHeader() noexcept
  {
    // Read info header.
    krys_debug_assert(_decodedOffset == _headerOffset);
    if ((_decodedOffset > _data->Size()) || ((_data->Size() - _decodedOffset) < _infoHeader.BiSize)
        || !ReadInfoHeader())
    {
      return false;
    }
    _decodedOffset += _infoHeader.BiSize;

    // Sanity-check header values.
    if (!IsInfoHeaderValid())
    {
      return _parent->SetFailed();
    }

    // Set our size.
    if (!_parent->SetSize(IntSize(_infoHeader.BiWidth, _infoHeader.BiHeight)))
    {
      return false;
    }

    // For paletted images, bitmaps can set biClrUsed to 0 to mean "all
    // colors", so set it to the maximum number of colors for this bit depth.
    // Also do this for bitmaps that put too large a value here.
    if (_infoHeader.BiBitCount < 16)
    {
      const uint32 maxColors = static_cast<uint32>(1) << _infoHeader.BiBitCount;
      if (!_infoHeader.BiClrUsed || (_infoHeader.BiClrUsed > maxColors))
      {
        _infoHeader.BiClrUsed = maxColors;
      }
    }

    // For any bitmaps that set their BitCount to the wrong value, reset the
    // counts now that we've calculated the number of necessary colors, since
    // other code relies on this value being correct.
    if (_infoHeader.BiCompression == RLE8)
    {
      _infoHeader.BiBitCount = 8;
    }
    else if (_infoHeader.BiCompression == RLE4)
    {
      _infoHeader.BiBitCount = 4;
    }

    // Tell caller what still needs to be processed.
    if (_infoHeader.BiBitCount >= 16)
    {
      _needToProcessBitmasks = true;
    }
    else if (_infoHeader.BiBitCount)
    {
      _needToProcessColorTable = true;
    }

    return true;
  }

  bool BMPImageReader::ReadInfoHeader() noexcept
  {
    // Pre-initialize some fields that not all headers set.
    _infoHeader.BiCompression = RGB;
    _infoHeader.BiClrUsed = 0;

    if (_isOS21x)
    {
      _infoHeader.BiWidth = ReadUint16(4);
      _infoHeader.BiHeight = ReadUint16(6);
      krys_debug_assert(_andMaskState == None); // ICO is a Windows format, not OS/2!
      _infoHeader.BiBitCount = ReadUint16(10);
      return true;
    }

    _infoHeader.BiWidth = static_cast<int32>(ReadUint32(4));
    _infoHeader.BiHeight = static_cast<int32>(ReadUint32(8));
    if (_andMaskState != None)
    {
      _infoHeader.BiHeight /= 2;
    }
    _infoHeader.BiBitCount = ReadUint16(14);

    // Read compression type, if present.
    if (_infoHeader.BiSize >= 20)
    {
      uint32 biCompression = ReadUint32(16);

      // Detect OS/2 2.x-specific compression types.
      if ((biCompression == 3) && (_infoHeader.BiBitCount == 1))
      {
        _infoHeader.BiCompression = HUFFMAN1D;
        _isOS22x = true;
      }
      else if ((biCompression == 4) && (_infoHeader.BiBitCount == 24))
      {
        _infoHeader.BiCompression = RLE24;
        _isOS22x = true;
      }
      else if (biCompression > 5)
      {
        return _parent->SetFailed(); // Some type we don't understand.
      }
      else
      {
        _infoHeader.BiCompression = static_cast<CompressionType>(biCompression);
      }
    }

    // Read colors used, if present.
    if (_infoHeader.BiSize >= 36)
    {
      _infoHeader.BiClrUsed = ReadUint32(32);
    }

    // Windows V4+ can safely read the four bitmasks from 40-56 bytes in, so do
    // that here.  If the bit depth is less than 16, these values will be
    // ignored by the image data decoders.  If the bit depth is at least 16 but
    // the compression format isn't BITFIELDS, these values will be ignored and
    // overwritten* in processBitmasks().
    // NOTE: We allow alpha here.  Microsoft doesn't really document this well,
    // but some BMPs appear to use it.
    //
    // For non-Windows V4+, m_bitMasks[] et. al will be initialized later
    // during processBitmasks().
    //
    // *Except the alpha channel.  Bizarrely, some RGB bitmaps expect decoders
    // to pay attention to the alpha mask here, so there's a special case in
    // processBitmasks() that doesn't always overwrite that value.
    if (IsWindowsV4Plus())
    {
      _bitMasks[0] = ReadUint32(40);
      _bitMasks[1] = ReadUint32(44);
      _bitMasks[2] = ReadUint32(48);
      _bitMasks[3] = ReadUint32(52);
    }

    // Detect top-down BMPs.
    if (_infoHeader.BiHeight < 0)
    {
      // We can't negate INT32_MIN below to get a positive int32_t.
      // isInfoHeaderValid() will reject heights of 1 << 16 or larger anyway,
      // so just reject this bitmap now.
      if (_infoHeader.BiHeight == INT32_MIN)
      {
        return _parent->SetFailed();
      }
      _isTopDown = true;
      _infoHeader.BiHeight = -_infoHeader.BiHeight;
    }

    return true;
  }

  bool BMPImageReader::IsInfoHeaderValid() const noexcept
  {
    // Non-positive widths/heights are invalid.  (We've already flipped the
    // sign of the height for top-down bitmaps.)
    if ((_infoHeader.BiWidth <= 0) || !_infoHeader.BiHeight)
    {
      return false;
    }

    // Only Windows V3+ has top-down bitmaps.
    if (_isTopDown && (_isOS21x || _isOS22x))
    {
      return false;
    }

    // Only bit depths of 1, 4, 8, or 24 are universally supported.
    if ((_infoHeader.BiBitCount != 1) && (_infoHeader.BiBitCount != 4) && (_infoHeader.BiBitCount != 8)
        && (_infoHeader.BiBitCount != 24))
    {
      // Windows V3+ additionally supports bit depths of 0 (for embedded
      // JPEG/PNG images), 16, and 32.
      if (_isOS21x || _isOS22x
          || (_infoHeader.BiBitCount && (_infoHeader.BiBitCount != 16) && (_infoHeader.BiBitCount != 32)))
      {
        return false;
      }
    }

    // Each compression type is only valid with certain bit depths (except RGB,
    // which can be used with any bit depth).  Also, some formats do not
    // some compression types.
    switch (_infoHeader.BiCompression)
    {
      case RGB:
        if (!_infoHeader.BiBitCount)
        {
          return false;
        }
        break;

      case RLE8:
        // Supposedly there are undocumented formats like "BitCount = 1,
        // Compression = RLE4" (which means "4 bit, but with a 2-color table"),
        // so also allow the paletted RLE compression types to have too low a
        // bit count; we'll correct this later.
        if (!_infoHeader.BiBitCount || (_infoHeader.BiBitCount > 8))
        {
          return false;
        }
        break;

      case RLE4:
        // See comments in RLE8.
        if (!_infoHeader.BiBitCount || (_infoHeader.BiBitCount > 4))
        {
          return false;
        }
        break;

      case BITFIELDS:
        // Only valid for Windows V3+.
        if (_isOS21x || _isOS22x || ((_infoHeader.BiBitCount != 16) && (_infoHeader.BiBitCount != 32)))
        {
          return false;
        }
        break;

      case JPEG:
      case PNG:
        // Only valid for Windows V3+.
        if (_isOS21x || _isOS22x || _infoHeader.BiBitCount)
        {
          return false;
        }
        break;

      case HUFFMAN1D:
        // Only valid for OS/2 2.x.
        if (!_isOS22x || (_infoHeader.BiBitCount != 1))
        {
          return false;
        }
        break;

      case RLE24:
        // Only valid for OS/2 2.x.
        if (!_isOS22x || (_infoHeader.BiBitCount != 24))
        {
          return false;
        }
        break;

      default:
        // Some type we don't understand.  This should have been caught in
        // readInfoHeader().
        krys_debug_assert(false);
        return false;
    }

    // Top-down bitmaps cannot be compressed; they must be RGB or BITFIELDS.
    if (_isTopDown && (_infoHeader.BiCompression != RGB) && (_infoHeader.BiCompression != BITFIELDS))
    {
      return false;
    }

    // Reject the following valid bitmap types that we don't currently bother
    // decoding.  Few other people decode these either, they're unlikely to be
    // in much use.
    // FIXME: Consider supporting these someday.
    //   * Bitmaps larger than 2^16 pixels in either dimension (Windows
    //     probably doesn't draw these well anyway, and the decoded data would
    //     take a lot of memory).
    if ((_infoHeader.BiWidth >= (1 << 16)) || (_infoHeader.BiHeight >= (1 << 16)))
    {
      return false;
    }
    // * Windows V3+ JPEG-in-BMP and PNG-in-BMP bitmaps (supposedly not found
    //   in the wild, only used to send data to printers?).
    if ((_infoHeader.BiCompression == JPEG) || (_infoHeader.BiCompression == PNG))
    {
      return false;
    }
    // * OS/2 2.x Huffman-encoded monochrome bitmaps (see
    //    http://www.fileformat.info/mirror/egff/ch09_05.htm , re: "G31D"
    //    algorithm).
    if (_infoHeader.BiCompression == HUFFMAN1D)
    {
      return false;
    }

    return true;
  }

  bool BMPImageReader::ProcessBitmasks() noexcept
  {
    // Create m_bitMasks[] values.
    if (_infoHeader.BiCompression != BITFIELDS)
    {
      // The format doesn't actually use bitmasks.  To simplify the decode
      // logic later, create bitmasks for the RGB data.  For Windows V4+,
      // this overwrites the masks we read from the header, which are
      // supposed to be ignored in non-BITFIELDS cases.
      // 16 bits:    MSB <-                     xRRRRRGG GGGBBBBB -> LSB
      // 24/32 bits: MSB <- [AAAAAAAA] RRRRRRRR GGGGGGGG BBBBBBBB -> LSB
      const int numBits = (_infoHeader.BiBitCount == 16) ? 5 : 8;
      for (int i = 0; i <= 2; ++i)
      {
        _bitMasks[static_cast<size_t>(i)] = ((static_cast<uint32>(1) << (numBits * (3 - i))) - 1)
                                            ^ ((static_cast<uint32>(1) << (numBits * (2 - i))) - 1);
      }

      // For Windows V4+ 32-bit RGB, don't overwrite the alpha mask from the
      // header (see note in readInfoHeader()).
      if (_infoHeader.BiBitCount < 32)
      {
        _bitMasks[3] = 0;
      }
      else if (!IsWindowsV4Plus())
      {
        _bitMasks[3] = static_cast<uint32>(0xff'00'00'00);
      }
    }
    else if (!IsWindowsV4Plus())
    {
      // For Windows V4+ BITFIELDS mode bitmaps, this was already done when
      // we read the info header.

      // Fail if we don't have enough file space for the bitmasks.
      const size_t headerEnd = _headerOffset + _infoHeader.BiSize;
      constexpr size_t bitmasksSize = 12;
      const size_t bitmasksEnd = headerEnd + bitmasksSize;
      if ((bitmasksEnd < headerEnd) || (_imgDataOffset && (_imgDataOffset < bitmasksEnd)))
      {
        return _parent->SetFailed();
      }

      // Read bitmasks.
      if ((_data->Size() - _decodedOffset) < bitmasksSize)
      {
        return false;
      }
      _bitMasks[0] = ReadUint32(0);
      _bitMasks[1] = ReadUint32(4);
      _bitMasks[2] = ReadUint32(8);
      // No alpha in anything other than Windows V4+.
      _bitMasks[3] = 0;

      _decodedOffset += bitmasksSize;
    }

    // We've now decoded all the non-image data we care about.  Skip anything
    // else before the actual raster data.
    if (_imgDataOffset)
    {
      _decodedOffset = _imgDataOffset;
    }
    _needToProcessBitmasks = false;

    // Check masks and set shift values.
    for (size_t i = 0; i < 4; ++i)
    {
      // Trim the mask to the allowed bit depth.  Some Windows V4+ BMPs
      // specify a bogus alpha channel in bits that don't exist in the pixel
      // data (for example, bits 25-31 in a 24-bit RGB format).
      if (_infoHeader.BiBitCount < 32)
      {
        _bitMasks[i] &= ((static_cast<uint32>(1) << _infoHeader.BiBitCount) - 1);
      }

      // For empty masks (common on the alpha channel, especially after the
      // trimming above), quickly clear the shifts and continue, to avoid an
      // infinite loop in the counting code below.
      uint32 tempMask = _bitMasks[i];
      if (!tempMask)
      {
        _bitShiftsRight[i] = _bitShiftsLeft[i] = 0;
        continue;
      }

      // Make sure bitmask does not overlap any other bitmasks.
      for (size_t j = 0; j < i; ++j)
      {
        if (tempMask & _bitMasks[j])
        {
          return _parent->SetFailed();
        }
      }

      // Count offset into pixel data.
      for (_bitShiftsRight[i] = 0; !(tempMask & 1); tempMask >>= 1)
      {
        ++_bitShiftsRight[i];
      }

      // Count size of mask.
      for (_bitShiftsLeft[i] = 8; tempMask & 1; tempMask >>= 1)
      {
        --_bitShiftsLeft[i];
      }

      // Make sure bitmask is contiguous.
      if (tempMask)
      {
        return _parent->SetFailed();
      }

      // Since RGBABuffer tops out at 8 bits per channel, adjust the shift
      // amounts to use the most significant 8 bits of the channel.
      if (_bitShiftsLeft[i] < 0)
      {
        _bitShiftsRight[i] -= _bitShiftsLeft[i];
        _bitShiftsLeft[i] = 0;
      }
    }

    return true;
  }

  bool BMPImageReader::ProcessColorTable() noexcept
  {
    // Fail if we don't have enough file space for the color table.
    const size_t headerEnd = _headerOffset + _infoHeader.BiSize;
    const size_t tableSizeInBytes = static_cast<size_t>(_infoHeader.BiClrUsed) * (_isOS21x ? 3 : 4);
    const size_t tableEnd = headerEnd + tableSizeInBytes;
    if ((tableEnd < headerEnd) || (_imgDataOffset && (_imgDataOffset < tableEnd)))
    {
      return _parent->SetFailed();
    }

    // Read color table.
    if ((_decodedOffset > _data->Size()) || ((_data->Size() - _decodedOffset) < tableSizeInBytes))
    {
      return false;
    }
    _colorTable.clear();
    _colorTable.reserve(_infoHeader.BiClrUsed);
    const uint8 *bytes = _data->Data();
    for (uint32 i = 0; i < _infoHeader.BiClrUsed; ++i)
    {
      RGBTriple triple {};
      triple.RgbBlue = bytes[_decodedOffset++];
      triple.RgbGreen = bytes[_decodedOffset++];
      triple.RgbRed = bytes[_decodedOffset++];
      // Skip padding byte (not present on OS/2 1.x).
      if (!_isOS21x)
      {
        ++_decodedOffset;
      }
      _colorTable.push_back(triple);
    }

    // We've now decoded all the non-image data we care about.  Skip anything
    // else before the actual raster data.
    if (_imgDataOffset)
    {
      _decodedOffset = _imgDataOffset;
    }
    _needToProcessColorTable = false;

    return true;
  }

  bool BMPImageReader::ProcessRLEData() noexcept
  {
    if (_decodedOffset > _data->Size())
    {
      return false;
    }

    // RLE decoding is poorly specified.  Two main problems:
    // (1) Are EOL markers necessary?  What happens when we have too many
    //     pixels for one row?
    //     http://www.fileformat.info/format/bmp/egff.htm says extra pixels
    //     should wrap to the next line.  Real BMPs I've encountered seem to
    //     instead expect extra pixels to be ignored until the EOL marker is
    //     seen, although this has only happened in a few cases and I suspect
    //     those BMPs may be invalid.  So we only change lines on EOL (or Delta
    //     with dy > 0), and fail in most cases when pixels extend past the end
    //     of the line.
    // (2) When Delta, EOL, or EOF are seen, what happens to the "skipped"
    //     pixels?
    //     http://www.daubnet.com/formats/BMP.html says these should be filled
    //     with color 0.  However, the "do nothing" and "don't care" comments
    //     of other references suggest leaving these alone, i.e. letting them
    //     be transparent to the background behind the image.  This seems to
    //     match how MSPAINT treats BMPs, so we do that.  Note that when we
    //     actually skip pixels for a case like this, we need to note on the
    //     framebuffer that we have alpha.

    // Impossible to decode row-at-a-time, so just do things as a stream of
    // bytes.
    while (true)
    {
      // Every entry takes at least two bytes; bail if there isn't enough
      // data.
      if ((_data->Size() - _decodedOffset) < 2)
      {
        return false;
      }

      // For every entry except EOF, we'd better not have reached the end of
      // the image.
      const uint8 *bytes = _data->Data();
      const uint8 count = bytes[_decodedOffset];
      const uint8 code = bytes[_decodedOffset + 1];
      if ((count || (code != 1)) && PastEndOfImage(0))
      {
        return _parent->SetFailed();
      }

      // Decode.
      if (!count)
      {
        switch (code)
        {
          case 0: // Magic token: EOL
            // Skip any remaining pixels in this row.
            if (_coord.X() < _parent->Size().Width())
            {
              _buffer->SetHasAlpha(true);
            }
            MoveBufferToNextRow();

            _decodedOffset += 2;
            break;

          case 1: // Magic token: EOF
            // Skip any remaining pixels in the image.
            if ((_coord.X() < _parent->Size().Width())
                || (_isTopDown ? (_coord.Y() < (_parent->Size().Height() - 1)) : (_coord.Y() > 0)))
            {
              _buffer->SetHasAlpha(true);
            }
            return true;

          case 2:
          { // Magic token: Delta
            // The next two bytes specify dx and dy.  Bail if there isn't
            // enough data.
            if ((_data->Size() - _decodedOffset) < 4)
            {
              return false;
            }

            // Fail if this takes us past the end of the desired row or
            // past the end of the image.
            const uint8 dx = bytes[_decodedOffset + 2];
            const uint8 dy = bytes[_decodedOffset + 3];
            if (dx || dy)
            {
              _buffer->SetHasAlpha(true);
            }
            if (((_coord.X() + dx) > _parent->Size().Width()) || PastEndOfImage(dy))
            {
              return _parent->SetFailed();
            }

            // Skip intervening pixels.
            _coord.Move(dx, _isTopDown ? dy : -dy);

            _decodedOffset += 4;
            break;
          }

          default:
          { // Absolute mode
            // |code| pixels specified as in BI_RGB, zero-padded at the end
            // to a multiple of 16 bits.
            // Because processNonRLEData() expects m_decodedOffset to
            // point to the beginning of the pixel data, bump it past
            // the escape bytes and then reset if decoding failed.
            _decodedOffset += 2;
            const ProcessingResult result = ProcessNonRLEData(true, code);
            if (result == Failure)
            {
              return _parent->SetFailed();
            }
            if (result == InsufficientData)
            {
              _decodedOffset -= 2;
              return false;
            }
            break;
          }
        }
      }
      else
      { // Encoded mode
        // The following color data is repeated for |count| total pixels.
        // Strangely, some BMPs seem to specify excessively large counts
        // here; ignore pixels past the end of the row.
        const int endX = std::min(_coord.X() + count, _parent->Size().Width());

        if (_infoHeader.BiCompression == RLE24)
        {
          // Bail if there isn't enough data.
          if ((_data->Size() - _decodedOffset) < 4)
          {
            return false;
          }

          // One BGR triple that we copy |count| times.
          FillRGBA(endX, bytes[_decodedOffset + 3], bytes[_decodedOffset + 2], code, 0xff);
          _decodedOffset += 4;
        }
        else
        {
          // RLE8 has one color index that gets repeated; RLE4 has two
          // color indexes in the upper and lower 4 bits of the byte,
          // which are alternated.
          std::array<size_t, 2> colorIndexes {code, code};
          if (_infoHeader.BiCompression == RLE4)
          {
            colorIndexes[0] = (colorIndexes[0] >> 4) & 0xf;
            colorIndexes[1] &= 0xf;
          }
          if ((colorIndexes[0] >= _infoHeader.BiClrUsed) || (colorIndexes[1] >= _infoHeader.BiClrUsed))
          {
            return _parent->SetFailed();
          }
          for (size_t which = 0; _coord.X() < endX;)
          {
            SetI(colorIndexes[which]);
            which = !which;
          }

          _decodedOffset += 2;
        }
      }
    }
  }

  BMPImageReader::ProcessingResult BMPImageReader::ProcessNonRLEData(bool inRLE, int numPixels) noexcept
  {
    if (_decodedOffset > _data->Size())
    {
      return InsufficientData;
    }

    if (!inRLE)
    {
      numPixels = _parent->Size().Width();
    }

    // Fail if we're being asked to decode more pixels than remain in the row.
    const int endX = _coord.X() + numPixels;
    if (endX > _parent->Size().Width())
    {
      return Failure;
    }

    // Determine how many bytes of data the requested number of pixels
    // requires.
    const size_t pixelsPerByte = 8 / _infoHeader.BiBitCount;
    const size_t bytesPerPixel = _infoHeader.BiBitCount / 8;
    const size_t unpaddedNumBytes = (_infoHeader.BiBitCount < 16)
                                      ? ((static_cast<size_t>(numPixels) + pixelsPerByte - 1) / pixelsPerByte)
                                      : (static_cast<size_t>(numPixels) * bytesPerPixel);
    // RLE runs are zero-padded at the end to a multiple of 16 bits.  Non-RLE
    // data is in rows and is zero-padded to a multiple of 32 bits.
    const size_t alignBits = inRLE ? 1 : 3;
    const size_t paddedNumBytes = (unpaddedNumBytes + alignBits) & ~alignBits;

    // Decode as many rows as we can.  (For RLE, where we only want to decode
    // one row, we've already checked that this condition is true.)
    while (!PastEndOfImage(0))
    {
      // Bail if we don't have enough data for the desired number of pixels.
      if ((_data->Size() - _decodedOffset) < paddedNumBytes)
      {
        return InsufficientData;
      }

      if (_infoHeader.BiBitCount < 16)
      {
        // Paletted data.  Pixels are stored little-endian within bytes.
        // Decode pixels one byte at a time, left to right (so, starting at
        // the most significant bits in the byte).
        const uint8 mask = static_cast<uint8>((1 << _infoHeader.BiBitCount) - 1);
        for (size_t byte = 0; byte < unpaddedNumBytes; ++byte)
        {
          uint8 pixelData = _data->Data()[_decodedOffset + byte];
          for (size_t pixel = 0; (pixel < pixelsPerByte) && (_coord.X() < endX); ++pixel)
          {
            const size_t colorIndex = (pixelData >> (8 - _infoHeader.BiBitCount)) & mask;
            if (_andMaskState == Decoding)
            {
              // There's no way to accurately represent an AND + XOR
              // operation as an RGBA image, so where the AND values
              // are 1, we simply set the framebuffer pixels to fully
              // transparent, on the assumption that most ICOs on the
              // web will not be doing a lot of inverting.
              if (colorIndex)
              {
                SetPixel(0, 0, 0, 0);
                _buffer->SetHasAlpha(true);
              }
              else
              {
                _coord.Move(1, 0);
              }
            }
            else
            {
              if (colorIndex >= _infoHeader.BiClrUsed)
              {
                return Failure;
              }
              SetI(colorIndex);
            }
            pixelData = static_cast<uint8>(pixelData << _infoHeader.BiBitCount);
          }
        }
      }
      else
      {
        // RGB data.  Decode pixels one at a time, left to right.
        while (_coord.X() < endX)
        {
          const uint32 pixel = ReadCurrentPixel(bytesPerPixel);

          // Some BMPs specify an alpha channel but don't actually use it
          // (it contains all 0s).  To avoid displaying these images as
          // fully-transparent, decode as if images are fully opaque
          // until we actually see a non-zero alpha value; at that point,
          // reset any previously-decoded pixels to fully transparent and
          // continue decoding based on the real alpha channel values.
          // As an optimization, avoid setting "hasAlpha" to true for
          // images where all alpha values are 255; opaque images are
          // faster to draw.
          unsigned alpha = GetAlpha(pixel);
          if (!_seenNonZeroAlphaPixel && !alpha)
          {
            _seenZeroAlphaPixel = true;
            alpha = 255;
          }
          else
          {
            _seenNonZeroAlphaPixel = true;
            if (_seenZeroAlphaPixel)
            {
              _buffer->BackingStore()->Clear();
              _buffer->SetHasAlpha(true);
              _seenZeroAlphaPixel = false;
            }
            else if (alpha != 255)
            {
              _buffer->SetHasAlpha(true);
            }
          }

          SetPixel(GetComponent(pixel, 0), GetComponent(pixel, 1), GetComponent(pixel, 2), alpha);
        }
      }

      // Success, keep going.
      _decodedOffset += paddedNumBytes;
      if (inRLE)
      {
        return Success;
      }
      MoveBufferToNextRow();
    }

    // Finished decoding whole image.
    return Success;
  }

  void BMPImageReader::MoveBufferToNextRow() noexcept
  {
    _coord.Move(-_coord.X(), _isTopDown ? 1 : -1);
  }
}
