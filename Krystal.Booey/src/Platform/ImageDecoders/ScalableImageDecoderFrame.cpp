#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoderFrame.hpp"

namespace krys::boo
{
  ScalableImageDecoderFrame::ScalableImageDecoderFrame() noexcept = default;

  ScalableImageDecoderFrame::~ScalableImageDecoderFrame() noexcept = default;

  ScalableImageDecoderFrame &
    ScalableImageDecoderFrame::operator=(const ScalableImageDecoderFrame &other) noexcept
  {
    if (this == &other)
    {
      return *this;
    }

    _decodingStatus = other._decodingStatus;
    if (other.BackingStore())
    {
      (void)Initialize(*other.BackingStore());
    }
    else
    {
      _backingStore = nullptr;
    }
    _disposalMethod = other._disposalMethod;
    _orientation = other._orientation;
    _duration = other._duration;
    _hasAlpha = other._hasAlpha;
    return *this;
  }

  void ScalableImageDecoderFrame::SetDecodingStatus(DecodingStatus decodingStatus) noexcept
  {
    krys_debug_assert(decodingStatus != DecodingStatus::Decoding);
    _decodingStatus = decodingStatus;
  }

  DecodingStatus ScalableImageDecoderFrame::GetDecodingStatus() const noexcept
  {
    krys_debug_assert(_decodingStatus != DecodingStatus::Decoding);
    return _decodingStatus;
  }

  void ScalableImageDecoderFrame::Clear() noexcept
  {
    *this = ScalableImageDecoderFrame();
  }

  bool ScalableImageDecoderFrame::Initialize(const ImageBackingStore &backingStore) noexcept
  {
    if (&backingStore == BackingStore())
    {
      return true;
    }

    _backingStore = ImageBackingStore::Create(backingStore);
    return _backingStore != nullptr;
  }

  bool ScalableImageDecoderFrame::Initialize(const IntSize &size, bool premultiplyAlpha) noexcept
  {
    if (size.IsEmpty())
    {
      return false;
    }

    _backingStore = ImageBackingStore::Create(size, premultiplyAlpha);
    return _backingStore != nullptr;
  }

  IntSize ScalableImageDecoderFrame::Size() const noexcept
  {
    if (HasBackingStore())
    {
      return BackingStore()->Size();
    }

    return {};
  }
}
