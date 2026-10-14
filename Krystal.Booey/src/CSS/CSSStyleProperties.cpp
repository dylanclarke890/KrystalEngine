#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/StyleAttributeMutationScope.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css
{
  namespace
  {
    enum class PropertyNamePrefix : uint8
    {
      None,
      WebKit
    };

    KRYS_NODISCARD bool MatchesCSSPropertyNamePrefix(CSSOMStringView propertyName,
                                                     CSSOMStringView prefix) noexcept
    {
      krys_debug_assert(text::ToASCIILower(propertyName[0]) == prefix[0]);
      constexpr auto offset = 1uz;

      // The prefix within the property name must be followed by a capital letter.
      // Other characters in the prefix within the property name must be lowercase.
      if (propertyName.size() < prefix.size() + 1)
      {
        return false;
      }

      for (auto i = offset; i < prefix.size(); ++i)
      {
        if (propertyName[i] != prefix[i])
        {
          return false;
        }
      }

      if (!text::IsASCIIUpper(propertyName[prefix.size()]))
      {
        return false;
      }

      return true;
    }

    KRYS_NODISCARD PropertyNamePrefix GetPropertyNamePrefix(CSSOMStringView propertyName) noexcept
    {
      krys_debug_assert(!propertyName.empty());

      // First character of the prefix within the property name may be upper or lowercase.
      if (text::ToASCIILower(propertyName[0]) == u8'w'
          && MatchesCSSPropertyNamePrefix(propertyName, u8"webkit"))
      {
        return PropertyNamePrefix::WebKit;
      }

      return PropertyNamePrefix::None;
    }

    KRYS_NODISCARD PropertyId ParseJavaScriptCSSPropertyName(const CSSOMStringAtom &propertyName) noexcept
    {
      static MainThreadNeverDestroyed<HashMap<CSSOMStringAtom, PropertyId>> propertyIdCache;

      if (propertyName == CSSOMStringAtom::Null())
      {
        return PropertyId::Invalid;
      }

      auto name = propertyName.View();
      auto length = name.size();
      if (!length)
      {
        return PropertyId::Invalid;
      }

      auto &cache = static_cast<HashMap<CSSOMStringAtom, PropertyId> &>(propertyIdCache);
      if (auto it = cache.find(propertyName); it != cache.end())
      {
        return it->second;
      }

      Array<char8, MaxPropertyNameLength> buffer;
      auto outputLength = 0uz;
      auto i = 0uz;

      switch (GetPropertyNamePrefix(name))
      {
        case PropertyNamePrefix::None:
        {
          if (text::IsASCIIUpper(name[0]))
          {
            return PropertyId::Invalid;
          }
          break;
        }
        case PropertyNamePrefix::WebKit:
        {
          constexpr CSSOMStringView webkitPrefix = u8"-webkit-";
          std::copy(webkitPrefix.begin(), webkitPrefix.end(), buffer.begin());
          outputLength = webkitPrefix.size();
          i += 6;
          break;
        }
      }

      buffer[outputLength++] = text::ToASCIILower(name[i++]);

      auto stringEnd = buffer.size() - 1;
      auto bufferSizeLeft = stringEnd - outputLength;
      auto propertySizeLeft = length - i;
      if (propertySizeLeft > bufferSizeLeft)
      {
        return PropertyId::Invalid;
      }

      for (; i < length; ++i)
      {
        auto c = name[i];
        if (!c || !text::IsASCII(c))
        {
          // illegal character
          return PropertyId::Invalid;
        }
        if (text::IsASCIIUpper(c))
        {
          bufferSizeLeft = stringEnd - outputLength;
          propertySizeLeft = length - i + 1;
          if (propertySizeLeft > bufferSizeLeft)
          {
            return PropertyId::Invalid;
          }
          buffer[outputLength++] = u8'-';
          buffer[outputLength++] = text::ToASCIILowerUnchecked(c);
        }
        else
        {
          buffer[outputLength++] = c;
        }
        krys_debug_assert(outputLength < buffer.size());
      }

      auto id = FindProperty(CSSOMStringView(buffer.data(), outputLength));
      // FIXME: Why aren't we memoizing CSS property names we fail to find?
      if (id != PropertyId::Invalid)
      {
        cache.emplace(propertyName, id);
      }
      return id;
    }

    enum class PropertyLookupMode : uint8
    {
      ConvertUsingDashPrefix,
      ConvertUsingNoDashPrefix,
      NoConversion
    };

    template <PropertyLookupMode Mode>
    KRYS_NODISCARD PropertyId LookupCSSPropertyFromIDLAttribute(const CSSOMStringAtom &attribute) noexcept
    {
      static MainThreadNeverDestroyed<HashMap<CSSOMStringAtom, PropertyId>> idCache;

      auto &cache = static_cast<HashMap<CSSOMStringAtom, PropertyId> &>(idCache);
      if (auto it = cache.find(attribute); it != cache.end())
      {
        return it->second;
      }

      CSSOMString output;
      if constexpr (Mode == PropertyLookupMode::ConvertUsingDashPrefix
                    || Mode == PropertyLookupMode::ConvertUsingNoDashPrefix)
      {
        // Conversion is implementing the "IDL attribute to CSS property algorithm"
        // from https://drafts.csswg.org/cssom/#idl-attribute-to-css-property.
        if constexpr (Mode == PropertyLookupMode::ConvertUsingDashPrefix)
        {
          output.push_back(u8'-');
        }

        for (auto c : attribute.View())
        {
          krys_debug_assert(text::IsASCII(c));
          if (text::IsASCIIUpper(c))
          {
            output.push_back(u8'-');
            output.push_back(text::ToASCIILowerUnchecked(c));
          }
          else
          {
            output.push_back(c);
          }
        }
      }
      else
      {
        for (auto c : attribute.View())
        {
          krys_debug_assert(c == u8'-' || text::IsASCIILower(c));
          output.push_back(c);
        }
      }

      auto id = FindProperty(output);
      krys_debug_assert(id != PropertyId::Invalid);
      cache.emplace(attribute, id);
      return id;
    }
  }

#pragma region CSSStyleProperties

  PropertyId
    CSSStyleProperties::GetPropertyIdFromJavaScriptPropertyName(const CSSOMStringAtom &propertyName) noexcept
  {
    // FIXME: This exposes properties disabled by settings. Pass the result of Settings instead of null?
    auto property = ParseJavaScriptCSSPropertyName(propertyName);
    return property != PropertyId::Invalid && css::IsExposed(property, nullptr) ? property
                                                                                : PropertyId::Invalid;
  }

  CSSOMString
    CSSStyleProperties::GetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept
  {
    auto property =
      LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::ConvertUsingNoDashPrefix>(attribute);
    return GetPropertyValueInternal(property);
  }

  dom::ExceptionOr<void>
    CSSStyleProperties::SetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                  const CSSOMString &value) noexcept
  {
    auto property =
      LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::ConvertUsingNoDashPrefix>(attribute);
    return SetPropertyInternal(property, value, IsImportant(false));
  }

  CSSOMString
    CSSStyleProperties::GetPropertyValueForWebKitCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept
  {
    auto property = LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::ConvertUsingDashPrefix>(attribute);
    return GetPropertyValueInternal(property);
  }

  dom::ExceptionOr<void>
    CSSStyleProperties::SetPropertyValueForWebKitCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                   const CSSOMString &value) noexcept
  {
    auto property = LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::ConvertUsingDashPrefix>(attribute);
    return SetPropertyInternal(property, value, IsImportant(false));
  }

  CSSOMString
    CSSStyleProperties::GetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute) noexcept
  {
    auto property = LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::NoConversion>(attribute);
    return GetPropertyValueInternal(property);
  }

  dom::ExceptionOr<void>
    CSSStyleProperties::SetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute,
                                                              const CSSOMString &value) noexcept
  {
    auto property = LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::NoConversion>(attribute);
    return SetPropertyInternal(property, value, IsImportant(false));
  }

  CSSOMString CSSStyleProperties::CssFloat() noexcept
  {
    return GetPropertyValueInternal(PropertyId::Float);
  }

  dom::ExceptionOr<void> CSSStyleProperties::SetCssFloat(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Float, value, IsImportant(false));
  }

#pragma endregion

#pragma region PropertySetCSSStyleProperties

  void PropertySetCSSStyleProperties::AddRef() const noexcept
  {
    _propertySet->AddRef();
  }

  void PropertySetCSSStyleProperties::SubRef() const noexcept
  {
    _propertySet->SubRef();
  }

  uint32 PropertySetCSSStyleProperties::GetRefCount() const noexcept
  {
    return _propertySet->GetRefCount();
  }

  uint32 PropertySetCSSStyleProperties::Length() const noexcept
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

  CSSOMString PropertySetCSSStyleProperties::Item(uint32 i) const noexcept
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

  CSSOMString PropertySetCSSStyleProperties::CssText() const noexcept
  {
    return _propertySet->AsText(DefaultSerialisationContext());
  }

  dom::ExceptionOr<void> PropertySetCSSStyleProperties::SetCssText(const CSSOMString &text) noexcept
  {
    StyleAttributeMutationScope mutationScope {ParentElement()};
    if (!WillMutate())
    {
      return {};
    }

    bool changed = _propertySet->ParseDeclaration(text, CssParserContext());
    DidMutate(changed ? MutationType::PropertyChanged : MutationType::StyleAttributeChanged);

    mutationScope.EnqueueMutationRecord();
    return {};
  }

  CSSOMString PropertySetCSSStyleProperties::GetPropertyValue(const CSSOMString &propertyName) noexcept
  {
    if (IsCustomPropertyName(propertyName))
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

  CSSOMString PropertySetCSSStyleProperties::GetPropertyPriority(const CSSOMString &propertyName) noexcept
  {
    if (IsCustomPropertyName(propertyName))
    {
      return _propertySet->CustomPropertyIsImportant(propertyName) ? u8"important" : CSSOMString();
    }

    auto property = PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }
    return _propertySet->PropertyIsImportant(property) ? u8"important" : CSSOMString();
  }

  CSSOMString PropertySetCSSStyleProperties::GetPropertyShorthand(const CSSOMString &propertyName) noexcept
  {
    auto property = PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }
    return CSSOMString(_propertySet->GetPropertyShorthand(property));
  }

  bool PropertySetCSSStyleProperties::IsPropertyImplicit(const CSSOMString &propertyName) noexcept
  {
    return _propertySet->IsPropertyImplicit(PropertyIdFromName(propertyName));
  }

  dom::ExceptionOr<void> PropertySetCSSStyleProperties::SetProperty(const CSSOMString &propertyName,
                                                                    const CSSOMString &value,
                                                                    const CSSOMString &priority) noexcept
  {
    StyleAttributeMutationScope mutationScope {ParentElement()};

    auto property = PropertyIdFromName(propertyName);
    if (IsCustomPropertyName(propertyName))
    {
      property = PropertyId::Custom;
    }

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

    bool changed;
    if (property == PropertyId::Custom) KRYS_UNLIKELY
    {
      changed =
        _propertySet->SetCustomProperty(propertyName, value, CssParserContext(), IsImportant(important));
    }
    else
    {
      changed = _propertySet->SetProperty(property, value, CssParserContext(), IsImportant(important));
    }

    DidMutate(changed ? MutationType::PropertyChanged : MutationType::NoChanges);

    if (changed)
    {
      // CSS DOM requires raising SyntaxError of parsing failed, but this is too dangerous for compatibility,
      // see <http://bugs.webkit.org/show_bug.cgi?id=7296>.
      mutationScope.EnqueueMutationRecord();
    }
    return {};
  }

  dom::ExceptionOr<CSSOMString>
    PropertySetCSSStyleProperties::RemoveProperty(const CSSOMString &propertyName) noexcept
  {
    StyleAttributeMutationScope mutationScope {ParentElement()};
    auto property = PropertyIdFromName(propertyName);
    if (IsCustomPropertyName(propertyName))
    {
      property = PropertyId::Custom;
    }

    if (!IsExposed(property))
    {
      return CSSOMString();
    }

    if (!WillMutate())
    {
      return CSSOMString();
    }

    CSSOMString result;
    bool changed = property != PropertyId::Custom ? _propertySet->RemoveProperty(property, &result)
                                                  : _propertySet->RemoveCustomProperty(propertyName, &result);

    DidMutate(changed ? MutationType::PropertyChanged : MutationType::NoChanges);

    if (changed)
    {
      mutationScope.EnqueueMutationRecord();
    }
    return result;
  }

  CSSOMString PropertySetCSSStyleProperties::GetPropertyValueInternal(PropertyId property) noexcept
  {
    if (!IsExposed(property))
    {
      return {};
    }
    return _propertySet->GetPropertyValue(property);
  }

  dom::ExceptionOr<void> PropertySetCSSStyleProperties::SetPropertyInternal(PropertyId property,
                                                                            const CSSOMString &value,
                                                                            IsImportant important) noexcept
  {
    StyleAttributeMutationScope mutationScope {ParentElement()};
    if (!WillMutate())
    {
      return {};
    }

    if (!IsExposed(property))
    {
      return {};
    }

    if (_propertySet->SetProperty(property, value, CssParserContext(), important))
    {
      DidMutate(MutationType::PropertyChanged);
      mutationScope.EnqueueMutationRecord();
    }
    else
    {
      DidMutate(MutationType::NoChanges);
    }

    return {};
  }

  bool PropertySetCSSStyleProperties::IsExposed(PropertyId property) const noexcept
  {
    if (property == PropertyId::Invalid)
    {
      return false;
    }

    auto parserContext = CssParserContext();
    bool parsingDescriptor =
      parserContext.EnclosingRuleType && *parserContext.EnclosingRuleType != RuleType::Style;
    return css::IsExposed(property, &parserContext.PropertySettings)
           && (!Property::IsDescriptorOnly(property) || parsingDescriptor);
  }

  StyleSheetContents *PropertySetCSSStyleProperties::ContextStyleSheet() const noexcept
  {
    auto *cssStyleSheet = ParentStyleSheet();
    return cssStyleSheet ? &cssStyleSheet->Contents() : nullptr;
  }

  ParserContext PropertySetCSSStyleProperties::CssParserContext() const noexcept
  {
    return ParserContext(_propertySet->ParserMode());
  }

  Ref<MutableStyleProperties> PropertySetCSSStyleProperties::CopyProperties() const noexcept
  {
    return _propertySet->MutableCopy();
  }

#pragma endregion

#pragma region StyleRuleCSSStyleProperties

  StyleRuleCSSStyleProperties::StyleRuleCSSStyleProperties(MutableStyleProperties &propertySet,
                                                           CSSRule &parentRule) noexcept
      : PropertySetCSSStyleProperties(propertySet), _parentRuleType(parentRule.StyleRuleType()),
        _parentRule(WeakPtr<CSSRule>::WithRef(parentRule))
  {
    _propertySet->AddRef();
  }

  StyleRuleCSSStyleProperties::~StyleRuleCSSStyleProperties() noexcept
  {
    _propertySet->SubRef();
  }

  Ref<StyleRuleCSSStyleProperties> StyleRuleCSSStyleProperties::Create(MutableStyleProperties &propertySet,
                                                                       CSSRule &parentRule) noexcept
  {
    return AdoptRef(*new StyleRuleCSSStyleProperties(propertySet, parentRule));
  }

  bool StyleRuleCSSStyleProperties::WillMutate() noexcept
  {
    if (!_parentRule || !_parentRule->ParentStyleSheet())
    {
      return false;
    }
    std::ignore = _parentRule->ParentStyleSheet()->WillMutateRules();
    return true;
  }

  void StyleRuleCSSStyleProperties::DidMutate(MutationType) noexcept
  {
    krys_debug_assert(_parentRule);
    krys_debug_assert(_parentRule->ParentStyleSheet());

    // Style sheet mutation needs to be signaled even if the change failed. WillMutate*/DidMutate* must pair.
    _parentRule->ParentStyleSheet()->DidMutateRuleFromCSSStyleDeclaration();
  }

  CSSStyleSheet *StyleRuleCSSStyleProperties::ParentStyleSheet() const noexcept
  {
    return _parentRule ? _parentRule->ParentStyleSheet() : nullptr;
  }

  CSSRule *StyleRuleCSSStyleProperties::ParentRule() const noexcept
  {
    return _parentRule.get();
  }

  ParserContext StyleRuleCSSStyleProperties::CssParserContext() const noexcept
  {
    auto *styleSheet = ContextStyleSheet();
    if (!styleSheet)
    {
      return PropertySetCSSStyleProperties::CssParserContext();
    }

    auto context = styleSheet->GetParserContext();
    context.EnclosingRuleType = _parentRuleType;
    return context;
  }

  void StyleRuleCSSStyleProperties::Reattach(MutableStyleProperties &propertySet) noexcept
  {
    _propertySet->SubRef();
    _propertySet = &propertySet;
    _propertySet->AddRef();
  }

#pragma endregion

#pragma region InlineCSSStyleProperties

  InlineCSSStyleProperties::InlineCSSStyleProperties(MutableStyleProperties &propertySet,
                                                     dom::StyledElement &parentElement) noexcept
      : PropertySetCSSStyleProperties(propertySet), _parentElement(CreateWeakPtr(&parentElement))
  {
  }

  dom::Element *InlineCSSStyleProperties::ParentElement() const noexcept
  {
    return _parentElement.get();
  }

  void InlineCSSStyleProperties::DidMutate(MutationType type) noexcept
  {
    if (type == MutationType::NoChanges)
    {
      return;
    }

    if (type == MutationType::StyleAttributeChanged && _parentElement)
    {
      _parentElement->DirtyStyleAttribute();
      return;
    }

    if (!_parentElement)
    {
      return;
    }

    _parentElement->InvalidateStyleAttribute();
  }

  ParserContext InlineCSSStyleProperties::CssParserContext() const noexcept
  {
    if (!_parentElement)
    {
      return PropertySetCSSStyleProperties::CssParserContext();
    }

    auto context = _parentElement->NodeDocument().CssParserContext();
    context.Mode = _propertySet->ParserMode();
    return context;
  }

#pragma endregion
}
