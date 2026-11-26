/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Css/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/Css/CSSOM/StyleAttributeMutationScope.hpp"
#include "Krystal.Booey/Css/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/Css/Properties/Property.hpp"
#include "Krystal.Booey/Css/Properties/PropertyQueries.hpp"
#include "Krystal.Booey/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/Css/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/StyledElement.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD css::PropertyId
      ParseJavaScriptCSSPropertyName(const CSSOMStringAtom &propertyName) noexcept
    {
      static MainThreadNeverDestroyed<HashMap<CSSOMStringAtom, css::PropertyId>> propertyIdCache;

      if (propertyName == null)
      {
        return css::PropertyId::Invalid;
      }

      auto name = propertyName.View();
      auto length = name.size();
      if (!length)
      {
        return css::PropertyId::Invalid;
      }

      auto &cache = static_cast<HashMap<CSSOMStringAtom, css::PropertyId> &>(propertyIdCache);
      if (auto it = cache.find(propertyName); it != cache.end())
      {
        return it->second;
      }

      Array<char8, css::MaxPropertyNameLength> buffer;
      auto outputLength = 0uz;
      auto i = 0uz;

      if (text::IsASCIIUpper(name[0]))
      {
        return css::PropertyId::Invalid;
      }

      buffer[outputLength++] = text::ToASCIILower(name[i++]);

      auto stringEnd = buffer.size() - 1;
      auto bufferSizeLeft = stringEnd - outputLength;
      auto propertySizeLeft = length - i;
      if (propertySizeLeft > bufferSizeLeft)
      {
        return css::PropertyId::Invalid;
      }

      for (; i < length; ++i)
      {
        auto c = name[i];
        if (!c || !text::IsASCII(c))
        {
          // illegal character
          return css::PropertyId::Invalid;
        }
        if (text::IsASCIIUpper(c))
        {
          bufferSizeLeft = stringEnd - outputLength;
          propertySizeLeft = length - i + 1;
          if (propertySizeLeft > bufferSizeLeft)
          {
            return css::PropertyId::Invalid;
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

      auto id = css::FindProperty(CSSOMStringView(buffer.data(), outputLength));
      // FIXME: Why aren't we memoizing CSS property names we fail to find?
      if (id != css::PropertyId::Invalid)
      {
        cache.emplace(propertyName, id);
      }
      return id;
    }

    enum class PropertyLookupMode : uint8
    {
      ConvertUsingNoDashPrefix,
      NoConversion,
    };

    template <PropertyLookupMode Mode>
    KRYS_NODISCARD css::PropertyId
      LookupCSSPropertyFromIDLAttribute(const CSSOMStringAtom &attribute) noexcept
    {
      static MainThreadNeverDestroyed<HashMap<CSSOMStringAtom, css::PropertyId>> idCache;

      auto &cache = static_cast<HashMap<CSSOMStringAtom, css::PropertyId> &>(idCache);
      if (auto it = cache.find(attribute); it != cache.end())
      {
        return it->second;
      }

      CSSOMString output;
      if constexpr (Mode == PropertyLookupMode::ConvertUsingNoDashPrefix)
      {
        // Conversion is implementing the "IDL attribute to CSS property algorithm"
        // from https://drafts.csswg.org/cssom/#idl-attribute-to-css-property.
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

      auto id = css::FindProperty(output);
      krys_debug_assert(id != css::PropertyId::Invalid);
      cache.emplace(attribute, id);
      return id;
    }
  }

#pragma region CSSStyleProperties

  css::PropertyId
    CSSStyleProperties::GetPropertyIdFromJavaScriptPropertyName(const CSSOMStringAtom &propertyName) noexcept
  {
    auto property = ParseJavaScriptCSSPropertyName(propertyName);
    return property != css::PropertyId::Invalid && css::IsExposed(property, nullptr)
             ? property
             : css::PropertyId::Invalid;
  }

  CSSOMString
    CSSStyleProperties::GetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept
  {
    auto property =
      LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::ConvertUsingNoDashPrefix>(attribute);
    return GetPropertyValueInternal(property);
  }

  ExceptionOr<void>
    CSSStyleProperties::SetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                  const CSSOMString &value) noexcept
  {
    auto property =
      LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::ConvertUsingNoDashPrefix>(attribute);
    return SetPropertyInternal(property, value, css::IsImportant::No);
  }

  CSSOMString
    CSSStyleProperties::GetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute) noexcept
  {
    auto property = LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::NoConversion>(attribute);
    return GetPropertyValueInternal(property);
  }

  ExceptionOr<void>
    CSSStyleProperties::SetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute,
                                                              const CSSOMString &value) noexcept
  {
    auto property = LookupCSSPropertyFromIDLAttribute<PropertyLookupMode::NoConversion>(attribute);
    return SetPropertyInternal(property, value, css::IsImportant::No);
  }

  CSSOMString CSSStyleProperties::CssFloat() noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Float);
  }

  ExceptionOr<void> CSSStyleProperties::SetCssFloat(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Float, value, css::IsImportant::No);
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
    for (auto j = 0u; j <= i && j < _propertySet->size(); j++)
    {
      if (!IsExposed(_propertySet->PropertyAt(j).Id()))
      {
        i++;
      }
    }

    if (i >= _propertySet->size())
    {
      return {};
    }

    return _propertySet->PropertyAt(i).CssName();
  }

  CSSOMString PropertySetCSSStyleProperties::CssText() const noexcept
  {
    return _propertySet->AsText(css::DefaultSerialisationContext());
  }

  ExceptionOr<void> PropertySetCSSStyleProperties::SetCssText(const CSSOMString &text) noexcept
  {
    html::CEReactionsScope ceReactions;
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
    if (css::IsCustomPropertyName(propertyName))
    {
      return _propertySet->GetCustomPropertyValue(propertyName);
    }

    auto property = css::PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }
    return GetPropertyValueInternal(property);
  }

  CSSOMString PropertySetCSSStyleProperties::GetPropertyPriority(const CSSOMString &propertyName) noexcept
  {
    if (css::IsCustomPropertyName(propertyName))
    {
      return _propertySet->CustomPropertyIsImportant(propertyName) ? u8"important" : CSSOMString();
    }

    auto property = css::PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }
    return _propertySet->PropertyIsImportant(property) ? u8"important" : CSSOMString();
  }

  ExceptionOr<void> PropertySetCSSStyleProperties::SetProperty(const CSSOMString &propertyName,
                                                               const CSSOMString &value,
                                                               const CSSOMString &priority) noexcept
  {
    html::CEReactionsScope ceReactions;
    StyleAttributeMutationScope mutationScope {ParentElement()};

    auto property = css::PropertyIdFromName(propertyName);
    if (css::IsCustomPropertyName(propertyName))
    {
      property = css::PropertyId::Custom;
    }

    if (!IsExposed(property))
    {
      return {};
    }

    if (!WillMutate())
    {
      return {};
    }

    bool important = text::IsASCIICaselessEqual(CSSOMStringView(priority), u8"important");
    if (!important && !priority.empty())
    {
      return {};
    }

    bool changed;
    if (property == css::PropertyId::Custom) KRYS_UNLIKELY
    {
      changed =
        _propertySet->SetCustomProperty(propertyName, value, CssParserContext(), css::IsImportant(important));
    }
    else
    {
      changed = _propertySet->SetProperty(property, value, CssParserContext(), css::IsImportant(important));
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

  ExceptionOr<CSSOMString>
    PropertySetCSSStyleProperties::RemoveProperty(const CSSOMString &propertyName) noexcept
  {
    html::CEReactionsScope ceReactions;
    StyleAttributeMutationScope mutationScope {ParentElement()};
    auto property = css::PropertyIdFromName(propertyName);
    if (css::IsCustomPropertyName(propertyName))
    {
      property = css::PropertyId::Custom;
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
    bool changed = property != css::PropertyId::Custom
                     ? _propertySet->RemoveProperty(property, &result)
                     : _propertySet->RemoveCustomProperty(propertyName, &result);

    DidMutate(changed ? MutationType::PropertyChanged : MutationType::NoChanges);

    if (changed)
    {
      mutationScope.EnqueueMutationRecord();
    }
    return result;
  }

  CSSOMString PropertySetCSSStyleProperties::GetPropertyValueInternal(css::PropertyId property) noexcept
  {
    if (!IsExposed(property))
    {
      return {};
    }
    return _propertySet->GetPropertyValue(property);
  }

  ExceptionOr<void> PropertySetCSSStyleProperties::SetPropertyInternal(css::PropertyId property,
                                                                       const CSSOMString &value,
                                                                       css::IsImportant important) noexcept
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

  bool PropertySetCSSStyleProperties::IsExposed(css::PropertyId property) const noexcept
  {
    if (property == css::PropertyId::Invalid)
    {
      return false;
    }

    auto parserContext = CssParserContext();
    bool parsingDescriptor =
      parserContext.EnclosingRuleType && *parserContext.EnclosingRuleType != css::RuleType::Style;
    return css::IsExposed(property, &parserContext.PropertySettings)
           && (!css::IsDescriptorOnly(property) || parsingDescriptor);
  }

  css::StyleSheetContents *PropertySetCSSStyleProperties::ContextStyleSheet() const noexcept
  {
    auto *cssStyleSheet = ParentStyleSheet();
    return cssStyleSheet ? &cssStyleSheet->Contents() : nullptr;
  }

  css::ParserContext PropertySetCSSStyleProperties::CssParserContext() const noexcept
  {
    return css::ParserContext(_propertySet->ParserMode());
  }

  Ref<css::MutableStyleProperties> PropertySetCSSStyleProperties::CopyProperties() const noexcept
  {
    return _propertySet->MutableCopy();
  }

#pragma endregion

#pragma region StyleRuleCSSStyleProperties

  StyleRuleCSSStyleProperties::StyleRuleCSSStyleProperties(css::MutableStyleProperties &propertySet,
                                                           CSSRule &parentRule) noexcept
      : PropertySetCSSStyleProperties(propertySet), _parentRuleType(parentRule.StyleRuleType()),
        _parentRule(CreateWeakPtr(&parentRule))
  {
    _propertySet->AddRef();
  }

  StyleRuleCSSStyleProperties::~StyleRuleCSSStyleProperties() noexcept
  {
    _propertySet->SubRef();
  }

  Ref<StyleRuleCSSStyleProperties>
    StyleRuleCSSStyleProperties::Create(css::MutableStyleProperties &propertySet,
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

  css::ParserContext StyleRuleCSSStyleProperties::CssParserContext() const noexcept
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

  void StyleRuleCSSStyleProperties::Reattach(css::MutableStyleProperties &propertySet) noexcept
  {
    _propertySet->SubRef();
    _propertySet = &propertySet;
    _propertySet->AddRef();
  }

#pragma endregion

#pragma region InlineCSSStyleProperties

  InlineCSSStyleProperties::InlineCSSStyleProperties(css::MutableStyleProperties &propertySet,
                                                     StyledElement &parentElement) noexcept
      : PropertySetCSSStyleProperties(propertySet), _parentElement(CreateWeakPtr(&parentElement))
  {
  }

  Element *InlineCSSStyleProperties::ParentElement() const noexcept
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

  css::ParserContext InlineCSSStyleProperties::CssParserContext() const noexcept
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
