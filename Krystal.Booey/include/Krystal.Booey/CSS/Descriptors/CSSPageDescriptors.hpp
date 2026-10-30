#pragma once

#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class CSSPageRule;

  /// @brief The descriptors of an `@page` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#csspagedescriptors
  class CSSPageDescriptors final : public PropertySetCSSDescriptors
  {
  private:
    CSSPageDescriptors(MutableStyleProperties &propertySet, CSSPageRule &parentRule) noexcept;

  public:
    ~CSSPageDescriptors() noexcept;

    KRYS_NODISCARD static Ref<CSSPageDescriptors> Create(MutableStyleProperties &propertySet,
                                                         CSSPageRule &parentRule) noexcept;

    KRYS_NODISCARD StyleDeclarationType GetStyleDeclarationType() const noexcept final
    {
      return StyleDeclarationType::Page;
    }

    KRYS_NODISCARD CSSOMString Margin() const noexcept;
    dom::ExceptionOr<void> SetMargin(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginTop() const noexcept;
    dom::ExceptionOr<void> SetMarginTop(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginRight() const noexcept;
    dom::ExceptionOr<void> SetMarginRight(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginBottom() const noexcept;
    dom::ExceptionOr<void> SetMarginBottom(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginLeft() const noexcept;
    dom::ExceptionOr<void> SetMarginLeft(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString Size() const noexcept;
    dom::ExceptionOr<void> SetSize(const CSSOMString &value) noexcept;

  private:
    KRYS_NODISCARD RuleType GetRuleType() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPageDescriptors)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.GetStyleDeclarationType() == krys::boo::css::StyleDeclarationType::Page;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
