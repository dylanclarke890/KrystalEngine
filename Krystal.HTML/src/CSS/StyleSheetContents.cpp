#include "Krystal.HTML/CSS/StyleSheetContents.hpp"
#include "Krystal.HTML/CSS/CSSCharsetRule.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleRule.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"

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
    {
      return _namespaceRules[index].get();
    }

    index -= _namespaceRules.size();
    return _childRules[index].get();
  }

  bool StyleSheetContents::InsertRule(Ref<CSSRule> &&rule, size_t index) noexcept
  {
    assert(!Is<CSSCharsetRule>(rule.get()));
    assert(index <= TotalRules());

    if (index < _importRules.size() || (index == _importRules.size() && Is<CSSImportRule>(rule)))
    {
      // Inserting non-import rule before @import is not allowed.
      auto *importRule = DynamicDowncast<CSSImportRule>(rule.get());
      if (importRule == nullptr)
      {
        return false;
      }

      _importRules.insert(_importRules.begin() + index, ShareRef(*importRule));
      return true;
    }

    // Inserting @import rule after a non-import rule is not allowed.
    if (Is<CSSImportRule>(rule))
    {
      return false;
    }
    index -= _importRules.size();

    if (index < _namespaceRules.size() || (index == _namespaceRules.size() && Is<CSSNamespaceRule>(rule)))
    {
      // Inserting non-namespace rules other than import rules before @namespace is not allowed.
      auto *namespaceRule = DynamicDowncast<CSSNamespaceRule>(rule.get());
      if (namespaceRule == nullptr)
      {
        return false;
      }

      // Inserting @namespace rule when rules other than import/namespace/charset
      // are present is not allowed.
      if (!_childRules.empty())
      {
        return false;
      }

      _namespaceRules.insert(_namespaceRules.begin() + index, ShareRef(*namespaceRule));

      // TODO: look at the below
      // For now, to be compatible with IE and Firefox, if a namespace rule with the same prefix is added, it
      // overwrites previous ones.
      // TODO: The eventual correct behavior would be to ensure that the last value in the list wins.
      // parserAddNamespace(namespaceRule->prefix(), namespaceRule->uri());

      return true;
    }

    // Inserting @namespace rule after a non-namespace rule is not allowed.
    if (Is<CSSNamespaceRule>(rule))
    {
      return false;
    }
    index -= _namespaceRules.size();

    _childRules.insert(_childRules.begin() + index, Krys::Move(rule));
    return true;
  }

  bool StyleSheetContents::DeleteRule(size_t index) noexcept
  {
    // TODO: Set old rule’s parent CSS rule and parent CSS style sheet to null.
    assert(index < TotalRules());

    if (index < _importRules.size())
    {
      _importRules.erase(_importRules.begin() + index);
      return true;
    }
    index -= _importRules.size();

    if (index < _namespaceRules.size())
    {
      // Deleting @namespace rule when list contains anything other than @import or @namespace rules is not
      // allowed.
      if (!_childRules.empty())
      {
        return false;
      }

      _namespaceRules.erase(_namespaceRules.begin() + index);
      return true;
    }
    index -= _namespaceRules.size();

    _childRules.erase(_childRules.begin() + index);

    return true;
  }
}