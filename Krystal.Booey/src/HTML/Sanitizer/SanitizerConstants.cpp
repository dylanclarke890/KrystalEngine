#include "Krystal.Booey/HTML/Sanitizer/SanitizerConstants.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo::html
{
  namespace
  {
    struct SanitizerDefaultElement
    {
      utf8_stringview Name;
      /// @brief The attributes' names, separated by spaces.
      utf8_stringview Attributes;
    };

    // The HTML elements whose definitions categorize them as Default, with the attributes the definitions
    // list. https://html.spec.whatwg.org/#concept-element-sanitization
    constexpr SanitizerDefaultElement DefaultHTMLElements[] = {
      {u8"html", u8""},
      {u8"head", u8""},
      {u8"title", u8""},
      {u8"body", u8""},
      {u8"article", u8""},
      {u8"section", u8""},
      {u8"nav", u8""},
      {u8"aside", u8""},
      {u8"h1", u8""},
      {u8"h2", u8""},
      {u8"h3", u8""},
      {u8"h4", u8""},
      {u8"h5", u8""},
      {u8"h6", u8""},
      {u8"hgroup", u8""},
      {u8"header", u8""},
      {u8"footer", u8""},
      {u8"address", u8""},
      {u8"p", u8""},
      {u8"hr", u8""},
      {u8"pre", u8""},
      {u8"blockquote", u8"cite"},
      {u8"ol", u8"reversed start type"},
      {u8"ul", u8""},
      {u8"menu", u8""},
      {u8"li", u8"value"},
      {u8"dl", u8""},
      {u8"dt", u8""},
      {u8"dd", u8""},
      {u8"figure", u8""},
      {u8"figcaption", u8""},
      {u8"main", u8""},
      {u8"search", u8""},
      {u8"div", u8""},
      {u8"a", u8"href hreflang type"},
      {u8"em", u8""},
      {u8"strong", u8""},
      {u8"small", u8""},
      {u8"s", u8""},
      {u8"cite", u8""},
      {u8"q", u8""},
      {u8"dfn", u8""},
      {u8"abbr", u8""},
      {u8"ruby", u8""},
      {u8"rt", u8""},
      {u8"rp", u8""},
      {u8"data", u8"value"},
      {u8"time", u8"datetime"},
      {u8"code", u8""},
      {u8"var", u8""},
      {u8"samp", u8""},
      {u8"kbd", u8""},
      {u8"sub", u8""},
      {u8"sup", u8""},
      {u8"i", u8""},
      {u8"b", u8""},
      {u8"u", u8""},
      {u8"mark", u8""},
      {u8"bdi", u8""},
      {u8"bdo", u8""},
      {u8"span", u8""},
      {u8"br", u8""},
      {u8"wbr", u8""},
      {u8"ins", u8"cite datetime"},
      {u8"del", u8"cite datetime"},
      {u8"table", u8""},
      {u8"caption", u8""},
      {u8"colgroup", u8"span"},
      {u8"col", u8"span"},
      {u8"tbody", u8""},
      {u8"thead", u8""},
      {u8"tfoot", u8""},
      {u8"tr", u8""},
      {u8"td", u8"colspan headers rowspan"},
      {u8"th", u8"abbr colspan headers rowspan scope"},
    };

    // The MathML elements of the built-in safe default configuration's table.
    constexpr SanitizerDefaultElement DefaultMathMLElements[] = {
      {u8"a", u8"href hreflang type"},
      {u8"math", u8""},
      {u8"merror", u8""},
      {u8"mfrac", u8""},
      {u8"mi", u8""},
      {u8"mmultiscripts", u8""},
      {u8"mn", u8""},
      {u8"mo",
       u8"fence form largeop lspace maxsize minsize movablelimits rspace separator stretchy symmetric"},
      {u8"mover", u8"accent"},
      {u8"mpadded", u8"depth height lspace voffset width"},
      {u8"mphantom", u8""},
      {u8"mprescripts", u8""},
      {u8"mroot", u8""},
      {u8"mrow", u8""},
      {u8"ms", u8""},
      {u8"mspace", u8"depth height width"},
      {u8"msqrt", u8""},
      {u8"mstyle", u8""},
      {u8"msub", u8""},
      {u8"msubsup", u8""},
      {u8"msup", u8""},
      {u8"mtable", u8""},
      {u8"mtd", u8"columnspan rowspan"},
      {u8"mtext", u8""},
      {u8"mtr", u8""},
      {u8"munder", u8"accentunder"},
      {u8"munderover", u8"accent accentunder"},
      {u8"semantics", u8""},
    };

    // The SVG elements of the built-in safe default configuration's table.
    constexpr SanitizerDefaultElement DefaultSVGElements[] = {
      {u8"a", u8"href hreflang type"},
      {u8"circle", u8"cx cy pathLength r"},
      {u8"defs", u8""},
      {u8"desc", u8""},
      {u8"ellipse", u8"cx cy pathLength rx ry"},
      {u8"foreignObject", u8"height width x y"},
      {u8"g", u8""},
      {u8"line", u8"pathLength x1 x2 y1 y2"},
      {u8"marker", u8"markerHeight markerUnits markerWidth orient preserveAspectRatio refX refY viewBox"},
      {u8"metadata", u8""},
      {u8"path", u8"d pathLength"},
      {u8"polygon", u8"pathLength points"},
      {u8"polyline", u8"pathLength points"},
      {u8"rect", u8"height pathLength rx ry width x y"},
      {u8"svg", u8"height preserveAspectRatio viewBox width x y"},
      {u8"text", u8"dx dy lengthAdjust rotate textLength x y"},
      {u8"textPath", u8"lengthAdjust method path side spacing startOffset textLength"},
      {u8"title", u8""},
      {u8"tspan", u8"dx dy lengthAdjust rotate textLength x y"},
    };

    // The built-in safe default configuration's attributes.
    constexpr utf8_stringview DefaultGlobalAttributes[] = {
      u8"dir",
      u8"lang",
      u8"title",
      u8"alignment-baseline",
      u8"baseline-shift",
      u8"clip-path",
      u8"clip-rule",
      u8"color",
      u8"color-interpolation",
      u8"cursor",
      u8"direction",
      u8"display",
      u8"displaystyle",
      u8"dominant-baseline",
      u8"fill",
      u8"fill-opacity",
      u8"fill-rule",
      u8"font-family",
      u8"font-size",
      u8"font-size-adjust",
      u8"font-stretch",
      u8"font-style",
      u8"font-variant",
      u8"font-weight",
      u8"letter-spacing",
      u8"marker-end",
      u8"marker-mid",
      u8"marker-start",
      u8"mathbackground",
      u8"mathcolor",
      u8"mathsize",
      u8"opacity",
      u8"paint-order",
      u8"pointer-events",
      u8"scriptlevel",
      u8"shape-rendering",
      u8"stop-color",
      u8"stop-opacity",
      u8"stroke",
      u8"stroke-dasharray",
      u8"stroke-dashoffset",
      u8"stroke-linecap",
      u8"stroke-linejoin",
      u8"stroke-miterlimit",
      u8"stroke-opacity",
      u8"stroke-width",
      u8"text-anchor",
      u8"text-decoration",
      u8"text-overflow",
      u8"text-rendering",
      u8"transform",
      u8"transform-origin",
      u8"unicode-bidi",
      u8"vector-effect",
      u8"visibility",
      u8"white-space",
      u8"word-spacing",
      u8"writing-mode",
    };

#define KRYS_SANITIZER_EVENT_HANDLER_NAME(Name, name) u8"on" name,

    // The event handler content attributes: those of the event handlers Booey has (HTML's, with the
    // animation, transition, selection and gamepad ones), then those of other specifications it doesn't have
    // yet.
    constexpr utf8_stringview EventHandlerNames[] = {
      KRYS_GLOBAL_EVENT_HANDLERS(KRYS_SANITIZER_EVENT_HANDLER_NAME)
        KRYS_WINDOW_EVENT_HANDLERS(KRYS_SANITIZER_EVENT_HANDLER_NAME)
          KRYS_DOCUMENT_AND_ELEMENT_EVENT_HANDLERS(KRYS_SANITIZER_EVENT_HANDLER_NAME)
      // Pointer Events, Touch Events and Pointer Lock.
      u8"ongotpointercapture",
      u8"onlostpointercapture",
      u8"onpointerdown",
      u8"onpointermove",
      u8"onpointerrawupdate",
      u8"onpointerup",
      u8"onpointercancel",
      u8"onpointerover",
      u8"onpointerout",
      u8"onpointerenter",
      u8"onpointerleave",
      u8"ontouchstart",
      u8"ontouchend",
      u8"ontouchmove",
      u8"ontouchcancel",
      u8"onpointerlockchange",
      u8"onpointerlockerror",
      // Fullscreen.
      u8"onfullscreenchange",
      u8"onfullscreenerror",
      // SVG's animation event attributes.
      u8"onbegin",
      u8"onend",
      u8"onrepeat",
    };

#undef KRYS_SANITIZER_EVENT_HANDLER_NAME

    KRYS_NODISCARD SanitizerElementNamespaceWithAttributes
      DefaultSanitizerElement(const SanitizerDefaultElement &element, utf8_stringview namespaceURI) noexcept
    {
      SanitizerElementNamespaceWithAttributes result {dom::DOMString(element.Name),
                                                      dom::DOMStringAtom(namespaceURI)};
      result.Attributes.emplace();
      for (auto attribute : std::views::split(element.Attributes, u8' '))
      {
        result.Attributes->emplace_back(dom::DOMString(attribute.begin(), attribute.end()));
      }
      return result;
    }

    KRYS_NODISCARD bool IsSanitizerPair(const SanitizerElementNamespace &element,
                                        const SanitizerAttributeNamespace &attribute,
                                        utf8_stringview elementName, utf8_stringview elementNamespace,
                                        utf8_stringview attributeName,
                                        dom::DOMStringAtom attributeNamespace) noexcept
    {
      return element.Name == elementName && element.Namespace == dom::DOMStringAtom(elementNamespace)
             && attribute.Name == attributeName && attribute.Namespace == attributeNamespace;
    }
  }

  const SanitizerConfig &SanitizerConstants::BuiltInSafeDefaultConfiguration() noexcept
  {
    static NeverDestroyed<SanitizerConfig> configuration {
      []
      {
        SanitizerConfig config;

        // SPEC: processingInstructions: « »
        config.ProcessingInstructions.emplace();

        // SPEC: attributes: a list consisting of: [...]
        config.Attributes.emplace();
        for (auto name : DefaultGlobalAttributes)
        {
          config.Attributes->emplace_back(dom::DOMString(name));
        }

        // SPEC: comments: false; dataAttributes: false; javascriptURLs: false.
        config.Comments = false;
        config.DataAttributes = false;
        config.JavascriptURLs = false;

        // SPEC: elements: all HTML elements normatively categorized as Default within their individual
        // definitions, alongside the MathML and SVG elements listed in the table below.
        config.Elements.emplace();
        for (auto &element : DefaultHTMLElements)
        {
          config.Elements->push_back(DefaultSanitizerElement(element, infra::Namespaces::HTML));
        }
        for (auto &element : DefaultMathMLElements)
        {
          config.Elements->push_back(DefaultSanitizerElement(element, infra::Namespaces::MathML));
        }
        for (auto &element : DefaultSVGElements)
        {
          config.Elements->push_back(DefaultSanitizerElement(element, infra::Namespaces::SVG));
        }
        return config;
      }()};
    return configuration.get();
  }

  const SanitizerConfig &SanitizerConstants::BuiltInSafeBaselineConfiguration() noexcept
  {
    static NeverDestroyed<SanitizerConfig> configuration {
      []
      {
        // SPEC: Its removeElements list consists of all HTML elements normatively marked as Unsafe within
        // their individual definitions, along with the obsolete frame element, and the SVG script and SVG use
        // elements, and its removeAttributes list is empty.
        SanitizerConfig config;
        config.RemoveElements = List<SanitizerElementNamespace> {
          u8"base",
          u8"embed",
          u8"frame",
          u8"iframe",
          u8"object",
          u8"script",
          {u8"script", dom::DOMStringAtom(infra::Namespaces::SVG)},
          {u8"use", dom::DOMStringAtom(infra::Namespaces::SVG)},
        };
        config.RemoveAttributes.emplace();
        return config;
      }()};
    return configuration.get();
  }

  Span<const SanitizerElementNamespace> SanitizerConstants::BuiltInNonReplaceableElements() noexcept
  {
    static NeverDestroyed<List<SanitizerElementNamespace>> elements {List<SanitizerElementNamespace> {
      u8"html",
      {u8"svg", dom::DOMStringAtom(infra::Namespaces::SVG)},
      {u8"math", dom::DOMStringAtom(infra::Namespaces::MathML)},
    }};
    return elements.get();
  }

  bool SanitizerConstants::IsNavigatingURLAttribute(const SanitizerElementNamespace &element,
                                                    const SanitizerAttributeNamespace &attribute) noexcept
  {
    auto none = dom::DOMStringAtom::Null();
    auto xlink = dom::DOMStringAtom(infra::Namespaces::XLink);
    using infra::Namespaces::HTML;
    using infra::Namespaces::SVG;

    // SPEC: The built-in navigating URL attributes list corresponds to all HTML elements marked with
    // navigating URL attributes in their normative definitions, as well as the element-attribute pairs
    // represented in the following table.
    return IsSanitizerPair(element, attribute, u8"a", HTML, u8"href", none)
           || IsSanitizerPair(element, attribute, u8"area", HTML, u8"href", none)
           || IsSanitizerPair(element, attribute, u8"form", HTML, u8"action", none)
           || IsSanitizerPair(element, attribute, u8"input", HTML, u8"formaction", none)
           || IsSanitizerPair(element, attribute, u8"button", HTML, u8"formaction", none)
           || IsSanitizerPair(element, attribute, u8"a", SVG, u8"href", none)
           || IsSanitizerPair(element, attribute, u8"a", SVG, u8"href", xlink);
  }

  bool SanitizerConstants::IsAnimatingURLAttribute(const SanitizerElementNamespace &element,
                                                   const SanitizerAttributeNamespace &attribute) noexcept
  {
    auto none = dom::DOMStringAtom::Null();
    using infra::Namespaces::SVG;
    return IsSanitizerPair(element, attribute, u8"animate", SVG, u8"attributeName", none)
           || IsSanitizerPair(element, attribute, u8"animateTransform", SVG, u8"attributeName", none)
           || IsSanitizerPair(element, attribute, u8"set", SVG, u8"attributeName", none);
  }

  Span<const utf8_stringview> SanitizerConstants::EventHandlerContentAttributes() noexcept
  {
    return EventHandlerNames;
  }
}
