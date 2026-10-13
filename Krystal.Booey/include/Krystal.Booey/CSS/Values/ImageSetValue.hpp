#pragma once

#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleImage;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief The value of an `image-set()` function: a comma-separated list of `ImageSetOptionValue`s.
  class ImageSetValue final : public ValueContainingList
  {
  private:
    explicit ImageSetValue(ValueListBuilder builder) noexcept;

  public:
    KRYS_NODISCARD static Ref<ImageSetValue> Create(ValueListBuilder builder) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const ImageSetValue &other) const noexcept
    {
      return ItemsEqual(other);
    }

    // FIXME: not defined until style::BuilderState and StyleImage are ported.
    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ImageSetValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsImageSetValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
