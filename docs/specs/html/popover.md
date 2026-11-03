# HTML review checklist: 6.12 The popover attribute

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 26 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html popover` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [6.12 The popover attribute](https://html.spec.whatwg.org/#the-popover-attribute)

Status: reviewed (2026-11-01)
> `HTMLElement`'s popover members (`ShowPopoverInternal`, `HidePopoverInternal`, `CheckPopoverValidity`, `RunPopoverFocusingSteps`), the element's `PopoverData`, and `Document`'s popover stacks; the showing auto and hint popover lists are the top layer's. The toggle task tracker is `ToggleEventTask`, which skips the earlier task rather than removing it. There's no fullscreen flag to check.
<!-- definitions: attr-popover popover-visibility-state popover-hidden-state popover-showing-state popover-pointerdown-target popover-trigger popover-hiding popover-toggle-task-tracker popover-close-watcher opened-in-popover-mode hiding-popover-nesting-count showing-popover -->

- [x] [No Popover](https://html.spec.whatwg.org/#attr-popover-none-state) — The attribute's missing value default is the No Popover state, its invalid value default is the Manual state, and its empty value default is the Auto state.

- [x] [HTMLElement.popover](https://html.spec.whatwg.org/#dom-popover) — The popover IDL attribute must reflect the popover attribute, limited to only known values.

- [x] [The following attribute change steps, given element, localName, oldValue, value…](https://html.spec.whatwg.org/#hint-stack-parent) — The following attribute change steps, given element, localName, oldValue, value, and namespace, are used for all HTML elements:
  - 1. If namespace is not null, then return.
  - 2. If localName is not popover, then return.
  - 3. If element's popover visibility state is in the showing state and oldValue and value are in different states, then run the hide popover algorithm given element, true, true, false, and null.

- [x] [HTMLElement.showPopover(options)](https://html.spec.whatwg.org/#dom-showpopover) — The showPopover(options) method steps are:
  - 1. Let source be options["source"] if it exists; otherwise, null.
  - 2. Run show popover given this, true, and source.

- [x] [show popover](https://html.spec.whatwg.org/#show-popover) — To show popover, given an HTML element element, a boolean throwExceptions, and an HTML element or null source:
  - 1. Let document be element's node document.
  - 2. If document's showing popover is true, or document's hiding popover nesting count is not 0:
    - 2a. If throwExceptions is true, then throw a "InvalidStateError" DOMException.
    - 2b. Return.
  - 3. Let validityResult be the result of running check popover validity given element, false, and null. If this throws an exception, catch it, and set validityResult to that exception.
  - 4. If validityResult is not true:
    - 4a. If throwExceptions is true and validityResult is a DOMException, then throw validityResult.
    - 4b. Return.
  - 5. Assert: element's popover trigger is null.
  - 6. Assert: element is not in document's top layer.
  - 7. Set document's showing popover to true.
  - 8. Let cleanupShowingSteps be the following steps:
    - 8a. Set document's showing popover to false.
  - 9. If the result of firing an event named beforetoggle, using ToggleEvent, with the cancelable attribute initialized to true, the oldState attribute initialized to "closed", the newState attribute initialized to "open", and the source attribute initialized to source at element is false, then run cleanupShowingSteps and return.
  - 10. Set validityResult to the result of running check popover validity given element, false, and document. If this throws an exception, catch it, and set validityResult to that exception.
  - 11. If validityResult is not true:
    - 11a. Run cleanupShowingSteps.
    - 11b. If throwExceptions is true and validityResult is a DOMException, then throw validityResult.
    - 11c. Return.
  - 12. Let shouldRestoreFocus be false.
  - 13. Let originalType be the current state of element's popover attribute.
  - 14. Let effectiveType be originalType.
  - 15. If originalType is Auto or Hint:
    - 15a. Let ancestor be the result of running the topmost popover ancestor algorithm given element, source, and true.
    - 15b. If all of the following are true: ancestor is not null;; ancestor's opened in popover mode is "hint"; and; effectiveType is the Auto state, then set effectiveType to the Hint state.
    - 15c. Run hide popover stack until given document, ancestor, Hint, shouldRestoreFocus, and true.
    - 15d. If effectiveType is the Auto state, then run hide popover stack until given document, ancestor, Auto, shouldRestoreFocus, and true.
    - 15e. If originalType is not equal to the value of element's popover attribute:
      - 15e.i. Run cleanupShowingSteps.
      - 15e.ii. If throwExceptions is true, then throw an "InvalidStateError" DOMException.
      - 15e.iii. Return.
    - 15f. Set validityResult to the result of running check popover validity given element, false, and document. If this throws an exception, catch it, and set validityResult to that exception.
    - 15g. If validityResult is not true:
      - 15g.i. Run cleanupShowingSteps.
      - 15g.ii. If throwExceptions is true and validityResult is a DOMException, then throw validityResult.
      - 15g.iii. Return.
    - 15h. If the result of running topmost auto or hint popover on document is null, then set shouldRestoreFocus to true.
    - 15i. If effectiveType is Auto: Otherwise:
      - 15i.i. Assert: document's showing auto popover list does not contain element.
      - 15i.ii. Set element's opened in popover mode to "auto".
      - 15i.i. Assert: effectiveType is Hint.
      - 15i.ii. Assert: document's showing hint popover list does not contain element.
      - 15i.iii. Set element's opened in popover mode to "hint".
    - 15j. Set element's popover close watcher to the result of establishing a close watcher given element's relevant global object, with: cancelAction being to return true.; closeAction being to hide a popover given element, true, true, false, and null.; getEnabledState being to return true.
  - 16. Set element's previously focused element to null.
  - 17. Let originallyFocusedElement be document's focused area of the document's DOM anchor.
  - 18. Add an element to the top layer given element.
  - 19. If effectiveType is Hint and ancestor's opened in popover mode is "auto", then set document's hint stack parent to ancestor.
  - 20. Set element's popover visibility state to showing.
  - 21. Set element's popover trigger to source.
  - 22. Set element's implicit anchor element to source.
  - 23. Run the popover focusing steps given element.
  - 24. If shouldRestoreFocus is true and element's popover attribute is not in the No Popover state, then set element's previously focused element to originallyFocusedElement.
  - 25. Run cleanupShowingSteps.
  - 26. Queue a popover toggle event task given element, "closed", "open", and source.

- [x] [queue a popover toggle event task](https://html.spec.whatwg.org/#queue-a-popover-toggle-event-task) — To queue a popover toggle event task given an element element, a string oldState, a string newState, and an Element or null source:
  - 1. If element's popover toggle task tracker is not null:
    - 1a. Set oldState to element's popover toggle task tracker's old state.
    - 1b. Remove element's popover toggle task tracker's task from its task queue.
    - 1c. Set element's popover toggle task tracker to null.
  - 2. Queue an element task given the DOM manipulation task source and element to run the following steps:
    - 2a. Fire an event named toggle at element, using ToggleEvent, with the oldState attribute initialized to oldState, the newState attribute initialized to newState, and the source attribute initialized to source.
    - 2b. Set element's popover toggle task tracker to null.
  - 3. Set element's popover toggle task tracker to a struct with task set to the just-queued task and old state set to oldState.

- [x] [HTMLElement.hidePopover()](https://html.spec.whatwg.org/#dom-hidepopover) — The hidePopover() method steps are:
  - 1. Run the hide popover algorithm given this, true, true, true, and null.

- [x] [hide a popover](https://html.spec.whatwg.org/#hide-popover-algorithm) — To hide a popover given an HTML element element, a boolean focusPreviousElement, a boolean fireEvents, a boolean throwExceptions, and an HTML element or null source:
  - 1. Let validityResult be the result of running check popover validity given element, true, and null. If this throws an exception, catch it, and let validityResult be that exception.
  - 2. If validityResult is not true:
    - 2a. If throwExceptions is true and validityResult is a DOMException, then throw validityResult.
    - 2b. Return.
  - 3. Let document be element's node document.
  - 4. Let nestedHide be element's popover hiding.
  - 5. Set element's popover hiding to true.
  - 6. If nestedHide is true, then set fireEvents to false.
  - 7. Increment document's hiding popover nesting count.
  - 8. Let cleanupSteps be the following steps:
    - 8a. If nestedHide is false, then set element's popover hiding to false.
    - 8b. Decrement document's hiding popover nesting count.
  - 9. Let autoPopoverListContainsElement be true if document's showing auto popover list contains element; otherwise false.
  - 10. Let hintPopoverListContainsElement be true if document's showing hint popover list contains element; otherwise false.
  - 11. If element's opened in popover mode is "auto" or "hint":
    - 11a. If hintPopoverListContainsElement is true, then run hide popover stack until given document, element, Hint, focusPreviousElement, and fireEvents.
    - 11b. If element is document's hint stack parent, then run hide popover stack until given document, null, Hint, focusPreviousElement, and fireEvents.
    - 11c. If autoPopoverListContainsElement is true, then run hide popover stack until given document, element, Auto, focusPreviousElement, and fireEvents.
    - 11d. Set validityResult to the result of running check popover validity given element, true, and document. If this throws an exception, catch it, and set validityResult to that exception.
    - 11e. If validityResult is not true:
      - 11e.i. Run cleanupSteps.
      - 11e.ii. If throwExceptions is true and validityResult is a DOMException, then throw validityResult.
      - 11e.iii. Return.
  - 12. If fireEvents is true:
    - 12a. Fire an event named beforetoggle, using ToggleEvent, with the oldState attribute initialized to "open", the newState attribute initialized to "closed", and the source attribute set to source at element.
    - 12b. Set validityResult to the result of running check popover validity given element, true, and document. If this throws an exception, catch it, and set validityResult to that exception.
    - 12c. If validityResult is not true:
      - 12c.i. Run cleanupSteps.
      - 12c.ii. If throwExceptions is true and validityResult is a DOMException, then throw validityResult.
      - 12c.iii. Return.
    - 12d. Request an element to be removed from the top layer given element.
    - 12e. Set element's implicit anchor element to null.
  - 13. Otherwise, remove an element from the top layer immediately given element.
  - 14. Set element's popover trigger to null.
  - 15. Set element's opened in popover mode to null.
  - 16. Set element's popover visibility state to hidden.
  - 17. If element is document's hint stack parent, or document's showing hint popover list is empty, then set document's hint stack parent to null.
  - 18. If fireEvents is true, then queue a popover toggle event task given element, "open", "closed", and source.
  - 19. Let previouslyFocusedElement be element's previously focused element.
  - 20. If previouslyFocusedElement is not null:
    - 20a. Set element's previously focused element to null.
    - 20b. If focusPreviousElement is true and document's focused area of the document's DOM anchor is a shadow-including inclusive descendant of element, then run the focusing steps for previouslyFocusedElement; the viewport should not be scrolled by doing this step.
  - 21. If element's popover close watcher is not null:
    - 21a. Destroy element's popover close watcher.
    - 21b. Set element's popover close watcher to null.
  - 22. Run cleanupSteps.

- [x] [HTMLElement.togglePopover(options)](https://html.spec.whatwg.org/#dom-togglepopover) — The togglePopover(options) method steps are:
  - 1. Let force be null.
  - 2. If options is a boolean, set force to options.
  - 3. Otherwise, if options["force"] exists, set force to options["force"].
  - 4. Let source be options["source"] if it exists; otherwise, null.
  - 5. If this's popover visibility state is showing, and force is null or false, then run the hide popover algorithm given this, true, true, true, and null.
  - 6. Otherwise, if force is null or true, then run show popover given this, true, and source.
  - 7. Otherwise:
    - 7a. Let expectedToBeShowing be true if this's popover visibility state is showing; otherwise false.
    - 7b. Run check popover validity given this, expectedToBeShowing, and null.
  - 8. Return true if this's popover visibility state is showing; otherwise false.

- [x] [hide popovers until](https://html.spec.whatwg.org/#hide-popovers-until) — To hide popovers until, given a Document document, an HTML element or null endpoint, a boolean focusPreviousElement, and a boolean fireEvents:
  - 1. Let endpointIsHint be true if document's showing hint popover list contains endpoint; otherwise false.
  - 2. Run hide popover stack until given document, endpoint, Hint, focusPreviousElement, and fireEvents.
  - 3. Let autoEndpoint be endpoint.
  - 4. If endpointIsHint is true, then set autoEndpoint to document's hint stack parent.
  - 5. Run hide popover stack until given document, autoEndpoint, Auto, focusPreviousElement, and fireEvents.

- [x] [hide popover stack until](https://html.spec.whatwg.org/#hide-popover-stack-until) — To hide popover stack until, given a Document document, an HTML element or null endpoint, an Auto or Hint stackType, a boolean focusPreviousElement, and a boolean fireEvents:
  - 1. Let popoverList be document's showing auto popover list if stackType is Auto; otherwise document's showing hint popover list.
  - 2. Let lastHideIndex be 0 if popoverList does not contain endpoint; otherwise the index of endpoint in popoverList plus 1.
  - 3. Let toHide be a slice of popoverList from lastHideIndex, in reverse order.
  - 4. Let toRemain be a slice of popoverList from 0 to lastHideIndex.
  - 5. For each popover of toHide: run the hide popover algorithm given popover, focusPreviousElement, fireEvents, false, and null.
  - 6. Let newPopoverList be document's showing auto popover list if stackType is Auto; otherwise document's showing hint popover list.
  - 7. Let toCheck be newPopoverList in reverse order.
  - 8. For each popover of toCheck:
    - 8a. If toRemain contains popover, then continue.
    - 8b. Run the hide popover algorithm given popover, focusPreviousElement, false, false, and null.

- [x] [topmost popover ancestor](https://html.spec.whatwg.org/#topmost-popover-ancestor) — To find the topmost popover ancestor, given a Node newPopoverOrTopLayerElement, an HTML element or null source, and a boolean isPopover, perform the following steps. They return an HTML element or null.
  - 1. If isPopover is true:
    - 1a. Assert: newPopoverOrTopLayerElement is an HTML element.
    - 1b. Assert: newPopoverOrTopLayerElement's popover attribute is not in the No Popover State or the Manual state.
    - 1c. Assert: newPopoverOrTopLayerElement's popover visibility state is not in the popover showing state.
  - 2. Otherwise:
    - 2a. Assert: source is null.
  - 3. Let document be newPopoverOrTopLayerElement's node document.
  - 4. Let combinedPopovers be document's showing auto popover list extended with document's showing hint popover list.
  - 5. Let popoverAncestorIndex be the index of the last item in combinedPopovers of which newPopoverOrTopLayerElement is a flat tree descendant, otherwise -1.
  - 6. Let sourceAncestorIndex be -1.
  - 7. If source is not null, then set sourceAncestorIndex to the index of the last item in combinedPopovers of which source is a flat tree descendant, otherwise -1.
  - 8. Let ancestorIndex be the maximum of popoverAncestorIndex and sourceAncestorIndex.
  - 9. If ancestorIndex is -1, then return null.
  - 10. Return combinedPopovers[ancestorIndex].

- [x] [nearest inclusive open popover](https://html.spec.whatwg.org/#nearest-inclusive-open-popover) — To find the nearest inclusive open popover given a Node node, perform the following steps. They return an HTML element or null.
  - 1. Let currentNode be node.
  - 2. While currentNode is not null:
    - 2a. If currentNode's opened in popover mode is "auto" or "hint", and currentNode's popover visibility state is showing, then return currentNode.
    - 2b. Set currentNode to currentNode's parent in the flat tree.
  - 3. Return null.

- [x] [find the topmost auto or hint popover](https://html.spec.whatwg.org/#topmost-auto-or-hint-popover) — To find the topmost auto or hint popover given a Document document, perform the following steps. They return an HTML element or null.
  - 1. If document's showing hint popover list is not empty, then return document's showing hint popover list's last element.
  - 2. If document's showing auto popover list is not empty, then return document's showing auto popover list's last element.
  - 3. Return null.

- [x] [popover focusing steps](https://html.spec.whatwg.org/#popover-focusing-steps) — To perform the popover focusing steps for an HTML element subject:
  - 1. If the allow focus steps given subject's node document return false, then return.
  - 2. If subject is a dialog element, then run the dialog focusing steps given subject and return.
  - 3. If subject has the autofocus attribute, then let control be subject.
  - 4. Otherwise, let control be the autofocus delegate for subject given "other".
  - 5. If control is null, then return.
  - 6. Run the focusing steps given control.
  - 7. Let topDocument be control's node navigable's top-level traversable's active document.
  - 8. If control's node document's origin is not the same as the origin of topDocument, then return.
  - 9. Empty topDocument's autofocus candidates.
  - 10. Set topDocument's autofocus processed flag to true.

- [x] [check popover validity](https://html.spec.whatwg.org/#check-popover-validity) — To check popover validity for an HTML element element given a boolean expectedToBeShowing and a Document or null expectedDocument, perform the following steps. They throw an exception or return a boolean.
  - 1. If element's popover attribute is in the No Popover state, then throw a "NotSupportedError" DOMException.
  - 2. If any of the following are true: expectedToBeShowing is true and element's popover visibility state is not showing; or; expectedToBeShowing is false and element's popover visibility state is not hidden, then return false.
  - 3. If any of the following are true: expectedToBeShowing is false and element is not connected;; expectedToBeShowing is false and element's node document is not fully active;; expectedDocument is not null and element's node document is not expectedDocument;; element is a dialog element and its is modal is set to true; or; element's fullscreen flag is set, then throw an "InvalidStateError" DOMExcepti…
  - 4. Return true.

- [x] [showing auto popover list](https://html.spec.whatwg.org/#auto-popover-list) — To get the showing auto popover list for a Document document:
  - 1. Let popovers be « ».
  - 2. For each Element element of document's top layer:
    - 2a. If all of the following are true: element is an HTML element;; element's opened in popover mode is "auto"; and; element's popover visibility state is showing, then append element to popovers.
  - 3. Return popovers.

- [x] [showing hint popover list](https://html.spec.whatwg.org/#showing-hint-popover-list) — To get the showing hint popover list for a Document document:
  - 1. Let popovers be « ».
  - 2. For each Element element of document's top layer:
    - 2a. If all of the following are true: element is an HTML element;; element's opened in popover mode is "hint"; and; element's popover visibility state is showing, then append element to popovers.
  - 3. Return popovers.

#### [6.12.1 The popover target attributes](https://html.spec.whatwg.org/#the-popover-target-attributes)

Status: reviewed (2026-11-01)
> `HTMLFormControlElement::PopoverTargetElement` and `HandlePopoverTargetAction`.
<!-- definitions: attr-popovertarget attr-popovertargetaction -->

- [x] [HTMLElement.popoverTargetAction](https://html.spec.whatwg.org/#dom-popovertargetaction) — The popoverTargetAction IDL attribute must reflect the popovertargetaction attribute, limited to only known values.

- [x] [popover target attribute activation behavior](https://html.spec.whatwg.org/#popover-target-attribute-activation-behavior) — To run the popover target attribute activation behavior given a Node node and a Node eventTarget:
  - 1. Let popover be node's popover target element.
  - 2. If popover is null, then return.
  - 3. If eventTarget is a shadow-including inclusive descendant of popover and popover is a shadow-including descendant of node, then return.
  - 4. If node's popovertargetaction attribute is in the show state and popover's popover visibility state is showing, then return.
  - 5. If node's popovertargetaction attribute is in the hide state and popover's popover visibility state is hidden, then return.
  - 6. If popover's popover visibility state is showing, then run the hide popover algorithm given popover, true, true, false, and node.
  - 7. Otherwise:
    - 7a. Let validity be the result of running check popover validity given popover, false, and null. If this throws an exception, catch it, and let validity be false.
    - 7b. If validity is true, then run show popover given popover, false, and node.

- [x] [popover target element](https://html.spec.whatwg.org/#popover-target-element) — To get the popover target element given a Node node, perform the following steps. They return an HTML element or null.
  - 1. If node is not a button, then return null.
  - 2. If node is disabled, then return null.
  - 3. If node has a form owner and node is a submit button, then return null.
  - 4. Let popoverElement be the result of running node's get the popovertarget-associated element.
  - 5. If popoverElement is null, then return null.
  - 6. If popoverElement's popover attribute is in the No Popover state, then return null.
  - 7. Return popoverElement.

#### [6.12.2 Popover light dismiss](https://html.spec.whatwg.org/#popover-light-dismiss)

Status: reviewed (2026-11-01)
> `Document::HandleLightDismiss` and `TopmostClickedPopover`; the topmost popover ancestor also counts a source that's an open popover (see `DIFFERENCES.md`).
> Fixed: a button's `commandfor` popover counted as the popover it targets whatever its command, and whatever its form owner and type.

- [x] [light dismiss open popovers](https://html.spec.whatwg.org/#light-dismiss-open-popovers) — To light dismiss open popovers, given a PointerEvent event:
  - 1. Assert: event's isTrusted attribute is true.
  - 2. Let target be event's target.
  - 3. Let document be target's node document.
  - 4. If the result of running topmost auto or hint popover given document is null, then return.
  - 5. If event's type is "pointerdown": set document's popover pointerdown target to the result of running topmost clicked popover given target.
  - 6. If event's type is "pointerup":
    - 6a. Let ancestor be the result of running topmost clicked popover given target.
    - 6b. Let sameTarget be true if ancestor is document's popover pointerdown target.
    - 6c. Set document's popover pointerdown target to null.
    - 6d. If sameTarget is false, then return.
    - 6e. Run hide popovers until given document, ancestor, false, and true.

- [x] [topmost clicked popover](https://html.spec.whatwg.org/#topmost-clicked-popover) — To find the topmost clicked popover, given a Node node:
  - 1. Let clickedPopover be the result of running nearest inclusive open popover given node.
  - 2. Let targetPopover be the result of running nearest inclusive target popover given node.
  - 3. If the result of getting the popover stack position given clickedPopover is greater than the result of getting the popover stack position given targetPopover, then return clickedPopover.
  - 4. Return targetPopover.

- [x] [get the popover stack position](https://html.spec.whatwg.org/#get-the-popover-stack-position) — To get the popover stack position, given an HTML element popover:
  - 1. Let hintList be popover's node document's showing hint popover list.
  - 2. Let autoList be popover's node document's showing auto popover list.
  - 3. If popover is in hintList, then return the index of popover in hintList + the size of autoList + 1.
  - 4. If popover is in autoList, then return the index of popover in autoList + 1.
  - 5. Return 0.

- [x] [nearest inclusive target popover](https://html.spec.whatwg.org/#nearest-inclusive-target-popover) — To find the nearest inclusive target popover given a Node node:
  - 1. Let currentNode be node.
  - 2. While currentNode is not null:
    - 2a. Let targetPopover be currentNode's target popover.
    - 2b. If targetPopover is not null and targetPopover's popover attribute is in the Auto state or the Hint state, and targetPopover's popover visibility state is showing, then return targetPopover.
    - 2c. Set currentNode to currentNode's ancestor in the flat tree.

- [x] [target popover](https://html.spec.whatwg.org/#target-popover) — To get the target popover given a Node node, perform the following steps. They return an HTML element or null.
  - 1. If node is not a button element, then return node's popover target element.
  - 2. If node is disabled, then return null.
  - 3. Let target be the result of running node's get the commandfor-associated element.
  - 4. If target is null, then return node's popover target element.
  - 5. If node has a form owner:
    - 5a. If element is a submit button, then return null.
    - 5b. If element's type attribute is in the Reset Button state, then return null.
    - 5c. If element's type attribute is in the Auto state, then return null.
  - 6. Let command be node's command attribute.
  - 7. If command is not in the Toggle Popover, Hide Popover, or Show Popover states, then return null.
  - 8. If target's popover attribute is in the No Popover state, then return null.
  - 9. Return target.
