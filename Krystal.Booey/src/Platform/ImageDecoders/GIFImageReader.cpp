/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Chris Saari <saari@netscape.com>
 *   Apple Inc.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/*
The Graphics Interchange Format(c) is the copyright property of CompuServe
Incorporated. Only CompuServe Incorporated is authorized to define, redefine,
enhance, alter, modify or change in any way the definition of the format.

CompuServe Incorporated hereby grants a limited, non-exclusive, royalty-free
license for the use of the Graphics Interchange Format(sm) in computer
software; computer software utilizing GIF(sm) must acknowledge ownership of the
Graphics Interchange Format and its Service Mark by CompuServe Incorporated, in
User and Technical Documentation. Computer software utilizing GIF, which is
distributed or may be distributed without User or Technical Documentation must
display to the screen or printer a message acknowledging ownership of the
Graphics Interchange Format and the Service Mark by CompuServe Incorporated; in
this case, the acknowledgement may be displayed in an opening screen or leading
banner, or a closing screen or trailing banner. A message such as the following
may be used:

    "The Graphics Interchange Format(c) is the Copyright property of
    CompuServe Incorporated. GIF(sm) is a Service Mark property of
    CompuServe Incorporated."

For further information, please contact :

    CompuServe Incorporated
    Graphics Technology Department
    5000 Arlington Center Boulevard
    Columbus, Ohio  43220
    U. S. A.

CompuServe Incorporated maintains a mailing list with all those individuals and
organizations who wish to receive copies of this document when it is corrected
or revised. This service is offered free of charge; please provide us with your
mailing address.
*/

#include "Krystal.Booey/Platform/ImageDecoders/GIFImageReader.hpp"
#include <algorithm>
#include <cstring>

namespace krys::boo
{
  namespace
  {
    // Get a 16-bit value stored in little-endian format.
    KRYS_NODISCARD unsigned GetInt16(const uint8 *p) noexcept
    {
      return static_cast<unsigned>(p[1] << 8 | p[0]);
    }

    KRYS_NODISCARD bool HasPrefix(Span<const uint8> data, const char *prefix) noexcept
    {
      size_t length = std::strlen(prefix);
      return data.size() >= length && !std::memcmp(data.data(), prefix, length);
    }
  }

  // GETN(n, s) requests at least 'n' bytes available from 'q', at start of state 's'.
  //
  // Note, the hold will never need to be bigger than 256 bytes to gather up in the hold,
  // as each GIF block (except colormaps) can never be bigger than 256 bytes.
  // Colormaps are directly copied in the resp. global_colormap or dynamically allocated local_colormap.
  // So a fixed buffer in GIFImageReader is good enough.
  // This buffer is only needed to copy left-over data from one GifWrite call to the next
#define KRYS_GIF_GETN(n, s)                                                                                  \
  do                                                                                                         \
  {                                                                                                          \
    _bytesToConsume = (n);                                                                                   \
    _state = (s);                                                                                            \
  } while (false)

  // Send the data to the display front-end.
  bool GIFLZWContext::OutputRow() noexcept
  {
    int drowStart = static_cast<int>(_irow);
    int drowEnd = static_cast<int>(_irow);

    // Haeberli-inspired hack for interlaced GIFs: Replicate lines while
    // displaying to diminish the "venetian-blind" effect as the image is
    // loaded. Adjust pixel vertical positions to avoid the appearance of the
    // image crawling up the screen as successive passes are drawn.
    if (_frameContext->ProgressiveDisplay && _frameContext->Interlaced && _ipass < 4)
    {
      unsigned rowDup = 0;
      unsigned rowShift = 0;

      switch (_ipass)
      {
        case 1:
          rowDup = 7;
          rowShift = 3;
          break;
        case 2:
          rowDup = 3;
          rowShift = 1;
          break;
        case 3:
          rowDup = 1;
          rowShift = 0;
          break;
        default: break;
      }

      drowStart -= static_cast<int>(rowShift);
      drowEnd = drowStart + static_cast<int>(rowDup);

      // Extend if bottom edge isn't covered because of the shift upward.
      if (((_frameContext->Height - 1) - static_cast<unsigned>(drowEnd)) <= rowShift)
      {
        drowEnd = static_cast<int>(_frameContext->Height) - 1;
      }

      // Clamp first and last rows to upper and lower edge of image.
      if (drowStart < 0)
      {
        drowStart = 0;
      }

      if (static_cast<unsigned>(drowEnd) >= _frameContext->Height)
      {
        drowEnd = static_cast<int>(_frameContext->Height) - 1;
      }
    }

    // Protect against too much image data.
    if (static_cast<unsigned>(drowStart) >= _frameContext->Height)
    {
      return true;
    }

    // CALLBACK: Let the client know we have decoded a row.
    if (!_client->HaveDecodedRow(
          static_cast<unsigned>(_frameContext->FrameId), _rowBuffer, _frameContext->Width,
          static_cast<size_t>(drowStart), static_cast<unsigned>(drowEnd - drowStart + 1),
          _frameContext->ProgressiveDisplay && _frameContext->Interlaced && _ipass > 1))
    {
      return false;
    }

    if (!_frameContext->Interlaced)
    {
      _irow++;
    }
    else
    {
      do
      {
        switch (_ipass)
        {
          case 1:
            _irow += 8;
            if (_irow >= _frameContext->Height)
            {
              _ipass++;
              _irow = 4;
            }
            break;

          case 2:
            _irow += 8;
            if (_irow >= _frameContext->Height)
            {
              _ipass++;
              _irow = 2;
            }
            break;

          case 3:
            _irow += 4;
            if (_irow >= _frameContext->Height)
            {
              _ipass++;
              _irow = 1;
            }
            break;

          case 4:
            _irow += 2;
            if (_irow >= _frameContext->Height)
            {
              _ipass++;
              _irow = 0;
            }
            break;

          default: break;
        }
      } while (_irow > (_frameContext->Height - 1));
    }
    return true;
  }

  // Perform Lempel-Ziv-Welch decoding.
  // Returns true if decoding was successful. In this case the block will have been completely consumed and/or
  // rowsRemaining will be 0. Otherwise, decoding failed; returns false in this case, which will always cause
  // the GIFImageReader to set the "decode failed" flag.
  bool GIFLZWContext::DoLZW(Span<const uint8> block) noexcept
  {
    if (_rowPosition == _rowBuffer.size())
    {
      return true;
    }

#define KRYS_GIF_OUTPUT_ROW                                                                                  \
  do                                                                                                         \
  {                                                                                                          \
    if (!OutputRow())                                                                                        \
    {                                                                                                        \
      return false;                                                                                          \
    }                                                                                                        \
    _rowsRemaining--;                                                                                        \
    _rowPosition = 0;                                                                                        \
    if (!_rowsRemaining)                                                                                     \
    {                                                                                                        \
      return true;                                                                                           \
    }                                                                                                        \
  } while (0)

    for (auto ch : block)
    {
      // Feed the next byte into the decoder's 32-bit input buffer.
      _datum += static_cast<int>(ch) << _bits;
      _bits += 8;

      // Check for underflow of decoder's 32-bit input buffer.
      while (_bits >= _codesize)
      {
        // Get the leading variable-length symbol from the data stream.
        int code = _datum & _codemask;
        _datum >>= _codesize;
        _bits -= _codesize;

        // Reset the dictionary to its original state, if requested.
        if (code == _clearCode)
        {
          _codesize = _frameContext->Datasize + 1;
          _codemask = (1 << _codesize) - 1;
          _avail = _clearCode + 2;
          _oldcode = -1;
          continue;
        }

        // Check for explicit end-of-stream code.
        if (code == (_clearCode + 1))
        {
          // end-of-stream should only appear after all image data.
          if (!_rowsRemaining)
          {
            return true;
          }
          return false;
        }

        if (_oldcode == -1)
        {
          _rowBuffer[_rowPosition++] = _suffix[static_cast<size_t>(code)];
          if (_rowPosition == _rowBuffer.size())
          {
            KRYS_GIF_OUTPUT_ROW;
          }

          _firstchar = static_cast<unsigned char>(code);
          _oldcode = code;
          continue;
        }

        int incode = code;
        if (code >= _avail)
        {
          _stack[_stackp++] = _firstchar;
          code = _oldcode;

          if (_stackp == MaxBytes)
          {
            return false;
          }
        }

        while (code >= _clearCode)
        {
          if (code >= MaxBytes || code == _prefix[static_cast<size_t>(code)])
          {
            return false;
          }

          // Even though suffix[] only holds characters through suffix[avail - 1],
          // allowing code >= avail here lets us be more tolerant of malformed
          // data. As long as code < MAX_BYTES, the only risk is a garbled image,
          // which is no worse than refusing to display it.
          _stack[_stackp++] = _suffix[static_cast<size_t>(code)];
          code = _prefix[static_cast<size_t>(code)];

          if (_stackp == MaxBytes)
          {
            return false;
          }
        }

        _firstchar = _suffix[static_cast<size_t>(code)];
        _stack[_stackp++] = _firstchar;

        // Define a new codeword in the dictionary.
        if (_avail < 4'096)
        {
          _prefix[static_cast<size_t>(_avail)] = static_cast<unsigned short>(_oldcode);
          _suffix[static_cast<size_t>(_avail)] = _firstchar;
          _avail++;

          // If we've used up all the codewords of a given length
          // increase the length of codewords by one bit, but don't
          // exceed the specified maximum codeword size of 12 bits.
          if ((!(_avail & _codemask)) && (_avail < 4'096))
          {
            _codesize++;
            _codemask += _avail;
          }
        }
        _oldcode = incode;

        // Copy the decoded data out to the scanline buffer.
        do
        {
          _rowBuffer[_rowPosition++] = _stack[--_stackp];
          if (_rowPosition == _rowBuffer.size())
          {
            KRYS_GIF_OUTPUT_ROW;
          }
        } while (_stackp > 0);
      }
    }

    return true;
#undef KRYS_GIF_OUTPUT_ROW
  }

  // Perform decoding for this frame. frameDecoded will be true if the entire frame is decoded.
  // Returns false if a decoding error occurred. This is a fatal error and causes the GIFImageReader to set
  // the "decode failed" flag. Otherwise, either not enough data is available to decode further than before,
  // or the new data has been decoded successfully; returns true in this case.
  bool GIFFrameContext::Decode(Span<const uint8> data, GIFImageDecoder *client, bool &frameDecoded) noexcept
  {
    frameDecoded = false;
    if (!_lzwContext)
    {
      // Wait for more data to properly initialize GIFLZWContext.
      if (!IsDataSizeDefined() || !IsHeaderDefined())
      {
        return true;
      }

      _lzwContext = CreateUnique<GIFLZWContext>(client, this);
      if (!_lzwContext->PrepareToDecode())
      {
        _lzwContext = nullptr;
        return false;
      }

      _currentLzwBlock = 0;
    }

    // Some bad GIFs have extra blocks beyond the last row, which we don't want to decode.
    while (_currentLzwBlock < _lzwBlocks.size() && _lzwContext->HasRemainingRows())
    {
      size_t blockPosition = _lzwBlocks[_currentLzwBlock].BlockPosition;
      size_t blockSize = _lzwBlocks[_currentLzwBlock].BlockSize;
      if (data.size() < blockSize || blockPosition > data.size() - blockSize)
      {
        return false;
      }
      if (!_lzwContext->DoLZW(data.subspan(blockPosition, blockSize)))
      {
        return false;
      }
      ++_currentLzwBlock;
    }

    // If this frame is data complete then the previous loop must have completely decoded all LZW blocks.
    // There will be no more decoding for this frame so it's time to cleanup.
    if (IsComplete())
    {
      frameDecoded = true;
      _lzwContext = nullptr;
    }
    return true;
  }

  // Decode all frames before haltAtFrame.
  // This method uses GIFFrameContext:decode() to decode each frame; decoding error is reported to client as a
  // critical failure. Return true if decoding has progressed. Return false if an error has occurred.
  bool GIFImageReader::Decode(GIFImageDecoder::GIFQuery query, unsigned haltAtFrame) noexcept
  {
    krys_debug_assert(_bytesRead <= _data->Size());

    if (!Parse(_bytesRead, _data->Size() - _bytesRead, query == GIFImageDecoder::GIFSizeQuery))
    {
      return false;
    }

    if (query != GIFImageDecoder::GIFFullQuery)
    {
      return true;
    }

    // Already decoded frames can be deleted from the cache and then they require to be decoded again, so
    // the haltAtFrame value we receive here may be lower than m_currentDecodingFrame. In this case
    // we position m_currentDecodingFrame to haltAtFrame - 1 and decode from there.
    // See bug https://bugs.webkit.org/show_bug.cgi?id=176089.
    _currentDecodingFrame = std::min(_currentDecodingFrame, static_cast<size_t>(haltAtFrame) - 1);

    while (_currentDecodingFrame < std::min(_frames.size(), static_cast<size_t>(haltAtFrame)))
    {
      bool frameDecoded = false;
      GIFFrameContext *currentFrame = _frames[_currentDecodingFrame].get();

      if (!currentFrame->Decode(Data(0, _data->Size()), _client, frameDecoded))
      {
        return false;
      }

      // We need more data to continue decoding.
      if (!frameDecoded)
      {
        break;
      }

      if (!_client->FrameComplete(static_cast<unsigned>(_currentDecodingFrame), currentFrame->DelayTime,
                                  currentFrame->DisposalMethod))
      {
        return false;
      }
      ++_currentDecodingFrame;
    }

    // All frames decoded.
    if (_currentDecodingFrame == _frames.size() && _parseCompleted)
    {
      _client->GifComplete();
    }
    return true;
  }

  // Parse incoming GIF data stream into internal data structures.
  // Return true if parsing has progressed or there is not enough data.
  // Return false if a fatal error is encountered.
  bool GIFImageReader::Parse(size_t dataPosition, size_t len, bool parseSizeOnly) noexcept
  {
    if (!len)
    {
      // No new data has come in since the last call, just ignore this call.
      return true;
    }

    if (len < _bytesToConsume)
    {
      return true;
    }

    // This loop reads as many components from |m_data| as possible.
    // At the beginning of each iteration, dataPosition will be advanced by m_bytesToConsume to
    // point to the next component. len will be decremented accordingly.
    while (len >= _bytesToConsume)
    {
      const size_t currentComponentPosition = dataPosition;
      auto currentComponent = Data(dataPosition, _bytesToConsume);

      // Mark the current component as consumed. Note that currentComponent will remain pointed at this
      // component until the next loop iteration.
      dataPosition += _bytesToConsume;
      len -= _bytesToConsume;

      switch (_state)
      {
        case GIFLZW:
          krys_debug_assert(!_frames.empty());
          // m_bytesToConsume is the current component size because it hasn't been updated.
          _frames.back()->AddLzwBlock(currentComponentPosition, _bytesToConsume);
          KRYS_GIF_GETN(1, GIFSubBlock);
          break;

        case GIFLZWStart:
        {
          krys_debug_assert(!_frames.empty());
          _frames.back()->SetDataSize(currentComponent.front());
          KRYS_GIF_GETN(1, GIFSubBlock);
          break;
        }

        case GIFType:
        {
          // All GIF files begin with "GIF87a" or "GIF89a".
          if (HasPrefix(currentComponent, "GIF89a"))
          {
            _version = 89;
          }
          else if (HasPrefix(currentComponent, "GIF87a"))
          {
            _version = 87;
          }
          else
          {
            return false;
          }
          KRYS_GIF_GETN(7, GIFGlobalHeader);
          break;
        }

        case GIFGlobalHeader:
        {
          // This is the height and width of the "screen" or frame into which images are rendered. The
          // individual images can be smaller than the screen size and located with an origin anywhere
          // within the screen.
          _screenWidth = GetInt16(currentComponent.data());
          _screenHeight = GetInt16(currentComponent.data() + 2);

          // CALLBACK: Inform the decoderplugin of our size.
          // Note: A subsequent frame might have dimensions larger than the "screen" dimensions.
          if (_client
              && !_client->SetSize(
                IntSize(static_cast<int32>(_screenWidth), static_cast<int32>(_screenHeight))))
          {
            return false;
          }

          _screenBgcolor = currentComponent[5];
          _globalColormapSize = 2 << (currentComponent[4] & 0x07);

          if ((currentComponent[4] & 0x80) && _globalColormapSize > 0)
          { /* global map */
            // Get the global colormap
            const size_t globalColormapBytes = 3 * static_cast<size_t>(_globalColormapSize);
            _globalColormapPosition = dataPosition;

            if (len < globalColormapBytes)
            {
              // Wait until we have enough bytes to consume the entire colormap at once.
              KRYS_GIF_GETN(globalColormapBytes, GIFGlobalColormap);
              break;
            }

            _isGlobalColormapDefined = true;
            dataPosition += globalColormapBytes;
            len -= globalColormapBytes;
          }

          KRYS_GIF_GETN(1, GIFImageStart);

          // currentComponent[6] = Pixel Aspect Ratio
          //   Not used
          //   float aspect = (float)((currentComponent[6] + 15) / 64.0);
          break;
        }

        case GIFGlobalColormap:
        {
          _isGlobalColormapDefined = true;
          KRYS_GIF_GETN(1, GIFImageStart);
          break;
        }

        case GIFImageStart:
        {
          if (currentComponent.front() == ',')
          { // image separator.
            KRYS_GIF_GETN(9, GIFImageHeader);
            break;
          }

          if (currentComponent.front() == '!')
          { // extension.
            KRYS_GIF_GETN(2, GIFExtension);
            break;
          }

          // If we get anything other than ',' (image separator), '!'
          // (extension), or ';' (trailer), there is extraneous data
          // between blocks. The GIF87a spec tells us to keep reading
          // until we find an image separator, but GIF89a says such
          // a file is corrupt. We follow major browsers' implementation
          // and proceed as if the file were correctly terminated, so the
          // GIF will display.
          KRYS_GIF_GETN(0, GIFDone);
          break;
        }

        case GIFExtension:
        {
          size_t bytesInBlock = currentComponent[1];
          GIFState es = GIFSkipBlock;

          switch (currentComponent.front())
          {
            case 0xf9:
              es = GIFControlExtension;
              // The GIF spec mandates that the GIFControlExtension header block length is 4 bytes,
              // and the parser for this block reads 4 bytes, so we must enforce that the buffer
              // contains at least this many bytes. If the GIF specifies a different length, we
              // allow that, so long as it's larger; the additional data will simply be ignored.
              bytesInBlock = std::max(bytesInBlock, static_cast<size_t>(4));
              break;

            // The GIF spec also specifies the lengths of the following two extensions' headers
            // (as 12 and 11 bytes, respectively). Because we ignore the plain text extension entirely
            // and sanity-check the actual length of the application extension header before reading it,
            // we allow GIFs to deviate from these values in either direction. This is important for
            // real-world compatibility, as GIFs in the wild exist with application extension headers
            // that are both shorter and longer than 11 bytes.
            case 0x01:
              // ignoring plain text extension
              break;

            case 0xff: es = GIFApplicationExtension; break;

            case 0xfe: es = GIFConsumeComment; break;

            default:   break;
          }

          if (bytesInBlock)
          {
            KRYS_GIF_GETN(bytesInBlock, es);
          }
          else
          {
            KRYS_GIF_GETN(1, GIFImageStart);
          }
          break;
        }

        case GIFConsumeBlock:
        {
          if (!currentComponent.front())
          {
            KRYS_GIF_GETN(1, GIFImageStart);
          }
          else
          {
            KRYS_GIF_GETN(currentComponent.front(), GIFSkipBlock);
          }
          break;
        }

        case GIFSkipBlock:
        {
          KRYS_GIF_GETN(1, GIFConsumeBlock);
          break;
        }

        case GIFControlExtension:
        {
          AddFrameIfNecessary();
          GIFFrameContext *currentFrame = _frames.back().get();
          currentFrame->IsTransparent = currentComponent.front() & 0x1;
          if (currentFrame->IsTransparent)
          {
            currentFrame->Tpixel = currentComponent[3];
          }

          // We ignore the "user input" bit.

          // NOTE: This relies on the values in the DisposalMethod enum
          // matching those in the GIF spec!
          int disposalMethod = (currentComponent.front() >> 2) & 0x7;
          if (disposalMethod < 4)
          {
            currentFrame->DisposalMethod =
              static_cast<ScalableImageDecoderFrame::DisposalMethod>(disposalMethod);
          }
          else if (disposalMethod == 4)
          {
            // Some specs say that disposal method 3 is "overwrite previous", others that setting
            // the third bit of the field (i.e. method 4) is. We map both to the same value.
            currentFrame->DisposalMethod = ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious;
          }
          currentFrame->DelayTime = GetInt16(currentComponent.data() + 1) * 10;
          KRYS_GIF_GETN(1, GIFConsumeBlock);
          break;
        }

        case GIFCommentExtension:
        {
          if (currentComponent.front())
          {
            KRYS_GIF_GETN(currentComponent.front(), GIFConsumeComment);
          }
          else
          {
            KRYS_GIF_GETN(1, GIFImageStart);
          }
          break;
        }

        case GIFConsumeComment:
        {
          KRYS_GIF_GETN(1, GIFCommentExtension);
          break;
        }

        case GIFApplicationExtension:
        {
          // Check for netscape application extension.
          if (_bytesToConsume == 11
              && (HasPrefix(currentComponent, "NETSCAPE2.0") || HasPrefix(currentComponent, "ANIMEXTS1.0")))
          {
            KRYS_GIF_GETN(1, GIFNetscapeExtensionBlock);
          }
          else
          {
            KRYS_GIF_GETN(1, GIFConsumeBlock);
          }
          break;
        }

        // Netscape-specific GIF extension: animation looping.
        case GIFNetscapeExtensionBlock:
        {
          // GIFConsumeNetscapeExtension always reads 3 bytes from the stream; we should at least wait for
          // this amount.
          if (currentComponent.front())
          {
            KRYS_GIF_GETN(std::max(static_cast<size_t>(3), static_cast<size_t>(currentComponent.front())),
                          GIFConsumeNetscapeExtension);
          }
          else
          {
            KRYS_GIF_GETN(1, GIFImageStart);
          }
          break;
        }

        // Parse netscape-specific application extensions
        case GIFConsumeNetscapeExtension:
        {
          int netscapeExtension = currentComponent[0] & 7;

          // Loop entire animation specified # of times. Only read the loop count during the first iteration.
          if (netscapeExtension == 1)
          {
            _loopCount = static_cast<int>(GetInt16(currentComponent.data() + 1));

            // Zero loop count is infinite animation loop request.
            if (!_loopCount)
            {
              _loopCount = RepetitionCountInfinite;
            }

            KRYS_GIF_GETN(1, GIFNetscapeExtensionBlock);
          }
          else if (netscapeExtension == 2)
          {
            // Wait for specified # of bytes to enter buffer.

            // Don't do this, this extension doesn't exist (isn't used at all)
            // and doesn't do anything, as our streaming/buffering takes care of it all...
            // See: http://semmix.pl/color/exgraf/eeg24.htm
            KRYS_GIF_GETN(1, GIFNetscapeExtensionBlock);
          }
          else
          {
            // 0,3-7 are yet to be defined netscape extension codes
            return false;
          }
          break;
        }

        case GIFImageHeader:
        {
          unsigned height, width, xOffset, yOffset;

          /* Get image offsets, with respect to the screen origin */
          xOffset = GetInt16(currentComponent.data());
          yOffset = GetInt16(currentComponent.data() + 2);

          /* Get image width and height. */
          width = GetInt16(currentComponent.data() + 4);
          height = GetInt16(currentComponent.data() + 6);

          /* Work around broken GIF files where the logical screen
           * size has weird width or height.  We assume that GIF87a
           * files don't contain animations.
           */
          if (CurrentFrameIsFirstFrame()
              && ((_screenHeight < height) || (_screenWidth < width) || (_version == 87)))
          {
            _screenHeight = height;
            _screenWidth = width;
            xOffset = 0;
            yOffset = 0;

            // CALLBACK: Inform the decoderplugin of our size.
            if (_client
                && !_client->SetSize(
                  IntSize(static_cast<int32>(_screenWidth), static_cast<int32>(_screenHeight))))
            {
              return false;
            }
          }

          // Work around more broken GIF files that have zero image width or height
          if (!height || !width)
          {
            height = _screenHeight;
            width = _screenWidth;
            if (!height || !width)
            {
              return false;
            }
          }

          if (parseSizeOnly)
          {
            // The decoder needs to stop. Hand back the number of bytes we consumed from
            // buffer minus 9 (the amount we consumed to read the header).
            SetRemainingBytes(len + 9);
            KRYS_GIF_GETN(9, GIFImageHeader);
            return true;
          }

          AddFrameIfNecessary();
          GIFFrameContext *currentFrame = _frames.back().get();

          currentFrame->SetHeaderDefined();
          currentFrame->XOffset = xOffset;
          currentFrame->YOffset = yOffset;
          currentFrame->Height = height;
          currentFrame->Width = width;
          _screenWidth = std::max(_screenWidth, width);
          _screenHeight = std::max(_screenHeight, height);
          currentFrame->Interlaced = currentComponent[8] & 0x40;

          // Overlaying interlaced, transparent GIFs over
          // existing image data using the Haeberli display hack
          // requires saving the underlying image in order to
          // avoid jaggies at the transparency edges. We are
          // unprepared to deal with that, so don't display such
          // images progressively. Which means only the first
          // frame can be progressively displayed.
          // FIXME: It is possible that a non-transparent frame
          // can be interlaced and progressively displayed.
          currentFrame->ProgressiveDisplay = CurrentFrameIsFirstFrame();

          const bool isLocalColormapDefined = currentComponent[8] & 0x80;
          if (isLocalColormapDefined)
          {
            // The three low-order bits of currentComponent[8] specify the bits per pixel.
            int numColors = 2 << (currentComponent[8] & 0x7);
            const size_t localColormapBytes = 3 * static_cast<size_t>(numColors);

            // Switch to the new local palette after it loads
            currentFrame->LocalColormapPosition = dataPosition;
            currentFrame->LocalColormapSize = numColors;

            if (len < localColormapBytes)
            {
              // Wait until we have enough bytes to consume the entire colormap at once.
              KRYS_GIF_GETN(localColormapBytes, GIFImageColormap);
              break;
            }

            currentFrame->IsLocalColormapDefined = true;
            dataPosition += localColormapBytes;
            len -= localColormapBytes;
          }
          else
          {
            // Switch back to the global palette
            currentFrame->IsLocalColormapDefined = false;
          }
          KRYS_GIF_GETN(1, GIFLZWStart);
          break;
        }

        case GIFImageColormap:
        {
          krys_debug_assert(!_frames.empty());
          _frames.back()->IsLocalColormapDefined = true;
          KRYS_GIF_GETN(1, GIFLZWStart);
          break;
        }

        case GIFSubBlock:
        {
          const size_t bytesInBlock = currentComponent.front();
          if (bytesInBlock)
          {
            KRYS_GIF_GETN(bytesInBlock, GIFLZW);
          }
          else
          {
            // Finished parsing one frame; Process next frame.
            krys_debug_assert(!_frames.empty());
            // Note that some broken GIF files do not have enough LZW blocks to fully
            // decode all rows but we treat it as frame complete.
            _frames.back()->SetComplete();
            KRYS_GIF_GETN(1, GIFImageStart);
          }
          break;
        }

        case GIFDone:
        {
          _parseCompleted = true;
          return true;
        }

        default:
          // We shouldn't ever get here.
          return false;
      }
    }

    SetRemainingBytes(len);
    return true;
  }

#undef KRYS_GIF_GETN

  void GIFImageReader::SetRemainingBytes(size_t remainingBytes) noexcept
  {
    krys_debug_assert(remainingBytes <= _data->Size());
    _bytesRead = _data->Size() - remainingBytes;
  }

  void GIFImageReader::AddFrameIfNecessary() noexcept
  {
    if (_frames.empty() || _frames.back()->IsComplete())
    {
      _frames.push_back(CreateUnique<GIFFrameContext>(static_cast<int>(_frames.size())));
    }
  }

  // FIXME: Move this method to close to doLZW().
  bool GIFLZWContext::PrepareToDecode() noexcept
  {
    krys_debug_assert(_frameContext->IsDataSizeDefined() && _frameContext->IsHeaderDefined());

    // Since we use a codesize of 1 more than the datasize, we need to ensure
    // that our datasize is strictly less than the MAX_LZW_BITS value (12).
    // This sets the largest possible codemask correctly at 4095.
    if (_frameContext->Datasize >= MaxLZWBits)
    {
      return false;
    }
    _clearCode = 1 << _frameContext->Datasize;
    if (_clearCode >= MaxBytes)
    {
      return false;
    }

    _avail = _clearCode + 2;
    _oldcode = -1;
    _codesize = _frameContext->Datasize + 1;
    _codemask = (1 << _codesize) - 1;
    _datum = _bits = 0;
    _ipass = _frameContext->Interlaced ? 1 : 0;
    _irow = 0;

    // Initialize the tables lazily, this allows frame count query to use less memory.
    _suffix.resize(MaxBytes);
    _stack.resize(MaxBytes);
    _prefix.resize(MaxBytes);

    // Initialize output row buffer.
    _rowBuffer.resize(_frameContext->Width);
    _rowPosition = 0;
    _rowsRemaining = _frameContext->Height;

    // Clearing the whole suffix table lets us be more tolerant of bad data.
    std::ranges::fill(_suffix, 0);
    for (int i = 0; i < _clearCode; i++)
    {
      _suffix[static_cast<size_t>(i)] = static_cast<unsigned char>(i);
    }
    _stackp = 0;
    return true;
  }
}
