#pragma once

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathValue.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssmathmin
  class CSSMathMin final : public CSSMathValue
  {
  private:
    const Ref<CSSNumericArray> _values;

  public:
    /// @return A `SyntaxError` if there are no values, or a `TypeError` if their types can't be combined.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSMathMin>> Create(List<CSSNumberish> &&numberishes) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSMathMin>>
      Create(List<Ref<CSSNumericValue>> &&values) noexcept;
    KRYS_NODISCARD const CSSNumericArray &Values() const noexcept
    {
      return *_values;
    }

    KRYS_NODISCARD Maybe<calc::Child> ToCalcTreeNode() const noexcept final;

  private:
    KRYS_NODISCARD CSSMathOperator GetOperator() const noexcept final
    {
      return CSSMathOperator::Min;
    }
    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSMathMin;
    }
    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;
    KRYS_NODISCARD Maybe<SumValue> ToSumValue() const noexcept final;
    KRYS_NODISCARD bool Equals(const CSSNumericValue &other) const noexcept final
    {
      return EqualsImpl<CSSMathMin>(other);
    }

    CSSMathMin(List<Ref<CSSNumericValue>> &&values, CSSNumericType &&type) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMathMin)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSMathMin;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
