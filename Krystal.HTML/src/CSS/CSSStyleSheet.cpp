#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include "Krystal.HTML/CSS/Parser/CSSParser.hpp"
#include "Krystal.HTML/CSS/StyleSheetContents.hpp"

namespace Krys::HTML
{
  CSSStyleSheet::CSSStyleSheet(CSSStyleSheetInit options) noexcept
  {
  }

#pragma region CSSStyleSheet

  RefPtr<CSSImportRule> CSSStyleSheet::OwnerRule() const noexcept
  {
    return _ownerCssRule.lock();
  }

  ExceptionOr<Ref<CSSRuleList>> CSSStyleSheet::CssRules() const noexcept
  {
    if (!_originClean)
    {
      return ExceptionCode::SecurityError;
    }

    if (!_cssRules)
    {
      _cssRules = CreateUnique<CSSRuleList>();
    }

    return ShareRef(*_cssRules);
  }

  ExceptionOr<size_t> CSSStyleSheet::InsertRule(CSSOMStringView rule, size_t index) noexcept
  {
    if (!_originClean)
    {
      return ExceptionCode::SecurityError;
    }

    if (_disallowModification)
    {
      return ExceptionCode::NotAllowedError;
    }

    if (index > _contents->TotalRules())
    {
      return ExceptionCode::IndexSizeError;
    }

    //RefPtr<CSSRule> rule = CSSParser::ParseRule(ruleString, m_contents.ptr(), CSSAllowedRules::Import);

    //if (rule == nullptr)
    //{
    //  return Exception {ExceptionCode::SyntaxError};
    //}

    //if (_constructed && Is<CSSImportRule>(rule))
    //{
    //  return Exception {
    //    ExceptionCode::SyntaxError,
    //    utf8_string(u8"Cannot inserted an @import rule in a constructed CSSStyleSheet object")};
    //}

    return ExceptionCode::NotSupportedError;
  }

#pragma endregion

#pragma region StyleSheet

#pragma endregion
}