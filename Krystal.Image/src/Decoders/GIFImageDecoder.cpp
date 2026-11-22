#include "Krystal.Image/Decoders/GIFImageDecoder.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include <algorithm>
#include <cstring>
#include <tuple>

namespace krys::img
{
  namespace
  {
    // LZW codes are at most 12 bits, so a dictionary holds up to 4096 entries.
    constexpr int32 MaxCodeSize = 12;
    constexpr size_t DictionarySize = size_t {1} << MaxCodeSize;

    enum class BlockType : uint8
    {
      Extension = 0x21,
      Image = 0x2C,
      Trailer = 0x3B,
    };

    enum class ExtensionType : uint8
    {
      GraphicControl = 0xF9,
      Application = 0xFF,
    };

    KRYS_NODISCARD uint16 ReadUInt16(const uint8 *bytes) noexcept
    {
      return static_cast<uint16>(bytes[0] | (bytes[1] << 8));
    }

    KRYS_NODISCARD ImageDecoderFrame::DisposalMethod ToDisposalMethod(uint8 value) noexcept
    {
      switch (value)
      {
        case 1:  return ImageDecoderFrame::DisposalMethod::DoNotDispose;
        case 2:  return ImageDecoderFrame::DisposalMethod::RestoreToBackground;
        // Some old encoders wrote 4 for restoring the previous frame.
        case 3:
        case 4:  return ImageDecoderFrame::DisposalMethod::RestoreToPrevious;
        default: return ImageDecoderFrame::DisposalMethod::Unspecified;
      }
    }

    // The interlaced row order: four passes, each starting at a row and stepping down a number of rows.
    constexpr int32 InterlaceStart[] = {0, 4, 2, 1};
    constexpr int32 InterlaceStep[] = {8, 8, 4, 2};
  }

  struct GIFImageDecoder::FrameInfo
  {
    RectI Rect;
    bool Interlaced {false};
    Maybe<uint8> TransparentIndex;
    ImageDecoderFrame::DisposalMethod Disposal {ImageDecoderFrame::DisposalMethod::Unspecified};
    milliseconds Delay {};
    // The frame's colour table (its own, or else the global one): where it is, and how many colours it has.
    size_t ColorTableOffset {0};
    size_t ColorCount {0};
    uint8 MinimumCodeSize {0};
    // Where the frame's data sub-blocks start, once the parser has reached them.
    Maybe<size_t> DataOffset;
  };

  /// @brief Reads the structure of a GIF as its data arrives: a resumable walk over its blocks.
  class GIFImageDecoder::Parser
  {
  public:
    enum class Result : uint8
    {
      NeedsMoreData,
      Complete,
      Failed,
    };

  private:
    enum class Stage : uint8
    {
      Header,
      GlobalColorTable,
      Block,
      ExtensionSubBlock,
      ImageDescriptor,
      LocalColorTable,
      MinimumCodeSize,
      ImageSubBlock,
      Done,
    };

    // The graphic control extension's values, which apply to the next image.
    struct Control
    {
      Maybe<uint8> TransparentIndex;
      ImageDecoderFrame::DisposalMethod Disposal {ImageDecoderFrame::DisposalMethod::Unspecified};
      milliseconds Delay {};
    };

    Stage _stage {Stage::Header};
    size_t _position {0};
    bool _isGIF87a {false};
    SizeI _screenSize;
    size_t _globalColorTableOffset {0};
    size_t _globalColorCount {0};
    size_t _pendingTableColorCount {0};
    Control _control;
    ExtensionType _extension {ExtensionType::Application};
    size_t _extensionSubBlockIndex {0};
    bool _inLoopExtension {false};
    Maybe<uint16> _loopCount;
    List<FrameInfo> _frames;

  public:
    KRYS_NODISCARD const SizeI &ScreenSize() const noexcept
    {
      return _screenSize;
    }

    /// @brief The Netscape loop count: how many times the animation repeats after its first play, 0 for
    /// forever. Null when the image doesn't say.
    KRYS_NODISCARD Maybe<uint16> LoopCount() const noexcept
    {
      return _loopCount;
    }

    /// @brief The frames whose image descriptor has arrived.
    KRYS_NODISCARD const List<FrameInfo> &Frames() const noexcept
    {
      return _frames;
    }

    Result Parse(Span<const uint8> data) noexcept
    {
      while (true)
      {
        auto available = data.size() - _position;
        const auto *bytes = data.data() + _position;
        switch (_stage)
        {
          case Stage::Header:
          {
            // The signature, then the logical screen descriptor.
            constexpr size_t headerSize = 13;
            if (available < headerSize)
            {
              return Result::NeedsMoreData;
            }
            if (std::memcmp(bytes, "GIF87a", 6) && std::memcmp(bytes, "GIF89a", 6))
            {
              return Result::Failed;
            }
            _isGIF87a = bytes[4] == '7';
            _screenSize = {ReadUInt16(bytes + 6), ReadUInt16(bytes + 8)};
            auto flags = bytes[10];
            _position += headerSize;
            if (flags & 0x80)
            {
              _globalColorCount = size_t {2} << (flags & 0x07);
              _stage = Stage::GlobalColorTable;
            }
            else
            {
              _stage = Stage::Block;
            }
            break;
          }
          case Stage::GlobalColorTable:
            if (available < _globalColorCount * 3)
            {
              return Result::NeedsMoreData;
            }
            _globalColorTableOffset = _position;
            _position += _globalColorCount * 3;
            _stage = Stage::Block;
            break;
          case Stage::Block:
            if (!available)
            {
              return Result::NeedsMoreData;
            }
            switch (static_cast<BlockType>(bytes[0]))
            {
              case BlockType::Extension:
                if (available < 2)
                {
                  return Result::NeedsMoreData;
                }
                _extension = static_cast<ExtensionType>(bytes[1]);
                _extensionSubBlockIndex = 0;
                _inLoopExtension = false;
                _position += 2;
                _stage = Stage::ExtensionSubBlock;
                break;
              case BlockType::Image: _stage = Stage::ImageDescriptor; break;
              default:
                // The trailer ends the image. So does anything else here: browsers show such images as though
                // they ended properly.
                _stage = Stage::Done;
                break;
            }
            break;
          case Stage::ExtensionSubBlock:
          {
            if (!available)
            {
              return Result::NeedsMoreData;
            }
            size_t size = bytes[0];
            if (!size)
            {
              ++_position;
              _stage = Stage::Block;
              break;
            }
            if (available < 1 + size)
            {
              return Result::NeedsMoreData;
            }
            ReadExtensionSubBlock(Span<const uint8>(bytes + 1, size));
            ++_extensionSubBlockIndex;
            _position += 1 + size;
            break;
          }
          case Stage::ImageDescriptor:
          {
            constexpr size_t descriptorSize = 10;
            if (available < descriptorSize)
            {
              return Result::NeedsMoreData;
            }
            if (!ReadImageDescriptor(bytes))
            {
              return Result::Failed;
            }
            _position += descriptorSize;
            _stage = _pendingTableColorCount ? Stage::LocalColorTable : Stage::MinimumCodeSize;
            break;
          }
          case Stage::LocalColorTable:
            if (available < _pendingTableColorCount * 3)
            {
              return Result::NeedsMoreData;
            }
            _frames.back().ColorTableOffset = _position;
            _frames.back().ColorCount = _pendingTableColorCount;
            _position += _pendingTableColorCount * 3;
            _stage = Stage::MinimumCodeSize;
            break;
          case Stage::MinimumCodeSize:
            if (!available)
            {
              return Result::NeedsMoreData;
            }
            // Codes start a bit wider than this, and can't outgrow 12 bits.
            if (bytes[0] >= MaxCodeSize)
            {
              return Result::Failed;
            }
            _frames.back().MinimumCodeSize = bytes[0];
            _frames.back().DataOffset = ++_position;
            _stage = Stage::ImageSubBlock;
            break;
          case Stage::ImageSubBlock:
          {
            // The decoder reads the data itself; the parser only walks past it.
            if (!available)
            {
              return Result::NeedsMoreData;
            }
            size_t size = bytes[0];
            if (available < 1 + size)
            {
              return Result::NeedsMoreData;
            }
            _position += 1 + size;
            if (!size)
            {
              _stage = Stage::Block;
            }
            break;
          }
          case Stage::Done: return Result::Complete;
        }
      }
    }

  private:
    void ReadExtensionSubBlock(Span<const uint8> block) noexcept
    {
      if (_extension == ExtensionType::GraphicControl && !_extensionSubBlockIndex && block.size() >= 4)
      {
        auto flags = block[0];
        _control.Disposal = ToDisposalMethod(static_cast<uint8>((flags >> 2) & 0x07));
        // The delay is in hundredths of a second.
        _control.Delay = milliseconds(ReadUInt16(block.data() + 1) * 10);
        _control.TransparentIndex = flags & 0x01 ? Maybe<uint8>(block[3]) : null;
        return;
      }
      if (_extension == ExtensionType::Application)
      {
        // The Netscape extension (or its ANIMEXTS twin) holds the loop count: a sub-block of 1, then the
        // count.
        if (!_extensionSubBlockIndex)
        {
          _inLoopExtension = block.size() == 11
                             && (!std::memcmp(block.data(), "NETSCAPE2.0", 11)
                                 || !std::memcmp(block.data(), "ANIMEXTS1.0", 11));
        }
        else if (_inLoopExtension && block.size() >= 3 && block[0] == 1)
        {
          _loopCount = ReadUInt16(block.data() + 1);
        }
      }
    }

    bool ReadImageDescriptor(const uint8 *bytes) noexcept
    {
      auto x = static_cast<int32>(ReadUInt16(bytes + 1));
      auto y = static_cast<int32>(ReadUInt16(bytes + 3));
      auto width = static_cast<int32>(ReadUInt16(bytes + 5));
      auto height = static_cast<int32>(ReadUInt16(bytes + 7));
      auto flags = bytes[9];

      // As browsers do: a frame with no size covers the screen, and the first frame defines the screen when
      // it's bigger than the screen claims to be, or the image is a GIF87a.
      if (!width || !height)
      {
        width = _screenSize.Width();
        height = _screenSize.Height();
      }
      if (_frames.empty() && (_isGIF87a || width > _screenSize.Width() || height > _screenSize.Height()))
      {
        _screenSize = {width, height};
        x = 0;
        y = 0;
      }
      if (!width || !height)
      {
        return false;
      }

      FrameInfo frame;
      frame.Rect = {x, y, width, height};
      frame.Interlaced = flags & 0x40;
      frame.TransparentIndex = _control.TransparentIndex;
      frame.Disposal = _control.Disposal;
      frame.Delay = _control.Delay;
      frame.ColorTableOffset = _globalColorTableOffset;
      frame.ColorCount = _globalColorCount;
      _frames.push_back(frame);
      _control = {};
      _pendingTableColorCount = flags & 0x80 ? size_t {2} << (flags & 0x07) : 0;
      return true;
    }
  };

  /// @brief Decodes a frame's LZW-compressed colour indices into rows, as its data arrives.
  class GIFImageDecoder::LZWDecoder
  {
  public:
    enum class Result : uint8
    {
      NeedsMoreData,
      Complete,
      Failed,
    };

  private:
    size_t _frameIndex;
    FrameInfo _frame;
    bool _progressive;
    // The next sub-block to read.
    size_t _position;

    int32 _clearCode;
    int32 _codeSize;
    int32 _nextCode;
    int32 _previousCode {-1};
    uint8 _firstIndex {0};
    uint32 _bits {0};
    int32 _bitCount {0};
    bool _endOfData {false};
    // Each code's entry is its prefix code's entry followed by its suffix index.
    Array<uint16, DictionarySize> _prefix {};
    Array<uint8, DictionarySize> _suffix {};
    // An entry's indices, built back to front.
    Array<uint8, DictionarySize> _stack {};

    List<uint8> _row;
    int32 _pass {0};
    int32 _y {0};
    bool _rowsDone {false};

  public:
    LZWDecoder(size_t frameIndex, const FrameInfo &frame) noexcept
        : _frameIndex(frameIndex), _frame(frame), _progressive(frame.Interlaced && !frameIndex),
          _position(*frame.DataOffset), _clearCode(1 << frame.MinimumCodeSize),
          _codeSize(frame.MinimumCodeSize + 1), _nextCode(_clearCode + 2)
    {
      for (int32 code = 0; code < _clearCode; ++code)
      {
        _suffix[static_cast<size_t>(code)] = static_cast<uint8>(code);
      }
      _row.reserve(static_cast<size_t>(frame.Rect.Width()));
    }

    KRYS_NODISCARD size_t FrameIndex() const noexcept
    {
      return _frameIndex;
    }

    /// @brief Decodes the sub-blocks that have arrived, writing rows to @p decoder. Complete once the frame's
    /// terminating sub-block is reached.
    Result Decode(Span<const uint8> data, GIFImageDecoder &decoder) noexcept
    {
      while (_position < data.size())
      {
        size_t size = data[_position];
        if (!size)
        {
          ++_position;
          return Result::Complete;
        }
        if (data.size() - _position < 1 + size)
        {
          return Result::NeedsMoreData;
        }
        auto block = data.subspan(_position + 1, size);
        _position += 1 + size;
        // Past the end code or the last row, the rest of the data is skipped.
        if (!_endOfData && !_rowsDone && !DecodeBlock(block, decoder))
        {
          return Result::Failed;
        }
      }
      return Result::NeedsMoreData;
    }

  private:
    bool DecodeBlock(Span<const uint8> block, GIFImageDecoder &decoder) noexcept
    {
      for (auto byte : block)
      {
        _bits |= uint32 {byte} << _bitCount;
        _bitCount += 8;
        while (_bitCount >= _codeSize)
        {
          auto code = static_cast<int32>(_bits & ((1u << _codeSize) - 1));
          _bits >>= _codeSize;
          _bitCount -= _codeSize;
          if (!DecodeCode(code, decoder))
          {
            return false;
          }
          if (_endOfData || _rowsDone)
          {
            return true;
          }
        }
      }
      return true;
    }

    bool DecodeCode(int32 code, GIFImageDecoder &decoder) noexcept
    {
      if (code == _clearCode)
      {
        _codeSize = _frame.MinimumCodeSize + 1;
        _nextCode = _clearCode + 2;
        _previousCode = -1;
        return true;
      }
      if (code == _clearCode + 1)
      {
        _endOfData = true;
        return true;
      }

      size_t stackSize = 0;
      if (_previousCode < 0)
      {
        // The first code after a clear stands for one index.
        _firstIndex = _suffix[static_cast<size_t>(code)];
        _previousCode = code;
        return Output(Span<const uint8>(&_firstIndex, 1), decoder);
      }

      auto entry = code;
      if (code >= _nextCode)
      {
        // A code can only be the next one to be defined, standing for the previous entry and its own first
        // index.
        if (code > _nextCode)
        {
          return false;
        }
        _stack[stackSize++] = _firstIndex;
        entry = _previousCode;
      }
      while (entry >= _clearCode)
      {
        if (stackSize >= DictionarySize - 1)
        {
          return false;
        }
        _stack[stackSize++] = _suffix[static_cast<size_t>(entry)];
        entry = _prefix[static_cast<size_t>(entry)];
      }
      _firstIndex = static_cast<uint8>(entry);
      _stack[stackSize++] = _firstIndex;

      if (_nextCode < static_cast<int32>(DictionarySize))
      {
        _prefix[static_cast<size_t>(_nextCode)] = static_cast<uint16>(_previousCode);
        _suffix[static_cast<size_t>(_nextCode)] = _firstIndex;
        ++_nextCode;
        if (_nextCode == (1 << _codeSize) && _codeSize < MaxCodeSize)
        {
          ++_codeSize;
        }
      }
      _previousCode = code;

      std::reverse(_stack.begin(), _stack.begin() + static_cast<ptrdiff_t>(stackSize));
      return Output(Span<const uint8>(_stack.data(), stackSize), decoder);
    }

    // Adds indices to the row, writing each row as it fills.
    bool Output(Span<const uint8> indices, GIFImageDecoder &decoder) noexcept
    {
      auto width = static_cast<size_t>(_frame.Rect.Width());
      for (auto index : indices)
      {
        _row.push_back(index);
        if (_row.size() < width)
        {
          continue;
        }
        if (!WriteRow(decoder))
        {
          return false;
        }
        if (_rowsDone)
        {
          return true;
        }
      }
      return true;
    }

    bool WriteRow(GIFImageDecoder &decoder) noexcept
    {
      auto height = _frame.Rect.Height();
      // An interlaced first frame shows each early pass's rows in place of the rows below them, until those
      // arrive; later passes then overwrite them, transparent pixels included.
      auto repeat = 1;
      if (_progressive)
      {
        repeat = std::min(InterlaceStep[_pass] - (_pass ? InterlaceStart[_pass] : 0), height - _y);
        repeat = std::max(repeat, 1);
      }
      if (!decoder.WriteRow(_frameIndex, _row, _y, repeat, _progressive && _pass))
      {
        return false;
      }
      _row.clear();

      if (!_frame.Interlaced)
      {
        _rowsDone = ++_y >= height;
        return true;
      }
      _y += InterlaceStep[_pass];
      while (_y >= height && _pass < 3)
      {
        _y = InterlaceStart[++_pass];
      }
      _rowsDone = _y >= height;
      return true;
    }
  };

  GIFImageDecoder::GIFImageDecoder(AlphaOption alphaOption,
                                   GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  GIFImageDecoder::~GIFImageDecoder() noexcept = default;

  bool GIFImageDecoder::SetSize(const SizeI &size) noexcept
  {
    // The parser reports the size again whenever it parses more.
    if (IsSizeAvailable() && Size() == size)
    {
      return true;
    }
    return ImageDecoder::SetSize(size);
  }

  void GIFImageDecoder::TryDecodeSize(bool) noexcept
  {
    std::ignore = Parse();
  }

  bool GIFImageDecoder::Parse() noexcept
  {
    if (Failed())
    {
      return false;
    }
    if (!_parser)
    {
      _parser = CreateUnique<Parser>();
    }
    if (_parser->Parse(_data->GetSpan()) == Parser::Result::Failed)
    {
      return SetFailed();
    }
    if (!_parser->ScreenSize().IsEmpty() && !SetSize(_parser->ScreenSize()))
    {
      return false;
    }
    // A frame is counted once its descriptor has arrived.
    if (_frameBufferCache.size() < _parser->Frames().size())
    {
      _frameBufferCache.resize(_parser->Frames().size());
    }
    return true;
  }

  size_t GIFImageDecoder::FrameCount() const noexcept
  {
    std::ignore = const_cast<GIFImageDecoder *>(this)->Parse();
    return _frameBufferCache.size();
  }

  RepetitionCount GIFImageDecoder::GetRepetitionCount() const noexcept
  {
    if (Failed() || !_parser || _parser->Frames().empty())
    {
      return RepetitionCountOnce;
    }
    auto loopCount = _parser->LoopCount();
    if (!loopCount)
    {
      return RepetitionCountOnce;
    }
    // The loop count is how many times the animation repeats after it first plays.
    return *loopCount ? static_cast<RepetitionCount>(*loopCount) + 1 : RepetitionCountInfinite;
  }

  ImageDecoderFrame *GIFImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (index >= FrameCount())
    {
      return nullptr;
    }
    if (!_frameBufferCache[index].IsComplete())
    {
      for (auto frame = FirstFrameToDecode(index); frame <= index; ++frame)
      {
        if (!DecodeFrame(frame))
        {
          break;
        }
      }
    }
    return index < _frameBufferCache.size() ? &_frameBufferCache[index] : nullptr;
  }

  bool GIFImageDecoder::DecodeFrame(size_t index) noexcept
  {
    if (_frameBufferCache[index].IsComplete())
    {
      return true;
    }
    const auto &info = _parser->Frames()[index];
    if (!info.DataOffset)
    {
      return false;
    }
    if (!_lzw || _lzw->FrameIndex() != index)
    {
      // A partly decoded frame is only ever the one the decoder is working on; anything else starts afresh.
      _frameBufferCache[index].Clear();
      _lzw = CreateUnique<LZWDecoder>(index, info);
    }

    switch (_lzw->Decode(_data->GetSpan(), *this))
    {
      case LZWDecoder::Result::Complete: _lzw = nullptr; return CompleteFrame(index);
      case LZWDecoder::Result::NeedsMoreData:
        if (IsAllDataReceived())
        {
          // The data ended part way through the frame.
          _lzw = nullptr;
          std::ignore = SetFailed();
        }
        return false;
      case LZWDecoder::Result::Failed:
        _lzw = nullptr;
        std::ignore = SetFailed();
        return false;
    }
    return false;
  }

  ImageDecoder::FrameLayout GIFImageDecoder::LayoutOfFrame(size_t index) const noexcept
  {
    const auto &info = _parser->Frames()[index];
    return {info.Rect, info.Disposal};
  }

  bool GIFImageDecoder::InitializeFrame(size_t index) noexcept
  {
    if (!InitializeAnimationFrame(index))
    {
      return false;
    }
    _frameSawTransparency = false;
    return true;
  }

  bool GIFImageDecoder::WriteRow(size_t index, Span<const uint8> colorIndices, int32 y, int32 repeat,
                                 bool writeTransparentPixels) noexcept
  {
    const auto &info = _parser->Frames()[index];
    auto &frame = _frameBufferCache[index];
    if (frame.IsInvalid() && !InitializeFrame(index))
    {
      return false;
    }

    // The row may run past the image, which the frame's rect isn't required to stay within.
    auto left = info.Rect.X();
    auto top = info.Rect.Y() + y;
    auto right = std::min(info.Rect.MaxX(), Size().Width());
    auto bottom = std::min(top + repeat, Size().Height());
    if (right <= left || bottom <= top)
    {
      return true;
    }

    auto table = _data->GetSpan().subspan(info.ColorTableOffset, info.ColorCount * 3);
    auto &pixels = *frame.BackingStore();
    auto *destination = pixels.PixelsStartingAt(left, top);
    for (auto x = left; x < right; ++x, ++destination)
    {
      auto colorIndex = colorIndices[static_cast<size_t>(x - left)];
      auto offset = size_t {colorIndex} * 3;
      if (info.TransparentIndex != colorIndex && offset + 2 < table.size())
      {
        pixels.SetPixel(*destination, table[offset], table[offset + 1], table[offset + 2], 0xFF);
        continue;
      }
      // Transparent pixels show what's beneath them, unless they're replacing an earlier pass's stand-ins.
      _frameSawTransparency = true;
      if (writeTransparentPixels)
      {
        pixels.SetPixel(*destination, 0, 0, 0, 0);
      }
    }
    pixels.RepeatFirstRow({left, top, right - left, bottom - top});
    return true;
  }

  bool GIFImageDecoder::CompleteFrame(size_t index) noexcept
  {
    auto &frame = _frameBufferCache[index];
    const auto &info = _parser->Frames()[index];
    // A frame with no rows still shows the frames before it.
    if (frame.IsInvalid() && !InitializeFrame(index))
    {
      return false;
    }
    frame.SetDecodingStatus(DecodingStatus::Complete);
    frame.SetDuration(info.Delay);
    frame.SetDisposalMethod(info.Disposal);

    // An opaque frame covering the whole image has no alpha, nor does one that covers the area the frame
    // before it cleared; otherwise it keeps the alpha of what it started from.
    if (_frameSawTransparency)
    {
      return true;
    }
    auto rect = frame.BackingStore()->FrameRect();
    if (rect.Contains(RectI {PointI {}, Size()}))
    {
      frame.SetHasAlpha(false);
      return true;
    }
    auto base = index;
    while (base > 0
           && _parser->Frames()[base - 1].Disposal == ImageDecoderFrame::DisposalMethod::RestoreToPrevious)
    {
      --base;
    }
    if (base)
    {
      const auto &previous = _frameBufferCache[base - 1];
      if (_parser->Frames()[base - 1].Disposal == ImageDecoderFrame::DisposalMethod::RestoreToBackground
          && previous.HasBackingStore() && !previous.HasAlpha()
          && rect.Contains(previous.BackingStore()->FrameRect()))
      {
        frame.SetHasAlpha(false);
      }
    }
    return true;
  }

  void GIFImageDecoder::ClearFrameBufferCache(size_t clearBeforeFrame) noexcept
  {
    ClearAnimationFrames(clearBeforeFrame);
  }
}
