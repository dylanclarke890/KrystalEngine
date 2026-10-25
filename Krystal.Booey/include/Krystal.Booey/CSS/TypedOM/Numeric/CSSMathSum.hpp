#pragma once

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathValue.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssmathsum
  class CSSMathSum final : public CSSMathValue
  {
  private:
    const Ref<CSSNumericArray> _values;

  public:
    /// @return A `SyntaxError` if there are no values, or a `TypeError` if their types can't be combined.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSMathSum>> Create(List<CSSNumberish> &&numberishes) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSMathSum>>
      Create(List<Ref<CSSNumericValue>> &&values) noexcept;
    KRYS_NODISCARD const CSSNumericArray &Values() const noexcept
    {
      return *_values;
    }

    KRYS_NODISCARD Maybe<calc::Child> ToCalcTreeNode() const noexcept final;

  private:
    KRYS_NODISCARD CSSMathOperator GetOperator() const noexcept final
    {
      return CSSMathOperator::Sum;
    }
    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSMathSum;
    }
    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;
    KRYS_NODISCARD Maybe<SumValue> ToSumValue() const noexcept final;
    KRYS_NODISCARD bool Equals(const CSSNumericValue &other) const noexcept final
    {
      return EqualsImpl<CSSMathSum>(other);
    }

    CSSMathSum(List<Ref<CSSNumericValue>> &&values, CSSNumericType &&type) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMathSum)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSMathSum;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
