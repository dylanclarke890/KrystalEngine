#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
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
    return _ownerRule.lock();
  }

  ExceptionOr<Ref<CSSRuleList>> CSSStyleSheet::CssRules() const noexcept
  {
    if (!_originClean)
    {
      return ExceptionCode::SecurityError;
    }

    if (_cssRules == nullptr)
    {
      _cssRules = CreateUnique<CSSRuleList>();
    }

    return ShareRef(*_cssRules);
  }

  ExceptionOr<size_t> CSSStyleSheet::InsertRule(CSSOMString &&rule, size_t index) noexcept
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

    RefPtr<CSSRule> parsedRule = CSSParser::ParseRule(Krys::Move(rule), CSSAllowedRules::Import);
    if (parsedRule == nullptr)
    {
      return ExceptionCode::SyntaxError;
    }

    if (_constructed && Is<CSSImportRule>(parsedRule))
    {
      return Exception {ExceptionCode::SyntaxError,
                        u8"Cannot insert an @import rule in a constructed CSSStyleSheet object"};
    }

    bool isNamespace = Is<CSSNamespaceRule>(parsedRule);
    bool success = _contents.get()->InsertRule(Krys::Move(parsedRule), index);
    if (!success)
    {
      if (isNamespace)
      {
        return ExceptionCode::InvalidStateError;
      }

      return ExceptionCode::HierarchyRequestError;
    }

    return index;
  }

  ExceptionOr<void> CSSStyleSheet::DeleteRule(size_t index) noexcept
  {
    if (index > _contents->TotalRules())
    {
      return ExceptionCode::IndexSizeError;
    }

    if (!_contents->DeleteRule(index))
    {
      return ExceptionCode::InvalidStateError;
    }

    return {};
  }

#pragma endregion

#pragma region StyleSheet

#pragma endregion
}