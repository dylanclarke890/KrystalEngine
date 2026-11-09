/*
 * Copyright (C) 2015-2021 Apple Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  /// @brief The value of a custom property (`--name`): unresolved references, tokens, or a CSS-wide keyword.
  class CustomPropertyValue final : public Value
  {
  public:
    using VariantValue = Variant<Ref<VariableReferenceValue>, Ref<VariableData>, GlobalKeyword>;

  private:
    const CSSOMStringAtom _name;
    const VariantValue _value;
    mutable CSSOMString _cachedCSSText;

    CustomPropertyValue(const CSSOMStringAtom &name, VariantValue &&value) noexcept
        : css::Value(ValueType::CustomProperty), _name(name), _value(krys::move(value))
    {
    }

  public:
    KRYS_NODISCARD static Ref<CustomPropertyValue> CreateEmpty(const CSSOMStringAtom &name) noexcept;

    KRYS_NODISCARD static Ref<CustomPropertyValue>
      CreateUnresolved(const CSSOMStringAtom &name, Ref<VariableReferenceValue> &&value) noexcept;

    KRYS_NODISCARD static Ref<CustomPropertyValue> CreateSyntaxAll(const CSSOMStringAtom &name,
                                                                   Ref<VariableData> &&value) noexcept;

    KRYS_NODISCARD static Ref<CustomPropertyValue> CreateWithGlobalKeyword(const CSSOMStringAtom &name,
                                                                           GlobalKeyword keyword) noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const VariantValue &Value() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD Ref<const VariableData> AsVariableData() const noexcept;

    KRYS_NODISCARD bool IsCurrentColor() const noexcept;

    KRYS_NODISCARD bool IsVariableReference() const noexcept;
    KRYS_NODISCARD bool IsVariableData() const noexcept;
    KRYS_NODISCARD bool IsGlobalKeyword() const noexcept;

    KRYS_NODISCARD Maybe<GlobalKeyword> TryGlobalKeyword() const noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const CustomPropertyValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept;

  private:
    KRYS_NODISCARD const List<Token> &Tokens() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CustomPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCustomPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()