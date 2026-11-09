/*
 * Copyright (C) 2022 Igalia S.L. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"

namespace krys::boo::css
{
  BorderImageWidthValue::BorderImageWidthValue(Quad widths, bool overridesBorderWidths) noexcept
      : Value(ValueType::BorderImageWidth), _widths(krys::move(widths)),
        _overridesBorderWidths(overridesBorderWidths)
  {
  }

  Ref<BorderImageWidthValue> BorderImageWidthValue::Create(Quad widths, bool overridesBorderWidths) noexcept
  {
    return AdoptRef(*new BorderImageWidthValue(krys::move(widths), overridesBorderWidths));
  }

  CSSOMString BorderImageWidthValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    // The border-image-width longhand can't set _overridesBorderWidths to true, so serialise as empty string.
    // This can only be created by the -webkit-border-image shorthand, which will not serialise as empty
    // string in this case. This is an unconventional relationship between a longhand and a shorthand, which
    // we may want to revise.
    if (_overridesBorderWidths)
    {
      return CSSOMString();
    }
    return _widths.CssText(context);
  }

  bool BorderImageWidthValue::Equals(const BorderImageWidthValue &other) const noexcept
  {
    return _overridesBorderWidths == other._overridesBorderWidths && _widths.Equals(other._widths);
  }
}
