#include "Krystal.HTML/CSS/StyleSheetContents.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSStyleRule.hpp"

namespace Krys::HTML
{
  size_t StyleSheetContents::TotalRules() const noexcept
  {
    size_t count = 0uz;

    count += _importRules.size();
    count += _namespaceRules.size();
    count += _childRules.size();

    return count;
  }

  RawPtr<CSSRule> StyleSheetContents::RuleAt(size_t index) const noexcept
  {
    assert(index < TotalRules());

    if (index < _importRules.size())
    {
      return _importRules[index].get();
    }

    index -= _importRules.size();

    if (index < _namespaceRules.size())
      return _namespaceRules[index].get();

    index -= _namespaceRules.size();

    return _childRules[index].get();
  }
}