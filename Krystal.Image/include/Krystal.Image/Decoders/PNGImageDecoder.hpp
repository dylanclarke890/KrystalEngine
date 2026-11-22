#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Image/ICCProfileTransform.hpp"
#include "Krystal.Image/ImageDecoder.hpp"

namespace krys::img
{
  /// @brief Decodes PNG and animated PNG (APNG) images as their data arrives, with libpng. Rows show as they
  /// decode; an interlaced frame fills in pass by pass. Pixels are converted to sRGB from an embedded ICC
  /// profile, or else corrected for the image's gamma.
  /// @see https://www.w3.org/TR/png-3/
  class PNGImageDecoder final : public ImageDecoder
  {
    class FrameReader;
    class Parser;
    struct FrameInfo;

  private:
    // The structure of the image: its header, the chunks that describe it, and each frame's placement and
    // where its data is. Kept while the decoder lives, so a frame whose pixels were cleared can be decoded
    // again.
    UniquePtr<Parser> _parser;
    // The libpng reader of the frame being decoded, kept while its data arrives.
    UniquePtr<FrameReader> _reader;
    // The conversion from the colour space of the image's ICC profile, and a row converted with it.
    ICCProfileTransform _colorTransform;
    List<uint8> _transformedRow;
    // Whether the frame being decoded has written a pixel that isn't opaque.
    bool _frameSawAlpha {false};

  public:
    KRYS_NODISCARD static Ref<ImageDecoder>
      Create(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return AdoptRef(*new PNGImageDecoder(alphaOption, gammaAndColorProfileOption));
    }

    ~PNGImageDecoder() noexcept override;

    KRYS_NODISCARD utf8_string FilenameExtension() const noexcept override
    {
      return u8"png";
    }

    bool SetSize(const SizeI &size) noexcept override;

    KRYS_NODISCARD size_t FrameCount() const noexcept override;

    KRYS_NODISCARD RepetitionCount GetRepetitionCount() const noexcept override;

    ImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept override;

    void ClearFrameBufferCache(size_t clearBeforeFrame) noexcept override;

  private:
    PNGImageDecoder(AlphaOption, GammaAndColorProfileOption) noexcept;

    void TryDecodeSize(bool allDataReceived) noexcept override;

    KRYS_NODISCARD FrameLayout LayoutOfFrame(size_t index) const noexcept override;

    /// @brief Parses the structure of what's arrived, sizing the image and the frame cache. False if the data
    /// is malformed, which fails the decoder.
    bool Parse(bool allDataReceived) noexcept;

    /// @brief Decodes what's arrived of frame @p index, whose predecessors are decoded. True once it's
    /// complete.
    bool DecodeFrame(size_t index) noexcept;

    /// @brief Sets frame @p index up to be drawn into, and resets the tracking of its alpha.
    bool InitializeFrame(size_t index) noexcept;

    /// @brief Writes row @p y of frame @p index from @p pixels, decoded RGB or RGBA, blending them over
    /// what's beneath if the frame asks.
    bool WriteRow(size_t index, const uint8 *pixels, int32 y, bool hasAlphaChannel) noexcept;

    bool CompleteFrame(size_t index) noexcept;
  };
}
