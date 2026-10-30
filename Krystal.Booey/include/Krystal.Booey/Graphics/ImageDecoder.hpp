#pragma once

#include "Krystal.Booey/Graphics/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/PlatformImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class ImageFrame;
  class SharedBuffer;

  struct ImageDecoderFrameInfo
  {
    bool HasAlpha;
    seconds Duration;
  };

  /// @brief Decodes an image's encoded data, as it arrives, into its size, frames and frame metadata.
  class ImageDecoder : public RefCounted<ImageDecoder>, public CanMakeWeakPtr<ImageDecoder>
  {
    KRYS_NON_COPY_MOVABLE(ImageDecoder);

  public:
    using FrameInfo = ImageDecoderFrameInfo;

    enum class MediaType : uint8
    {
      Image,
      Video,
    };

    /// @brief A decoder for @p data, sniffed from its first bytes, or null if no decoder supports it (or not
    /// enough has arrived to tell).
    /// @param mimeType The type the data came with, which decoders don't rely on.
    // FIXME: Video decoders, once there's video.
    KRYS_NODISCARD static RefPtr<ImageDecoder>
      Create(const SharedBuffer &data, utf8_stringview mimeType, AlphaOption alphaOption,
             GammaAndColorProfileOption gammaAndColorProfileOption) noexcept;

    virtual ~ImageDecoder() noexcept;

    KRYS_NODISCARD static bool SupportsMediaType(MediaType mediaType) noexcept;

    KRYS_NODISCARD virtual size_t BytesDecodedToDetermineProperties() const noexcept = 0;

    KRYS_NODISCARD virtual EncodedDataStatus GetEncodedDataStatus() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsSizeAvailable() const noexcept
    {
      return GetEncodedDataStatus() >= EncodedDataStatus::SizeAvailable;
    }

    KRYS_NODISCARD virtual IntSize Size() const noexcept = 0;
    KRYS_NODISCARD virtual size_t FrameCount() const noexcept = 0;
    KRYS_NODISCARD virtual size_t PrimaryFrameIndex() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual RepetitionCount GetRepetitionCount() const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string FilenameExtension() const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string AccessibilityDescription() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual Maybe<IntPoint> HotSpot() const noexcept = 0;

    KRYS_NODISCARD virtual IntSize
      FrameSizeAtIndex(size_t index, SubsamplingLevel = SubsamplingLevel::Default) const noexcept = 0;
    KRYS_NODISCARD virtual bool FrameIsCompleteAtIndex(size_t index) const noexcept = 0;
    KRYS_NODISCARD virtual ImageOrientation FrameOrientationAtIndex(size_t) const noexcept
    {
      return ImageOrientation::Orientation::None;
    }
    KRYS_NODISCARD virtual Maybe<IntSize> FrameDensityCorrectedSizeAtIndex(size_t) const noexcept
    {
      return null;
    }

    KRYS_NODISCARD virtual seconds FrameDurationAtIndex(size_t index) const noexcept = 0;
    KRYS_NODISCARD virtual bool FrameHasAlphaAtIndex(size_t index) const noexcept = 0;

    virtual bool FetchFrameMetaDataAtIndex(size_t index, SubsamplingLevel subsamplingLevel,
                                           const DecodingOptions &options, ImageFrame &frame) const noexcept;

    KRYS_NODISCARD virtual PlatformImagePtr CreateFrameImageAtIndex(
      size_t index, SubsamplingLevel = SubsamplingLevel::Default,
      const DecodingOptions & = DecodingOptions(DecodingMode::Synchronous)) noexcept = 0;

    virtual void SetExpectedContentSize(int64) noexcept
    {
    }
    virtual void SetData(const SharedBuffer &data, bool allDataReceived) noexcept = 0;
    KRYS_NODISCARD virtual bool IsAllDataReceived() const noexcept = 0;
    virtual void ClearFrameBufferCache(size_t index) noexcept = 0;

  protected:
    ImageDecoder() noexcept;
  };
}
