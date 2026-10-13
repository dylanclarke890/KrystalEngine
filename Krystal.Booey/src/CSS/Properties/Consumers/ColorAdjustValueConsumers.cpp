#include "Krystal.Booey/CSS/Properties/Consumers/ColorAdjust.hpp"
#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/Values/ColorSchemeValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeColorScheme(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    auto colorScheme = ConsumeUnresolvedColorScheme(tokens, state);
    if (!colorScheme)
    {
      return {};
    }

    return ColorSchemeValue::Create(krys::move(*colorScheme));
  }
}
