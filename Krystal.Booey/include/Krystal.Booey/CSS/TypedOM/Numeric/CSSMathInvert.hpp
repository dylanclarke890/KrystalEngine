#pragma once

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathValue.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssinvert
  class CSSMathInvert final : public CSSMathValue
  {
  private:
    const Ref<CSSNumericValue> _value;

  public:
    KRYS_NODISCARD static Ref<CSSMathInvert> Create(CSSNumberish &&numberish) noexcept;
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
      return CSSMathOperator::Invert;
    }
    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSMathInvert;
    }
    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;
    KRYS_NODISCARD Maybe<SumValue> ToSumValue() const noexcept final;
    KRYS_NODISCARD bool Equals(const CSSNumericValue &other) const noexcept final;

    explicit CSSMathInvert(CSSNumberish &&numberish) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMathInvert)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSMathInvert;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
