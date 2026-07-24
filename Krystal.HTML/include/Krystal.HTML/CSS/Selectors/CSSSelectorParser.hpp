#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelectorList.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelectorParserContext.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class StyleSheetContents;

  class CSSSelectorParser
  {
  private:
    const CSSSelectorParserContext _context;
    const RefPtr<StyleSheetContents> _stylesheet;

  public:
    CSSSelectorParser(const CSSSelectorParserContext &context, RawPtr<StyleSheetContents> stylesheet) noexcept
        : _context(context), _stylesheet(ShareRefPtr(stylesheet))
    {
    }

    KRYS_NODISCARD Maybe<CSSSelectorList> ParseSelector(CSSOMString &&input,
                                                        const CSSSelectorParserContext &context,
                                                        RawPtr<StyleSheetContents> stylesheet) noexcept;

    KRYS_NODISCARD Maybe<CSSSelectorList> ParseSelector(CSSTokenRange tokens,
                                                        const CSSSelectorParserContext &context,
                                                        RawPtr<StyleSheetContents> stylesheet) noexcept;

  private:
  };
}