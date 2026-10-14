#include "Krystal.Booey/CSS/CSSOM/CSSFunctionRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"

namespace krys::boo::css
{
  CSSFunctionRule::CSSFunctionRule(css::FunctionRule &rule, CSSStyleSheet *parent) noexcept
      : CSSGroupingRule(rule, parent)
  {
  }

  Ref<CSSFunctionRule> CSSFunctionRule::Create(css::FunctionRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSFunctionRule(rule, parent));
  }

  CSSOMString CSSFunctionRule::Name() const noexcept
  {
    return CSSOMString(GetFunctionRule().Name().View());
  }

  List<CSSFunctionRule::FunctionParameter> CSSFunctionRule::GetParameters() const noexcept
  {
    List<FunctionParameter> result;

    for (const auto &parameter : GetFunctionRule().Parameters())
    {
      const auto &defaultValue = parameter.DefaultValue;
      result.push_back({
        .Name = CSSOMString(parameter.Name.View()),
        .Type = u8"*", // FIXME: Implement.
        .DefaultValue = defaultValue ? Maybe<CSSOMString>(defaultValue->Serialise()) : null,
      });
    }

    return result;
  }

  CSSOMString CSSFunctionRule::ReturnType() const noexcept
  {
    return u8"*";
  }

  CSSOMString CSSFunctionRule::CssText() const noexcept
  {
    CSSOMString builder;
    builder.append(u8"@function ");
    SerialiseIdentifier(Name(), builder);
    builder.push_back(u8'(');

    CSSOMStringView separator = u8"";
    for (const auto &parameter : GetFunctionRule().Parameters())
    {
      builder.append(separator);
      SerialiseIdentifier(CSSOMString(parameter.Name.View()), builder);
      // FIMXE: Serialize the type.
      if (const auto &defaultValue = parameter.DefaultValue)
      {
        builder.append(u8": ").append(defaultValue->Serialise());
      }
      separator = u8", ";
    }

    builder.append(u8") { ");
    for (uint32 index = 0; index < Length(); ++index)
    {
      auto ruleText = Item(index)->CssText();
      builder.append(ruleText).push_back(u8' ');
    }

    builder.push_back(u8'}');
    return builder;
  }

  const css::FunctionRule &CSSFunctionRule::GetFunctionRule() const noexcept
  {
    return Downcast<css::FunctionRule>(GetGroupRule());
  }
}
