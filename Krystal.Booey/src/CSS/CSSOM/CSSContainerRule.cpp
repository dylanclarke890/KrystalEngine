#include "Krystal.Booey/CSS/CSSOM/CSSContainerRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

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
    CSSOMString builder;
    builder.append(u8"@container ");
    cq::Serialise(builder, GetContainerRule().ContainerQuery());
    AppendCSSTextForItems(builder);
    return builder;
  }

  CSSOMString CSSContainerRule::ConditionText() const noexcept
  {
    CSSOMString builder;
    cq::Serialise(builder, GetContainerRule().ContainerQuery());
    return builder;
  }

  CSSOMString CSSContainerRule::ContainerName() const noexcept
  {
    CSSOMString builder;
    auto name = GetContainerRule().ContainerQuery().Name;
    if (name != CSSOMStringAtom::Null() && !name.View().empty())
    {
      SerialiseIdentifier(CSSOMString(name.View()), builder);
    }
    return builder;
  }

  CSSOMString CSSContainerRule::ContainerQuery() const noexcept
  {
    CSSOMString builder;
    mq::Serialise(builder, GetContainerRule().ContainerQuery().Condition);
    return builder;
  }
}
