#pragma once

#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <string>

namespace krys::boo::css::tests
{
  /// @brief Fixture for tests that create values: initialises the static value pool, which pooled values
  /// (`PrimitiveValue::Create(ValueId)`, small integers, ...) live in.
  struct ValueFixture
  {
    ValueFixture() noexcept
    {
      StaticValuePool::Init();
    }
  };

  /// @brief Converts a CSSOM string to a narrow string, so Catch2 can print it.
  KRYS_NODISCARD inline std::string ToNarrow(CSSOMStringView str) noexcept
  {
    return std::string(reinterpret_cast<const char *>(str.data()), str.size());
  }

  /// @brief Serialises `value` with the default serialisation context.
  KRYS_NODISCARD inline std::string CssText(const Value &value) noexcept
  {
    return ToNarrow(value.CssText(DefaultSerialisationContext()));
  }

  /// @brief Collects the values `VisitChildren` visits, in order.
  KRYS_NODISCARD inline List<const Value *> Children(const Value &value) noexcept
  {
    List<const Value *> children;
    std::ignore = value.VisitChildren(
      [&](Value &child)
      {
        children.push_back(&child);
        return IterationStatus::Continue;
      });
    return children;
  }
}
