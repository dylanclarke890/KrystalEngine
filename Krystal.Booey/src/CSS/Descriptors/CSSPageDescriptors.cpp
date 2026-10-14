#include "Krystal.Booey/CSS/Descriptors/CSSPageDescriptors.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSPageRule.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"

namespace krys::boo::css
{
  CSSPageDescriptors::CSSPageDescriptors(MutableStyleProperties &propertySet,
                                         CSSPageRule &parentRule) noexcept
      : PropertySetCSSDescriptors(propertySet, parentRule)
  {
  }

  CSSPageDescriptors::~CSSPageDescriptors() noexcept = default;

  Ref<CSSPageDescriptors> CSSPageDescriptors::Create(MutableStyleProperties &propertySet,
                                                     CSSPageRule &parentRule) noexcept
  {
    return AdoptRef(*new CSSPageDescriptors(propertySet, parentRule));
  }

  RuleType CSSPageDescriptors::GetRuleType() const noexcept
  {
    return RuleType::Page;
  }

#pragma region Descriptors

  // 'margin'
  CSSOMString CSSPageDescriptors::Margin() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Margin);
  }

  dom::ExceptionOr<void> CSSPageDescriptors::SetMargin(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Margin, value, IsImportant(false));
  }

  // 'margin-top'
  CSSOMString CSSPageDescriptors::MarginTop() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginTop);
  }

  dom::ExceptionOr<void> CSSPageDescriptors::SetMarginTop(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginTop, value, IsImportant(false));
  }

  // 'margin-right'
  CSSOMString CSSPageDescriptors::MarginRight() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginRight);
  }

  dom::ExceptionOr<void> CSSPageDescriptors::SetMarginRight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginRight, value, IsImportant(false));
  }

  // 'margin-bottom'
  CSSOMString CSSPageDescriptors::MarginBottom() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginBottom);
  }

  dom::ExceptionOr<void> CSSPageDescriptors::SetMarginBottom(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginBottom, value, IsImportant(false));
  }

  // 'margin-left'
  CSSOMString CSSPageDescriptors::MarginLeft() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginLeft);
  }

  dom::ExceptionOr<void> CSSPageDescriptors::SetMarginLeft(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginLeft, value, IsImportant(false));
  }

  // @page 'size'
  CSSOMString CSSPageDescriptors::Size() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Size);
  }

  dom::ExceptionOr<void> CSSPageDescriptors::SetSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Size, value, IsImportant(false));
  }

#pragma endregion
}
