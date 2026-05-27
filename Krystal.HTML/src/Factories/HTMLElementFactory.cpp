#include "Krystal.HTML/Factories/HTMLElementFactory.hpp"
#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLPreElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLTagName.hpp"
#include "Krystal.HTML/HTMLElement/HTMLUnknownElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  RefPtr<HTMLElement> HTMLElementFactory::TryCreate(Document &document, DOMStringAtom localName) noexcept
  {
    auto tagName = TryParseHTMLTagName(Krys::Text::ToASCIIUppercase(localName.View()));

    if (tagName.has_value())
    {
      switch (tagName.value())
      {
        case HTMLTagName::Applet:
        case HTMLTagName::Bgsound:
        case HTMLTagName::Blink:
        case HTMLTagName::Isindex:
        case HTMLTagName::Keygen:
        case HTMLTagName::Multicol:
        case HTMLTagName::Nextid:
        case HTMLTagName::Spacer:
        {
          return CreateRef<HTMLUnknownElement>(document);
        }
        case HTMLTagName::Acronym:
        case HTMLTagName::Basefont:
        case HTMLTagName::Big:
        case HTMLTagName::Center:
        case HTMLTagName::Nobr:
        case HTMLTagName::Noembed:
        case HTMLTagName::Noframes:
        case HTMLTagName::Plaintext:
        case HTMLTagName::Rb:
        case HTMLTagName::Rtc:
        case HTMLTagName::Strike:
        case HTMLTagName::Tt:
        {
          return CreateRef<HTMLElement>(document);
        }
        case HTMLTagName::Listing:
        case HTMLTagName::Xmp:
        {
          return CreateRef<HTMLPreElement>(document);
        }
        default:
        {
          if (tagName == HTMLTagName::Unknown)
          {
            return CreateRef<HTMLUnknownElement>(document);
          }

          return CreateRef<HTMLElement>(document);
        }
      }
    }

    if (NameValidation::IsValidCustomElementName(localName.View()))
    {
      return CreateRef<HTMLElement>(document);
    }

    return CreateRef<HTMLUnknownElement>(document);
  }
}