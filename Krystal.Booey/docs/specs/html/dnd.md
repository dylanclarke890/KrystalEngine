# HTML review checklist: 6.11 Drag and drop

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 24 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html dnd` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [6.11 Drag and drop](https://html.spec.whatwg.org/#dnd)

Status: reviewed (2026-11-01)
> The sections below; drags stay in the page (see `KNOWN-ISSUES.md`).

#### [6.11.1 Introduction](https://html.spec.whatwg.org/#event-drag)

Status: n/a (introduction)

#### [6.11.2 The drag data store](https://html.spec.whatwg.org/#the-drag-data-store)

Status: reviewed (2026-11-01)
> The data transfer's pasteboard (a `StaticPasteboard` for `dragstart` and `new DataTransfer()`) and its `StoreMode`.
<!-- definitions: drag-data-store drag-data-store-item-list the-drag-data-item-kind the-drag-data-item-type-string drag-data-store-default-feedback drag-data-store-bitmap drag-data-store-hot-spot-coordinate drag-data-store-mode concept-dnd-rw concept-dnd-ro concept-dnd-p drag-data-store-allowed-effects-state -->

- [x] [created](https://html.spec.whatwg.org/#create-a-drag-data-store) — When a drag data store is created, it must be initialized such that its drag data store item list is empty, it has no drag data store default feedback, it has no drag data store bitmap and drag data store hot spot coordinate, its drag data store mode is protected mode, and its drag data store allowed effects state is the string "uninitialized".

#### [6.11.3 The DataTransfer interface](https://html.spec.whatwg.org/#the-datatransfer-interface)

Status: reviewed (2026-11-01)
> `DataTransfer`, as upstream's: it normalises and sanitises types and data, and only a drag's effects can be set (see `DIFFERENCES.md`).
<!-- definitions: dom-datatransfer-dropeffect-none dom-datatransfer-dropeffect-copy dom-datatransfer-dropeffect-link dom-datatransfer-dropeffect-move dom-datatransfer-effectallowed-none dom-datatransfer-effectallowed-copy dom-datatransfer-effectallowed-copylink dom-datatransfer-effectallowed-copymove dom-datatransfer-effectallowed-link dom-datatransfer-effectallowed-linkmove dom-datatransfer-effectallowed-move dom-datatransfer-effectallowed-all dom-datatransfer-effectallowed-uninitialized -->

- [x] [types array](https://html.spec.whatwg.org/#concept-datatransfer-types) — A DataTransfer object has an associated types array, which is a FrozenArray<DOMString>, initially empty. When the contents of the DataTransfer object's drag data store item list change, or when the DataTransfer object becomes no longer associated with a drag data store, run the following steps:
  - 1. Let L be an empty sequence.
  - 2. If the DataTransfer object is still associated with a drag data store:
    - 2a. For each item in the DataTransfer object's drag data store item list whose kind is text, add an entry to L consisting of the item's type string.
    - 2b. If there are any items in the DataTransfer object's drag data store item list whose kind is File, then add an entry to L consisting of the string "Files". (This value can be distinguished from the other values because it is not lowercase.)
  - 3. Set the DataTransfer object's types array to the result of creating a frozen array from L.

- [x] [DataTransfer.DataTransfer()](https://html.spec.whatwg.org/#dom-datatransfer) — The DataTransfer() constructor, when invoked, must return a newly created DataTransfer object initialized as follows:
  - 1. Set the drag data store's item list to be an empty list.
  - 2. Set the drag data store's mode to read/write mode.
  - 3. Set the dropEffect and effectAllowed to "none".

- [x] [DataTransfer.dropEffect](https://html.spec.whatwg.org/#dom-datatransfer-dropeffect) — The dropEffect attribute controls the drag-and-drop feedback that the user is given during a drag-and-drop operation. When the DataTransfer object is created, the dropEffect attribute is set to a string value. On getting, it must return its current value. On setting, if the new value is one of "none", "copy", "link", or "move", then the attribute's current value must be set to the new value. Other values must be ignored.

- [x] [DataTransfer.effectAllowed](https://html.spec.whatwg.org/#dom-datatransfer-effectallowed) — The effectAllowed attribute is used in the drag-and-drop processing model to initialize the dropEffect attribute during the dragenter and dragover events. When the DataTransfer object is created, the effectAllowed attribute is set to a string value. On getting, it must return its current value. On setting, if the drag data store's mode is the read/write mode and the new value is one of "none", "copy", "copyLink", "copyMove", "link", "linkMove", "move", "all", or "uninitialized", then the attribute's current value must be set to the new value. Otherwise, it must be left unchanged.

- [x] [DataTransfer.items](https://html.spec.whatwg.org/#dom-datatransfer-items) — The items attribute must return a DataTransferItemList object associated with the DataTransfer object.

- [x] [DataTransfer.setDragImage(image, x, y)](https://html.spec.whatwg.org/#dom-datatransfer-setdragimage) — The setDragImage(image, x, y) method must run the following steps:
  - 1. If the DataTransfer object is no longer associated with a drag data store, return. Nothing happens.
  - 2. If the drag data store's mode is not the read/write mode, return. Nothing happens.
  - 3. If image is an img element, then set the drag data store bitmap to the element's image (at its natural size); otherwise, set the drag data store bitmap to an image generated from the given element (the exact mechanism for doing so is not currently specified).
  - 4. Set the drag data store hot spot coordinate to the given x, y coordinate.

- [x] [DataTransfer.types](https://html.spec.whatwg.org/#dom-datatransfer-types) — The types attribute must return this DataTransfer object's types array.

- [x] [DataTransfer.getData(format)](https://html.spec.whatwg.org/#dom-datatransfer-getdata) — The getData(format) method must run the following steps:
  - 1. If the DataTransfer object is no longer associated with a drag data store, then return the empty string.
  - 2. If the drag data store's mode is the protected mode, then return the empty string.
  - 3. Let format be the first argument, converted to ASCII lowercase.
  - 4. Let convert-to-URL be false.
  - 5. If format equals "text", change it to "text/plain".
  - 6. If format equals "url", change it to "text/uri-list" and set convert-to-URL to true.
  - 7. If there is no item in the drag data store item list whose kind is text and whose type string is equal to format, return the empty string.
  - 8. Let result be the data of the item in the drag data store item list whose kind is Plain Unicode string and whose type string is equal to format.
  - 9. If convert-to-URL is true, then parse result as appropriate for text/uri-list data, and then set result to the first URL from the list, if any, or the empty string otherwise. [RFC2483]
  - 10. Return result.

- [x] [DataTransfer.setData(format, data)](https://html.spec.whatwg.org/#dom-datatransfer-setdata) — The setData(format, data) method must run the following steps:
  - 1. If the DataTransfer object is no longer associated with a drag data store, return. Nothing happens.
  - 2. If the drag data store's mode is not the read/write mode, return. Nothing happens.
  - 3. Let format be the first argument, converted to ASCII lowercase.
  - 4. If format equals "text", change it to "text/plain". If format equals "url", change it to "text/uri-list".
  - 5. Remove the item in the drag data store item list whose kind is text and whose type string is equal to format, if there is one.
  - 6. Add an item to the drag data store item list whose kind is text, whose type string is equal to format, and whose data is the string given by the method's second argument.

- [x] [DataTransfer.clearData(format)](https://html.spec.whatwg.org/#dom-datatransfer-cleardata) — The clearData(format) method must run the following steps:
  - 1. If the DataTransfer object is no longer associated with a drag data store, return. Nothing happens.
  - 2. If the drag data store's mode is not the read/write mode, return. Nothing happens.
  - 3. If the method was called with no arguments, remove each item in the drag data store item list whose kind is Plain Unicode string, and return.
  - 4. Set format to format, converted to ASCII lowercase.
  - 5. If format equals "text", change it to "text/plain". If format equals "url", change it to "text/uri-list".
  - 6. Remove the item in the drag data store item list whose kind is text and whose type string is equal to format, if there is one.

- [x] [DataTransfer.files](https://html.spec.whatwg.org/#dom-datatransfer-files) — The files attribute must return a live FileList sequence consisting of File objects representing the files found by the following steps. Furthermore, for a given FileList object and a given underlying file, the same File object must be used each time.
  - 1. Start with an empty list L.
  - 2. If the DataTransfer object is no longer associated with a drag data store, the FileList is empty. Return the empty list L.
  - 3. If the drag data store's mode is the protected mode, return the empty list L.
  - 4. For each item in the drag data store item list whose kind is File, add the item's data (the file, in particular its name and contents, as well as its type) to the list L.
  - 5. The files found by these steps are those in the list L.

##### [6.11.3.1 The DataTransferItemList interface](https://html.spec.whatwg.org/#the-datatransferitemlist-interface)

Status: reviewed (2026-11-01)
> `DataTransferItemList`.
> Fixed: `clear()` cleared data that couldn't be written, such as a drop's.

- [x] [DataTransferItemList.length](https://html.spec.whatwg.org/#dom-datatransferitemlist-length) — The length attribute must return zero if the object is in the disabled mode; otherwise it must return the number of items in the drag data store item list.

- [x] [DataTransferItemList.add()](https://html.spec.whatwg.org/#dom-datatransferitemlist-add) — The add() method must run the following steps:
  - 1. If the DataTransferItemList object is not in the read/write mode, return null.
  - 2. Jump to the appropriate set of steps from the following list: If the first argument to the method is a string If there is already an item in the drag data store item list whose kind is text and whose type string is equal to the value of the method's second argument, converted to ASCII lowercase, then throw a "NotSupportedError" DOMException. Otherwise, add an item to the drag data store item list…
  - 3. Determine the value of the indexed property corresponding to the newly added item, and return that value (a newly created DataTransferItem object).

- [x] [DataTransferItemList.remove(index)](https://html.spec.whatwg.org/#dom-datatransferitemlist-remove) — The remove(index) method must run these steps:
  - 1. If the DataTransferItemList object is not in the read/write mode, throw an "InvalidStateError" DOMException.
  - 2. If the drag data store does not contain an indexth item, then return.
  - 3. Remove the indexth item from the drag data store.

- [x] [DataTransferItemList.clear()](https://html.spec.whatwg.org/#dom-datatransferitemlist-clear) — The clear() method, if the DataTransferItemList object is in the read/write mode, must remove all the items from the drag data store. Otherwise, it must do nothing.

##### [6.11.3.2 The DataTransferItem interface](https://html.spec.whatwg.org/#the-datatransferitem-interface)

Status: reviewed (2026-11-01)
> `DataTransferItem`; `getAsFile()` returns the same file each time, and there's no `getAsEntry` (see `DIFFERENCES.md` and `KNOWN-ISSUES.md`).
> Fixed: a removed item's `kind` wasn't empty.

- [x] [DataTransferItem.kind](https://html.spec.whatwg.org/#dom-datatransferitem-kind) — The kind attribute must return the empty string if the DataTransferItem object is in the disabled mode; otherwise it must return the string given in the cell from the second column of the following table from the row whose cell in the first column contains the drag data item kind of the item represented by the DataTransferItem object:

- [x] [DataTransferItem.type](https://html.spec.whatwg.org/#dom-datatransferitem-type) — The type attribute must return the empty string if the DataTransferItem object is in the disabled mode; otherwise it must return the drag data item type string of the item represented by the DataTransferItem object.

- [x] [DataTransferItem.getAsString(callback)](https://html.spec.whatwg.org/#dom-datatransferitem-getasstring) — The getAsString(callback) method must run the following steps:
  - 1. If the callback is null, return.
  - 2. If the DataTransferItem object is not in the read/write mode or the read-only mode, return. The callback is never invoked.
  - 3. If the drag data item kind is not text, then return. The callback is never invoked.
  - 4. Otherwise, queue a task to invoke callback, passing the actual data of the item represented by the DataTransferItem object as the argument.

- [x] [DataTransferItem.getAsFile()](https://html.spec.whatwg.org/#dom-datatransferitem-getasfile) — The getAsFile() method must run the following steps:
  - 1. If the DataTransferItem object is not in the read/write mode or the read-only mode, then return null.
  - 2. If the drag data item kind is not File, then return null.
  - 3. Return a new File object representing the actual data of the item represented by the DataTransferItem object.

#### [6.11.4 The DragEvent interface](https://html.spec.whatwg.org/#the-dragevent-interface)

Status: reviewed (2026-11-01)
> `DragEvent`; drag events are fired by `EventHandler::DispatchDragEvent`.
<!-- definitions: concept-platform-dropeffect-override -->

- [x] [DragEvent.dataTransfer](https://html.spec.whatwg.org/#dom-dragevent-datatransfer) — The dataTransfer attribute of the DragEvent interface must return the value it was initialized to. It represents the context information for the event.

- [x] [fire a DND event](https://html.spec.whatwg.org/#fire-a-dnd-event) — When a user agent is required to fire a DND event named e at an element, using a particular drag data store, and optionally with a specific related target, the user agent must run the following steps:
  - 1. Let dataDragStoreWasChanged be false.
  - 2. If no specific related target was provided, set related target to null.
  - 3. Let window be the relevant global object of the Document object of the specified target element.
  - 4. If e is dragstart, then set the drag data store mode to the read/write mode and set dataDragStoreWasChanged to true. If e is drop, set the drag data store mode to the read-only mode.
  - 5. Let dataTransfer be a newly created DataTransfer object associated with the given drag data store.
  - 6. Set the effectAllowed attribute to the drag data store's drag data store allowed effects state.
  - 7. Set the dropEffect attribute to "none" if e is dragstart, drag, or dragleave; to the value corresponding to the current drag operation if e is drop or dragend; and to a value based on the effectAllowed attribute's value and the drag-and-drop source, as given by the following table, otherwise (i.e. if e is dragenter or dragover): Where the table above provides possibly appropriate alternatives, us…
  - 8. Let event be the result of creating an event using DragEvent.
  - 9. Initialize event's type attribute to e, its bubbles attribute to true, its view attribute to window, its relatedTarget attribute to related target, and its dataTransfer attribute to dataTransfer.
  - 10. If e is not dragleave or dragend, then initialize event's cancelable attribute to true.
  - 11. Initialize event's mouse and key attributes according to the state of the input devices as they would be for user interaction events. If there is no relevant pointing device, then initialize event's screenX, screenY, clientX, clientY, and button attributes to 0.
  - 12. Dispatch event at the specified target element.
  - 13. Set the drag data store allowed effects state to the current value of dataTransfer's effectAllowed attribute. (It can only have changed value if e is dragstart.)
  - 14. If dataDragStoreWasChanged is true, then set the drag data store mode back to the protected mode.
  - 15. Break the association between dataTransfer and the drag data store.

#### [6.11.5 Processing model](https://html.spec.whatwg.org/#drag-and-drop-processing-model)

Status: reviewed (2026-11-01)
> `DragController` and `EventHandler`, as upstream's (see `DIFFERENCES.md`); drag events' modifier keys are missing (see `KNOWN-ISSUES.md`).
<!-- definitions: source-node list-of-dragged-nodes initiate-the-drag-and-drop-operation immediate-user-selection current-target-element current-drag-operation concept-current-drag-operation-none concept-current-drag-operation-copy concept-current-drag-operation-link -->

- [x] [User agents must, as soon as the drag operation is initiated and every 350ms (±…](https://html.spec.whatwg.org/#concept-current-drag-operation-move) — User agents must, as soon as the drag operation is initiated and every 350ms (±200ms) thereafter for as long as the drag operation is ongoing, queue a task to perform the following steps in sequence:
  - 1. If the user agent is still performing the previous iteration of the sequence (if any) when the next iteration becomes due, return for this iteration (effectively "skipping missed frames" of the drag-and-drop operation).
  - 2. Fire a DND event named drag at the source node. If this event is canceled, the user agent must set the current drag operation to "none" (no drag operation).
  - 3. If the drag event was not canceled and the user has not ended the drag-and-drop operation, check the state of the drag-and-drop operation, as follows:
    - 3a. If the user is indicating a different immediate user selection than during the last iteration (or if this is the first iteration), and if this immediate user selection is not the same as the current target element, then update the current target element as follows: If the new immediate user selection is nullSet the current target element to null also.If the new immediate user selection is in a no…
    - 3b. If the previous step caused the current target element to change, and if the previous target element was not null or a part of a non-DOM document, then fire a DND event named dragleave at the previous target element, with the new current target element as the specific related target.
    - 3c. If the current target element is a DOM element, then fire a DND event named dragover at this current target element. If the dragover event is not canceled, run the appropriate step from the following list: If the current target element is a text control (e.g., textarea, or an input element whose type attribute is in the Text state) or an editing host or editable element, and the drag data store i…
    - 3d. Otherwise, if the current target element is not a DOM element, use platform-specific mechanisms to determine what drag operation is being performed (none, copy, link, or move), and set the current drag operation accordingly.
    - 3e. Update the drag feedback (e.g. the mouse cursor) to match the current drag operation, as follows:
  - 4. Otherwise, if the user ended the drag-and-drop operation (e.g. by releasing the mouse button in a mouse-driven drag-and-drop interface), or if the drag event was canceled, then this will be the last iteration. Run the following steps, then stop the drag-and-drop operation:
    - 4a. If the current drag operation is "none" (no drag operation), or if the user ended the drag-and-drop operation by canceling it (e.g. by hitting the Escape key), or if the current target element is null, then the drag operation failed. Run these substeps: Otherwise, the drag operation might be a success; run these substeps:
      - 4a.i. Let dropped be false.
      - 4a.ii. If the current target element is a DOM element, fire a DND event named dragleave at it; otherwise, if it is not null, use platform-specific conventions for drag cancelation.
      - 4a.iii. Set the current drag operation to "none".
      - 4a.i. Let dropped be true.
      - 4a.ii. If the current target element is a DOM element, fire a DND event named drop at it; otherwise, use platform-specific conventions for indicating a drop.
      - 4a.iii. If the event is canceled, set the current drag operation to the value of the dropEffect attribute of the DragEvent object's dataTransfer object as it stood after the event dispatch finished. Otherwise, the event is not canceled; perform the event's default action, which depends on the exact target as follows: If the current target element is a text control (e.g., textarea, or an input element who…
    - 4b. Fire a DND event named dragend at the source node.
    - 4c. Run the appropriate steps from the following list as the default action of the dragend event: If dropped is true, the current target element is a text control (see below), the current drag operation is "move", and the source of the drag-and-drop operation is a selection in the DOM that is entirely contained within an editing hostDelete the selection.If dropped is true, the current target element…

#### [6.11.6 Events summary](https://html.spec.whatwg.org/#dndevents)

Status: n/a (summary)

#### [6.11.7 The draggable attribute](https://html.spec.whatwg.org/#the-draggable-attribute)

Status: reviewed (2026-11-01)
> `HTMLElement::Draggable`; objects don't show images, so none is draggable by default.
<!-- definitions: attr-draggable dom-draggable -->

- [x] [Auto](https://html.spec.whatwg.org/#attr-draggable-auto-state) — The attribute's missing value default and invalid value default are both the Auto state. The auto state uses the default behavior of the user agent.

#### [6.11.8 Security risks in the drag-and-drop model](https://html.spec.whatwg.org/#security-risks-in-the-drag-and-drop-model)

Status: reviewed (2026-11-01)
> Data is only given to `drop` (read-only) and `dragstart` (read/write); other events get protected data transfers.
