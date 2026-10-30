#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::css
{
  PropertySetCSSDescriptors::PropertySetCSSDescriptors(MutableStyleProperties &propertySet,
                                                       CSSRule &parentRule) noexcept
      : _parentRule(WeakPtr<CSSRule>::WithRef(parentRule)), _propertySet(ShareRef(propertySet))
  {
  }

  PropertySetCSSDescriptors::~PropertySetCSSDescriptors() noexcept = default;

  Ref<MutableStyleProperties> PropertySetCSSDescriptors::ProtectedPropertySet() const noexcept
  {
    return _propertySet;
  }

  uint32 PropertySetCSSDescriptors::Length() const noexcept
  {
    auto exposed = 0u;
    for (auto property : *_propertySet)
    {
      if (IsExposed(property.Id()))
      {
        exposed++;
      }
    }
    return exposed;
  }

  CSSOMString PropertySetCSSDescriptors::Item(uint32 i) const noexcept
  {
    for (auto j = 0u; j <= i && j < _propertySet->PropertyCount(); j++)
    {
      if (!IsExposed(_propertySet->PropertyAt(j).Id()))
      {
        i++;
      }
    }

    if (i >= _propertySet->PropertyCount())
    {
      return {};
    }

    return _propertySet->PropertyAt(i).CssName();
  }

  CSSOMString PropertySetCSSDescriptors::CssText() const noexcept
  {
    return _propertySet->AsText(DefaultSerialisationContext());
  }

  dom::ExceptionOr<void> PropertySetCSSDescriptors::SetCssText(const CSSOMString &text) noexcept
  {
    if (!WillMutate())
    {
      return {};
    }

    bool changed = ProtectedPropertySet()->ParseDeclaration(text, CssParserContext());
    DidMutate(changed ? MutationType::PropertyChanged : MutationType::StyleAttributeChanged);
    return {};
  }

  CSSOMString PropertySetCSSDescriptors::GetPropertyValue(const CSSOMString &propertyName) noexcept
  {
    if (GetStyleDeclarationType() == StyleDeclarationType::Function && IsCustomPropertyName(propertyName))
    {
      return _propertySet->GetCustomPropertyValue(propertyName);
    }

    auto property = PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }
    return GetPropertyValueInternal(property);
  }

  CSSOMString PropertySetCSSDescriptors::GetPropertyPriority(const CSSOMString &propertyName) noexcept
  {
    auto property = PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }
    return _propertySet->PropertyIsImportant(property) ? u8"important" : CSSOMString();
  }

  CSSOMString PropertySetCSSDescriptors::GetPropertyShorthand(const CSSOMString &propertyName) noexcept
  {
    auto property = PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }
    return CSSOMString(_propertySet->GetPropertyShorthand(property));
  }

  bool PropertySetCSSDescriptors::IsPropertyImplicit(const CSSOMString &propertyName) noexcept
  {
    return _propertySet->IsPropertyImplicit(PropertyIdFromName(propertyName));
  }

  dom::ExceptionOr<void> PropertySetCSSDescriptors::SetProperty(const CSSOMString &propertyName,
                                                                const CSSOMString &value,
                                                                const CSSOMString &priority) noexcept
  {
    auto property = PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }

    if (!WillMutate())
    {
      return {};
    }

    bool important = text::IsASCIICaselessEqual(CSSOMStringView(priority), CSSOMStringView(u8"important"));
    if (!important && !priority.empty())
    {
      return {};
    }

    bool changed =
      ProtectedPropertySet()->SetProperty(property, value, CssParserContext(), IsImportant(important));

    // CSS DOM requires raising SyntaxError of parsing failed, but this is too dangerous for compatibility,
    // see <http://bugs.webkit.org/show_bug.cgi?id=7296>.
    DidMutate(changed ? MutationType::PropertyChanged : MutationType::NoChanges);
    return {};
  }

  dom::ExceptionOr<CSSOMString>
    PropertySetCSSDescriptors::RemoveProperty(const CSSOMString &propertyName) noexcept
  {
    auto property = PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return CSSOMString();
    }

    if (!WillMutate())
    {
      return CSSOMString();
    }

    CSSOMString result;
    bool changed = ProtectedPropertySet()->RemoveProperty(property, &result);

    DidMutate(changed ? MutationType::PropertyChanged : MutationType::NoChanges);
    return result;
  }

  CSSOMString PropertySetCSSDescriptors::GetPropertyValueInternal(PropertyId property) const noexcept
  {
    if (!IsExposed(property))
    {
      return {};
    }
    return _propertySet->GetPropertyValue(property);
  }

  dom::ExceptionOr<void> PropertySetCSSDescriptors::SetPropertyInternal(PropertyId property,
                                                                        const CSSOMString &value,
                                                                        IsImportant important) noexcept
  {
    if (!WillMutate())
    {
      return {};
    }

    if (!IsExposed(property))
    {
      return {};
    }

    if (ProtectedPropertySet()->SetProperty(property, value, CssParserContext(), important))
    {
      DidMutate(MutationType::PropertyChanged);
    }
    else
    {
      DidMutate(MutationType::NoChanges);
    }

    return {};
  }

  bool PropertySetCSSDescriptors::IsExposed(PropertyId property) const noexcept
  {
    if (property == PropertyId::Invalid)
    {
      return false;
    }

    auto parserContext = CssParserContext();
    return css::IsExposed(property, &parserContext.PropertySettings);
  }

  bool PropertySetCSSDescriptors::WillMutate() noexcept
  {
    if (GetStyleDeclarationType() == StyleDeclarationType::Function)
    {
      // FIXME: Use <declaration-rule-list> parsing.
      return false;
    }

    auto *parentRule = _parentRule.get();
    krys_debug_assert(parentRule);
    if (!parentRule)
    {
      return false;
    }

    auto *parentStyleSheet = parentRule->ParentStyleSheet();
    krys_debug_assert(parentStyleSheet);
    if (!parentStyleSheet)
    {
      return false;
    }

    std::ignore = parentStyleSheet->WillMutateRules();
    return true;
  }

  void PropertySetCSSDescriptors::DidMutate(MutationType) noexcept
  {
    auto *parentRule = _parentRule.get();
    krys_debug_assert(parentRule);
    if (!parentRule)
    {
      return;
    }

    auto *parentStyleSheet = parentRule->ParentStyleSheet();
    krys_debug_assert(parentStyleSheet);
    if (!parentStyleSheet)
    {
      return;
    }

    // Style sheet mutation needs to be signaled even if the change failed. WillMutate*/DidMutate* must pair.
    parentStyleSheet->DidMutateRuleFromCSSStyleDeclaration();
  }

  CSSStyleSheet *PropertySetCSSDescriptors::ParentStyleSheet() const noexcept
  {
    auto *parentRule = _parentRule.get();
    if (!parentRule)
    {
      return nullptr;
    }
    return parentRule->ParentStyleSheet();
  }

  CSSRule *PropertySetCSSDescriptors::ParentRule() const noexcept
  {
    return _parentRule.get();
  }

  ParserContext PropertySetCSSDescriptors::CssParserContext() const noexcept
  {
    auto *cssStyleSheet = ParentStyleSheet();
    auto context = cssStyleSheet ? cssStyleSheet->Contents().GetParserContext()
                                 : ParserContext(_propertySet->ParserMode());
    context.EnclosingRuleType = GetRuleType();
    return context;
  }

  void PropertySetCSSDescriptors::Reattach(MutableStyleProperties &propertySet) noexcept
  {
    _propertySet = ShareRef(propertySet);
  }
}
