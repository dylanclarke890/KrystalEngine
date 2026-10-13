#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/DynamicRangeLimitValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  // MARK: - Value consuming entry points

  RefPtr<Value> ConsumeColor(TokenRange &tokens, PropertyParserState &state,
                             const ColorParsingOptions &options) noexcept
  {
    if (auto color = ConsumeUnresolvedColor(tokens, state, options))
    {
      // Special case top level color keywords, hex (SRGBA<uint8>) and absolute color values to maintain
      // backwards compatibility and existing optimizations.
      if (auto keyword = color->Keyword())
      {
        return PrimitiveValue::Create(keyword->Value);
      }
      if (auto hex = color->Hex())
      {
        return state.Pool->CreateColorValue(boo::Color {hex->Value});
      }
      if (auto resolved = color->Resolved())
      {
        return state.Pool->CreateColorValue(krys::move(resolved->Value));
      }

      return ColorValue::Create(krys::move(*color));
    }

    return nullptr;
  }

  // MARK: - <'dynamic-range-limit'> (Value)

  RefPtr<Value> ConsumeDynamicRangeLimit(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    auto dynamicRangeLimit = ConsumeUnresolvedDynamicRangeLimit(tokens, state);
    if (!dynamicRangeLimit)
    {
      return {};
    }
    return DynamicRangeLimitValue::Create(krys::move(*dynamicRangeLimit));
  }
}
