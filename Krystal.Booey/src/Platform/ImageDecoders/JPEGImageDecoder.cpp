/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 *
 * Portions are Copyright (C) 2001-6 mozilla.org
 *
 * Other contributors:
 *   Stuart Parmenter <stuart@mozilla.com>
 *
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#include "Krystal.Booey/Platform/ImageDecoders/JPEGImageDecoder.hpp"
#include <algorithm>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C"
{
#include <jpeglib.h>
}

// libjpeg reports errors with longjmp to the setjmp in the reader, which owns no objects with destructors.
// clang-format off
#pragma warning(disable : 4611) // interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4324) // structure was padded due to alignment specifier (jmp_buf)
// clang-format on

namespace krys::boo
{
  namespace
  {
    // libjpeg-turbo writes BGRA, which is ARGB on little-endian machines.
    KRYS_NODISCARD J_COLOR_SPACE RGBOutputColorSpace() noexcept
    {
      return JCS_EXT_BGRA;
    }

    KRYS_NODISCARD bool TurboSwizzled(J_COLOR_SPACE colorSpace) noexcept
    {
      return colorSpace == JCS_EXT_RGBA || colorSpace == JCS_EXT_BGRA;
    }

    KRYS_NODISCARD J_DCT_METHOD DctMethod() noexcept
    {
      return JDCT_ISLOW;
    }

    KRYS_NODISCARD J_DITHER_MODE DitherMode() noexcept
    {
      return JDITHER_FS;
    }

    KRYS_NODISCARD bool DoFancyUpsampling() noexcept
    {
      return true;
    }

    constexpr int ExifMarker = JPEG_APP0 + 1;
    constexpr int ICCMarker = JPEG_APP0 + 2;
    // An ICC APP2 block is "ICC_PROFILE\0", then its sequence number and the number of blocks.
    constexpr size_t ICCHeaderSize = 14;

    struct DecoderErrorManager
    {
      struct jpeg_error_mgr Pub; // "public" fields for IJG library
      jmp_buf SetjmpBuffer;      // For handling catastropic errors
    };

    enum class JState : uint8
    {
      Header, // Reading JFIF headers
      StartDecompress,
      DecompressProgressive, // Output progressive pixels
      DecompressSequential,  // Output sequential pixels
      Done,
      Error
    };

    void InitSource(j_decompress_ptr jd);
    boolean FillInputBuffer(j_decompress_ptr jd);
    void SkipInputData(j_decompress_ptr jd, long numBytes);
    void TermSource(j_decompress_ptr jd);
    void ErrorExit(j_common_ptr cinfo);

    /// @brief The EXIF metadata of an image of @p size, from its first APP1 block that holds it.
    KRYS_NODISCARD EXIFMetadata ReadEXIFMetadata(jpeg_decompress_struct *info, const IntSize &size) noexcept
    {
      // An EXIF APP1 block is "Exif\0", a fill byte, then the TIFF block that holds the metadata.
      // FIXME: Possibly implement XMP and IPTC support.
      constexpr size_t exifHeaderSize = 6;
      for (jpeg_saved_marker_ptr marker = info->marker_list; marker; marker = marker->next)
      {
        const JOCTET *data = marker->data;
        if (marker->marker != ExifMarker || marker->data_length < exifHeaderSize
            || std::memcmp(data, "Exif\0", 5) != 0)
        {
          continue;
        }
        return EXIFMetadata::Read(Span<const uint8>(reinterpret_cast<const uint8 *>(data) + exifHeaderSize,
                                                    marker->data_length - exifHeaderSize),
                                  size);
      }
      return {};
    }

    /// @brief The ICC profile split across the image's ICC APP2 blocks, joined in their sequence; empty if
    /// there's none, or the blocks don't make a whole profile.
    KRYS_NODISCARD List<uint8> ReadICCProfile(jpeg_decompress_struct *info) noexcept
    {
      List<jpeg_saved_marker_ptr> blocks;
      uint32 blockCount = 0;
      for (jpeg_saved_marker_ptr marker = info->marker_list; marker; marker = marker->next)
      {
        if (marker->marker != ICCMarker || marker->data_length < ICCHeaderSize
            || std::memcmp(marker->data, "ICC_PROFILE\0", 12) != 0)
        {
          continue;
        }
        const uint32 count = marker->data[13];
        if (!count || (blockCount && count != blockCount))
        {
          return {};
        }
        blockCount = count;
        blocks.push_back(marker);
      }
      if (blocks.size() != blockCount)
      {
        return {};
      }

      List<uint8> profile;
      for (uint32 sequenceNumber = 1; sequenceNumber <= blockCount; ++sequenceNumber)
      {
        auto block = std::ranges::find_if(blocks, [&](jpeg_saved_marker_ptr marker)
                                          { return marker->data[12] == sequenceNumber; });
        if (block == blocks.end())
        {
          return {};
        }
        const uint8 *data = reinterpret_cast<const uint8 *>((*block)->data);
        profile.insert(profile.end(), data + ICCHeaderSize, data + (*block)->data_length);
      }
      return profile;
    }
  }

  // Implementation of a JPEG src object that understands our state machine
  struct DecoderSourceManager
  {
    // public fields; must be first in this struct!
    struct jpeg_source_mgr Pub;

    JPEGImageReader *Decoder;
  };

  class JPEGImageReader
  {
    KRYS_NON_COPY_MOVABLE(JPEGImageReader);

  private:
    JPEGImageDecoder *_decoder;
    unsigned _bufferLength {0};
    long _bytesToSkip {0};
    bool _decodingSizeOnly {false};

    jpeg_decompress_struct _info {};
    DecoderErrorManager _err {};
    JState _state {JState::Header};

    JSAMPARRAY _samples {nullptr};

  public:
    explicit JPEGImageReader(JPEGImageDecoder *decoder) noexcept : _decoder(decoder)
    {
      // We set up the normal JPEG error routines, then override error_exit.
      _info.err = jpeg_std_error(&_err.Pub);
      _err.Pub.error_exit = ErrorExit;

      // Allocate and initialize JPEG decompression object.
      jpeg_create_decompress(&_info);

      auto *src = static_cast<DecoderSourceManager *>(std::calloc(1, sizeof(DecoderSourceManager)));
      if (!src)
      {
        _state = JState::Error;
        return;
      }

      _info.src = reinterpret_cast<jpeg_source_mgr *>(src);

      // Set up callback functions.
      src->Pub.init_source = InitSource;
      src->Pub.fill_input_buffer = FillInputBuffer;
      src->Pub.skip_input_data = SkipInputData;
      src->Pub.resync_to_restart = jpeg_resync_to_restart;
      src->Pub.term_source = TermSource;
      src->Decoder = this;

      // Keep APP1 blocks, for obtaining exif data.
      jpeg_save_markers(&_info, ExifMarker, 0xFFFF);
      // Keep APP2 blocks, for obtaining ICC profiles.
      if (!_decoder->IgnoresGammaAndColorProfile())
      {
        jpeg_save_markers(&_info, ICCMarker, 0xFFFF);
      }
    }

    ~JPEGImageReader() noexcept
    {
      Close();
    }

    void Close() noexcept
    {
      auto *src = reinterpret_cast<DecoderSourceManager *>(_info.src);
      std::free(src);
      _info.src = nullptr;

      jpeg_destroy_decompress(&_info);
    }

    void SkipBytes(long numBytes) noexcept
    {
      auto *src = reinterpret_cast<DecoderSourceManager *>(_info.src);
      long bytesToSkip = std::min(numBytes, static_cast<long>(src->Pub.bytes_in_buffer));
      src->Pub.bytes_in_buffer -= static_cast<size_t>(bytesToSkip);
      src->Pub.next_input_byte += bytesToSkip;

      _bytesToSkip = std::max(numBytes - bytesToSkip, 0l);
    }

    // NOTE: not noexcept: libjpeg's errors longjmp back to the setjmp here.
    bool Decode(const SharedBuffer &data, bool onlySize)
    {
      _decodingSizeOnly = onlySize;

      unsigned newByteCount = static_cast<unsigned>(data.Size()) - _bufferLength;
      unsigned readOffset = _bufferLength - static_cast<unsigned>(_info.src->bytes_in_buffer);

      _info.src->bytes_in_buffer += newByteCount;
      _info.src->next_input_byte = data.Data() + readOffset;

      // If we still have bytes to skip, try to skip those now.
      if (_bytesToSkip)
      {
        SkipBytes(_bytesToSkip);
      }

      _bufferLength = static_cast<unsigned>(data.Size());

      // We need to do the setjmp here. Otherwise bad things will happen
      if (setjmp(_err.SetjmpBuffer))
      {
        return _decoder->SetFailed();
      }

      switch (_state)
      {
        case JState::Header:
          // Read file parameters with jpeg_read_header().
          if (jpeg_read_header(&_info, TRUE) == JPEG_SUSPENDED)
          {
            return false; // I/O suspension.
          }

          switch (_info.jpeg_color_space)
          {
            case JCS_GRAYSCALE:
            case JCS_RGB:
            case JCS_YCbCr:
              // libjpeg can convert GRAYSCALE and YCbCr image pixels to RGB.
              _info.out_color_space = RGBOutputColorSpace();
              break;
            case JCS_CMYK:
            case JCS_YCCK:
              // libjpeg can convert YCCK to CMYK, but neither to RGB, so we
              // manually convert CMKY to RGB.
              _info.out_color_space = JCS_CMYK;
              break;
            default: return _decoder->SetFailed();
          }

          _state = JState::StartDecompress;

          // We can fill in the size now that the header is available.
          if (!_decoder->SetSize(
                IntSize(static_cast<int32>(_info.image_width), static_cast<int32>(_info.image_height))))
          {
            return false;
          }

          _decoder->SetEXIFMetadata(ReadEXIFMetadata(
            Info(), IntSize(static_cast<int32>(_info.image_width), static_cast<int32>(_info.image_height))));

          if (!_decoder->IgnoresGammaAndColorProfile())
          {
            _decoder->SetICCProfile(ReadICCProfile(Info()), _info.out_color_space == JCS_CMYK
                                                              ? ICCProfileTransform::PixelLayout::InvertedCMYK
                                                              : ICCProfileTransform::PixelLayout::BGRA);
          }

          // Don't allocate a giant and superfluous memory buffer when the
          // image is a sequential JPEG.
          _info.buffered_image = jpeg_has_multiple_scans(&_info);

          // Used to set up image size so arrays can be allocated.
          jpeg_calc_output_dimensions(&_info);

          // Make a one-row-high sample array that will go away when done with
          // image. Always make it big enough to hold an RGB row. Since this
          // uses the IJG memory manager, it must be allocated before the call
          // to jpeg_start_compress().
          // FIXME: note that some output color spaces do not need the samples
          // buffer. Remove this allocation for those color spaces.
          _samples = (*_info.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&_info), JPOOL_IMAGE,
                                                _info.output_width * 4, 1);

          if (_decodingSizeOnly)
          {
            // We can stop here. Reduce our buffer length and available data.
            _bufferLength -= static_cast<unsigned>(_info.src->bytes_in_buffer);
            _info.src->bytes_in_buffer = 0;
            return true;
          }
          [[fallthrough]];

        case JState::StartDecompress:
          // Set parameters for decompression.
          // FIXME -- Should reset dct_method and dither mode for final pass
          // of progressive JPEG.
          _info.dct_method = DctMethod();
          _info.dither_mode = DitherMode();
          _info.do_fancy_upsampling = DoFancyUpsampling() ? TRUE : FALSE;
          _info.enable_2pass_quant = FALSE;
          _info.do_block_smoothing = TRUE;

          // Start decompressor.
          if (!jpeg_start_decompress(&_info))
          {
            return false; // I/O suspension.
          }

          // If this is a progressive JPEG ...
          _state = (_info.buffered_image) ? JState::DecompressProgressive : JState::DecompressSequential;
          [[fallthrough]];

        case JState::DecompressSequential:
          if (_state == JState::DecompressSequential)
          {
            if (!_decoder->OutputScanlines())
            {
              return false; // I/O suspension.
            }

            // If we've completed image output...
            krys_debug_assert(_info.output_scanline == _info.output_height);
            _state = JState::Done;
          }
          [[fallthrough]];

        case JState::DecompressProgressive:
          if (_state == JState::DecompressProgressive)
          {
            int status;
            do
            {
              status = jpeg_consume_input(&_info);
            } while ((status != JPEG_SUSPENDED) && (status != JPEG_REACHED_EOI));

            for (;;)
            {
              if (!_info.output_scanline)
              {
                int scan = _info.input_scan_number;

                // If we haven't displayed anything yet
                // (output_scan_number == 0) and we have enough data for
                // a complete scan, force output of the last full scan.
                if (!_info.output_scan_number && (scan > 1) && (status != JPEG_REACHED_EOI))
                {
                  --scan;
                }

                if (!jpeg_start_output(&_info, scan))
                {
                  return false; // I/O suspension.
                }
              }

              if (_info.output_scanline == 0xffffff)
              {
                _info.output_scanline = 0;
              }

              // If outputScanlines() fails, it deletes |this|. Therefore,
              // copy the decoder pointer and use it to check for failure
              // to avoid member access in the failure case.
              JPEGImageDecoder *decoder = _decoder;
              if (!decoder->OutputScanlines())
              {
                if (decoder->Failed()) // Careful; |this| is deleted.
                {
                  return false;
                }
                if (!_info.output_scanline)
                {
                  // Didn't manage to read any lines - flag so we
                  // don't call jpeg_start_output() multiple times for
                  // the same scan.
                  _info.output_scanline = 0xffffff;
                }
                return false; // I/O suspension.
              }

              if (_info.output_scanline == _info.output_height)
              {
                if (!jpeg_finish_output(&_info))
                {
                  return false; // I/O suspension.
                }

                if (jpeg_input_complete(&_info) && (_info.input_scan_number == _info.output_scan_number))
                {
                  break;
                }

                _info.output_scanline = 0;
              }
            }

            _state = JState::Done;
          }
          [[fallthrough]];

        case JState::Done:
          // Finish decompression.
          return jpeg_finish_decompress(&_info);

        case JState::Error:
          // We can get here if the constructor failed.
          return _decoder->SetFailed();
      }

      return true;
    }

    KRYS_NODISCARD jpeg_decompress_struct *Info() noexcept
    {
      return &_info;
    }
    KRYS_NODISCARD JSAMPARRAY Samples() const noexcept
    {
      return _samples;
    }
    KRYS_NODISCARD JPEGImageDecoder *Decoder() noexcept
    {
      return _decoder;
    }
  };

  namespace
  {
    // Override the standard error method in the IJG JPEG decoder code.
    void ErrorExit(j_common_ptr cinfo)
    {
      // Return control to the setjmp point.
      auto *err = reinterpret_cast<DecoderErrorManager *>(cinfo->err);
      longjmp(err->SetjmpBuffer, -1);
    }

    void InitSource(j_decompress_ptr)
    {
    }

    void SkipInputData(j_decompress_ptr jd, long numBytes)
    {
      auto *src = reinterpret_cast<DecoderSourceManager *>(jd->src);
      src->Decoder->SkipBytes(numBytes);
    }

    boolean FillInputBuffer(j_decompress_ptr)
    {
      // Our decode step always sets things up properly, so if this method is ever
      // called, then we have hit the end of the buffer.  A return value of false
      // indicates that we have no data to supply yet.
      return FALSE;
    }

    void TermSource(j_decompress_ptr jd)
    {
      auto *src = reinterpret_cast<DecoderSourceManager *>(jd->src);
      src->Decoder->Decoder()->JpegComplete();
    }

    template <int TColorSpace>
    void SetPixel(ScalableImageDecoderFrame &buffer, uint32 &currentAddress, JSAMPARRAY samples,
                  int column) noexcept
    {
      JSAMPLE *jsample = *samples + column * (TColorSpace == JCS_RGB ? 3 : 4);

      switch (TColorSpace)
      {
        case JCS_RGB:
          buffer.BackingStore()->SetPixel(currentAddress, jsample[0], jsample[1], jsample[2], 0xFF);
          break;
        case JCS_CMYK:
        {
          // Source is 'Inverted CMYK', output is RGB.
          // See: http://www.easyrgb.com/math.php?MATH=M12#text12
          // Or: http://www.ilkeratalay.com/colorspacesfaq.php#rgb
          // From CMYK to CMY:
          // X =   X    * (1 -   K   ) +   K  [for X = C, M, or Y]
          // Thus, from Inverted CMYK to CMY is:
          // X = (1-iX) * (1 - (1-iK)) + (1-iK) => 1 - iX*iK
          // From CMY (0..1) to RGB (0..1):
          // R = 1 - C => 1 - (1 - iC*iK) => iC*iK  [G and B similar]
          unsigned k = jsample[3];
          buffer.BackingStore()->SetPixel(currentAddress, static_cast<uint8>(jsample[0] * k / 255),
                                          static_cast<uint8>(jsample[1] * k / 255),
                                          static_cast<uint8>(jsample[2] * k / 255), 0xFF);
          break;
        }
        default: break;
      }
    }
  }

  JPEGImageDecoder::JPEGImageDecoder(AlphaOption alphaOption,
                                     GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ScalableImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  JPEGImageDecoder::~JPEGImageDecoder() noexcept
  {
    Clear();
  }

  ScalableImageDecoderFrame *JPEGImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (index)
    {
      return nullptr;
    }

    if (_frameBufferCache.empty())
    {
      _frameBufferCache.resize(1);
    }

    auto &frame = _frameBufferCache[0];
    if (!frame.IsComplete())
    {
      Decode(false, IsAllDataReceived());
    }
    return &_frameBufferCache[0];
  }

  void JPEGImageDecoder::Clear() noexcept
  {
    _reader = nullptr;
  }

  bool JPEGImageDecoder::SetFailed() noexcept
  {
    Clear();
    return ScalableImageDecoder::SetFailed();
  }

  template <int TColorSpace>
  bool JPEGImageDecoder::OutputScanlines(ScalableImageDecoderFrame &buffer)
  {
    JSAMPARRAY samples = _reader->Samples();
    jpeg_decompress_struct *info = _reader->Info();
    int width = static_cast<int>(info->output_width);

    while (info->output_scanline < info->output_height)
    {
      // jpeg_read_scanlines will increase the scanline counter, so we
      // save the scanline before calling it.
      int sourceY = static_cast<int>(info->output_scanline);
      /* Request one scanline.  Returns 0 or 1 scanlines. */
      if (jpeg_read_scanlines(info, samples, 1) != 1)
      {
        return false;
      }

      auto *currentAddress = buffer.BackingStore()->PixelsStartingAt(0, sourceY);
      if (TColorSpace == JCS_CMYK && _iccTransform)
      {
        // The profile converts the CMYK samples to RGB.
        _transformedRow.resize(static_cast<size_t>(width) * 3u);
        _iccTransform.Transform(*samples, _transformedRow.data(), static_cast<uint32>(width));
        const uint8 *rgb = _transformedRow.data();
        for (int x = 0; x < width; ++x, ++currentAddress, rgb += 3)
        {
          buffer.BackingStore()->SetPixel(*currentAddress, rgb[0], rgb[1], rgb[2], 0xFF);
        }
        continue;
      }
      for (int x = 0; x < width; ++x, ++currentAddress)
      {
        SetPixel<TColorSpace>(buffer, *currentAddress, samples, x);
      }
    }
    return true;
  }

  bool JPEGImageDecoder::OutputScanlines()
  {
    if (_frameBufferCache.empty())
    {
      return false;
    }

    // Initialize the framebuffer if needed.
    auto &buffer = _frameBufferCache[0];
    if (buffer.IsInvalid())
    {
      if (!buffer.Initialize(Size(), _premultiplyAlpha))
      {
        return SetFailed();
      }
      buffer.SetDecodingStatus(DecodingStatus::Partial);
      // The buffer is transparent outside the decoded area while the image is
      // loading. The completed image will be marked fully opaque in jpegComplete().
      buffer.SetHasAlpha(true);
    }

    jpeg_decompress_struct *info = _reader->Info();

    if (TurboSwizzled(info->out_color_space))
    {
      while (info->output_scanline < info->output_height)
      {
        auto *row = reinterpret_cast<unsigned char *>(
          buffer.BackingStore()->PixelsStartingAt(0, static_cast<int32>(info->output_scanline)));
        if (jpeg_read_scanlines(info, &row, 1) != 1)
        {
          return false;
        }
        _iccTransform.Transform(row, row, info->output_width);
      }
      return true;
    }

    switch (info->out_color_space)
    {
      // The code inside outputScanlines<int> will be executed
      // for each pixel, so we want to avoid any extra comparisons there.
      // That is why we use template and template specializations here so
      // the proper code will be generated at compile time.
      case JCS_RGB:  return OutputScanlines<JCS_RGB>(buffer);
      case JCS_CMYK: return OutputScanlines<JCS_CMYK>(buffer);
      default:       krys_debug_assert(false); break;
    }

    return SetFailed();
  }

  void JPEGImageDecoder::SetICCProfile(Span<const uint8> profile,
                                       ICCProfileTransform::PixelLayout layout) noexcept
  {
    _iccTransform = ICCProfileTransform::Create(profile, layout,
                                                layout == ICCProfileTransform::PixelLayout::InvertedCMYK
                                                  ? ICCProfileTransform::PixelLayout::RGB
                                                  : ICCProfileTransform::PixelLayout::BGRA);
  }

  void JPEGImageDecoder::JpegComplete()
  {
    if (_frameBufferCache.empty())
    {
      return;
    }

    // Hand back an appropriately sized buffer, even if the image ended up being
    // empty.
    auto &buffer = _frameBufferCache[0];
    buffer.SetHasAlpha(false);
    buffer.SetDecodingStatus(DecodingStatus::Complete);
  }

  void JPEGImageDecoder::Decode(bool onlySize, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    if (!_reader)
    {
      Clear();
      _reader = CreateUnique<JPEGImageReader>(this);
    }

    // If we couldn't decode the image but we've received all the data, decoding
    // has failed.
    if (!_reader->Decode(*_data, onlySize) && allDataReceived)
    {
      (void)SetFailed();
    }
    // If we're done decoding the image, we don't need the JPEGImageReader
    // anymore.  (If we failed, |m_reader| has already been cleared.)
    else if (!_frameBufferCache.empty() && (_frameBufferCache[0].IsComplete()))
    {
      Clear();
    }
  }
}
