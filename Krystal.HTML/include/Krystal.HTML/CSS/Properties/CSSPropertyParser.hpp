#pragma once
#include "Krystal.HTML/CSS/Enums/CSSRuleType.hpp"
#include "Krystal.HTML/CSS/Parser/CSSParserContext.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Parser/Types/ParsedCSSPropertyList.hpp"
#include "Krystal.HTML/CSS/Properties/CSSProperty.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"

namespace Krys::HTML
{
  class CSSPropertyParser
  {
  public:
    KRYS_NODISCARD static bool ParseValue(CSSTokenRange tokens, const CSSParserContext &context,
                                          CSSPropertyId id, CSSRuleType ruleType, IsImportant important,
                                          ParsedCSSPropertyList &properties) noexcept;

    KRYS_NODISCARD static bool ConsumeStyleProperty(CSSTokenRange &tokens, const CSSParserContext &context,
                                                    CSSPropertyId id, IsImportant important,
                                                    CSSRuleType ruleType,
                                                    ParsedCSSPropertyList &properties) noexcept;
  };
}