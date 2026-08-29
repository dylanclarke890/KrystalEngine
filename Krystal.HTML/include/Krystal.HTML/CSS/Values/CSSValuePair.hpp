#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
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