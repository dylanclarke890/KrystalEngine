#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSFunctionDescriptors;
  class FunctionDeclarationsRule;

  /// @brief The declarations of an `@function` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-mixins/#cssfunctiondeclarations
  class CSSFunctionDeclarations final : public CSSRule
  {
  private:
    Ref<FunctionDeclarationsRule> _styleRule;
    RefPtr<CSSFunctionDescriptors> _descriptorsCSSOMWrapper;

    CSSFunctionDeclarations(FunctionDeclarationsRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSFunctionDeclarations() noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionDeclarations> Create(FunctionDeclarationsRule &rule,
                                                              CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSFunctionDescriptors &Style() noexcept;

  private:
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::FunctionDeclarations;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFunctionDeclarations)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::FunctionDeclarations;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
