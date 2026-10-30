#pragma once

#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <array>

struct png_struct_def;
struct png_info_def;
struct png_unknown_chunk_t;

namespace krys::boo
{
  class PNGImageReader;

  // This class decodes the PNG image format.
  // @note libpng reports errors with longjmp, which on Windows unwinds as an exception would, so the
  // functions its callbacks run through (the callbacks themselves and what they call) can't be noexcept.
  // FIXME: ICC colour profiles (LCMS), once there's colour management.
  class PNGImageDecoder final : public ScalableImageDecoder
  {
    friend class PNGImageReader;

  private:
    UniquePtr<PNGImageReader> _reader;
    bool _doNothingOnFailure {false};
    uint32 _currentFrame {0};
    png_struct_def *_png {nullptr};
    png_info_def *_info {nullptr};
    bool _isAnimated {false};
    bool _frameInfo {false};
    bool _frameIsHidden {false};
    bool _hasInfo {false};
    int32 _gamma {45'455};
    size_t _frameCount {1};
    size_t _decodedPixelCount {0};
    uint32 _playCount {0};
    uint32 _totalFrames {0};
    uint32 _sizePLTE {0};
    uint32 _sizetRNS {0};
    uint32 _sequenceNumber {0};
    uint32 _width {0};
    uint32 _height {0};
    uint32 _xOffset {0};
    uint32 _yOffset {0};
    uint32 _delayNumerator {1};
    uint32 _delayDenominator {1};
    uint32 _dispose {0};
    uint32 _blend {0};
    std::array<uint8, 12 + 13> _dataIHDR {};
    std::array<uint8, 12 + 256 * 3> _dataPLTE {};
    std::array<uint8, 12 + 256> _datatRNS {};

  public:
    KRYS_NODISCARD static Ref<ScalableImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new PNGImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~PNGImageDecoder() noexcept override;

    // ScalableImageDecoder
    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept override
    {
      return u8"png";
    }
    KRYS_NODISCARD size_t FrameCount() const noexcept override
    {
      return _frameCount;
    }
    KRYS_NODISCARD RepetitionCount GetRepetitionCount() const noexcept override;
    ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept override;
    // CAUTION: setFailed() deletes |m_reader|.  Be careful to avoid
    // accessing deleted memory, especially when calling this from inside
    // PNGImageReader!
    bool SetFailed() noexcept override;

    // Callbacks from libpng
    void HeaderAvailable();
    void RowAvailable(unsigned char *rowBuffer, unsigned rowIndex, int interlacePass);
    void PngComplete();
    void ReadChunks(png_unknown_chunk_t *chunk);
    void FrameHeader();

    void Init() noexcept;
    void ClearFrameBufferCache(size_t clearBeforeFrame) noexcept override;

    KRYS_NODISCARD bool IsComplete() const noexcept
    {
      if (_frameBufferCache.empty())
      {
        return false;
      }

      for (auto &imageFrame : _frameBufferCache)
      {
        if (!imageFrame.IsComplete())
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool IsCompleteAtIndex(size_t index) const noexcept
    {
      return index < _frameBufferCache.size() && _frameBufferCache[index].IsComplete();
    }

  private:
    PNGImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;
    void TryDecodeSize(bool allDataReceived) noexcept override
    {
      Decode(true, 0, allDataReceived);
    }

    // Decodes the image.  If |onlySize| is true, stops decoding after
    // calculating the image size.  If decoding fails but there is no more
    // data coming, sets the "decode failure" flag.
    void Decode(bool onlySize, unsigned haltAtFrame, bool allDataReceived) noexcept;
    void InitFrameBuffer(size_t frameIndex);
    void FrameComplete();
    int ProcessingStart(png_unknown_chunk_t *chunk);
    int ProcessingFinish();
    void FallbackNotAnimated() noexcept;

    void Clear() noexcept;
  };
}
