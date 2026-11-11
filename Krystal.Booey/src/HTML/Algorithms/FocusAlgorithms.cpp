/*
 * Copyright (C) 2006, 2007, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nuanti Ltd.
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

#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/FocusEvent.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/SpatialNavigation.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD bool IsHTMLElement(const dom::Element &element, TagName tagName) noexcept
    {
      return Is<HTMLElement>(element) && element.TagQualifiedName().TagName() == tagName;
    }

    /// @brief Whether @p element is an editing host: editable, and not inside an editable element.
    /// @see https://html.spec.whatwg.org/#editing-host
    KRYS_NODISCARD bool IsEditingHost(const dom::Element &element) noexcept
    {
      if (!Is<HTMLElement>(element) || IsHTMLElement(element, TagName::input)
          || IsHTMLElement(element, TagName::textarea))
      {
        return false;
      }
      if (!css::MatchesReadWritePseudoClass(element))
      {
        return false;
      }
      // NOTE: as in Chrome, the body is one in design mode, not just the document element.
      if (element.NodeDocument().InDesignMode() && IsHTMLElement(element, TagName::body)
          && element.ParentNode() == element.NodeDocument().DocumentElement())
      {
        return true;
      }
      auto *parent = element.ParentElement();
      return !parent || !Is<HTMLElement>(*parent) || IsHTMLElement(*parent, TagName::input)
             || IsHTMLElement(*parent, TagName::textarea) || !css::MatchesReadWritePseudoClass(*parent);
    }

    /// @brief Whether the user agent determines @p element to be focusable when it has no `tabindex`
    /// (following the elements the specification suggests).
    KRYS_NODISCARD bool IsFocusableByDefault(const dom::Element &element) noexcept
    {
      // Links, including the shapes of `area` elements.
      if (css::IsLink(element))
      {
        return true;
      }
      if (Is<HTMLElement>(element))
      {
        switch (element.TagQualifiedName().TagName())
        {
          case TagName::button:
          case TagName::select:
          case TagName::textarea:
          case TagName::dialog:
          // Navigable containers.
          case TagName::iframe:
          case TagName::frame:    return true;
          case TagName::input:
          {
            auto type = text::ToASCIILower(
              dom::DOMStringView(element.GetAttribute(u8"type").value_or(dom::DOMString())));
            return type != u8"hidden";
          }
          default: break;
        }
      }
      // The options of a select with base appearance, which the keys move between.
      if (auto *option = DynamicDowncast<HTMLOptionElement>(element))
      {
        auto *select = option->OwnerSelectElement();
        return select && select->ContentsUseBaseAppearance() && !option->IsDisabledFormControl();
      }
      // FIXME: Elements with a draggable attribute, once a drag can start without a pointing device.
      if (auto *summary = DynamicDowncast<HTMLSummaryElement>(element); summary && summary->IsActiveSummary())
      {
        return true;
      }
      return IsEditingHost(element);
    }

    /// @brief Whether @p element is being rendered or delegating its rendering to its children
    /// (`display: contents`), and visible.
    /// @see https://html.spec.whatwg.org/#being-rendered
    KRYS_NODISCARD bool IsBeingRendered(const dom::Element &element) noexcept
    {
      auto *style = element.RenderOrDisplayContentsStyle();
      if (!style)
      {
        // Compute the style of a yet unstyled subtree without resolving the document's style.
        style = const_cast<dom::Element &>(element).ResolveComputedStyle(
          dom::Element::ResolveComputedStyleMode::RenderedOnly);
      }
      return style && style->Display() != DisplayType::None && style->GetVisibility() == Visibility::Visible;
    }

    /// @brief Whether @p element is a canvas's fallback content, which isn't rendered, while the canvas is
    /// not being rendered either.
    /// @see https://html.spec.whatwg.org/#being-used-as-relevant-canvas-fallback-content
    KRYS_NODISCARD bool IsIrrelevantCanvasFallbackContent(const dom::Element &element) noexcept
    {
      if (element.Renderer())
      {
        return false;
      }
      for (auto *ancestor = element.ParentElement(); ancestor; ancestor = ancestor->ParentElement())
      {
        if (auto *canvas = DynamicDowncast<HTMLCanvasElement>(*ancestor))
        {
          return !IsBeingRendered(*canvas);
        }
      }
      return false;
    }

    KRYS_NODISCARD bool IsShadowHostThatDelegatesFocus(const dom::Element &element) noexcept
    {
      auto *shadowRoot = dom::TreeQueries::GetShadowRoot(element);
      return shadowRoot && shadowRoot->DelegatesFocus();
    }

    KRYS_NODISCARD bool IsFocusableAreaNode(const dom::Node &node) noexcept
    {
      if (Is<dom::Document>(node))
      {
        // The document's viewport.
        return true;
      }
      auto *element = DynamicDowncast<dom::Element>(node);
      return element && FocusAlgorithms::IsFocusableArea(*element);
    }

    /// @brief Whether @p node is in a user agent shadow tree whose root delegates focus: a part of a control
    /// that's focused instead of the control.
    KRYS_NODISCARD bool IsInUserAgentShadowTreeThatDelegatesFocus(const dom::Node &node) noexcept
    {
      if (!node.IsInUserAgentShadowTree())
      {
        return false;
      }
      auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(node);
      return shadowRoot && shadowRoot->DelegatesFocus();
    }

    /// @brief Whether @p area is click focusable: the user agent considers every focusable area click
    /// focusable.
    KRYS_NODISCARD bool IsClickFocusable(const dom::Node &area) noexcept
    {
      // The insides of a control (a text field's inner text, which is an editing host) aren't focused by a
      // click; the control is, unless the control delegates its focus to them.
      if (area.IsInUserAgentShadowTree() && !IsInUserAgentShadowTreeThatDelegatesFocus(area))
      {
        return false;
      }
      return IsFocusableAreaNode(area);
    }

    /// @brief Whether @p element is a part of a control in its user agent shadow tree, and the control (or a
    /// control it's part of) is disabled.
    KRYS_NODISCARD bool IsPartOfDisabledControl(const dom::Element &element) noexcept
    {
      for (auto *host = dom::TreeQueries::ShadowHost(element); host;
           host = host->IsInUserAgentShadowTree() ? dom::TreeQueries::ShadowHost(*host) : nullptr)
      {
        if (css::MatchesDisabledPseudoClass(*host))
        {
          return true;
        }
      }
      return false;
    }
  }

  // https://html.spec.whatwg.org/#tabindex-value
  // The tabindex value of an element is the value of its tabindex attribute, parsed using the rules for
  // parsing integers. If parsing fails or the attribute is not specified, then the tabindex value is null. (A
  // value outside the long range is null too, as the tabIndex getter needs.)
  Maybe<int32> FocusAlgorithms::TabIndexValue(const dom::Element &element) noexcept
  {
    auto attribute = element.GetAttribute(u8"tabindex");
    if (!attribute)
    {
      return null;
    }
    auto result = Numbers::ParseInteger(*attribute);
    if (!result.Success() || result.Value < std::numeric_limits<int32>::min()
        || result.Value > std::numeric_limits<int32>::max())
    {
      return null;
    }
    return static_cast<int32>(result.Value);
  }

  // https://html.spec.whatwg.org/#inert
  bool FocusAlgorithms::IsInert(const dom::Node &node) noexcept
  {
    auto *element = DynamicDowncast<dom::Element>(node);
    if (!element)
    {
      element = node.ParentElementInComposedTree();
    }
    // https://html.spec.whatwg.org/#blocked-by-a-modal-dialog
    // While document is so blocked, every node that is connected to document, with the exception of the
    // subject element and its flat tree descendants, must become inert. subject can additionally become inert
    // via the inert attribute, but only if specified on subject itself (i.e., subject escapes inertness of
    // ancestors); subject's flat tree descendants can become inert in a similar fashion.
    auto *modalDialog = node.IsConnected() ? node.NodeDocument().ActiveModalDialog() : nullptr;
    for (; element; element = element->ParentElementInComposedTree())
    {
      if (Is<HTMLElement>(*element) && element->HasAttribute(u8"inert"))
      {
        return true;
      }
      // https://html.spec.whatwg.org/#the-button-element
      // If a button element is the first child which is an element of a select element, then it is inert.
      if (Is<HTMLButtonElement>(*element) && Is<HTMLSelectElement>(element->ParentNode())
          && dom::TreeQueries::FirstElementChild(*element->ParentNode()) == element)
      {
        return true;
      }
      if (element == modalDialog)
      {
        return false;
      }
    }
    return modalDialog;
  }

  // https://html.spec.whatwg.org/#focusable-area
  bool FocusAlgorithms::IsFocusableArea(const dom::Element &element) noexcept
  {
    if (!element.IsConnected())
    {
      return false;
    }
    // Elements that meet all the following criteria:
    // - the element's tabindex value is non-null, or the element is determined by the user agent to be
    //   focusable;
    if (!TabIndexValue(element) && !IsFocusableByDefault(element))
    {
      return false;
    }
    // - the element is either not a shadow host, or has a shadow root whose delegates focus is false;
    if (IsShadowHostThatDelegatesFocus(element))
    {
      return false;
    }
    // - the element is not actually disabled;
    // (Nor are the user agent provided subwidgets of a control that is.)
    if (css::MatchesDisabledPseudoClass(element))
    {
      return false;
    }
    if (element.IsInUserAgentShadowTree() && IsPartOfDisabledControl(element))
    {
      return false;
    }
    // - the element is not inert;
    if (IsInert(element))
    {
      return false;
    }
    // - the element is either being rendered, delegating its rendering to its children, or being used as
    //   relevant canvas fallback content.
    // The shapes of an `area` element are focusable areas while the image that uses its image map is being
    // rendered (the area itself never is).
    if (auto *area = DynamicDowncast<HTMLAreaElement>(element))
    {
      auto image = area->ImageElement();
      return image && IsBeingRendered(*image);
    }
    // Fallback content isn't rendered, but it's focusable while its canvas is rendered.
    if (IsIrrelevantCanvasFallbackContent(element))
    {
      return false;
    }
    return IsBeingRendered(element);
  }

  // https://html.spec.whatwg.org/#get-the-focusable-area
  dom::Node *FocusAlgorithms::GetFocusableArea(dom::Element &focusTarget,
                                               dom::FocusTrigger focusTrigger) noexcept
  {
    // If focus target is an area element with one or more shapes that are focusable areas: return the shape
    // corresponding to the first img element in tree order that uses the image map to which the area element
    // belongs.
    // (The area element is itself the focusable area, as IsFocusableArea has it.)
    // If focus target is an element with one or more scrollable regions that are focusable areas: return the
    // element's first scrollable region, according to a pre-order, depth-first traversal of the flat tree.
    // FIXME: The scrollable regions of elements.

    // If focus target is the document element of its Document: return the Document's viewport.
    if (&focusTarget == dom::TreeQueries::DocumentElement(focusTarget.NodeDocument()))
    {
      return &focusTarget.NodeDocument();
    }

    // If focus target is a navigable: return the navigable's active document.
    // If focus target is a navigable container with a non-null content navigable: return the navigable
    // container's content navigable's active document.
    // FIXME: Navigables and navigable containers with a content navigable, once there are navigables.

    // If focus target is a shadow host whose shadow root's delegates focus is true:
    if (IsShadowHostThatDelegatesFocus(focusTarget))
    {
      // 1. Let focusedElement be the currently focused area of a top-level traversable's DOM anchor.
      // 2. If focus target is a shadow-including inclusive ancestor of focusedElement, then return
      //    focusedElement.
      auto *focusedArea = CurrentlyFocusedArea(focusTarget.NodeDocument());
      if (focusedArea && Is<dom::Element>(*focusedArea)
          && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(focusTarget, *focusedArea))
      {
        return focusedArea;
      }
      // 3. Return the focus delegate for focus target given focus trigger.
      return FocusDelegate(focusTarget, focusTrigger);
    }

    // Otherwise: return null.
    return nullptr;
  }

  // https://html.spec.whatwg.org/#focus-delegate
  dom::Node *FocusAlgorithms::FocusDelegate(dom::Element &focusTarget,
                                            dom::FocusTrigger focusTrigger) noexcept
  {
    // 1. If focusTarget is a shadow host and its shadow root's delegates focus is false, then return null.
    auto *shadowRoot = dom::TreeQueries::GetShadowRoot(focusTarget);
    if (shadowRoot && !shadowRoot->DelegatesFocus())
    {
      return nullptr;
    }

    // 2. Let whereToLook be focusTarget.
    dom::ContainerNode *whereToLook = &focusTarget;
    // 3. If whereToLook is a shadow host, then set whereToLook to whereToLook's shadow root.
    if (shadowRoot)
    {
      whereToLook = shadowRoot;
    }

    // 4. Let autofocusDelegate be the autofocus delegate for whereToLook given focusTrigger.
    // 5. If autofocusDelegate is not null, then return autofocusDelegate.
    if (auto *autofocusDelegate = AutofocusDelegate(*whereToLook, focusTrigger))
    {
      return autofocusDelegate;
    }

    // 6. For each descendant of whereToLook's descendants, in tree order:
    for (auto *node = dom::TreeQueries::Next(*whereToLook, whereToLook); node;
         node = dom::TreeQueries::Next(*node, whereToLook))
    {
      auto *descendant = DynamicDowncast<dom::Element>(*node);
      if (!descendant)
      {
        continue;
      }
      // 6.1. Let focusableArea be null.
      dom::Node *focusableArea = nullptr;
      // 6.2. If focusTarget is a dialog element and descendant is sequentially focusable, then set
      //      focusableArea to descendant.
      // 6.3. Otherwise, if focusTarget is not a dialog and descendant is a focusable area, set focusableArea
      //      to descendant.
      if (IsHTMLElement(focusTarget, TagName::dialog) ? IsSequentiallyFocusable(*descendant)
                                                      : IsFocusableArea(*descendant))
      {
        focusableArea = descendant;
      }
      // 6.4. Otherwise, set focusableArea to the result of getting the focusable area for descendant given
      //      focusTrigger.
      else
      {
        focusableArea = GetFocusableArea(*descendant, focusTrigger);
      }
      // 6.5. If focusableArea is not null, then return focusableArea.
      if (focusableArea)
      {
        return focusableArea;
      }
    }

    // 7. Return null.
    return nullptr;
  }

  // https://html.spec.whatwg.org/#autofocus-delegate
  dom::Node *FocusAlgorithms::AutofocusDelegate(dom::ContainerNode &focusTarget,
                                                dom::FocusTrigger focusTrigger) noexcept
  {
    // 1. For each descendant descendant of focus target, in tree order:
    for (auto *node = dom::TreeQueries::Next(focusTarget, &focusTarget); node;
         node = dom::TreeQueries::Next(*node, &focusTarget))
    {
      // 1.1. If descendant does not have an autofocus content attribute, then continue.
      auto *descendant = DynamicDowncast<dom::Element>(*node);
      if (!descendant || !descendant->HasAttribute(u8"autofocus"))
      {
        continue;
      }
      // 1.2. Let focusable area be descendant, if descendant is a focusable area; otherwise let focusable
      //      area be the result of getting the focusable area for descendant given focus trigger.
      dom::Node *focusableArea =
        IsFocusableArea(*descendant) ? descendant : GetFocusableArea(*descendant, focusTrigger);
      // 1.3. If focusable area is null, then continue.
      if (!focusableArea)
      {
        continue;
      }
      // 1.4. If focusable area is not click focusable and focus trigger is "click", then continue.
      if (!IsClickFocusable(*focusableArea) && focusTrigger == dom::FocusTrigger::Click)
      {
        continue;
      }
      // 1.5. Return focusable area.
      return focusableArea;
    }
    // 2. Return null.
    return nullptr;
  }

  // https://html.spec.whatwg.org/#currently-focused-area-of-a-top-level-traversable
  dom::Node *FocusAlgorithms::CurrentlyFocusedArea(dom::Document &document) noexcept
  {
    // 1. If traversable does not have system focus, then return null.
    // FIXME: Nested navigables, once there are any; a document is its own top-level traversable's active
    // document.
    if (!document.IsFocusedAndActive())
    {
      return nullptr;
    }
    // 2. Let candidate be traversable's active document.
    // 3. While candidate's focused area is a navigable container with a non-null content navigable: set
    //    candidate to the active document of that navigable container's content navigable.
    // 4. If candidate's focused area is non-null, set candidate to candidate's focused area.
    // 5. Return candidate.
    if (auto *focusedElement = document.FocusedElement())
    {
      return focusedElement;
    }
    return &document;
  }

  // https://html.spec.whatwg.org/#focus-chain
  FocusAlgorithms::FocusChain FocusAlgorithms::GetFocusChain(dom::Node &subject) noexcept
  {
    // 1. Let output be an empty list.
    FocusChain output;
    // 2. Let currentObject be subject.
    // 3. While true:
    // 3.1. Append currentObject to output.
    output.push_back(ShareRef(subject));
    // 3.2. If currentObject is an area element's shape, then append that area element to output.
    //      Otherwise, if currentObject's DOM anchor is an element that is not currentObject itself, then
    //      append currentObject's DOM anchor to output.
    // (Focusable areas are their own DOM anchors.)
    // 3.3. If currentObject is a focusable area, then set currentObject to currentObject's DOM anchor's node
    //      document. Otherwise, if currentObject is a Document whose node navigable's parent is non-null,
    //      then set currentObject to currentObject's node navigable's parent. Otherwise, break.
    // FIXME: Parent navigables, once there are any.
    if (!Is<dom::Document>(subject))
    {
      output.push_back(ShareRef<dom::Node>(subject.NodeDocument()));
    }
    // 4. Return output.
    return output;
  }

  // https://html.spec.whatwg.org/#focusing-steps
  void FocusAlgorithms::FocusingSteps(dom::Node &newFocusTarget, dom::Node *fallbackTarget,
                                      const dom::FocusOptions &options) noexcept
  {
    // 1. If new focus target is not a focusable area, then set new focus target to the result of getting the
    //    focusable area for new focus target, given focus trigger if it was passed.
    dom::Node *target = &newFocusTarget;
    if (!IsFocusableAreaNode(*target))
    {
      auto *element = DynamicDowncast<dom::Element>(*target);
      target = element ? GetFocusableArea(*element, options.Trigger) : nullptr;
    }

    // 2. If new focus target is null:
    if (!target)
    {
      // 2.1. If no fallback target was specified, then return.
      if (!fallbackTarget)
      {
        return;
      }
      // 2.2. Otherwise, set new focus target to the fallback target.
      target = fallbackTarget;
    }

    // 3. If new focus target is a navigable container with non-null content navigable, then set new focus
    //    target to the content navigable's active document.
    // FIXME: Navigable containers with a content navigable, once there are navigables.

    // 4. If new focus target is a focusable area and its DOM anchor is inert, then return.
    // (A document's viewport isn't inert.)
    if (Is<dom::Element>(*target) && IsInert(*target))
    {
      return;
    }

    // 5. If new focus target is the currently focused area of a top-level traversable, then return.
    auto &document = target->NodeDocument();
    if (target == CurrentlyFocusedArea(document))
    {
      return;
    }

    // 6. Let old chain be the current focus chain of the top-level traversable in which new focus target
    // finds
    //    itself.
    auto *currentlyFocusedArea = CurrentlyFocusedArea(document);
    auto oldChain = currentlyFocusedArea ? GetFocusChain(*currentlyFocusedArea) : FocusChain {};
    // 7. Let new chain be the focus chain of new focus target.
    auto newChain = GetFocusChain(*target);
    // 8. Run the focus update steps with old chain, new chain, and new focus target respectively.
    FocusUpdateSteps(krys::move(oldChain), krys::move(newChain), target, options);
  }

  // https://html.spec.whatwg.org/#unfocusing-steps
  void FocusAlgorithms::UnfocusingSteps(dom::Element &oldFocusTarget) noexcept
  {
    auto &document = oldFocusTarget.NodeDocument();
    dom::Node *target = &oldFocusTarget;

    // 1. If old focus target is a shadow host whose shadow root's delegates focus is true, and old focus
    //    target's shadow root is a shadow-including inclusive ancestor of the currently focused area of a
    //    top-level traversable's DOM anchor, then set old focus target to that currently focused area of a
    //    top-level traversable.
    auto *currentlyFocusedArea = CurrentlyFocusedArea(document);
    if (IsShadowHostThatDelegatesFocus(oldFocusTarget) && currentlyFocusedArea
        && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(
          *dom::TreeQueries::GetShadowRoot(oldFocusTarget), *currentlyFocusedArea))
    {
      target = currentlyFocusedArea;
    }

    // 2. If old focus target is inert, then return.
    if (IsInert(*target))
    {
      return;
    }

    // 3. If old focus target is an area element and one of its shapes is the currently focused area of a
    //    top-level traversable, or, if old focus target is an element with one or more scrollable regions,
    //    and one of them is the currently focused area of a top-level traversable, then let old focus target
    //    be that currently focused area of a top-level traversable.
    // (The area element is its own shape.)
    // FIXME: Scrollable regions.

    // 4. Let old chain be the current focus chain of the top-level traversable in which old focus target
    //    finds itself.
    auto oldChain = currentlyFocusedArea ? GetFocusChain(*currentlyFocusedArea) : FocusChain {};
    // 5. If old focus target is not one of the entries in old chain, then return.
    if (std::ranges::none_of(oldChain, [&](const Ref<dom::Node> &entry) { return entry.get() == target; }))
    {
      return;
    }

    // 6. If old focus target is not a focusable area, then return.
    if (!IsFocusableAreaNode(*target))
    {
      return;
    }

    // 7. Let topDocument be old chain's last entry.
    auto &topDocument = Downcast<dom::Document>(*oldChain.back());
    // 8. If topDocument's node navigable has system focus, then run the focusing steps for topDocument's
    //    viewport.
    if (topDocument.IsFocusedAndActive())
    {
      FocusingSteps(topDocument);
    }
    // Otherwise, apply any relevant platform-specific conventions for removing system focus from
    // topDocument's node navigable, and run the focus update steps given old chain, an empty list, and null.
    else
    {
      FocusUpdateSteps(krys::move(oldChain), {}, nullptr);
    }
  }

  // https://html.spec.whatwg.org/#focus-update-steps
  void FocusAlgorithms::FocusUpdateSteps(FocusChain oldChain, FocusChain newChain, dom::Node *newFocusTarget,
                                         const dom::FocusOptions &options) noexcept
  {
    // 1. If the last entry in old chain and the last entry in new chain are the same, pop the last entry from
    //    old chain and the last entry from new chain and redo this step.
    while (!oldChain.empty() && !newChain.empty() && oldChain.back().get() == newChain.back().get())
    {
      oldChain.pop_back();
      newChain.pop_back();
    }

    // The old focused element loses its focus before the blur events (as other engines do), so they see the
    // viewport focused.
    for (auto &entry : oldChain)
    {
      if (auto *element = DynamicDowncast<dom::Element>(*entry);
          element && element->NodeDocument().FocusedElement() == element)
      {
        DesignateFocusedArea(element->NodeDocument(), nullptr, dom::FocusVisibility::Invisible);
      }
    }

    // 2. For each entry entry in old chain, in order, run these substeps:
    for (size_t i = 0; i < oldChain.size(); ++i)
    {
      auto &entry = oldChain[i];
      // 2.1. If entry is an input element, and the change event applies to the element, and the element does
      //      not have a defined activation behavior, and the user has changed the element's value or its list
      //      of selected files while the control was focused without committing that change (such that it is
      //      different to what it was when the control was first focused):
      //      1. Set entry's user validity to true.
      //      2. Fire an event named change at the element, with the bubbles attribute initialized to true.
      // (A textarea, or a control in a user agent shadow tree, commits its change here too.)
      if (auto *formControlElement = DynamicDowncast<HTMLFormControlElement>(*entry);
          formControlElement && formControlElement->WasChangedSinceLastFormControlChangeEvent())
      {
        formControlElement->DispatchFormControlChangeEvent();
      }

      // 2.2. If entry is an element, let blur event target be entry. If entry is a Document object, let blur
      //      event target be that Document object's relevant global object. Otherwise, let blur event target
      //      be null.
      // (A document stays in a chain only while it doesn't have system focus, without nested navigables.
      // Its Window gets its events from SystemFocusChanged, when that changes.)
      RefPtr<dom::Element> blurEventTarget = DynamicDowncast<dom::Element>(entry);

      // 2.3. If entry is the last entry in old chain, and entry is an Element, and the last entry in new
      // chain
      //      is also an Element, then let related blur target be the last entry in new chain. Otherwise, let
      //      related blur target be null.
      dom::EventTarget *relatedBlurTarget = nullptr;
      if (i == oldChain.size() - 1 && blurEventTarget && !newChain.empty()
          && Is<dom::Element>(*newChain.back()))
      {
        relatedBlurTarget = newChain.back().get();
      }

      // 2.4. If blur event target is not null, fire a focus event named blur at blur event target, with
      //      related blur target as the related target.
      if (blurEventTarget)
      {
        blurEventTarget->DispatchBlurEvent(relatedBlurTarget);
        blurEventTarget->DidBlur();
      }
    }

    // 3. Apply any relevant platform-specific conventions for focusing new focus target. (For example, some
    //    platforms select the contents of a text control when that control is focused.)
    // (Focusing a text control selects its contents, as Element::FindTargetAndUpdateFocusAppearance does
    // after the focusing steps.)
    (void)newFocusTarget;

    // 4. For each entry entry in new chain, in reverse order, run these substeps:
    for (size_t i = newChain.size(); i-- > 0;)
    {
      auto &entry = newChain[i];
      // 4.1. If entry is a focusable area, and the focused area of the document is not entry:
      if (IsFocusableAreaNode(*entry))
      {
        auto &document = entry->NodeDocument();
        auto *element = DynamicDowncast<dom::Element>(*entry);
        auto *focusedArea = document.FocusedElement() ? static_cast<dom::Node *>(document.FocusedElement())
                                                      : static_cast<dom::Node *>(&document);
        if (focusedArea != entry.get())
        {
          // 4.1.1. Set document's relevant global object's navigation API's focus changed during ongoing
          //        navigation to true.
          // 4.1.2. Designate entry as the focused area of the document.
          // (DesignateFocusedArea does both.)
          DesignateFocusedArea(document, element, options.Visibility);
        }
      }

      // 4.2. If entry is an element, let focus event target be entry. If entry is a Document object, let
      // focus
      //      event target be that Document object's relevant global object. Otherwise, let focus event target
      //      be null.
      // (See the blur event target, above.)
      auto *focusEventTarget = DynamicDowncast<dom::Element>(*entry);

      // 4.3. If entry is the last entry in new chain, and entry is an Element, and the last entry in old
      // chain
      //      is also an Element, then let related focus target be the last entry in old chain. Otherwise, let
      //      related focus target be null.
      dom::EventTarget *relatedFocusTarget = nullptr;
      if (i == newChain.size() - 1 && focusEventTarget && !oldChain.empty()
          && Is<dom::Element>(*oldChain.back()))
      {
        relatedFocusTarget = oldChain.back().get();
      }

      // 4.4. If focus event target is not null, fire a focus event named focus at focus event target, with
      //      related focus target as the related target.
      if (focusEventTarget)
      {
        focusEventTarget->DispatchFocusEvent(relatedFocusTarget, options);
      }
    }

    if (options.Trigger != dom::FocusTrigger::Bindings && !newChain.empty())
    {
      newChain.back()->NodeDocument()._latestFocusTrigger = options.Trigger;
    }
  }

  // https://html.spec.whatwg.org/#fire-a-focus-event
  void FocusAlgorithms::FireFocusEvent(dom::DOMStringAtom e, dom::Element &target,
                                       dom::EventTarget *relatedTarget) noexcept
  {
    // 1. Let view be target if target is a Window object; otherwise target's node document's relevant global
    //    object.
    auto *view = target.NodeDocument().Window();

    // 2. Fire an event named e at target, using FocusEvent, with the relatedTarget attribute initialized to
    //    relatedTarget, the view attribute initialized to view, and the composed flag set.
    dom::FocusEventInit attributes;
    attributes.View = view;
    attributes.RelatedTarget = ShareRefPtr(relatedTarget);
    attributes.Composed = true;
    (void)dom::EventDispatcher::FireEvent<dom::FocusEvent>(e, target, attributes);

    // The bubbling `focusin` and `focusout` events follow `focus` and `blur`.
    // https://w3c.github.io/uievents/#events-focusevent-event-order
    auto bubblingType = e.View() == u8"focus" ? u8"focusin" : e.View() == u8"blur" ? u8"focusout" : nullptr;
    if (bubblingType)
    {
      attributes.Bubbles = true;
      (void)dom::EventDispatcher::FireEvent<dom::FocusEvent>(dom::DOMStringAtom(bubblingType), target,
                                                             attributes);
    }
  }

  // https://html.spec.whatwg.org/#has-focus-steps
  bool FocusAlgorithms::HasFocusSteps(const dom::Document &target) noexcept
  {
    // 1. If target's node navigable's top-level traversable does not have system focus, then return false.
    // 2. Let candidate be target's node navigable's top-level traversable's active document.
    // 3. While true:
    // 3.1. If candidate is target, then return true.
    // 3.2. If the focused area of candidate is a navigable container with a non-null content navigable, then
    //      set candidate to the active document of that navigable container's content navigable.
    // 3.3. Otherwise, return false.
    // FIXME: Nested navigables, once there are any; a document is its top-level traversable's active
    // document.
    return target.IsFocusedAndActive();
  }

  // https://html.spec.whatwg.org/#the-autofocus-attribute
  void FocusAlgorithms::AutofocusInsertionSteps(dom::Element &element) noexcept
  {
    if (!element.HasAttribute(u8"autofocus"))
    {
      return;
    }

    // NOTE: "inserted into a document" means into a document tree.
    if (!element.IsConnected() || element.IsInShadowTree())
    {
      return;
    }

    // 1. If the user has indicated (for example, by starting to type in a form control) that they do not wish
    // focus to be changed, then optionally return.

    // 2. Let target be the element's node document.
    auto &target = element.NodeDocument();

    // 3. If target is not fully active, then return.
    // NOTE: a document is fully active while it's shown in a frame.
    if (!target.Frame())
    {
      return;
    }

    // 4. If target's active sandboxing flag set has the sandboxed automatic features browsing context flag,
    //    then return.
    // (There's no sandboxing, as there are no nested browsing contexts.)

    // 5. If the allow focus steps given target return false, then return.
    // (Focus is always allowed; see Focus in HTMLOrSVGOrMathMLElement.)

    // 6. Let topDocument be target's node navigable's top-level traversable's active document.
    // (A document is its frame's, the only one.)
    auto &topDocument = target;

    // 7. If topDocument's autofocus processed flag is false, then remove the element from topDocument's
    // autofocus candidates, and append the element to topDocument's autofocus candidates.
    if (topDocument.AutofocusProcessed())
    {
      return;
    }
    auto &candidates = topDocument.AutofocusCandidates();
    std::erase_if(candidates,
                  [&](const WeakPtr<dom::Element> &candidate) { return candidate.get() == &element; });
    candidates.push_back(CreateWeakPtr(&element));
    topDocument.ScheduleRenderingUpdate({RenderingUpdateStep::FlushAutofocusCandidates});
  }

  // https://html.spec.whatwg.org/#flush-autofocus-candidates
  void FocusAlgorithms::FlushAutofocusCandidates(dom::Document &topDocument) noexcept
  {
    // 1. If topDocument's autofocus processed flag is true, then return.
    if (topDocument.AutofocusProcessed())
    {
      return;
    }

    // 2. Let candidates be topDocument's autofocus candidates.
    auto &candidates = topDocument.AutofocusCandidates();

    // 3. If candidates is empty, then return.
    if (candidates.empty())
    {
      return;
    }

    // 4. If topDocument's focused area is not topDocument itself, or topDocument has non-null target element,
    // then: empty candidates, set topDocument's autofocus processed flag to true, and return.
    if (topDocument.FocusedElement() || topDocument.CSSTarget())
    {
      candidates.clear();
      topDocument.SetAutofocusProcessed();
      return;
    }

    // 5. While candidates is not empty:
    while (!candidates.empty())
    {
      // 1. Let element be candidates[0].
      RefPtr<dom::Element> element = ShareRefPtr(candidates.front().get());

      // 2. Let doc be element's node document.
      // 3. If doc is not fully active, then remove element from candidates, and continue.
      if (!element || !element->NodeDocument().Frame())
      {
        candidates.erase(candidates.begin());
        continue;
      }

      // 4. If doc's node navigable's top-level traversable is not the same as topDocument's node navigable,
      // then remove element from candidates, and continue.
      if (&element->NodeDocument() != &topDocument)
      {
        candidates.erase(candidates.begin());
        continue;
      }

      // 5. If doc's script-blocking style sheet set is not empty, then return.
      // FIXME: Once style sheets load.

      // 6. Remove element from candidates.
      candidates.erase(candidates.begin());

      // 7. Let inclusiveAncestorDocuments be a list consisting of the active document of doc's inclusive
      //    ancestor navigables.
      // 8. If any Document in inclusiveAncestorDocuments has non-null target element, then continue.
      // (Without nested navigables doc is topDocument, whose target element step 4 checked.)

      // 9. Let target be element.
      // 10. If target is not a focusable area, then set target to the result of getting the focusable area
      // for
      //     target.
      element->NodeDocument().UpdateStyleIfNeeded();
      dom::Node *target = IsFocusableArea(*element) ? element.get() : GetFocusableArea(*element);

      // 11. If target is not null:
      if (target)
      {
        // 11.1. Empty candidates.
        candidates.clear();
        // 11.2. Set topDocument's autofocus processed flag to true.
        topDocument.SetAutofocusProcessed();
        // 11.3. Run the focusing steps for target.
        FocusingSteps(*target);
        return;
      }
    }
  }

  void FocusAlgorithms::FocusForClick(dom::Element &element) noexcept
  {
    dom::FocusOptions options {.Trigger = dom::FocusTrigger::Click};
    if (auto *candidate = ClickFocusableInclusiveAncestor(element))
    {
      FocusingSteps(*candidate, nullptr, options);
      return;
    }
    FocusingSteps(element.NodeDocument(), nullptr, options);
  }

  dom::Element *FocusAlgorithms::ClickFocusableInclusiveAncestor(dom::Element &element) noexcept
  {
    element.NodeDocument().UpdateStyleIfNeeded();
    for (auto *candidate = &element; candidate; candidate = candidate->ParentElementInComposedTree())
    {
      if (IsFocusableArea(*candidate) && IsClickFocusable(*candidate))
      {
        return candidate;
      }
      // A shadow host that delegates focus is focused through its focus delegate, by the focusing steps.
      if (IsShadowHostThatDelegatesFocus(*candidate) && !candidate->IsInUserAgentShadowTree()
          && GetFocusableArea(*candidate, dom::FocusTrigger::Click))
      {
        return candidate;
      }
    }
    return nullptr;
  }

  // https://html.spec.whatwg.org/#focus-navigation-scope-owner
  // A node is a focus navigation scope owner if it is a Document, a shadow host, a slot, or an element which
  // is the popover trigger of an element in the popover showing state.
  bool FocusAlgorithms::IsFocusNavigationScopeOwner(const dom::Node &node) noexcept
  {
    if (Is<dom::Document>(node))
    {
      return true;
    }
    auto *element = DynamicDowncast<dom::Element>(node);
    if (!element)
    {
      return false;
    }
    // Sequential navigation doesn't enter a control's user agent shadow tree, unless the control delegates
    // its focus to its parts.
    if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*element);
        shadowRoot && (shadowRoot->Mode() != dom::ShadowRootMode::UserAgent || shadowRoot->DelegatesFocus()))
    {
      return true;
    }
    // The trigger of a showing popover, whose scope the popover is in.
    if (auto *popover = DynamicDowncast<HTMLElement>(element->InvokedPopover());
        popover && popover->IsPopoverShowing() && popover->GetPopoverData()->Invoker() == element)
    {
      return true;
    }
    return Is<HTMLSlotElement>(*element);
  }

  // https://html.spec.whatwg.org/#associated-focus-navigation-owner
  dom::Node *FocusAlgorithms::AssociatedFocusNavigationOwner(const dom::Element &element) noexcept
  {
    // 1. If element's parent is null, then return null.
    auto *parent = element.ParentNode();
    if (!parent)
    {
      return nullptr;
    }
    // 2. If element's parent is a shadow host, then return element's assigned slot.
    if (dom::TreeQueries::IsShadowHost(*parent))
    {
      return dom::SlotAlgorithms::GetAssignedSlot(element);
    }
    // 3. If element's parent is a shadow root, then return the parent's host.
    if (auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(*parent))
    {
      return shadowRoot->Host();
    }
    // 4. If element's parent is the document element, then return the parent's node document.
    if (parent == dom::TreeQueries::DocumentElement(element.NodeDocument()))
    {
      return &element.NodeDocument();
    }
    // (The document element itself, which the specification leaves out, is in the document's scope.)
    if (Is<dom::Document>(*parent))
    {
      return parent;
    }
    // 5. If element is in the popover showing state and has a popover trigger set, then return element's
    //    popover trigger.
    if (auto *popover = DynamicDowncast<HTMLElement>(element); popover && popover->IsPopoverShowing())
    {
      if (auto *trigger = popover->GetPopoverData()->Invoker())
      {
        return trigger;
      }
    }
    // 6. Return element's parent's associated focus navigation owner.
    auto *parentElement = DynamicDowncast<dom::Element>(*parent);
    return parentElement ? AssociatedFocusNavigationOwner(*parentElement) : nullptr;
  }

  // https://html.spec.whatwg.org/#sequentially-focusable
  // (The user agent determines every area in its document's sequential focus navigation order sequentially
  // focusable.)
  bool FocusAlgorithms::IsSequentiallyFocusable(const dom::Element &element) noexcept
  {
    return IsFocusableArea(element) && TabIndexValue(element).value_or(0) >= 0;
  }

  // https://html.spec.whatwg.org/#sequential-focus-navigation-order
  // https://html.spec.whatwg.org/#tabindex-ordered-focus-navigation-scope
  // https://html.spec.whatwg.org/#flattened-tabindex-ordered-focus-navigation-scope
  List<dom::Element *> FocusAlgorithms::SequentialFocusNavigationOrder(dom::Document &document) noexcept
  {
    // Each focus navigation scope's elements, in shadow-including tree order.
    HashMap<const dom::Node *, List<dom::Element *>> scopes;
    for (auto *node = dom::TreeQueries::NextShadowIncluding(document); node;
         node = dom::TreeQueries::NextShadowIncluding(*node))
    {
      if (auto *element = DynamicDowncast<dom::Element>(*node))
      {
        if (auto *owner = AssociatedFocusNavigationOwner(*element))
        {
          scopes[owner].push_back(element);
        }
      }
    }

    List<dom::Element *> result;
    auto appendFlattenedScope = [&](this auto &self, const dom::Node &owner) -> void
    {
      auto scope = scopes.find(&owner);
      if (scope == scopes.end())
      {
        return;
      }

      // The tabindex-ordered focus navigation scope: the scope owners and focusable areas in the scope,
      // without those with negative tabindex values; positive values first, ascending, then zero and omitted
      // values (owners without one count as zero), each in shadow-including tree order.
      auto orderingKey = [](const dom::Element &element)
      {
        auto value = TabIndexValue(element).value_or(0);
        return value > 0 ? value : std::numeric_limits<int32>::max();
      };
      List<dom::Element *> ordered;
      for (auto *element : scope->second)
      {
        if ((IsFocusNavigationScopeOwner(*element) || IsFocusableArea(*element))
            && TabIndexValue(*element).value_or(0) >= 0)
        {
          ordered.push_back(element);
        }
      }
      std::ranges::stable_sort(ordered, [&](const dom::Element *a, const dom::Element *b)
                               { return orderingKey(*a) < orderingKey(*b); });

      // Flattened:
      // 1. Let result be a clone of owner's tabindex-ordered focus navigation scope.
      // 2. For each item of result:
      // 2.1. If item is not a focus navigation scope owner, then continue.
      // 2.2. If item is not a focusable area, then replace item with all of the items in item's flattened
      //      tabindex-ordered focus navigation scope.
      // 2.3. Otherwise, insert the contents of item's flattened tabindex-ordered focus navigation scope after
      //      item.
      for (auto *item : ordered)
      {
        if (!IsFocusNavigationScopeOwner(*item) || IsFocusableArea(*item))
        {
          result.push_back(item);
        }
        if (IsFocusNavigationScopeOwner(*item))
        {
          self(*item);
        }
      }
    };
    appendFlattenedScope(document);
    return result;
  }

  // https://html.spec.whatwg.org/#selection-mechanism-sequential
  bool FocusAlgorithms::SequentialNavigation(dom::Document &document, dom::FocusDirection direction) noexcept
  {
    krys_debug_assert(direction == dom::FocusDirection::Forward
                      || direction == dom::FocusDirection::Backward);
    // 3. Let direction be "forward" if the user requested the next control, and "backward" if the user
    //    requested the previous control.
    bool forward = direction == dom::FocusDirection::Forward;

    // Focusability depends on style.
    document.UpdateStyleIfNeeded();
    auto order = SequentialFocusNavigationOrder(document);

    // 1. Let starting point be the currently focused area of a top-level traversable, if the user requested
    // to
    //    move focus sequentially from there, or else the top-level traversable itself, if the user instead
    //    requested to move focus from outside the top-level traversable.
    // 2. If there is a sequential focus navigation starting point defined and it is inside starting point,
    //    then let starting point be the sequential focus navigation starting point instead.
    // (The document stands for the traversable, and the focused area is its focused element or viewport.)
    using Position = dom::SequentialFocusNavigationStartingPointPosition;
    dom::Node *startingPoint = document.FocusedElement();
    if (!startingPoint)
    {
      startingPoint = &document;
    }
    auto position = Position::At;
    if (auto *point = document._sequentialFocusNavigationStartingPoint.get();
        point && point->IsConnected()
        && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(*startingPoint, *point))
    {
      startingPoint = point;
      position = document._sequentialFocusNavigationStartingPointPosition;
    }

    // 4. Loop: Let selection mechanism be "sequential" if starting point is a navigable or if starting point
    //    is in its Document's sequential focus navigation order. Otherwise, starting point is not in its
    //    Document's sequential focus navigation order; let selection mechanism be "DOM".
    // 5. Let candidate be the result of running the sequential navigation search algorithm with starting
    //    point, direction, and selection mechanism.
    // https://html.spec.whatwg.org/#sequential-navigation-search-algorithm
    // (A suitable sequentially focusable area is one in the order, whose areas aren't inert.)
    dom::Element *candidate = nullptr;
    auto inOrder = std::ranges::find(order, startingPoint);
    if (startingPoint == &document && position == Position::At)
    {
      // starting point is a navigable: the first or last suitable sequentially focusable area in starting
      // point's active document.
      if (!order.empty())
      {
        candidate = forward ? order.front() : order.back();
      }
    }
    else if (position == Position::At && inOrder != order.end())
    {
      // selection mechanism is "sequential": the first suitable sequentially focusable area after starting
      // point, or the last before it, in starting point's Document's sequential focus navigation order.
      if (forward && std::next(inOrder) != order.end())
      {
        candidate = *std::next(inOrder);
      }
      else if (!forward && inOrder != order.begin())
      {
        candidate = *std::prev(inOrder);
      }
    }
    else
    {
      // selection mechanism is "DOM": the suitable sequentially focusable area that appears nearest after
      // starting point, or before it, in starting point's Document, in shadow-including tree order.
      List<dom::Node *> nodes;
      for (dom::Node *node = &document; node; node = dom::TreeQueries::NextShadowIncluding(*node))
      {
        nodes.push_back(node);
      }
      auto start = std::ranges::find(nodes, startingPoint);
      if (start != nodes.end())
      {
        auto index = static_cast<size_t>(start - nodes.begin());
        // The first node after and the last node before the starting point.
        size_t after = index + 1;
        size_t before = index;
        if (position == Position::After)
        {
          while (after < nodes.size()
                 && dom::TreeQueries::IsShadowIncludingDescendant(*nodes[after], *startingPoint))
          {
            ++after;
          }
          before = after;
        }
        else if (position == Position::Inside)
        {
          before = index + 1;
        }
        auto isCandidate = [&](dom::Node *node)
        {
          return std::ranges::find(order, node) != order.end();
        };
        if (forward)
        {
          for (size_t i = after; i < nodes.size() && !candidate; ++i)
          {
            candidate = isCandidate(nodes[i]) ? static_cast<dom::Element *>(nodes[i]) : nullptr;
          }
        }
        else
        {
          for (size_t i = before; i-- > 0 && !candidate;)
          {
            candidate = isCandidate(nodes[i]) ? static_cast<dom::Element *>(nodes[i]) : nullptr;
          }
        }
      }
    }

    // 7. Otherwise, unset the sequential focus navigation starting point.
    if (!candidate)
    {
      document._sequentialFocusNavigationStartingPoint = nullptr;
      // 8. If starting point is a top-level traversable, or a focusable area in the top-level traversable,
      // the
      //    user agent should transfer focus to its own controls appropriately (if any), honouring direction,
      //    and then return. If the user agent has no sequentially focusable controls, then the user agent may
      //    instead restart these steps with the starting point being the top-level traversable itself.
      // (There are no user agent controls, so it restarts from the traversable: the first or last area.)
      // 9. Otherwise, starting point is a focusable area in a child navigable. Set starting point to that
      // child
      //    navigable's parent and return to the step labeled loop.
      // FIXME: Child navigables, once there are any.
      if (order.empty())
      {
        return false;
      }
      candidate = forward ? order.front() : order.back();
    }

    // 6. If candidate is not null, then run the focusing steps for candidate and return.
    FocusingSteps(*candidate, nullptr, {.Direction = direction, .Visibility = dom::FocusVisibility::Visible});
    // What navigation focuses is shown focused (scrolled into view, or with a caret in it), as `focus()`
    // shows it.
    if (RefPtr<dom::Element> focused = ShareRefPtr(document.FocusedElement()))
    {
      focused->FindTargetAndUpdateFocusAppearance(dom::SelectionRestorationMode::SelectAll,
                                                  SelectionRevealMode::Reveal);
    }
    return true;
  }

  namespace
  {
    void UpdateFocusCandidateIfNeeded(dom::FocusDirection direction, const FocusCandidate &current,
                                      FocusCandidate &candidate, FocusCandidate &closest) noexcept
    {
      krys_debug_assert(candidate.VisibleNode->Renderer());

      // Ignore off screen child nodes of containers that do not scroll (overflow:hidden)
      if (candidate.IsOffscreen && !CanBeScrolledIntoView(direction, candidate))
      {
        return;
      }

      DistanceDataForNode(direction, current, candidate);
      if (candidate.Distance == MaxDistance())
      {
        return;
      }

      if (candidate.IsOffscreenAfterScrolling && candidate.Alignment < RectsAlignment::Full)
      {
        return;
      }

      if (closest.IsNull())
      {
        closest = candidate;
        return;
      }

      LayoutRect intersectionRect = Intersection(candidate.Rect, closest.Rect);
      if (!intersectionRect.IsEmpty() && !AreElementsOnSameLine(closest, candidate))
      {
        // If 2 nodes are intersecting, do hit test to find which node in on top.
        auto center = FlooredIntPoint(intersectionRect.Center()); // FIXME: Would roundedIntPoint be better?
        constexpr EnumSet<HitTestRequest::Type> hitType {
          HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active, HitTestRequest::Type::IgnoreClipping,
          HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::AllowChildFrameContent};
        // The main frame: there's one frame.
        auto *frame = candidate.VisibleNode->NodeDocument().Frame();
        if (!frame)
        {
          return;
        }
        HitTestResult result = frame->GetEventHandler().HitTestResultAtPoint(LayoutPoint(center), hitType);
        if (candidate.VisibleNode->Contains(result.InnerNode()))
        {
          closest = candidate;
          return;
        }
        if (closest.VisibleNode->Contains(result.InnerNode()))
        {
          return;
        }
      }

      if (candidate.Alignment == closest.Alignment)
      {
        if (candidate.Distance < closest.Distance)
        {
          closest = candidate;
        }
        return;
      }

      if (candidate.Alignment > closest.Alignment)
      {
        closest = candidate;
      }
    }

    void FindFocusCandidateInContainer(const dom::ContainerNode &container, const LayoutRect &startingRect,
                                       dom::FocusDirection direction, FocusCandidate &closest) noexcept
    {
      auto *focusedNode = container.NodeDocument().FocusedElement();

      FocusCandidate current;
      current.Rect = startingRect;
      current.FocusableNode = CreateWeakPtr(focusedNode);
      current.VisibleNode = CreateWeakPtr(focusedNode);

      auto next = [&](const dom::Element &element) -> dom::Element *
      {
        dom::Node *node = CanScrollInDirection(element, direction)
                            ? dom::TreeQueries::NextSkippingChildren(element, &container)
                            : dom::TreeQueries::Next(element, &container);
        while (node && !Is<dom::Element>(*node))
        {
          node = dom::TreeQueries::Next(*node, &container);
        }
        return static_cast<dom::Element *>(node);
      };
      dom::Node *first = dom::TreeQueries::Next(container, &container);
      while (first && !Is<dom::Element>(*first))
      {
        first = dom::TreeQueries::Next(*first, &container);
      }
      for (auto *element = static_cast<dom::Element *>(first); element; element = next(*element))
      {
        if (element == focusedNode)
        {
          continue;
        }

        if (!FocusAlgorithms::IsSequentiallyFocusable(*element) && !CanScrollInDirection(*element, direction))
        {
          continue;
        }

        FocusCandidate candidate = FocusCandidate(element, direction);
        if (candidate.IsNull())
        {
          continue;
        }

        if (!IsValidCandidate(direction, current, candidate))
        {
          continue;
        }

        candidate.EnclosingScrollableBox = CreateWeakPtr(const_cast<dom::ContainerNode *>(&container));
        UpdateFocusCandidateIfNeeded(direction, current, candidate, closest);
      }
    }

    bool AdvanceFocusDirectionallyInContainer(const dom::ContainerNode &container,
                                              const LayoutRect &startingRect,
                                              dom::FocusDirection direction) noexcept
    {
      LayoutRect newStartingRect = startingRect;

      if (startingRect.IsEmpty())
      {
        newStartingRect = VirtualRectForDirection(direction, NodeRectInAbsoluteCoordinates(container));
      }

      // Find the closest node within current container in the direction of the navigation.
      FocusCandidate focusCandidate;
      FindFocusCandidateInContainer(container, newStartingRect, direction, focusCandidate);

      if (focusCandidate.IsNull())
      {
        // Nothing to focus, scroll if possible.
        // NOTE: If no scrolling is performed (i.e. scrollInDirection returns false), the
        // spatial navigation algorithm will skip this container.
        return ScrollInDirection(container, direction);
      }

      if (RefPtr<dom::Element> visibleNode = ShareRefPtr(focusCandidate.VisibleNode.get());
          visibleNode && CanScrollInDirection(*visibleNode, direction))
      {
        if (focusCandidate.IsOffscreenAfterScrolling)
        {
          (void)ScrollInDirection(*visibleNode, direction);
          return true;
        }
        // Navigate into a new scrollable container.
        LayoutRect newRect;
        auto *focusedElement = container.NodeDocument().FocusedElement();
        if (focusedElement && !HasOffscreenRect(*focusedElement))
        {
          newRect = NodeRectInAbsoluteCoordinates(*focusedElement, true);
        }
        return AdvanceFocusDirectionallyInContainer(*visibleNode, newRect, direction);
      }
      if (focusCandidate.IsOffscreenAfterScrolling)
      {
        RefPtr<dom::ContainerNode> scrollableBox = ShareRefPtr(focusCandidate.EnclosingScrollableBox.get());
        (void)ScrollInDirection(*scrollableBox, direction);
        return true;
      }

      // We found a new focus node, navigate to it.
      RefPtr<dom::Element> element = ShareRefPtr(focusCandidate.FocusableNode.get());
      FocusAlgorithms::FocusingSteps(*element, nullptr,
                                     {.SelectionRestoration = dom::SelectionRestorationMode::SelectAll,
                                      .Direction = direction,
                                      .Visibility = dom::FocusVisibility::Visible});
      if (element->NodeDocument().FocusedElement() == element.get())
      {
        element->FindTargetAndUpdateFocusAppearance(dom::SelectionRestorationMode::SelectAll,
                                                    SelectionRevealMode::Reveal);
      }
      return true;
    }
  }

  bool FocusAlgorithms::DirectionalNavigation(dom::Document &document, dom::FocusDirection direction) noexcept
  {
    krys_debug_assert(direction == dom::FocusDirection::Up || direction == dom::FocusDirection::Down
                      || direction == dom::FocusDirection::Left || direction == dom::FocusDirection::Right);
    if (!document.View())
    {
      return false;
    }

    (void)document.UpdateLayoutIgnorePendingStylesheets();

    // Figure out the starting rect.
    RefPtr<dom::ContainerNode> container = ShareRefPtr(&document);
    LayoutRect startingRect;
    if (auto *focusedElement = document.FocusedElement())
    {
      if (!HasOffscreenRect(*focusedElement))
      {
        container =
          ShareRefPtr(ScrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, *focusedElement));
        startingRect = NodeRectInAbsoluteCoordinates(*focusedElement, true /* ignore border */);
      }
      else if (auto *area = DynamicDowncast<HTMLAreaElement>(*focusedElement); area && area->ImageElement())
      {
        container = ShareRefPtr(
          ScrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, *area->ImageElement()));
        startingRect = VirtualRectForAreaElementAndDirection(area, direction);
      }
    }

    krys_debug_assert(container);
    bool consumed = false;
    do
    {
      consumed = AdvanceFocusDirectionallyInContainer(*container, startingRect, direction);
      (void)document.UpdateLayoutIgnorePendingStylesheets();
      startingRect = NodeRectInAbsoluteCoordinates(*container, true /* ignore border */);
      container = ShareRefPtr(ScrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, *container));
    } while (!consumed && container);

    return consumed;
  }

  void FocusAlgorithms::SystemFocusChanged(dom::Document &document, bool focused) noexcept
  {
    if (document.IsFocusedAndActive() == focused)
    {
      return;
    }
    document.SetFocusedAndActive(focused);

    // The focused element gets `blur` before the window, and `focus` after it.
    if (RefPtr<dom::Element> focusedElement = ShareRefPtr(document.FocusedElement());
        !focused && focusedElement)
    {
      FireFocusEvent(dom::DOMStringAtom(u8"blur"), *focusedElement, nullptr);
    }
    if (RefPtr<LocalDOMWindow> window = ShareRefPtr(document.Window()))
    {
      dom::FocusEventInit attributes;
      attributes.View = window.get();
      attributes.Composed = true;
      (void)dom::EventDispatcher::FireEvent<dom::FocusEvent>(
        dom::DOMStringAtom(focused ? u8"focus" : u8"blur"), *window, attributes);
    }
    // NOTE: the window's listeners may have moved the focus.
    if (RefPtr<dom::Element> focusedElement = ShareRefPtr(document.FocusedElement());
        focused && focusedElement)
    {
      FireFocusEvent(dom::DOMStringAtom(u8"focus"), *focusedElement, nullptr);
    }
  }

  void FocusAlgorithms::DesignateFocusedArea(dom::Document &document, dom::Element *element,
                                             dom::FocusVisibility visibility) noexcept
  {
    // The sequential focus navigation starting point follows the focused element, so navigation continues
    // from it (as other engines do; the specification only uses the starting point inside the focused area).
    RefPtr<dom::Element> oldFocusedElement = std::exchange(document._focusedElement, ShareRefPtr(element));
    if (oldFocusedElement)
    {
      oldFocusedElement->SetFocus(false);
      document.SetSequentialFocusNavigationStartingPoint(nullptr);

      document.ScheduleContentRelevancyUpdate(dom::ContentRelevancy::Focused);
    }
    if (element)
    {
      document.SetSequentialFocusNavigationStartingPoint(element);
      element->SetFocus(true, visibility);

      document.ScheduleContentRelevancyUpdate(dom::ContentRelevancy::Focused);
    }

    if (auto *editorClient = document.GetEditor().Client())
    {
      editorClient->SetInputMethodState(element);
    }
    if (auto *cache = document.ExistingAXObjectCache())
    {
      cache->OnFocusChange(oldFocusedElement.get(), element);
    }
  }
}
