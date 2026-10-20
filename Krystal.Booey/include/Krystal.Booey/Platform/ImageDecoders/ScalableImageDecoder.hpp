#pragma once

#include "Krystal.Booey/Graphics/ImageDecoder.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoderFrame.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  // ScalableImageDecoder is a base for all format-specific decoders
  // (e.g. JPEGImageDecoder). This base manages the ScalableImageDecoderFrame cache.
  // FIXME: The lock that guards the frame cache, once decoding moves off the main thread.
  class ScalableImageDecoder : public ImageDecoder
  {
  protected:
    RefPtr<const SharedBuffer> _data;
    List<ScalableImageDecoderFrame> _frameBufferCache;
    bool _premultiplyAlpha;
    bool _ignoreGammaAndColorProfile;
    ImageOrientation _orientation;
    Maybe<IntSize> _densityCorrectedSize;

  private:
    IntSize _size;
    EncodedDataStatus _encodedDataStatus {EncodedDataStatus::TypeAvailable};
    bool _decodingSizeFromSetData {false};

  public:
    ScalableImageDecoder(AlphaOption alphaOption,
                         GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
        : _premultiplyAlpha(alphaOption == AlphaOption::Premultiplied),
          _ignoreGammaAndColorProfile(gammaAndColorProfileOption == GammaAndColorProfileOption::Ignored)
    {
    }

    ~ScalableImageDecoder() noexcept override = default;

    KRYS_NODISCARD static bool SupportsMediaType(MediaType type) noexcept
    {
      return type == MediaType::Image;
    }

    // Returns nullptr if we can't sniff a supported type from the provided data (possibly
    // because there isn't enough data yet).
    KRYS_NODISCARD static RefPtr<ScalableImageDecoder> Create(const SharedBuffer &data, AlphaOption,
                                                              GammaAndColorProfileOption) noexcept;

    KRYS_NODISCARD bool PremultiplyAlpha() const noexcept
    {
      return _premultiplyAlpha;
    }

    KRYS_NODISCARD bool IsAllDataReceived() const noexcept override
    {
      krys_debug_assert(!_decodingSizeFromSetData);
      return _encodedDataStatus == EncodedDataStatus::Complete;
    }

    void SetData(const SharedBuffer &data, bool allDataReceived) noexcept override
    {
      if (_encodedDataStatus == EncodedDataStatus::Error)
      {
        return;
      }

      _data = ShareRefPtr(&data);

      if (_encodedDataStatus == EncodedDataStatus::TypeAvailable)
      {
        _decodingSizeFromSetData = true;
        TryDecodeSize(allDataReceived);
        _decodingSizeFromSetData = false;
      }

      if (_encodedDataStatus == EncodedDataStatus::Error)
      {
        return;
      }

      if (allDataReceived)
      {
        krys_debug_assert(_encodedDataStatus == EncodedDataStatus::SizeAvailable);
        _encodedDataStatus = EncodedDataStatus::Complete;
      }
    }

    KRYS_NODISCARD EncodedDataStatus GetEncodedDataStatus() const noexcept override
    {
      return _encodedDataStatus;
    }

    KRYS_NODISCARD bool IsSizeAvailable() const noexcept override
    {
      return _encodedDataStatus >= EncodedDataStatus::SizeAvailable;
    }

    KRYS_NODISCARD IntSize Size() const noexcept override
    {
      return IsSizeAvailable() ? _size : IntSize();
    }

    // This will only differ from size() for ICO (where each frame is a
    // different icon) or other formats where different frames are different
    // sizes. This does NOT differ from size() for GIF, since decoding GIFs
    // composites any smaller frames against previous frames to create full-
    // size frames.
    KRYS_NODISCARD IntSize FrameSizeAtIndex(size_t, SubsamplingLevel) const noexcept override
    {
      return Size();
    }

    // Returns whether the size is legal (i.e. not going to result in
    // overflow elsewhere). If not, marks decoding as failed.
    virtual bool SetSize(const IntSize &size) noexcept
    {
      if (ImageBackingStore::IsOverSize(size))
      {
        return SetFailed();
      }
      _size = size;
      _encodedDataStatus = EncodedDataStatus::SizeAvailable;
      return true;
    }

    // Lazily-decodes enough of the image to get the frame count (if
    // possible), without decoding the individual frames.
    // FIXME: Right now that has to be done by each subclass; factor the
    // decode call out and use it here.
    KRYS_NODISCARD size_t FrameCount() const noexcept override
    {
      return 1;
    }

    KRYS_NODISCARD RepetitionCount GetRepetitionCount() const noexcept override
    {
      return RepetitionCountNone;
    }

    // Decodes as much of the requested frame as possible, and returns an
    // ScalableImageDecoder-owned pointer.
    virtual ScalableImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept = 0;

    KRYS_NODISCARD bool FrameIsCompleteAtIndex(size_t index) const noexcept override;

    // Make the best effort guess to check if the requested frame has alpha channel.
    KRYS_NODISCARD bool FrameHasAlphaAtIndex(size_t index) const noexcept override;

    KRYS_NODISCARD seconds FrameDurationAtIndex(size_t index) const noexcept final;

    KRYS_NODISCARD PlatformImagePtr CreateFrameImageAtIndex(
      size_t index, SubsamplingLevel = SubsamplingLevel::Default,
      const DecodingOptions & = DecodingOptions(DecodingMode::Synchronous)) noexcept override;

    void SetIgnoreGammaAndColorProfile(bool flag) noexcept
    {
      _ignoreGammaAndColorProfile = flag;
    }
    KRYS_NODISCARD bool IgnoresGammaAndColorProfile() const noexcept
    {
      return _ignoreGammaAndColorProfile;
    }

    KRYS_NODISCARD ImageOrientation FrameOrientationAtIndex(size_t) const noexcept final
    {
      return _orientation;
    }
    KRYS_NODISCARD Maybe<IntSize> FrameDensityCorrectedSizeAtIndex(size_t) const noexcept final
    {
      return _densityCorrectedSize;
    }

    static constexpr size_t ICCColorProfileHeaderLength = 128;

    KRYS_NODISCARD size_t BytesDecodedToDetermineProperties() const noexcept final
    {
      return 0;
    }

    KRYS_NODISCARD static SubsamplingLevel SubsamplingLevelForScale(float, SubsamplingLevel) noexcept
    {
      return SubsamplingLevel::Default;
    }

    // Sets the "decode failure" flag. For caller convenience (since so
    // many callers want to return false after calling this), returns false
    // to enable easy tailcalling. Subclasses may override this to also
    // clean up any local data.
    virtual bool SetFailed() noexcept
    {
      _encodedDataStatus = EncodedDataStatus::Error;
      return false;
    }

    KRYS_NODISCARD bool Failed() const noexcept
    {
      return _encodedDataStatus == EncodedDataStatus::Error;
    }

    // Clears decoded pixel data from before the provided frame unless that
    // data may be needed to decode future frames (e.g. due to GIF frame
    // compositing).
    void ClearFrameBufferCache(size_t) noexcept override
    {
    }

    // If the image has a cursor hot-spot, stores it in the argument
    // and returns true. Otherwise returns false.
    KRYS_NODISCARD Maybe<IntPoint> HotSpot() const noexcept override
    {
      return null;
    }

  private:
    virtual void TryDecodeSize(bool allDataReceived) noexcept = 0;
  };
}
