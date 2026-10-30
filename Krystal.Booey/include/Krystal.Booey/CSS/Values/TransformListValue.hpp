#pragma once

#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A space-separated list of transform functions, e.g. `translateX(10px) rotate(45deg)`.
  class TransformListValue final : public ValueContainingList
  {
  private:
    explicit TransformListValue(ValueListBuilder builder) noexcept;
    explicit TransformListValue(Ref<Value> value) noexcept;

  public:
    KRYS_NODISCARD static Ref<TransformListValue> Create(ValueListBuilder builder) noexcept;
    KRYS_NODISCARD static Ref<TransformListValue> Create(Ref<Value> value) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept
    {
      return SerialiseItems(context);
    }

    KRYS_NODISCARD bool Equals(const TransformListValue &other) const noexcept
    {
      return ItemsEqual(other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::TransformListValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsTransformListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
