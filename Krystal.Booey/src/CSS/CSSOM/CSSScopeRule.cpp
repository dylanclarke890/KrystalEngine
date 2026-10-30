#include "Krystal.Booey/CSS/CSSOM/CSSScopeRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSScopeRule::CSSScopeRule(css::ScopeRule &rule, CSSStyleSheet *parent) noexcept
      : CSSGroupingRule(rule, parent)
  {
  }

  Ref<CSSScopeRule> CSSScopeRule::Create(css::ScopeRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSScopeRule(rule, parent));
  }

  const css::ScopeRule &CSSScopeRule::GetScopeRule() const noexcept
  {
    return Downcast<css::ScopeRule>(GetGroupRule());
  }

  CSSOMString CSSScopeRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@scope");

    auto start = Start();
    if (!start.empty())
    {
      builder.Append(u8" (", start, u8')');
    }

    auto end = End();
    if (!end.empty())
    {
      builder.Append(u8" to ", u8"(", end, u8')');
    }

    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSScopeRule::Start() const noexcept
  {
    const auto &scope = GetScopeRule().OriginalScopeStart();
    if (scope.IsEmpty())
    {
      return {};
    }
    return scope.SelectorsText();
  }

  CSSOMString CSSScopeRule::End() const noexcept
  {
    const auto &scope = GetScopeRule().OriginalScopeEnd();
    if (scope.IsEmpty())
    {
      return {};
    }
    return scope.SelectorsText();
  }
}
