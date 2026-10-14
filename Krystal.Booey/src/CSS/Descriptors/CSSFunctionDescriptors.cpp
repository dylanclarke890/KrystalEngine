#include "Krystal.Booey/CSS/Descriptors/CSSFunctionDescriptors.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSFunctionDeclarations.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"

namespace krys::boo::css
{
  CSSFunctionDescriptors::CSSFunctionDescriptors(MutableStyleProperties &propertySet,
                                                 CSSFunctionDeclarations &parentRule) noexcept
      : PropertySetCSSDescriptors(propertySet, parentRule)
  {
  }

  CSSFunctionDescriptors::~CSSFunctionDescriptors() noexcept = default;

  Ref<CSSFunctionDescriptors> CSSFunctionDescriptors::Create(MutableStyleProperties &propertySet,
                                                             CSSFunctionDeclarations &parentRule) noexcept
  {
    return AdoptRef(*new CSSFunctionDescriptors(propertySet, parentRule));
  }

  RuleType CSSFunctionDescriptors::GetRuleType() const noexcept
  {
    return RuleType::FunctionDeclarations;
  }

#pragma region Descriptors

  // @function 'result'
  CSSOMString CSSFunctionDescriptors::Result() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Result);
  }

  dom::ExceptionOr<void> CSSFunctionDescriptors::SetResult(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Result, value, IsImportant(false));
  }

#pragma endregion
}
