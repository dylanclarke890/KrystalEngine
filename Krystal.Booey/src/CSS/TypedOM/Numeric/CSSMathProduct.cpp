#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathProduct.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathInvert.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSMathProduct>> CSSMathProduct::Create(List<CSSNumberish> &&numberishes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(numberishes.size());
    for (auto &numberish : numberishes)
    {
      values.push_back(RectifyNumberish(krys::move(numberish)));
    }
    return Create(krys::move(values));
  }

  dom::ExceptionOr<Ref<CSSMathProduct>> CSSMathProduct::Create(List<Ref<CSSNumericValue>> &&values) noexcept
  {
    if (values.empty())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    auto type = CSSNumericType::MultiplyTypes(values);
    if (!type)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    return AdoptRef(*new CSSMathProduct(krys::move(values), krys::move(*type)));
  }

  CSSMathProduct::CSSMathProduct(List<Ref<CSSNumericValue>> &&values, CSSNumericType &&type) noexcept
      : CSSMathValue(krys::move(type)), _values(CSSNumericArray::Create(krys::move(values)))
  {
  }

  void CSSMathProduct::Serialise(StringBuilder &builder,
                                 EnumSet<SerialisationArguments> arguments) const noexcept
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
          if (auto *mathInvert = DynamicDowncast<CSSMathInvert>(numericValue))
          {
            builder.Append(u8" / ");
            mathInvert->Value().Serialise(builder, operandSerialisationArguments);
            return;
          }
          builder.Append(u8" * ");
        }
        numericValue.Serialise(builder, operandSerialisationArguments);
      });
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8')');
    }
  }

  auto CSSMathProduct::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    auto productOfUnits = [](const UnitMap &units1, const UnitMap &units2)
    {
      // https://drafts.css-houdini.org/css-typed-om/#product-of-two-unit-maps
      auto result = units1;
      for (const auto &[unit, exponent] : units2)
      {
        auto [iterator, isNewEntry] = result.try_emplace(unit, exponent);
        if (!isNewEntry)
        {
          iterator->second += exponent;
        }
        if (!iterator->second)
        {
          result.erase(iterator);
        }
      }
      return result;
    };

    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    SumValue values {Addend {1.0, {}}};
    for (const auto &item : _values->Array())
    {
      auto newValues = item->ToSumValue();
      if (!newValues)
      {
        return null;
      }
      SumValue temp;
      for (const auto &item1 : values)
      {
        for (const auto &item2 : *newValues)
        {
          temp.push_back(Addend {item1.Value * item2.Value, productOfUnits(item1.Units, item2.Units)});
        }
      }
      values = krys::move(temp);
    }
    return values;
  }

  Maybe<calc::Child> CSSMathProduct::ToCalcTreeNode() const noexcept
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

    auto operation = calc::Product {.Children = krys::move(children)};
    auto type = calc::ToType(operation);
    if (!type)
    {
      return null;
    }

    return calc::CreateChild(krys::move(operation), *type);
  }
}
