# HTML review checklist: 4.11 Interactive elements

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 34 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html interactive-elements` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.11 Interactive elements](https://html.spec.whatwg.org/#interactive-elements)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.11.1 The details element](https://html.spec.whatwg.org/#the-details-element)

Status: reviewed (2026-11-01)
> `HTMLDetailsElement`; the summary slot is `SlotForHostChild`, and the attribute change steps are `OnAttributeChanged`.
> The insertion steps run once it's connected (see `DIFFERENCES.md`).
<!-- definitions: attr-details-name attr-details-open details-notification-task-steps -->

- [x] [details name group](https://html.spec.whatwg.org/#details-name-group) — The details name group that contains a details element a also contains all the other details elements b that fulfill all of the following conditions:

- [x] [The following attribute change steps, given element, localName, oldValue, value…](https://html.spec.whatwg.org/#details-toggle-task-tracker) — The following attribute change steps, given element, localName, oldValue, value, and namespace, are used for all details elements:
  - 1. If namespace is not null, then return.
  - 2. If localName is name, then ensure details exclusivity by closing the given element if needed given element.
  - 3. If localName is open:
    - 3a. If one of oldValue or value is null and the other is not null, run the following steps, which are known as the details notification task steps, for this details element:
      - 3a.i. If oldValue is null, queue a details toggle event task given the details element, "closed", and "open".
      - 3a.ii. Otherwise, queue a details toggle event task given the details element, "open", and "closed".
    - 3b. If oldValue is null and value is not null, then ensure details exclusivity by closing other elements if needed given element.

- [x] [The details HTML element insertion steps, given insertedNode,](https://html.spec.whatwg.org/#the-details-element) — The details HTML element insertion steps, given insertedNode, are:
  - 1. Ensure details exclusivity by closing the given element if needed given insertedNode.

- [x] [queue a details toggle event task](https://html.spec.whatwg.org/#queue-a-details-toggle-event-task) — To queue a details toggle event task given a details element element, a string oldState, and a string newState:
  - 1. If element's details toggle task tracker is not null:
    - 1a. Set oldState to element's details toggle task tracker's old state.
    - 1b. Remove element's details toggle task tracker's task from its task queue.
    - 1c. Set element's details toggle task tracker to null.
  - 2. Queue an element task given the DOM manipulation task source and element to run the following steps:
    - 2a. Fire an event named toggle at element, using ToggleEvent, with the oldState attribute initialized to oldState and the newState attribute initialized to newState.
    - 2b. Set element's details toggle task tracker to null.
  - 3. Set element's details toggle task tracker to a struct with task set to the just-queued task and old state set to oldState.

- [x] [ensure details exclusivity by closing other elements if needed](https://html.spec.whatwg.org/#ensure-details-exclusivity-by-closing-other-elements-if-needed) — To ensure details exclusivity by closing other elements if needed given a details element element:
  - 1. Assert: element has an open attribute.
  - 2. If element does not have a name attribute, or its name attribute is the empty string, then return.
  - 3. Let groupMembers be a list of elements, containing all elements in element's details name group except for element, in tree order.
  - 4. For each element otherElement of groupMembers:
    - 4a. If the open attribute is set on otherElement:
      - 4a.i. Assert: otherElement is the only element in groupMembers that has the open attribute set.
      - 4a.ii. Remove the open attribute on otherElement.
      - 4a.iii. Break.

- [x] [ensure details exclusivity by closing the given element if needed](https://html.spec.whatwg.org/#ensure-details-exclusivity-by-closing-the-given-element-if-needed) — To ensure details exclusivity by closing the given element if needed given a details element element:
  - 1. If element does not have an open attribute, then return.
  - 2. If element does not have a name attribute, or its name attribute is the empty string, then return.
  - 3. Let groupMembers be a list of elements, containing all elements in element's details name group except for element, in tree order.
  - 4. For each element otherElement of groupMembers:
    - 4a. If the open attribute is set on otherElement:
      - 4a.i. Remove the open attribute on element.
      - 4a.ii. Break.

#### [4.11.2 The summary element](https://html.spec.whatwg.org/#the-summary-element)

Status: reviewed (2026-11-01)
> `HTMLSummaryElement::ActivationBehavior` and `HTMLDetailsElement::IsActiveSummary`; activating interactive content in it doesn't toggle (see `DIFFERENCES.md`).

- [x] [summary for its parent details](https://html.spec.whatwg.org/#summary-for-its-parent-details) — A summary element is a summary for its parent details if the following algorithm returns true:
  - 1. If this summary element has no parent, then return false.
  - 2. Let parent be this summary element's parent.
  - 3. If parent is not a details element, then return false.
  - 4. If parent's first summary element child is not this summary element, then return false.
  - 5. Return true.

- [x] [The activation behavior of summary elements](https://html.spec.whatwg.org/#summary-for-its-parent-details) — The activation behavior of summary elements is to run the following steps:
  - 1. If this summary element is not the summary for its parent details, then return.
  - 2. Let parent be this summary element's parent.
  - 3. If the open attribute is present on parent, then remove it. Otherwise, set parent's open attribute to the empty string.

#### [4.11.3 Commands](https://html.spec.whatwg.org/#commands)

Status: reviewed (2026-11-01)
> Headings only.

##### [4.11.3.1 Facets](https://html.spec.whatwg.org/#facets-2)

Status: reviewed (2026-11-01)
> Access keys aren't handled, so there are no commands (see `KNOWN-ISSUES.md`); `HTMLElement::AccessKeyAction` is the Action.
<!-- definitions: concept-command concept-facet command-facet-label command-facet-accesskey command-facet-hiddenstate command-facet-disabledstate command-facet-action -->

##### [4.11.3.2 Using the a element to define a command](https://html.spec.whatwg.org/#using-the-a-element-to-define-a-command)

Status: reviewed (2026-11-01)
> Access keys aren't handled, so there are no commands (see `KNOWN-ISSUES.md`); `HTMLElement::AccessKeyAction` is the Action.

##### [4.11.3.3 Using the button element to define a command](https://html.spec.whatwg.org/#using-the-button-element-to-define-a-command)

Status: reviewed (2026-11-01)
> Access keys aren't handled, so there are no commands (see `KNOWN-ISSUES.md`); `HTMLElement::AccessKeyAction` is the Action.

##### [4.11.3.4 Using the input element to define a command](https://html.spec.whatwg.org/#using-the-input-element-to-define-a-command)

Status: reviewed (2026-11-01)
> Access keys aren't handled, so there are no commands (see `KNOWN-ISSUES.md`); `HTMLElement::AccessKeyAction` is the Action.

- [x] [The Label of the command is determined as follows](https://html.spec.whatwg.org/#command-facet-label) — The Label of the command is determined as follows:
  - 1. If the type attribute is in one of the Submit Button, Reset Button, Image Button, or Button states, then the Label is the string given by the value attribute, if any, and a UA-dependent, locale-dependent value that the UA uses to label the button itself if the attribute is absent.
  - 2. Otherwise, if the element is a labeled control, then the Label is the descendant text content of the first label element in tree order whose labeled control is the element in question. (In JavaScript terms, this is given by element.labels[0].textContent.)
  - 3. Otherwise, if the value attribute is present, then the Label is the value of that attribute.
  - 4. Otherwise, the Label is the empty string.

##### [4.11.3.5 Using the option element to define a command](https://html.spec.whatwg.org/#using-the-option-element-to-define-a-command)

Status: reviewed (2026-11-01)
> Access keys aren't handled, so there are no commands (see `KNOWN-ISSUES.md`); `HTMLElement::AccessKeyAction` is the Action.

##### [4.11.3.6 Using the accesskey attribute on a legend element to define a command](https://html.spec.whatwg.org/#using-the-accesskey-attribute-on-a-legend-element-to-define-a-command)

Status: reviewed (2026-11-01)
> Access keys aren't handled, so there are no commands (see `KNOWN-ISSUES.md`); `HTMLElement::AccessKeyAction` is the Action.

- [x] [the legend element's accesskey delegatee](https://html.spec.whatwg.org/#the-legend-element's-accesskey-delegatee) — Its parent has a descendant that defines a command that is neither a label element nor a legend element. This element, if it exists, is the legend element's accesskey delegatee.

##### [4.11.3.7 Using the accesskey attribute to define a command on other elements](https://html.spec.whatwg.org/#using-the-accesskey-attribute-to-define-a-command-on-other-elements)

Status: reviewed (2026-11-01)
> Access keys aren't handled, so there are no commands (see `KNOWN-ISSUES.md`); `HTMLElement::AccessKeyAction` is the Action.

- [x] [The Action of the command](https://html.spec.whatwg.org/#command-facet-action) — The Action of the command is to run the following steps:
  - 1. Run the focusing steps for the element.
  - 2. Fire a click event at the element.

#### [4.11.4 The dialog element](https://html.spec.whatwg.org/#the-dialog-element)

Status: reviewed (2026-11-01)
> `HTMLDialogElement`; the open dialogs list and active modal dialog are on `Document`.
> Dialogs are focusable, so a dialog with no focus delegate focuses itself.
<!-- definitions: attr-dialog-open attr-dialog-closedby attr-dialog-closedby-auto-state dialog-pointerdown-target previously-focused-element dialog-toggle-task-tracker dialog-close-watcher request-close-return-value request-close-source-element enable-close-watcher-for-requestclose() -->

- [x] [HTMLDialogElement.show()](https://html.spec.whatwg.org/#dom-dialog-show) — The show() method steps are:
  - 1. If this has an open attribute and is modal of this is false, then return.
  - 2. If this has an open attribute, then throw an "InvalidStateError" DOMException.
  - 3. If the result of firing an event named beforetoggle, using ToggleEvent, with the cancelable attribute initialized to true, the oldState attribute initialized to "closed", and the newState attribute initialized to "open" at this is false, then return.
  - 4. If this has an open attribute, then return.
  - 5. Queue a dialog toggle event task given this, "closed", "open", and null.
  - 6. Add an open attribute to this, whose value is the empty string.
  - 7. Set this's previously focused element to the focused element.
  - 8. Let document be this's node document.
  - 9. Let hideUntil be the result of running topmost popover ancestor given this, null, and false.
  - 10. Run hide popovers until given document, hideUntil, false, and true.
  - 11. Run the dialog focusing steps given this.

- [x] [HTMLDialogElement.showModal()](https://html.spec.whatwg.org/#dom-dialog-showmodal) — The showModal() method steps are to show a modal dialog given this and null.

- [x] [HTMLDialogElement.close(returnValue)](https://html.spec.whatwg.org/#dom-dialog-close) — The close(returnValue) method steps are:
  - 1. If returnValue is not given, then set it to null.
  - 2. Close the dialog this with returnValue and null.

- [x] [HTMLDialogElement.requestClose(returnValue)](https://html.spec.whatwg.org/#dom-dialog-requestclose) — The requestClose(returnValue) method steps are:
  - 1. If returnValue is not given, then set it to null.
  - 2. Request to close the dialog this with returnValue and null.

- [x] [HTMLDialogElement.returnValue](https://html.spec.whatwg.org/#dom-dialog-returnvalue) — The returnValue IDL attribute, on getting, must return the last value to which it was set. On setting, it must be set to the new value. When the element is created, it must be set to the empty string.

- [x] [HTMLDialogElement.closedBy](https://html.spec.whatwg.org/#dom-dialog-closedby) — The closedBy getter steps are to return the keyword corresponding to the computed closed-by state given this.

- [x] [The dialog HTML element insertion steps, given insertedNode,](https://html.spec.whatwg.org/#the-dialog-element) — The dialog HTML element insertion steps, given insertedNode, are:
  - 1. If insertedNode's node document is not fully active, then return.
  - 2. If insertedNode has an open attribute and is connected, then run the dialog setup steps given insertedNode.

- [x] [The dialog HTML element removing steps, given removedNode, isSubtreeRoot, and o…](https://html.spec.whatwg.org/#the-dialog-element) — The dialog HTML element removing steps, given removedNode, isSubtreeRoot, and oldAncestor are:
  - 1. If removedNode has an open attribute, then run the dialog cleanup steps given removedNode.
  - 2. If removedNode's node document's top layer contains removedNode, then remove an element from the top layer immediately given removedNode.
  - 3. Set is modal of removedNode to false.

- [x] [The following attribute change steps, given element, localName, oldValue, value…](https://html.spec.whatwg.org/#is-modal) — The following attribute change steps, given element, localName, oldValue, value, and namespace are used for dialog elements:
  - 1. If namespace is not null, then return.
  - 2. If localName is not open, then return.
  - 3. If value is null and oldValue is not null, then run the dialog cleanup steps given element.
  - 4. If element's node document is not fully active, then return.
  - 5. If element is not connected, then return.
  - 6. If value is not null and oldValue is null, then run the dialog setup steps given element.

- [x] [show a modal dialog](https://html.spec.whatwg.org/#show-a-modal-dialog) — To show a modal dialog given a dialog element subject and an Element or null source:
  - 1. If subject has an open attribute and is modal of subject is true, then return.
  - 2. If subject has an open attribute, then throw an "InvalidStateError" DOMException.
  - 3. If subject's node document is not fully active, then throw an "InvalidStateError" DOMException.
  - 4. If subject is not connected, then throw an "InvalidStateError" DOMException.
  - 5. If subject is in the popover showing state, then throw an "InvalidStateError" DOMException.
  - 6. If the result of firing an event named beforetoggle, using ToggleEvent, with the cancelable attribute initialized to true, the oldState attribute initialized to "closed", the newState attribute initialized to "open", and the source attribute initialized to source at subject is false, then return.
  - 7. If subject has an open attribute, then return.
  - 8. If subject is not connected, then return.
  - 9. If subject is in the popover showing state, then return.
  - 10. Queue a dialog toggle event task given subject, "closed", "open", and source.
  - 11. Add an open attribute to subject, whose value is the empty string.
  - 12. Assert: subject's close watcher is not null.
  - 13. Set is modal of subject to true.
  - 14. Set subject's node document to be blocked by the modal dialog subject.
  - 15. If subject's node document's top layer does not already contain subject, then add an element to the top layer given subject.
  - 16. Set subject's previously focused element to the focused element.
  - 17. Let document be subject's node document.
  - 18. Let hideUntil be the result of running topmost popover ancestor given subject, null, and false.
  - 19. Run hide popovers until given document, hideUntil, false, and true.
  - 20. Run the dialog focusing steps given subject.

- [x] [set the dialog close watcher](https://html.spec.whatwg.org/#set-the-dialog-close-watcher) — To set the dialog close watcher, given a dialog element dialog:
  - 1. Assert: dialog's close watcher is null.
  - 2. Assert: dialog has an open attribute and dialog's node document is fully active.
  - 3. Set dialog's close watcher to the result of establishing a close watcher given dialog's relevant global object, with: cancelAction given canPreventClose being to return the result of firing an event named cancel at dialog, with the cancelable attribute initialized to canPreventClose.; closeAction being to close the dialog given dialog, dialog's request close return value, and dialog's request clo…

- [x] [The is valid command steps for dialog elements, given a command attribute comma…](https://html.spec.whatwg.org/#is-valid-command-steps) — The is valid command steps for dialog elements, given a command attribute command, are:
  - 1. If command is in the Close state, the Request Close state, or the Show Modal state, then return true.
  - 2. Return false.

- [x] [The command steps for dialog elements, given an element element, an element sou…](https://html.spec.whatwg.org/#command-steps) — The command steps for dialog elements, given an element element, an element source, and a command attribute command, are:
  - 1. If element is in the popover showing state, then return.
  - 2. If command is in the Close state and element has an open attribute, then close the dialog element with source's optional value and source.
  - 3. If command is in the Request Close state and element has an open attribute, then request to close the dialog element with source's optional value and source.
  - 4. If command is the Show Modal state and element does not have an open attribute, then show a modal dialog given element and source.

- [x] [closed](https://html.spec.whatwg.org/#close-the-dialog) — When a dialog element subject is to be closed, with null or a string result and an Element or null source, run these steps:
  - 1. If subject does not have an open attribute, then return.
  - 2. Fire an event named beforetoggle, using ToggleEvent, with the oldState attribute initialized to "open", the newState attribute initialized to "closed", and the source attribute initialized to source at subject.
  - 3. If subject does not have an open attribute, then return.
  - 4. Queue a dialog toggle event task given subject, "open", "closed", and source.
  - 5. Remove subject's open attribute.
  - 6. If is modal of subject is true, then request an element to be removed from the top layer given subject.
  - 7. Let wasModal be the value of subject's is modal flag.
  - 8. Set is modal of subject to false.
  - 9. If result is not null, then set subject's returnValue attribute to result.
  - 10. Set subject's request close return value to null.
  - 11. Set subject's request close source element to null.
  - 12. If subject's previously focused element is not null:
    - 12a. Let element be subject's previously focused element.
    - 12b. Set subject's previously focused element to null.
    - 12c. If subject's node document's focused area of the document's DOM anchor is a shadow-including inclusive descendant of subject, or wasModal is true, then run the focusing steps for element; the viewport should not be scrolled by doing this step.
  - 13. Queue an element task on the user interaction task source given the subject element to fire an event named close at subject.

- [x] [request to close](https://html.spec.whatwg.org/#dialog-request-close) — To request to close dialog element subject, given null or a string returnValue and null or an Element source:
  - 1. If subject does not have an open attribute, then return.
  - 2. If subject is not connected or subject's node document is not fully active, then return.
  - 3. Assert: subject's close watcher is not null.
  - 4. Set subject's enable close watcher for request close to true.
  - 5. Set subject's request close return value to returnValue.
  - 6. Set subject's request close source element to source.
  - 7. Request to close subject's close watcher with false.
  - 8. Set subject's enable close watcher for request close to false.

- [x] [queue a dialog toggle event task](https://html.spec.whatwg.org/#queue-a-dialog-toggle-event-task) — To queue a dialog toggle event task given a dialog element element, a string oldState, a string newState, and an Element or null source:
  - 1. If element's dialog toggle task tracker is not null:
    - 1a. Set oldState to element's dialog toggle task tracker's old state.
    - 1b. Remove element's dialog toggle task tracker's task from its task queue.
    - 1c. Set element's dialog toggle task tracker to null.
  - 2. Queue an element task given the DOM manipulation task source and element to run the following steps:
    - 2a. Fire an event named toggle at element, using ToggleEvent, with the oldState attribute initialized to oldState, the newState attribute initialized to newState, and the source attribute initialized to source.
    - 2b. Set element's dialog toggle task tracker to null.
  - 3. Set element's dialog toggle task tracker to a struct with task set to the just-queued task and old state set to oldState.

- [x] [computed closed-by state](https://html.spec.whatwg.org/#computed-closed-by-state) — To retrieve a dialog's computed closed-by state, given a dialog dialog:
  - 1. If the state of dialog's closedby attribute is Auto:
    - 1a. If dialog's is modal is true, then return Close Request.
    - 1b. Return None.
  - 2. Return the state of dialog's closedby attribute.

- [x] [dialog focusing steps](https://html.spec.whatwg.org/#dialog-focusing-steps) — The dialog focusing steps, given a dialog element subject, are as follows:
  - 1. If the allow focus steps given subject's node document return false, then return.
  - 2. Let control be null.
  - 3. If subject has the autofocus attribute, then set control to subject.
  - 4. If control is null, then set control to the focus delegate of subject.
  - 5. If control is null, then set control to subject.
  - 6. Run the focusing steps for control.
  - 7. Let topDocument be control's node navigable's top-level traversable's active document.
  - 8. If control's node document's origin is not the same as the origin of topDocument, then return.
  - 9. Empty topDocument's autofocus candidates.
  - 10. Set topDocument's autofocus processed flag to true.

- [x] [dialog setup steps](https://html.spec.whatwg.org/#dialog-setup-steps) — The dialog setup steps, given a dialog element subject, are as follows:
  - 1. Assert: subject has an open attribute.
  - 2. Assert: subject is connected.
  - 3. Assert: subject's node document's open dialogs list does not contain subject.
  - 4. Add subject to subject's node document's open dialogs list.
  - 5. Set the dialog close watcher with subject.

- [x] [dialog cleanup steps](https://html.spec.whatwg.org/#dialog-cleanup-steps) — The dialog cleanup steps, given a dialog element subject, are as follows:
  - 1. Remove subject from subject's node document's open dialogs list.
  - 2. If subject's close watcher is not null:
    - 2a. Destroy subject's close watcher.
    - 2b. Set subject's close watcher to null.

#### [4.11.5 Dialog light dismiss](https://html.spec.whatwg.org/#dialog-light-dismiss)

Status: reviewed (2026-11-01)
> `Document::HandleLightDismiss`, called by `PointerCaptureController`.

- [x] [light dismiss open dialogs](https://html.spec.whatwg.org/#light-dismiss-open-dialogs) — To light dismiss open dialogs, given a PointerEvent event:
  - 1. Assert: event's isTrusted attribute is true.
  - 2. Let document be event's target's node document.
  - 3. If document's open dialogs list is empty, then return.
  - 4. Let ancestor be the result of running nearest clicked dialog given event.
  - 5. If event's type is "pointerdown", then set document's dialog pointerdown target to ancestor.
  - 6. If event's type is "pointerup":
    - 6a. Let sameTarget be true if ancestor is document's dialog pointerdown target.
    - 6b. Set document's dialog pointerdown target to null.
    - 6c. If sameTarget is false, then return.
    - 6d. Let topmostDialog be the last element of document's open dialogs list.
    - 6e. If ancestor is topmostDialog, then return.
    - 6f. If topmostDialog's computed closed-by state is not Any, then return.
    - 6g. Assert: topmostDialog's close watcher is not null.
    - 6h. Request to close topmostDialog's close watcher with false.

- [x] [run light dismiss activities](https://html.spec.whatwg.org/#run-light-dismiss-activities) — To run light dismiss activities, given a PointerEvent event:
  - 1. Run light dismiss open popovers with event.
  - 2. Run light dismiss open dialogs with event.

- [x] [nearest clicked dialog](https://html.spec.whatwg.org/#nearest-clicked-dialog) — To find the nearest clicked dialog, given a PointerEvent event:
  - 1. Let target be event's target.
  - 2. If target is a dialog element, target has an open attribute, target's is modal is true, and event's clientX and clientY are outside the bounds of target, then return null.
  - 3. Let currentNode be target.
  - 4. While currentNode is not null:
    - 4a. If currentNode is a dialog element and currentNode has an open attribute, then return currentNode.
    - 4b. Set currentNode to currentNode's parent in the flat tree.
  - 5. Return null.
