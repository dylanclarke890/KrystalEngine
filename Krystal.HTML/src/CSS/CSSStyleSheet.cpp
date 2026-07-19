#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"

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

    // TODO(CSSStyleSheet): Implement InsertRule(), needs a CSSRule parser.
    return ExceptionCode::NotSupportedError;
  }

#pragma endregion

#pragma region StyleSheet

#pragma endregion
}