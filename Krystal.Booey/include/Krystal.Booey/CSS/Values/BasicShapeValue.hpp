#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/BasicShape.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<basic-shape>` value, e.g. `circle(50% at center)`.
  class BasicShapeValue final : public Value
  {
  private:
    BasicShape _shape;

    explicit BasicShapeValue(BasicShape &&shape) noexcept;

  public:
    KRYS_NODISCARD static Ref<BasicShapeValue> Create(BasicShape shape) noexcept;

    KRYS_NODISCARD const BasicShape &GetShape() const noexcept
    {
      return _shape;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const BasicShapeValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BasicShapeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBasicShapeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
