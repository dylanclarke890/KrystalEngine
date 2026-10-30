#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathNegate.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"

namespace krys::boo::css
{
  Ref<CSSMathNegate> CSSMathNegate::Create(CSSNumberish &&numberish) noexcept
  {
    return AdoptRef(*new CSSMathNegate(krys::move(numberish)));
  }

  KRYS_NODISCARD static CSSNumericType CopyType(const CSSNumberish &numberish) noexcept
  {
    return krys::SwitchOn(
      numberish, [](double) { return CSSNumericType(); },
      [](const RefPtr<CSSNumericValue> &value)
      {
        if (!value)
        {
          return CSSNumericType();
        }
        return value->Type();
      });
  }

  CSSMathNegate::CSSMathNegate(CSSNumberish &&numberish) noexcept
      : CSSMathValue(CopyType(numberish)), _value(RectifyNumberish(krys::move(numberish)))
  {
  }

  void CSSMathNegate::Serialise(StringBuilder &builder,
                                EnumSet<SerialisationArguments> arguments) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#calc-serialization
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(arguments.Contains(SerialisationArguments::Nested) ? u8"(" : u8"calc(");
    }
    builder.Append(u8"-");
    _value->Serialise(builder, arguments);
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8')');
    }
  }

  auto CSSMathNegate::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    auto values = _value->ToSumValue();
    if (!values)
    {
      return null;
    }
    for (auto &value : *values)
    {
      value.Value = value.Value * -1;
    }
    return values;
  }

  bool CSSMathNegate::Equals(const CSSNumericValue &other) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#equal-numeric-value
    auto *otherNegate = DynamicDowncast<CSSMathNegate>(other);
    if (!otherNegate)
    {
      return false;
    }
    return _value->Equals(otherNegate->Value());
  }

  Maybe<calc::Child> CSSMathNegate::ToCalcTreeNode() const noexcept
  {
    auto child = _value->ToCalcTreeNode();
    if (!child)
    {
      return null;
    }

    auto operation = calc::Negate {.A = krys::move(*child)};
    auto type = calc::ToType(operation);
    if (!type)
    {
      return null;
    }

    return calc::CreateChild(krys::move(operation), *type);
  }
}
