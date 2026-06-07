#include "Krystal.HTML/HTML/Internals/HTMLElementFactory.hpp"
#include "Krystal.HTML/Constants/ElementNames.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLAnchorElement.hpp"
#include "Krystal.HTML/HTML/HTMLAreaElement.hpp"
#include "Krystal.HTML/HTML/HTMLAudioElement.hpp"
#include "Krystal.HTML/HTML/HTMLBaseElement.hpp"
#include "Krystal.HTML/HTML/HTMLBodyElement.hpp"
#include "Krystal.HTML/HTML/HTMLButtonElement.hpp"
#include "Krystal.HTML/HTML/HTMLCanvasElement.hpp"
#include "Krystal.HTML/HTML/HTMLDataElement.hpp"
#include "Krystal.HTML/HTML/HTMLDataListElement.hpp"
#include "Krystal.HTML/HTML/HTMLDetailsElement.hpp"
#include "Krystal.HTML/HTML/HTMLDialogElement.hpp"
#include "Krystal.HTML/HTML/HTMLDivElement.hpp"
#include "Krystal.HTML/HTML/HTMLDListElement.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/HTML/HTMLEmbedElement.hpp"
#include "Krystal.HTML/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.HTML/HTML/HTMLFormElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadElement.hpp"
#include "Krystal.HTML/HTML/HTMLHeadingElement.hpp"
#include "Krystal.HTML/HTML/HTMLHRElement.hpp"
#include "Krystal.HTML/HTML/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTML/HTMLIFrameElement.hpp"
#include "Krystal.HTML/HTML/HTMLImageElement.hpp"
#include "Krystal.HTML/HTML/HTMLInputElement.hpp"
#include "Krystal.HTML/HTML/HTMLLabelElement.hpp"
#include "Krystal.HTML/HTML/HTMLLegendElement.hpp"
#include "Krystal.HTML/HTML/HTMLLIElement.hpp"
#include "Krystal.HTML/HTML/HTMLLinkElement.hpp"
#include "Krystal.HTML/HTML/HTMLMapElement.hpp"
#include "Krystal.HTML/HTML/HTMLMediaElement.hpp"
#include "Krystal.HTML/HTML/HTMLMenuElement.hpp"
#include "Krystal.HTML/HTML/HTMLMetaElement.hpp"
#include "Krystal.HTML/HTML/HTMLMeterElement.hpp"
#include "Krystal.HTML/HTML/HTMLModElement.hpp"
#include "Krystal.HTML/HTML/HTMLObjectElement.hpp"
#include "Krystal.HTML/HTML/HTMLOListElement.hpp"
#include "Krystal.HTML/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.HTML/HTML/HTMLOptionElement.hpp"
#include "Krystal.HTML/HTML/HTMLOutputElement.hpp"
#include "Krystal.HTML/HTML/HTMLParagraphElement.hpp"
#include "Krystal.HTML/HTML/HTMLPictureElement.hpp"
#include "Krystal.HTML/HTML/HTMLPreElement.hpp"
#include "Krystal.HTML/HTML/HTMLProgressElement.hpp"
#include "Krystal.HTML/HTML/HTMLQuoteElement.hpp"
#include "Krystal.HTML/HTML/HTMLScriptElement.hpp"
#include "Krystal.HTML/HTML/HTMLSelectedContentElement.hpp"
#include "Krystal.HTML/HTML/HTMLSelectElement.hpp"
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
#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.HTML/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.HTML/HTML/HTMLTimeElement.hpp"
#include "Krystal.HTML/HTML/HTMLTitleElement.hpp"
#include "Krystal.HTML/HTML/HTMLTrackElement.hpp"
#include "Krystal.HTML/HTML/HTMLUListElement.hpp"
#include "Krystal.HTML/HTML/HTMLUnknownElement.hpp"
#include "Krystal.HTML/HTML/HTMLVideoElement.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  RefPtr<HTMLElement> HTMLElementFactory::TryCreate(Document &document, ElementName elementName) noexcept
  {
    switch (elementName)
    {
      case ElementName::abbr:
      case ElementName::address:
      case ElementName::article:
      case ElementName::aside:
      case ElementName::b:
      case ElementName::bdi:
      case ElementName::bdo:
      case ElementName::cite:
      case ElementName::code:
      case ElementName::dd:
      case ElementName::dfn:
      case ElementName::dt:
      case ElementName::em:
      case ElementName::figure:
      case ElementName::figcaption:
      case ElementName::footer:
      case ElementName::hgroup:
      case ElementName::header:
      case ElementName::i:
      case ElementName::kbd:
      case ElementName::main:
      case ElementName::mark:
      case ElementName::nav:
      case ElementName::noscript:
      case ElementName::ruby:
      case ElementName::rp:
      case ElementName::rt:
      case ElementName::s:
      case ElementName::samp:
      case ElementName::search:
      case ElementName::section:
      case ElementName::small:
      case ElementName::strong:
      case ElementName::sub:
      case ElementName::summary:
      case ElementName::sup:
      case ElementName::u:
      case ElementName::var:
      case ElementName::wbr:
      {
        return CreateRefPtr<HTMLElement>(document);
      }
      case ElementName::a:
      {
        return CreateRefPtr<HTMLAnchorElement>(document);
      }
      case ElementName::area:
      {
        return CreateRefPtr<HTMLAreaElement>(document);
      }
      case ElementName::audio:
      {
        return CreateRefPtr<HTMLAudioElement>(document);
      }
      case ElementName::base:
      {
        return CreateRefPtr<HTMLBaseElement>(document);
      }
      case ElementName::blockquote:
      case ElementName::q:
      {
        return CreateRefPtr<HTMLQuoteElement>(document);
      }
      case ElementName::body:
      {
        return CreateRefPtr<HTMLBodyElement>(document);
      }
      case ElementName::button:
      {
        return CreateRefPtr<HTMLButtonElement>(document);
      }
      case ElementName::canvas:
      {
        return CreateRefPtr<HTMLCanvasElement>(document);
      }
      case ElementName::caption:
      {
        return CreateRefPtr<HTMLTableCaptionElement>(document);
      }
      case ElementName::col:
      case ElementName::colgroup:
      {
        return CreateRefPtr<HTMLTableColElement>(document);
      }
      case ElementName::data:
      {
        return CreateRefPtr<HTMLDataElement>(document);
      }
      case ElementName::datalist:
      {
        return CreateRefPtr<HTMLDataListElement>(document);
      }
      case ElementName::del:
      case ElementName::ins:
      {
        return CreateRefPtr<HTMLModElement>(document);
      }
      case ElementName::details:
      {
        return CreateRefPtr<HTMLDetailsElement>(document);
      }
      case ElementName::dialog:
      {
        return CreateRefPtr<HTMLDialogElement>(document);
      }
      case ElementName::div:
      {
        return CreateRefPtr<HTMLDivElement>(document);
      }
      case ElementName::dl:
      {
        return CreateRefPtr<HTMLDListElement>(document);
      }
      case ElementName::embed:
      {
        return CreateRefPtr<HTMLEmbedElement>(document);
      }
      case ElementName::fieldset:
      {
        return CreateRefPtr<HTMLFieldSetElement>(document);
      }
      case ElementName::form:
      {
        return CreateRefPtr<HTMLFormElement>(document);
      }
      case ElementName::head:
      {
        return CreateRefPtr<HTMLHeadElement>(document);
      }
      case ElementName::h1:
      case ElementName::h2:
      case ElementName::h3:
      case ElementName::h4:
      case ElementName::h5:
      case ElementName::h6:
      {
        return CreateRefPtr<HTMLHeadingElement>(document);
      }
      case ElementName::hr:
      {
        return CreateRefPtr<HTMLHRElement>(document);
      }
      case ElementName::html:
      {
        return CreateRefPtr<HTMLHtmlElement>(document);
      }
      case ElementName::iframe:
      {
        return CreateRefPtr<HTMLIFrameElement>(document);
      }
      case ElementName::input:
      {
        return CreateRefPtr<HTMLInputElement>(document);
      }
      case ElementName::img:
      {
        return CreateRefPtr<HTMLImageElement>(document);
      }
      case ElementName::label:
      {
        return CreateRefPtr<HTMLLabelElement>(document);
      }
      case ElementName::legend:
      {
        return CreateRefPtr<HTMLLegendElement>(document);
      }
      case ElementName::li:
      {
        return CreateRefPtr<HTMLLIElement>(document);
      }
      case ElementName::link:
      {
        return CreateRefPtr<HTMLLinkElement>(document);
      }
      case ElementName::map:
      {
        return CreateRefPtr<HTMLMapElement>(document);
      }
      case ElementName::menu:
      {
        return CreateRefPtr<HTMLMenuElement>(document);
      }
      case ElementName::meta:
      {
        return CreateRefPtr<HTMLMetaElement>(document);
      }
      case ElementName::meter:
      {
        return CreateRefPtr<HTMLMeterElement>(document);
      }
      case ElementName::object:
      {
        return CreateRefPtr<HTMLObjectElement>(document);
      }
      case ElementName::ol:
      {
        return CreateRefPtr<HTMLOListElement>(document);
      }
      case ElementName::optgroup:
      {
        return CreateRefPtr<HTMLOptGroupElement>(document);
      }
      case ElementName::option:
      {
        return CreateRefPtr<HTMLOptionElement>(document);
      }
      case ElementName::output:
      {
        return CreateRefPtr<HTMLOutputElement>(document);
      }
      case ElementName::p:
      {
        return CreateRefPtr<HTMLParagraphElement>(document);
      }
      case ElementName::picture:
      {
        return CreateRefPtr<HTMLPictureElement>(document);
      }
      case ElementName::pre:
      {
        return CreateRefPtr<HTMLPreElement>(document);
      }
      case ElementName::progress:
      {
        return CreateRefPtr<HTMLProgressElement>(document);
      }
      case ElementName::script:
      {
        return CreateRefPtr<HTMLScriptElement>(document);
      }
      case ElementName::select:
      {
        return CreateRefPtr<HTMLSelectElement>(document);
      }
      case ElementName::selectedcontent:
      {
        return CreateRefPtr<HTMLSelectedContentElement>(document);
      }
      case ElementName::slot:
      {
        return CreateRefPtr<HTMLSlotElement>(document);
      }
      case ElementName::source:
      {
        return CreateRefPtr<HTMLSourceElement>(document);
      }
      case ElementName::span:
      {
        return CreateRefPtr<HTMLSpanElement>(document);
      }
      case ElementName::style:
      {
        return CreateRefPtr<HTMLStyleElement>(document);
      }
      case ElementName::table:
      {
        return CreateRefPtr<HTMLTableElement>(document);
      }
      case ElementName::tbody:
      case ElementName::thead:
      case ElementName::tfoot:
      {
        return CreateRefPtr<HTMLTableSectionElement>(document);
      }
      case ElementName::td:
      case ElementName::th:
      {
        return CreateRefPtr<HTMLTableCellElement>(document);
      }
      case ElementName::template_:
      {
        return CreateRefPtr<HTMLTemplateElement>(document);
      }
      case ElementName::tr:
      {
        return CreateRefPtr<HTMLTableRowElement>(document);
      }
      case ElementName::textarea:
      {
        return CreateRefPtr<HTMLTextAreaElement>(document);
      }
      case ElementName::title:
      {
        return CreateRefPtr<HTMLTitleElement>(document);
      }
      case ElementName::time:
      {
        return CreateRefPtr<HTMLTimeElement>(document);
      }
      case ElementName::track:
      {
        return CreateRefPtr<HTMLTrackElement>(document);
      }
      case ElementName::ul:
      {
        return CreateRefPtr<HTMLUListElement>(document);
      }
      case ElementName::video:
      {
        return CreateRefPtr<HTMLVideoElement>(document);
      }
#pragma region Obsolete elements
      case ElementName::applet:
      case ElementName::bgsound:
      case ElementName::blink:
      case ElementName::isindex:
      case ElementName::keygen:
      case ElementName::multicol:
      case ElementName::nextid:
      case ElementName::spacer:
      {
        return CreateRefPtr<HTMLUnknownElement>(document);
      }
      case ElementName::acronym:
      case ElementName::basefont:
      case ElementName::big:
      case ElementName::center:
      case ElementName::nobr:
      case ElementName::noembed:
      case ElementName::noframes:
      case ElementName::plaintext:
      case ElementName::rb:
      case ElementName::rtc:
      case ElementName::strike:
      case ElementName::tt:
      {
        return CreateRefPtr<HTMLElement>(document);
      }
      case ElementName::listing:
      case ElementName::xmp:
      {
        return CreateRefPtr<HTMLPreElement>(document);
      }
#pragma endregion
    }

    return nullptr;
  }
}