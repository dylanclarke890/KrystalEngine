#pragma once
#include "Krystal.Booey/CSS/Parser/Context/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Parser/Context/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"

namespace krys::boo::css
{
  class PropertyParser
  {
  public:
    KRYS_NODISCARD static bool ParseValue(TokenRange tokens, const ParserContext &context, PropertyId id,
                                          RuleType ruleType, IsImportant important,
                                          ParsedPropertyList &properties) noexcept;

    KRYS_NODISCARD static bool ConsumeStyleProperty(TokenRange &tokens, const ParserContext &context,
                                                    PropertyId id, IsImportant important,
                                                    RuleType ruleType,
                                                    ParsedPropertyList &properties) noexcept;
  };
}