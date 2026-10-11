#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/BasicShape.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class BasicShapeValue : public Value
  {
  private:
    BasicShape _shape;

    BasicShapeValue(BasicShape shape) noexcept : Value(ValueType::BasicShape), _shape(krys::move(shape))
    {
    }

  public:
    KRYS_NODISCARD static Ref<BasicShapeValue> Create(BasicShape shape) noexcept
    {
      return AdoptRef(*new BasicShapeValue(krys::move(shape)));
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const BasicShapeValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;

    KRYS_NODISCARD const BasicShape &Shape() const noexcept
    {
      return _shape;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BasicShapeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBasicShapeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()