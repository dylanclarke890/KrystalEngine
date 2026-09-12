#pragma once
#include "Krystal.Booey/CSS/Parser/Context/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Parser/Context/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"

namespace krys::boo::css
{
  class CSSPropertyParser
  {
  public:
    KRYS_NODISCARD static bool ParseValue(TokenRange tokens, const ParserContext &context, CSSPropertyId id,
                                          RuleType ruleType, IsImportant important,
                                          ParsedPropertyList &properties) noexcept;

    KRYS_NODISCARD static bool ConsumeStyleProperty(TokenRange &tokens, const ParserContext &context,
                                                    CSSPropertyId id, IsImportant important,
                                                    RuleType ruleType,
                                                    ParsedPropertyList &properties) noexcept;
  };
}