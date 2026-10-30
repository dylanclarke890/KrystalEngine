#pragma once

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathValue.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssnegate
  class CSSMathNegate final : public CSSMathValue
  {
  private:
    const Ref<CSSNumericValue> _value;

  public:
    KRYS_NODISCARD static Ref<CSSMathNegate> Create(CSSNumberish &&numberish) noexcept;
    KRYS_NODISCARD CSSNumericValue &Value() noexcept
    {
      return *_value;
    }
    KRYS_NODISCARD const CSSNumericValue &Value() const noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD Maybe<calc::Child> ToCalcTreeNode() const noexcept final;

  private:
    KRYS_NODISCARD CSSMathOperator GetOperator() const noexcept final
    {
      return CSSMathOperator::Negate;
    }
    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSMathNegate;
    }
    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;
    KRYS_NODISCARD Maybe<SumValue> ToSumValue() const noexcept final;
    KRYS_NODISCARD bool Equals(const CSSNumericValue &other) const noexcept final;

    explicit CSSMathNegate(CSSNumberish &&numberish) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMathNegate)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSMathNegate;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
