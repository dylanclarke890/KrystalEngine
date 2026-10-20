#pragma once

#include "Krystal.Booey/Graphics/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief A frame of a bitmap image: its metadata, and its native image once decoded.
  // FIXME: The HDR source, once there's HDR.
  class ImageFrame
  {
    friend class BitmapImageSource;
    friend class ImageDecoder;

  public:
    enum class Caching : uint8
    {
      Metadata,
      MetadataAndImage
    };

  private:
    struct Source
    {
      RefPtr<NativeImage> Image;
      DecodingOptions Options {DecodingMode::Auto};

      KRYS_NODISCARD bool HasNativeImage() const noexcept
      {
        return static_cast<bool>(Image);
      }

      KRYS_NODISCARD bool HasFullSizeNativeImage() const noexcept
      {
        return HasNativeImage() && Options.HasFullSize();
      }

      KRYS_NODISCARD bool
        HasDecodedNativeImageCompatibleWithOptions(const DecodingOptions &options) const noexcept
      {
        return HasNativeImage() && Options.IsCompatibleWith(options);
      }

      void Clear() noexcept
      {
        if (std::exchange(Image, nullptr))
        {
          Options = DecodingOptions();
        }
      }
    };

    DecodingStatus _decodingStatus {DecodingStatus::Invalid};
    IntSize _size;
    Maybe<IntSize> _densityCorrectedSize;
    SubsamplingLevel _subsamplingLevel {SubsamplingLevel::Default};
    ImageOrientation _orientation {ImageOrientation::Orientation::None};
    seconds _duration {};
    bool _hasAlpha {true};
    Source _source;

  public:
    ImageFrame() noexcept = default;
    explicit ImageFrame(Ref<NativeImage> &&nativeImage) noexcept;
    ImageFrame(const ImageFrame &other) noexcept = default;
    ImageFrame &operator=(const ImageFrame &other) noexcept = default;
    ~ImageFrame() noexcept;

    KRYS_NODISCARD static const ImageFrame &DefaultFrame() noexcept;

    /// @return The bytes freed.
    uint32 ClearImage() noexcept;
    uint32 Clear() noexcept;

    void SetDecodingStatus(DecodingStatus decodingStatus) noexcept;
    KRYS_NODISCARD DecodingStatus GetDecodingStatus() const noexcept;

    KRYS_NODISCARD bool IsInvalid() const noexcept
    {
      return _decodingStatus == DecodingStatus::Invalid;
    }
    KRYS_NODISCARD bool IsPartial() const noexcept
    {
      return _decodingStatus == DecodingStatus::Partial;
    }
    KRYS_NODISCARD bool IsComplete() const noexcept
    {
      return _decodingStatus == DecodingStatus::Complete;
    }

    void SetSize(const IntSize &size) noexcept
    {
      _size = size;
    }
    KRYS_NODISCARD IntSize Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD uint32 SizeInBytes() const noexcept
    {
      return static_cast<uint32>(Size().UnclampedArea()) * sizeof(uint32);
    }

    void SetDensityCorrectedSize(const IntSize &size) noexcept
    {
      _densityCorrectedSize = size;
    }
    KRYS_NODISCARD Maybe<IntSize> DensityCorrectedSize() const noexcept
    {
      return _densityCorrectedSize;
    }
    KRYS_NODISCARD SubsamplingLevel GetSubsamplingLevel() const noexcept
    {
      return _subsamplingLevel;
    }

    KRYS_NODISCARD RefPtr<NativeImage> GetNativeImage() const noexcept
    {
      return _source.Image;
    }
    KRYS_NODISCARD DecodingOptions GetDecodingOptions() const noexcept
    {
      return _source.Options;
    }

    void SetOrientation(ImageOrientation orientation) noexcept
    {
      _orientation = orientation;
    }
    KRYS_NODISCARD ImageOrientation Orientation() const noexcept
    {
      return _orientation;
    }

    void SetDuration(seconds duration) noexcept
    {
      _duration = duration;
    }
    KRYS_NODISCARD seconds Duration() const noexcept
    {
      return _duration;
    }

    void SetHasAlpha(bool hasAlpha) noexcept
    {
      _hasAlpha = hasAlpha;
    }
    KRYS_NODISCARD bool HasAlpha() const noexcept
    {
      return !HasMetadata() || _hasAlpha;
    }

    KRYS_NODISCARD bool HasNativeImage() const noexcept
    {
      return _source.HasNativeImage();
    }
    KRYS_NODISCARD bool HasNativeImage(SubsamplingLevel subsamplingLevel) const noexcept;
    KRYS_NODISCARD bool HasFullSizeNativeImage(SubsamplingLevel subsamplingLevel) const noexcept;
    KRYS_NODISCARD bool
      HasDecodedNativeImageCompatibleWithOptions(const DecodingOptions &options,
                                                 SubsamplingLevel subsamplingLevel) const noexcept;
    KRYS_NODISCARD bool HasMetadata() const noexcept
    {
      return !Size().IsEmpty();
    }
  };
}
