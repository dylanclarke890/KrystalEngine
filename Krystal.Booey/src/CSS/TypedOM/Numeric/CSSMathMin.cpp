#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathMin.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSMathMin>> CSSMathMin::Create(List<CSSNumberish> &&numberishes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(numberishes.size());
    for (auto &numberish : numberishes)
    {
      values.push_back(RectifyNumberish(krys::move(numberish)));
    }
    return Create(krys::move(values));
  }

  dom::ExceptionOr<Ref<CSSMathMin>> CSSMathMin::Create(List<Ref<CSSNumericValue>> &&values) noexcept
  {
    if (values.empty())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    auto type = CSSNumericType::AddTypes(values);
    if (!type)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    return AdoptRef(*new CSSMathMin(krys::move(values), krys::move(*type)));
  }

  CSSMathMin::CSSMathMin(List<Ref<CSSNumericValue>> &&values, CSSNumericType &&type) noexcept
      : CSSMathValue(krys::move(type)), _values(CSSNumericArray::Create(krys::move(values)))
  {
  }

  void CSSMathMin::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#calc-serialization
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8"min(");
    }
    _values->ForEach(
      [&](const CSSNumericValue &numericValue, bool first)
      {
        if (!first)
        {
          builder.Append(u8", ");
        }
        numericValue.Serialise(builder,
                               {SerialisationArguments::Nested, SerialisationArguments::WithoutParentheses});
      });
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8')');
    }
  }

  auto CSSMathMin::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    const auto &valuesArray = _values->Array();
    Maybe<SumValue> currentMax = valuesArray[0]->ToSumValue();
    if (!currentMax || currentMax->size() != 1)
    {
      return null;
    }
    for (size_t i = 1; i < valuesArray.size(); ++i)
    {
      auto currentValue = valuesArray[i]->ToSumValue();
      if (!currentValue || currentValue->size() != 1 || (*currentValue)[0].Units != (*currentMax)[0].Units)
      {
        return null;
      }
      if ((*currentValue)[0].Value < (*currentMax)[0].Value)
      {
        currentMax = krys::move(currentValue);
      }
    }
    return currentMax;
  }

  Maybe<calc::Child> CSSMathMin::ToCalcTreeNode() const noexcept
  {
    List<calc::Child> children;
    for (const auto &child : _values->Array())
    {
      if (auto node = child->ToCalcTreeNode())
      {
        children.push_back(krys::move(*node));
      }
    }
    if (children.empty())
    {
      return null;
    }

    auto operation = calc::Min {.Children = krys::move(children)};
    auto type = calc::ToType(operation);
    if (!type)
    {
      return null;
    }

    return calc::CreateChild(krys::move(operation), *type);
  }
}
