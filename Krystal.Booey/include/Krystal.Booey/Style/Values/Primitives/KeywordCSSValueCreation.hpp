#pragma once

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordValueRepresentationNeeded.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  template <css::EnumWithoutValueRepresentation T>
  struct CSSValueCreation<T>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, T value) noexcept
    {
      return css::PrimitiveValue::Create(ToValueId(value));
    }
  };

  template <css::EnumWithValueRepresentation T>
  struct CSSValueCreation<T>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              T value) noexcept
    {
      return valueRepresentation(value, [&](const auto &alternative) -> Ref<css::Value>
                                 { return CreateCSSValue(pool, style, alternative); });
    }
  };
}
