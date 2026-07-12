#include "Krystal.HTML/HTML/Internals/HTMLElementFactory.hpp"
#include "Krystal.HTML/Constants/TagNames.hpp"
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
  RefPtr<HTMLElement> HTMLElementFactory::TryCreate(Document &document, TagName elementName) noexcept
  {
    switch (elementName)
    {
      case TagName::abbr:
      case TagName::address:
      case TagName::article:
      case TagName::aside:
      case TagName::b:
      case TagName::bdi:
      case TagName::bdo:
      case TagName::cite:
      case TagName::code:
      case TagName::dd:
      case TagName::dfn:
      case TagName::dt:
      case TagName::em:
      case TagName::figure:
      case TagName::figcaption:
      case TagName::footer:
      case TagName::hgroup:
      case TagName::header:
      case TagName::i:
      case TagName::kbd:
      case TagName::main:
      case TagName::mark:
      case TagName::nav:
      case TagName::noscript:
      case TagName::ruby:
      case TagName::rp:
      case TagName::rt:
      case TagName::s:
      case TagName::samp:
      case TagName::search:
      case TagName::section:
      case TagName::small:
      case TagName::strong:
      case TagName::sub:
      case TagName::summary:
      case TagName::sup:
      case TagName::u:
      case TagName::var:
      case TagName::wbr:
      {
        return CreateRefPtr<HTMLElement>(document);
      }
      case TagName::a:
      {
        return CreateRefPtr<HTMLAnchorElement>(document);
      }
      case TagName::area:
      {
        return CreateRefPtr<HTMLAreaElement>(document);
      }
      case TagName::audio:
      {
        return CreateRefPtr<HTMLAudioElement>(document);
      }
      case TagName::base:
      {
        return CreateRefPtr<HTMLBaseElement>(document);
      }
      case TagName::blockquote:
      case TagName::q:
      {
        return CreateRefPtr<HTMLQuoteElement>(document);
      }
      case TagName::body:
      {
        return CreateRefPtr<HTMLBodyElement>(document);
      }
      case TagName::button:
      {
        return CreateRefPtr<HTMLButtonElement>(document);
      }
      case TagName::canvas:
      {
        return CreateRefPtr<HTMLCanvasElement>(document);
      }
      case TagName::caption:
      {
        return CreateRefPtr<HTMLTableCaptionElement>(document);
      }
      case TagName::col:
      case TagName::colgroup:
      {
        return CreateRefPtr<HTMLTableColElement>(document);
      }
      case TagName::data:
      {
        return CreateRefPtr<HTMLDataElement>(document);
      }
      case TagName::datalist:
      {
        return CreateRefPtr<HTMLDataListElement>(document);
      }
      case TagName::del:
      case TagName::ins:
      {
        return CreateRefPtr<HTMLModElement>(document);
      }
      case TagName::details:
      {
        return CreateRefPtr<HTMLDetailsElement>(document);
      }
      case TagName::dialog:
      {
        return CreateRefPtr<HTMLDialogElement>(document);
      }
      case TagName::div:
      {
        return CreateRefPtr<HTMLDivElement>(document);
      }
      case TagName::dl:
      {
        return CreateRefPtr<HTMLDListElement>(document);
      }
      case TagName::embed:
      {
        return CreateRefPtr<HTMLEmbedElement>(document);
      }
      case TagName::fieldset:
      {
        return CreateRefPtr<HTMLFieldSetElement>(document);
      }
      case TagName::form:
      {
        return CreateRefPtr<HTMLFormElement>(document);
      }
      case TagName::head:
      {
        return CreateRefPtr<HTMLHeadElement>(document);
      }
      case TagName::h1:
      case TagName::h2:
      case TagName::h3:
      case TagName::h4:
      case TagName::h5:
      case TagName::h6:
      {
        return CreateRefPtr<HTMLHeadingElement>(document);
      }
      case TagName::hr:
      {
        return CreateRefPtr<HTMLHRElement>(document);
      }
      case TagName::html:
      {
        return CreateRefPtr<HTMLHtmlElement>(document);
      }
      case TagName::iframe:
      {
        return CreateRefPtr<HTMLIFrameElement>(document);
      }
      case TagName::input:
      {
        return CreateRefPtr<HTMLInputElement>(document);
      }
      case TagName::img:
      {
        return CreateRefPtr<HTMLImageElement>(document);
      }
      case TagName::label:
      {
        return CreateRefPtr<HTMLLabelElement>(document);
      }
      case TagName::legend:
      {
        return CreateRefPtr<HTMLLegendElement>(document);
      }
      case TagName::li:
      {
        return CreateRefPtr<HTMLLIElement>(document);
      }
      case TagName::link:
      {
        return CreateRefPtr<HTMLLinkElement>(document);
      }
      case TagName::map:
      {
        return CreateRefPtr<HTMLMapElement>(document);
      }
      case TagName::menu:
      {
        return CreateRefPtr<HTMLMenuElement>(document);
      }
      case TagName::meta:
      {
        return CreateRefPtr<HTMLMetaElement>(document);
      }
      case TagName::meter:
      {
        return CreateRefPtr<HTMLMeterElement>(document);
      }
      case TagName::object:
      {
        return CreateRefPtr<HTMLObjectElement>(document);
      }
      case TagName::ol:
      {
        return CreateRefPtr<HTMLOListElement>(document);
      }
      case TagName::optgroup:
      {
        return CreateRefPtr<HTMLOptGroupElement>(document);
      }
      case TagName::option:
      {
        return CreateRefPtr<HTMLOptionElement>(document);
      }
      case TagName::output:
      {
        return CreateRefPtr<HTMLOutputElement>(document);
      }
      case TagName::p:
      {
        return CreateRefPtr<HTMLParagraphElement>(document);
      }
      case TagName::picture:
      {
        return CreateRefPtr<HTMLPictureElement>(document);
      }
      case TagName::pre:
      {
        return CreateRefPtr<HTMLPreElement>(document);
      }
      case TagName::progress:
      {
        return CreateRefPtr<HTMLProgressElement>(document);
      }
      case TagName::script:
      {
        return CreateRefPtr<HTMLScriptElement>(document);
      }
      case TagName::select:
      {
        return CreateRefPtr<HTMLSelectElement>(document);
      }
      case TagName::selectedcontent:
      {
        return CreateRefPtr<HTMLSelectedContentElement>(document);
      }
      case TagName::slot:
      {
        return CreateRefPtr<HTMLSlotElement>(document);
      }
      case TagName::source:
      {
        return CreateRefPtr<HTMLSourceElement>(document);
      }
      case TagName::span:
      {
        return CreateRefPtr<HTMLSpanElement>(document);
      }
      case TagName::style:
      {
        return CreateRefPtr<HTMLStyleElement>(document);
      }
      case TagName::table:
      {
        return CreateRefPtr<HTMLTableElement>(document);
      }
      case TagName::tbody:
      case TagName::thead:
      case TagName::tfoot:
      {
        return CreateRefPtr<HTMLTableSectionElement>(document);
      }
      case TagName::td:
      case TagName::th:
      {
        return CreateRefPtr<HTMLTableCellElement>(document);
      }
      case TagName::template_:
      {
        return CreateRefPtr<HTMLTemplateElement>(document);
      }
      case TagName::tr:
      {
        return CreateRefPtr<HTMLTableRowElement>(document);
      }
      case TagName::textarea:
      {
        return CreateRefPtr<HTMLTextAreaElement>(document);
      }
      case TagName::title:
      {
        return CreateRefPtr<HTMLTitleElement>(document);
      }
      case TagName::time:
      {
        return CreateRefPtr<HTMLTimeElement>(document);
      }
      case TagName::track:
      {
        return CreateRefPtr<HTMLTrackElement>(document);
      }
      case TagName::ul:
      {
        return CreateRefPtr<HTMLUListElement>(document);
      }
      case TagName::video:
      {
        return CreateRefPtr<HTMLVideoElement>(document);
      }
#pragma region Obsolete elements
      case TagName::applet:
      case TagName::bgsound:
      case TagName::blink:
      case TagName::isindex:
      case TagName::keygen:
      case TagName::multicol:
      case TagName::nextid:
      case TagName::spacer:
      {
        return CreateRefPtr<HTMLUnknownElement>(document);
      }
      case TagName::acronym:
      case TagName::basefont:
      case TagName::big:
      case TagName::center:
      case TagName::nobr:
      case TagName::noembed:
      case TagName::noframes:
      case TagName::plaintext:
      case TagName::rb:
      case TagName::rtc:
      case TagName::strike:
      case TagName::tt:
      {
        return CreateRefPtr<HTMLElement>(document);
      }
      case TagName::listing:
      case TagName::xmp:
      {
        return CreateRefPtr<HTMLPreElement>(document);
      }
#pragma endregion
    }

    return nullptr;
  }
}