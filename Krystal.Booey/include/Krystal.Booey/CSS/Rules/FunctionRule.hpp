#pragma once

#include "Krystal.Booey/CSS/Parser/CustomPropertySyntax.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"

namespace krys::boo::css
{
  /// @brief `@function`: a custom function's name, parameters, return type and body.
  class FunctionRule final : public GroupRule
  {
  public:
    struct Parameter
    {
      CSSOMStringAtom Name;
      CustomPropertySyntax Type;
      RefPtr<VariableData> DefaultValue;
    };

  private:
    const CSSOMStringAtom _name;
    const List<Parameter> _parameters;
    const CustomPropertySyntax _returnType;

    FunctionRule(const CSSOMStringAtom &name, List<Parameter> &&parameters, CustomPropertySyntax &&returnType,
                 List<Ref<Rule>> &&rules) noexcept;
    FunctionRule(const FunctionRule &other) noexcept;

  public:
    KRYS_NODISCARD static Ref<FunctionRule> Create(const CSSOMStringAtom &name, List<Parameter> &&parameters,
                                                   CustomPropertySyntax &&returnType,
                                                   List<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD Ref<FunctionRule> Copy() const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const List<Parameter> &Parameters() const noexcept
    {
      return _parameters;
    }

    KRYS_NODISCARD const CustomPropertySyntax &ReturnType() const noexcept
    {
      return _returnType;
    }
  };

  /// @brief The declarations in the body of `@function`.
  class FunctionDeclarationsRule final : public Rule
  {
  private:
    Ref<StyleProperties> _properties;

    explicit FunctionDeclarationsRule(Ref<StyleProperties> &&properties) noexcept;
    FunctionDeclarationsRule(const FunctionDeclarationsRule &other) noexcept;

  public:
    ~FunctionDeclarationsRule() noexcept;

    KRYS_NODISCARD static Ref<FunctionDeclarationsRule> Create(Ref<StyleProperties> &&properties) noexcept;

    KRYS_NODISCARD Ref<FunctionDeclarationsRule> Copy() const noexcept;

    /// @note Only contains the `result` descriptor and custom properties.
    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return *_properties;
    }

    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FunctionRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.Type() == krys::boo::css::RuleType::Function;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FunctionDeclarationsRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.Type() == krys::boo::css::RuleType::FunctionDeclarations;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
