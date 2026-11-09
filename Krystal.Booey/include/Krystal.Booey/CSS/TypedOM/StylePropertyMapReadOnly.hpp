/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  /// @brief A read-only map of properties to their values, reified.
  /// @see https://drafts.css-houdini.org/css-typed-om/#stylepropertymapreadonly
  class StylePropertyMapReadOnly : public RefCounted<StylePropertyMapReadOnly>
  {
    KRYS_NON_COPY_MOVABLE(StylePropertyMapReadOnly);

  public:
    /// @brief A property and its values.
    struct Entry
    {
      CSSOMString Property;
      List<Ref<CSSStyleValue>> Values;
    };

    enum class Type : uint8
    {
      Computed,
      Declared,
      Inline
    };

    virtual ~StylePropertyMapReadOnly() noexcept;

    KRYS_NODISCARD virtual Type MapType() const noexcept = 0;

    /// @brief The first value of @p property, or null if it has none.
    /// @return A `TypeError` if @p property isn't a property.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymapreadonly-get
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<CSSStyleValue>> Get(CSSOMStringView property) const noexcept;
    /// @brief The values of @p property.
    /// @return A `TypeError` if @p property isn't a property.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymapreadonly-getall
    KRYS_NODISCARD dom::ExceptionOr<List<Ref<CSSStyleValue>>> GetAll(CSSOMStringView property) const noexcept;
    /// @return A `TypeError` if @p property isn't a property.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymapreadonly-has
    KRYS_NODISCARD dom::ExceptionOr<bool> Has(CSSOMStringView property) const noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymapreadonly-size
    KRYS_NODISCARD virtual size_t Size() const noexcept = 0;
    /// @brief The properties and their values, as the map iterates over them.
    /// @see https://drafts.css-houdini.org/css-typed-om/#the-stylepropertymap
    KRYS_NODISCARD virtual List<Entry> Entries() const noexcept = 0;

    /// @brief @p value reified for @p property, or null if there's no value or it can't be.
    KRYS_NODISCARD static RefPtr<CSSStyleValue> ReifyValue(dom::Document &document, RefPtr<Value> value,
                                                           Maybe<PropertyId> property) noexcept;
    /// @brief @p value reified for @p property, its items each reified if it's a list-valued property's list.
    KRYS_NODISCARD static List<Ref<CSSStyleValue>>
      ReifyValueToList(dom::Document &document, RefPtr<Value> value, Maybe<PropertyId> property) noexcept;

    KRYS_NODISCARD virtual bool IsStylePropertyMap() const noexcept
    {
      return false;
    }

  protected:
    StylePropertyMapReadOnly() noexcept = default;

    /// @brief The document to reify values in, or null if there's none (the rule was removed, or the element
    /// destroyed).
    KRYS_NODISCARD virtual dom::Document *Document() const noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<Value> PropertyValue(PropertyId property) const noexcept = 0;
    KRYS_NODISCARD virtual CSSOMString ShorthandPropertySerialisation(PropertyId property) const noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<Value>
      CustomPropertyValue(const CSSOMStringAtom &property) const noexcept = 0;
  };
}
