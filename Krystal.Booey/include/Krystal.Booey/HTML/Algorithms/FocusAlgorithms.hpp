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

    /// @brief Run when an element with the `autofocus` attribute is inserted into a document: it becomes one
    /// of the top-level document's autofocus candidates, unless autofocus was processed.
    /// @see https://html.spec.whatwg.org/#the-autofocus-attribute
    static void AutofocusInsertionSteps(dom::Element &element) noexcept;

    /// @brief Focuses the first of @p topDocument's autofocus candidates that's focusable, in the rendering
    /// update.
    /// @see https://html.spec.whatwg.org/#flush-autofocus-candidates
    static void FlushAutofocusCandidates(dom::Document &topDocument) noexcept;

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
    /// @brief The nearest click focusable area that is @p element or one of its flat tree ancestors, if any.
    KRYS_NODISCARD static dom::Element *ClickFocusableInclusiveAncestor(dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#focus-navigation-scope-owner
    // FIXME: Elements in the popover showing state with a popover trigger, once there are popovers.
    KRYS_NODISCARD static bool IsFocusNavigationScopeOwner(const dom::Node &node) noexcept;

    /// @see https://html.spec.whatwg.org/#associated-focus-navigation-owner
    KRYS_NODISCARD static dom::Node *AssociatedFocusNavigationOwner(const dom::Element &element) noexcept;

    /// @brief Whether @p element is a focusable area with a non-negative tabindex value (an omitted one
    /// counts as 0 for the elements that are focusable by default).
    /// @note Areas in a focus navigation scope whose owner has a negative tabindex value are sequentially
    /// focusable by this test but missing from the sequential focus navigation order.
    /// @see https://html.spec.whatwg.org/#sequentially-focusable
    KRYS_NODISCARD static bool IsSequentiallyFocusable(const dom::Element &element) noexcept;

    /// @brief The document's flattened tabindex-ordered focus navigation scope: its sequentially focusable
    /// areas, those with positive tabindex values first (ascending), then the rest in shadow-including tree
    /// order, each scope (shadow trees, slots) navigated as a whole.
    /// @see https://html.spec.whatwg.org/#sequential-focus-navigation-order
    KRYS_NODISCARD static List<dom::Element *>
      SequentialFocusNavigationOrder(dom::Document &document) noexcept;

    /// @brief Moves the focus to the next (@p direction `Forward`) or previous (`Backward`) suitable
    /// sequentially focusable area, from the focused area or the sequential focus navigation starting point
    /// inside it, wrapping around at the end of the document. The new focused element is focus visible.
    /// @return Whether there was an area to move to.
    /// @see https://html.spec.whatwg.org/#sequential-navigation
    // FIXME: Navigables (frames), and moving the focus out to the host, once it can take it.
    static bool SequentialNavigation(dom::Document &document, dom::FocusDirection direction) noexcept;

    /// @brief Updates @p document when its window gains or loses system focus: its focused and active flag,
    /// and a `focus` or `blur` event at its focused element, which stays focused.
    static void SystemFocusChanged(dom::Document &document, bool focused) noexcept;

  private:
    /// @brief Designates @p element (or the viewport, if it's null) as the focused area of @p document,
    /// updating the elements' focus state.
    static void DesignateFocusedArea(dom::Document &document, dom::Element *element,
                                     dom::FocusVisibility visibility) noexcept;
  };
}
