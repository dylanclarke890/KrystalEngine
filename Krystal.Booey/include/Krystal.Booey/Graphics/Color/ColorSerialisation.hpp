/*
 * Copyright (C) 2020-2021 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys
{
  enum class ColourSpace : uint8;
}

namespace krys::boo
{
  class Color;

  /// @brief Returns the color serialised according to CSS.
  /// @see https://drafts.csswg.org/css-color-4/#serializing-color-values
  KRYS_NODISCARD utf8_string SerialisationForCSS(const Color &color) noexcept;

  /// @brief Returns the color serialised according to HTML5.
  /// @see https://html.spec.whatwg.org/multipage/scripting.html#fill-and-stroke-styles (10 September 2015).
  KRYS_NODISCARD utf8_string SerialisationForHTML(const Color &color) noexcept;

  /// @brief Returns the color serialised for DumpRenderTree: #RRGGBB, #RRGGBBAA or the CSS serialisation.
  KRYS_NODISCARD utf8_string SerialisationForRenderTreeAsText(const Color &color) noexcept;

  /// @brief Returns the CSS name of the color space, as used by `color()`.
  KRYS_NODISCARD utf8_stringview Serialisation(ColourSpace colorSpace) noexcept;
}
