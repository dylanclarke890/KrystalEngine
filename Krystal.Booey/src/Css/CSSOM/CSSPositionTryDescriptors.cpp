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
 * PROFITS;OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Css/CSSOM/CSSPositionTryDescriptors.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSPositionTryRule.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  CSSPositionTryDescriptors::CSSPositionTryDescriptors(css::MutableStyleProperties &propertySet,
                                                       CSSPositionTryRule &parentRule) noexcept
      : PropertySetCSSDescriptors(propertySet, parentRule)
  {
  }

  CSSPositionTryDescriptors::~CSSPositionTryDescriptors() noexcept = default;

  Ref<CSSPositionTryDescriptors> CSSPositionTryDescriptors::Create(css::MutableStyleProperties &propertySet,
                                                                   CSSPositionTryRule &parentRule) noexcept
  {
    return AdoptRef(*new CSSPositionTryDescriptors(propertySet, parentRule));
  }

  css::RuleType CSSPositionTryDescriptors::GetRuleType() const noexcept
  {
    return css::RuleType::PositionTry;
  }

#pragma region Descriptors

  // @position-try 'margin'
  CSSOMString CSSPositionTryDescriptors::Margin() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Margin);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMargin(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Margin, value, css::IsImportant::No);
  }

  // @position-try 'margin-top'
  CSSOMString CSSPositionTryDescriptors::MarginTop() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginTop);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginTop(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginTop, value, css::IsImportant::No);
  }

  // @position-try 'margin-right'
  CSSOMString CSSPositionTryDescriptors::MarginRight() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginRight);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginRight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginRight, value, css::IsImportant::No);
  }

  // @position-try 'margin-bottom'
  CSSOMString CSSPositionTryDescriptors::MarginBottom() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginBottom);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginBottom(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginBottom, value, css::IsImportant::No);
  }

  // @position-try 'margin-left'
  CSSOMString CSSPositionTryDescriptors::MarginLeft() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginLeft);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginLeft(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginLeft, value, css::IsImportant::No);
  }

  // @position-try 'margin-block'
  CSSOMString CSSPositionTryDescriptors::MarginBlock() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginBlock);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginBlock(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginBlock, value, css::IsImportant::No);
  }

  // @position-try 'margin-block-start'
  CSSOMString CSSPositionTryDescriptors::MarginBlockStart() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginBlockStart);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginBlockStart(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginBlockStart, value, css::IsImportant::No);
  }

  // @position-try 'margin-block-end'
  CSSOMString CSSPositionTryDescriptors::MarginBlockEnd() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginBlockEnd);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginBlockEnd(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginBlockEnd, value, css::IsImportant::No);
  }

  // @position-try 'margin-inline'
  CSSOMString CSSPositionTryDescriptors::MarginInline() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginInline);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginInline(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginInline, value, css::IsImportant::No);
  }

  // @position-try 'margin-inline-start'
  CSSOMString CSSPositionTryDescriptors::MarginInlineStart() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginInlineStart);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginInlineStart(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginInlineStart, value, css::IsImportant::No);
  }

  // @position-try 'margin-inline-end'
  CSSOMString CSSPositionTryDescriptors::MarginInlineEnd() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MarginInlineEnd);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMarginInlineEnd(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MarginInlineEnd, value, css::IsImportant::No);
  }

  // @position-try 'inset'
  CSSOMString CSSPositionTryDescriptors::Inset() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Inset);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetInset(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Inset, value, css::IsImportant::No);
  }

  // @position-try 'inset-block'
  CSSOMString CSSPositionTryDescriptors::InsetBlock() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::InsetBlock);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetInsetBlock(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::InsetBlock, value, css::IsImportant::No);
  }

  // @position-try 'inset-block-start'
  CSSOMString CSSPositionTryDescriptors::InsetBlockStart() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::InsetBlockStart);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetInsetBlockStart(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::InsetBlockStart, value, css::IsImportant::No);
  }

  // @position-try 'inset-block-end'
  CSSOMString CSSPositionTryDescriptors::InsetBlockEnd() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::InsetBlockEnd);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetInsetBlockEnd(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::InsetBlockEnd, value, css::IsImportant::No);
  }

  // @position-try 'inset-inline'
  CSSOMString CSSPositionTryDescriptors::InsetInline() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::InsetInline);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetInsetInline(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::InsetInline, value, css::IsImportant::No);
  }

  // @position-try 'inset-inline-start'
  CSSOMString CSSPositionTryDescriptors::InsetInlineStart() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::InsetInlineStart);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetInsetInlineStart(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::InsetInlineStart, value, css::IsImportant::No);
  }

  // @position-try 'inset-inline-end'
  CSSOMString CSSPositionTryDescriptors::InsetInlineEnd() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::InsetInlineEnd);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetInsetInlineEnd(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::InsetInlineEnd, value, css::IsImportant::No);
  }

  // @position-try 'top'
  CSSOMString CSSPositionTryDescriptors::Top() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Top);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetTop(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Top, value, css::IsImportant::No);
  }

  // @position-try 'left'
  CSSOMString CSSPositionTryDescriptors::Left() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Left);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetLeft(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Left, value, css::IsImportant::No);
  }

  // @position-try 'right'
  CSSOMString CSSPositionTryDescriptors::Right() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Right);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetRight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Right, value, css::IsImportant::No);
  }

  // @position-try 'bottom'
  CSSOMString CSSPositionTryDescriptors::Bottom() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Bottom);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetBottom(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Bottom, value, css::IsImportant::No);
  }

  // @position-try 'width'
  CSSOMString CSSPositionTryDescriptors::Width() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Width);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetWidth(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Width, value, css::IsImportant::No);
  }

  // @position-try 'min-width'
  CSSOMString CSSPositionTryDescriptors::MinWidth() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MinWidth);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMinWidth(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MinWidth, value, css::IsImportant::No);
  }

  // @position-try 'max-width'
  CSSOMString CSSPositionTryDescriptors::MaxWidth() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MaxWidth);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMaxWidth(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MaxWidth, value, css::IsImportant::No);
  }

  // @position-try 'height'
  CSSOMString CSSPositionTryDescriptors::Height() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::Height);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetHeight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::Height, value, css::IsImportant::No);
  }

  // @position-try 'min-height'
  CSSOMString CSSPositionTryDescriptors::MinHeight() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MinHeight);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMinHeight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MinHeight, value, css::IsImportant::No);
  }

  // @position-try 'max-height'
  CSSOMString CSSPositionTryDescriptors::MaxHeight() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MaxHeight);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMaxHeight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MaxHeight, value, css::IsImportant::No);
  }

  // @position-try 'block-size'
  CSSOMString CSSPositionTryDescriptors::BlockSize() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::BlockSize);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetBlockSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::BlockSize, value, css::IsImportant::No);
  }

  // @position-try 'min-block-size'
  CSSOMString CSSPositionTryDescriptors::MinBlockSize() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MinBlockSize);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMinBlockSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MinBlockSize, value, css::IsImportant::No);
  }

  // @position-try 'max-block-size'
  CSSOMString CSSPositionTryDescriptors::MaxBlockSize() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MaxBlockSize);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMaxBlockSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MaxBlockSize, value, css::IsImportant::No);
  }

  // @position-try 'inline-size'
  CSSOMString CSSPositionTryDescriptors::InlineSize() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::InlineSize);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetInlineSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::InlineSize, value, css::IsImportant::No);
  }

  // @position-try 'min-inline-size'
  CSSOMString CSSPositionTryDescriptors::MinInlineSize() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MinInlineSize);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMinInlineSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MinInlineSize, value, css::IsImportant::No);
  }

  // @position-try 'max-inline-size'
  CSSOMString CSSPositionTryDescriptors::MaxInlineSize() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::MaxInlineSize);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetMaxInlineSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::MaxInlineSize, value, css::IsImportant::No);
  }

  // @position-try 'place-self'
  CSSOMString CSSPositionTryDescriptors::PlaceSelf() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::PlaceSelf);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetPlaceSelf(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::PlaceSelf, value, css::IsImportant::No);
  }

  // @position-try 'align-self'
  CSSOMString CSSPositionTryDescriptors::AlignSelf() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::AlignSelf);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetAlignSelf(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::AlignSelf, value, css::IsImportant::No);
  }

  // @position-try 'justify-self'
  CSSOMString CSSPositionTryDescriptors::JustifySelf() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::JustifySelf);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetJustifySelf(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::JustifySelf, value, css::IsImportant::No);
  }

  // @position-try 'position-anchor'
  CSSOMString CSSPositionTryDescriptors::PositionAnchor() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::PositionAnchor);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetPositionAnchor(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::PositionAnchor, value, css::IsImportant::No);
  }

  // @position-try 'position-area'
  CSSOMString CSSPositionTryDescriptors::PositionArea() const noexcept
  {
    return GetPropertyValueInternal(css::PropertyId::PositionArea);
  }

  ExceptionOr<void> CSSPositionTryDescriptors::SetPositionArea(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(css::PropertyId::PositionArea, value, css::IsImportant::No);
  }

#pragma endregion

  ExceptionOr<void> CSSPositionTryDescriptors::SetPropertyInternal(css::PropertyId property,
                                                                   const CSSOMString &value,
                                                                   css::IsImportant important) noexcept
  {
    auto result = PropertySetCSSDescriptors::SetPropertyInternal(property, value, important);

    auto invalidateAllRememberedLastSuccessfulPositionOptions = [&]()
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

      if (auto *parentStyleSheetScope = parentStyleSheet->StyleScope())
      {
        // FIXME: only invalidate elements that use this position-try rule in position-try-fallbacks
        parentStyleSheetScope->SetLastSuccessfulPositionOptionIndexMap({});
      }
    };

    invalidateAllRememberedLastSuccessfulPositionOptions();

    return result;
  }
}
