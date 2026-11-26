#include "Krystal.Booey/Html/Internals/HTMLElementFactory.hpp"
#include "Krystal.Booey/Dom/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Html/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/Html/Elements/HTMLAnchorElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLAudioElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLBaseElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLBDIElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLBRElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLDataElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLDetailsElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLDirectoryElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLDivElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLDListElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLEmbedElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLFontElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLFrameSetElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLHeadingElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLHRElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLIFrameElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLLabelElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLLegendElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLLIElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLMarqueeElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLMediaElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLMenuElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLMetaElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLMeterElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLModElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLObjectElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLOListElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLOutputElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLParagraphElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLParamElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLPictureElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLPreElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLQuoteElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLScriptElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSelectedContentElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSourceElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSpanElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLStyleElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTableCaptionElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTableCellElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTableColElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTemplateElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTimeElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTitleElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTrackElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLUListElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLUnknownElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLVideoElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLWBRElement.hpp"

namespace krys::boo::html
{
  RefPtr<HTMLElement> HTMLElementFactory::TryCreate(Document &document, TagName elementName) noexcept
  {
    // 4. Otherwise, if this specification defines an interface appropriate for the element type corresponding
    //    to the local name name, then return that interface.
    // 5. If other applicable specifications define an appropriate interface for name, then return the
    //    interface they define.
    // (Steps 1 to 3 are the obsolete elements at the end; 6 and 7 are the caller's, for names with no
    // interface here.)
    switch (elementName)
    {
      case TagName::abbr:
      case TagName::address:
      case TagName::article:
      case TagName::aside:
      case TagName::b:
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
      case TagName::sup:
      case TagName::u:
      case TagName::var:
      {
        return CreateRefPtr<HTMLElement>(document);
      }
      case TagName::bdi:
      {
        return CreateRefPtr<HTMLBDIElement>(document);
      }
      case TagName::wbr:
      {
        return CreateRefPtr<HTMLWBRElement>(document);
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
        return HTMLDetailsElement::Create(document);
      }
      case TagName::dialog:
      {
        return CreateRefPtr<HTMLDialogElement>(document);
      }
      case TagName::dir:
      {
        return CreateRefPtr<HTMLDirectoryElement>(document);
      }
      case TagName::br:
      {
        return CreateRefPtr<HTMLBRElement>(document);
      }
      case TagName::font:
      {
        return CreateRefPtr<HTMLFontElement>(document);
      }
      case TagName::frameset:
      {
        return CreateRefPtr<HTMLFrameSetElement>(document);
      }
      case TagName::marquee:
      {
        return HTMLMarqueeElement::Create(document);
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
        return HTMLMeterElement::Create(document);
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
        return HTMLOptGroupElement::Create(document);
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
      case TagName::param:
      {
        return CreateRefPtr<HTMLParamElement>(document);
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
        return HTMLProgressElement::Create(document);
      }
      case TagName::script:
      {
        return CreateRefPtr<HTMLScriptElement>(document);
      }
      case TagName::select:
      {
        return HTMLSelectElement::Create(document);
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
      case TagName::summary:
      {
        return CreateRefPtr<HTMLSummaryElement>(document);
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
        return HTMLTextAreaElement::Create(document);
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

      // 1. If name is applet, bgsound, blink, isindex, keygen, multicol, nextid, or spacer, then return
      //    HTMLUnknownElement.
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
      // 2. If name is acronym, basefont, big, center, nobr, noembed, noframes, plaintext, rb, rtc, strike, or
      //    tt, then return HTMLElement.
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
      // 3. If name is listing or xmp, then return HTMLPreElement.
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
