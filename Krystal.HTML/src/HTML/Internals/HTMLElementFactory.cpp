#include "Krystal.HTML/HTML/Internals/HTMLElementFactory.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTagName.hpp"
#include "Krystal.HTML/HTML/HTMLBaseElement.hpp"
#include "Krystal.HTML/HTML/HTMLBodyElement.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadingElement.hpp"
#include "Krystal.HTML/HTML/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTML/HTMLLinkElement.hpp"
#include "Krystal.HTML/HTML/HTMLMetaElement.hpp"
#include "Krystal.HTML/HTML/HTMLPreElement.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/HTML/HTMLStyleElement.hpp"
#include "Krystal.HTML/HTML/HTMLTitleElement.hpp"
#include "Krystal.HTML/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  Ref<HTMLElement> HTMLElementFactory::Create(Document &document, DOMStringAtom localName) noexcept
  {
    auto tagName = TryParseHTMLTagName(Krys::Text::ToASCIIUppercase(localName.View()));

    if (tagName.has_value())
    {
      switch (tagName.value())
      {
#pragma region Obsolete elements

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

#pragma endregion

        case HTMLTagName::Address:
        case HTMLTagName::Article:
        case HTMLTagName::Aside:
        case HTMLTagName::Footer:
        case HTMLTagName::Hgroup:
        case HTMLTagName::Header:
        case HTMLTagName::Nav:
        case HTMLTagName::Section:
        {
          return CreateRef<HTMLElement>(document);
        }
        case HTMLTagName::Base:
        {
          return CreateRef<HTMLBaseElement>(document);
        }
        case HTMLTagName::Body:
        {
          return CreateRef<HTMLBodyElement>(document);
        }
        case HTMLTagName::Head:
        {
          return CreateRef<HTMLHeadElement>(document);
        }
        case HTMLTagName::H1:
        case HTMLTagName::H2:
        case HTMLTagName::H3:
        case HTMLTagName::H4:
        case HTMLTagName::H5:
        case HTMLTagName::H6:
        {
          return CreateRef<HTMLHeadingElement>(document);
        }
        case HTMLTagName::Html:
        {
          return CreateRef<HTMLHtmlElement>(document);
        }
        case HTMLTagName::Link:
        {
          return CreateRef<HTMLLinkElement>(document);
        }
        case HTMLTagName::Meta:
        {
          return CreateRef<HTMLMetaElement>(document);
        }
        case HTMLTagName::Pre:
        {
          return CreateRef<HTMLPreElement>(document);
        }
        case HTMLTagName::Slot:
        {
          return CreateRef<HTMLSlotElement>(document);
        }
        case HTMLTagName::Style:
        {
          return CreateRef<HTMLStyleElement>(document);
        }
        case HTMLTagName::Title:
        {
          return CreateRef<HTMLTitleElement>(document);
        }
        default:
        {
          if (NameValidation::IsValidCustomElementName(localName.View()))
          {
            return CreateRef<HTMLElement>(document);
          }

          return CreateRef<HTMLUnknownElement>(document);
        }
      }
    }

  }
}