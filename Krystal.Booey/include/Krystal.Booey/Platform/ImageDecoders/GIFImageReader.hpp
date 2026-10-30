/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#pragma once

// Define ourselves as the clientPtr.  Mozilla just hacked their C++ callback class into this old C decoder,
// so we will too.
#include "Krystal.Booey/Platform/ImageDecoders/GIFImageDecoder.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  constexpr int MaxLZWBits = 12;
  constexpr int MaxBytes = 4'097; /* 2^MAX_LZW_BITS+1 */
  constexpr int MaxColors = 256;
  constexpr int GIFColors = 3;

  constexpr int LoopCountNotSeen = -2;

  // List of possible parsing states.
  enum GIFState : uint8
  {
    GIFType,
    GIFGlobalHeader,
    GIFGlobalColormap,
    GIFImageStart,
    GIFImageHeader,
    GIFImageColormap,
    GIFImageBody,
    GIFLZWStart,
    GIFLZW,
    GIFSubBlock,
    GIFExtension,
    GIFControlExtension,
    GIFConsumeBlock,
    GIFSkipBlock,
    GIFDone,
    GIFCommentExtension,
    GIFApplicationExtension,
    GIFNetscapeExtensionBlock,
    GIFConsumeNetscapeExtension,
    GIFConsumeComment
  };

  struct GIFFrameContext;

  // LZW decoder state machine.
  class GIFLZWContext
  {
  private:
    // LZW decoding states and output states.
    size_t _stackp {0}; // Current stack pointer.
    int _codesize {0};
    int _codemask {0};
    int _clearCode {0}; // Codeword used to trigger dictionary reset.
    int _avail {0};     // Index of next available slot in dictionary.
    int _oldcode {0};
    unsigned char _firstchar {0};
    int _bits {0};    // Number of unread bits in "datum".
    int _datum {0};   // 32-bit input buffer.
    int _ipass {0};   // Interlace pass; Ranges 1-4 if interlaced.
    size_t _irow {0}; // Current output row, starting at zero.
    size_t _rowPosition {0};
    size_t _rowsRemaining {0}; // Rows remaining to be output.

    List<unsigned short> _prefix;
    List<unsigned char> _suffix;
    List<unsigned char> _stack;
    List<unsigned char> _rowBuffer; // Single scanline temporary buffer.

    // Initialized during construction and read-only.
    GIFImageDecoder *_client;
    const GIFFrameContext *_frameContext;

  public:
    GIFLZWContext(GIFImageDecoder *client, const GIFFrameContext *frameContext) noexcept
        : _client(client), _frameContext(frameContext)
    {
    }

    bool PrepareToDecode() noexcept;
    bool OutputRow() noexcept;
    bool DoLZW(Span<const uint8> block) noexcept;
    KRYS_NODISCARD bool HasRemainingRows() const noexcept
    {
      return _rowsRemaining;
    }
  };

  // Data structure for one LZW block.
  struct GIFLZWBlock
  {
    size_t BlockPosition;
    size_t BlockSize;
  };

  // Frame output state machine.
  struct GIFFrameContext
  {
  public:
    // FIXME: Move these members to private section.
    int FrameId;
    unsigned XOffset {0};
    unsigned YOffset {0}; // With respect to "screen" origin.
    unsigned Width {0};
    unsigned Height {0};
    int Tpixel {0}; // Index of transparent pixel.
    ScalableImageDecoderFrame::DisposalMethod DisposalMethod {
      ScalableImageDecoderFrame::DisposalMethod::Unspecified}; // Restore to background, leave in place, etc.
    size_t LocalColormapPosition {0};                          // Per-image colormap.
    int LocalColormapSize {0}; // Size of local colormap array (in 3-byte units)
    int Datasize {0};

    bool IsLocalColormapDefined : 1 {false};
    bool ProgressiveDisplay : 1 {false}; // If true, do Haeberli interlace hack.
    bool Interlaced : 1 {false};         // True, if scanlines arrive interlaced order.
    bool IsTransparent : 1 {false};      // TRUE, if tpixel is valid.

    unsigned DelayTime {0}; // Display time, in milliseconds, for this image in a multi-image GIF.

  private:
    UniquePtr<GIFLZWContext> _lzwContext;
    List<GIFLZWBlock> _lzwBlocks; // LZW blocks for this frame.
    size_t _currentLzwBlock {0};
    bool _isComplete {false};
    bool _isHeaderDefined {false};
    bool _isDataSizeDefined {false};

  public:
    explicit GIFFrameContext(int id) noexcept : FrameId(id)
    {
    }

    void AddLzwBlock(size_t position, size_t size) noexcept
    {
      _lzwBlocks.push_back(GIFLZWBlock {position, size});
    }

    bool Decode(Span<const uint8> data, GIFImageDecoder *client, bool &frameDecoded) noexcept;

    KRYS_NODISCARD bool IsComplete() const noexcept
    {
      return _isComplete;
    }
    void SetComplete() noexcept
    {
      _isComplete = true;
    }
    KRYS_NODISCARD bool IsHeaderDefined() const noexcept
    {
      return _isHeaderDefined;
    }
    void SetHeaderDefined() noexcept
    {
      _isHeaderDefined = true;
    }
    KRYS_NODISCARD bool IsDataSizeDefined() const noexcept
    {
      return _isDataSizeDefined;
    }
    void SetDataSize(int size) noexcept
    {
      Datasize = size;
      _isDataSizeDefined = true;
    }
  };

  class GIFImageReader
  {
  private:
    GIFImageDecoder *_client;

    // Parsing state machine.
    GIFState _state {GIFType};  // Current decoder master state.
    size_t _bytesToConsume {6}; // Number of bytes to consume for next stage of parsing.
    size_t _bytesRead {0};      // Number of bytes processed.

    // Global (multi-image) state.
    int _screenBgcolor {0};    // Logical screen background color.
    int _version {0};          // Either 89 for GIF89 or 87 for GIF87.
    unsigned _screenWidth {0}; // Logical screen width & height.
    unsigned _screenHeight {0};
    bool _isGlobalColormapDefined {false};
    size_t _globalColormapPosition {
      0}; // (3* MAX_COLORS in size) Default colormap if local not supplied, 3 bytes for each color.
    int _globalColormapSize {0};       // Size of global colormap array (in 3-byte units)
    int _loopCount {LoopCountNotSeen}; // Netscape specific extension block to control the number of animation
                                       // loops a GIF renders.

    List<UniquePtr<GIFFrameContext>> _frames;
    size_t _currentDecodingFrame {0};

    RefPtr<const SharedBuffer> _data;
    bool _parseCompleted {false};

  public:
    explicit GIFImageReader(GIFImageDecoder *client = nullptr) noexcept : _client(client)
    {
    }

    void SetData(Ref<const SharedBuffer> &&data) noexcept
    {
      _data = krys::move(data);
    }
    // FIXME: haltAtFrame should be size_t.
    bool Decode(GIFImageDecoder::GIFQuery query, unsigned haltAtFrame) noexcept;

    KRYS_NODISCARD size_t ImagesCount() const noexcept
    {
      if (_frames.empty())
      {
        return 0;
      }

      // This avoids counting an empty frame when the file is truncated right after
      // GIFControlExtension but before GIFImageHeader.
      // FIXME: This extra complexity is not necessary and we should just report m_frames.size().
      return _frames.back()->IsHeaderDefined() ? _frames.size() : _frames.size() - 1;
    }
    KRYS_NODISCARD int LoopCount() const noexcept
    {
      return _loopCount;
    }

    KRYS_NODISCARD Span<const uint8> GlobalColormap() const noexcept
    {
      return _isGlobalColormapDefined
               ? Data(_globalColormapPosition, static_cast<size_t>(_globalColormapSize) * 3)
               : Span<const uint8> {};
    }

    KRYS_NODISCARD Span<const uint8> LocalColormap(const GIFFrameContext *frame) const noexcept
    {
      return frame->IsLocalColormapDefined
               ? Data(frame->LocalColormapPosition, static_cast<size_t>(frame->LocalColormapSize) * 3)
               : Span<const uint8> {};
    }

    KRYS_NODISCARD const GIFFrameContext *FrameContext() const noexcept
    {
      return _currentDecodingFrame < _frames.size() ? _frames[_currentDecodingFrame].get() : nullptr;
    }

    KRYS_NODISCARD const GIFFrameContext *FrameContext(size_t frame) const noexcept
    {
      return frame < _frames.size() ? _frames[frame].get() : nullptr;
    }

  private:
    bool Parse(size_t dataPosition, size_t len, bool parseSizeOnly) noexcept;
    void SetRemainingBytes(size_t remainingBytes) noexcept;

    KRYS_NODISCARD Span<const uint8> Data(size_t offset, size_t length) const noexcept
    {
      return _data->GetSpan().subspan(offset, length);
    }

    void AddFrameIfNecessary() noexcept;
    KRYS_NODISCARD bool CurrentFrameIsFirstFrame() const noexcept
    {
      return _frames.empty() || (_frames.size() == 1u && !_frames[0]->IsComplete());
    }
  };
}
