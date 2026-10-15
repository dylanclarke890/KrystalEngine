#pragma once

#include "Krystal.Booey/CSS/Values/FilterEffects/Filter.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
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
  /// @brief The value of a `filter()` image function.
  class FilterImageValue final : public Value
  {
  private:
    const Ref<Value> _imageValueOrNone;
    boo::css::Filter _filter;

    FilterImageValue(Ref<Value> &&imageValueOrNone, boo::css::Filter &&filter) noexcept;

  public:
    KRYS_NODISCARD static Ref<FilterImageValue> Create(Ref<Value> &&imageValueOrNone,
                                                       boo::css::Filter &&filter) noexcept;

    KRYS_NODISCARD bool Equals(const FilterImageValue &other) const noexcept;
    KRYS_NODISCARD bool EqualInputImages(const FilterImageValue &other) const noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;

    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FilterImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFilterImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
