/*
 * Copyright (C) 2016-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Svg/SVGLength.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/SVGLengthContext.hpp"

namespace krys::web
{
  ExceptionOr<float> SVGLength::ValueForBindings() noexcept
  {
    RefPtr element = ShareRefPtr(ContextElement());

    // Ensure style is up-to-date for font-relative units (e.g., ch).
    // https://www.w3.org/TR/css-values-3/#absolute-lengths
    auto requiresStyleUpdate = [](SVGLengthType type)
    {
      switch (type)
      {
        case SVGLengthType::Number:
        case SVGLengthType::Pixels:
        case SVGLengthType::Centimeters:
        case SVGLengthType::Millimeters:
        case SVGLengthType::Inches:
        case SVGLengthType::Points:
        case SVGLengthType::Picas:       return false; // Absolute units don't need style update.
        default:                         return true;                          // Font-relative units need style update.
      }
    };

    if (requiresStyleUpdate(_value.LengthType()))
    {
      if (element)
      {
        element->NodeDocument().UpdateStyleIfNeeded();
      }
    }

    return _value.ValueForBindings(SVGLengthContext {element.get()});
  }

  ExceptionOr<void> SVGLength::SetValueForBindings(float value) noexcept
  {
    if (IsReadOnly())
    {
      return Exception {ExceptionCode::NoModificationAllowedError};
    }

    auto result = _value.SetValue(SVGLengthContext {RefPtr {ShareRefPtr(ContextElement())}.get()}, value);
    if (result.HasException())
    {
      return result;
    }

    CommitChange();
    return result;
  }

  ExceptionOr<void> SVGLength::ConvertToSpecifiedUnits(uint16 unitType) noexcept
  {
    if (IsReadOnly())
    {
      return Exception {ExceptionCode::NoModificationAllowedError};
    }

    if (unitType == SVG_LENGTHTYPE_UNKNOWN || unitType > SVG_LENGTHTYPE_PC)
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    auto result = _value.ConvertToSpecifiedUnits(
      SVGLengthContext {RefPtr {ShareRefPtr(ContextElement())}.get()}, static_cast<SVGLengthType>(unitType));
    if (result.HasException())
    {
      return result;
    }

    CommitChange();
    return result;
  }
}
