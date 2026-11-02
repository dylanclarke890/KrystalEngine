#include "Krystal.Booey/HTML/Mixins/HTMLOrSVGOrMathMLElement.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLSummaryElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include <string>

namespace krys::boo::html::mixins
{
  // https://html.spec.whatwg.org/#dom-tabindex
  int32 HTMLOrSVGOrMathMLElement::TabIndex(const dom::Element &element) noexcept
  {
    // 1. Let attribute be this's tabindex attribute.
    // 2. If attribute is not null:
    // 2.1. Let parsedValue be the result of integer parsing attribute's value.
    // 2.2. If parsedValue is not an error and is within the long range, then return parsedValue.
    if (auto value = FocusAlgorithms::TabIndexValue(element))
    {
      return *value;
    }

    // 3. Return 0 if this is an a, area, button, frame, iframe, input, object, select, textarea, or SVG a
    //    element, or MathML a element, or is a summary element that is a summary for its parent details;
    //    otherwise −1.
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
          auto *summary = DynamicDowncast<HTMLSummaryElement>(element);
          return summary && summary->IsActiveSummary() ? 0 : -1;
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

  // https://html.spec.whatwg.org/#dom-focus
  void HTMLOrSVGOrMathMLElement::Focus(dom::Element &element, const dom::FocusOptions &options) noexcept
  {
    // 1. If the allow focus steps given this's node document return false, then return.
    // https://html.spec.whatwg.org/#allow-focus-steps
    // (The "focus-without-user-activation" feature's default allowlist is *, and without frames or
    // permissions policies every document is allowed to use it, so focus is always allowed.)

    // Focusability depends on style.
    element.NodeDocument().UpdateStyleIfNeeded();

    // 3. If options["focusVisible"] is true, or does not exist but in an implementation-defined way the user
    //    agent determines it would be best to do so, then indicate focus.
    // (The heuristic indicates it unless the user's latest focus change was a click.)
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

    // 2. Run the focusing steps for this.
    RefPtr<dom::Element> previouslyFocused = ShareRefPtr(element.NodeDocument().FocusedElement());
    FocusAlgorithms::FocusingSteps(element, nullptr, withVisibility);

    // 4. If options["preventScroll"] is false, then scroll a target into view given this, "auto", "center",
    //    and "center".
    // (What got the focus is shown focused: scrolled into view, or with its contents selected. That's this
    // or its focus delegate; nothing is shown if this couldn't be focused.)
    if (RefPtr<dom::Element> focused = ShareRefPtr(element.NodeDocument().FocusedElement());
        focused && (focused == &element || focused != previouslyFocused))
    {
      focused->FindTargetAndUpdateFocusAppearance(options.SelectionRestoration,
                                                  options.PreventScroll ? SelectionRevealMode::DoNotReveal
                                                                        : SelectionRevealMode::Reveal);
    }
  }

  // https://html.spec.whatwg.org/#dom-blur
  // The user agent should run the unfocusing steps given this.
  void HTMLOrSVGOrMathMLElement::Blur(dom::Element &element) noexcept
  {
    FocusAlgorithms::UnfocusingSteps(element);
  }
}
