/*
 * Copyright (C) 2022 Igalia S.L.
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

#include "Krystal.Booey/CSS/TypedOM/StylePropertyMapReadOnly.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  /// @brief An element's computed style (`computedStyleMap()`), as a read-only property map.
  /// @see https://drafts.css-houdini.org/css-typed-om/#computed-stylepropertymapreadonly-objects
  class ComputedStylePropertyMapReadOnly final : public StylePropertyMapReadOnly
  {
  private:
    WeakPtr<dom::Element> _element;

  public:
    KRYS_NODISCARD static Ref<ComputedStylePropertyMapReadOnly> Create(dom::Element &element) noexcept;

    ~ComputedStylePropertyMapReadOnly() noexcept override;

    KRYS_NODISCARD Type MapType() const noexcept final
    {
      return Type::Computed;
    }
    /// @brief The number of exposed properties, then of the element's custom properties.
    KRYS_NODISCARD size_t Size() const noexcept final;
    /// @brief The exposed properties, then the custom properties, sorted as the specification says.
    KRYS_NODISCARD List<Entry> Entries() const noexcept final;

  private:
    explicit ComputedStylePropertyMapReadOnly(dom::Element &element) noexcept;

    KRYS_NODISCARD dom::Document *Document() const noexcept final;
    KRYS_NODISCARD RefPtr<Value> PropertyValue(PropertyId property) const noexcept final;
    KRYS_NODISCARD CSSOMString ShorthandPropertySerialisation(PropertyId property) const noexcept final;
    KRYS_NODISCARD RefPtr<Value> CustomPropertyValue(const CSSOMStringAtom &property) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ComputedStylePropertyMapReadOnly)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::StylePropertyMapReadOnly &map) noexcept
  {
    return map.MapType() == krys::boo::css::StylePropertyMapReadOnly::Type::Computed;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
