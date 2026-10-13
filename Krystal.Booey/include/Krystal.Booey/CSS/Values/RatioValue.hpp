#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Ratio.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<ratio>` value, e.g. `aspect-ratio: 16 / 9`.
  class RatioValue final : public Value
  {
  private:
    Ratio _ratio;

    explicit RatioValue(Ratio &&ratio) noexcept;

  public:
    KRYS_NODISCARD static Ref<RatioValue> Create(Ratio ratio) noexcept;

    KRYS_NODISCARD const Ratio &GetRatio() const noexcept
    {
      return _ratio;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const RatioValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::RatioValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsRatioValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
