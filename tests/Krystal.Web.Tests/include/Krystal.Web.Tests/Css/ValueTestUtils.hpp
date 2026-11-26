#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Web/Css/Values/GridLineNamesValue.hpp"
#include "Krystal.Web/Css/Values/ImageValue.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"
#include "Krystal.Web/Css/Values/ValuePool.hpp"
#include <string>

namespace krys::web::css::tests
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

  /// @brief An image value for `https://example.com/<name>.png`, for tests that need some image.
  KRYS_NODISCARD inline Ref<ImageValue> TestImage(utf8_stringview name) noexcept
  {
    return ImageValue::Create(net::Url {u8"https://example.com/" + utf8_string {name} + u8".png"});
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

  /// @brief @p value serialised with the default context.
  template <typename T>
  KRYS_NODISCARD CSSOMString SerialiseForTest(const T &value)
  {
    return Serialise(DefaultSerialisationContext(), value);
  }

  /// @brief A length of @p value pixels.
  KRYS_NODISCARD inline Ref<PrimitiveValue> Px(double value) noexcept
  {
    return PrimitiveValue::Create(value, UnitType::px);
  }

  /// @brief The keyword @p id as a value.
  KRYS_NODISCARD inline Ref<PrimitiveValue> IdentValue(ValueId id) noexcept
  {
    return PrimitiveValue::Create(id);
  }

  /// @brief A list of grid line names.
  KRYS_NODISCARD inline Ref<GridLineNamesValue> LineNames(List<CSSOMString> names)
  {
    return GridLineNamesValue::Create(names);
  }
}
