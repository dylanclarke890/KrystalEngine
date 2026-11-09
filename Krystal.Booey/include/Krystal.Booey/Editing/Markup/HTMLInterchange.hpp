/*
 * Copyright (C) 2004, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class Text;
}

namespace krys::boo
{
  constexpr utf8_stringview AppleInterchangeNewline = u8"Apple-interchange-newline";
  constexpr utf8_stringview AppleConvertedSpace = u8"Apple-converted-space";
  constexpr utf8_stringview WebKitMSOListQuirksStyle = u8"WebKit-mso-list-quirks-style";
  constexpr utf8_stringview AppleTabSpanClass = u8"Apple-tab-span";

  enum class AnnotateForInterchange : bool
  {
    No,
    Yes
  };

  /// @brief @p in, from @p node, with runs of collapsible spaces kept as spans of no-break spaces, so they
  /// survive being pasted.
  KRYS_NODISCARD utf8_string ConvertHTMLTextToInterchangeFormat(utf8_stringview in,
                                                                const dom::Text *node) noexcept;
}
