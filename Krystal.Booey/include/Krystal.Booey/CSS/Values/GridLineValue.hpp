#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<grid-line>` value: an optional `span`, an optional integer and an optional line name, e.g.
  /// `span 2 header`.
  class GridLineValue final : public Value
  {
  private:
    RefPtr<PrimitiveValue> _spanValue;
    RefPtr<PrimitiveValue> _numericValue;
    RefPtr<PrimitiveValue> _gridLineName;

    GridLineValue(RefPtr<PrimitiveValue> spanValue, RefPtr<PrimitiveValue> numericValue,
                  RefPtr<PrimitiveValue> gridLineName) noexcept;

  public:
    KRYS_NODISCARD static Ref<GridLineValue> Create(RefPtr<PrimitiveValue> spanValue,
                                                    RefPtr<PrimitiveValue> numericValue,
                                                    RefPtr<PrimitiveValue> gridLineName) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const GridLineValue &other) const noexcept;

    KRYS_NODISCARD PrimitiveValue *SpanValue() const noexcept
    {
      return _spanValue.get();
    }

    KRYS_NODISCARD RefPtr<PrimitiveValue> ProtectedSpanValue() const noexcept
    {
      return _spanValue;
    }

    KRYS_NODISCARD PrimitiveValue *NumericValue() const noexcept
    {
      return _numericValue.get();
    }

    KRYS_NODISCARD RefPtr<PrimitiveValue> ProtectedNumericValue() const noexcept
    {
      return _numericValue;
    }

    KRYS_NODISCARD PrimitiveValue *GridLineName() const noexcept
    {
      return _gridLineName.get();
    }

    KRYS_NODISCARD RefPtr<PrimitiveValue> ProtectedGridLineName() const noexcept
    {
      return _gridLineName;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridLineValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridLineValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
