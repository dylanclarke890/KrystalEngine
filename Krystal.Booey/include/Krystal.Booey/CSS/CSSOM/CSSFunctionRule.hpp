#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class FunctionRule;

  /// @brief An `@function` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-mixins/#the-function-interface
  class CSSFunctionRule final : public CSSGroupingRule
  {
  public:
    /// @see https://drafts.csswg.org/css-mixins/#dictdef-functionparameter
    struct FunctionParameter
    {
      CSSOMString Name;
      CSSOMString Type;
      Maybe<CSSOMString> DefaultValue;
    };

  private:
    CSSFunctionRule(css::FunctionRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSFunctionRule> Create(css::FunctionRule &rule,
                                                      CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString Name() const noexcept;
    KRYS_NODISCARD List<FunctionParameter> GetParameters() const noexcept;
    KRYS_NODISCARD CSSOMString ReturnType() const noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

  private:
    KRYS_NODISCARD const css::FunctionRule &GetFunctionRule() const noexcept;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Function;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFunctionRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Function;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
