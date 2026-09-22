#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/MediaList.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

namespace krys::boo::css
{
  CSSStyleSheet::CSSStyleSheet(CSSStyleSheetInit options) noexcept
  {
  }

#pragma region CSSStyleSheet

  RefPtr<CSSImportRule> CSSStyleSheet::OwnerRule() const noexcept
  {
    return _ownerRule.lock();
  }

  dom::ExceptionOr<Ref<CSSRuleList>> CSSStyleSheet::CssRules() const noexcept
  {
    if (!_originClean)
    {
      return dom::ExceptionCode::SecurityError;
    }

    if (_cssRules == nullptr)
    {
      _cssRules = CreateUnique<CSSRuleList>();
    }

    return ShareRef(*_cssRules);
  }

  dom::ExceptionOr<size_t> CSSStyleSheet::InsertRule(CSSOMString &&rule, size_t index) noexcept
  {
    if (!_originClean)
    {
      return dom::ExceptionCode::SecurityError;
    }

    if (_disallowModification)
    {
      return dom::ExceptionCode::NotAllowedError;
    }

    if (index > _contents->TotalRules())
    {
      return dom::ExceptionCode::IndexSizeError;
    }

    RefPtr<CSSRule> parsedRule = Parser::ParseRule(krys::move(rule), {}, AllowedRules::Import);
    if (parsedRule == nullptr)
    {
      return dom::ExceptionCode::SyntaxError;
    }

    if (_constructed && Is<CSSImportRule>(parsedRule))
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError,
                             u8"Cannot insert an @import rule in a constructed CSSStyleSheet object"};
    }

    bool isNamespace = Is<CSSNamespaceRule>(parsedRule);
    bool success = _contents.get()->InsertRule(krys::move(parsedRule), index);
    if (!success)
    {
      if (isNamespace)
      {
        return dom::ExceptionCode::InvalidStateError;
      }

      return dom::ExceptionCode::HierarchyRequestError;
    }

    return index;
  }

  dom::ExceptionOr<void> CSSStyleSheet::DeleteRule(size_t index) noexcept
  {
    if (index > _contents->TotalRules())
    {
      return dom::ExceptionCode::IndexSizeError;
    }

    if (!_contents->DeleteRule(index))
    {
      return dom::ExceptionCode::InvalidStateError;
    }

    return {};
  }

#pragma endregion

#pragma region StyleSheet

#pragma endregion
}