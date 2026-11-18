/*
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
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

namespace krys::boo
{
  class StyledElement;

  /// @brief An element's inline style (`attributeStyleMap`), as a property map.
  /// @see https://drafts.css-houdini.org/css-typed-om/#inline-stylepropertymap-objects
  class InlineStylePropertyMap final : public StylePropertyMap
  {
  private:
    WeakPtr<StyledElement> _element;

  public:
    KRYS_NODISCARD static Ref<InlineStylePropertyMap> Create(StyledElement &element) noexcept;

    ~InlineStylePropertyMap() noexcept override;

    KRYS_NODISCARD Type MapType() const noexcept final
    {
      return Type::Inline;
    }

    KRYS_NODISCARD size_t Size() const noexcept final;

    KRYS_NODISCARD List<Entry> Entries() const noexcept final;

    void Clear() noexcept final;

  private:
    explicit InlineStylePropertyMap(StyledElement &element) noexcept;

    KRYS_NODISCARD boo::Document *Document() const noexcept final;

    KRYS_NODISCARD RefPtr<css::Value> PropertyValue(css::PropertyId property) const noexcept final;

    KRYS_NODISCARD CSSOMString ShorthandPropertySerialisation(css::PropertyId property) const noexcept final;

    KRYS_NODISCARD RefPtr<css::Value>
      CustomPropertyValue(const CSSOMStringAtom &property) const noexcept final;

    void RemoveProperty(css::PropertyId property) noexcept final;

    void RemoveCustomProperty(const CSSOMStringAtom &property) noexcept final;

    bool SetShorthandProperty(css::PropertyId property, const CSSOMString &value) noexcept final;

    bool SetProperty(css::PropertyId property, Ref<css::Value> value) noexcept final;

    bool SetCustomProperty(const CSSOMStringAtom &property,
                           Ref<css::VariableReferenceValue> value) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::InlineStylePropertyMap)
  KRYS_NODISCARD static bool IsType(const krys::boo::StylePropertyMapReadOnly &map) noexcept
  {
    return map.MapType() == krys::boo::StylePropertyMapReadOnly::Type::Inline;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
