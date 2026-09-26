#pragma once

#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"

namespace krys::boo::css
{
  class CSSPageRule;

  struct ParserContext;

  class CSSPageDescriptors final : public PropertySetCSSDescriptors
  {
  private:
    CSSPageDescriptors(MutableStyleProperties &properties, CSSPageRule &parentRule) noexcept;

  public:
    virtual ~CSSPageDescriptors() noexcept;

    KRYS_NODISCARD static Ref<CSSPageDescriptors> Create(MutableStyleProperties &propertySet,
                                                         CSSPageRule &parentRule) noexcept
    {
      return AdoptRef(*new CSSPageDescriptors(propertySet, parentRule));
    }

    StyleDeclarationType DeclarationType() const noexcept final
    {
      return StyleDeclarationType::Page;
    }

    KRYS_NODISCARD CSSOMString Margin() const noexcept;
    dom::ExceptionOr<void> Margin(const CSSOMString &value) noexcept;
    
    KRYS_NODISCARD CSSOMString MarginTop() const noexcept;
    dom::ExceptionOr<void> MarginTop(const CSSOMString &value) noexcept;
    
    KRYS_NODISCARD CSSOMString MarginRight() const noexcept;
    dom::ExceptionOr<void> MarginRight(const CSSOMString &value) noexcept;
    
    KRYS_NODISCARD CSSOMString MarginBottom() const noexcept;
    dom::ExceptionOr<void> MarginBottom(const CSSOMString &value) noexcept;
    
    KRYS_NODISCARD CSSOMString MarginLeft() const noexcept;
    dom::ExceptionOr<void> MarginLeft(const CSSOMString &value) noexcept;
    
    KRYS_NODISCARD CSSOMString Size() const noexcept;
    dom::ExceptionOr<void> Size(const CSSOMString &value) noexcept;

  private:
    KRYS_NODISCARD css::RuleType RuleType() const noexcept final;
  };

}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPageDescriptors)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.DeclarationType() == krys::boo::css::StyleDeclarationType::Page;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
