#include "Krystal.Booey/CSS/CSSOM/CSSContainerRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSContainerRule::CSSContainerRule(css::ContainerRule &rule, CSSStyleSheet *parent) noexcept
      : CSSConditionRule(rule, parent)
  {
  }

  Ref<CSSContainerRule> CSSContainerRule::Create(css::ContainerRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSContainerRule(rule, parent));
  }

  const css::ContainerRule &CSSContainerRule::GetContainerRule() const noexcept
  {
    return Downcast<css::ContainerRule>(GetGroupRule());
  }

  CSSOMString CSSContainerRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@container ");
    cq::Serialise(builder, GetContainerRule().ContainerQueries());
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSContainerRule::ConditionText() const noexcept
  {
    StringBuilder builder;
    cq::Serialise(builder, GetContainerRule().ContainerQueries());
    return krys::move(builder).ToString();
  }

  CSSOMString CSSContainerRule::ContainerName() const noexcept
  {
    // If conditions has exactly one item, return that item's name; otherwise the empty string.
    auto conditions = Conditions();
    return conditions.size() == 1 ? krys::move(conditions.front().Name) : CSSOMString();
  }

  CSSOMString CSSContainerRule::ContainerQuery() const noexcept
  {
    // If conditions has exactly one item, return that item's query; otherwise the empty string.
    auto conditions = Conditions();
    return conditions.size() == 1 ? krys::move(conditions.front().Query) : CSSOMString();
  }

  List<CSSContainerCondition> CSSContainerRule::Conditions() const noexcept
  {
    List<CSSContainerCondition> conditions;
    for (const auto &query : GetContainerRule().ContainerQueries())
    {
      StringBuilder name;
      if (query.Name != CSSOMStringAtom::Null() && !query.Name.View().empty())
      {
        SerialiseIdentifier(CSSOMString(query.Name.View()), name);
      }
      StringBuilder condition;
      mq::Serialise(condition, query.Condition);
      conditions.push_back({krys::move(name).ToString(), krys::move(condition).ToString()});
    }
    return conditions;
  }
}
