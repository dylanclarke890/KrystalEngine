#include "Krystal.Booey/HTML/Elements/HTMLHeadingElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include <algorithm>

namespace krys::boo::html
{
  HTMLHeadingElement::HTMLHeadingElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Heading)
  {
  }

  uint32 HTMLHeadingElement::ComputedHeadingLevel() const noexcept
  {
    // 1. Let level be 0.
    // 2. If element's local name is h1, then set level to 1.
    // 3. If element's local name is h2, then set level to 2.
    // 4. If element's local name is h3, then set level to 3.
    // 5. If element's local name is h4, then set level to 4.
    // 6. If element's local name is h5, then set level to 5.
    // 7. If element's local name is h6, then set level to 6.
    uint64 level = 0;
    switch (TagQualifiedName().TagName())
    {
      case TagName::h1: level = 1; break;
      case TagName::h2: level = 2; break;
      case TagName::h3: level = 3; break;
      case TagName::h4: level = 4; break;
      case TagName::h5: level = 5; break;
      case TagName::h6: level = 6; break;
      default:          break;
    }

    // 8. Assert: level is not 0.
    krys_debug_assert(level != 0);

    // 9. Increment level by the result of getting an element's computed heading offset given element.
    level += std::min<uint64>(ComputedHeadingOffset(*this), 9);

    // 10. If level is greater than 9, then return 9.
    // 11. Return level.
    return static_cast<uint32>(std::min<uint64>(level, 9));
  }

  uint64 HTMLHeadingElement::ComputedHeadingOffset(const dom::Element &element) noexcept
  {
    // 1. Let offset be 0.
    uint64 offset = 0;

    // 2. Let inclusiveAncestor be element.
    const dom::Element *inclusiveAncestor = &element;

    // 3. While inclusiveAncestor is not null:
    while (inclusiveAncestor)
    {
      // 3a. Let nextOffset be 0.
      uint64 nextOffset = 0;

      // 3b. If inclusiveAncestor is an HTML element and has a headingoffset attribute, then parse its value
      //     using the rules for parsing non-negative integers. If the result of parsing the value is not an
      //     error, then set nextOffset to that value.
      auto *htmlElement = DynamicDowncast<HTMLElement>(*inclusiveAncestor);
      if (htmlElement)
      {
        if (auto value = htmlElement->GetAttribute(u8"headingoffset"))
        {
          // (An integer too large for int64 isn't an error, just more than any level needs.)
          if (auto parsed = Numbers::ParseNonNegativeInteger(*value); parsed.Success())
          {
            nextOffset = static_cast<uint64>(parsed.Value);
          }
          else if (parsed.Error == MicroParserError::OutOfRange)
          {
            nextOffset = UINT64_MAX;
          }
        }
      }

      // 3c. Increment offset by nextOffset.
      // (Saturating, as anything past 9 levels is 9.)
      offset = nextOffset > UINT64_MAX - offset ? UINT64_MAX : offset + nextOffset;

      // 3d. If inclusiveAncestor is an HTML element and has a headingreset attribute, then return offset.
      if (htmlElement && htmlElement->HasAttribute(u8"headingreset"))
      {
        return offset;
      }

      // 3e. If inclusiveAncestor's parent is a shadow root, then set inclusiveAncestor to that shadow root's
      //     host and continue.
      // 3f. Set inclusiveAncestor to inclusiveAncestor's parent element.
      inclusiveAncestor = dom::TreeQueries::ParentOrShadowHostElement(*inclusiveAncestor);
    }

    // 4. Return offset.
    return offset;
  }

#pragma region HTMLHeadingElement Obsolete members

  dom::DOMString HTMLHeadingElement::Align() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLHeadingElement::Align(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion
}