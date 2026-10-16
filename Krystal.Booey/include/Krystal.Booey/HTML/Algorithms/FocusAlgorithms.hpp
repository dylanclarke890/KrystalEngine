#pragma once

#include "Krystal.Booey/DOM/FocusOptions.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class EventTarget;
  class Node;
}

namespace krys::boo::html
{
  /// @brief The focus processing model. A focusable area is an element, or a document standing for its
  /// viewport; a focus chain is a list of such nodes ending with the document.
  /// @see https://html.spec.whatwg.org/#focus
  class FocusAlgorithms
  {
  public:
    using FocusChain = List<Ref<dom::Node>>;

    /// @see https://html.spec.whatwg.org/#tabindex-value
    KRYS_NODISCARD static Maybe<int32> TabIndexValue(const dom::Element &element) noexcept;

    /// @brief Whether @p node is inert: an element with an `inert` attribute or a flat tree descendant of
    /// one.
    /// @see https://html.spec.whatwg.org/#inert
    // FIXME: Modal dialogs and fullscreen elements, once there are any.
    KRYS_NODISCARD static bool IsInert(const dom::Node &node) noexcept;

    /// @brief Whether @p element is a focusable area (it is focusable).
    /// @see https://html.spec.whatwg.org/#focusable-area
    KRYS_NODISCARD static bool IsFocusableArea(const dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#get-the-focusable-area
    KRYS_NODISCARD static dom::Node *
      GetFocusableArea(dom::Element &focusTarget,
                       dom::FocusTrigger focusTrigger = dom::FocusTrigger::Other) noexcept;

    /// @see https://html.spec.whatwg.org/#focus-delegate
    KRYS_NODISCARD static dom::Node *
      FocusDelegate(dom::Element &focusTarget,
                    dom::FocusTrigger focusTrigger = dom::FocusTrigger::Other) noexcept;

    /// @see https://html.spec.whatwg.org/#autofocus-delegate
    KRYS_NODISCARD static dom::Node *AutofocusDelegate(dom::ContainerNode &focusTarget,
                                                       dom::FocusTrigger focusTrigger) noexcept;

    /// @brief The focused area of the document if its window has system focus, or null.
    /// @see https://html.spec.whatwg.org/#currently-focused-area-of-a-top-level-traversable
    KRYS_NODISCARD static dom::Node *CurrentlyFocusedArea(dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#focus-chain
    KRYS_NODISCARD static FocusChain GetFocusChain(dom::Node &subject) noexcept;

    /// @see https://html.spec.whatwg.org/#focusing-steps
    static void FocusingSteps(dom::Node &newFocusTarget, dom::Node *fallbackTarget = nullptr,
                              const dom::FocusOptions &options = {}) noexcept;

    /// @see https://html.spec.whatwg.org/#unfocusing-steps
    static void UnfocusingSteps(dom::Element &oldFocusTarget) noexcept;

    /// @see https://html.spec.whatwg.org/#focus-update-steps
    static void FocusUpdateSteps(FocusChain oldChain, FocusChain newChain, dom::Node *newFocusTarget,
                                 const dom::FocusOptions &options = {}) noexcept;

    /// @brief Fires a focus event named @p e at @p target, and the `focusin`/`focusout` that goes with it.
    /// @see https://html.spec.whatwg.org/#fire-a-focus-event
    static void FireFocusEvent(dom::DOMStringAtom e, dom::Element &target,
                               dom::EventTarget *relatedTarget) noexcept;

    /// @see https://html.spec.whatwg.org/#has-focus-steps
    KRYS_NODISCARD static bool HasFocusSteps(const dom::Document &target) noexcept;

    /// @brief Focuses what the user activated with a pointer: the nearest click focusable area that is
    /// @p element or one of its flat tree ancestors, or the viewport if there's none.
    /// @see https://html.spec.whatwg.org/#click-focusable
    static void FocusForClick(dom::Element &element) noexcept;

  private:
    /// @brief Designates @p element (or the viewport, if it's null) as the focused area of @p document,
    /// updating the elements' focus state.
    static void DesignateFocusedArea(dom::Document &document, dom::Element *element,
                                     dom::FocusVisibility visibility) noexcept;
  };
}
