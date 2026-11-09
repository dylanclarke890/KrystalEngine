/*
 * Copyright (C) 2010-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Platform/Decimal.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <limits>

namespace krys::boo::html
{
  /// @brief Whether @p character is a line feed or a carriage return.
  KRYS_NODISCARD constexpr bool IsHTMLLineBreak(char8 character) noexcept
  {
    return character == u8'\n' || character == u8'\r';
  }

  /// @brief The best representation of @p number as a floating-point number, for number-like input types.
  /// @see https://html.spec.whatwg.org/#best-representation-of-the-number-as-a-floating-point-number
  KRYS_NODISCARD dom::DOMString SerializeForNumberType(const Decimal &number) noexcept;

  /// @brief The best representation of @p number as a floating-point number: what script's `String(number)`
  /// gives.
  /// @see https://html.spec.whatwg.org/#best-representation-of-the-number-as-a-floating-point-number
  KRYS_NODISCARD dom::DOMString SerializeForNumberType(double number) noexcept;

  /// @brief Parses @p string as a valid floating-point number (no leading `+` or whitespace), or returns
  /// @p fallbackValue.
  /// @see https://html.spec.whatwg.org/#valid-floating-point-number
  KRYS_NODISCARD Decimal ParseToDecimalForNumberType(dom::DOMStringView string,
                                                     const Decimal &fallbackValue) noexcept;

  KRYS_NODISCARD Decimal ParseToDecimalForNumberType(dom::DOMStringView string) noexcept;

  /// @brief Parses @p string as a valid floating-point number, or returns @p fallbackValue.
  /// @see https://html.spec.whatwg.org/#valid-floating-point-number
  KRYS_NODISCARD double ParseToDoubleForNumberType(dom::DOMStringView string, double fallbackValue) noexcept;

  KRYS_NODISCARD double ParseToDoubleForNumberType(dom::DOMStringView string) noexcept;

  /// @brief Parses @p input with the rules for parsing floating-point number values, or returns
  /// @p fallbackValue.
  /// @see https://html.spec.whatwg.org/#rules-for-parsing-floating-point-number-values
  KRYS_NODISCARD double ParseHTMLFloatingPointNumberValue(dom::DOMStringView input,
                                                          double fallbackValue = 0) noexcept;
}
