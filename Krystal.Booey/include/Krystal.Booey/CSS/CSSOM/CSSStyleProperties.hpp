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

#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class StyledElement;
}

namespace krys::boo::css
{
  class MutableStyleProperties;
  struct ParserContext;
  class StyleSheetContents;
}

namespace krys::boo
{
  /// @brief A declaration block of CSS properties exposed to scripts: a style rule's `style`, an element's
  /// `style` attribute or a computed style.
  /// @see https://drafts.csswg.org/cssom/#the-cssstyleproperties-interface
  class CSSStyleProperties : public CSSStyleDeclaration
  {
  public:
    KRYS_NODISCARD StyleDeclarationType GetStyleDeclarationType() const noexcept final
    {
      return StyleDeclarationType::Style;
    }

    KRYS_NODISCARD CSSOMString CssFloat() noexcept;

    ExceptionOr<void> SetCssFloat(const CSSOMString &value) noexcept;

    // PropertyId versions of the CSSOM functions to support bindings and editing.
    KRYS_NODISCARD virtual CSSOMString GetPropertyValueInternal(css::PropertyId property) noexcept = 0;

    virtual ExceptionOr<void> SetPropertyInternal(css::PropertyId property, const CSSOMString &value,
                                                  css::IsImportant important) noexcept = 0;

    KRYS_NODISCARD virtual Ref<css::MutableStyleProperties> CopyProperties() const noexcept = 0;

    // Shared implementation for all properties that match
    // https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-camel_cased_attribute.
    KRYS_NODISCARD CSSOMString
      GetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;

    ExceptionOr<void> SetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                const CSSOMString &value) noexcept;

    // Shared implementation for all properties that match
    // https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-dashed_attribute.
    KRYS_NODISCARD CSSOMString
      GetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;

    ExceptionOr<void> SetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute,
                                                            const CSSOMString &value) noexcept;

    /// @brief The property named by a script property name: camel-cased (`backgroundColor`) or
    /// WebKit-cased (`webkitTransform`).
    KRYS_NODISCARD static css::PropertyId
      GetPropertyIdFromJavaScriptPropertyName(const CSSOMStringAtom &propertyName) noexcept;
  };

  /// @brief The CSSOM view of a declaration block. The declaration block owns it.
  class PropertySetCSSStyleProperties : public CSSStyleProperties
  {
  protected:
    enum class MutationType : uint8
    {
      NoChanges,
      StyleAttributeChanged,
      PropertyChanged,
    };

    css::MutableStyleProperties *_propertySet;

  public:
    explicit PropertySetCSSStyleProperties(css::MutableStyleProperties &propertySet) noexcept
        : _propertySet(&propertySet)
    {
    }

    void AddRef() const noexcept override;

    void SubRef() const noexcept override;

    KRYS_NODISCARD uint32 GetRefCount() const noexcept override;

    KRYS_NODISCARD css::StyleSheetContents *ContextStyleSheet() const noexcept;

  protected:
    KRYS_NODISCARD virtual css::ParserContext CssParserContext() const noexcept;

  private:
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept override
    {
      return nullptr;
    }

    KRYS_NODISCARD uint32 Length() const noexcept final;

    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyValue(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyPriority(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyShorthand(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD bool IsPropertyImplicit(const CSSOMString &propertyName) noexcept final;

    ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                  const CSSOMString &priority) noexcept final;

    ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    ExceptionOr<void> SetCssText(const CSSOMString &text) noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyValueInternal(css::PropertyId property) noexcept final;

    ExceptionOr<void> SetPropertyInternal(css::PropertyId property, const CSSOMString &value,
                                          css::IsImportant important) noexcept final;

    KRYS_NODISCARD Ref<css::MutableStyleProperties> CopyProperties() const noexcept final;

    KRYS_NODISCARD bool IsExposed(css::PropertyId property) const noexcept;

    KRYS_NODISCARD virtual bool WillMutate() noexcept
    {
      return true;
    }

    virtual void DidMutate(MutationType) noexcept
    {
    }
  };

  /// @brief The `style` of a style, keyframe or nested declarations rule.
  class StyleRuleCSSStyleProperties final : public PropertySetCSSStyleProperties,
                                            public RefCounted<StyleRuleCSSStyleProperties>
  {
  private:
    css::RuleType _parentRuleType;
    WeakPtr<CSSRule> _parentRule;

    StyleRuleCSSStyleProperties(css::MutableStyleProperties &propertySet, CSSRule &parentRule) noexcept;

  public:
    ~StyleRuleCSSStyleProperties() noexcept override;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }

    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    KRYS_NODISCARD static Ref<StyleRuleCSSStyleProperties> Create(css::MutableStyleProperties &propertySet,
                                                                  CSSRule &parentRule) noexcept;

    void ClearParentRule() noexcept
    {
      _parentRule = nullptr;
    }

    void Reattach(css::MutableStyleProperties &propertySet) noexcept;

  private:
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final;

    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;

    KRYS_NODISCARD bool WillMutate() noexcept final;

    void DidMutate(MutationType type) noexcept final;

    KRYS_NODISCARD css::ParserContext CssParserContext() const noexcept final;
  };

  /// @brief The `style` of an element: the CSSOM view of its inline style. Changes are written back to the
  /// element's `style` attribute.
  class InlineCSSStyleProperties final : public PropertySetCSSStyleProperties
  {
  private:
    WeakPtr<StyledElement> _parentElement;

  public:
    InlineCSSStyleProperties(css::MutableStyleProperties &propertySet, StyledElement &parentElement) noexcept;

    KRYS_NODISCARD Element *ParentElement() const noexcept final;

  private:
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final
    {
      return nullptr;
    }

    KRYS_NODISCARD bool WillMutate() noexcept final
    {
      return true;
    }

    void DidMutate(MutationType type) noexcept final;

    KRYS_NODISCARD css::ParserContext CssParserContext() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSStyleProperties)
  KRYS_NODISCARD static bool IsType(const krys::boo::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.GetStyleDeclarationType() == krys::boo::StyleDeclarationType::Style;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
