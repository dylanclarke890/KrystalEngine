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
#include "Krystal.HTML/HTML/HTMLDivElement.hpp"
#include "Krystal.HTML/HTML/HTMLDListElement.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadingElement.hpp"
#include "Krystal.HTML/HTML/HTMLHRElement.hpp"
#include "Krystal.HTML/HTML/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTML/HTMLLIElement.hpp"
#include "Krystal.HTML/HTML/HTMLLinkElement.hpp"
#include "Krystal.HTML/HTML/HTMLMenuElement.hpp"
#include "Krystal.HTML/HTML/HTMLMetaElement.hpp"
#include "Krystal.HTML/HTML/HTMLOListElement.hpp"
#include "Krystal.HTML/HTML/HTMLParagraphElement.hpp"
#include "Krystal.HTML/HTML/HTMLPreElement.hpp"
#include "Krystal.HTML/HTML/HTMLQuoteElement.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/HTML/HTMLStyleElement.hpp"
#include "Krystal.HTML/HTML/HTMLTitleElement.hpp"
#include "Krystal.HTML/HTML/HTMLUListElement.hpp"
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

        case HTMLTagName::applet:
        case HTMLTagName::bgsound:
        case HTMLTagName::blink:
        case HTMLTagName::isindex:
        case HTMLTagName::keygen:
        case HTMLTagName::multicol:
        case HTMLTagName::nextid:
        case HTMLTagName::spacer:
        {
          return CreateRef<HTMLUnknownElement>(document);
        }
        case HTMLTagName::acronym:
        case HTMLTagName::basefont:
        case HTMLTagName::big:
        case HTMLTagName::center:
        case HTMLTagName::nobr:
        case HTMLTagName::noembed:
        case HTMLTagName::noframes:
        case HTMLTagName::plaintext:
        case HTMLTagName::rb:
        case HTMLTagName::rtc:
        case HTMLTagName::strike:
        case HTMLTagName::tt:
        {
          return CreateRef<HTMLElement>(document);
        }
        case HTMLTagName::listing:
        case HTMLTagName::xmp:
        {
          return CreateRef<HTMLPreElement>(document);
        }

#pragma endregion

        case HTMLTagName::address:
        case HTMLTagName::article:
        case HTMLTagName::aside:
        case HTMLTagName::dd:
        case HTMLTagName::dt:
        case HTMLTagName::figure:
        case HTMLTagName::figcaption:
        case HTMLTagName::footer:
        case HTMLTagName::hgroup:
        case HTMLTagName::header:
        case HTMLTagName::main:
        case HTMLTagName::nav:
        case HTMLTagName::search:
        case HTMLTagName::section:
        {
          return CreateRef<HTMLElement>(document);
        }
        case HTMLTagName::base:
        {
          return CreateRef<HTMLBaseElement>(document);
        }
        case HTMLTagName::blockquote:
        {
          return CreateRef<HTMLQuoteElement>(document);
        }
        case HTMLTagName::body:
        {
          return CreateRef<HTMLBodyElement>(document);
        }
        case HTMLTagName::div:
        {
          return CreateRef<HTMLDivElement>(document);
        }
        case HTMLTagName::dl:
        {
          return CreateRef<HTMLDListElement>(document);
        }
        case HTMLTagName::head:
        {
          return CreateRef<HTMLHeadElement>(document);
        }
        case HTMLTagName::h1:
        case HTMLTagName::h2:
        case HTMLTagName::h3:
        case HTMLTagName::h4:
        case HTMLTagName::h5:
        case HTMLTagName::h6:
        {
          return CreateRef<HTMLHeadingElement>(document);
        }
        case HTMLTagName::hr:
        {
          return CreateRef<HTMLHRElement>(document);
        }
        case HTMLTagName::html:
        {
          return CreateRef<HTMLHtmlElement>(document);
        }
        case HTMLTagName::li:
        {
          return CreateRef<HTMLLinkElement>(document);
        }
        case HTMLTagName::link:
        {
          return CreateRef<HTMLLinkElement>(document);
        }
        case HTMLTagName::menu:
        {
          return CreateRef<HTMLMenuElement>(document);
        }
        case HTMLTagName::meta:
        {
          return CreateRef<HTMLMetaElement>(document);
        }
        case HTMLTagName::ol:
        {
          return CreateRef<HTMLOListElement>(document);
        }
        case HTMLTagName::p:
        {
          return CreateRef<HTMLParagraphElement>(document);
        }
        case HTMLTagName::pre:
        {
          return CreateRef<HTMLPreElement>(document);
        }
        case HTMLTagName::slot:
        {
          return CreateRef<HTMLSlotElement>(document);
        }
        case HTMLTagName::style:
        {
          return CreateRef<HTMLStyleElement>(document);
        }
        case HTMLTagName::title:
        {
          return CreateRef<HTMLTitleElement>(document);
        }
        case HTMLTagName::ul:
        {
          return CreateRef<HTMLUListElement>(document);
        }
        default:
        {
          break; // TODO: remove this once we've finished implementing the various interfaces
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