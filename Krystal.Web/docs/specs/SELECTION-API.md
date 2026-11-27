# Selection API review checklist

Generated from <https://w3c.github.io/selection-api/> on 2026-11-27 by `Krystal.Web/tools/spec_checklist.py`: 31 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Web/tools/spec_status.py selection-api` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[selection-api:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Web/tools/spec_status.py selection-api --tick <section anchor>` ticks a section's algorithms and sets its status.

## [1. Background](https://w3c.github.io/selection-api/#background)

Status: unreviewed

## [2. Definition](https://w3c.github.io/selection-api/#definition)

Status: unreviewed
<!-- definitions: dfn-selection dfn-empty dfn-direction dfn-forwards dfn-backwards dfn-directionless dfn-anchor dfn-focus dfn-has-scheduled-selectionchange-event -->

## [3. Selection interface](https://w3c.github.io/selection-api/#selection-interface)

Status: unreviewed
<!-- definitions: dfn-resolved-text-direction -->

- [ ] [Selection.anchorNode](https://w3c.github.io/selection-api/#dom-selection-anchornode) — The attribute must return the anchor node of this, or null if the anchor is null or anchor is not in the document tree.

- [ ] [Selection.anchorOffset](https://w3c.github.io/selection-api/#dom-selection-anchoroffset) — The attribute must return the anchor offset of this, or 0 if the anchor is null or anchor is not in the document tree.

- [ ] [Selection.focusNode](https://w3c.github.io/selection-api/#dom-selection-focusnode) — The attribute must return the focus node of this, or null if the focus is null or focus is not in the document tree.

- [ ] [Selection.focusOffset](https://w3c.github.io/selection-api/#dom-selection-focusoffset) — The attribute must return the focus offset of this, or 0 if the focus is null or focus is not in the document tree.

- [ ] [Selection.isCollapsed](https://w3c.github.io/selection-api/#dom-selection-iscollapsed) — The attribute must return true if and only if the anchor and focus are the same (including if both are null). Otherwise it must return false.

- [ ] [Selection.rangeCount](https://w3c.github.io/selection-api/#dom-selection-rangecount) — The attribute must return 0 if this is empty or either focus or anchor is not in the document tree, and must return 1 otherwise.

- [ ] [Selection.type](https://w3c.github.io/selection-api/#dom-selection-type) — The attribute must return "None" if this is empty or either focus or anchor is not in the document tree, "Caret" if this's range is collapsed, and "Range" otherwise.

- [ ] [Selection.direction](https://w3c.github.io/selection-api/#dom-selection-direction) — The attribute must return "none" if this is empty or this selection is directionless. "forward" if this selection's direction is forwards and "backward" if this selection's direction is backwards.

- [ ] [Selection.getRangeAt()](https://w3c.github.io/selection-api/#dom-selection-getrangeat) — The method must throw an IndexSizeError exception if index is not 0, or if this is empty or either focus or anchor is not in the document tree. Otherwise, it must return a reference to (not a copy of) this's range.

- [ ] [Selection.addRange()](https://w3c.github.io/selection-api/#dom-selection-addrange) — The method must follow these steps:
  - 1. If the root of the range's boundary points are not the document associated with this, abort these steps.
  - 2. If rangeCount is not 0, abort these steps.
  - 3. Set this's range to range by a strong reference (not by making a copy).

- [ ] [Selection.removeRange()](https://w3c.github.io/selection-api/#dom-selection-removerange) — The method must make this empty by disassociating its range if this's range is range. Otherwise, it must throw a NotFoundError.

- [ ] [Selection.removeAllRanges()](https://w3c.github.io/selection-api/#dom-selection-removeallranges) — The method must make this empty by disassociating its range if this has an associated range.

- [ ] [Selection.empty()](https://w3c.github.io/selection-api/#dom-selection-empty) — The method must be an alias, and behave identically, to removeAllRanges().

- [ ] [Selection.getComposedRanges()](https://w3c.github.io/selection-api/#dom-selection-getcomposedranges) — getComposedRanges() method
  - 1. If this is empty, return an empty array.
  - 2. Otherwise, let startNode be start node of the range associated with this, and let startOffset be start offset of the range.
  - 3. While startNode is a node, startNode's root is a shadow root, and startNode's root is not a shadow-including inclusive ancestor of any of options["shadowRoots"], repeat these steps:
    - 3a. Set startOffset to index of startNode's root's host.
    - 3b. Set startNode to startNode's root's host's parent.
  - 4. Let endNode be end node of the range associated with this, and let endOffset be end offset of the range.
  - 5. While endNode is a node, endNode's root is a shadow root, and endNode's root is not a shadow-including inclusive ancestor of any of options["shadowRoots"], repeat these steps:
    - 5a. Set endOffset to index of endNode's root's host plus 1.
    - 5b. Set endNode to endNode's root's host's parent.
  - 6. Return an array consisting of new StaticRange whose start node is startNode, start offset is startOffset, end node is endNode, and end offset is endOffset.

- [ ] [Selection.collapse()](https://w3c.github.io/selection-api/#dom-selection-collapse) — The method must follow these steps:
  - 1. If node is null, this method must behave identically as removeAllRanges() and abort these steps.
  - 2. If node is a DocumentType, throw an InvalidNodeTypeError exception and abort these steps.
  - 3. The method must throw an IndexSizeError exception if offset is longer than node's length and abort these steps.
  - 4. If document associated with this is not a shadow-including inclusive ancestor of node, abort these steps.
  - 5. Otherwise, let newRange be a new range.
  - 6. Set the start the start and the end of newRange to (node, offset).
  - 7. Set this's range to newRange.

- [ ] [Selection.setPosition()](https://w3c.github.io/selection-api/#dom-selection-setposition) — The method must be an alias, and behave identically, to collapse().

- [ ] [Selection.collapseToStart()](https://w3c.github.io/selection-api/#dom-selection-collapsetostart) — The method must throw InvalidStateError exception if the this is empty. Otherwise, it must create a new range, set the start both its start and end to the start of this's range, and then set this's range to the newly-created range.

- [ ] [Selection.collapseToEnd()](https://w3c.github.io/selection-api/#dom-selection-collapsetoend) — The method must throw InvalidStateError exception if the this is empty. Otherwise, it must create a new range, set the start both its start and end to the end of this's range, and then set this's range to the newly-created range.

- [ ] [Selection.extend()](https://w3c.github.io/selection-api/#dom-selection-extend) — The method must follow these steps:
  - 1. If the document associated with this is not a shadow-including inclusive ancestor of node, abort these steps.
  - 2. If this is empty, throw an InvalidStateError exception and abort these steps.
  - 3. Let oldAnchor and oldFocus be the this's anchor and focus, and let newFocus be the boundary point (node, offset).
  - 4. Let newRange be a new range.
  - 5. If node's root is not the same as the this's range's root, set the start newRange's start and end to newFocus.
  - 6. Otherwise, if oldAnchor is before or equal to newFocus, set the start newRange's start to oldAnchor, then set its end to newFocus.
  - 7. Otherwise, set the start newRange's start to newFocus, then set its end to oldAnchor.
  - 8. Set this's range to newRange.
  - 9. If newFocus is before oldAnchor, set this's direction to backwards. Otherwise, set it to forwards.

- [ ] [Selection.setBaseAndExtent()](https://w3c.github.io/selection-api/#dom-selection-setbaseandextent) — The method must follow these steps:
  - 1. If anchorOffset is longer than anchorNode's length or if focusOffset is longer than focusNode's length, throw an IndexSizeError exception and abort these steps.
  - 2. If document associated with this is not a shadow-including inclusive ancestor of anchorNode or focusNode, abort these steps.
  - 3. Let anchor be the boundary point (anchorNode, anchorOffset) and let focus be the boundary point (focusNode, focusOffset).
  - 4. Let newRange be a new range.
  - 5. If anchor is before focus, set the start the newRange's start to anchor and its end to focus. Otherwise, set the start them to focus and anchor respectively.
  - 6. Set this's range to newRange.
  - 7. If focus is before anchor, set this's direction to backwards. Otherwise, set it to forwards

- [ ] [Selection.selectAllChildren()](https://w3c.github.io/selection-api/#dom-selection-selectallchildren) — The method must follow these steps:
  - 1. If node is a DocumentType, throw an InvalidNodeTypeError exception and abort these steps.
  - 2. If node's root is not the document associated with this, abort these steps.
  - 3. Let newRange be a new range and childCount be the number of children of node.
  - 4. Set newRange's start to (node, 0).
  - 5. Set newRange's end to (node, childCount).
  - 6. Set this's range to newRange.
  - 7. Set this's direction to forwards.

- [ ] [Selection.modify()](https://w3c.github.io/selection-api/#dom-selection-modify) — The method must follow these steps:
  - 1. If alter is not ASCII case-insensitive match with "extend" or "move", abort these steps.
  - 2. If direction is not ASCII case-insensitive match with "forward", "backward", "left", or "right", abort these steps.
  - 3. If granularity is not ASCII case-insensitive match with "character", "word", "sentence", "line", "paragraph", "lineboundary", "sentenceboundary", "paragraphboundary", "documentboundary", abort these steps.
  - 4. If this selection is empty, abort these steps.
  - 5. Let effectiveDirection be backwards.
  - 6. If direction is ASCII case-insensitive match with "forward", set effectiveDirection to forwards.
  - 7. If direction is ASCII case-insensitive match with "right" and the resolved text direction at this selection's focus is ltr, set effectiveDirection to forwards.
  - 8. If direction is ASCII case-insensitive match with "left" and the resolved text direction at this selection's focus is rtl, set effectiveDirection to forwards.
  - 9. Set this selection's direction to effectiveDirection.
  - 10. If alter is ASCII case-insensitive match with "extend", set this selection's focus to the location as if the user had requested to extend selection by granularity.
  - 11. Otherwise, set this selection's focus and anchor to the location as if the user had requested to move selection by granularity.

- [ ] [Selection.deleteFromDocument()](https://w3c.github.io/selection-api/#dom-selection-deletefromdocument) — The method must invoke deleteContents() on this's range if this is not empty and both focus and anchor are in the document tree. Otherwise the method must do nothing.

- [ ] [Selection.containsNode()](https://w3c.github.io/selection-api/#dom-selection-containsnode) — The method must return false if this is empty or if node's root is not the document associated with this.

- [ ] [Selection.stringifier](https://w3c.github.io/selection-api/#dom-selection-stringifier) — The stringification must return the string, which is the concatenation of the rendered text if there is a range associated with this.

## [4. Extensions to Other Interfaces](https://w3c.github.io/selection-api/#extensions-to-other-interfaces)

Status: unreviewed

### [4.1 Extensions to Document interface](https://w3c.github.io/selection-api/#extensions-to-document-interface)

Status: unreviewed
<!-- definitions: dom-document -->

- [ ] [Document.getSelection()](https://w3c.github.io/selection-api/#dom-document-getselection) — The method must return the selection associated with this if this has an associated browsing context, and it must return null otherwise.

### [4.2 Extensions to Window interface](https://w3c.github.io/selection-api/#extensions-to-window-interface)

Status: unreviewed
<!-- definitions: dom-window -->

- [ ] [Window.getSelection()](https://w3c.github.io/selection-api/#dom-window-getselection) — The method must invoke and return the result of getSelection() on this's Window.document attribute.

### [4.3 Extensions to GlobalEventHandlers interface](https://w3c.github.io/selection-api/#extensions-to-globaleventhandlers-interface)

Status: unreviewed
<!-- definitions: dom-globaleventhandlers -->

- [ ] [GlobalEventHandlers.onselectstart](https://w3c.github.io/selection-api/#dom-globaleventhandlers-onselectstart) — The attribute must be an event handler IDL attribute for the selectstart event supported by all HTML elements, Document objects, and Window objects.

- [ ] [GlobalEventHandlers.onselectionchange](https://w3c.github.io/selection-api/#dom-globaleventhandlers-onselectionchange) — The attribute must be an event handler IDL attribute for the selectionchange event supported by all HTML elements, Document objects, and Window objects.

## [5. Responding to DOM Mutations](https://w3c.github.io/selection-api/#responding-to-dom-mutations)

Status: unreviewed

## [6. User Interactions](https://w3c.github.io/selection-api/#user-interactions)

Status: unreviewed

### [6.1 selectstart event](https://w3c.github.io/selection-api/#selectstart-event)

Status: unreviewed

### [6.2 selectionchange event](https://w3c.github.io/selection-api/#selectionchange-event)

Status: unreviewed

#### [6.2.1 Scheduling selectionchange event](https://w3c.github.io/selection-api/#scheduling-selectionchange-event)

Status: unreviewed

- [ ] [schedule a selectionchange event](https://w3c.github.io/selection-api/#dfn-schedule-a-selectionchange-event) — To schedule a selectionchange event on a node target, run these steps:
  - 1. If target's has scheduled selectionchange event is true, abort these steps.
  - 2. Set target's has scheduled selectionchange event to true.
  - 3. Queue a task on the user interaction task source to fire a selectionchange event on target.

#### [6.2.2 Firing selectionchange event](https://w3c.github.io/selection-api/#firing-selectionchange-event)

Status: unreviewed

- [ ] [fire a selectionchange event](https://w3c.github.io/selection-api/#dfn-fire-a-selectionchange-event) — To fire a selectionchange event on a node target, run these steps:
  - 1. Set target's has scheduled selectionchange event to false.
  - 2. If target is an element, fire an event named selectionchange, which bubbles and not cancelable, at target.
  - 3. Otherwise, if target is a document, fire an event named selectionchange, which does not bubble and not cancelable, at target.

## [7. Conformance](https://w3c.github.io/selection-api/#conformance)

Status: unreviewed

## [8. Security and Privacy considerations](https://w3c.github.io/selection-api/#security-and-privacy-considerations)

Status: unreviewed

## [A. Acknowledgements](https://w3c.github.io/selection-api/#acknowledgements)

Status: unreviewed

## [B. References](https://w3c.github.io/selection-api/#references)

Status: unreviewed

### [B.1 Normative references](https://w3c.github.io/selection-api/#normative-references)

Status: unreviewed
