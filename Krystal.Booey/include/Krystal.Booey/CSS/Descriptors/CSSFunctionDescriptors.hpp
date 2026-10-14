#pragma once

#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class CSSFunctionDeclarations;

  /// @brief The descriptors of the declarations in an `@function` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-mixins/#cssfunctiondescriptors
  class CSSFunctionDescriptors final : public PropertySetCSSDescriptors
  {
  private:
    CSSFunctionDescriptors(MutableStyleProperties &propertySet, CSSFunctionDeclarations &parentRule) noexcept;

  public:
    ~CSSFunctionDescriptors() noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionDescriptors> Create(MutableStyleProperties &propertySet,
                                                             CSSFunctionDeclarations &parentRule) noexcept;

    KRYS_NODISCARD StyleDeclarationType GetStyleDeclarationType() const noexcept final
    {
      return StyleDeclarationType::Function;
    }

    KRYS_NODISCARD CSSOMString Result() const noexcept;
    dom::ExceptionOr<void> SetResult(const CSSOMString &value) noexcept;

  private:
    KRYS_NODISCARD RuleType GetRuleType() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFunctionDescriptors)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.GetStyleDeclarationType() == krys::boo::css::StyleDeclarationType::Function;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
