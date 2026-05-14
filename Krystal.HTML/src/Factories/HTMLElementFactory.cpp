#include "Krystal.HTML/Factories/HTMLElementFactory.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLPreElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLTagName.hpp"
#include "Krystal.HTML/HTMLElement/HTMLUnknownElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  RefPtr<HTMLElement> HTMLElementFactory::TryCreate(Document &document, const QualifiedName &name,
                                                    DOMStringAtom is,
                                                    RawPtr<CustomElementRegistry> registry) noexcept
  {
    auto tagName = TryParseHTMLTagName(Krys::Text::ToASCIIUppercase(name.LocalName.View()));

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
          return CreateRef<HTMLUnknownElement>(document, name);
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
          return CreateRef<HTMLElement>(document, name);
        }
        case HTMLTagName::Listing:
        case HTMLTagName::Xmp:
        {
          return CreateRef<HTMLPreElement>(document, name);
        }
        default:
        {
          if (tagName == HTMLTagName::Unknown)
          {
            return CreateRef<HTMLUnknownElement>(document, name);
          }

          return CreateRef<HTMLElement>(document, name);
        }
      }
    }

    if (NameValidation::IsValidCustomElementName(name.LocalName.View()))
    {
      return CreateRef<HTMLElement>(document, name);
    }

    return CreateRef<HTMLUnknownElement>(document, name);
  }
}