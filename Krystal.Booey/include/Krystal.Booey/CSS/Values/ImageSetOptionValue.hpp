#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief One option of an `image-set()`: an image, its resolution and an optional `type()`.
  class ImageSetOptionValue final : public Value
  {
  private:
    const Ref<Value> _image;
    Ref<PrimitiveValue> _resolution;
    Maybe<CSSOMString> _mimeType;

    ImageSetOptionValue(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution) noexcept;
    ImageSetOptionValue(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution, CSSOMString &&type) noexcept;

  public:
    KRYS_NODISCARD static Ref<ImageSetOptionValue> Create(Ref<Value> &&image) noexcept;
    KRYS_NODISCARD static Ref<ImageSetOptionValue> Create(Ref<Value> &&image,
                                                          Ref<PrimitiveValue> &&resolution) noexcept;
    KRYS_NODISCARD static Ref<ImageSetOptionValue>
      Create(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution, CSSOMString type) noexcept;

    KRYS_NODISCARD bool Equals(const ImageSetOptionValue &other) const noexcept;
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD Value &Image() const noexcept
    {
      return *_image;
    }

    KRYS_NODISCARD PrimitiveValue &Resolution() const noexcept
    {
      return *_resolution;
    }

    KRYS_NODISCARD Ref<PrimitiveValue> ProtectedResolution() const noexcept
    {
      return _resolution;
    }

    void SetResolution(Ref<PrimitiveValue> &&resolution) noexcept;

    /// @brief The `type()` MIME type, if one was given.
    KRYS_NODISCARD const Maybe<CSSOMString> &Type() const noexcept
    {
      return _mimeType;
    }

    void SetType(CSSOMString type) noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*_image) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (func(*_resolution) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }

    KRYS_NODISCARD bool CustomTraverseSubresources(
      KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ImageSetOptionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsImageSetOptionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
