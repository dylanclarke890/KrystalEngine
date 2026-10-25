#pragma once

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathValue.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssmathclamp
  class CSSMathClamp final : public CSSMathValue
  {
  private:
    const Ref<CSSNumericValue> _lower;
    const Ref<CSSNumericValue> _value;
    const Ref<CSSNumericValue> _upper;

  public:
    /// @return A `TypeError` if the values' types can't be combined.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSMathClamp>>
      Create(CSSNumberish &&lower, CSSNumberish &&value, CSSNumberish &&upper) noexcept;
    KRYS_NODISCARD const CSSNumericValue &Lower() const noexcept
    {
      return *_lower;
    }
    KRYS_NODISCARD const CSSNumericValue &Value() const noexcept
    {
      return *_value;
    }
    KRYS_NODISCARD const CSSNumericValue &Upper() const noexcept
    {
      return *_upper;
    }

    KRYS_NODISCARD Maybe<calc::Child> ToCalcTreeNode() const noexcept final;

  private:
    KRYS_NODISCARD CSSMathOperator GetOperator() const noexcept final
    {
      return CSSMathOperator::Clamp;
    }
    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSMathClamp;
    }
    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;
    KRYS_NODISCARD Maybe<SumValue> ToSumValue() const noexcept final;
    KRYS_NODISCARD bool Equals(const CSSNumericValue &other) const noexcept final;

    CSSMathClamp(CSSNumericType &&type, Ref<CSSNumericValue> &&lower, Ref<CSSNumericValue> &&value,
                 Ref<CSSNumericValue> &&upper) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMathClamp)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSMathClamp;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
