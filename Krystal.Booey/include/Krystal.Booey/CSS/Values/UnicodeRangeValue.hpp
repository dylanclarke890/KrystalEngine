#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `unicode-range` descriptor value: an inclusive range of code points, e.g. `U+0-7f`.
  class UnicodeRangeValue final : public Value
  {
  private:
    char32 _from;
    char32 _to;

    UnicodeRangeValue(char32 from, char32 to) noexcept : Value(ValueType::UnicodeRange), _from(from), _to(to)
    {
    }

  public:
    KRYS_NODISCARD static Ref<UnicodeRangeValue> Create(char32 from, char32 to) noexcept
    {
      return AdoptRef(*new UnicodeRangeValue(from, to));
    }

    KRYS_NODISCARD char32 From() const noexcept
    {
      return _from;
    }

    KRYS_NODISCARD char32 To() const noexcept
    {
      return _to;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const UnicodeRangeValue &other) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::UnicodeRangeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsUnicodeRangeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
