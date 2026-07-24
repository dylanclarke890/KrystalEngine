#include "Krystal.HTML/CSS/Selectors/CSSSelectorParser.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelector.hpp"
#include "Krystal.HTML/CSS/StyleSheetContents.hpp"

namespace Krys::HTML
{
  Maybe<CSSSelectorList> CSSSelectorParser::ParseSelector(CSSOMString &&input,
                                                          const CSSSelectorParserContext &context,
                                                          RawPtr<StyleSheetContents> stylesheet) noexcept
  {
    CSSInputStream inputStream(Krys::Move(input));
    return ParseSelector(CSSTokenizer(inputStream).TokenRange(), context, stylesheet);
  }

  Maybe<CSSSelectorList> CSSSelectorParser::ParseSelector(CSSTokenRange tokens,
                                                          const CSSSelectorParserContext &context,
                                                          RawPtr<StyleSheetContents> stylesheet) noexcept
  {
    return Null;
  }
}