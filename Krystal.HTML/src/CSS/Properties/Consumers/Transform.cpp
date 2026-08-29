#include "Krystal.HTML/CSS/Properties/Consumers/Transform.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Percentage.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSFunctionValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeRotate3dFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-rotate3d
    // rotate3d() = rotate3d( <number> , <number> , <number> , [ <angle> | <zero> ] )

    auto consumeParameters = [](auto &args, auto &state) -> Maybe<CSSValueListBuilder>
    {
      auto firstValue = CSSPrimitiveValueResolver<Number<>>::ConsumeAndResolve(args, state);
      if (!firstValue)
      {
        return {};
      }

      if (!ConsumeComma(args))
      {
        return {};
      }

      auto secondValue = CSSPrimitiveValueResolver<Number<>>::ConsumeAndResolve(args, state);
      if (!secondValue)
      {
        return {};
      }

      if (!ConsumeComma(args))
      {
        return {};
      }

      auto thirdValue = CSSPrimitiveValueResolver<Number<>>::ConsumeAndResolve(args, state);
      if (!thirdValue)
      {
        return {};
      }

      if (!ConsumeComma(args))
      {
        return {};
      }

      auto angle = CSSPrimitiveValueResolver<Angle<>>::ConsumeAndResolve(
        args, state, {.UnitlessZeroAngle = AllowUnitlessZero(true)});
      if (!angle)
      {
        return {};
      }

      CSSValueListBuilder parameters;
      parameters.push_back(Krys::Move(firstValue));
      parameters.push_back(Krys::Move(secondValue));
      parameters.push_back(Krys::Move(thirdValue));
      parameters.push_back(Krys::Move(angle));

      return {Krys::Move(parameters)};
    };

    auto functionId = tokens.Peek().FunctionId();
    if (functionId != CSSValueId::Rotate3d)
    {
      return {};
    }

    auto rangeCopy = tokens;
    auto args = ConsumeFunction(rangeCopy);
    if (args.IsAtEnd())
    {
      return {};
    }

    auto parameters = consumeParameters(args, state);
    if (!parameters || !args.IsAtEnd())
    {
      return {};
    }

    tokens = rangeCopy;
    return CSSFunctionValue::Create(functionId, Krys::Move(*parameters));
  }

  RefPtr<CSSValue> ConsumeTranslateFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-1/#funcdef-transform-translate
    // translate() = translate( <length-percentage> , <length-percentage>? )

    auto consumeParameters = [](auto &args, auto &state) -> std::optional<CSSValueListBuilder>
    {
      CSSValueListBuilder arguments;

      auto firstValue = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(args, state);
      if (!firstValue)
      {
        return {};
      }
      arguments.push_back(Krys::Move(firstValue));

      if (ConsumeComma(args))
      {
        auto secondValue = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(args, state);
        if (!secondValue)
        {
          return {};
        }

        // A second value of `0` is the same as no second argument, so there is no need to store one if we
        // know it is `0`.
        if (secondValue->IsZero() != true)
        {
          arguments.push_back(Krys::Move(secondValue));
        }
      }

      return {Krys::Move(arguments)};
    };

    auto functionId = tokens.Peek().FunctionId();
    if (functionId != CSSValueId::Translate)
    {
      return {};
    }

    auto rangeCopy = tokens;
    auto args = ConsumeFunction(rangeCopy);
    if (args.IsAtEnd())
    {
      return {};
    }

    auto parameters = consumeParameters(args, state);
    if (!parameters || !args.IsAtEnd())
    {
      return {};
    }

    tokens = rangeCopy;
    return CSSFunctionValue::Create(functionId, Krys::Move(*parameters));
  }

  RefPtr<CSSValue> ConsumeTranslate3dFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#funcdef-translate3d
    // translate3d() = translate3d( <length-percentage> , <length-percentage> , <length> )

    auto consumeParameters = [](auto &args, auto &state) -> std::optional<CSSValueListBuilder>
    {
      auto firstValue = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(args, state);
      if (!firstValue)
      {
        return {};
      }

      if (!ConsumeComma(args))
      {
        return {};
      }

      auto secondValue = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(args, state);
      if (!secondValue)
      {
        return {};
      }

      if (!ConsumeComma(args))
      {
        return {};
      }

      auto thirdValue = CSSPrimitiveValueResolver<Length<>>::ConsumeAndResolve(args, state);
      if (!thirdValue)
      {
        return {};
      }

      CSSValueListBuilder parameters;
      parameters.push_back(Krys::Move(firstValue));
      parameters.push_back(Krys::Move(secondValue));
      parameters.push_back(Krys::Move(thirdValue));

      return {Krys::Move(parameters)};
    };

    auto functionId = tokens.Peek().FunctionId();
    if (functionId != CSSValueId::Translate3d)
    {
      return {};
    }

    auto rangeCopy = tokens;
    auto args = ConsumeFunction(rangeCopy);
    if (args.IsAtEnd())
    {
      return {};
    }

    auto parameters = consumeParameters(args, state);
    if (!parameters || !args.IsAtEnd())
    {
      return {};
    }

    tokens = rangeCopy;
    return CSSFunctionValue::Create(functionId, Krys::Move(*parameters));
  }

  RefPtr<CSSValue> ConsumeTranslate(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#propdef-translate
    // none | <length-percentage> [ <length-percentage> <length>? ]?

    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    // https://drafts.csswg.org/css-transforms-2/#propdef-translate
    //
    // The translate property accepts 1-3 values, each specifying a translation against one axis, in the order
    // X, Y, then Z. If only one or two values are given, this specifies a 2d translation, equivalent to the
    // translate() function. If the second value is missing, it defaults to 0px. If three values are given,
    // this specifies a 3d translation, equivalent to the translate3d() function.

    auto x = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state);
    if (!x)
    {
      return nullptr;
    }

    tokens.DiscardWhitespace();

    if (tokens.IsAtEnd())
    {
      return CSSValueList::CreateSpaceSeparated(Krys::Move(x));
    }

    auto y = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state);
    if (!y)
    {
      return nullptr;
    }

    tokens.Discard();

    // If we have a calc() or non-zero y value, we can directly add it to the list. We only
    // want to add a zero y value if a non-zero z value is specified.
    // Always include 0% in serialization per-spec.
    bool haveNonZeroY = y->IsCalculated() || y->IsPercentage() || !*y->IsZero();

    if (tokens.IsAtEnd())
    {
      if (!haveNonZeroY)
      {
        return CSSValueList::CreateSpaceSeparated(Krys::Move(x));
      }

      return CSSValueList::CreateSpaceSeparated(Krys::Move(x), Krys::Move(y));
    }

    auto z = CSSPrimitiveValueResolver<Length<>>::ConsumeAndResolve(tokens, state);
    if (!z)
    {
      return nullptr;
    }

    // If the z value is a zero value and not a percent value, we have nothing left to add to the list.
    bool haveNonZeroZ = z && (z->IsCalculated() || z->IsPercentage() || !*z->IsZero());

    if (!haveNonZeroY && !haveNonZeroZ)
    {
      return CSSValueList::CreateSpaceSeparated(Krys::Move(x));
    }

    if (!haveNonZeroZ)
    {
      return CSSValueList::CreateSpaceSeparated(Krys::Move(x), Krys::Move(y));
    }

    return CSSValueList::CreateSpaceSeparated(Krys::Move(x), Krys::Move(y), Krys::Move(z));
  }

  RefPtr<CSSValue> ConsumeRotate(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#propdef-rotate
    // none | <angle> | [ x | y | z | <number>{3} ] && <angle>

    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    // https://www.w3.org/TR/css-transforms-2/#propdef-rotate
    //
    // The rotate property accepts an angle to rotate an element, and optionally an axis to rotate it around.
    //
    // If the axis is omitted, this specifies a 2d rotation, equivalent to the rotate() function.
    //
    // Otherwise, it specifies a 3d rotation: if x, y, or z is given, it specifies a rotation around that
    // axis, equivalent to the rotateX()/etc 3d transform functions. Alternately, the axis can be specified
    // explicitly by giving three numbers representing the x, y, and z components of an origin-centered
    // vector, equivalent to the rotate3d() function.

    CSSValueListBuilder list;
    RefPtr<CSSPrimitiveValue> angle;
    RefPtr<CSSPrimitiveValue> axisIdentifier;

    while (!tokens.IsAtEnd())
    {
      // First, attempt to parse a number, which might be in a series of 3 specifying the rotation axis.
      auto parsedValue = CSSPrimitiveValueResolver<Number<>>::ConsumeAndResolve(tokens, state);
      if (parsedValue)
      {
        // If we've encountered an axis identifier, then this value is invalid.
        if (axisIdentifier)
        {
          return nullptr;
        }

        list.push_back(Krys::Move(parsedValue));
        tokens.DiscardWhitespace();

        continue;
      }

      // Then, attempt to parse an angle. We try this as a fallback rather than the first option because
      // a unitless 0 angle would be consumed as an angle.
      parsedValue = CSSPrimitiveValueResolver<Angle<>>::ConsumeAndResolve(tokens, state);
      if (parsedValue)
      {
        // If we had already parsed an angle or numbers but not 3 in a row, this value is invalid.
        if (angle || (!list.empty() && list.size() != 3))
        {
          return nullptr;
        }

        angle = Krys::Move(parsedValue);
        tokens.DiscardWhitespace();

        continue;
      }

      // Finally, attempt to parse one of the axis identifiers.
      parsedValue = ConsumeIdent<CSSValueId::X, CSSValueId::Y, CSSValueId::Z>(tokens);
      // If we failed to find one of those identifiers or one was already specified, or we'd previously
      // encountered numbers to specify a rotation axis, then this value is invalid.
      if (!parsedValue || axisIdentifier || !list.empty())
      {
        return nullptr;
      }

      axisIdentifier = Krys::Move(parsedValue);
      tokens.DiscardWhitespace();
    }

    // We must have an angle to have a valid value.
    if (!angle)
    {
      return nullptr;
    }

    auto KnownToBeZero = [](std::optional<bool> value) -> bool
    {
      return !value ? false : *value == true;
    };

    auto KnownToBeNotZero = [](std::optional<bool> value) -> bool
    {
      return !value ? false : *value == false;
    };

    if (list.size() == 3)
    {
      // The first valid case is if we have 3 items in the list, meaning we parsed three consecutive number
      // values to specify the rotation axis. In that case, we must not also have encountered an axis
      // identifier.
      assert(!axisIdentifier);

      // Now we must check the values since if we have a vector in the x, y or z axis alone we must serialize
      // to the matching identifier.
      auto xIsZero = Downcast<CSSPrimitiveValue>(list[0].get())->IsZero();
      auto yIsZero = Downcast<CSSPrimitiveValue>(list[1].get())->IsZero();
      auto zIsZero = Downcast<CSSPrimitiveValue>(list[2].get())->IsZero();

      if (KnownToBeNotZero(xIsZero) && KnownToBeZero(yIsZero) && KnownToBeZero(zIsZero))
      {
        return CSSValueList::CreateSpaceSeparated(CSSPrimitiveValue::Create(CSSValueId::X), angle);
      }

      if (KnownToBeZero(xIsZero) && KnownToBeNotZero(yIsZero) && KnownToBeZero(zIsZero))
      {
        return CSSValueList::CreateSpaceSeparated(CSSPrimitiveValue::Create(CSSValueId::Y), angle);
      }

      if (KnownToBeZero(xIsZero) && KnownToBeZero(yIsZero) && KnownToBeNotZero(zIsZero))
      {
        return CSSValueList::CreateSpaceSeparated(angle);
      }

      list.push_back(Krys::Move(angle));
      return CSSValueList::CreateSpaceSeparated(Krys::Move(list));
    }

    if (list.empty())
    {
      // The second valid case is if we have no item in the list, meaning we have either an optional rotation
      // axis using an identifier. In that case, we must add the axis identifier is specified and then add the
      // angle.
      if (axisIdentifier && axisIdentifier->ValueId() != CSSValueId::Z)
      {
        return CSSValueList::CreateSpaceSeparated(Krys::Move(axisIdentifier), Krys::Move(angle));
      }

      return CSSValueList::CreateSpaceSeparated(Krys::Move(angle));
    }

    return nullptr;
  }

  RefPtr<CSSValue> ConsumeScale(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#propdef-scale
    // none | [ <number> | <percentage> ]{1,3}

    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    // https://www.w3.org/TR/css-transforms-2/#propdef-scale
    //
    // The scale property accepts 1-3 values, each specifying a scale along one axis, in order X, Y, then Z.
    //
    // If only the X value is given, the Y value defaults to the same value.
    //
    // If one or two values are given, this specifies a 2d scaling, equivalent to the scale() function.
    // If three values are given, this specifies a 3d scaling, equivalent to the scale3d() function.

    auto x = ConsumePercentageDividedBy100OrNumber(tokens, state);
    if (!x)
    {
      return nullptr;
    }

    tokens.DiscardWhitespace();

    if (tokens.IsAtEnd())
    {
      return CSSValueList::CreateSpaceSeparated(Krys::Move(x));
    }

    auto y = ConsumePercentageDividedBy100OrNumber(tokens, state);
    if (!y)
    {
      return nullptr;
    }

    tokens.DiscardWhitespace();

    auto xValue = x->ResolveAsNumberIfNotCalculated();
    auto yValue = y->ResolveAsNumberIfNotCalculated();

    if (tokens.IsAtEnd())
    {
      if (!xValue || !yValue || *xValue != *yValue)
      {
        return CSSValueList::CreateSpaceSeparated(Krys::Move(x), Krys::Move(y));
      }

      return CSSValueList::CreateSpaceSeparated(Krys::Move(x));
    }

    auto z = ConsumePercentageDividedBy100OrNumber(tokens, state);
    if (!z)
    {
      return nullptr;
    }

    auto zValue = z->ResolveAsNumberIfNotCalculated();

    if (zValue != 1.0)
    {
      return CSSValueList::CreateSpaceSeparated(Krys::Move(x), Krys::Move(y), Krys::Move(z));
    }

    if (!xValue || !yValue || *xValue != *yValue)
    {
      return CSSValueList::CreateSpaceSeparated(Krys::Move(x), Krys::Move(y));
    }

    return CSSValueList::CreateSpaceSeparated(Krys::Move(x));
  }
}