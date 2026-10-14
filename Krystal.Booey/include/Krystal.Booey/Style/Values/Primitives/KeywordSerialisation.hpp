#pragma once

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordValueRepresentationNeeded.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  template <css::EnumWithoutValueRepresentation T>
  struct Serialiser<T>
  {
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &, const RenderStyle &,
                    T value) noexcept
    {
      builder.append(css::ToLower(ToValueId(value)));
    }
  };

  template <css::EnumWithValueRepresentation T>
  struct Serialiser<T>
  {
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const T &value) noexcept
    {
      return valueRepresentation(value, [&](const auto &alternative)
                                 { Serialise(builder, context, style, alternative); });
    }
  };
}
