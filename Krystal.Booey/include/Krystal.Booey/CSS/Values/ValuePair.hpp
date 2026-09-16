#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class ValuePair final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<ValuePair> Create(Ref<Value> &&first, Ref<Value> &&second) noexcept;

    KRYS_NODISCARD static Ref<ValuePair> CreateSlashSeparated(Ref<Value> &&first,
                                                              Ref<Value> &&second) noexcept;

    KRYS_NODISCARD static Ref<ValuePair> CreateNonCoalescing(Ref<Value> &&first,
                                                             Ref<Value> &&second) noexcept;

    KRYS_NODISCARD Value &First() noexcept;

    KRYS_NODISCARD Value &Second() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ValuePair)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsValuePairValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()