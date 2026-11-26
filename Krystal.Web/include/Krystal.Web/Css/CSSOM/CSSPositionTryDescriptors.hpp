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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Css/CSSOM/PropertySetCSSDescriptors.hpp"

namespace krys::web
{
  class CSSPositionTryRule;

  /// @brief The descriptors of an `@position-try` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-anchor-position/#csspositiontrydescriptors
  class CSSPositionTryDescriptors final : public PropertySetCSSDescriptors
  {
  private:
    CSSPositionTryDescriptors(css::MutableStyleProperties &propertySet,
                              CSSPositionTryRule &parentRule) noexcept;

  public:
    ~CSSPositionTryDescriptors() noexcept override;

    KRYS_NODISCARD static Ref<CSSPositionTryDescriptors> Create(css::MutableStyleProperties &propertySet,
                                                                CSSPositionTryRule &parentRule) noexcept;

    KRYS_NODISCARD StyleDeclarationType GetStyleDeclarationType() const noexcept final
    {
      return StyleDeclarationType::PositionTry;
    }

    KRYS_NODISCARD CSSOMString Margin() const noexcept;

    ExceptionOr<void> SetMargin(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginTop() const noexcept;

    ExceptionOr<void> SetMarginTop(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginRight() const noexcept;

    ExceptionOr<void> SetMarginRight(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginBottom() const noexcept;

    ExceptionOr<void> SetMarginBottom(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginLeft() const noexcept;

    ExceptionOr<void> SetMarginLeft(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginBlock() const noexcept;

    ExceptionOr<void> SetMarginBlock(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginBlockStart() const noexcept;

    ExceptionOr<void> SetMarginBlockStart(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginBlockEnd() const noexcept;

    ExceptionOr<void> SetMarginBlockEnd(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginInline() const noexcept;

    ExceptionOr<void> SetMarginInline(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginInlineStart() const noexcept;

    ExceptionOr<void> SetMarginInlineStart(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginInlineEnd() const noexcept;

    ExceptionOr<void> SetMarginInlineEnd(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Inset() const noexcept;

    ExceptionOr<void> SetInset(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetBlock() const noexcept;

    ExceptionOr<void> SetInsetBlock(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetBlockStart() const noexcept;

    ExceptionOr<void> SetInsetBlockStart(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetBlockEnd() const noexcept;

    ExceptionOr<void> SetInsetBlockEnd(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetInline() const noexcept;

    ExceptionOr<void> SetInsetInline(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetInlineStart() const noexcept;

    ExceptionOr<void> SetInsetInlineStart(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetInlineEnd() const noexcept;

    ExceptionOr<void> SetInsetInlineEnd(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Top() const noexcept;

    ExceptionOr<void> SetTop(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Left() const noexcept;

    ExceptionOr<void> SetLeft(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Right() const noexcept;

    ExceptionOr<void> SetRight(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Bottom() const noexcept;

    ExceptionOr<void> SetBottom(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Width() const noexcept;

    ExceptionOr<void> SetWidth(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MinWidth() const noexcept;

    ExceptionOr<void> SetMinWidth(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MaxWidth() const noexcept;

    ExceptionOr<void> SetMaxWidth(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Height() const noexcept;

    ExceptionOr<void> SetHeight(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MinHeight() const noexcept;

    ExceptionOr<void> SetMinHeight(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MaxHeight() const noexcept;

    ExceptionOr<void> SetMaxHeight(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString BlockSize() const noexcept;

    ExceptionOr<void> SetBlockSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MinBlockSize() const noexcept;

    ExceptionOr<void> SetMinBlockSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MaxBlockSize() const noexcept;

    ExceptionOr<void> SetMaxBlockSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InlineSize() const noexcept;

    ExceptionOr<void> SetInlineSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MinInlineSize() const noexcept;

    ExceptionOr<void> SetMinInlineSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MaxInlineSize() const noexcept;

    ExceptionOr<void> SetMaxInlineSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString PlaceSelf() const noexcept;

    ExceptionOr<void> SetPlaceSelf(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString AlignSelf() const noexcept;

    ExceptionOr<void> SetAlignSelf(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString JustifySelf() const noexcept;

    ExceptionOr<void> SetJustifySelf(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString PositionAnchor() const noexcept;

    ExceptionOr<void> SetPositionAnchor(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString PositionArea() const noexcept;

    ExceptionOr<void> SetPositionArea(const CSSOMString &value) noexcept;

  private:
    // Override this to invalidate all remembered last-successful position options
    // when property value changes.
    ExceptionOr<void> SetPropertyInternal(css::PropertyId property, const CSSOMString &value,
                                          css::IsImportant important) noexcept override;

    KRYS_NODISCARD css::RuleType GetRuleType() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::CSSPositionTryDescriptors)
  KRYS_NODISCARD static bool IsType(const krys::web::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.GetStyleDeclarationType() == krys::web::StyleDeclarationType::PositionTry;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
