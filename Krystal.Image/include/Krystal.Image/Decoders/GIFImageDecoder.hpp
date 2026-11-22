#pragma once

#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Image/ImageDecoder.hpp"

namespace krys::img
{
  /// @brief Decodes GIF images, still and animated, as their data arrives. A frame shows what's decoded of it
  /// so far; an interlaced first frame fills in progressively, each pass's rows standing in for the rows
  /// below them until those arrive.
  /// @see https://www.w3.org/Graphics/GIF/spec-gif89a.txt
  class GIFImageDecoder final : public ImageDecoder
  {
    class LZWDecoder;
    class Parser;
    struct FrameInfo;

  private:
    // The structure of the image: its size, loop count, and each frame's placement and where its data is.
    // Kept while the decoder lives, so a frame whose pixels were cleared can be decoded again.
    UniquePtr<Parser> _parser;
    // The decoding state of the frame being decoded, kept while its data arrives.
    UniquePtr<LZWDecoder> _lzw;
    // Whether the frame being decoded has written (or skipped) a transparent pixel.
    bool _frameSawTransparency {false};

  public:
    KRYS_NODISCARD static Ref<ImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new GIFImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~GIFImageDecoder() noexcept override;

    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept override
    {
      return u8"gif";
    }

    bool SetSize(const SizeI &size) noexcept override;

    KRYS_NODISCARD size_t FrameCount() const noexcept override;

    KRYS_NODISCARD RepetitionCount GetRepetitionCount() const noexcept override;

    ImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept override;

    void ClearFrameBufferCache(size_t clearBeforeFrame) noexcept override;

  private:
    GIFImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;

    void TryDecodeSize(bool allDataReceived) noexcept override;

    KRYS_NODISCARD FrameLayout LayoutOfFrame(size_t index) const noexcept override;

    /// @brief Parses the structure of what's arrived, sizing the image and the frame cache. False if the data
    /// is malformed, which fails the decoder.
    bool Parse() noexcept;

    /// @brief Decodes what's arrived of frame @p index, whose predecessors are decoded. True once it's
    /// complete.
    bool DecodeFrame(size_t index) noexcept;

    /// @brief Sets frame @p index up to be drawn into, and resets the tracking of its alpha.
    bool InitializeFrame(size_t index) noexcept;

    bool WriteRow(size_t index, Span<const uint8> colorIndices, int32 y, int32 repeat,
                  bool writeTransparentPixels) noexcept;

    bool CompleteFrame(size_t index) noexcept;
  };
}
