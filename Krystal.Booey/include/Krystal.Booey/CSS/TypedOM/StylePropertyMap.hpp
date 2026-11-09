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

#include "Krystal.Booey/CSS/TypedOM/CSSStyleValueFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/StylePropertyMapReadOnly.hpp"

namespace krys::boo::css
{
  class VariableReferenceValue;

  /// @brief A map of properties to their values that can be changed.
  /// @see https://drafts.css-houdini.org/css-typed-om/#stylepropertymap
  class StylePropertyMap : public StylePropertyMapReadOnly
  {
  public:
    /// @brief Sets @p property to @p values, a string being parsed as its value.
    /// @return A `TypeError` if @p property isn't a property, or @p values aren't a valid value of it.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-set
    dom::ExceptionOr<void> Set(CSSOMStringView property, List<CSSStyleValueOrString> values) noexcept;
    /// @brief Appends @p values to the list-valued @p property.
    /// @return A `TypeError` if @p property isn't a list-valued property, or @p values aren't valid items of
    /// it.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-append
    dom::ExceptionOr<void> Append(CSSOMStringView property, List<CSSStyleValueOrString> values) noexcept;
    /// @brief Removes @p property.
    /// @return A `TypeError` if @p property isn't a property.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-delete
    dom::ExceptionOr<void> Delete(CSSOMStringView property) noexcept;
    /// @brief Removes every property.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-clear
    virtual void Clear() noexcept = 0;

    KRYS_NODISCARD bool IsStylePropertyMap() const noexcept final
    {
      return true;
    }

  protected:
    virtual void RemoveProperty(PropertyId property) noexcept = 0;
    virtual void RemoveCustomProperty(const CSSOMStringAtom &property) noexcept = 0;
    /// @return False if @p value doesn't parse as @p property's value.
    virtual bool SetShorthandProperty(PropertyId property, const CSSOMString &value) noexcept = 0;
    /// @return False if @p value isn't a valid value of @p property.
    virtual bool SetProperty(PropertyId property, Ref<Value> value) noexcept = 0;
    virtual bool SetCustomProperty(const CSSOMStringAtom &property,
                                   Ref<VariableReferenceValue> value) noexcept = 0;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::StylePropertyMap)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::StylePropertyMapReadOnly &map) noexcept
  {
    return map.IsStylePropertyMap();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
