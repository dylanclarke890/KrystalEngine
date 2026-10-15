#include "Krystal.Booey/CSS/Values/PendingSubstitutionValue.hpp"
#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/Style/Builder.hpp"

namespace krys::boo::css
{
  RefPtr<Value> PendingSubstitutionValue::ResolveValue(style::Builder &builder,
                                                       PropertyId propertyId) const noexcept
  {
    if (!_shorthandValue->ResolveAndCacheValue(
          builder,
          [this](const auto &data)
          {
            ParsedPropertyList parsedProperties;
            if (!PropertyParser::ParseValue(_shorthandPropertyId, IsImportant(false), data->TokenRange(),
                                            data->Context(), parsedProperties, RuleType::Style))
            {
              _cachedPropertyValues = {};
              return;
            }

            _cachedPropertyValues.assign(parsedProperties.begin(), parsedProperties.end());
          }))
    {
      return nullptr;
    }

    for (auto &property : _cachedPropertyValues)
    {
      if (property.Id() == propertyId)
      {
        return ShareRefPtr(&property.Value());
      }
    }

    return nullptr;
  }
}
