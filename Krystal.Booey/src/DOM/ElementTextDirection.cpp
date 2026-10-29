#include "Krystal.Booey/DOM/ElementTextDirection.hpp"
#include "Krystal.Booey/Core/UnicodeHelpers.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLBDIElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::dom
{
  TextDirectionState ParseTextDirectionState(Maybe<DOMStringView> value) noexcept
  {
    if (!value)
    {
      return TextDirectionState::Undefined;
    }
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"ltr"))
    {
      return TextDirectionState::LTR;
    }
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"rtl"))
    {
      return TextDirectionState::RTL;
    }
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"auto"))
    {
      return TextDirectionState::Auto;
    }
    return TextDirectionState::Undefined;
  }

  TextDirectionState ElementTextDirectionState(const Element &element) noexcept
  {
    auto value = element.GetAttribute(u8"dir");
    return ParseTextDirectionState(value ? Maybe<DOMStringView>(*value) : null);
  }

  bool ElementHasValidTextDirectionState(const Element &element) noexcept
  {
    return ElementTextDirectionState(element) != TextDirectionState::Undefined;
  }

  bool ElementHasAutoTextDirectionState(const Element &element) noexcept
  {
    auto textDirectionState = ElementTextDirectionState(element);
    return textDirectionState == TextDirectionState::Auto
           || (Is<html::HTMLBDIElement>(element) && textDirectionState == TextDirectionState::Undefined);
  }

  namespace
  {
    KRYS_NODISCARD bool HasHTMLTagName(const Node &node, html::TagName tagName) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      return element && element->TagQualifiedName().TagName() == tagName;
    }

    void UpdateHasDirAutoFlagForSubtree(Element &firstNode, TextDirectionState textDirectionState) noexcept
    {
      bool dirIsAuto = textDirectionState == TextDirectionState::Auto;
      firstNode.SetSelfOrPrecedingNodesAffectDirAuto(dirIsAuto);

      for (auto *node = firstNode.FirstChild(); node;)
      {
        auto *element = DynamicDowncast<Element>(*node);

        if (element && (Is<html::HTMLBDIElement>(*element) || ElementHasValidTextDirectionState(*element)))
        {
          node = TreeQueries::NextSkippingChildren(*node, &firstNode);
          continue;
        }

        node->SetSelfOrPrecedingNodesAffectDirAuto(dirIsAuto);
        node = TreeQueries::Next(*node, &firstNode);
      }
    }

    void UpdateElementHasDirAutoFlag(Element &element, TextDirectionState textDirectionState) noexcept
    {
      auto *parent = TreeQueries::ParentOrShadowHostElement(element);
      element.NodeDocument().SetIsDirAttributeDirty();

      switch (textDirectionState)
      {
        case TextDirectionState::LTR:
        case TextDirectionState::RTL:
          if (element.SelfOrPrecedingNodesAffectDirAuto()
              || (parent && parent->SelfOrPrecedingNodesAffectDirAuto()))
          {
            UpdateHasDirAutoFlagForSubtree(element, textDirectionState);
          }
          break;

        case TextDirectionState::Auto: UpdateHasDirAutoFlagForSubtree(element, textDirectionState); break;

        case TextDirectionState::Undefined:
          if (element.SelfOrPrecedingNodesAffectDirAuto()
              && !(parent && parent->SelfOrPrecedingNodesAffectDirAuto())
              && !Is<html::HTMLBDIElement>(element))
          {
            UpdateHasDirAutoFlagForSubtree(element, textDirectionState);
          }
          break;
      }
    }

    // Specs: https://html.spec.whatwg.org/multipage/dom.html#text-node-directionality
    KRYS_NODISCARD Maybe<TextDirection> ComputeTextDirectionFromText(DOMStringView text) noexcept
    {
      return DefaultWritingDirection(text);
    }

    // Specs: https://html.spec.whatwg.org/multipage/dom.html#attr-dir
    KRYS_NODISCARD Maybe<TextDirection> ComputeTextDirection(const Element &element,
                                                             TextDirectionState textDirectionState) noexcept
    {
      switch (textDirectionState)
      {
        case TextDirectionState::LTR: return TextDirection::LTR;
        case TextDirectionState::RTL: return TextDirection::RTL;

        case TextDirectionState::Auto:
          // Specs: Return the auto directionality of element.
          // Specs: If directionality is null, then return 'ltr'.
          return ComputeAutoDirectionality(element).value_or(TextDirection::LTR);

        case TextDirectionState::Undefined:
        {
          // Specs: If element is a bdi element, then return the auto directionality of element.
          // Specs: If directionality is null, then return 'ltr'.
          if (Is<html::HTMLBDIElement>(element))
          {
            return ComputeAutoDirectionality(element).value_or(TextDirection::LTR);
          }

          // Specs: If element is an input element whose type attribute is in the Telephone state
          // then return LTR;
          if (auto *input = DynamicDowncast<html::HTMLInputElement>(element);
              input && input->IsTelephoneField())
          {
            return TextDirection::LTR;
          }

          // Specs: If the parent is a shadow root, then return the directionality of parent's host.
          // Specs: If the parent is an element, then return the directionality of parent.
          if (auto *parent = TreeQueries::ParentOrShadowHostElement(element))
          {
            return ComputeTextDirection(*parent, ElementTextDirectionState(*parent));
          }

          return null;
        }
      }

      return null;
    }

    // Specs: https://html.spec.whatwg.org/multipage/dom.html#contained-text-auto-directionality
    KRYS_NODISCARD Maybe<TextDirection> ComputeContainedTextAutoDirection(const Element &element) noexcept
    {
      for (auto *child = element.FirstChild(); child;)
      {
        // Specs: Skip bdi, script, style nodes.
        if (HasHTMLTagName(*child, html::TagName::bdi) || HasHTMLTagName(*child, html::TagName::script)
            || HasHTMLTagName(*child, html::TagName::style))
        {
          child = TreeQueries::NextSkippingChildren(*child, &element);
          continue;
        }

        if (auto *childElement = DynamicDowncast<Element>(*child))
        {
          // Specs: Skip text form controls
          // Specs: Skip the element whose dir attribute is not in the undefined state
          if (childElement->IsTextField() || ElementHasValidTextDirectionState(*childElement))
          {
            child = TreeQueries::NextSkippingChildren(*child, &element);
            continue;
          }
        }

        // Specs: If child is a slot element whose root is a shadow root,
        // then return the directionality of that shadow root's host.
        if (auto *childSlotElement = DynamicDowncast<html::HTMLSlotElement>(*child))
        {
          if (auto *childHost = TreeQueries::ShadowHost(*childSlotElement))
          {
            return ComputeTextDirection(*childHost, ElementTextDirectionState(*childHost));
          }
        }

        // Specs: If descendant is a Text node and its text direction is not null,
        // then return it.
        if (auto *text = DynamicDowncast<Text>(*child))
        {
          if (auto direction = ComputeTextDirectionFromText(text->Data()))
          {
            return direction;
          }
        }

        child = TreeQueries::Next(*child, &element);
      }

      return null;
    }

    // Specs: https://html.spec.whatwg.org/multipage/dom.html#auto-directionality
    KRYS_NODISCARD Maybe<TextDirection>
      ComputeTextDirectionOfSlotElement(const html::HTMLSlotElement &slotElement) noexcept
    {
      // Specs: If element is a slot element whose root is a shadow root
      // and element's assigned nodes are not empty, then return the
      // directionality of this slot.
      if (!slotElement.IsInShadowTree())
      {
        return ComputeContainedTextAutoDirection(slotElement);
      }

      auto *nodes = slotElement.AssignedNodes();
      if (!nodes)
      {
        return ComputeContainedTextAutoDirection(slotElement);
      }

      for (auto &child : *nodes)
      {
        // Specs: If child is a Text node and its text direction is
        // not null, then return it.
        if (auto *text = DynamicDowncast<Text>(*child))
        {
          if (auto direction = ComputeTextDirectionFromText(text->Data()))
          {
            return direction;
          }
        }

        if (auto *element = DynamicDowncast<Element>(*child))
        {
          // Specs: If child direction is not null, then return child direction.
          if (auto direction = ComputeAutoDirectionality(*element))
          {
            return direction;
          }
        }
      }

      return null;
    }
  }

  // Specs: https://html.spec.whatwg.org/multipage/dom.html#auto-directionality
  Maybe<TextDirection> ComputeAutoDirectionality(const Element &element) noexcept
  {
    if (auto *textFormControl = DynamicDowncast<html::HTMLTextFormControlElement>(element))
    {
      if (!textFormControl->DirAutoUsesValue())
      {
        return ComputeContainedTextAutoDirection(*textFormControl);
      }

      // Specs: The directionality of the auto-directionality form-associated
      // element is calculated from its value() text.
      // Specs: If element's value is not the empty string, then return 'ltr'.
      if (auto value = textFormControl->Value(); !value.empty())
      {
        return ComputeTextDirectionFromText(value).value_or(TextDirection::LTR);
      }
      return null;
    }

    if (auto *slotElement = DynamicDowncast<html::HTMLSlotElement>(element))
    {
      return ComputeTextDirectionOfSlotElement(*slotElement);
    }

    return ComputeContainedTextAutoDirection(element);
  }

  Maybe<TextDirection> ComputeTextDirectionIfDirIsAuto(const Element &element) noexcept
  {
    if (!(element.SelfOrPrecedingNodesAffectDirAuto() && element.HasAutoTextDirectionState()))
    {
      return null;
    }
    return ComputeTextDirection(element, ElementTextDirectionState(element));
  }

  namespace
  {
    void UpdateEffectiveTextDirectionOfElementAndShadowTree(Element &element, Maybe<TextDirection> direction,
                                                            Element *initiator = nullptr) noexcept
    {
      bool usesEffectiveTextDirection = direction.has_value();
      auto effectiveDirection = direction.value_or(TextDirection::LTR);

      style::PseudoClassChangeInvalidation styleInvalidation(element, css::SelectorPseudoClass::Dir,
                                                             style::PseudoClassChangeInvalidation::AnyValue);
      element.SetUsesEffectiveTextDirection(usesEffectiveTextDirection);
      element.SetEffectiveTextDirection(effectiveDirection);

      if (auto *shadowRoot = TreeQueries::GetShadowRoot(element))
      {
        for (auto *node = shadowRoot->FirstChild(); node; node = node->NextSibling())
        {
          auto *child = DynamicDowncast<Element>(*node);
          if (!child || child == initiator)
          {
            continue;
          }

          UpdateEffectiveTextDirectionOfElementAndShadowTree(*child, direction);
          UpdateEffectiveTextDirectionOfDescendants(*child, direction);
        }
      }

      if (element.Renderer()
          && element.Renderer()->GetWritingMode().ComputedTextDirection() != effectiveDirection)
      {
        element.InvalidateStyleForSubtree();
      }
    }

    Maybe<TextDirection> UpdateEffectiveTextDirectionOfElementAndDescendants(
      Element &element, TextDirectionState textDirectionState, Element *initiator = nullptr) noexcept
    {
      UpdateElementHasDirAutoFlag(element, textDirectionState);

      auto direction = ComputeTextDirection(element, textDirectionState);

      UpdateEffectiveTextDirectionOfElementAndShadowTree(element, direction, initiator);
      UpdateEffectiveTextDirectionOfDescendants(element, direction, initiator);

      return direction;
    }
  }

  void TextDirectionStateChanged(Element &element, TextDirectionState textDirectionState) noexcept
  {
    (void)UpdateEffectiveTextDirectionOfElementAndDescendants(element, textDirectionState);

    auto *parent = TreeQueries::ParentOrShadowHostElement(element);
    if (!parent)
    {
      return;
    }

    if (parent->SelfOrPrecedingNodesAffectDirAuto())
    {
      UpdateEffectiveTextDirectionOfAncestors(*parent, &element);
    }
  }

  void UpdateEffectiveTextDirectionState(Element &element, TextDirectionState textDirectionState,
                                         Element *initiator) noexcept
  {
    auto direction =
      UpdateEffectiveTextDirectionOfElementAndDescendants(element, textDirectionState, initiator);

    auto *parent = TreeQueries::ParentOrShadowHostElement(element);
    if (!parent)
    {
      return;
    }

    // This element won't affect its ancestors if its effective direction
    // is the same as its parent's effective direction.
    if (direction && parent->UsesEffectiveTextDirection() && *direction == parent->EffectiveTextDirection())
    {
      return;
    }

    if (parent->SelfOrPrecedingNodesAffectDirAuto())
    {
      UpdateEffectiveTextDirectionOfAncestors(*parent, &element);
    }
  }

  void UpdateEffectiveTextDirectionOfDescendants(Element &element, Maybe<TextDirection> direction,
                                                 Element *initiator) noexcept
  {
    for (auto *node = element.FirstChild(); node;)
    {
      auto *child = DynamicDowncast<Element>(*node);
      if (!child)
      {
        node = TreeQueries::Next(*node, &element);
        continue;
      }

      if (child == initiator || ElementHasValidTextDirectionState(*child))
      {
        node = TreeQueries::NextSkippingChildren(*child, &element);
        continue;
      }

      UpdateEffectiveTextDirectionOfElementAndShadowTree(*child, direction);
      node = TreeQueries::Next(*child, &element);
    }
  }

  void UpdateEffectiveTextDirectionOfAncestors(Element &element, Element *initiator) noexcept
  {
    krys_debug_assert(element.SelfOrPrecedingNodesAffectDirAuto());
    for (auto *ancestor = &element; ancestor; ancestor = ancestor->ParentElement())
    {
      if (!ElementHasAutoTextDirectionState(*ancestor))
      {
        continue;
      }
      UpdateEffectiveTextDirectionState(*ancestor, TextDirectionState::Auto, initiator);
      break;
    }
  }
}
