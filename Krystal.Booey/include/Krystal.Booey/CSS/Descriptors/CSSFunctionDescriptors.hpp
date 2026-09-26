#pragma once

#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"

namespace krys::boo::css
{
  class CSSFunctionDeclarations;

  struct ParserContext;

  class CSSFunctionDescriptors final : public PropertySetCSSDescriptors
  {
  private:
    CSSFunctionDescriptors(MutableStyleProperties &properties,
                           CSSFunctionDeclarations &declarations) noexcept;

  public:
    virtual ~CSSFunctionDescriptors() noexcept;

    KRYS_NODISCARD static Ref<CSSFunctionDescriptors> Create(MutableStyleProperties &propertySet,
                                                             CSSFunctionDeclarations &parentRule) noexcept
    {
      return AdoptRef(*new CSSFunctionDescriptors(propertySet, parentRule));
    }

    KRYS_NODISCARD StyleDeclarationType DeclarationType() const noexcept final
    {
      return StyleDeclarationType::Function;
    }

    KRYS_NODISCARD css::RuleType RuleType() const noexcept final
    {
      return RuleType::FunctionDeclarations;
    }

    KRYS_NODISCARD CSSOMString Result() const noexcept;

    KRYS_NODISCARD dom::ExceptionOr<void> Result(const CSSOMString &result) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFunctionDescriptors)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.DeclarationType() == krys::boo::css::StyleDeclarationType::Function;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
