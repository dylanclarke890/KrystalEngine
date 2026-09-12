#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSValuePair final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSValuePair> Create(Ref<CSSValue> &&first, Ref<CSSValue> &&second) noexcept;

    KRYS_NODISCARD static Ref<CSSValuePair> CreateSlashSeparated(Ref<CSSValue> &&first,
                                                                 Ref<CSSValue> &&second) noexcept;

    KRYS_NODISCARD static Ref<CSSValuePair> CreateNonCoalescing(Ref<CSSValue> &&first,
                                                                Ref<CSSValue> &&second) noexcept;

    KRYS_NODISCARD CSSValue &First() noexcept;

    KRYS_NODISCARD CSSValue &Second() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSValuePair)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSValuePairValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()