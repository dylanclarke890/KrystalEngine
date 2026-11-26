#include "Krystal.Web/Html/Mixins/HTMLOrSVGOrMathMLElement.hpp"
#include "Krystal.Web/Css/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Web/Dom/Nodes/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Html/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include <string>

namespace krys::web::html::mixins
{
  // https://html.spec.whatwg.org/multipage/interaction.html#dom-tabindex
  int32 HTMLOrSVGOrMathMLElement::TabIndex(const Element &element) noexcept
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
        && (namespaceURI == infra::namespaces::SVG || namespaceURI == infra::namespaces::MathML))
    {
      return 0;
    }
    return -1;
  }

  void HTMLOrSVGOrMathMLElement::TabIndex(Element &element, int32 value) noexcept
  {
    CEReactionsScope ceReactions;
    auto string = std::to_string(value);
    dom::ElementAlgorithms::SetAttributeValue(element, u8"tabindex", DOMString(string.begin(), string.end()));
  }

  bool HTMLOrSVGOrMathMLElement::Autofocus(const Element &element) noexcept
  {
    return element.HasAttribute(u8"autofocus");
  }

  void HTMLOrSVGOrMathMLElement::Autofocus(Element &element, bool value) noexcept
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

  // https://html.spec.whatwg.org/multipage/interaction.html#dom-focus
  void HTMLOrSVGOrMathMLElement::Focus(Element &element, const html::FocusOptions &options) noexcept
  {
    // 1. If the allow focus steps given this's node document return false, then return.
    // https://html.spec.whatwg.org/multipage/interaction.html#allow-focus-steps
    // (The "focus-without-user-activation" feature's default allowlist is *, and without frames or
    // permissions policies every document is allowed to use it, so focus is always allowed.)

    // Focusability depends on style.
    element.NodeDocument().UpdateStyleIfNeeded();

    // 3. If options["focusVisible"] is true, or does not exist but in an implementation-defined way the user
    //    agent determines it would be best to do so, then indicate focus.
    // (The heuristic indicates it unless the user's latest focus change was a click.)
    auto withVisibility = options;
    withVisibility.Trigger = html::FocusTrigger::Bindings;
    if (options.FocusVisible)
    {
      withVisibility.Visibility =
        *options.FocusVisible ? html::FocusVisibility::Visible : html::FocusVisibility::ForceInvisible;
    }
    else if (element.NodeDocument().WasLastFocusByClick())
    {
      withVisibility.Visibility = html::FocusVisibility::Invisible;
    }
    else
    {
      withVisibility.Visibility = html::FocusVisibility::Visible;
    }

    // 2. Run the focusing steps for this.
    RefPtr<Element> previouslyFocused = ShareRefPtr(element.NodeDocument().FocusedElement());
    FocusAlgorithms::FocusingSteps(element, nullptr, withVisibility);

    // 4. If options["preventScroll"] is false, then scroll a target into view given this, "auto", "center",
    //    and "center".
    // (What got the focus is shown focused: scrolled into view, or with its contents selected. That's this
    // or its focus delegate; nothing is shown if this couldn't be focused.)
    if (RefPtr<Element> focused = ShareRefPtr(element.NodeDocument().FocusedElement());
        focused && (focused == &element || focused != previouslyFocused))
    {
      focused->FindTargetAndUpdateFocusAppearance(options.SelectionRestoration,
                                                  options.PreventScroll ? SelectionRevealMode::DoNotReveal
                                                                        : SelectionRevealMode::Reveal);
    }
  }

  // https://html.spec.whatwg.org/multipage/interaction.html#dom-blur
  // The user agent should run the unfocusing steps given this.
  void HTMLOrSVGOrMathMLElement::Blur(Element &element) noexcept
  {
    FocusAlgorithms::UnfocusingSteps(element);
  }
}
