#include "Krystal.Booey/MathML/MathMLMencloseElement.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
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
    }

    MathMLRowElement::AttributeChanged(name, oldValue, value);
  }
}
