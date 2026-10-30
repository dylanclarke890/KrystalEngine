#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"

namespace krys::boo::css
{
  Ref<FunctionRule> FunctionRule::Create(const CSSOMStringAtom &name, List<Parameter> &&parameters,
                                         CustomPropertySyntax &&returnType, List<Ref<Rule>> &&rules) noexcept
  {
    return AdoptRef(
      *new FunctionRule(name, krys::move(parameters), krys::move(returnType), krys::move(rules)));
  }

  FunctionRule::FunctionRule(const CSSOMStringAtom &name, List<Parameter> &&parameters,
                             CustomPropertySyntax &&returnType, List<Ref<Rule>> &&rules) noexcept
      : GroupRule(RuleType::Function, krys::move(rules)), _name(name), _parameters(krys::move(parameters)),
        _returnType(krys::move(returnType))
  {
  }

  FunctionRule::FunctionRule(const FunctionRule &other) noexcept = default;

  Ref<FunctionRule> FunctionRule::Copy() const noexcept
  {
    return AdoptRef(*new FunctionRule(*this));
  }

  FunctionDeclarationsRule::FunctionDeclarationsRule(Ref<StyleProperties> &&properties) noexcept
      : Rule(RuleType::FunctionDeclarations), _properties(krys::move(properties))
  {
  }

  FunctionDeclarationsRule::FunctionDeclarationsRule(const FunctionDeclarationsRule &other) noexcept =
    default;

  FunctionDeclarationsRule::~FunctionDeclarationsRule() noexcept = default;

  Ref<FunctionDeclarationsRule> FunctionDeclarationsRule::Create(Ref<StyleProperties> &&properties) noexcept
  {
    return AdoptRef(*new FunctionDeclarationsRule(krys::move(properties)));
  }

  Ref<FunctionDeclarationsRule> FunctionDeclarationsRule::Copy() const noexcept
  {
    return AdoptRef(*new FunctionDeclarationsRule(*this));
  }

  MutableStyleProperties &FunctionDeclarationsRule::MutableProperties() noexcept
  {
    if (!Is<MutableStyleProperties>(*_properties))
    {
      _properties = _properties->MutableCopy();
    }
    return Downcast<MutableStyleProperties>(*_properties);
  }
}
