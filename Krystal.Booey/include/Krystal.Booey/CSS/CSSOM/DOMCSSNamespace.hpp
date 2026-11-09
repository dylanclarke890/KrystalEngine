/*
 * Copyright (C) 2012 Motorola Mobility Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Motorola Mobility Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  /// @brief A custom property to register with `CSS.registerProperty()`.
  /// @see https://drafts.css-houdini.org/css-properties-values-api/#dictdef-propertydefinition
  struct PropertyDefinition
  {
    CSSOMString Name;
    CSSOMString Syntax {u8"*"};
    bool Inherits {false};
    Maybe<CSSOMString> InitialValue;
  };

  /// @brief The `CSS` namespace. Members that depend on a global take the document standing in for it.
  /// @see https://drafts.csswg.org/cssom/#namespacedef-css
  struct DOMCSSNamespace
  {
    /// @brief @p ident escaped for use as a CSS identifier.
    /// @see https://drafts.csswg.org/cssom/#dom-css-escape
    KRYS_NODISCARD static CSSOMString Escape(const CSSOMString &ident) noexcept;

    /// @brief Whether @p property is a supported property (or a custom property) and @p value parses for it.
    /// @see https://drafts.csswg.org/css-conditional-3/#dom-css-supports
    KRYS_NODISCARD static bool Supports(const dom::Document &document, CSSOMStringView property,
                                        CSSOMStringView value) noexcept;

    /// @brief Whether @p conditionText, a `<supports-condition>` (or, failing that, a declaration as if it
    /// were in parentheses), is supported.
    /// @see https://drafts.csswg.org/css-conditional-3/#dom-css-supports-conditiontext
    KRYS_NODISCARD static bool Supports(const dom::Document &document,
                                        CSSOMStringView conditionText) noexcept;

    /// @brief Registers the custom property @p definition describes in @p document.
    /// @return A `SyntaxError` if the name, syntax or initial value is invalid, and an
    /// `InvalidModificationError` if the property is already registered.
    /// @see https://drafts.css-houdini.org/css-properties-values-api/#dom-css-registerproperty
    static dom::ExceptionOr<void> RegisterProperty(dom::Document &document,
                                                   const PropertyDefinition &definition) noexcept;
  };
}
