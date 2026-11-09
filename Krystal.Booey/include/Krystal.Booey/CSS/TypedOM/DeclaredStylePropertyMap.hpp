/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/StylePropertyMap.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleRule;
  class StyleRule;

  /// @brief A style rule's declarations (`styleMap`), as a property map.
  /// @see https://drafts.css-houdini.org/css-typed-om/#declared-stylepropertymap-objects
  class DeclaredStylePropertyMap final : public StylePropertyMap
  {
  private:
    WeakPtr<CSSStyleRule> _ownerRule;

  public:
    KRYS_NODISCARD static Ref<DeclaredStylePropertyMap> Create(CSSStyleRule &ownerRule) noexcept;

    ~DeclaredStylePropertyMap() noexcept override;

    KRYS_NODISCARD Type MapType() const noexcept final
    {
      return Type::Declared;
    }
    KRYS_NODISCARD size_t Size() const noexcept final;
    KRYS_NODISCARD List<Entry> Entries() const noexcept final;
    void Clear() noexcept final;

  private:
    explicit DeclaredStylePropertyMap(CSSStyleRule &ownerRule) noexcept;

    KRYS_NODISCARD StyleRule *GetStyleRule() const noexcept;

    KRYS_NODISCARD dom::Document *Document() const noexcept final;
    KRYS_NODISCARD RefPtr<Value> PropertyValue(PropertyId property) const noexcept final;
    KRYS_NODISCARD CSSOMString ShorthandPropertySerialisation(PropertyId property) const noexcept final;
    KRYS_NODISCARD RefPtr<Value> CustomPropertyValue(const CSSOMStringAtom &property) const noexcept final;
    void RemoveProperty(PropertyId property) noexcept final;
    void RemoveCustomProperty(const CSSOMStringAtom &property) noexcept final;
    bool SetShorthandProperty(PropertyId property, const CSSOMString &value) noexcept final;
    bool SetProperty(PropertyId property, Ref<Value> value) noexcept final;
    bool SetCustomProperty(const CSSOMStringAtom &property, Ref<VariableReferenceValue> value) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::DeclaredStylePropertyMap)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::StylePropertyMapReadOnly &map) noexcept
  {
    return map.MapType() == krys::boo::css::StylePropertyMapReadOnly::Type::Declared;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
