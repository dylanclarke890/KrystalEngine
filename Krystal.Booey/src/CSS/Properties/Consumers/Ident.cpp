#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<PrimitiveValue> ConsumeIdent(TokenRange &tokens) noexcept
  {
    if (auto result = ConsumeIdentRaw(tokens))
    {
      return PrimitiveValue::Create(*result);
    }

    return nullptr;
  }

  RefPtr<PrimitiveValue> ConsumeIdentRange(TokenRange &tokens, ValueId lower, ValueId upper) noexcept
  {
    auto value = ConsumeIdentRangeRaw(tokens, lower, upper);
    if (!value)
    {
      return nullptr;
    }

    return PrimitiveValue::Create(*value);
  }

  RefPtr<PrimitiveValue> ConsumeCustomIdent(TokenRange &tokens, bool shouldLowercase) noexcept
  {
    auto identifier = ConsumeCustomIdentRaw(tokens, shouldLowercase);
    if (!identifier)
    {
      return nullptr;
    }

    return PrimitiveValue::CreateCustomIdent(*identifier);
  }

  RefPtr<PrimitiveValue> ConsumeDashedIdent(TokenRange &tokens, bool shouldLowercase) noexcept
  {
    auto identifier = ConsumeDashedIdentRaw(tokens, shouldLowercase);
    if (!identifier)
    {
      return nullptr;
    }

    return PrimitiveValue::CreateCustomIdent(*identifier);
  }
}
