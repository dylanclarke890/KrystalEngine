/*
 * Copyright (C) 2014 Gurpreet Kaur (k.gurpreet@samsung.com). All rights reserved.
 * Copyright (C) 2016 Igalia S.L.
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

#include "Krystal.Booey/MathML/Elements/MathMLMencloseElement.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLMenclose.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::mathml
{
  MathMLMencloseElement::MathMLMencloseElement(dom::Document &document) noexcept
      : MathMLRowElement(document, MathMLElementInterface::Menclose)
  {
    // By default we draw a longdiv.
    ClearNotations();
    AddNotation(LongDiv);
  }

  void MathMLMencloseElement::AddNotationFlags(dom::DOMStringView notation) noexcept
  {
    krys_debug_assert(_notationFlags);
    if (notation == u8"longdiv")
    {
      AddNotation(LongDiv);
    }
    else if (notation == u8"roundedbox")
    {
      AddNotation(RoundedBox);
    }
    else if (notation == u8"circle")
    {
      AddNotation(Circle);
    }
    else if (notation == u8"left")
    {
      AddNotation(Left);
    }
    else if (notation == u8"right")
    {
      AddNotation(Right);
    }
    else if (notation == u8"top")
    {
      AddNotation(Top);
    }
    else if (notation == u8"bottom")
    {
      AddNotation(Bottom);
    }
    else if (notation == u8"updiagonalstrike")
    {
      AddNotation(UpDiagonalStrike);
    }
    else if (notation == u8"downdiagonalstrike")
    {
      AddNotation(DownDiagonalStrike);
    }
    else if (notation == u8"verticalstrike")
    {
      AddNotation(VerticalStrike);
    }
    else if (notation == u8"horizontalstrike")
    {
      AddNotation(HorizontalStrike);
    }
    else if (notation == u8"updiagonalarrow")
    {
      AddNotation(UpDiagonalArrow);
    }
    else if (notation == u8"phasorangle")
    {
      AddNotation(PhasorAngle);
    }
    else if (notation == u8"box")
    {
      AddNotation(Left);
      AddNotation(Right);
      AddNotation(Top);
      AddNotation(Bottom);
    }
    else if (notation == u8"actuarial")
    {
      AddNotation(Right);
      AddNotation(Top);
    }
    else if (notation == u8"madruwb")
    {
      AddNotation(Right);
      AddNotation(Bottom);
    }
  }

  void MathMLMencloseElement::ParseNotationAttribute() noexcept
  {
    ClearNotations();
    auto attribute = AttributeValue(MathMLNames::NotationAttr());
    if (!attribute)
    {
      AddNotation(LongDiv); // The default value is longdiv.
      return;
    }

    // We parse the list of whitespace-separated notation names.
    dom::DOMStringView value = *attribute;
    size_t length = value.size();
    size_t start = 0;
    while (start < length)
    {
      if (text::IsASCIIWhitespace(value[start]))
      {
        start++;
        continue;
      }
      size_t end = start + 1;
      while (end < length && !text::IsASCIIWhitespace(value[end]))
      {
        end++;
      }
      AddNotationFlags(value.substr(start, end - start));
      start = end;
    }
  }

  bool MathMLMencloseElement::HasNotation(MencloseNotationFlag notationFlag) noexcept
  {
    if (!_notationFlags)
    {
      ParseNotationAttribute();
    }
    return *_notationFlags & notationFlag;
  }

  void MathMLMencloseElement::AttributeChanged(const dom::QualifiedName &name,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == MathMLNames::NotationAttr())
    {
      _notationFlags.reset();
      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }

    MathMLRowElement::AttributeChanged(name, oldValue, value);
  }

  RenderPtr<RenderElement> MathMLMencloseElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(HasTagName(html::TagName::menclose));
    return CreateRenderer<RenderMathMLMenclose>(*this, krys::move(style));
  }
}
