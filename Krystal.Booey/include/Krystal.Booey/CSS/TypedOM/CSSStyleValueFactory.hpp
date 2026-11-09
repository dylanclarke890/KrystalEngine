/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  class Value;

  /// @brief A style value, or a string to parse as one or more.
  using CSSStyleValueOrString = Variant<RefPtr<CSSStyleValue>, CSSOMString>;

  /// @brief Parses CSS text into Typed OM values, and reifies CSS values as them.
  struct CSSStyleValueFactory
  {
    /// @brief Reifies @p value, a value of @p property (or of no particular property).
    /// @see https://drafts.css-houdini.org/css-typed-om/#reify-as-a-cssstylevalue
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSStyleValue>>
      ReifyValue(dom::Document &document, const Value &value, Maybe<PropertyId> property) noexcept;
    /// @brief Parses @p cssText as a value of @p property, then reifies its first iteration, or each of them
    /// if
    /// @p parseMultiple.
    /// @return A `TypeError` if @p property isn't a property or @p cssText doesn't parse as its value.
    /// @see https://drafts.css-houdini.org/css-typed-om/#parse-a-cssstylevalue
    KRYS_NODISCARD static dom::ExceptionOr<List<Ref<CSSStyleValue>>>
      ParseStyleValue(dom::Document &document, CSSOMStringView property, CSSOMStringView cssText,
                      bool parseMultiple) noexcept;
    /// @brief A shorthand's value, from its serialisation: an unparsed value if it references variables,
    /// otherwise a generic style value. Null if @p serialisation is.
    KRYS_NODISCARD static RefPtr<CSSStyleValue>
      ConstructStyleValueForShorthandSerialisation(dom::Document &document,
                                                   const CSSOMString &serialisation) noexcept;
    /// @brief @p values as style values for @p property, the strings among them parsed as each of their
    /// iterations.
    KRYS_NODISCARD static dom::ExceptionOr<List<Ref<CSSStyleValue>>>
      ListFromStyleValuesOrStrings(dom::Document &document, CSSOMStringView property,
                                   List<CSSStyleValueOrString> values) noexcept;
  };
}
