#include "Krystal.Booey/CSS/CSSOM/CSSContainerRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
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
    cq::Serialise(builder, GetContainerRule().ContainerQuery());
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSContainerRule::ConditionText() const noexcept
  {
    StringBuilder builder;
    cq::Serialise(builder, GetContainerRule().ContainerQuery());
    return krys::move(builder).ToString();
  }

  CSSOMString CSSContainerRule::ContainerName() const noexcept
  {
    StringBuilder builder;
    auto name = GetContainerRule().ContainerQuery().Name;
    if (name != CSSOMStringAtom::Null() && !name.View().empty())
    {
      SerialiseIdentifier(CSSOMString(name.View()), builder);
    }
    return krys::move(builder).ToString();
  }

  CSSOMString CSSContainerRule::ContainerQuery() const noexcept
  {
    StringBuilder builder;
    mq::Serialise(builder, GetContainerRule().ContainerQuery().Condition);
    return krys::move(builder).ToString();
  }
}
