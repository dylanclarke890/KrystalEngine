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

#include "Krystal.Web/Css/CSSOM/PropertySetCSSDescriptors.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Web/Css/CSSOM/CSSRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Web/Css/Parser/KeywordLookup.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"

namespace krys::web
{
  PropertySetCSSDescriptors::PropertySetCSSDescriptors(css::MutableStyleProperties &propertySet,
                                                       CSSRule &parentRule) noexcept
      : _parentRule(CreateWeakPtr(&parentRule)), _propertySet(ShareRef(propertySet))
  {
  }

  PropertySetCSSDescriptors::~PropertySetCSSDescriptors() noexcept = default;

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

  CSSOMString PropertySetCSSDescriptors::CssText() const noexcept
  {
    return _propertySet->AsText(css::DefaultSerialisationContext());
  }

  ExceptionOr<void> PropertySetCSSDescriptors::SetCssText(const CSSOMString &text) noexcept
  {
    if (!WillMutate())
    {
      return {};
    }

    bool changed = _propertySet->ParseDeclaration(text, CssParserContext());
    DidMutate(changed ? MutationType::PropertyChanged : MutationType::StyleAttributeChanged);
    return {};
  }

  CSSOMString PropertySetCSSDescriptors::GetPropertyValue(const CSSOMString &propertyName) noexcept
  {
    if (GetStyleDeclarationType() == StyleDeclarationType::Function
        && css::IsCustomPropertyName(propertyName))
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

  CSSOMString PropertySetCSSDescriptors::GetPropertyPriority(const CSSOMString &propertyName) noexcept
  {
    auto property = css::PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return {};
    }
    return _propertySet->PropertyIsImportant(property) ? u8"important" : CSSOMString();
  }

  ExceptionOr<void> PropertySetCSSDescriptors::SetProperty(const CSSOMString &propertyName,
                                                           const CSSOMString &value,
                                                           const CSSOMString &priority) noexcept
  {
    auto property = css::PropertyIdFromName(propertyName);
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

    bool changed =
      _propertySet->SetProperty(property, value, CssParserContext(), css::IsImportant(important));

    // CSS DOM requires raising SyntaxError of parsing failed, but this is too dangerous for compatibility,
    // see <http://bugs.webkit.org/show_bug.cgi?id=7296>.
    DidMutate(changed ? MutationType::PropertyChanged : MutationType::NoChanges);
    return {};
  }

  ExceptionOr<CSSOMString> PropertySetCSSDescriptors::RemoveProperty(const CSSOMString &propertyName) noexcept
  {
    auto property = css::PropertyIdFromName(propertyName);
    if (!IsExposed(property))
    {
      return CSSOMString();
    }

    if (!WillMutate())
    {
      return CSSOMString();
    }

    CSSOMString result;
    bool changed = _propertySet->RemoveProperty(property, &result);

    DidMutate(changed ? MutationType::PropertyChanged : MutationType::NoChanges);
    return result;
  }

  CSSOMString PropertySetCSSDescriptors::GetPropertyValueInternal(css::PropertyId property) const noexcept
  {
    if (!IsExposed(property))
    {
      return {};
    }
    return _propertySet->GetPropertyValue(property);
  }

  ExceptionOr<void> PropertySetCSSDescriptors::SetPropertyInternal(css::PropertyId property,
                                                                   const CSSOMString &value,
                                                                   css::IsImportant important) noexcept
  {
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
    }
    else
    {
      DidMutate(MutationType::NoChanges);
    }

    return {};
  }

  bool PropertySetCSSDescriptors::IsExposed(css::PropertyId property) const noexcept
  {
    if (property == css::PropertyId::Invalid)
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

  css::ParserContext PropertySetCSSDescriptors::CssParserContext() const noexcept
  {
    auto *cssStyleSheet = ParentStyleSheet();
    auto context = cssStyleSheet ? cssStyleSheet->Contents().GetParserContext()
                                 : css::ParserContext(_propertySet->ParserMode());
    context.EnclosingRuleType = GetRuleType();
    return context;
  }

  void PropertySetCSSDescriptors::Reattach(css::MutableStyleProperties &propertySet) noexcept
  {
    _propertySet = ShareRef(propertySet);
  }
}
