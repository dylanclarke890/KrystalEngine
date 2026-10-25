#pragma once

#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathOperator.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSNumericArray.hpp"

namespace krys::boo::css
{
  /// @brief A math expression of numeric values.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssmathvalue
  class CSSMathValue : public CSSNumericValue
  {
  public:
    explicit CSSMathValue(CSSNumericType type) noexcept : CSSNumericValue(krys::move(type))
    {
    }

    KRYS_NODISCARD virtual CSSMathOperator GetOperator() const noexcept = 0;

    template <typename T>
    KRYS_NODISCARD bool EqualsImpl(const CSSNumericValue &other) const noexcept;

    KRYS_NODISCARD RefPtr<css::Value> ToCSSValue() const noexcept final;
  };

  template <typename T>
  bool CSSMathValue::EqualsImpl(const CSSNumericValue &other) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#equal-numeric-value
    auto *otherT = DynamicDowncast<T>(other);
    if (!otherT)
    {
      return false;
    }

    krys_debug_assert(StyleValueType() == other.StyleValueType());
    const auto &thisValues = static_cast<const T *>(this)->Values();
    const auto &otherValues = otherT->Values();
    auto length = thisValues.Length();
    if (length != otherValues.Length())
    {
      return false;
    }

    for (size_t i = 0; i < length; ++i)
    {
      if (!thisValues.Array()[i]->Equals(*otherValues.Array()[i]))
      {
        return false;
      }
    }

    return true;
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMathValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return krys::boo::css::IsCSSMathValue(styleValue.StyleValueType());
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
