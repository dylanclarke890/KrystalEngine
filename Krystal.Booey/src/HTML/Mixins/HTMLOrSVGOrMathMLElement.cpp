#include "Krystal.Booey/HTML/Mixins/HTMLOrSVGOrMathMLElement.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include <string>

namespace krys::boo::html::mixins
{
  int32 HTMLOrSVGOrMathMLElement::TabIndex(const dom::Element &element) noexcept
  {
    // Let attribute be this's tabindex attribute. If attribute is not null, and its value parses as an
    // integer within the long range, return it.
    if (auto value = FocusAlgorithms::TabIndexValue(element))
    {
      return *value;
    }

    // Return 0 if this is an a, area, button, frame, iframe, input, object, select, textarea, or SVG a
    // element, or MathML a element, or is a summary element that is a summary for its parent details;
    // otherwise −1.
    auto localName = element.LocalName().View();
    auto namespaceURI = element.NamespaceURI().View();
    if (Is<HTMLElement>(element))
    {
      switch (element.TagQualifiedName().TagName())
      {
        case TagName::a:
        case TagName::area:
        case TagName::button:
        case TagName::frame:
        case TagName::iframe:
        case TagName::input:
        case TagName::object:
        case TagName::select:
        case TagName::textarea: return 0;
        case TagName::summary:
        {
          auto *parent = element.ParentElement();
          if (!parent || !Is<HTMLElement>(*parent)
              || parent->TagQualifiedName().TagName() != TagName::details)
          {
            return -1;
          }
          for (auto *child = parent->FirstChild(); child; child = child->NextSibling())
          {
            if (auto *summary = DynamicDowncast<HTMLElement>(*child);
                summary && summary->TagQualifiedName().TagName() == TagName::summary)
            {
              return summary == &element ? 0 : -1;
            }
          }
          return -1;
        }
        default: return -1;
      }
    }
    if (localName == u8"a"
        && (namespaceURI == infra::Namespaces::SVG || namespaceURI == infra::Namespaces::MathML))
    {
      return 0;
    }
    return -1;
  }

  void HTMLOrSVGOrMathMLElement::TabIndex(dom::Element &element, int32 value) noexcept
  {
    CEReactionsScope ceReactions;
    auto string = std::to_string(value);
    dom::ElementAlgorithms::SetAttributeValue(element, u8"tabindex",
                                              dom::DOMString(string.begin(), string.end()));
  }

  bool HTMLOrSVGOrMathMLElement::Autofocus(const dom::Element &element) noexcept
  {
    return element.HasAttribute(u8"autofocus");
  }

  void HTMLOrSVGOrMathMLElement::Autofocus(dom::Element &element, bool value) noexcept
  {
    if (value)
    {
      dom::ElementAlgorithms::SetAttributeValue(element, u8"autofocus", u8"");
    }
    else
    {
      element.RemoveAttribute(u8"autofocus");
    }
  }

  void HTMLOrSVGOrMathMLElement::Focus(dom::Element &element, const dom::FocusOptions &options) noexcept
  {
    // FIXME: The allow focus steps (the focus-without-user-activation feature and transient activation), once
    // there are permissions policies and user activation; until then focus is always allowed.

    // Focusability depends on style.
    element.NodeDocument().UpdateStyleIfNeeded();

    // Whether to indicate focus: as the options say, or (by the engine's heuristic) unless the user's latest
    // focus change was a click.
    auto withVisibility = options;
    withVisibility.Trigger = dom::FocusTrigger::Bindings;
    if (options.FocusVisible)
    {
      withVisibility.Visibility =
        *options.FocusVisible ? dom::FocusVisibility::Visible : dom::FocusVisibility::ForceInvisible;
    }
    else if (element.NodeDocument().WasLastFocusByClick())
    {
      withVisibility.Visibility = dom::FocusVisibility::Invisible;
    }
    else
    {
      withVisibility.Visibility = dom::FocusVisibility::Visible;
    }

    FocusAlgorithms::FocusingSteps(element, nullptr, withVisibility);

    if (RefPtr<dom::Element> focused = ShareRefPtr(element.NodeDocument().FocusedElement()))
    {
      focused->UpdateFocusAppearance(options.SelectionRestoration, options.PreventScroll
                                                                     ? SelectionRevealMode::DoNotReveal
                                                                     : SelectionRevealMode::Reveal);
    }
  }

  void HTMLOrSVGOrMathMLElement::Blur(dom::Element &element) noexcept
  {
    FocusAlgorithms::UnfocusingSteps(element);
  }
}
