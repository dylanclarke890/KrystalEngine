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
#include "Krystal.HTML/HTML/HTMLAnchorElement.hpp"
#include "Krystal.HTML/HTML/HTMLAreaElement.hpp"
#include "Krystal.HTML/HTML/HTMLAudioElement.hpp"
#include "Krystal.HTML/HTML/HTMLBaseElement.hpp"
#include "Krystal.HTML/HTML/HTMLBodyElement.hpp"
#include "Krystal.HTML/HTML/HTMLDataElement.hpp"
#include "Krystal.HTML/HTML/HTMLDivElement.hpp"
#include "Krystal.HTML/HTML/HTMLDListElement.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/HTML/HTMLEmbedElement.hpp"
#include "Krystal.HTML/HTML/HTMLFormElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadingElement.hpp"
#include "Krystal.HTML/HTML/HTMLHRElement.hpp"
#include "Krystal.HTML/HTML/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTML/HTMLIFrameElement.hpp"
#include "Krystal.HTML/HTML/HTMLImageElement.hpp"
#include "Krystal.HTML/HTML/HTMLInputElement.hpp"
#include "Krystal.HTML/HTML/HTMLLabelElement.hpp"
#include "Krystal.HTML/HTML/HTMLLIElement.hpp"
#include "Krystal.HTML/HTML/HTMLLinkElement.hpp"
#include "Krystal.HTML/HTML/HTMLMapElement.hpp"
#include "Krystal.HTML/HTML/HTMLMediaElement.hpp"
#include "Krystal.HTML/HTML/HTMLMenuElement.hpp"
#include "Krystal.HTML/HTML/HTMLMetaElement.hpp"
#include "Krystal.HTML/HTML/HTMLModElement.hpp"
#include "Krystal.HTML/HTML/HTMLObjectElement.hpp"
#include "Krystal.HTML/HTML/HTMLOListElement.hpp"
#include "Krystal.HTML/HTML/HTMLParagraphElement.hpp"
#include "Krystal.HTML/HTML/HTMLPictureElement.hpp"
#include "Krystal.HTML/HTML/HTMLPreElement.hpp"
#include "Krystal.HTML/HTML/HTMLQuoteElement.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/HTML/HTMLSourceElement.hpp"
#include "Krystal.HTML/HTML/HTMLSpanElement.hpp"
#include "Krystal.HTML/HTML/HTMLStyleElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableCaptionElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableCellElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableColElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableRowElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableSectionElement.hpp"
#include "Krystal.HTML/HTML/HTMLTimeElement.hpp"
#include "Krystal.HTML/HTML/HTMLTitleElement.hpp"
#include "Krystal.HTML/HTML/HTMLTrackElement.hpp"
#include "Krystal.HTML/HTML/HTMLUListElement.hpp"
#include "Krystal.HTML/HTML/HTMLUnknownElement.hpp"
#include "Krystal.HTML/HTML/HTMLVideoElement.hpp"
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
        case HTMLTagName::abbr:
        case HTMLTagName::address:
        case HTMLTagName::article:
        case HTMLTagName::aside:
        case HTMLTagName::b:
        case HTMLTagName::bdi:
        case HTMLTagName::bdo:
        case HTMLTagName::cite:
        case HTMLTagName::code:
        case HTMLTagName::dd:
        case HTMLTagName::dfn:
        case HTMLTagName::dt:
        case HTMLTagName::em:
        case HTMLTagName::figure:
        case HTMLTagName::figcaption:
        case HTMLTagName::footer:
        case HTMLTagName::hgroup:
        case HTMLTagName::header:
        case HTMLTagName::i:
        case HTMLTagName::kbd:
        case HTMLTagName::main:
        case HTMLTagName::mark:
        case HTMLTagName::nav:
        case HTMLTagName::ruby:
        case HTMLTagName::rp:
        case HTMLTagName::rt:
        case HTMLTagName::s:
        case HTMLTagName::samp:
        case HTMLTagName::search:
        case HTMLTagName::section:
        case HTMLTagName::small:
        case HTMLTagName::strong:
        case HTMLTagName::sub:
        case HTMLTagName::sup:
        case HTMLTagName::u:
        case HTMLTagName::var:
        case HTMLTagName::wbr:
        {
          return CreateRef<HTMLElement>(document);
        }
        case HTMLTagName::a:
        {
          return CreateRef<HTMLAnchorElement>(document);
        }
        case HTMLTagName::area:
        {
          return CreateRef<HTMLAreaElement>(document);
        }
        case HTMLTagName::audio:
        {
          return CreateRef<HTMLAudioElement>(document);
        }
        case HTMLTagName::base:
        {
          return CreateRef<HTMLBaseElement>(document);
        }
        case HTMLTagName::blockquote:
        case HTMLTagName::q:
        {
          return CreateRef<HTMLQuoteElement>(document);
        }
        case HTMLTagName::body:
        {
          return CreateRef<HTMLBodyElement>(document);
        }
        case HTMLTagName::caption:
        {
          return CreateRef<HTMLTableCaptionElement>(document);
        }
        case HTMLTagName::col:
        case HTMLTagName::colgroup:
        {
          return CreateRef<HTMLTableColElement>(document);
        }
        case HTMLTagName::data:
        {
          return CreateRef<HTMLDataElement>(document);
        }
        case HTMLTagName::del:
        case HTMLTagName::ins:
        {
          return CreateRef<HTMLModElement>(document);
        }
        case HTMLTagName::div:
        {
          return CreateRef<HTMLDivElement>(document);
        }
        case HTMLTagName::dl:
        {
          return CreateRef<HTMLDListElement>(document);
        }
        case HTMLTagName::embed:
        {
          return CreateRef<HTMLEmbedElement>(document);
        }
        case HTMLTagName::form:
        {
          return CreateRef<HTMLFormElement>(document);
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
        case HTMLTagName::iframe:
        {
          return CreateRef<HTMLIFrameElement>(document);
        }
        case HTMLTagName::input:
        {
          return CreateRef<HTMLInputElement>(document);
        }
        case HTMLTagName::img:
        {
          return CreateRef<HTMLImageElement>(document);
        }
        case HTMLTagName::label:
        {
          return CreateRef<HTMLLabelElement>(document);
        }
        case HTMLTagName::li:
        {
          return CreateRef<HTMLLIElement>(document);
        }
        case HTMLTagName::link:
        {
          return CreateRef<HTMLLinkElement>(document);
        }
        case HTMLTagName::map:
        {
          return CreateRef<HTMLMapElement>(document);
        }
        case HTMLTagName::menu:
        {
          return CreateRef<HTMLMenuElement>(document);
        }
        case HTMLTagName::meta:
        {
          return CreateRef<HTMLMetaElement>(document);
        }
        case HTMLTagName::object:
        {
          return CreateRef<HTMLObjectElement>(document);
        }
        case HTMLTagName::ol:
        {
          return CreateRef<HTMLOListElement>(document);
        }
        case HTMLTagName::p:
        {
          return CreateRef<HTMLParagraphElement>(document);
        }
        case HTMLTagName::picture:
        {
          return CreateRef<HTMLPictureElement>(document);
        }
        case HTMLTagName::pre:
        {
          return CreateRef<HTMLPreElement>(document);
        }
        case HTMLTagName::slot:
        {
          return CreateRef<HTMLSlotElement>(document);
        }
        case HTMLTagName::source:
        {
          return CreateRef<HTMLSourceElement>(document);
        }
        case HTMLTagName::span:
        {
          return CreateRef<HTMLSpanElement>(document);
        }
        case HTMLTagName::style:
        {
          return CreateRef<HTMLStyleElement>(document);
        }
        case HTMLTagName::table:
        {
          return CreateRef<HTMLTableElement>(document);
        }
        case HTMLTagName::tbody:
        case HTMLTagName::thead:
        case HTMLTagName::tfoot:
        {
          return CreateRef<HTMLTableSectionElement>(document);
        }
        case HTMLTagName::td:
        case HTMLTagName::th:
        {
          return CreateRef<HTMLTableCellElement>(document);
        }
        case HTMLTagName::tr:
        {
          return CreateRef<HTMLTableRowElement>(document);
        }
        case HTMLTagName::title:
        {
          return CreateRef<HTMLTitleElement>(document);
        }
        case HTMLTagName::time:
        {
          return CreateRef<HTMLTimeElement>(document);
        }
        case HTMLTagName::track:
        {
          return CreateRef<HTMLTrackElement>(document);
        }
        case HTMLTagName::ul:
        {
          return CreateRef<HTMLUListElement>(document);
        }
        case HTMLTagName::video:
        {
          return CreateRef<HTMLVideoElement>(document);
        }
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
      }
    }

    if (NameValidation::IsValidCustomElementName(localName.View()))
    {
      return CreateRef<HTMLElement>(document);
    }

    return CreateRef<HTMLUnknownElement>(document);
  }
}