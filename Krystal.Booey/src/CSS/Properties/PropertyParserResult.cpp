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

#include "Krystal.Booey/CSS/Properties/PropertyParserResult.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  void PropertyParserResult::AddProperty(Property &&property) noexcept
  {
    ParsedProperties.emplace_back(krys::move(property));
  }

  void PropertyParserResult::AddProperty(KRYS_MAYBE_UNUSED PropertyParserState &state, PropertyId property,
                                         PropertyId currentShorthand, RefPtr<Value> &&value,
                                         IsImportant important, IsImplicit implicit) noexcept
  {
    uint8 shorthandIndex = 0u;
    IsSetFromShorthand setFromShorthand = IsSetFromShorthand(false);

    if (currentShorthand != PropertyId::Invalid)
    {
      auto shorthands = MatchingShorthandsForLonghand(property);
      setFromShorthand = IsSetFromShorthand(true);
      if (shorthands.size() > 1uz)
      {
        shorthandIndex = static_cast<uint8>(IndexOfShorthandForLonghand(currentShorthand, shorthands));
      }
    }

    // Allow anything to be set from a shorthand (e.g. the CSS all property always sets everything,
    // regardless of whether the longhands are enabled), and allow internal properties as we use
    // them to handle certain DOM-exposed values.
    krys_debug_assert(IsExposed(property, state.Context.PropertySettings) || setFromShorthand
                      || IsInternal(property));

    if (value && !value->IsImplicitInitialValue())
    {
      AddProperty(
        Property(property, krys::move(value), important, implicit, setFromShorthand, shorthandIndex));
    }
    else
    {
      krys_debug_assert(setFromShorthand);
      AddProperty(Property(property, ShareRef(PrimitiveValue::ImplicitInitialValue()), important,
                           IsImplicit(true), setFromShorthand, shorthandIndex));
    }
  }

#pragma region Utils

  void PropertyParserResult::AddPropertyForCurrentShorthand(PropertyParserState &state, PropertyId longhand,
                                                            RefPtr<Value> &&value,
                                                            IsImplicit implicit) noexcept
  {
    AddProperty(state, longhand, state.CurrentProperty, krys::move(value), state.Important, implicit);
  }

  void PropertyParserResult::AddPropertyForAllLonghandsOfShorthand(PropertyParserState &state,
                                                                   PropertyId shorthand,
                                                                   RefPtr<Value> &&value,
                                                                   IsImportant important,
                                                                   IsImplicit implicit) noexcept
  {
    for (auto longhand : ShorthandForProperty(shorthand))
    {
      AddProperty(state, longhand, shorthand, RefPtr<Value>(value), important, implicit);
    }
  }

  void PropertyParserResult::AddPropertyForAllLonghandsOfCurrentShorthand(PropertyParserState &state,
                                                                          RefPtr<Value> &&value,
                                                                          IsImplicit implicit) noexcept
  {
    AddPropertyForAllLonghandsOfShorthand(state, state.CurrentProperty, krys::move(value), state.Important,
                                          implicit);
  }

#pragma endregion
}