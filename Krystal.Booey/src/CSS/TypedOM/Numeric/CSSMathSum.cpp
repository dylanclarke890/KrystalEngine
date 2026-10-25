#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathSum.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathNegate.hpp"
#include <algorithm>

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSMathSum>> CSSMathSum::Create(List<CSSNumberish> &&numberishes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(numberishes.size());
    for (auto &numberish : numberishes)
    {
      values.push_back(RectifyNumberish(krys::move(numberish)));
    }
    return Create(krys::move(values));
  }

  dom::ExceptionOr<Ref<CSSMathSum>> CSSMathSum::Create(List<Ref<CSSNumericValue>> &&values) noexcept
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

    return AdoptRef(*new CSSMathSum(krys::move(values), krys::move(*type)));
  }

  CSSMathSum::CSSMathSum(List<Ref<CSSNumericValue>> &&values, CSSNumericType &&type) noexcept
      : CSSMathValue(krys::move(type)), _values(CSSNumericArray::Create(krys::move(values)))
  {
  }

  void CSSMathSum::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#calc-serialization
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(arguments.Contains(SerialisationArguments::Nested) ? u8"(" : u8"calc(");
    }
    _values->ForEach(
      [&](const CSSNumericValue &numericValue, bool first)
      {
        EnumSet<SerialisationArguments> operandSerialisationArguments {SerialisationArguments::Nested};
        if (arguments.Contains(SerialisationArguments::WithoutParentheses))
        {
          operandSerialisationArguments.Add(SerialisationArguments::WithoutParentheses);
        }
        if (!first)
        {
          if (auto *mathNegate = DynamicDowncast<CSSMathNegate>(numericValue))
          {
            builder.Append(u8" - ");
            mathNegate->Value().Serialise(builder, operandSerialisationArguments);
            return;
          }
          builder.Append(u8" + ");
        }
        numericValue.Serialise(builder, operandSerialisationArguments);
      });
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8')');
    }
  }

  auto CSSMathSum::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    auto convertToNumericType = [](const UnitMap &units) -> Maybe<CSSNumericType>
    {
      // https://drafts.css-houdini.org/css-typed-om/#create-a-type-from-a-unit-map
      CSSNumericType type;
      for (const auto &[unit, exponent] : units)
      {
        auto unitType = CSSNumericType::Create(unit, exponent);
        if (!unitType)
        {
          return null;
        }
        auto multipliedType = CSSNumericType::MultiplyTypes(type, *unitType);
        if (!multipliedType)
        {
          return null;
        }
        type = krys::move(*multipliedType);
      }
      return type;
    };

    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    SumValue values;
    for (const auto &item : _values->Array())
    {
      auto value = item->ToSumValue();
      if (!value)
      {
        return null;
      }
      for (auto &subvalue : *value)
      {
        auto it =
          std::ranges::find_if(values, [&](const Addend &addend) { return addend.Units == subvalue.Units; });
        if (it == values.end())
        {
          values.push_back(krys::move(subvalue));
        }
        else
        {
          it->Value += subvalue.Value;
        }
      }
    }

    auto type = convertToNumericType(values[0].Units);
    if (!type)
    {
      return null;
    }
    for (size_t i = 1; i < values.size(); ++i)
    {
      auto thisType = convertToNumericType(values[i].Units);
      if (!thisType)
      {
        return null;
      }
      type = CSSNumericType::AddTypes(*type, *thisType);
      if (!type)
      {
        return null;
      }
    }

    return values;
  }

  Maybe<calc::Child> CSSMathSum::ToCalcTreeNode() const noexcept
  {
    List<calc::Child> children;
    for (const auto &child : _values->Array())
    {
      if (auto node = child->ToCalcTreeNode())
      {
        children.push_back(krys::move(*node));
      }
    }
    if (children.size() != _values->Array().size())
    {
      return null;
    }

    auto operation = calc::Sum {.Children = krys::move(children)};
    auto type = calc::ToType(operation);
    if (!type)
    {
      return null;
    }

    return calc::CreateChild(krys::move(operation), *type);
  }
}
