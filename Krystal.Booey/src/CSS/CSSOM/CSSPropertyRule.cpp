#include "Krystal.Booey/CSS/CSSOM/CSSPropertyRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSPropertyRule::CSSPropertyRule(css::PropertyRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _propertyRule(ShareRef(rule))
  {
  }

  CSSPropertyRule::~CSSPropertyRule() noexcept = default;

  Ref<CSSPropertyRule> CSSPropertyRule::Create(css::PropertyRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSPropertyRule(rule, parent));
  }

  CSSOMString CSSPropertyRule::Name() const noexcept
  {
    return CSSOMString(_propertyRule->GetDescriptor().Name.View());
  }

  CSSOMString CSSPropertyRule::Syntax() const noexcept
  {
    return _propertyRule->GetDescriptor().Syntax;
  }

  bool CSSPropertyRule::Inherits() const noexcept
  {
    return _propertyRule->GetDescriptor().Inherits.value_or(false);
  }

  Maybe<CSSOMString> CSSPropertyRule::InitialValue() const noexcept
  {
    if (!_propertyRule->GetDescriptor().InitialValue)
    {
      return null;
    }
    return _propertyRule->GetDescriptor().InitialValue->Serialise();
  }

  CSSOMString CSSPropertyRule::CssText() const noexcept
  {
    StringBuilder builder;
    const auto &descriptor = _propertyRule->GetDescriptor();

    builder.Append(u8"@property ");
    SerialiseIdentifier(CSSOMString(descriptor.Name.View()), builder);
    builder.Append(u8" { ");

    if (!descriptor.Syntax.empty())
    {
      builder.Append(u8"syntax: ");
      SerialiseString(Syntax(), builder);
      builder.Append(u8"; ");
    }

    if (descriptor.Inherits)
    {
      builder.Append(u8"inherits: ", *descriptor.Inherits ? u8"true" : u8"false", u8"; ");
    }

    if (descriptor.InitialValue)
    {
      builder.Append(u8"initial-value: ", *InitialValue(), u8"; ");
    }

    builder.Append(u8'}');
    return krys::move(builder).ToString();
  }

  void CSSPropertyRule::Reattach(Rule &rule) noexcept
  {
    _propertyRule = ShareRef(Downcast<css::PropertyRule>(rule));
  }
}
