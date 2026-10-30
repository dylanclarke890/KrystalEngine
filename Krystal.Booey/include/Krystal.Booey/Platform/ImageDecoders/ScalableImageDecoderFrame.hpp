#pragma once

#include "Krystal.Booey/Graphics/ImageBackingStore.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief A frame a scalable image decoder decodes into: its backing store and its metadata.
  class ScalableImageDecoderFrame
  {
  public:
    enum class DisposalMethod : uint8
    {
      Unspecified,
      DoNotDispose,
      RestoreToBackground,
      RestoreToPrevious
    };

  private:
    DecodingStatus _decodingStatus {DecodingStatus::Invalid};

    UniquePtr<ImageBackingStore> _backingStore;
    DisposalMethod _disposalMethod {DisposalMethod::Unspecified};

    ImageOrientation _orientation {ImageOrientation::Orientation::None};
    seconds _duration {};
    bool _hasAlpha {true};

  public:
    ScalableImageDecoderFrame() noexcept;
    ScalableImageDecoderFrame(const ScalableImageDecoderFrame &other) noexcept
    {
      operator=(other);
    }

    ~ScalableImageDecoderFrame() noexcept;

    ScalableImageDecoderFrame &operator=(const ScalableImageDecoderFrame &other) noexcept;

    void Clear() noexcept;

    bool Initialize(const ImageBackingStore &backingStore) noexcept;
    bool Initialize(const IntSize &size, bool premultiplyAlpha) noexcept;

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

    KRYS_NODISCARD IntSize Size() const noexcept;

    void SetDisposalMethod(DisposalMethod method) noexcept
    {
      _disposalMethod = method;
    }
    KRYS_NODISCARD DisposalMethod GetDisposalMethod() const noexcept
    {
      return _disposalMethod;
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
    KRYS_NODISCARD bool HasMetadata() const noexcept
    {
      return !Size().IsEmpty();
    }

    KRYS_NODISCARD ImageBackingStore *BackingStore() const noexcept
    {
      return _backingStore.get();
    }
    KRYS_NODISCARD bool HasBackingStore() const noexcept
    {
      return BackingStore();
    }
  };
}
