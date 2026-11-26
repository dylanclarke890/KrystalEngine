/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Html/Algorithms/ElementTextDirection.hpp"
#include "Krystal.Text/UnicodeHelpers.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Html/Elements/HTMLBDIElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLTextFormControlElement.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include "Krystal.Web/Rendering/RenderObject.hpp"
#include "Krystal.Web/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Web/Text/WritingMode.hpp"

namespace krys::web::html
{
  namespace
  {
    KRYS_NODISCARD bool HasHTMLTagName(const Node &node, html::TagName tagName) noexcept
    {
      auto *element = DynamicDowncast<HTMLElement>(node);
      return element && element->TagQualifiedName().TagName() == tagName;
    }

    void UpdateHasDirAutoFlagForSubtree(Element &firstNode, dom::TextDirectionState textDirectionState) noexcept
    {
      bool dirIsAuto = textDirectionState == dom::TextDirectionState::Auto;
      firstNode.SetSelfOrPrecedingNodesAffectDirAuto(dirIsAuto);

      for (auto *node = firstNode.FirstChild(); node;)
      {
        auto *element = DynamicDowncast<Element>(*node);

        if (element && (Is<HTMLBDIElement>(*element) || dom::ElementHasValidTextDirectionState(*element)))
        {
          node = dom::TreeTraversal::NextSkippingChildren(*node, &firstNode);
          continue;
        }

        node->SetSelfOrPrecedingNodesAffectDirAuto(dirIsAuto);
        node = dom::TreeQueries::Next(*node, &firstNode);
      }
    }

    void UpdateElementHasDirAutoFlag(Element &element, dom::TextDirectionState textDirectionState) noexcept
    {
      auto *parent = dom::TreeTraversal::ParentOrShadowHostElement(element);
      element.NodeDocument().SetIsDirAttributeDirty();

      switch (textDirectionState)
      {
        case dom::TextDirectionState::LTR:
        case dom::TextDirectionState::RTL:
          if (element.SelfOrPrecedingNodesAffectDirAuto()
              || (parent && parent->SelfOrPrecedingNodesAffectDirAuto()))
          {
            UpdateHasDirAutoFlagForSubtree(element, textDirectionState);
          }
          break;

        case dom::TextDirectionState::Auto: UpdateHasDirAutoFlagForSubtree(element, textDirectionState); break;

        case dom::TextDirectionState::Undefined:
          if (element.SelfOrPrecedingNodesAffectDirAuto()
              && !(parent && parent->SelfOrPrecedingNodesAffectDirAuto()) && !Is<HTMLBDIElement>(element))
          {
            UpdateHasDirAutoFlagForSubtree(element, textDirectionState);
          }
          break;
      }
    }

    /// @see https://html.spec.whatwg.org/multipage/dom.html#text-node-directionality
    KRYS_NODISCARD Maybe<text::TextDirection> ComputeTextDirectionFromText(DOMStringView text) noexcept
    {
      // 1. If text's data does not contain a code point whose bidirectional character type is L, AL, or R,
      //    then return null.
      // 2. Let codePoint be the first code point in text's data whose bidirectional character type is L, AL,
      //    or R.
      // 3. If codePoint is of bidirectional character type AL or R, then return 'rtl'.
      // 4. If codePoint is of bidirectional character type L, then return 'ltr'.
      return text::DefaultWritingDirection(text);
    }

    /// @brief The directionality of @p element, whose `dir` attribute is in @p textDirectionState; null for
    /// an element without a parent whose directionality would be 'ltr'.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#the-directionality
    /// @see https://html.spec.whatwg.org/multipage/dom.html#parent-directionality
    KRYS_NODISCARD Maybe<text::TextDirection>
      ComputeTextDirection(const Element &element, dom::TextDirectionState textDirectionState) noexcept
    {
      switch (textDirectionState)
      {
        // LTR: Return 'ltr'.
        case dom::TextDirectionState::LTR: return text::TextDirection::LTR;

        // RTL: Return 'rtl'.
        case dom::TextDirectionState::RTL: return text::TextDirection::RTL;

        // Auto:
        // 1. Let result be the auto directionality of element.
        // 2. If result is null, then return 'ltr'.
        // 3. Return result.
        case dom::TextDirectionState::Auto:
          return ComputeAutoDirectionality(element).value_or(text::TextDirection::LTR);

        // Undefined:
        case dom::TextDirectionState::Undefined:
        {
          // If element is a bdi element:
          // 1. Let result be the auto directionality of element.
          // 2. If result is null, then return 'ltr'.
          // 3. Return result.
          if (Is<HTMLBDIElement>(element))
          {
            return ComputeAutoDirectionality(element).value_or(text::TextDirection::LTR);
          }

          // If element is an input element whose type attribute is in the Telephone state: Return 'ltr'.
          if (auto *input = DynamicDowncast<HTMLInputElement>(element); input && input->IsTelephoneField())
          {
            return text::TextDirection::LTR;
          }

          // Otherwise: Return the parent directionality of element.
          // 1. Let parentNode be element's parent node.
          // 2. If parentNode is a shadow root, then return the directionality of parentNode's host.
          // 3. If parentNode is an element, then return the directionality of parentNode.
          if (auto *parent = dom::TreeTraversal::ParentOrShadowHostElement(element))
          {
            return ComputeTextDirection(*parent, dom::ElementTextDirectionState(*parent));
          }

          // 4. Return 'ltr'.
          // (Null, which is 'ltr' where it's used.)
          return null;
        }
      }

      return null;
    }

    /// @brief Whether contained text auto directionality skips @p node and its descendants: a bdi, script,
    /// style or textarea element, or an element whose dir attribute is not in the Undefined state.
    KRYS_NODISCARD bool IsExcludedFromContainedTextAutoDirection(const Node &node) noexcept
    {
      if (HasHTMLTagName(node, html::TagName::bdi) || HasHTMLTagName(node, html::TagName::script)
          || HasHTMLTagName(node, html::TagName::style) || HasHTMLTagName(node, html::TagName::textarea))
      {
        return true;
      }
      auto *element = DynamicDowncast<Element>(node);
      return element && dom::ElementHasValidTextDirectionState(*element);
    }

    /// @see https://html.spec.whatwg.org/multipage/dom.html#contained-text-auto-directionality
    KRYS_NODISCARD Maybe<text::TextDirection> ComputeContainedTextAutoDirection(const Element &element,
                                                                                bool canExcludeRoot) noexcept
    {
      // (With canExcludeRoot, an excluded element skips every descendant.)
      if (canExcludeRoot && IsExcludedFromContainedTextAutoDirection(element))
      {
        return null;
      }

      // 1. For each node descendant of element's descendants, in tree order:
      for (auto *descendant = element.FirstChild(); descendant;)
      {
        // 1a. If any of descendant; any ancestor element of descendant that is a descendant of element; if
        //     canExcludeRoot is true, element is one of a bdi element; a script element; a style element; a
        //     textarea element; an element whose dir attribute is not in the Undefined state then continue.
        if (IsExcludedFromContainedTextAutoDirection(*descendant))
        {
          descendant = dom::TreeTraversal::NextSkippingChildren(*descendant, &element);
          continue;
        }

        // 1b. If descendant is a slot element whose root is a shadow root, then return the directionality of
        //     that shadow root's host.
        if (auto *slot = DynamicDowncast<HTMLSlotElement>(*descendant))
        {
          if (auto *host = dom::TreeTraversal::ShadowHost(*slot))
          {
            return ComputeTextDirection(*host, dom::ElementTextDirectionState(*host));
          }
        }

        // 1c. If descendant is not a Text node, then continue.
        // 1d. Let result be the text node directionality of descendant.
        // 1e. If result is not null, then return result.
        if (auto *text = DynamicDowncast<Text>(*descendant))
        {
          if (auto result = ComputeTextDirectionFromText(text->Data()))
          {
            return result;
          }
        }

        descendant = dom::TreeQueries::Next(*descendant, &element);
      }

      // 2. Return null.
      return null;
    }
  }

  Maybe<text::TextDirection> ComputeAutoDirectionality(const Element &element) noexcept
  {
    // 1. If element is an auto-directionality form-associated element:
    if (auto *textFormControl = DynamicDowncast<HTMLTextFormControlElement>(element);
        textFormControl && textFormControl->DirAutoUsesValue())
    {
      // 1a. If element's value contains a character of bidirectional character type AL or R, and there is no
      //     character of bidirectional character type L anywhere before it in the element's value, then
      //     return 'rtl'.
      // 1b. If element's value is not the empty string, then return 'ltr'.
      // 1c. Return null.
      if (auto value = textFormControl->Value(); !value.empty())
      {
        return ComputeTextDirectionFromText(value).value_or(text::TextDirection::LTR);
      }
      return null;
    }

    // 2. If element is a slot element whose root is a shadow root and element's assigned nodes are not empty:
    if (auto *slot = DynamicDowncast<HTMLSlotElement>(element))
    {
      if (auto *nodes = slot->AssignedNodes(); nodes && slot->IsInShadowTree())
      {
        // 2a. For each node child of element's assigned nodes:
        for (auto &child : *nodes)
        {
          // 2a.i. Let childDirection be null.
          Maybe<text::TextDirection> childDirection;

          // 2a.ii. If child is a Text node, then set childDirection to the text node directionality of child.
          if (auto *text = DynamicDowncast<Text>(*child))
          {
            childDirection = ComputeTextDirectionFromText(text->Data());
          }
          // 2a.iii. Otherwise:
          // 2a.iii.A. Assert: child is an Element node.
          // 2a.iii.B. Set childDirection to the contained text auto directionality of child with
          //           canExcludeRoot set to true.
          else if (auto *childElement = DynamicDowncast<Element>(*child))
          {
            childDirection = ComputeContainedTextAutoDirection(*childElement, true);
          }

          // 2a.iv. If childDirection is not null, then return childDirection.
          if (childDirection)
          {
            return childDirection;
          }
        }

        // 2b. Return null.
        return null;
      }
    }

    // 3. Return the contained text auto directionality of element with canExcludeRoot set to false.
    return ComputeContainedTextAutoDirection(element, false);
  }

  Maybe<text::TextDirection> ComputeTextDirectionIfDirIsAuto(const Element &element) noexcept
  {
    if (!(element.SelfOrPrecedingNodesAffectDirAuto() && element.HasAutoTextDirectionState()))
    {
      return null;
    }
    return ComputeTextDirection(element, dom::ElementTextDirectionState(element));
  }

  namespace
  {
    void UpdateEffectiveTextDirectionOfElementAndShadowTree(Element &element,
                                                            Maybe<text::TextDirection> direction,
                                                            Element *initiator = nullptr) noexcept
    {
      bool usesEffectiveTextDirection = direction.has_value();
      auto effectiveDirection = direction.value_or(text::TextDirection::LTR);

      style::PseudoClassChangeInvalidation styleInvalidation(element, css::SelectorPseudoClass::Dir,
                                                             style::PseudoClassChangeInvalidation::AnyValue);
      element.SetUsesEffectiveTextDirection(usesEffectiveTextDirection);
      element.SetEffectiveTextDirection(effectiveDirection);

      if (auto *shadowRoot = dom::TreeTraversal::GetShadowRoot(element))
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

    Maybe<text::TextDirection> UpdateEffectiveTextDirectionOfElementAndDescendants(
      Element &element, dom::TextDirectionState textDirectionState, Element *initiator = nullptr) noexcept
    {
      UpdateElementHasDirAutoFlag(element, textDirectionState);

      auto direction = ComputeTextDirection(element, textDirectionState);

      UpdateEffectiveTextDirectionOfElementAndShadowTree(element, direction, initiator);
      UpdateEffectiveTextDirectionOfDescendants(element, direction, initiator);

      return direction;
    }
  }

  void TextDirectionStateChanged(Element &element, dom::TextDirectionState textDirectionState) noexcept
  {
    (void)UpdateEffectiveTextDirectionOfElementAndDescendants(element, textDirectionState);

    auto *parent = dom::TreeTraversal::ParentOrShadowHostElement(element);
    if (!parent)
    {
      return;
    }

    if (parent->SelfOrPrecedingNodesAffectDirAuto())
    {
      UpdateEffectiveTextDirectionOfAncestors(*parent, &element);
    }
  }

  void UpdateEffectiveTextDirectionState(Element &element, dom::TextDirectionState textDirectionState,
                                         Element *initiator) noexcept
  {
    auto direction =
      UpdateEffectiveTextDirectionOfElementAndDescendants(element, textDirectionState, initiator);

    auto *parent = dom::TreeTraversal::ParentOrShadowHostElement(element);
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

  void UpdateEffectiveTextDirectionOfDescendants(Element &element, Maybe<text::TextDirection> direction,
                                                 Element *initiator) noexcept
  {
    for (auto *node = element.FirstChild(); node;)
    {
      auto *child = DynamicDowncast<Element>(*node);
      if (!child)
      {
        node = dom::TreeQueries::Next(*node, &element);
        continue;
      }

      if (child == initiator || dom::ElementHasValidTextDirectionState(*child))
      {
        node = dom::TreeTraversal::NextSkippingChildren(*child, &element);
        continue;
      }

      UpdateEffectiveTextDirectionOfElementAndShadowTree(*child, direction);
      node = dom::TreeQueries::Next(*child, &element);
    }
  }

  void UpdateEffectiveTextDirectionOfAncestors(Element &element, Element *initiator) noexcept
  {
    krys_debug_assert(element.SelfOrPrecedingNodesAffectDirAuto());
    for (auto *ancestor = &element; ancestor; ancestor = ancestor->ParentElement())
    {
      if (!dom::ElementHasAutoTextDirectionState(*ancestor))
      {
        continue;
      }
      UpdateEffectiveTextDirectionState(*ancestor, dom::TextDirectionState::Auto, initiator);
      break;
    }
  }
}
