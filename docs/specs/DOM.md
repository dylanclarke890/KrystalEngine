# DOM review checklist

Generated from <https://dom.spec.whatwg.org/> on 2026-10-30 by `tools/spec_checklist.py`: 408 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[dom:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note.

## [Abstract](https://dom.spec.whatwg.org/#abstract)

Status: unreviewed

## [1. Infrastructure](https://dom.spec.whatwg.org/#infrastructure)

Status: unreviewed
<!-- definitions: other-applicable-specifications -->

### [1.1. Trees](https://dom.spec.whatwg.org/#trees)

Status: reviewed (2026-10-30)
> Code: `TreeQueries`'s Trees region (`IsParent`, `IsChild`, `Root`, `IsDescendant` and the rest, `Index`), `TreeQueries::Next`/`Previous` for tree order, and `Node::ParentNode`, `FirstChild`, `LastChild`, `PreviousSibling` and `NextSibling`. Tests: `tests/DOM/Algorithms/TreeQueries.cpp`.
> `IsSibling` excludes the node itself, as "inclusive sibling" implies, though the definition alone doesn't.
<!-- definitions: concept-tree concept-tree-order concept-tree-participate concept-tree-parent concept-tree-child concept-tree-root concept-tree-descendant concept-tree-inclusive-descendant concept-tree-ancestor concept-tree-inclusive-ancestor concept-tree-sibling concept-tree-inclusive-sibling concept-tree-preceding concept-tree-following concept-tree-first-child concept-tree-last-child concept-tree-previous-sibling concept-tree-next-sibling concept-tree-index -->

### [1.2. Ordered sets](https://dom.spec.whatwg.org/#ordered-sets)

Status: reviewed (2026-10-30)
> Code: `OrderedSet::Parser` and `OrderedSet::Serialiser`, over Infra's `StringAlgorithms::SplitOnWhitespace` and `Concatenate`. Tests: `tests/DOM/Algorithms/OrderedSet.cpp`.

- [x] [ordered set parser](https://dom.spec.whatwg.org/#concept-ordered-set-parser) — The ordered set parser takes a string input and then runs these steps:
  - 1. Let inputTokens be the result of splitting input on ASCII whitespace.
  - 2. Let tokens be a new ordered set.
  - 3. For each token of inputTokens: append token to tokens.
  - 4. Return tokens.

- [x] [ordered set serializer](https://dom.spec.whatwg.org/#concept-ordered-set-serializer) — The ordered set serializer takes a set and returns the concatenation of set using U+0020 SPACE.

### [1.3. Selectors](https://dom.spec.whatwg.org/#selectors)

Status: unreviewed

- [ ] [scope-match a selectors string](https://dom.spec.whatwg.org/#scope-match-a-selectors-string) — To scope-match a selectors string given a string selectors against a node node:
  - 1. Let selector be the result of parse a selector selectors. [SELECTORS4]
  - 2. If selector is failure, then throw a "SyntaxError" DOMException.
  - 3. Return the result of match a selector against a tree with selector and node’s root using scoping root node. [SELECTORS4].

### [1.4. Name validation](https://dom.spec.whatwg.org/#namespaces)

Status: unreviewed
<!-- definitions: valid-doctype-name -->

- [ ] [valid namespace prefix](https://dom.spec.whatwg.org/#valid-namespace-prefix) — A string is a valid namespace prefix if its length is at least 1 and it does not contain ASCII whitespace, U+0000 NULL, U+002F (/), or U+003E (>).

- [ ] [valid attribute local name](https://dom.spec.whatwg.org/#valid-attribute-local-name) — A string is a valid attribute local name if its length is at least 1 and it does not contain ASCII whitespace, U+0000 NULL, U+002F (/), U+003D (=), or U+003E (>).

- [ ] [valid element local name](https://dom.spec.whatwg.org/#valid-element-local-name) — A string name is a valid element local name if the following steps return true:
  - 1. If name’s length is 0, then return false.
  - 2. If name’s 0th code point is an ASCII alpha:
    - 2a. If name contains ASCII whitespace, U+0000 NULL, U+002F (/), or U+003E (>), then return false.
    - 2b. Return true.
  - 3. If name’s 0th code point is not U+003A (:), U+005F (_), or in the range U+0080 to U+10FFFF, inclusive, then return false.
  - 4. If name’s subsequent code points, if any, are not ASCII alphas, ASCII digits, U+002D (-), U+002E (.), U+003A (:), U+005F (_), or in the range U+0080 to U+10FFFF, inclusive, then return false.
  - 5. Return true.

- [ ] [validate and extract](https://dom.spec.whatwg.org/#validate-and-extract) — To validate and extract a namespace and qualifiedName, given a context:
  - 1. If namespace is the empty string, then set it to null.
  - 2. Let prefix be null.
  - 3. Let localName be qualifiedName.
  - 4. If qualifiedName contains a U+003A (:):
    - 4a. Set prefix to the part of qualifiedName before the first U+003A (:).
    - 4b. Set localName to the part of qualifiedName after the first U+003A (:).
    - 4c. If prefix is not a valid namespace prefix, then throw an "InvalidCharacterError" DOMException.
  - 5. Assert: prefix is either null or a valid namespace prefix.
  - 6. If context is "attribute" and localName is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
  - 7. If context is "element" and localName is not a valid element local name, then throw an "InvalidCharacterError" DOMException.
  - 8. If prefix is non-null and namespace is null, then throw a "NamespaceError" DOMException.
  - 9. If prefix is "xml" and namespace is not the XML namespace, then throw a "NamespaceError" DOMException.
  - 10. If either qualifiedName or prefix is "xmlns" and namespace is not the XMLNS namespace, then throw a "NamespaceError" DOMException.
  - 11. If namespace is the XMLNS namespace and neither qualifiedName nor prefix is "xmlns", then throw a "NamespaceError" DOMException.
  - 12. Return (namespace, prefix, localName).

## [2. Events](https://dom.spec.whatwg.org/#events)

Status: unreviewed

### [2.1. Introduction to "DOM Events"](https://dom.spec.whatwg.org/#introduction-to-dom-events)

Status: unreviewed

### [2.2. Interface Event](https://dom.spec.whatwg.org/#interface-event)

Status: unreviewed
<!-- definitions: concept-event potential-event-target event-target event-relatedtarget event-touch-target-list event-path event-path-item event-path-invocation-target event-path-invocation-target-in-shadow-tree event-path-shadow-adjusted-target event-path-relatedtarget event-path-touch-target-list event-path-root-of-closed-tree event-path-slot-in-closed-tree dom-event-none dom-event-capturing_phase dom-event-at_target dom-event-bubbling_phase stop-propagation-flag stop-immediate-propagation-flag canceled-flag in-passive-listener-flag composed-flag initialized-flag dispatch-flag dom-event-cancelable -->

- [ ] [Event.type](https://dom.spec.whatwg.org/#dom-event-type) — The type attribute must return the value it was initialized to. When an event is created the attribute must be initialized to the empty string.

- [ ] [Event.target](https://dom.spec.whatwg.org/#dom-event-target) — The target getter steps are to return this’s target.

- [ ] [Event.srcElement](https://dom.spec.whatwg.org/#dom-event-srcelement) — The srcElement getter steps are to return this’s target.

- [ ] [Event.currentTarget](https://dom.spec.whatwg.org/#dom-event-currenttarget) — The currentTarget attribute must return the value it was initialized to. When an event is created the attribute must be initialized to null.

- [ ] [Event.composedPath()](https://dom.spec.whatwg.org/#dom-event-composedpath) — The composedPath() method steps are:
  - 1. Let composedPath be an empty list.
  - 2. Let path be this’s path.
  - 3. If path is empty, then return composedPath.
  - 4. Let currentTarget be this’s currentTarget attribute value.
  - 5. Assert: currentTarget is an EventTarget object.
  - 6. Append currentTarget to composedPath.
  - 7. Let currentTargetIndex be 0.
  - 8. Let currentTargetHiddenSubtreeLevel be 0.
  - 9. Let index be path’s size − 1.
  - 10. While index is greater than or equal to 0:
    - 10a. If path[index]'s root-of-closed-tree is true, then increase currentTargetHiddenSubtreeLevel by 1.
    - 10b. If path[index]'s invocation target is currentTarget, then set currentTargetIndex to index and break.
    - 10c. If path[index]'s slot-in-closed-tree is true, then decrease currentTargetHiddenSubtreeLevel by 1.
    - 10d. Decrease index by 1.
  - 11. Let currentHiddenLevel and maxHiddenLevel be currentTargetHiddenSubtreeLevel.
  - 12. Set index to currentTargetIndex − 1.
  - 13. While index is greater than or equal to 0:
    - 13a. If path[index]'s root-of-closed-tree is true, then increase currentHiddenLevel by 1.
    - 13b. If currentHiddenLevel is less than or equal to maxHiddenLevel, then prepend path[index]'s invocation target to composedPath.
    - 13c. If path[index]'s slot-in-closed-tree is true:
      - 13c.i. Decrease currentHiddenLevel by 1.
      - 13c.ii. If currentHiddenLevel is less than maxHiddenLevel, then set maxHiddenLevel to currentHiddenLevel.
    - 13d. Decrease index by 1.
  - 14. Set currentHiddenLevel and maxHiddenLevel to currentTargetHiddenSubtreeLevel.
  - 15. Set index to currentTargetIndex + 1.
  - 16. While index is less than path’s size:
    - 16a. If path[index]'s slot-in-closed-tree is true, then increase currentHiddenLevel by 1.
    - 16b. If currentHiddenLevel is less than or equal to maxHiddenLevel, then append path[index]'s invocation target to composedPath.
    - 16c. If path[index]'s root-of-closed-tree is true:
      - 16c.i. Decrease currentHiddenLevel by 1.
      - 16c.ii. If currentHiddenLevel is less than maxHiddenLevel, then set maxHiddenLevel to currentHiddenLevel.
    - 16d. Increase index by 1.
  - 17. Return composedPath.

- [ ] [Event.eventPhase](https://dom.spec.whatwg.org/#dom-event-eventphase) — The eventPhase attribute must return the value it was initialized to, which must be one of the following:
  - • NONE (numeric value 0): Events not currently dispatched are in this phase.
  - • CAPTURING_PHASE (numeric value 1): When an event is dispatched to an object that participates in a tree it will be in this phase before it reaches its target.
  - • AT_TARGET (numeric value 2): When an event is dispatched it will be in this phase on its target.
  - • BUBBLING_PHASE (numeric value 3): When an event is dispatched to an object that participates in a tree it will be in this phase after it reaches its target.

- [ ] [Event.stopPropagation()](https://dom.spec.whatwg.org/#dom-event-stoppropagation) — The stopPropagation() method steps are to set this’s stop propagation flag.

- [ ] [Event.cancelBubble](https://dom.spec.whatwg.org/#dom-event-cancelbubble) — The cancelBubble getter steps are to return true if this’s stop propagation flag is set; otherwise false.

- [ ] [The cancelBubble setter steps](https://dom.spec.whatwg.org/#dom-event-cancelbubble) — The cancelBubble setter steps are to set this’s stop propagation flag if the given value is true; otherwise do nothing.

- [ ] [Event.stopImmediatePropagation()](https://dom.spec.whatwg.org/#dom-event-stopimmediatepropagation) — The stopImmediatePropagation() method steps are to set this’s stop propagation flag and this’s stop immediate propagation flag.

- [ ] [Event.bubbles](https://dom.spec.whatwg.org/#dom-event-bubbles) — The bubbles and cancelable attributes must return the values they were initialized to.

- [ ] [set the canceled flag](https://dom.spec.whatwg.org/#set-the-canceled-flag) — To set the canceled flag, given an event event, if event’s cancelable attribute value is true and event’s in passive listener flag is unset, then set event’s canceled flag, and do nothing otherwise.

- [ ] [Event.returnValue](https://dom.spec.whatwg.org/#dom-event-returnvalue) — The returnValue getter steps are to return false if this’s canceled flag is set; otherwise true.

- [ ] [The returnValue setter steps](https://dom.spec.whatwg.org/#dom-event-returnvalue) — The returnValue setter steps are to set the canceled flag with this if the given value is false; otherwise do nothing.

- [ ] [Event.preventDefault()](https://dom.spec.whatwg.org/#dom-event-preventdefault) — The preventDefault() method steps are to set the canceled flag with this.

- [ ] [Event.defaultPrevented](https://dom.spec.whatwg.org/#dom-event-defaultprevented) — The defaultPrevented getter steps are to return true if this’s canceled flag is set; otherwise false.

- [ ] [Event.composed](https://dom.spec.whatwg.org/#dom-event-composed) — The composed getter steps are to return true if this’s composed flag is set; otherwise false.

- [ ] [Event.isTrusted](https://dom.spec.whatwg.org/#dom-event-istrusted) — The isTrusted attribute must return the value it was initialized to. When an event is created the attribute must be initialized to false.

- [ ] [Event.timeStamp](https://dom.spec.whatwg.org/#dom-event-timestamp) — The timeStamp attribute must return the value it was initialized to.

- [ ] [initialize (Event)](https://dom.spec.whatwg.org/#concept-event-initialize) — To initialize an event, with type, bubbles, and cancelable, run these steps:
  - 1. Set event’s initialized flag.
  - 2. Unset event’s stop propagation flag, stop immediate propagation flag, and canceled flag.
  - 3. Set event’s isTrusted attribute to false.
  - 4. Set event’s target to null.
  - 5. Set event’s type attribute to type.
  - 6. Set event’s bubbles attribute to bubbles.
  - 7. Set event’s cancelable attribute to cancelable.

- [ ] [Event.initEvent(type, bubbles, cancelable)](https://dom.spec.whatwg.org/#dom-event-initevent) — The initEvent(type, bubbles, cancelable) method steps are:
  - 1. If this’s dispatch flag is set, then return.
  - 2. Initialize this with type, bubbles, and cancelable.

### [2.3. Legacy extensions to the Window interface](https://dom.spec.whatwg.org/#interface-window-extensions)

Status: unreviewed
<!-- definitions: window-current-event -->

- [ ] [Window.event](https://dom.spec.whatwg.org/#dom-window-event) — The event getter steps are to return this’s current event.

### [2.4. Interface CustomEvent](https://dom.spec.whatwg.org/#interface-customevent)

Status: unreviewed

- [ ] [CustomEvent.detail](https://dom.spec.whatwg.org/#dom-customevent-detail) — The detail attribute must return the value it was initialized to.

- [ ] [CustomEvent.initCustomEvent(type, bubbles, cancelable, detail)](https://dom.spec.whatwg.org/#dom-customevent-initcustomevent) — The initCustomEvent(type, bubbles, cancelable, detail) method steps are:
  - 1. If this’s dispatch flag is set, then return.
  - 2. Initialize this with type, bubbles, and cancelable.
  - 3. Set this’s detail attribute to detail.

### [2.5. Constructing events](https://dom.spec.whatwg.org/#constructing-events)

Status: unreviewed
<!-- definitions: concept-event-constructor-ext -->

- [ ] [constructor (Event)](https://dom.spec.whatwg.org/#concept-event-constructor) — When a constructor of the Event interface, or of an interface that inherits from the Event interface, is invoked, these steps must be run, given the arguments type and eventInitDict:
  - 1. Let event be the result of running the inner event creation steps with this interface, null, now, and eventInitDict.
  - 2. Initialize event’s type attribute to type.
  - 3. Return event.

- [ ] [create an event](https://dom.spec.whatwg.org/#concept-event-create) — To create an event using eventInterface, which must be either Event or an interface that inherits from it, and optionally given a realm realm, run these steps:
  - 1. If realm is not given, then set it to null.
  - 2. Let dictionary be the result of converting the JavaScript value undefined to the dictionary type accepted by eventInterface’s constructor. (This dictionary type will either be EventInit or a dictionary that inherits from it.)
  - 3. Let event be the result of running the inner event creation steps with eventInterface, realm, the time of the occurrence that the event is signaling, and dictionary.
  - 4. Initialize event’s isTrusted attribute to true.
  - 5. Return event.

- [ ] [inner event creation steps](https://dom.spec.whatwg.org/#inner-event-creation-steps) — The inner event creation steps, given an eventInterface, realm, time, and dictionary, are as follows:
  - 1. Let event be the result of creating a new object using eventInterface. If realm is non-null, then use that realm; otherwise, use the default behavior defined in Web IDL.
  - 2. Set event’s initialized flag.
  - 3. Initialize event’s timeStamp attribute to the relative high resolution coarse time given time and event’s relevant global object.
  - 4. For each member → value of dictionary: if event has an attribute whose identifier is member, then initialize that attribute to value.
  - 5. Run the event constructing steps with event and dictionary.
  - 6. Return event.

### [2.6. Defining event interfaces](https://dom.spec.whatwg.org/#defining-event-interfaces)

Status: unreviewed

### [2.7. Interface EventTarget](https://dom.spec.whatwg.org/#interface-eventtarget)

Status: unreviewed
<!-- definitions: eventtarget-event-listener-list concept-event-listener event-listener-type event-listener-callback event-listener-capture event-listener-passive event-listener-once event-listener-signal event-listener-removed get-the-parent eventtarget-activation-behavior eventtarget-legacy-pre-activation-behavior eventtarget-legacy-canceled-activation-behavior -->

- [ ] [flatten (Event)](https://dom.spec.whatwg.org/#concept-flatten-options) — To flatten options, run these steps:
  - 1. If options is a boolean, then return options.
  - 2. Return options["capture"].

- [ ] [flatten more (Event)](https://dom.spec.whatwg.org/#event-flatten-more) — To flatten more options, run these steps:
  - 1. Let capture be the result of flattening options.
  - 2. Let once be false.
  - 3. Let passive and signal be null.
  - 4. If options is a dictionary:
    - 4a. Set once to options["once"].
    - 4b. If options["passive"] exists, then set passive to options["passive"].
    - 4c. If options["signal"] exists, then set signal to options["signal"].
  - 5. Return capture, passive, once, and signal.

- [ ] [EventTarget.new EventTarget()](https://dom.spec.whatwg.org/#dom-eventtarget-eventtarget) — The new EventTarget() constructor steps are to do nothing.

- [ ] [default passive value](https://dom.spec.whatwg.org/#default-passive-value) — The default passive value, given an event type type and an EventTarget eventTarget, is determined as follows:
  - 1. Return true if all of the following are true: type is one of "touchstart", "touchmove", "wheel", or "mousewheel". [TOUCH-EVENTS] [UIEVENTS]; eventTarget is a Window object, or is a node whose node document is eventTarget, or is a node whose node document’s document element is eventTarget, or is a node whose node document’s body element is eventTarget. [HTML]
  - 2. Return false.

- [ ] [add an event listener](https://dom.spec.whatwg.org/#add-an-event-listener) — To add an event listener, given an EventTarget object eventTarget and an event listener listener, run these steps:
  - 1. If eventTarget is a ServiceWorkerGlobalScope object, its service worker’s script resource’s has ever been evaluated flag is set, and listener’s type matches the type attribute value of any of the service worker events, then report a warning to the console that this might not give the expected results. [SERVICE-WORKERS]
  - 2. If listener’s signal is non-null and is aborted, then return.
  - 3. If listener’s callback is null, then return.
  - 4. If listener’s passive is null, then set it to the default passive value given listener’s type and eventTarget.
  - 5. If eventTarget’s event listener list does not contain an event listener whose type is listener’s type, callback is listener’s callback, and capture is listener’s capture, then append listener to eventTarget’s event listener list.
  - 6. If listener’s signal is non-null, then add the following abort steps to it:
    - 6a. Remove an event listener with eventTarget and listener.

- [ ] [EventTarget.addEventListener(type, callback, options)](https://dom.spec.whatwg.org/#dom-eventtarget-addeventlistener) — The addEventListener(type, callback, options) method steps are:
  - 1. Let capture, passive, once, and signal be the result of flattening more options.
  - 2. Add an event listener with this and an event listener whose type is type, callback is callback, capture is capture, passive is passive, once is once, and signal is signal.

- [ ] [remove an event listener](https://dom.spec.whatwg.org/#remove-an-event-listener) — To remove an event listener, given an EventTarget object eventTarget and an event listener listener, run these steps:
  - 1. If eventTarget is a ServiceWorkerGlobalScope object and its service worker’s set of event types to handle contains listener’s type, then report a warning to the console that this might not give the expected results. [SERVICE-WORKERS]
  - 2. Set listener’s removed to true and remove listener from eventTarget’s event listener list.

- [ ] [remove all event listeners](https://dom.spec.whatwg.org/#remove-all-event-listeners) — To remove all event listeners, given an EventTarget object eventTarget: for each listener of eventTarget’s event listener list: remove an event listener with eventTarget and listener.

- [ ] [EventTarget.removeEventListener(type, callback, options)](https://dom.spec.whatwg.org/#dom-eventtarget-removeeventlistener) — The removeEventListener(type, callback, options) method steps are:
  - 1. Let capture be the result of flattening options.
  - 2. If this’s event listener list contains an event listener whose type is type, callback is callback, and capture is capture, then remove an event listener with this and that event listener.

- [ ] [EventTarget.dispatchEvent(event)](https://dom.spec.whatwg.org/#dom-eventtarget-dispatchevent) — The dispatchEvent(event) method steps are:
  - 1. If event’s dispatch flag is set, or if its initialized flag is not set, then throw an "InvalidStateError" DOMException.
  - 2. Initialize event’s isTrusted attribute to false.
  - 3. Return the result of dispatching event to this.

### [2.8. Observing event listeners](https://dom.spec.whatwg.org/#observing-event-listeners)

Status: unreviewed

- [ ] [legacy-obtain service worker fetch event listener callbacks](https://dom.spec.whatwg.org/#legacy-obtain-service-worker-fetch-event-listener-callbacks) — To legacy-obtain service worker fetch event listener callbacks given a ServiceWorkerGlobalScope global, run these steps. They return a list of EventListener objects.
  - 1. Let callbacks be « ».
  - 2. For each listener of global’s event listener list: if listener’s type is "fetch" and listener’s callback is non-null, then append listener’s callback to callbacks.
  - 3. Return callbacks.

### [2.9. Dispatching events](https://dom.spec.whatwg.org/#dispatching-events)

Status: unreviewed

- [ ] [dispatch](https://dom.spec.whatwg.org/#concept-event-dispatch) — To dispatch an event event to an EventTarget object target, with an optional legacy target override flag and an optional legacyOutputDidListenersThrowFlag:
  - 1. Set event’s dispatch flag.
  - 2. Let targetOverride be target, if legacy target override flag is not given, and target’s associated Document otherwise. [HTML]
  - 3. Let activationTarget be null.
  - 4. Let relatedTarget be the result of retargeting event’s relatedTarget against target.
  - 5. Let clearTargets be false.
  - 6. If target is not relatedTarget or target is event’s relatedTarget:
    - 6a. Let touchTargets be a new list.
    - 6b. For each touchTarget of event’s touch target list: append the result of retargeting touchTarget against target to touchTargets.
    - 6c. Append to an event path with event, target, targetOverride, relatedTarget, touchTargets, and false.
    - 6d. Let isActivationEvent be true, if event is a MouseEvent object and event’s type attribute is "click"; otherwise false.
    - 6e. If isActivationEvent is true and target has activation behavior, then set activationTarget to target.
    - 6f. Let slottable be target, if target is a slottable and is assigned, and null otherwise.
    - 6g. Let slotInClosedTree be false.
    - 6h. Let parent be the result of invoking target’s get the parent with event.
    - 6i. While parent is non-null:
      - 6i.i. If slottable is non-null:
        - 6i.i.A. Assert: parent is a slot.
        - 6i.i.B. Set slottable to null.
        - 6i.i.C. If parent’s root is a shadow root whose mode is "closed", then set slotInClosedTree to true.
      - 6i.ii. If parent is a slottable and is assigned, then set slottable to parent.
      - 6i.iii. Let relatedTarget be the result of retargeting event’s relatedTarget against parent.
      - 6i.iv. Let touchTargets be a new list.
      - 6i.v. For each touchTarget of event’s touch target list: append the result of retargeting touchTarget against parent to touchTargets.
      - 6i.vi. If parent is a Window object, or parent is a node and target’s root is a shadow-including inclusive ancestor of parent:
        - 6i.vi.A. If isActivationEvent is true, event’s bubbles attribute is true, activationTarget is null, and parent has activation behavior, then set activationTarget to parent.
        - 6i.vi.B. Append to an event path with event, parent, null, relatedTarget, touchTargets, and slotInClosedTree.
      - 6i.vii. Otherwise, if parent is relatedTarget, then set parent to null.
      - 6i.viii. Otherwise:
        - 6i.viii.A. Set target to parent.
        - 6i.viii.B. If isActivationEvent is true, activationTarget is null, and target has activation behavior, then set activationTarget to target.
        - 6i.viii.C. Append to an event path with event, parent, target, relatedTarget, touchTargets, and slotInClosedTree.
      - 6i.ix. If parent is non-null, then set parent to the result of invoking parent’s get the parent with event.
      - 6i.x. Set slotInClosedTree to false.
    - 6j. Let clearTargetsItem be the last event path item in event’s path whose shadow-adjusted target is non-null.
    - 6k. If clearTargetsItem’s shadow-adjusted target, clearTargetsItem’s relatedTarget, or an EventTarget object in clearTargetsItem’s touch target list is a node whose root is a shadow root: set clearTargets to true.
    - 6l. If activationTarget is non-null and activationTarget has legacy-pre-activation behavior, then run activationTarget’s legacy-pre-activation behavior.
    - 6m. For each item of event’s path, in reverse order:
      - 6m.i. If item’s shadow-adjusted target is non-null, then set event’s eventPhase attribute to AT_TARGET.
      - 6m.ii. Otherwise, set event’s eventPhase attribute to CAPTURING_PHASE.
      - 6m.iii. Invoke with item, event, "capturing", and legacyOutputDidListenersThrowFlag if given.
    - 6n. For each item of event’s path:
      - 6n.i. If item’s shadow-adjusted target is non-null, then set event’s eventPhase attribute to AT_TARGET.
      - 6n.ii. Otherwise:
        - 6n.ii.A. If event’s bubbles attribute is false, then continue.
        - 6n.ii.B. Set event’s eventPhase attribute to BUBBLING_PHASE.
      - 6n.iii. Invoke with item, event, "bubbling", and legacyOutputDidListenersThrowFlag if given.
  - 7. Set event’s eventPhase attribute to NONE.
  - 8. Set event’s currentTarget attribute to null.
  - 9. Set event’s path to the empty list.
  - 10. Unset event’s dispatch flag, stop propagation flag, and stop immediate propagation flag.
  - 11. If clearTargets is true:
    - 11a. Set event’s target to null.
    - 11b. Set event’s relatedTarget to null.
    - 11c. Set event’s touch target list to the empty list.
  - 12. If activationTarget is non-null:
    - 12a. If event’s canceled flag is unset, then run activationTarget’s activation behavior with event.
    - 12b. Otherwise, if activationTarget has legacy-canceled-activation behavior, then run activationTarget’s legacy-canceled-activation behavior.
  - 13. Return false if event’s canceled flag is set; otherwise true.

- [ ] [append to an event path](https://dom.spec.whatwg.org/#concept-event-path-append) — To append to an event path, given an event event, an EventTarget object invocationTarget, a potential event target shadowAdjustedTarget, a potential event target relatedTarget, a list of potential event targets touchTargets, and a boolean slotInClosedTree:
  - 1. Let invocationTargetInShadowTree be false.
  - 2. If invocationTarget is a node and its root is a shadow root, then set invocationTargetInShadowTree to true.
  - 3. Let rootOfClosedTree be false.
  - 4. If invocationTarget is a shadow root whose mode is "closed", then set rootOfClosedTree to true.
  - 5. Append a new event path item to event’s path whose invocation target is invocationTarget, invocation-target-in-shadow-tree is invocationTargetInShadowTree, shadow-adjusted target is shadowAdjustedTarget, relatedTarget is relatedTarget, touch target list is touchTargets, root-of-closed-tree is rootOfClosedTree, and slot-in-closed-tree is slotInClosedTree.

- [ ] [invoke](https://dom.spec.whatwg.org/#concept-event-listener-invoke) — To invoke, given an event path item pathItem, an event event, a string phase, and an optional legacyOutputDidListenersThrowFlag:
  - 1. Let targetItem be pathItem.
  - 2. While targetItem’s shadow-adjusted target is null: set targetItem to the event path item preceding targetItem in event’s path.
  - 3. Set event’s target to targetItem’s shadow-adjusted target.
  - 4. Set event’s relatedTarget to pathItem’s relatedTarget.
  - 5. Set event’s touch target list to pathItem’s touch target list.
  - 6. If event’s stop propagation flag is set, then return.
  - 7. Initialize event’s currentTarget attribute to pathItem’s invocation target.
  - 8. Let listeners be a clone of event’s currentTarget attribute value’s event listener list.
  - 9. Let invocationTargetInShadowTree be pathItem’s invocation-target-in-shadow-tree.
  - 10. Let found be the result of running inner invoke with event, listeners, phase, invocationTargetInShadowTree, and legacyOutputDidListenersThrowFlag if given.
  - 11. If found is false and event’s isTrusted attribute is true:
    - 11a. Let originalEventType be event’s type attribute value.
    - 11b. If event’s type attribute value is a match for any of the strings in the first column in the following table, set event’s type attribute value to the string in the second column on the same row as the matching string, and return otherwise.
    - 11c. Inner invoke with event, listeners, phase, invocationTargetInShadowTree, and legacyOutputDidListenersThrowFlag if given.
    - 11d. Set event’s type attribute value to originalEventType.

- [ ] [inner invoke](https://dom.spec.whatwg.org/#concept-event-listener-inner-invoke) — To inner invoke, given an event event, a list of event listeners listeners, a string phase, a boolean invocationTargetInShadowTree, and an optional legacyOutputDidListenersThrowFlag:
  - 1. Let found be false.
  - 2. For each listener of listeners, whose removed is false:
    - 2a. If event’s type attribute value is not listener’s type, then continue.
    - 2b. Set found to true.
    - 2c. If phase is "capturing" and listener’s capture is false, then continue.
    - 2d. If phase is "bubbling" and listener’s capture is true, then continue.
    - 2e. If listener’s once is true, then remove an event listener given event’s currentTarget attribute value and listener.
    - 2f. Let targetGlobal be event’s currentTarget’s node document’s relevant global object if event’s currentTarget is a node; otherwise event’s currentTarget’s relevant global object.
    - 2g. Let listenerGlobal be listener callback’s associated realm’s global object.
    - 2h. If targetGlobal or listenerGlobal is a Window object whose associated Document is not fully active, then continue.
    - 2i. Let currentEvent be undefined.
    - 2j. If listenerGlobal is a Window object:
      - 2j.i. Set currentEvent to listenerGlobal’s current event.
      - 2j.ii. If invocationTargetInShadowTree is false, then set listenerGlobal’s current event to event.
    - 2k. If listener’s passive is true, then set event’s in passive listener flag.
    - 2l. If listenerGlobal is a Window object, then record timing info for event listener given event and listener.
    - 2m. Call a user object’s operation with listener’s callback, "handleEvent", « event », and event’s currentTarget attribute value. If this throws an exception exception:
      - 2m.i. Report exception for listener’s callback’s corresponding JavaScript object’s associated realm’s global object.
      - 2m.ii. Set legacyOutputDidListenersThrowFlag if given.
    - 2n. Unset event’s in passive listener flag.
    - 2o. If listenerGlobal is a Window object, then set listenerGlobal’s current event to currentEvent.
    - 2p. If event’s stop immediate propagation flag is set, then break.
  - 3. Return found.

### [2.10. Firing events](https://dom.spec.whatwg.org/#firing-events)

Status: unreviewed

- [ ] [fire an event](https://dom.spec.whatwg.org/#concept-event-fire) — To fire an event named e at target, optionally using an eventConstructor, with a description of how IDL attributes are to be initialized, and a legacy target override flag, run these steps:
  - 1. Let recordingTimingAsTask be the result of calling start recording task time if needed given target’s relevant settings object.
  - 2. If eventConstructor is not given, then let eventConstructor be Event.
  - 3. Let event be the result of creating an event given eventConstructor, in the relevant realm of target.
  - 4. Initialize event’s type attribute to e.
  - 5. Initialize any other IDL attributes of event as described in the invocation of this algorithm.
  - 6. Let result be the result of dispatching event at target, with legacy target override flag set if set.
  - 7. If recordingTimingAsTask is true:
    - 7a. Assert: target’s relevant settings object is a Document.
    - 7b. Record task end time given the unsafe shared current time and target’s relevant settings object.
  - 8. Return result.

### [2.11. Action versus occurrence](https://dom.spec.whatwg.org/#action-versus-occurrence)

Status: unreviewed

## [3. Aborting ongoing activities](https://dom.spec.whatwg.org/#aborting-ongoing-activities)

Status: unreviewed

### [3.1. Interface AbortController](https://dom.spec.whatwg.org/#interface-abortcontroller)

Status: unreviewed
<!-- definitions: abortcontroller-signal -->

- [ ] [AbortController.new AbortController()](https://dom.spec.whatwg.org/#dom-abortcontroller-abortcontroller) — The new AbortController() constructor steps are:
  - 1. Let signal be a new AbortSignal object.
  - 2. Set this’s signal to signal.

- [ ] [AbortController.signal](https://dom.spec.whatwg.org/#dom-abortcontroller-signal) — The signal getter steps are to return this’s signal.

- [ ] [AbortController.abort(reason)](https://dom.spec.whatwg.org/#dom-abortcontroller-abort) — The abort(reason) method steps are to signal abort on this with reason if it is given.

- [ ] [signal abort (AbortController)](https://dom.spec.whatwg.org/#abortcontroller-signal-abort) — To signal abort on an AbortController controller with an optional reason, signal abort on controller’s signal with reason if it is given.

### [3.2. Interface AbortSignal](https://dom.spec.whatwg.org/#interface-AbortSignal)

Status: unreviewed
<!-- definitions: abortsignal-abort-reason abortsignal-abort-algorithms abortsignal-dependent abortsignal-source-signals abortsignal-dependent-signals dom-abortsignal-onabort abortsignal-onabort eventdef-abortsignal-abort abortsignal-aborted -->

- [ ] [AbortSignal.abort(reason)](https://dom.spec.whatwg.org/#dom-abortsignal-abort) — The static abort(reason) method steps are:
  - 1. Let signal be a new AbortSignal object.
  - 2. Set signal’s abort reason to reason if it is given; otherwise to a new "AbortError" DOMException.
  - 3. Return signal.

- [ ] [AbortSignal.timeout(milliseconds)](https://dom.spec.whatwg.org/#dom-abortsignal-timeout) — The static timeout(milliseconds) method steps are:
  - 1. Let signal be a new AbortSignal object.
  - 2. Let global be signal’s relevant global object.
  - 3. Run steps after a timeout given global, "AbortSignal-timeout", milliseconds, and the following step: For the duration of this timeout, if signal has any event listeners registered for its abort event, there must be a strong reference from global to signal.
    - 3a. Queue a global task on the timer task source given global to signal abort given signal and a new "TimeoutError" DOMException.
  - 4. Return signal.

- [ ] [AbortSignal.any(signals)](https://dom.spec.whatwg.org/#dom-abortsignal-any) — The static any(signals) method steps are to return the result of creating a dependent abort signal from signals using AbortSignal and the current realm.

- [ ] [AbortSignal.aborted](https://dom.spec.whatwg.org/#dom-abortsignal-aborted) — The aborted getter steps are to return true if this is aborted; otherwise false.

- [ ] [AbortSignal.reason](https://dom.spec.whatwg.org/#dom-abortsignal-reason) — The reason getter steps are to return this’s abort reason.

- [ ] [AbortSignal.throwIfAborted()](https://dom.spec.whatwg.org/#dom-abortsignal-throwifaborted) — The throwIfAborted() method steps are to throw this’s abort reason, if this is aborted.

- [ ] [add (AbortSignal)](https://dom.spec.whatwg.org/#abortsignal-add) — To add an algorithm algorithm to an AbortSignal object signal:
  - 1. If signal is aborted, then return.
  - 2. Append algorithm to signal’s abort algorithms.

- [ ] [remove (AbortSignal)](https://dom.spec.whatwg.org/#abortsignal-remove) — To remove an algorithm algorithm from an AbortSignal signal, remove algorithm from signal’s abort algorithms.

- [ ] [signal abort (AbortSignal)](https://dom.spec.whatwg.org/#abortsignal-signal-abort) — To signal abort, given an AbortSignal object signal and an optional reason:
  - 1. If signal is aborted, then return.
  - 2. Set signal’s abort reason to reason if it is given; otherwise to a new "AbortError" DOMException.
  - 3. Let dependentSignalsToAbort be a new list.
  - 4. For each dependentSignal of signal’s dependent signals:
    - 4a. If dependentSignal is not aborted:
      - 4a.i. Set dependentSignal’s abort reason to signal’s abort reason.
      - 4a.ii. Append dependentSignal to dependentSignalsToAbort.
  - 5. Run the abort steps for signal.
  - 6. For each dependentSignal of dependentSignalsToAbort, run the abort steps for dependentSignal.

- [ ] [run the abort steps](https://dom.spec.whatwg.org/#run-the-abort-steps) — To run the abort steps for an AbortSignal signal:
  - 1. For each algorithm of signal’s abort algorithms: run algorithm.
  - 2. Empty signal’s abort algorithms.
  - 3. Fire an event named abort at signal.

- [ ] [create a dependent abort signal](https://dom.spec.whatwg.org/#create-a-dependent-abort-signal) — To create a dependent abort signal from a list of AbortSignal objects signals, using signalInterface, which must be either AbortSignal or an interface that inherits from it, and a realm:
  - 1. Let resultSignal be a new object implementing signalInterface using realm.
  - 2. For each signal of signals: if signal is aborted, then set resultSignal’s abort reason to signal’s abort reason and return resultSignal.
  - 3. Set resultSignal’s dependent to true.
  - 4. For each signal of signals:
    - 4a. If signal’s dependent is false:
      - 4a.i. Append signal to resultSignal’s source signals.
      - 4a.ii. Append resultSignal to signal’s dependent signals.
    - 4b. Otherwise, for each sourceSignal of signal’s source signals:
      - 4b.i. Assert: sourceSignal is not aborted and not dependent.
      - 4b.ii. Append sourceSignal to resultSignal’s source signals.
      - 4b.iii. Append resultSignal to sourceSignal’s dependent signals.
  - 5. Return resultSignal.

#### [3.2.1. Garbage collection](https://dom.spec.whatwg.org/#abort-signal-garbage-collection)

Status: unreviewed

### [3.3. Using AbortController and AbortSignal objects in APIs](https://dom.spec.whatwg.org/#abortcontroller-api-integration)

Status: unreviewed

## [4. Nodes](https://dom.spec.whatwg.org/#nodes)

Status: unreviewed

### [4.1. Introduction to "The DOM"](https://dom.spec.whatwg.org/#introduction-to-the-dom)

Status: unreviewed

### [4.2. Node tree](https://dom.spec.whatwg.org/#node-trees)

Status: unreviewed
<!-- definitions: concept-node concept-node-tree concept-node-empty -->

- [ ] [length (Node)](https://dom.spec.whatwg.org/#concept-node-length) — To determine the length of a node node, run these steps:
  - 1. If node is a DocumentType or Attr node, then return 0.
  - 2. If node is a CharacterData node, then return node’s data’s length.
  - 3. Return the number of node’s children.

#### [4.2.1. Document tree](https://dom.spec.whatwg.org/#document-trees)

Status: unreviewed
<!-- definitions: concept-document-tree document-element in-a-document-tree in-a-document -->

#### [4.2.2. Shadow tree](https://dom.spec.whatwg.org/#shadow-trees)

Status: unreviewed
<!-- definitions: concept-shadow-tree concept-light-tree connected -->

##### [4.2.2.1. Slots](https://dom.spec.whatwg.org/#shadow-tree-slots)

Status: unreviewed
<!-- definitions: concept-slot slot-assigned-nodes -->

- [ ] [Use these attribute change steps to update a slot’s name](https://dom.spec.whatwg.org/#slot-name) — Use these attribute change steps to update a slot’s name:
  - 1. If element is a slot, localName is name, and namespace is null:
    - 1a. If value is oldValue, then return.
    - 1b. If value is null and oldValue is the empty string, then return.
    - 1c. If value is the empty string and oldValue is null, then return.
    - 1d. If value is null or the empty string, then set element’s name to the empty string.
    - 1e. Otherwise, set element’s name to value.
    - 1f. Run assign slottables for a tree with element’s root.

##### [4.2.2.2. Slottables](https://dom.spec.whatwg.org/#light-tree-slotables)

Status: unreviewed
<!-- definitions: concept-slotable slotable-assigned-slot slotable-assigned slottable-manual-slot-assignment -->

- [ ] [Use these attribute change steps to update a slottable’s name](https://dom.spec.whatwg.org/#slotable-name) — Use these attribute change steps to update a slottable’s name:
  - 1. If localName is slot and namespace is null:
    - 1a. If value is oldValue, then return.
    - 1b. If value is null and oldValue is the empty string, then return.
    - 1c. If value is the empty string and oldValue is null, then return.
    - 1d. If value is null or the empty string, then set element’s name to the empty string.
    - 1e. Otherwise, set element’s name to value.
    - 1f. If element is assigned, then run assign slottables for element’s assigned slot.
    - 1g. Run assign a slot for element.

##### [4.2.2.3. Finding slots and slottables](https://dom.spec.whatwg.org/#finding-slots-and-slotables)

Status: unreviewed

- [ ] [find a slot](https://dom.spec.whatwg.org/#find-a-slot) — To find a slot for a given slottable slottable and an optional boolean open (default false):
  - 1. If slottable’s parent is null, then return null.
  - 2. Let shadow be slottable’s parent’s shadow root.
  - 3. If shadow is null, then return null.
  - 4. If open is true and shadow’s mode is not "open", then return null.
  - 5. If shadow’s slot assignment is "manual", then return the slot in shadow’s descendants whose manually assigned nodes contains slottable, if any; otherwise null.
  - 6. Return the first slot in tree order in shadow’s descendants whose name is slottable’s name, if any; otherwise null.

- [ ] [find slottables](https://dom.spec.whatwg.org/#find-slotables) — To find slottables for a given slot slot:
  - 1. Let result be « ».
  - 2. Let root be slot’s root.
  - 3. If root is not a shadow root, then return result.
  - 4. Let host be root’s host.
  - 5. If root’s slot assignment is "manual":
    - 5a. For each slottable slottable of slot’s manually assigned nodes, if slottable’s parent is host, append slottable to result.
  - 6. Otherwise, for each slottable child slottable of host, in tree order:
    - 6a. Let foundSlot be the result of finding a slot given slottable.
    - 6b. If foundSlot is slot, then append slottable to result.
  - 7. Return result.

- [ ] [find flattened slottables](https://dom.spec.whatwg.org/#find-flattened-slotables) — To find flattened slottables for a given slot slot:
  - 1. Let result be « ».
  - 2. If slot’s root is not a shadow root, then return result.
  - 3. Let slottables be the result of finding slottables given slot.
  - 4. If slottables is the empty list, then append each slottable child of slot, in tree order, to slottables.
  - 5. For each node of slottables:
    - 5a. If node is a slot whose root is a shadow root:
      - 5a.i. Let temporaryResult be the result of finding flattened slottables given node.
      - 5a.ii. Append each slottable in temporaryResult, in order, to result.
    - 5b. Otherwise, append node to result.
  - 6. Return result.

##### [4.2.2.4. Assigning slottables and slots](https://dom.spec.whatwg.org/#assigning-slotables-and-slots)

Status: unreviewed

- [ ] [assign slottables](https://dom.spec.whatwg.org/#assign-slotables) — To assign slottables for a slot slot:
  - 1. Let slottables be the result of finding slottables for slot.
  - 2. If slottables and slot’s assigned nodes are not identical, then run signal a slot change for slot.
  - 3. Set slot’s assigned nodes to slottables.
  - 4. For each slottable of slottables: set slottable’s assigned slot to slot.

- [ ] [assign slottables for a tree](https://dom.spec.whatwg.org/#assign-slotables-for-a-tree) — To assign slottables for a tree, given a node root, run assign slottables for each slot of root’s inclusive descendants, in tree order.

- [ ] [assign a slot](https://dom.spec.whatwg.org/#assign-a-slot) — To assign a slot, given a slottable slottable:
  - 1. Let slot be the result of finding a slot with slottable.
  - 2. If slot is non-null, then run assign slottables for slot.

##### [4.2.2.5. Signaling slot change](https://dom.spec.whatwg.org/#signaling-slot-change)

Status: unreviewed
<!-- definitions: signal-slot-list -->

- [ ] [signal a slot change](https://dom.spec.whatwg.org/#signal-a-slot-change) — To signal a slot change, for a slot slot:
  - 1. Append slot to slot’s relevant agent’s signal slots.
  - 2. Queue a mutation observer microtask.

#### [4.2.3. Mutation algorithms](https://dom.spec.whatwg.org/#mutation-algorithms)

Status: unreviewed
<!-- definitions: insert-suppressobservers remove-suppressobservers -->

- [ ] [ensure pre-insert validity](https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity) — To ensure pre-insert validity given a node node, a node parent, null or a node child, and a list of nodes childrenToExclude:
  - 1. If parent is not a Document, DocumentFragment, or Element node, then throw a "HierarchyRequestError" DOMException.
  - 2. If node is a host-including inclusive ancestor of parent, then throw a "HierarchyRequestError" DOMException.
  - 3. If child is non-null and its parent is not parent, then throw a "NotFoundError" DOMException.
  - 4. If node is not a DocumentFragment, DocumentType, Element, or CharacterData node, then throw a "HierarchyRequestError" DOMException.
  - 5. If parent is not a document:
    - 5a. If node is a doctype, then throw a "HierarchyRequestError" DOMException.
    - 5b. Return.
  - 6. If node is a Text node, then throw a "HierarchyRequestError" DOMException.
  - 7. If node is a CharacterData node, then return.
  - 8. If node is a DocumentFragment node:
    - 8a. If node has more than one element child or has a Text node child, then throw a "HierarchyRequestError" DOMException.
    - 8b. If node has no element child, then return.
  - 9. If node is a DocumentFragment or Element node:
    - 9a. If any of the following are true: parent has an element child that childrenToExclude does not contain;; child is non-null and a doctype is following child; or; child is a doctype that childrenToExclude does not contain, then throw a "HierarchyRequestError" DOMException.
    - 9b. Return.
  - 10. Assert: node is a doctype.
  - 11. If any of the following are true: parent has a doctype child that childrenToExclude does not contain;; child is non-null and an element is preceding child; or; child is null and parent has an element child that childrenToExclude does not contain, then throw a "HierarchyRequestError" DOMException.

- [ ] [pre-insert](https://dom.spec.whatwg.org/#concept-node-pre-insert) — To pre-insert a node node into a node parent before null or node child:
  - 1. Ensure pre-insert validity given node, parent, child, and « ».
  - 2. Let referenceChild be child.
  - 3. If referenceChild is node, then set referenceChild to node’s next sibling.
  - 4. Insert node into parent before referenceChild.
  - 5. Return node.

- [ ] [insertion steps](https://dom.spec.whatwg.org/#concept-node-insert-ext) — Specifications may define insertion steps for all or some nodes. The algorithm is passed insertedNode, as indicated in the insert algorithm below. These steps must not modify the node tree that insertedNode participates in, create browsing contexts, fire events, or otherwise execute JavaScript. These steps may queue tasks to do these things asynchronously, however.

- [ ] [post-connection steps](https://dom.spec.whatwg.org/#concept-node-post-connection-ext) — Specifications may also define post-connection steps for all or some nodes. The algorithm is passed connectedNode, as indicated in the insert algorithm below.

- [ ] [children changed steps](https://dom.spec.whatwg.org/#concept-node-children-changed-ext) — Specifications may define children changed steps for all or some nodes. The algorithm is passed no argument and is called from insert, remove, and replace data.

- [ ] [insert](https://dom.spec.whatwg.org/#concept-node-insert) — To insert a node node into a node parent before null or a node child, with an optional boolean suppressObservers (default false):
  - 1. Let nodes be node’s children, if node is a DocumentFragment node; otherwise « node ».
  - 2. Let count be nodes’s size.
  - 3. If count is 0, then return.
  - 4. If node is a DocumentFragment node:
    - 4a. Remove its children with suppressObservers set to true.
    - 4b. Queue a tree mutation record for node with « », nodes, null, and null.
  - 5. If child is non-null:
    - 5a. For each live range whose start node is parent and start offset is greater than child’s index: increase its start offset by count.
    - 5b. For each live range whose end node is parent and end offset is greater than child’s index: increase its end offset by count.
  - 6. Let previousSibling be child’s previous sibling or parent’s last child if child is null.
  - 7. For each node in nodes, in tree order:
    - 7a. Adopt node into parent’s node document.
    - 7b. If child is null, then append node to parent’s children.
    - 7c. Otherwise, insert node into parent’s children before child’s index.
    - 7d. If parent is a shadow host whose shadow root’s slot assignment is "named" and node is a slottable, then assign a slot for node.
    - 7e. If parent’s root is a shadow root, and parent is a slot whose assigned nodes is the empty list, then run signal a slot change for parent.
    - 7f. Run assign slottables for a tree with node’s root.
    - 7g. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including tree order:
      - 7g.i. Run the insertion steps with inclusiveDescendant.
      - 7g.ii. If inclusiveDescendant is not connected, then continue.
      - 7g.iii. If inclusiveDescendant is an element and inclusiveDescendant’s custom element registry is non-null:
        - 7g.iii.A. If inclusiveDescendant’s custom element registry’s is scoped is true, then append inclusiveDescendant’s node document to inclusiveDescendant’s custom element registry’s scoped document set.
        - 7g.iii.B. If inclusiveDescendant is custom, then enqueue a custom element callback reaction with inclusiveDescendant, callback name "connectedCallback", and « ».
        - 7g.iii.C. Otherwise, try to upgrade inclusiveDescendant.
      - 7g.iv. Otherwise, if inclusiveDescendant is a shadow root, inclusiveDescendant’s custom element registry is non-null, and inclusiveDescendant’s custom element registry’s is scoped is true, then append inclusiveDescendant’s node document to inclusiveDescendant’s custom element registry’s scoped document set.
  - 8. If suppressObservers is false, then queue a tree mutation record for parent with nodes, « », previousSibling, and child.
  - 9. Run the children changed steps for parent.
  - 10. Let staticNodeList be a list of nodes, initially « ».
  - 11. For each node of nodes, in tree order:
    - 11a. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including tree order: append inclusiveDescendant to staticNodeList.
  - 12. For each node of staticNodeList: if node is connected, then run the post-connection steps with node.

- [ ] [append](https://dom.spec.whatwg.org/#concept-node-append) — To append a node node to a node parent: pre-insert node into parent before null.

- [ ] [moving steps](https://dom.spec.whatwg.org/#concept-node-move-ext) — Specifications may define moving steps for all or some nodes. The algorithm is passed a node movedNode, a boolean isSubtreeRoot, and a node oldAncestor as indicated in the move algorithm below. Like the insertion steps, these steps must not modify the node tree that movedNode participates in, create browsing contexts, fire events, or otherwise execute JavaScript. These steps may queue tasks to do these things asynchronously, however.

- [ ] [move](https://dom.spec.whatwg.org/#move) — To move a node node into a node newParent before null or a node child:
  - 1. If newParent’s shadow-including root is not the same as node’s shadow-including root, then throw a "HierarchyRequestError" DOMException.
  - 2. If node is a host-including inclusive ancestor of newParent, then throw a "HierarchyRequestError" DOMException.
  - 3. If child is non-null and its parent is not newParent, then throw a "NotFoundError" DOMException.
  - 4. If node is not an Element or a CharacterData node, then throw a "HierarchyRequestError" DOMException.
  - 5. If node is a Text node and newParent is a document, then throw a "HierarchyRequestError" DOMException.
  - 6. If newParent is a document, node is an Element node, and either newParent has an element child, child is a doctype, or child is non-null and a doctype is following child then throw a "HierarchyRequestError" DOMException.
  - 7. Let oldParent be node’s parent.
  - 8. Assert: oldParent is non-null.
  - 9. Run the live range pre-remove steps, given node.
  - 10. For each NodeIterator object iterator whose root’s node document is node’s node document: run the NodeIterator pre-remove steps given node and iterator.
  - 11. Let oldPreviousSibling be node’s previous sibling.
  - 12. Let oldNextSibling be node’s next sibling.
  - 13. Remove node from oldParent’s children.
  - 14. If node is assigned, then run assign slottables for node’s assigned slot.
  - 15. If oldParent’s root is a shadow root, and oldParent is a slot whose assigned nodes is empty, then run signal a slot change for oldParent.
  - 16. If node has an inclusive descendant that is a slot:
    - 16a. Run assign slottables for a tree with oldParent’s root.
    - 16b. Run assign slottables for a tree with node.
  - 17. Add transient registered observers given node and oldParent.
  - 18. If child is non-null:
    - 18a. For each live range whose start node is newParent and start offset is greater than child’s index: increase its start offset by 1.
    - 18b. For each live range whose end node is newParent and end offset is greater than child’s index: increase its end offset by 1.
  - 19. Let newPreviousSibling be child’s previous sibling if child is non-null, and newParent’s last child otherwise.
  - 20. If child is null, then append node to newParent’s children.
  - 21. Otherwise, insert node into newParent’s children before child’s index.
  - 22. If newParent is a shadow host whose shadow root’s slot assignment is "named" and node is a slottable, then assign a slot for node.
  - 23. If newParent’s root is a shadow root, and newParent is a slot whose assigned nodes is empty, then run signal a slot change for newParent.
  - 24. Run assign slottables for a tree with node’s root.
  - 25. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including tree order:
    - 25a. Let isSubtreeRoot be true if inclusiveDescendant is node; otherwise false.
    - 25b. Run the moving steps with inclusiveDescendant, isSubtreeRoot, and oldParent.
    - 25c. If inclusiveDescendant is custom and newParent is connected, then enqueue a custom element callback reaction with inclusiveDescendant, callback name "connectedMoveCallback", and « ».
  - 26. Queue a tree mutation record for oldParent with « », « node », oldPreviousSibling, and oldNextSibling.
  - 27. Queue a tree mutation record for newParent with « node », « », newPreviousSibling, and child.

- [ ] [replace](https://dom.spec.whatwg.org/#concept-node-replace) — To replace a node child with a node node within a node parent:
  - 1. Ensure pre-insert validity given node, parent, child, and « child ».
  - 2. Let referenceChild be child’s next sibling.
  - 3. If referenceChild is node, then set referenceChild to node’s next sibling.
  - 4. Let previousSibling be child’s previous sibling.
  - 5. Let removedNodes be the empty set.
  - 6. Adopt node into parent’s node document.
  - 7. If child’s parent is non-null:
    - 7a. Set removedNodes to « child ».
    - 7b. Remove child with suppressObservers set to true.
  - 8. Let nodes be node’s children if node is a DocumentFragment node; otherwise « node ».
  - 9. Insert node into parent before referenceChild with suppressObservers set to true.
  - 10. Queue a tree mutation record for parent with nodes, removedNodes, previousSibling, and referenceChild.
  - 11. Return child.

- [ ] [replace all (Node)](https://dom.spec.whatwg.org/#concept-node-replace-all) — To replace all with a node or null node within a node parent:
  - 1. Let removedNodes be parent’s children.
  - 2. Let addedNodes be the empty set.
  - 3. If node is a DocumentFragment node, then set addedNodes to node’s children.
  - 4. Otherwise, if node is non-null, set addedNodes to « node ».
  - 5. Remove all parent’s children, in tree order, with suppressObservers set to true.
  - 6. If node is non-null, then insert node into parent before null with suppressObservers set to true.
  - 7. If either addedNodes or removedNodes is not empty, then queue a tree mutation record for parent with addedNodes, removedNodes, null, and null.

- [ ] [pre-remove](https://dom.spec.whatwg.org/#concept-node-pre-remove) — To pre-remove a node child from a node parent:
  - 1. If child’s parent is not parent, then throw a "NotFoundError" DOMException.
  - 2. Remove child.
  - 3. Return child.

- [ ] [removing steps](https://dom.spec.whatwg.org/#concept-node-remove-ext) — Specifications may define removing steps for all or some nodes. The algorithm is passed a node removedNode, a boolean isSubtreeRoot, and a node oldAncestor as indicated in the remove algorithm below.

- [ ] [remove](https://dom.spec.whatwg.org/#concept-node-remove) — To remove a node node, with an optional boolean suppressObservers (default false):
  - 1. Let parent be node’s parent.
  - 2. Assert: parent is non-null.
  - 3. Run the live range pre-remove steps, given node.
  - 4. For each NodeIterator object iterator whose root’s node document is node’s node document: run the NodeIterator pre-remove steps given node and iterator.
  - 5. Let oldPreviousSibling be node’s previous sibling.
  - 6. Let oldNextSibling be node’s next sibling.
  - 7. Remove node from its parent’s children.
  - 8. If node is assigned, then run assign slottables for node’s assigned slot.
  - 9. If parent’s root is a shadow root, and parent is a slot whose assigned nodes is the empty list, then run signal a slot change for parent.
  - 10. If node has an inclusive descendant that is a slot:
    - 10a. Run assign slottables for a tree with parent’s root.
    - 10b. Run assign slottables for a tree with node.
  - 11. Run the removing steps with node, true, and parent.
  - 12. Let isParentConnected be parent’s connected.
  - 13. If node is custom and isParentConnected is true, then enqueue a custom element callback reaction with node, callback name "disconnectedCallback", and « ».
  - 14. For each shadow-including descendant descendant of node, in shadow-including tree order:
    - 14a. Run the removing steps with descendant, false, and parent.
    - 14b. If descendant is custom and isParentConnected is true, then enqueue a custom element callback reaction with descendant, callback name "disconnectedCallback", and « ».
  - 15. Add transient registered observers given node and parent.
  - 16. If suppressObservers is false, then queue a tree mutation record for parent with « », « node », oldPreviousSibling, and oldNextSibling.
  - 17. Run the children changed steps for parent.

#### [4.2.4. Mixin NonElementParentNode](https://dom.spec.whatwg.org/#interface-nonelementparentnode)

Status: unreviewed

- [ ] [get an element by ID](https://dom.spec.whatwg.org/#get-an-element-by-id) — To get an element by ID, given a node node and a string elementId, return the first element, in tree order, within node’s descendants, whose ID is elementId; otherwise, if there is no such element, null.

- [ ] [NonElementParentNode.getElementById(elementId)](https://dom.spec.whatwg.org/#dom-nonelementparentnode-getelementbyid) — The getElementById(elementId) method steps are to return the result of getting an element by ID given this and elementId.

#### [4.2.5. Mixin DocumentOrShadowRoot](https://dom.spec.whatwg.org/#mixin-documentorshadowroot)

Status: unreviewed

- [ ] [DocumentOrShadowRoot.customElementRegistry](https://dom.spec.whatwg.org/#dom-documentorshadowroot-customelementregistry) — The customElementRegistry getter steps are:
  - 1. If this is a document, then return this’s custom element registry.
  - 2. Assert: this is a ShadowRoot node.
  - 3. Return this’s custom element registry.

#### [4.2.6. Mixin ParentNode](https://dom.spec.whatwg.org/#interface-parentnode)

Status: unreviewed

- [ ] [convert nodes into a node](https://dom.spec.whatwg.org/#convert-nodes-into-a-node) — To convert nodes into a node, given a list of nodes and strings nodes, and document document:
  - 1. Replace each string in nodes with the result of creating a text node given document and the string.
  - 2. If nodes’s size is 1, then return nodes[0].
  - 3. Let fragment be the result of creating a document fragment given document.
  - 4. For each node of nodes: append node to fragment.
  - 5. Return fragment.

- [ ] [ParentNode.children](https://dom.spec.whatwg.org/#dom-parentnode-children) — The children getter steps are to return an HTMLCollection collection rooted at this matching only element children.

- [ ] [ParentNode.firstElementChild](https://dom.spec.whatwg.org/#dom-parentnode-firstelementchild) — The firstElementChild getter steps are to return the first child that is an element; otherwise null.

- [ ] [ParentNode.lastElementChild](https://dom.spec.whatwg.org/#dom-parentnode-lastelementchild) — The lastElementChild getter steps are to return the last child that is an element; otherwise null.

- [ ] [ParentNode.childElementCount](https://dom.spec.whatwg.org/#dom-parentnode-childelementcount) — The childElementCount getter steps are to return the number of children of this that are elements.

- [ ] [ParentNode.prepend(nodes)](https://dom.spec.whatwg.org/#dom-parentnode-prepend) — The prepend(nodes) method steps are:
  - 1. Let node be the result of converting nodes into a node given nodes and this’s node document.
  - 2. Pre-insert node into this before this’s first child.

- [ ] [ParentNode.append(nodes)](https://dom.spec.whatwg.org/#dom-parentnode-append) — The append(nodes) method steps are:
  - 1. Let node be the result of converting nodes into a node given nodes and this’s node document.
  - 2. Append node to this.

- [ ] [ParentNode.replaceChildren(nodes)](https://dom.spec.whatwg.org/#dom-parentnode-replacechildren) — The replaceChildren(nodes) method steps are:
  - 1. Let node be the result of converting nodes into a node given nodes and this’s node document.
  - 2. Ensure pre-insert validity given node, this, null, and this’s children.
  - 3. Replace all with node within this.

- [ ] [ParentNode.moveBefore(node, child)](https://dom.spec.whatwg.org/#dom-parentnode-movebefore) — The moveBefore(node, child) method steps are:
  - 1. Let referenceChild be child.
  - 2. If referenceChild is node, then set referenceChild to node’s next sibling.
  - 3. Move node into this before referenceChild.

- [ ] [ParentNode.querySelector(selectors)](https://dom.spec.whatwg.org/#dom-parentnode-queryselector) — The querySelector(selectors) method steps are to return the first result of running scope-match a selectors string selectors against this, if the result is not an empty list; otherwise null.

- [ ] [ParentNode.querySelectorAll(selectors)](https://dom.spec.whatwg.org/#dom-parentnode-queryselectorall) — The querySelectorAll(selectors) method steps are to return the static result of running scope-match a selectors string selectors against this.

#### [4.2.7. Mixin NonDocumentTypeChildNode](https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode)

Status: unreviewed

- [ ] [NonDocumentTypeChildNode.previousElementSibling](https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling) — The previousElementSibling getter steps are to return the first preceding sibling that is an element; otherwise null.

- [ ] [NonDocumentTypeChildNode.nextElementSibling](https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling) — The nextElementSibling getter steps are to return the first following sibling that is an element; otherwise null.

#### [4.2.8. Mixin ChildNode](https://dom.spec.whatwg.org/#interface-childnode)

Status: unreviewed

- [ ] [ChildNode.before(nodes)](https://dom.spec.whatwg.org/#dom-childnode-before) — The before(nodes) method steps are:
  - 1. Let parent be this’s parent.
  - 2. If parent is null, then return.
  - 3. Let viablePreviousSibling be this’s first preceding sibling not in nodes; otherwise null.
  - 4. Let node be the result of converting nodes into a node, given nodes and this’s node document.
  - 5. If viablePreviousSibling is null, then set it to parent’s first child; otherwise to viablePreviousSibling’s next sibling.
  - 6. Pre-insert node into parent before viablePreviousSibling.

- [ ] [ChildNode.after(nodes)](https://dom.spec.whatwg.org/#dom-childnode-after) — The after(nodes) method steps are:
  - 1. Let parent be this’s parent.
  - 2. If parent is null, then return.
  - 3. Let viableNextSibling be this’s first following sibling not in nodes; otherwise null.
  - 4. Let node be the result of converting nodes into a node, given nodes and this’s node document.
  - 5. Pre-insert node into parent before viableNextSibling.

- [ ] [ChildNode.replaceWith(nodes)](https://dom.spec.whatwg.org/#dom-childnode-replacewith) — The replaceWith(nodes) method steps are:
  - 1. Let parent be this’s parent.
  - 2. If parent is null, then return.
  - 3. Let viableNextSibling be this’s first following sibling not in nodes; otherwise null.
  - 4. Let node be the result of converting nodes into a node, given nodes and this’s node document.
  - 5. If this’s parent is parent, replace this with node within parent.
  - 6. Otherwise, pre-insert node into parent before viableNextSibling.

- [ ] [ChildNode.remove()](https://dom.spec.whatwg.org/#dom-childnode-remove) — The remove() method steps are:
  - 1. If this’s parent is null, then return.
  - 2. Remove this.

#### [4.2.9. Mixin Slottable](https://dom.spec.whatwg.org/#mixin-slotable)

Status: unreviewed

- [ ] [Slottable.assignedSlot](https://dom.spec.whatwg.org/#dom-slotable-assignedslot) — The assignedSlot getter steps are to return the result of find a slot given this and true.

#### [4.2.10. Old-style collections: NodeList and HTMLCollection](https://dom.spec.whatwg.org/#old-style-collections)

Status: unreviewed
<!-- definitions: concept-collection concept-collection-live concept-collection-static represented-by-the-collection -->

##### [4.2.10.1. Interface NodeList](https://dom.spec.whatwg.org/#interface-nodelist)

Status: unreviewed

- [ ] [NodeList.length](https://dom.spec.whatwg.org/#dom-nodelist-length) — The length attribute must return the number of nodes represented by the collection.

- [ ] [NodeList.item(index)](https://dom.spec.whatwg.org/#dom-nodelist-item) — The item(index) method must return the indexth node in the collection. If there is no indexth node in the collection, then the method must return null.

##### [4.2.10.2. Interface HTMLCollection](https://dom.spec.whatwg.org/#interface-htmlcollection)

Status: unreviewed

- [ ] [HTMLCollection.length](https://dom.spec.whatwg.org/#dom-htmlcollection-length) — The length getter steps are to return the number of nodes represented by the collection.

- [ ] [HTMLCollection.item(index)](https://dom.spec.whatwg.org/#dom-htmlcollection-item) — The item(index) method steps are to return the indexth element in the collection. If there is no indexth element in the collection, then the method must return null.

- [ ] [The supported property names are the values from the list returned by these ste…](https://dom.spec.whatwg.org/#dom-htmlcollection-item) — The supported property names are the values from the list returned by these steps:
  - 1. Let result be an empty list.
  - 2. For each element represented by the collection, in tree order:
    - 2a. If element has an ID which is not in result, append element’s ID to result.
    - 2b. If element is in the HTML namespace and has a name attribute whose value is neither the empty string nor is in result, append element’s name attribute value to result.
  - 3. Return result.

- [ ] [HTMLCollection.namedItem(key)](https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key) — The namedItem(key) method steps are:
  - 1. If key is the empty string, return null.
  - 2. Return the first element in the collection for which at least one of the following is true: it has an ID which is key;; it is in the HTML namespace and has a name attribute whose value is key; or null if there is no such element.

### [4.3. Mutation observers](https://dom.spec.whatwg.org/#mutation-observers)

Status: unreviewed
<!-- definitions: mutation-observer-compound-microtask-queued-flag mutation-observer-list mutation-observer-counter eventdef-htmlslotelement-slotchange registered-observer-list registered-observer registered-observer-observer registered-observer-options transient-registered-observer transient-registered-observer-source -->

- [ ] [queue a mutation observer microtask](https://dom.spec.whatwg.org/#queue-a-mutation-observer-compound-microtask) — To queue a mutation observer microtask:
  - 1. If the surrounding agent’s mutation observer microtask queued is true, then return.
  - 2. Set the surrounding agent’s mutation observer microtask queued to true.
  - 3. Queue a microtask to notify mutation observers.

- [ ] [notify mutation observers](https://dom.spec.whatwg.org/#notify-mutation-observers) — To notify mutation observers:
  - 1. Set the surrounding agent’s mutation observer microtask queued to false.
  - 2. Let notifyList be the result of sorting the surrounding agent’s pending mutation observers in ascending order, with a being less than b if a’s creation index is less than b’s creation index.
  - 3. Empty the surrounding agent’s pending mutation observers.
  - 4. Let signalSet be a clone of the surrounding agent’s signal slots.
  - 5. Empty the surrounding agent’s signal slots.
  - 6. For each mo of notifyList:
    - 6a. Let records be a clone of mo’s record queue.
    - 6b. Empty mo’s record queue.
    - 6c. Remove transient registered observers for mo.
    - 6d. If records is not empty, then invoke mo’s callback with « records, mo » and "report", and with callback this value mo.
  - 7. For each slot of signalSet: fire an event named slotchange, with its bubbles attribute set to true, at slot.

- [ ] [add transient registered observers](https://dom.spec.whatwg.org/#add-transient-registered-observers) — To add transient registered observers given a node node and a node parent:
  - 1. For each inclusive ancestor inclusiveAncestor of parent, and then for each registered of inclusiveAncestor’s registered observer list:
    - 1a. If registered’s options["subtree"] is false, then continue.
    - 1b. Let observer be registered’s observer.
    - 1c. Let source be registered’s source if registered is a transient registered observer; otherwise registered.
    - 1d. If node’s registered observer list contains a transient registered observer whose source is source, then continue.
    - 1e. Append a new transient registered observer whose observer is observer, options is source’s options, and source is source to node’s registered observer list.
    - 1f. Append a weak reference to node to observer’s transient node set.
    - 1g. Append observer to the surrounding agent’s pending mutation observers.
    - 1h. Queue a mutation observer microtask.

- [ ] [remove transient registered observers](https://dom.spec.whatwg.org/#remove-transient-registered-observers) — To remove transient registered observers for a MutationObserver object observer:
  - 1. For each node of observer’s transient node set: remove all transient registered observers whose observer is observer from node’s registered observer list.
  - 2. Empty observer’s transient node set.

#### [4.3.1. Interface MutationObserver](https://dom.spec.whatwg.org/#interface-mutationobserver)

Status: unreviewed
<!-- definitions: concept-mo-callback mutationobserver-creation-index mutationobserver-node-list mutationobserver-transient-node-set concept-mo-queue -->

- [ ] [MutationObserver.new MutationObserver(callback)](https://dom.spec.whatwg.org/#dom-mutationobserver-mutationobserver) — The new MutationObserver(callback) constructor steps are:
  - 1. Set this’s callback to callback.
  - 2. Set this’s creation index to this’s relevant agent’s mutation observer counter.
  - 3. Increase this’s relevant agent’s mutation observer counter by 1.

- [ ] [MutationObserver.observe(target, options)](https://dom.spec.whatwg.org/#dom-mutationobserver-observe) — The observe(target, options) method steps are:
  - 1. If either options["attributeOldValue"] or options["attributeFilter"] exists, and options["attributes"] does not exist, then set options["attributes"] to true.
  - 2. If options["characterDataOldValue"] exists and options["characterData"] does not exist, then set options["characterData"] to true.
  - 3. If none of options["childList"], options["attributes"], and options["characterData"] is true, then throw a TypeError.
  - 4. If options["attributeOldValue"] is true and options["attributes"] is false, then throw a TypeError.
  - 5. If options["attributeFilter"] exists and options["attributes"] is false, then throw a TypeError.
  - 6. If options["characterDataOldValue"] is true and options["characterData"] is false, then throw a TypeError.
  - 7. Let registered be the registered observer in target’s registered observer list that is not a transient registered observer and whose observer is this, if any; otherwise null.
  - 8. If registered is non-null:
    - 8a. For each node of this’s transient node set: remove all transient registered observers whose source is registered from node’s registered observer list.
    - 8b. Set registered’s options to options.
  - 9. Otherwise:
    - 9a. Append a new registered observer whose observer is this and options is options to target’s registered observer list.
    - 9b. Append a weak reference to target to this’s node list.

- [ ] [MutationObserver.disconnect()](https://dom.spec.whatwg.org/#dom-mutationobserver-disconnect) — The disconnect() method steps are:
  - 1. For each node of this’s node list: remove any registered observer from node’s registered observer list for which this is the observer.
  - 2. Remove transient registered observers for this.
  - 3. Empty this’s node list.
  - 4. Empty this’s record queue.

- [ ] [MutationObserver.takeRecords()](https://dom.spec.whatwg.org/#dom-mutationobserver-takerecords) — The takeRecords() method steps are:
  - 1. Let records be a clone of this’s record queue.
  - 2. Empty this’s record queue.
  - 3. Return records.

#### [4.3.2. Queuing a mutation record](https://dom.spec.whatwg.org/#queueing-a-mutation-record)

Status: unreviewed

- [ ] [queue a mutation record](https://dom.spec.whatwg.org/#queue-a-mutation-record) — To queue a mutation record of type for target with name, namespace, oldValue, addedNodes, removedNodes, previousSibling, and nextSibling:
  - 1. Let interestedObservers be an empty map.
  - 2. Let nodes be the inclusive ancestors of target.
  - 3. For each node of nodes, and then for each registered of node’s registered observer list:
    - 3a. Let options be registered’s options.
    - 3b. If none of the following are true node is not target and options["subtree"] is false; type is "attributes" and options["attributes"] either does not exist or is false; type is "attributes", options["attributeFilter"] exists, and options["attributeFilter"] does not contain name or namespace is non-null; type is "characterData" and options["characterData"] either does not exist or is false; type is…
      - 3b.i. Let mo be registered’s observer.
      - 3b.ii. If interestedObservers[mo] does not exist, then set interestedObservers[mo] to null.
      - 3b.iii. If either type is "attributes" and options["attributeOldValue"] is true, or type is "characterData" and options["characterDataOldValue"] is true, then set interestedObservers[mo] to oldValue.
  - 4. If interestedObservers is empty, then return.
  - 5. For each observer → mappedOldValue of interestedObservers:
    - 5a. Let record be a new MutationRecord object with its type set to type, target set to target, attributeName set to name, attributeNamespace set to namespace, oldValue set to mappedOldValue, addedNodes set to addedNodes, removedNodes set to removedNodes, previousSibling set to previousSibling, and nextSibling set to nextSibling.
    - 5b. Enqueue record to observer’s record queue.
    - 5c. Append observer to the surrounding agent’s pending mutation observers.
  - 6. Queue a mutation observer microtask.

- [ ] [queue a tree mutation record](https://dom.spec.whatwg.org/#queue-a-tree-mutation-record) — To queue a tree mutation record for target with addedNodes, removedNodes, previousSibling, and nextSibling:
  - 1. Assert: either addedNodes or removedNodes is not empty.
  - 2. Queue a mutation record of "childList" for target with null, null, null, addedNodes, removedNodes, previousSibling, and nextSibling.

#### [4.3.3. Interface MutationRecord](https://dom.spec.whatwg.org/#interface-mutationrecord)

Status: unreviewed
<!-- definitions: dom-mutationrecord-target dom-mutationrecord-addednodes dom-mutationrecord-removednodes dom-mutationrecord-previoussibling dom-mutationrecord-nextsibling dom-mutationrecord-attributename dom-mutationrecord-attributenamespace dom-mutationrecord-oldvalue -->

- [ ] [MutationRecord.type](https://dom.spec.whatwg.org/#dom-mutationrecord-type) — The type, target, addedNodes, removedNodes, previousSibling, nextSibling, attributeName, attributeNamespace, and oldValue attributes must return the values they were initialized to.

### [4.4. Interface Node](https://dom.spec.whatwg.org/#interface-node)

Status: unreviewed
<!-- definitions: concept-node-document dom-node-element_node dom-node-attribute_node dom-node-text_node dom-node-cdata_section_node dom-node-processing_instruction_node dom-node-comment_node dom-node-document_node dom-node-document_type_node dom-node-document_fragment_node concept-node-clone-ext clone-a-node-document clone-a-node-subtree clone-a-node-parent clone-a-node-fallbackregistry dom-node-document_position_disconnected dom-node-document_position_preceding dom-node-document_position_following dom-node-document_position_contains dom-node-document_position_contained_by dom-node-document_position_implementation_specific -->

- [ ] [create a node](https://dom.spec.whatwg.org/#create-a-node) — To create a node that implements an interface interface, given a document document:
  - 1. Assert: interface is neither Document nor an interface that inherits from Document.
  - 2. Return a new node that implements interface, in document’s relevant realm, with its node document set to document.

- [ ] [Node.nodeType](https://dom.spec.whatwg.org/#dom-node-nodetype) — The nodeType getter steps are to return the first matching statement, switching on the interface this implements:
  - • Element: ELEMENT_NODE (1)
  - • Attr: ATTRIBUTE_NODE (2);
  - • An exclusive Text node: TEXT_NODE (3);
  - • CDATASection: CDATA_SECTION_NODE (4);
  - • ProcessingInstruction: PROCESSING_INSTRUCTION_NODE (7);
  - • Comment: COMMENT_NODE (8);
  - • Document: DOCUMENT_NODE (9);
  - • DocumentType: DOCUMENT_TYPE_NODE (10);
  - • DocumentFragment: DOCUMENT_FRAGMENT_NODE (11).

- [ ] [Node.nodeName](https://dom.spec.whatwg.org/#dom-node-nodename) — The nodeName getter steps are to return the first matching statement, switching on the interface this implements:
  - • Element: Its HTML-uppercased qualified name.
  - • Attr: Its qualified name.
  - • An exclusive Text node: "#text".
  - • CDATASection: "#cdata-section".
  - • ProcessingInstruction: Its target.
  - • Comment: "#comment".
  - • Document: "#document".
  - • DocumentType: Its name.
  - • DocumentFragment: "#document-fragment".

- [ ] [Node.baseURI](https://dom.spec.whatwg.org/#dom-node-baseuri) — The baseURI getter steps are:
  - 1. Let baseURL be the result of running this’s node document’s get the base URL algorithm.
  - 2. If baseURL is null, then set baseURL to this’s node document’s URL.
  - 3. Return baseURL, serialized.

- [ ] [Node.isConnected](https://dom.spec.whatwg.org/#dom-node-isconnected) — The isConnected getter steps are to return true, if this is connected; otherwise false.

- [ ] [Node.ownerDocument](https://dom.spec.whatwg.org/#dom-node-ownerdocument) — The ownerDocument getter steps are to return null, if this is a document; otherwise this’s node document.

- [ ] [Node.getRootNode(options)](https://dom.spec.whatwg.org/#dom-node-getrootnode) — The getRootNode(options) method steps are to return this’s shadow-including root if options["composed"] is true; otherwise this’s root.

- [ ] [Node.parentNode](https://dom.spec.whatwg.org/#dom-node-parentnode) — The parentNode getter steps are to return this’s parent.

- [ ] [Node.parentElement](https://dom.spec.whatwg.org/#dom-node-parentelement) — The parentElement getter steps are to return this’s parent element.

- [ ] [Node.hasChildNodes()](https://dom.spec.whatwg.org/#dom-node-haschildnodes) — The hasChildNodes() method steps are to return true if this has children; otherwise false.

- [ ] [Node.childNodes](https://dom.spec.whatwg.org/#dom-node-childnodes) — The childNodes getter steps are to return a NodeList rooted at this matching only children.

- [ ] [Node.firstChild](https://dom.spec.whatwg.org/#dom-node-firstchild) — The firstChild getter steps are to return this’s first child.

- [ ] [Node.lastChild](https://dom.spec.whatwg.org/#dom-node-lastchild) — The lastChild getter steps are to return this’s last child.

- [ ] [Node.previousSibling](https://dom.spec.whatwg.org/#dom-node-previoussibling) — The previousSibling getter steps are to return this’s previous sibling.

- [ ] [Node.nextSibling](https://dom.spec.whatwg.org/#dom-node-nextsibling) — The nextSibling getter steps are to return this’s next sibling.

- [ ] [Node.nodeValue](https://dom.spec.whatwg.org/#dom-node-nodevalue) — The nodeValue getter steps are to return the following, switching on the interface this implements:
  - • Attr: this’s value.
  - • CharacterData: this’s data.
  - • Otherwise: Null.

- [ ] [The nodeValue setter steps](https://dom.spec.whatwg.org/#dom-node-nodevalue) — The nodeValue setter steps are to, if the given value is null, act as if it was the empty string instead, and then do as described below, switching on the interface this implements:

- [ ] [get text content](https://dom.spec.whatwg.org/#get-text-content) — To get text content with a node node, return the following, switching on the interface node implements:
  - • DocumentFragment / Element: The descendant text content of node.
  - • Attr: node’s value.
  - • CharacterData: node’s data.
  - • Otherwise: Null.

- [ ] [Node.textContent](https://dom.spec.whatwg.org/#dom-node-textcontent) — The textContent getter steps are to return the result of running get text content with this.

- [ ] [string replace all](https://dom.spec.whatwg.org/#string-replace-all) — To string replace all with a string string within a node parent, run these steps:
  - 1. Let node be null.
  - 2. If string is not the empty string, then set node to the result of creating a text node given parent’s node document and string.
  - 3. Replace all with node within parent.

- [ ] [set text content](https://dom.spec.whatwg.org/#set-text-content) — To set text content with a node node and a string value, do as defined below, switching on the interface node implements:
  - • DocumentFragment / Element: String replace all with value within node.
  - • Attr: Set an existing attribute value with node and value.
  - • CharacterData: Replace data of node with 0, node’s length, and value.
  - • Otherwise: Do nothing.

- [ ] [The textContent setter steps](https://dom.spec.whatwg.org/#dom-node-textcontent) — The textContent setter steps are to, if the given value is null, act as if it was the empty string instead, and then run set text content with this and the given value.

- [ ] [Node.normalize()](https://dom.spec.whatwg.org/#dom-node-normalize) — The normalize() method steps are to run these steps for each descendant exclusive Text node node of this:
  - 1. Let length be node’s length.
  - 2. If length is zero, then remove node and continue with the next exclusive Text node, if any.
  - 3. Let data be the concatenation of the data of node’s contiguous exclusive Text nodes (excluding itself), in tree order.
  - 4. Replace data of node with length, 0, and data.
  - 5. Let currentNode be node’s next sibling.
  - 6. While currentNode is an exclusive Text node:
    - 6a. For each live range whose start node is currentNode: add length to its start offset and set its start node to node.
    - 6b. For each live range whose end node is currentNode: add length to its end offset and set its end node to node.
    - 6c. For each live range whose start node is currentNode’s parent and start offset is currentNode’s index: set its start node to node and its start offset to length.
    - 6d. For each live range whose end node is currentNode’s parent and end offset is currentNode’s index: set its end node to node and its end offset to length.
    - 6e. Add currentNode’s length to length.
    - 6f. Set currentNode to its next sibling.
  - 7. Remove node’s contiguous exclusive Text nodes (excluding itself), in tree order.

- [ ] [clone a node](https://dom.spec.whatwg.org/#concept-node-clone) — To clone a node given a node node and an optional document document (default node’s node document), boolean subtree (default false), node-or-null parent (default null), and null or a CustomElementRegistry object fallbackRegistry (default null):
  - 1. Assert: node is not a document or node is document.
  - 2. Let copy be the result of cloning a single node given node, document, and fallbackRegistry.
  - 3. Run any cloning steps defined for node in other applicable specifications and pass node, copy, and subtree as parameters.
  - 4. If parent is non-null, then append copy to parent.
  - 5. If subtree is true, then for each child of node’s children, in tree order: clone a node given child with document set to document, subtree set to subtree, parent set to copy, and fallbackRegistry set to fallbackRegistry.
  - 6. If node is an element, node is a shadow host, and node’s shadow root’s clonable is true:
    - 6a. Assert: copy is not a shadow host.
    - 6b. Let shadowRootRegistry be node’s shadow root’s custom element registry.
    - 6c. If shadowRootRegistry is a global custom element registry, then set shadowRootRegistry to document’s custom element registry’s effective global custom element registry.
    - 6d. Attach a shadow root with copy, node’s shadow root’s mode, node’s shadow root’s delegates focus, node’s shadow root’s serializable, node’s shadow root’s slot assignment, true, and shadowRootRegistry.
    - 6e. Set copy’s shadow root’s declarative to node’s shadow root’s declarative.
    - 6f. Set copy’s shadow root’s keep custom element registry null to node’s shadow root’s keep custom element registry null.
    - 6g. For each child of node’s shadow root’s children, in tree order: clone a node given child with document set to document, subtree set to true, and parent set to copy’s shadow root.
  - 7. Return copy.

- [ ] [clone a single node](https://dom.spec.whatwg.org/#clone-a-single-node) — To clone a single node given a node node, document document, and null or a CustomElementRegistry object fallbackRegistry:
  - 1. Let copy be null.
  - 2. If node is an element:
    - 2a. Let registry be node’s custom element registry.
    - 2b. If registry is null, then set registry to fallbackRegistry.
    - 2c. If registry is a global custom element registry, then set registry to document’s custom element registry’s effective global custom element registry.
    - 2d. Set copy to the result of creating an element, given document, node’s local name, node’s namespace, node’s namespace prefix, node’s is value, false, and registry.
    - 2e. For each attribute of node’s attribute list:
      - 2e.i. Let copyAttribute be the result of cloning a single node given attribute, document, and null.
      - 2e.ii. Append copyAttribute to copy.
  - 3. Otherwise, if node is a document, set copy to the result of creating a document that implements the same interfaces as node, given document’s relevant realm.
  - 4. Otherwise, set copy to the result of creating a node that implements the same interfaces as node, given document.
  - 5. Fulfill these additional requirements, switching on the interface node implements: Document DocumentType Set copy’s name, public ID, and system ID to those of node. Attr Set copy’s namespace, namespace prefix, local name, and value to those of node. Text Comment Set copy’s data to that of node. ProcessingInstruction Set copy’s target and data to those of node. Otherwise Do nothing.
    - 5a. Set copy’s encoding, content type, URL, origin, type, mode, and allow declarative shadow roots, to those of node.
    - 5b. Set copy’s base URL override to the result of running node’s get the base URL.
    - 5c. If node’s custom element registry’s is scoped is true, then set copy’s custom element registry to node’s custom element registry.
  - 6. Assert: copy is a node.
  - 7. If node is a document, then set document to copy.
  - 8. Set copy’s node document to document.
  - 9. Return copy.

- [ ] [Node.cloneNode(subtree)](https://dom.spec.whatwg.org/#dom-node-clonenode) — The cloneNode(subtree) method steps are:
  - 1. If this is a shadow root, then throw a "NotSupportedError" DOMException.
  - 2. Return the result of cloning a node given this with subtree set to subtree.

- [ ] [equals (Node)](https://dom.spec.whatwg.org/#concept-node-equals) — A node A equals a node B if all of the following conditions are true:

- [ ] [Node.isEqualNode(otherNode)](https://dom.spec.whatwg.org/#dom-node-isequalnode) — The isEqualNode(otherNode) method steps are to return true if otherNode is non-null and this equals otherNode; otherwise false.

- [ ] [Node.isSameNode(otherNode)](https://dom.spec.whatwg.org/#dom-node-issamenode) — The isSameNode(otherNode) method steps are to return true if otherNode is this; otherwise false.

- [ ] [Node.compareDocumentPosition(other)](https://dom.spec.whatwg.org/#dom-node-comparedocumentposition) — The compareDocumentPosition(other) method steps are:
  - 1. If this is other, then return zero.
  - 2. Let node1 be other and node2 be this.
  - 3. Let attr1 and attr2 be null.
  - 4. If node1 is an attribute, then set attr1 to node1 and node1 to attr1’s element.
  - 5. If node2 is an attribute:
    - 5a. Set attr2 to node2 and node2 to attr2’s element.
    - 5b. If attr1 and node1 are non-null, and node2 is node1:
      - 5b.i. For each attr of node2’s attribute list:
        - 5b.i.A. If attr equals attr1, then return the result of adding DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC and DOCUMENT_POSITION_PRECEDING.
        - 5b.i.B. If attr equals attr2, then return the result of adding DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC and DOCUMENT_POSITION_FOLLOWING.
  - 6. If node1 or node2 is null, or node1’s root is not node2’s root, then return the result of adding DOCUMENT_POSITION_DISCONNECTED, DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC, and either DOCUMENT_POSITION_PRECEDING or DOCUMENT_POSITION_FOLLOWING, with the constraint that this is to be consistent, together.
  - 7. If node1 is an ancestor of node2 and attr1 is null, or node1 is node2 and attr2 is non-null, then return the result of adding DOCUMENT_POSITION_CONTAINS to DOCUMENT_POSITION_PRECEDING.
  - 8. If node1 is a descendant of node2 and attr2 is null, or node1 is node2 and attr1 is non-null, then return the result of adding DOCUMENT_POSITION_CONTAINED_BY to DOCUMENT_POSITION_FOLLOWING.
  - 9. If node1 is preceding node2, then return DOCUMENT_POSITION_PRECEDING.
  - 10. Return DOCUMENT_POSITION_FOLLOWING.

- [ ] [Node.contains(other)](https://dom.spec.whatwg.org/#dom-node-contains) — The contains(other) method steps are to return true if other is an inclusive descendant of this; otherwise false (including when other is null).

- [ ] [locate a namespace prefix](https://dom.spec.whatwg.org/#locate-a-namespace-prefix) — To locate a namespace prefix for an element using namespace, run these steps:
  - 1. If element’s namespace is namespace and its namespace prefix is non-null, then return its namespace prefix.
  - 2. If element has an attribute whose namespace prefix is "xmlns" and value is namespace, then return element’s first such attribute’s local name.
  - 3. If element’s parent element is non-null, then return the result of running locate a namespace prefix on that element using namespace.
  - 4. Return null.

- [ ] [locate a namespace](https://dom.spec.whatwg.org/#locate-a-namespace) — To locate a namespace for a node using prefix, switch on the interface node implements:
  - • Element:
    - 1. If prefix is "xml", then return the XML namespace.
    - 2. If prefix is "xmlns", then return the XMLNS namespace.
    - 3. If its namespace is non-null and its namespace prefix is prefix, then return namespace.
    - 4. If it has an attribute whose namespace is the XMLNS namespace, namespace prefix is "xmlns", and local name is prefix, or if prefix is null and it has an attribute whose namespace is the XMLNS namespace, namespace prefix is null, and local name is "xmlns", then return its value if it is not the empty string, and null otherwise.
    - 5. If its parent element is null, then return null.
    - 6. Return the result of running locate a namespace on its parent element using prefix.
  - • Document:
    - 1. If its document element is null, then return null.
    - 2. Return the result of running locate a namespace on its document element using prefix.
  - • DocumentType / DocumentFragment: Return null.
  - • Attr:
    - 1. If its element is null, then return null.
    - 2. Return the result of running locate a namespace on its element using prefix.
  - • Otherwise:
    - 1. If its parent element is null, then return null.
    - 2. Return the result of running locate a namespace on its parent element using prefix.

- [ ] [Node.lookupPrefix(namespace)](https://dom.spec.whatwg.org/#dom-node-lookupprefix) — The lookupPrefix(namespace) method steps are:
  - 1. If namespace is null or the empty string, then return null.
  - 2. Switch on the interface this implements: Element Return the result of locating a namespace prefix for this using namespace. Document DocumentType DocumentFragment Return null. Attr Otherwise
    - 2a. If this’s document element is null, then return null.
    - 2b. Return the result of locating a namespace prefix for this’s document element using namespace.
    - 2a. If this’s element is null, then return null.
    - 2b. Return the result of locating a namespace prefix for this’s element using namespace.
    - 2a. If this’s parent element is null, then return null.
    - 2b. Return the result of locating a namespace prefix for this’s parent element using namespace.

- [ ] [Node.lookupNamespaceURI(prefix)](https://dom.spec.whatwg.org/#dom-node-lookupnamespaceuri) — The lookupNamespaceURI(prefix) method steps are:
  - 1. If prefix is the empty string, then set it to null.
  - 2. Return the result of running locate a namespace for this using prefix.

- [ ] [Node.isDefaultNamespace(namespace)](https://dom.spec.whatwg.org/#dom-node-isdefaultnamespace) — The isDefaultNamespace(namespace) method steps are:
  - 1. If namespace is the empty string, then set it to null.
  - 2. Let defaultNamespace be the result of running locate a namespace for this using null.
  - 3. Return true if defaultNamespace is the same as namespace; otherwise false.

- [ ] [Node.insertBefore(node, child)](https://dom.spec.whatwg.org/#dom-node-insertbefore) — The insertBefore(node, child) method steps are to return the result of pre-inserting node into this before child.

- [ ] [Node.appendChild(node)](https://dom.spec.whatwg.org/#dom-node-appendchild) — The appendChild(node) method steps are to return the result of appending node to this.

- [ ] [Node.replaceChild(node, child)](https://dom.spec.whatwg.org/#dom-node-replacechild) — The replaceChild(node, child) method steps are to return the result of replacing child with node within this.

- [ ] [Node.removeChild(child)](https://dom.spec.whatwg.org/#dom-node-removechild) — The removeChild(child) method steps are to return the result of pre-removing child from this.

- [ ] [list of elements with qualified name qualifiedName](https://dom.spec.whatwg.org/#concept-getelementsbytagname) — The list of elements with qualified name qualifiedName for a node root is the HTMLCollection returned by the following algorithm:
  - 1. If qualifiedName is U+002A (*), then return an HTMLCollection rooted at root, whose filter matches only descendant elements.
  - 2. Otherwise, if root’s node document is an HTML document, return an HTMLCollection rooted at root, whose filter matches the following descendant elements: Whose namespace is the HTML namespace and whose qualified name is qualifiedName, in ASCII lowercase.; Whose namespace is not the HTML namespace and whose qualified name is qualifiedName.
  - 3. Otherwise, return an HTMLCollection rooted at root, whose filter matches descendant elements whose qualified name is qualifiedName.

- [ ] [list of elements with namespace namespace and local name localName](https://dom.spec.whatwg.org/#concept-getelementsbytagnamens) — The list of elements with namespace namespace and local name localName for a node root is the HTMLCollection returned by the following algorithm:
  - 1. If namespace is the empty string, then set it to null.
  - 2. If both namespace and localName are U+002A (*), then return an HTMLCollection rooted at root, whose filter matches descendant elements.
  - 3. If namespace is U+002A (*), then return an HTMLCollection rooted at root, whose filter matches descendant elements whose local name is localName.
  - 4. If localName is U+002A (*), then return an HTMLCollection rooted at root, whose filter matches descendant elements whose namespace is namespace.
  - 5. Return an HTMLCollection rooted at root, whose filter matches descendant elements whose namespace is namespace and local name is localName.

- [ ] [list of elements with class names classNames](https://dom.spec.whatwg.org/#concept-getelementsbyclassname) — The list of elements with class names classNames for a node root is the HTMLCollection returned by the following algorithm:
  - 1. Let classes be the result of running the ordered set parser on classNames.
  - 2. If classes is the empty set, return an empty HTMLCollection.
  - 3. Return an HTMLCollection rooted at root, whose filter matches descendant elements that have all their classes in classes. The comparisons for the classes must be done in an ASCII case-insensitive manner if root’s node document’s mode is "quirks"; otherwise in an identical to manner.

### [4.5. Interface Document](https://dom.spec.whatwg.org/#interface-document)

Status: unreviewed
<!-- definitions: concept-document concept-document-encoding concept-document-content-type concept-document-url concept-document-origin concept-document-type concept-document-mode document-allow-declarative-shadow-roots document-custom-element-registry xml-document html-document get-the-base-url document-base-url-override concept-document-no-quirks concept-document-quirks concept-document-limited-quirks dom-document-documenturi dom-document-charset dom-document-inputencoding concept-element-interface is-a-global-custom-element-registry -->

- [ ] [create a document](https://dom.spec.whatwg.org/#create-a-document) — To create a document that implements an interface interface, given a realm realm:
  - 1. Assert: interface is Document or an interface that inherits from Document.
  - 2. Return a new node that implements interface, in realm.

- [ ] [Document.new Document()](https://dom.spec.whatwg.org/#dom-document-document) — The new Document() constructor steps are to set this’s origin to the origin of current global object’s associated Document. [HTML]

- [ ] [Document.implementation](https://dom.spec.whatwg.org/#dom-document-implementation) — The implementation getter steps are to return the DOMImplementation object that is associated with this.

- [ ] [Document.URL](https://dom.spec.whatwg.org/#dom-document-url) — The URL and documentURI getter steps are to return this’s URL, serialized.

- [ ] [Document.compatMode](https://dom.spec.whatwg.org/#dom-document-compatmode) — The compatMode getter steps are to return "BackCompat" if this’s mode is "quirks"; otherwise "CSS1Compat".

- [ ] [Document.characterSet](https://dom.spec.whatwg.org/#dom-document-characterset) — The characterSet, charset, and inputEncoding getter steps are to return this’s encoding’s name.

- [ ] [Document.contentType](https://dom.spec.whatwg.org/#dom-document-contenttype) — The contentType getter steps are to return this’s content type.

- [ ] [Document.doctype](https://dom.spec.whatwg.org/#dom-document-doctype) — The doctype getter steps are to return the child of this that is a doctype; otherwise null.

- [ ] [Document.documentElement](https://dom.spec.whatwg.org/#dom-document-documentelement) — The documentElement getter steps are to return this’s document element.

- [ ] [Document.getElementsByTagName(qualifiedName)](https://dom.spec.whatwg.org/#dom-document-getelementsbytagname) — The getElementsByTagName(qualifiedName) method steps are to return the list of elements with qualified name qualifiedName for this.

- [ ] [Document.getElementsByTagNameNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-document-getelementsbytagnamens) — The getElementsByTagNameNS(namespace, localName) method steps are to return the list of elements with namespace namespace and local name localName for this.

- [ ] [Document.getElementsByClassName(classNames)](https://dom.spec.whatwg.org/#dom-document-getelementsbyclassname) — The getElementsByClassName(classNames) method steps are to return the list of elements with class names classNames for this.

- [ ] [Document.createElement(localName, options)](https://dom.spec.whatwg.org/#dom-document-createelement) — The createElement(localName, options) method steps are:
  - 1. If localName is not a valid element local name, then throw an "InvalidCharacterError" DOMException.
  - 2. If this is an HTML document, then set localName to localName in ASCII lowercase.
  - 3. Let registry and is be the result of flattening element creation options given options and this.
  - 4. Let namespace be the HTML namespace, if this is an HTML document or this’s content type is "application/xhtml+xml"; otherwise null.
  - 5. Return the result of creating an element given this, localName, namespace, null, is, true, and registry.

- [ ] [internal createElementNS steps](https://dom.spec.whatwg.org/#internal-createelementns-steps) — The internal createElementNS steps, given document, namespace, qualifiedName, and options, are as follows:
  - 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and qualifiedName given "element".
  - 2. Let registry and is be the result of flattening element creation options given options and this.
  - 3. Return the result of creating an element given document, localName, namespace, prefix, is, true, and registry.

- [ ] [Document.createElementNS(namespace, qualifiedName, options)](https://dom.spec.whatwg.org/#dom-document-createelementns) — The createElementNS(namespace, qualifiedName, options) method steps are to return the result of running the internal createElementNS steps, given this, namespace, qualifiedName, and options.

- [ ] [flatten element creation options](https://dom.spec.whatwg.org/#flatten-element-creation-options) — To flatten element creation options, given a string or ElementCreationOptions dictionary options and a document document:
  - 1. Let registry be the result of looking up a custom element registry given document.
  - 2. Let is be null.
  - 3. If options is a dictionary:
    - 3a. If options["is"] exists, then set is to it.
    - 3b. If options["customElementRegistry"] exists:
      - 3b.i. If is is non-null, then throw a "NotSupportedError" DOMException.
      - 3b.ii. Set registry to options["customElementRegistry"].
    - 3c. If registry is non-null, registry’s is scoped is false, and registry is not document’s custom element registry, then throw a "NotSupportedError" DOMException.
  - 4. Return registry and is.

- [ ] [Document.createDocumentFragment()](https://dom.spec.whatwg.org/#dom-document-createdocumentfragment) — The createDocumentFragment() method steps are to return the result of creating a document fragment given this.

- [ ] [Document.createTextNode(data)](https://dom.spec.whatwg.org/#dom-document-createtextnode) — The createTextNode(data) method steps are to return the result of creating a text node given this and data.

- [ ] [Document.createCDATASection(data)](https://dom.spec.whatwg.org/#dom-document-createcdatasection) — The createCDATASection(data) method steps are:
  - 1. If this is an HTML document, then throw a "NotSupportedError" DOMException.
  - 2. If data contains the string "]]>", then throw an "InvalidCharacterError" DOMException.
  - 3. Return the result of creating a CDATA section node given this and data.

- [ ] [Document.createComment(data)](https://dom.spec.whatwg.org/#dom-document-createcomment) — The createComment(data) method steps are to return the result of creating a comment node given this and data.

- [ ] [Document.createProcessingInstruction(target, data)](https://dom.spec.whatwg.org/#dom-document-createprocessinginstruction) — The createProcessingInstruction(target, data) method steps are to return the result of creating a processing instruction node given this, target, and data.

- [ ] [Document.importNode(node, options)](https://dom.spec.whatwg.org/#dom-document-importnode) — The importNode(node, options) method steps are:
  - 1. If node is a document or shadow root, then throw a "NotSupportedError" DOMException.
  - 2. Let subtree be false.
  - 3. Let registry be null.
  - 4. If options is a boolean, then set subtree to options.
  - 5. Otherwise:
    - 5a. Set subtree to the negation of options["selfOnly"].
    - 5b. If options["customElementRegistry"] exists, then set registry to it.
    - 5c. If registry’s is scoped is false and registry is not this’s custom element registry, then throw a "NotSupportedError" DOMException.
  - 6. If registry is null, then set registry to the result of looking up a custom element registry given this.
  - 7. Return the result of cloning a node given node with document set to this, subtree set to subtree, and fallbackRegistry set to registry.

- [ ] [adopting steps](https://dom.spec.whatwg.org/#concept-node-adopt-ext) — Specifications may define adopting steps for all or some nodes. The algorithm is passed node and oldDocument, as indicated in the adopt algorithm.

- [ ] [adopt](https://dom.spec.whatwg.org/#concept-node-adopt) — To adopt a node node into a document document:
  - 1. Let oldDocument be node’s node document.
  - 2. If node’s parent is non-null, then remove node.
  - 3. If document is not oldDocument, then for each inclusiveDescendant of node’s shadow-including inclusive descendants, in shadow-including tree order:
    - 3a. Set inclusiveDescendant’s node document to document.
    - 3b. If inclusiveDescendant is a shadow root and if any of the following are true: inclusiveDescendant’s custom element registry is null and inclusiveDescendant’s keep custom element registry null is false; or; inclusiveDescendant’s custom element registry is a global custom element registry, then set inclusiveDescendant’s custom element registry to document’s custom element registry’s effective globa…
    - 3c. Otherwise, if inclusiveDescendant is an element:
      - 3c.i. Set the node document of each attribute in inclusiveDescendant’s attribute list to document.
      - 3c.ii. If inclusiveDescendant’s custom element registry is null or inclusiveDescendant’s custom element registry’s is scoped is false:
        - 3c.ii.A. Let registry be null.
        - 3c.ii.B. If inclusiveDescendant’s custom element registry is non-null, inclusiveDescendant’s parent is null, or inclusiveDescendant’s parent is an exclusive DocumentFragment node, then set registry to document’s custom element registry.
        - 3c.ii.C. Otherwise, set registry to the result of looking up a custom element registry given inclusiveDescendant’s parent.
        - 3c.ii.D. Set inclusiveDescendant’s custom element registry to registry’s effective global custom element registry.
      - 3c.iii. If inclusiveDescendant is custom, then enqueue a custom element callback reaction with inclusiveDescendant, callback name "adoptedCallback", and « oldDocument, document ».
    - 3d. Run the adopting steps with inclusiveDescendant and oldDocument.

- [ ] [Document.adoptNode(node)](https://dom.spec.whatwg.org/#dom-document-adoptnode) — The adoptNode(node) method steps are:
  - 1. If node is a document, then throw a "NotSupportedError" DOMException.
  - 2. If node is a shadow root, then throw a "HierarchyRequestError" DOMException.
  - 3. Adopt node into this.
  - 4. Return node.

- [ ] [effective global custom element registry](https://dom.spec.whatwg.org/#effective-global-custom-element-registry) — Null or a CustomElementRegistry object registry’s effective global custom element registry is:
  - 1. If registry is a global custom element registry, then return registry.
  - 2. Return null.

- [ ] [Document.createAttribute(localName)](https://dom.spec.whatwg.org/#dom-document-createattribute) — The createAttribute(localName) method steps are:
  - 1. If localName is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
  - 2. If this is an HTML document, then set localName to localName in ASCII lowercase.
  - 3. Return the result of creating an attribute given this and localName.

- [ ] [Document.createAttributeNS(namespace, qualifiedName)](https://dom.spec.whatwg.org/#dom-document-createattributens) — The createAttributeNS(namespace, qualifiedName) method steps are:
  - 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and qualifiedName given "attribute".
  - 2. Return the result of creating an attribute given this, localName, namespace, and prefix.

- [ ] [Document.createEvent(interface)](https://dom.spec.whatwg.org/#dom-document-createevent) — The createEvent(interface) method steps are:
  - 1. Let constructor be null.
  - 2. If interface is an ASCII case-insensitive match for any of the strings in the first column in the following table, then set constructor to the interface in the second column on the same row as the matching string:
  - 3. If constructor is null, then throw a "NotSupportedError" DOMException.
  - 4. If the interface indicated by constructor is not exposed on the relevant global object of this, then throw a "NotSupportedError" DOMException.
  - 5. Let event be the result of creating an event given constructor.
  - 6. Initialize event’s type attribute to the empty string.
  - 7. Initialize event’s timeStamp attribute to the result of calling current high resolution time with this’s relevant global object.
  - 8. Initialize event’s isTrusted attribute to false.
  - 9. Unset event’s initialized flag.
  - 10. Return event.

- [ ] [Document.createRange()](https://dom.spec.whatwg.org/#dom-document-createrange) — The createRange() method steps are to return a new live range with (this, 0) as its start an end.

- [ ] [Document.createNodeIterator(root, whatToShow, filter)](https://dom.spec.whatwg.org/#dom-document-createnodeiterator) — The createNodeIterator(root, whatToShow, filter) method steps are:
  - 1. Let iterator be a new NodeIterator object.
  - 2. Set iterator’s root to root.
  - 3. Set iterator’s reference to (root, true).
  - 4. Set iterator’s whatToShow to whatToShow.
  - 5. Set iterator’s filter to filter.
  - 6. Return iterator.

- [ ] [Document.createTreeWalker(root, whatToShow, filter)](https://dom.spec.whatwg.org/#dom-document-createtreewalker) — The createTreeWalker(root, whatToShow, filter) method steps are:
  - 1. Let walker be a new TreeWalker object.
  - 2. Set walker’s root and walker’s current to root.
  - 3. Set walker’s whatToShow to whatToShow.
  - 4. Set walker’s filter to filter.
  - 5. Return walker.

#### [4.5.1. Interface DOMImplementation](https://dom.spec.whatwg.org/#interface-domimplementation)

Status: unreviewed

- [ ] [DOMImplementation.createDocumentType(name, publicId, systemId)](https://dom.spec.whatwg.org/#dom-domimplementation-createdocumenttype) — The createDocumentType(name, publicId, systemId) method steps are:
  - 1. If name is not a valid doctype name, then throw an "InvalidCharacterError" DOMException.
  - 2. Return the result of creating a doctype given the associated document of this, name, publicId, and systemId.

- [ ] [DOMImplementation.createDocument(namespace, qualifiedName, doctype)](https://dom.spec.whatwg.org/#dom-domimplementation-createdocument) — The createDocument(namespace, qualifiedName, doctype) method steps are:
  - 1. Let document be the result of creating a document that implements XMLDocument, given this’s relevant realm.
  - 2. Let element be null.
  - 3. If qualifiedName is not the empty string, then set element to the result of running the internal createElementNS steps, given document, namespace, qualifiedName, and an empty dictionary.
  - 4. If doctype is non-null, append doctype to document.
  - 5. If element is non-null, append element to document.
  - 6. document’s origin is this’s associated document’s origin.
  - 7. document’s content type is determined by namespace: HTML namespace application/xhtml+xml SVG namespace image/svg+xml Any other namespace application/xml
  - 8. Return document.

- [ ] [DOMImplementation.createHTMLDocument(title)](https://dom.spec.whatwg.org/#dom-domimplementation-createhtmldocument) — The createHTMLDocument(title) method steps are:
  - 1. Let doc be the result of creating a document that implements Document, given this’s relevant realm.
  - 2. Set doc’s type to "html" and content type to "text/html".
  - 3. Let doctype be the result of creating a doctype given doc and "html".
  - 4. Append doctype to doc.
  - 5. Append the result of creating an element given doc, "html", and the HTML namespace, to doc.
  - 6. Append the result of creating an element given doc, "head", and the HTML namespace, to the html element created earlier.
  - 7. If title is given:
    - 7a. Append the result of creating an element given doc, "title", and the HTML namespace, to the head element created earlier.
    - 7b. Let text be the result of creating a text node given doc and title (which could be the empty string).
    - 7c. Append text to the title element created earlier.
  - 8. Append the result of creating an element given doc, "body", and the HTML namespace, to the html element created earlier.
  - 9. doc’s origin is this’s associated document’s origin.
  - 10. Return doc.

- [ ] [DOMImplementation.hasFeature()](https://dom.spec.whatwg.org/#dom-domimplementation-hasfeature) — The hasFeature() method steps are to return true.

### [4.6. Interface DocumentType](https://dom.spec.whatwg.org/#interface-documenttype)

Status: unreviewed
<!-- definitions: concept-doctype concept-doctype-name concept-doctype-publicid concept-doctype-systemid -->

- [ ] [create a doctype](https://dom.spec.whatwg.org/#create-a-doctype) — To create a doctype given a document document, a string name, and optionally a string publicId (default the empty string) and a string systemId (default the empty string):
  - 1. Let doctype be the result of creating a node that implements DocumentType, given document.
  - 2. Set doctype’s name to name, public ID to publicId, and system ID to systemId.
  - 3. Return doctype.

- [ ] [DocumentType.name](https://dom.spec.whatwg.org/#dom-documenttype-name) — The name getter steps are to return this’s name.

- [ ] [DocumentType.publicId](https://dom.spec.whatwg.org/#dom-documenttype-publicid) — The publicId getter steps are to return this’s public ID.

- [ ] [DocumentType.systemId](https://dom.spec.whatwg.org/#dom-documenttype-systemid) — The systemId getter steps are to return this’s system ID.

### [4.7. Interface DocumentFragment](https://dom.spec.whatwg.org/#interface-documentfragment)

Status: unreviewed
<!-- definitions: concept-documentfragment-host -->

- [ ] [exclusive DocumentFragment node](https://dom.spec.whatwg.org/#exclusive-documentfragment-node) — An exclusive DocumentFragment node is a DocumentFragment node that is not a ShadowRoot node.

- [ ] [host-including inclusive ancestor](https://dom.spec.whatwg.org/#concept-tree-host-including-inclusive-ancestor) — An object A is a host-including inclusive ancestor of an object B, if either A is an inclusive ancestor of B, or if B’s root has a non-null host and A is a host-including inclusive ancestor of B’s root’s host.

- [ ] [create a document fragment](https://dom.spec.whatwg.org/#create-a-document-fragment) — To create a document fragment given a document document: return the result of creating a node that implements DocumentFragment, given document.

- [ ] [DocumentFragment.new DocumentFragment()](https://dom.spec.whatwg.org/#dom-documentfragment-documentfragment) — The new DocumentFragment() constructor steps are to set this’s node document to current global object’s associated Document.

### [4.8. Interface ShadowRoot](https://dom.spec.whatwg.org/#interface-shadowroot)

Status: unreviewed
<!-- definitions: concept-shadow-root shadowroot-mode shadowroot-delegates-focus shadowroot-available-to-element-internals shadowroot-declarative shadowroot-serializable shadowroot-slot-assignment shadowroot-clonable shadowroot-custom-element-registry shadowroot-keep-custom-element-registry-null dom-shadowroot-onslotchange shadowroot-onslotchange concept-shadow-including-tree-order shadow-including-preorder-depth-first-traversal concept-shadow-including-root concept-shadow-including-descendant concept-shadow-including-inclusive-descendant concept-shadow-including-ancestor concept-shadow-including-inclusive-ancestor concept-closed-shadow-hidden -->

- [ ] [ShadowRoot.mode](https://dom.spec.whatwg.org/#dom-shadowroot-mode) — The mode getter steps are to return this’s mode.

- [ ] [ShadowRoot.delegatesFocus](https://dom.spec.whatwg.org/#dom-shadowroot-delegatesfocus) — The delegatesFocus getter steps are to return this’s delegates focus.

- [ ] [ShadowRoot.serializable](https://dom.spec.whatwg.org/#dom-shadowroot-serializable) — The serializable getter steps are to return this’s serializable.

- [ ] [ShadowRoot.slotAssignment](https://dom.spec.whatwg.org/#dom-shadowroot-slotassignment) — The slotAssignment getter steps are to return this’s slot assignment.

- [ ] [ShadowRoot.clonable](https://dom.spec.whatwg.org/#dom-shadowroot-clonable) — The clonable getter steps are to return this’s clonable.

- [ ] [ShadowRoot.host](https://dom.spec.whatwg.org/#dom-shadowroot-host) — The host getter steps are to return this’s host.

- [ ] [retarget](https://dom.spec.whatwg.org/#retarget) — To retarget an object A against an object B, repeat these steps until they return an object:
  - 1. If one of the following is true A is not a node; A’s root is not a shadow root; B is a node and A’s root is a shadow-including inclusive ancestor of B then return A.
  - 2. Set A to A’s root’s host.

### [4.9. Interface Element](https://dom.spec.whatwg.org/#interface-element)

Status: unreviewed
<!-- definitions: concept-element concept-element-namespace concept-element-namespace-prefix concept-element-local-name element-custom-element-registry concept-element-custom-element-state concept-element-custom-element-definition concept-element-is-value concept-element-defined concept-element-custom concept-element-shadow-root element-shadow-host concept-element-qualified-name concept-element-attribute concept-element-attribute-has concept-element-attributes-change-ext parent-element concept-reflect concept-class dom-element-setattributenodens valid-shadow-host-name dom-element-webkitmatchesselector -->

- [ ] [HTML-uppercased qualified name (Element)](https://dom.spec.whatwg.org/#element-html-uppercased-qualified-name) — An element’s HTML-uppercased qualified name is the return value of these steps:
  - 1. Let qualifiedName be this’s qualified name.
  - 2. If this is in the HTML namespace and its node document is an HTML document, then return qualifiedName in ASCII uppercase.
  - 3. Return qualifiedName.

- [ ] [create an element](https://dom.spec.whatwg.org/#concept-create-element) — To create an element, given a document document, string localName, string-or-null namespace, and optionally a string-or-null prefix (default null), string-or-null is (default null), boolean synchronousCustomElements (default false), and "default", null, or a CustomElementRegistry object registry (d…
  - 1. Let result be null.
  - 2. If registry is "default", then set registry to the result of looking up a custom element registry given document.
  - 3. Let definition be the result of looking up a custom element definition given registry, namespace, localName, and is.
  - 4. If definition is non-null, and definition’s name is not equal to its local name (i.e., definition represents a customized built-in element):
    - 4a. Let interface be the element interface for localName and the HTML namespace.
    - 4b. Set result to the result of creating an element internal given document, interface, localName, the HTML namespace, prefix, "undefined", is, and registry.
    - 4c. If synchronousCustomElements is true, then run this step while catching any exceptions: If this step threw an exception exception:
      - 4c.i. Upgrade result using definition.
      - 4c.i. Report exception for definition’s constructor’s corresponding JavaScript object’s associated realm’s global object.
      - 4c.ii. Set result’s custom element state to "failed".
    - 4d. Otherwise, enqueue a custom element upgrade reaction given result and definition.
  - 5. Otherwise, if definition is non-null:
    - 5a. If synchronousCustomElements is true:
      - 5a.i. Let C be definition’s constructor.
      - 5a.ii. Let previousRegistry be the surrounding agent’s active custom element constructor map[C] with default null.
      - 5a.iii. Set the surrounding agent’s active custom element constructor map[C] to registry.
      - 5a.iv. Run these steps while catching any exceptions: If any of these steps threw an exception exception:
        - 5a.iv.A. Set result to the result of constructing C, with no arguments.
        - 5a.iv.B. Assert: one of the following is true: result’s custom element state is "custom" and its custom element definition is non-null;; result’s custom element state is "precustomized"; or; result’s custom element state is neither "custom" nor "precustomized" and its custom element definition is null.
        - 5a.iv.C. Assert: result’s namespace is the HTML namespace.
        - 5a.iv.D. If result’s attribute list is not empty, then throw a "NotSupportedError" DOMException.
        - 5a.iv.E. If result has children, then throw a "NotSupportedError" DOMException.
        - 5a.iv.F. If result’s parent is non-null, then throw a "NotSupportedError" DOMException.
        - 5a.iv.G. If result’s node document is not document, then throw a "NotSupportedError" DOMException.
        - 5a.iv.H. If result’s local name is not equal to localName, then throw a "NotSupportedError" DOMException.
        - 5a.iv.I. Set result’s namespace prefix to prefix.
        - 5a.iv.J. Set result’s is value to null.
        - 5a.iv.K. Set result’s custom element registry to registry.
        - 5a.iv.A. Report exception for definition’s constructor’s corresponding JavaScript object’s associated realm’s global object.
        - 5a.iv.B. Set result to the result of creating an element internal given document, HTMLUnknownElement, localName, the HTML namespace, prefix, "failed", null, and registry.
      - 5a.v. If previousRegistry is null, then remove the surrounding agent’s active custom element constructor map[C].
      - 5a.vi. Otherwise, set the surrounding agent’s active custom element constructor map[C] to previousRegistry.
    - 5b. Otherwise:
      - 5b.i. Set result to the result of creating an element internal given document, HTMLElement, localName, the HTML namespace, prefix, "undefined", null, and registry.
      - 5b.ii. Enqueue a custom element upgrade reaction given result and definition.
  - 6. Otherwise:
    - 6a. Let interface be the element interface for localName and namespace.
    - 6b. Set result to the result of creating an element internal given document, interface, localName, namespace, prefix, "uncustomized", is, and registry.
    - 6c. If namespace is the HTML namespace, and either localName is a valid custom element name or is is non-null, then set result’s custom element state to "undefined".
  - 7. Return result.

- [ ] [create an element internal](https://dom.spec.whatwg.org/#create-an-element-internal) — To create an element internal given a document document, an interface interface a string localName, a string-or-null namespace, a string-or-null prefix, a string state, a string-or-null is, and null or a CustomElementRegistry object registry:
  - 1. Let element be the result of creating a node that implements interface, given document.
  - 2. Set element’s namespace to namespace, namespace prefix to prefix, local name to localName, custom element registry to registry, custom element state to state, custom element definition to null, and is value to is.
  - 3. Assert: element’s attribute list is empty.
  - 4. Return element.

- [ ] [handle attribute changes](https://dom.spec.whatwg.org/#handle-attribute-changes) — To handle attribute changes for an attribute attribute with element, oldValue, and newValue, run these steps:
  - 1. Queue a mutation record of "attributes" for element with attribute’s local name, attribute’s namespace, oldValue, « », « », null, and null.
  - 2. If element is custom, then enqueue a custom element callback reaction with element, callback name "attributeChangedCallback", and « attribute’s local name, oldValue, newValue, attribute’s namespace ».
  - 3. Run the attribute change steps with element, attribute’s local name, oldValue, newValue, and attribute’s namespace.

- [ ] [change](https://dom.spec.whatwg.org/#concept-element-attributes-change) — To change an attribute attribute to value:
  - 1. Let oldValue be attribute’s value.
  - 2. Set attribute’s value to value.
  - 3. Handle attribute changes for attribute with attribute’s element, oldValue, and value.

- [ ] [append](https://dom.spec.whatwg.org/#concept-element-attributes-append) — To append an attribute attribute to an element element:
  - 1. Append attribute to element’s attribute list.
  - 2. Set attribute’s element to element.
  - 3. Set attribute’s node document to element’s node document.
  - 4. Handle attribute changes for attribute with element, null, and attribute’s value.

- [ ] [remove](https://dom.spec.whatwg.org/#concept-element-attributes-remove) — To remove an attribute attribute:
  - 1. Let element be attribute’s element.
  - 2. Remove attribute from element’s attribute list.
  - 3. Set attribute’s element to null.
  - 4. Handle attribute changes for attribute with element, attribute’s value, and null.

- [ ] [replace](https://dom.spec.whatwg.org/#concept-element-attributes-replace) — To replace an attribute oldAttribute with an attribute newAttribute:
  - 1. Let element be oldAttribute’s element.
  - 2. Replace oldAttribute by newAttribute in element’s attribute list.
  - 3. Set newAttribute’s element to element.
  - 4. Set newAttribute’s node document to element’s node document.
  - 5. Set oldAttribute’s element to null.
  - 6. Handle attribute changes for oldAttribute with element, oldAttribute’s value, and newAttribute’s value.

- [ ] [get an attribute by name](https://dom.spec.whatwg.org/#concept-element-attributes-get-by-name) — To get an attribute by name given a string qualifiedName and an element element:
  - 1. If element is in the HTML namespace and its node document is an HTML document, then set qualifiedName to qualifiedName in ASCII lowercase.
  - 2. Return the first attribute in element’s attribute list whose qualified name is qualifiedName; otherwise null.

- [ ] [get an attribute by namespace and local name](https://dom.spec.whatwg.org/#concept-element-attributes-get-by-namespace) — To get an attribute by namespace and local name given null or a string namespace, a string localName, and an element element:
  - 1. If namespace is the empty string, then set it to null.
  - 2. Return the attribute in element’s attribute list whose namespace is namespace and local name is localName, if any; otherwise null.

- [ ] [get an attribute value](https://dom.spec.whatwg.org/#concept-element-attributes-get-value) — To get an attribute value given an element element, a string localName, and an optional null or string namespace (default null):
  - 1. Let attr be the result of getting an attribute given namespace, localName, and element.
  - 2. If attr is null, then return the empty string.
  - 3. Return attr’s value.

- [ ] [set an attribute](https://dom.spec.whatwg.org/#concept-element-attributes-set) — To set an attribute given an attribute attr and an element element:
  - 1. Let verifiedValue be the result of calling get trusted type compliant attribute value with attr’s local name, attr’s namespace, element, and attr’s value. [TRUSTED-TYPES]
  - 2. If attr’s element is neither null nor element, throw an "InUseAttributeError" DOMException.
  - 3. Let oldAttr be the result of getting an attribute given attr’s namespace, attr’s local name, and element.
  - 4. If oldAttr is attr, return attr.
  - 5. Set attr’s value to verifiedValue.
  - 6. If oldAttr is non-null, then replace oldAttr with attr.
  - 7. Otherwise, append attr to element.
  - 8. Return oldAttr.

- [ ] [set an attribute value](https://dom.spec.whatwg.org/#concept-element-attributes-set-value) — To set an attribute value given an element element, a string localName, a string value, an optional null or string prefix (default null), and an optional null or string namespace (default null):
  - 1. Let attribute be the result of getting an attribute given namespace, localName, and element.
  - 2. If attribute is null, then append the result of creating an attribute given element’s node document, localName, namespace, prefix, and value to element, and then return.
  - 3. Change attribute to value.

- [ ] [remove an attribute by name](https://dom.spec.whatwg.org/#concept-element-attributes-remove-by-name) — To remove an attribute by name given a string qualifiedName and an element element:
  - 1. Let attr be the result of getting an attribute given qualifiedName and element.
  - 2. If attr is non-null, then remove attr.
  - 3. Return attr.

- [ ] [remove an attribute by namespace and local name](https://dom.spec.whatwg.org/#concept-element-attributes-remove-by-namespace) — To remove an attribute by namespace and local name given null or a string namespace, a string localName, and an element element:
  - 1. Let attr be the result of getting an attribute given namespace, localName, and element.
  - 2. If attr is non-null, then remove attr.
  - 3. Return attr.

- [ ] [Use these attribute change steps to update an element’s ID](https://dom.spec.whatwg.org/#concept-id) — Use these attribute change steps to update an element’s ID:
  - 1. If localName is id, namespace is null, and value is null or the empty string, then unset element’s ID.
  - 2. Otherwise, if localName is id, namespace is null, then set element’s ID to value.

- [ ] [Element.namespaceURI](https://dom.spec.whatwg.org/#dom-element-namespaceuri) — The namespaceURI getter steps are to return this’s namespace.

- [ ] [Element.prefix](https://dom.spec.whatwg.org/#dom-element-prefix) — The prefix getter steps are to return this’s namespace prefix.

- [ ] [Element.localName](https://dom.spec.whatwg.org/#dom-element-localname) — The localName getter steps are to return this’s local name.

- [ ] [Element.tagName](https://dom.spec.whatwg.org/#dom-element-tagname) — The tagName getter steps are to return this’s HTML-uppercased qualified name.

- [ ] [Element.id](https://dom.spec.whatwg.org/#dom-element-id) — The id attribute must reflect "id".

- [ ] [Element.className](https://dom.spec.whatwg.org/#dom-element-classname) — The className attribute must reflect "class".

- [ ] [Element.classList](https://dom.spec.whatwg.org/#dom-element-classlist) — The classList getter steps are to return a DOMTokenList object whose associated element is this and whose associated attribute’s local name is class. The token set of this particular DOMTokenList object are also known as the element’s classes.

- [ ] [Element.slot](https://dom.spec.whatwg.org/#dom-element-slot) — The slot attribute must reflect "slot".

- [ ] [Element.hasAttributes()](https://dom.spec.whatwg.org/#dom-element-hasattributes) — The hasAttributes() method steps are to return false if this’s attribute list is empty; otherwise true.

- [ ] [Element.attributes](https://dom.spec.whatwg.org/#dom-element-attributes) — The attributes getter steps are to return the associated NamedNodeMap.

- [ ] [Element.getAttributeNames()](https://dom.spec.whatwg.org/#dom-element-getattributenames) — The getAttributeNames() method steps are to return the qualified names of the attributes in this’s attribute list, in order; otherwise a new list.

- [ ] [Element.getAttribute(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-getattribute) — The getAttribute(qualifiedName) method steps are:
  - 1. Let attr be the result of getting an attribute given qualifiedName and this.
  - 2. If attr is null, return null.
  - 3. Return attr’s value.

- [ ] [Element.getAttributeNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-getattributens) — The getAttributeNS(namespace, localName) method steps are:
  - 1. Let attr be the result of getting an attribute given namespace, localName, and this.
  - 2. If attr is null, return null.
  - 3. Return attr’s value.

- [ ] [Element.setAttribute(qualifiedName, value)](https://dom.spec.whatwg.org/#dom-element-setattribute) — The setAttribute(qualifiedName, value) method steps are:
  - 1. If qualifiedName is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
  - 2. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName to qualifiedName in ASCII lowercase.
  - 3. Let verifiedValue be the result of calling get trusted type compliant attribute value with qualifiedName, null, this, and value. [TRUSTED-TYPES]
  - 4. Let attribute be the first attribute in this’s attribute list whose qualified name is qualifiedName, and null otherwise.
  - 5. If attribute is non-null, then change attribute to verifiedValue and return.
  - 6. Set attribute to the result of creating an attribute given this’s node document, qualifiedName, null, null, and verifiedValue.
  - 7. Append attribute to this.

- [ ] [Element.setAttributeNS(namespace, qualifiedName, value)](https://dom.spec.whatwg.org/#dom-element-setattributens) — The setAttributeNS(namespace, qualifiedName, value) method steps are:
  - 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and qualifiedName given "attribute".
  - 2. Let verifiedValue be the result of calling get trusted type compliant attribute value with localName, namespace, this, and value. [TRUSTED-TYPES]
  - 3. Set an attribute value for this using localName, verifiedValue, prefix, and namespace.

- [ ] [Element.removeAttribute(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-removeattribute) — The removeAttribute(qualifiedName) method steps are to remove an attribute given qualifiedName and this, and then return undefined.

- [ ] [Element.removeAttributeNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-removeattributens) — The removeAttributeNS(namespace, localName) method steps are to remove an attribute given namespace, localName, and this, and then return undefined.

- [ ] [Element.hasAttribute(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-hasattribute) — The hasAttribute(qualifiedName) method steps are:
  - 1. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName to qualifiedName in ASCII lowercase.
  - 2. Return true if this has an attribute whose qualified name is qualifiedName; otherwise false.

- [ ] [Element.toggleAttribute(qualifiedName, force)](https://dom.spec.whatwg.org/#dom-element-toggleattribute) — The toggleAttribute(qualifiedName, force) method steps are:
  - 1. If qualifiedName is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
  - 2. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName to qualifiedName in ASCII lowercase.
  - 3. Let attribute be the first attribute in this’s attribute list whose qualified name is qualifiedName, and null otherwise.
  - 4. If attribute is null:
    - 4a. If force is not given or is true, then append the result of creating an attribute given this’s node document and qualifiedName to this, and then return true.
    - 4b. Return false.
  - 5. If force is not given or is false, remove an attribute given qualifiedName and this, and then return false.
  - 6. Return true.

- [ ] [Element.hasAttributeNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-hasattributens) — The hasAttributeNS(namespace, localName) method steps are:
  - 1. If namespace is the empty string, then set it to null.
  - 2. Return true if this has an attribute whose namespace is namespace and local name is localName; otherwise false.

- [ ] [Element.getAttributeNode(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-getattributenode) — The getAttributeNode(qualifiedName) method steps are to return the result of getting an attribute given qualifiedName and this.

- [ ] [Element.getAttributeNodeNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-getattributenodens) — The getAttributeNodeNS(namespace, localName) method steps are to return the result of getting an attribute given namespace, localName, and this.

- [ ] [Element.setAttributeNode(attr)](https://dom.spec.whatwg.org/#dom-element-setattributenode) — The setAttributeNode(attr) and setAttributeNodeNS(attr) methods steps are to return the result of setting an attribute given attr and this.

- [ ] [Element.removeAttributeNode(attr)](https://dom.spec.whatwg.org/#dom-element-removeattributenode) — The removeAttributeNode(attr) method steps are:
  - 1. If this’s attribute list does not contain attr, then throw a "NotFoundError" DOMException.
  - 2. Remove attr.
  - 3. Return attr.

- [ ] [Element.attachShadow(init)](https://dom.spec.whatwg.org/#dom-element-attachshadow) — The attachShadow(init) method steps are:
  - 1. Let registry be this’s node document’s custom element registry.
  - 2. If init["customElementRegistry"] exists, then set registry to it.
  - 3. If registry is non-null, registry’s is scoped is false, and registry is not this’s node document’s custom element registry, then throw a "NotSupportedError" DOMException.
  - 4. Run attach a shadow root with this, init["mode"], init["delegatesFocus"], init["serializable"], init["slotAssignment"], init["clonable"], and registry.
  - 5. Return this’s shadow root.

- [ ] [attach a shadow root](https://dom.spec.whatwg.org/#concept-attach-a-shadow-root) — To attach a shadow root, given an element element, a string mode, a boolean delegatesFocus, a boolean serializable, a string slotAssignment, a boolean clonable, and null or a CustomElementRegistry object registry:
  - 1. If element’s namespace is not the HTML namespace, then throw a "NotSupportedError" DOMException.
  - 2. If element’s local name is not a valid shadow host name, then throw a "NotSupportedError" DOMException.
  - 3. If element’s local name is a valid custom element name, or element’s is value is non-null:
    - 3a. Let definition be the result of looking up a custom element definition given element’s custom element registry, its namespace, its local name, and its is value.
    - 3b. If definition is non-null and definition’s disable shadow is true, then throw a "NotSupportedError" DOMException.
  - 4. If element is a shadow host:
    - 4a. Let currentShadowRoot be element’s shadow root.
    - 4b. If any of the following are true: currentShadowRoot’s declarative is false; or; currentShadowRoot’s mode is not mode, then throw a "NotSupportedError" DOMException.
    - 4c. Remove all of currentShadowRoot’s children, in tree order.
    - 4d. Set currentShadowRoot’s declarative to false.
    - 4e. Return.
  - 5. Let shadow be the result of creating a node that implements ShadowRoot, given element’s node document.
  - 6. Set shadow’s host to element.
  - 7. Set shadow’s mode to mode.
  - 8. Set shadow’s delegates focus to delegatesFocus.
  - 9. If element’s custom element state is "precustomized" or "custom", then set shadow’s available to element internals to true.
  - 10. Set shadow’s declarative to false.
  - 11. Set shadow’s serializable to serializable.
  - 12. Set shadow’s slot assignment to slotAssignment.
  - 13. Set shadow’s clonable to clonable.
  - 14. Set shadow’s custom element registry to registry.
  - 15. Set element’s shadow root to shadow.

- [ ] [Element.shadowRoot](https://dom.spec.whatwg.org/#dom-element-shadowroot) — The shadowRoot getter steps are:
  - 1. Let shadow be this’s shadow root.
  - 2. If shadow is null or its mode is "closed", then return null.
  - 3. Return shadow.

- [ ] [Element.customElementRegistry](https://dom.spec.whatwg.org/#dom-element-customelementregistry) — The customElementRegistry getter steps are to return this’s custom element registry.

- [ ] [Element.closest(selectors)](https://dom.spec.whatwg.org/#dom-element-closest) — The closest(selectors) method steps are:
  - 1. Let selector be the result of parse a selector from selectors. [SELECTORS4]
  - 2. If selector is failure, then throw a "SyntaxError" DOMException.
  - 3. Let elements be this’s inclusive ancestors that are elements, in reverse tree order.
  - 4. For each element of elements: if match a selector against an element, using selector, element, and scoping root this, returns success, return element. [SELECTORS4]
  - 5. Return null.

- [ ] [Element.matches(selectors)](https://dom.spec.whatwg.org/#dom-element-matches) — The matches(selectors) and webkitMatchesSelector(selectors) method steps are:
  - 1. Let selector be the result of parse a selector from selectors. [SELECTORS4]
  - 2. If selector is failure, then throw a "SyntaxError" DOMException.
  - 3. If the result of match a selector against an element, using selector, this, and scoping root this, returns success, then return true; otherwise, return false. [SELECTORS4]

- [ ] [Element.getElementsByTagName(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-getelementsbytagname) — The getElementsByTagName(qualifiedName) method steps are to return the list of elements with qualified name qualifiedName for this.

- [ ] [Element.getElementsByTagNameNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-getelementsbytagnamens) — The getElementsByTagNameNS(namespace, localName) method steps are to return the list of elements with namespace namespace and local name localName for this.

- [ ] [Element.getElementsByClassName(classNames)](https://dom.spec.whatwg.org/#dom-element-getelementsbyclassname) — The getElementsByClassName(classNames) method steps are to return the list of elements with class names classNames for this.

- [ ] [insert adjacent](https://dom.spec.whatwg.org/#insert-adjacent) — To insert adjacent, given an element element, string where, and a node node, run the steps associated with the first ASCII case-insensitive match for where:
  - • "beforebegin": If element’s parent is null, return null. Return the result of pre-inserting node into element’s parent before element.
  - • "afterbegin": Return the result of pre-inserting node into element before element’s first child.
  - • "beforeend": Return the result of pre-inserting node into element before null.
  - • "afterend": If element’s parent is null, return null. Return the result of pre-inserting node into element’s parent before element’s next sibling.
  - • Otherwise: Throw a "SyntaxError" DOMException.

- [ ] [Element.insertAdjacentElement(where, element)](https://dom.spec.whatwg.org/#dom-element-insertadjacentelement) — The insertAdjacentElement(where, element) method steps are to return the result of running insert adjacent, give this, where, and element.

- [ ] [Element.insertAdjacentText(where, data)](https://dom.spec.whatwg.org/#dom-element-insertadjacenttext) — The insertAdjacentText(where, data) method steps are:
  - 1. Let text be the result of creating a text node given this’s node document and data.
  - 2. Run insert adjacent, given this, where, and text.

#### [4.9.1. Interface NamedNodeMap](https://dom.spec.whatwg.org/#interface-namednodemap)

Status: unreviewed
<!-- definitions: concept-namednodemap-element concept-namednodemap-attribute dom-namednodemap-setnameditemns -->

- [ ] [NamedNodeMap.length](https://dom.spec.whatwg.org/#dom-namednodemap-length) — The length getter steps are to return the attribute list’s size.

- [ ] [NamedNodeMap.item(index)](https://dom.spec.whatwg.org/#dom-namednodemap-item) — The item(index) method steps are:
  - 1. If index is equal to or greater than this’s attribute list’s size, then return null.
  - 2. Otherwise, return this’s attribute list[index].

- [ ] [A NamedNodeMap object’s supported property names are the return value of runnin…](https://dom.spec.whatwg.org/#dom-namednodemap-item) — A NamedNodeMap object’s supported property names are the return value of running these steps:
  - 1. Let names be the qualified names of the attributes in this NamedNodeMap object’s attribute list, with duplicates omitted, in order.
  - 2. If this NamedNodeMap object’s element is in the HTML namespace and its node document is an HTML document, then for each name of names:
    - 2a. Let lowercaseName be name, in ASCII lowercase.
    - 2b. If lowercaseName is not equal to name, remove name from names.
  - 3. Return names.

- [ ] [NamedNodeMap.getNamedItem(qualifiedName)](https://dom.spec.whatwg.org/#dom-namednodemap-getnameditem) — The getNamedItem(qualifiedName) method steps are to return the result of getting an attribute given qualifiedName and element.

- [ ] [NamedNodeMap.getNamedItemNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-namednodemap-getnameditemns) — The getNamedItemNS(namespace, localName) method steps are to return the result of getting an attribute given namespace, localName, and element.

- [ ] [NamedNodeMap.setNamedItem(attr)](https://dom.spec.whatwg.org/#dom-namednodemap-setnameditem) — The setNamedItem(attr) and setNamedItemNS(attr) method steps are to return the result of setting an attribute given attr and element.

- [ ] [NamedNodeMap.removeNamedItem(qualifiedName)](https://dom.spec.whatwg.org/#dom-namednodemap-removenameditem) — The removeNamedItem(qualifiedName) method steps are:
  - 1. Let attr be the result of removing an attribute given qualifiedName and element.
  - 2. If attr is null, then throw a "NotFoundError" DOMException.
  - 3. Return attr.

- [ ] [NamedNodeMap.removeNamedItemNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-namednodemap-removenameditemns) — The removeNamedItemNS(namespace, localName) method steps are:
  - 1. Let attr be the result of removing an attribute given namespace, localName, and element.
  - 2. If attr is null, then throw a "NotFoundError" DOMException.
  - 3. Return attr.

#### [4.9.2. Interface Attr](https://dom.spec.whatwg.org/#interface-attr)

Status: unreviewed
<!-- definitions: concept-attribute concept-attribute-namespace concept-attribute-namespace-prefix concept-attribute-local-name concept-attribute-value concept-attribute-element concept-attribute-qualified-name concept-named-attribute -->

- [ ] [create an attribute](https://dom.spec.whatwg.org/#create-an-attribute) — To create an attribute given a document document, a string localName, and optionally a string-or-null namespace (default null), a string-or-null prefix (default null), and a string value (default the empty string):
  - 1. Let attribute be the result of creating a node that implements Attr, given document.
  - 2. Set attribute’s namespace to namespace, namespace prefix to prefix, local name to localName, and value to value.
  - 3. Return attribute.

- [ ] [Attr.namespaceURI](https://dom.spec.whatwg.org/#dom-attr-namespaceuri) — The namespaceURI getter steps are to return this’s namespace.

- [ ] [Attr.prefix](https://dom.spec.whatwg.org/#dom-attr-prefix) — The prefix getter steps are to return this’s namespace prefix.

- [ ] [Attr.localName](https://dom.spec.whatwg.org/#dom-attr-localname) — The localName getter steps are to return this’s local name.

- [ ] [Attr.name](https://dom.spec.whatwg.org/#dom-attr-name) — The name getter steps are to return this’s qualified name.

- [ ] [Attr.value](https://dom.spec.whatwg.org/#dom-attr-value) — The value getter steps are to return this’s value.

- [ ] [set an existing attribute value](https://dom.spec.whatwg.org/#set-an-existing-attribute-value) — To set an existing attribute value, given an attribute attribute and string value, run these steps:
  - 1. If attribute’s element is null, then set attribute’s value to value and return.
  - 2. Let element be attribute’s element.
  - 3. Let verifiedValue be the result of calling get trusted type compliant attribute value with attribute’s local name, attribute’s namespace, element, and value. [TRUSTED-TYPES]
  - 4. If attribute’s element is null, then set attribute’s value to verifiedValue and return.
  - 5. Change attribute to verifiedValue.

- [ ] [The value setter steps](https://dom.spec.whatwg.org/#dom-attr-value) — The value setter steps are to set an existing attribute value with this and the given value.

- [ ] [Attr.ownerElement](https://dom.spec.whatwg.org/#dom-attr-ownerelement) — The ownerElement getter steps are to return this’s element.

- [ ] [Attr.specified](https://dom.spec.whatwg.org/#dom-attr-specified) — The specified getter steps are to return true.

### [4.10. Interface CharacterData](https://dom.spec.whatwg.org/#interface-characterdata)

Status: unreviewed
<!-- definitions: concept-cd-data -->

- [ ] [replace data](https://dom.spec.whatwg.org/#concept-cd-replace) — To replace data of a node node with an integer offset, integer count, string data, and an optional boolean piAttributesAlreadyUpdated (default false):
  - 1. Let length be node’s length.
  - 2. If offset is greater than length, then throw an "IndexSizeError" DOMException.
  - 3. If offset + count is greater than length, then set count to length − offset.
  - 4. Queue a mutation record of "characterData" for node with null, null, node’s data, « », « », null, and null.
  - 5. Insert data into node’s data after offset code units.
  - 6. Let deleteOffset be offset + data’s length.
  - 7. Starting from deleteOffset code units, remove count code units from node’s data.
  - 8. For each live range whose start node is node and start offset is greater than offset but less than or equal to offset + count: set its start offset to offset.
  - 9. For each live range whose end node is node and end offset is greater than offset but less than or equal to offset + count: set its end offset to offset.
  - 10. For each live range whose start node is node and start offset is greater than offset + count: increase its start offset by data’s length and decrease it by count.
  - 11. For each live range whose end node is node and end offset is greater than offset + count: increase its end offset by data’s length and decrease it by count.
  - 12. If node is a ProcessingInstruction node and piAttributesAlreadyUpdated is false, then update attributes from data given node.
  - 13. If node’s parent is non-null, then run the children changed steps for node’s parent.

- [ ] [substring data (CharacterData, Text, Comment, ProcessingInstruction)](https://dom.spec.whatwg.org/#concept-cd-substring) — To substring data of a node node with an integer offset and integer count:
  - 1. Let length be node’s length.
  - 2. If offset is greater than length, then throw an "IndexSizeError" DOMException.
  - 3. If offset + count is greater than length, then return a string whose value is the code units from the offsetth code unit to the end of node’s data.
  - 4. Return a string whose value is the code units from the offsetth code unit to the offset+countth code unit in node’s data.

- [ ] [CharacterData.data](https://dom.spec.whatwg.org/#dom-characterdata-data) — The data getter steps are to return this’s data. Its setter steps are to replace data of this with 0, this’s length, and the given value.

- [ ] [CharacterData.length](https://dom.spec.whatwg.org/#dom-characterdata-length) — The length getter steps are to return this’s length.

- [ ] [CharacterData.substringData(offset, count)](https://dom.spec.whatwg.org/#dom-characterdata-substringdata) — The substringData(offset, count) method steps are to return the result of substringing data of this with offset and count.

- [ ] [CharacterData.appendData(data)](https://dom.spec.whatwg.org/#dom-characterdata-appenddata) — The appendData(data) method steps are to replace data of this with this’s length, 0, and data.

- [ ] [CharacterData.insertData(offset, data)](https://dom.spec.whatwg.org/#dom-characterdata-insertdata) — The insertData(offset, data) method steps are to replace data of this with offset, 0, and data.

- [ ] [CharacterData.deleteData(offset, count)](https://dom.spec.whatwg.org/#dom-characterdata-deletedata) — The deleteData(offset, count) method steps are to replace data of this with offset, count, and the empty string.

- [ ] [CharacterData.replaceData(offset, count, data)](https://dom.spec.whatwg.org/#dom-characterdata-replacedata) — The replaceData(offset, count, data) method steps are to replace data of this with offset, count, and data.

### [4.11. Interface Text](https://dom.spec.whatwg.org/#interface-text)

Status: unreviewed

- [ ] [exclusive Text node](https://dom.spec.whatwg.org/#exclusive-text-node) — An exclusive Text node is a Text node that is not a CDATASection node.

- [ ] [contiguous Text nodes](https://dom.spec.whatwg.org/#contiguous-text-nodes) — The contiguous Text nodes of a node node are node, node’s previous sibling Text node, if any, and its contiguous Text nodes, and node’s next sibling Text node, if any, and its contiguous Text nodes, avoiding any duplicates.

- [ ] [contiguous exclusive Text nodes](https://dom.spec.whatwg.org/#contiguous-exclusive-text-nodes) — The contiguous exclusive Text nodes of a node node are node, node’s previous sibling exclusive Text node, if any, and its contiguous exclusive Text nodes, and node’s next sibling exclusive Text node, if any, and its contiguous exclusive Text nodes, avoiding any duplicates.

- [ ] [child text content](https://dom.spec.whatwg.org/#concept-child-text-content) — The child text content of a node node is the concatenation of the data of all the Text node children of node, in tree order.

- [ ] [descendant text content](https://dom.spec.whatwg.org/#concept-descendant-text-content) — The descendant text content of a node node is the concatenation of the data of all the Text node descendants of node, in tree order.

- [ ] [create a text node](https://dom.spec.whatwg.org/#create-a-text-node) — To create a text node given a document document and a string data:
  - 1. Let text be the result of creating a node that implements Text, given document.
  - 2. Set text’s data to data.
  - 3. Return text.

- [ ] [Text.new Text(data)](https://dom.spec.whatwg.org/#dom-text-text) — The new Text(data) constructor steps are to set this’s data to data and this’s node document to current global object’s associated Document.

- [ ] [split](https://dom.spec.whatwg.org/#concept-text-split) — To split a Text node node with integer offset:
  - 1. Let length be node’s length.
  - 2. If offset is greater than length, then throw an "IndexSizeError" DOMException.
  - 3. Let count be length − offset.
  - 4. Let newData be the result of substringing data of node with offset and count.
  - 5. Let newNode be the result of creating a text node given node’s node document and newData.
  - 6. Let parent be node’s parent.
  - 7. If parent is non-null:
    - 7a. Insert newNode into parent before node’s next sibling.
    - 7b. For each live range whose start node is node and start offset is greater than offset, set its start node to newNode and decrease its start offset by offset.
    - 7c. For each live range whose end node is node and end offset is greater than offset, set its end node to newNode and decrease its end offset by offset.
    - 7d. For each live range whose start node is parent and start offset is equal to the index of node plus 1, increase its start offset by 1.
    - 7e. For each live range whose end node is parent and end offset is equal to the index of node plus 1, increase its end offset by 1.
  - 8. Replace data of node with offset, count, and the empty string.
  - 9. Return newNode.

- [ ] [Text.splitText(offset)](https://dom.spec.whatwg.org/#dom-text-splittext) — The splitText(offset) method steps are to split this with offset offset.

- [ ] [Text.wholeText](https://dom.spec.whatwg.org/#dom-text-wholetext) — The wholeText getter steps are to return the concatenation of the data of the contiguous Text nodes of this, in tree order.

### [4.12. Interface CDATASection](https://dom.spec.whatwg.org/#interface-cdatasection)

Status: unreviewed

- [ ] [create a CDATA section node](https://dom.spec.whatwg.org/#create-a-cdata-section-node) — To create a CDATA section node given a document document and a string data:
  - 1. Let node be the result of creating a node that implements CDATASection, given document.
  - 2. Set node’s data to data.
  - 3. Return node.

### [4.13. Interface ProcessingInstruction](https://dom.spec.whatwg.org/#interface-processinginstruction)

Status: unreviewed
<!-- definitions: concept-pi-target concept-pi-attribute-map -->

- [ ] [create a processing instruction node](https://dom.spec.whatwg.org/#create-a-processing-instruction-node) — To create a processing instruction node given a document document, a string target, and a string data:
  - 1. Let pi be the result of creating a node that implements ProcessingInstruction, given document.
  - 2. Initialize pi with target and data.
  - 3. Return pi.

- [ ] [initialize (ProcessingInstruction)](https://dom.spec.whatwg.org/#processinginstruction-initialize) — To initialize a ProcessingInstruction node pi, with target and data:
  - 1. If target does not match the Name production, then throw an "InvalidCharacterError" DOMException.
  - 2. If data contains the string "?>", then throw an "InvalidCharacterError" DOMException.
  - 3. Set pi’s target to target.
  - 4. Set pi’s data to data.
  - 5. Update attributes from data given pi.

- [ ] [update attributes from data](https://dom.spec.whatwg.org/#update-attributes-from-data) — To update attributes from data, given a ProcessingInstruction node pi:
  - 1. Clear pi’s attribute map.
  - 2. Let result be the parsing result of invoking the rules for parsing pseudo-attributes from a string given pi’s data.
  - 3. If result is an error, then return.
  - 4. For each pseudo-attribute pseudoAttr of result: set pi’s attribute map[pseudoAttr’s name] to pseudoAttr’s value.

- [ ] [update data from attributes](https://dom.spec.whatwg.org/#update-data-from-attributes) — To update data from attributes, given a ProcessingInstruction node pi:
  - 1. Let data be the empty string.
  - 2. For each name → value of pi’s attribute map:
    - 2a. If data is not the empty string, then append U+0020 SPACE to data.
    - 2b. Append name to data.
    - 2c. Append U+003D (=) to data.
    - 2d. Append U+0022 (") to data.
    - 2e. Replace any U+0026 (&) in value with "&amp;".
    - 2f. Replace any U+003C (<) in value with "&lt;".
    - 2g. Replace any U+003E (>) in value with "&gt;".
    - 2h. Replace any U+0022 (") in value with "&quot;".
    - 2i. Append value to data.
    - 2j. Append U+0022 (") to data.
  - 3. Replace data of pi with 0, pi’s length, data, and true.

- [ ] [get a processing instruction attribute](https://dom.spec.whatwg.org/#get-a-processing-instruction-attribute) — To get a processing instruction attribute, given a ProcessingInstruction node pi and a string name: return pi’s attribute map[name] with default null.

- [ ] [ProcessingInstruction.new ProcessingInstruction(target, data)](https://dom.spec.whatwg.org/#dom-processinginstruction-processinginstruction) — The new ProcessingInstruction(target, data) constructor steps are:
  - 1. Set this’s node document to current global object’s associated Document.
  - 2. Initialize this with target and data.

- [ ] [ProcessingInstruction.target](https://dom.spec.whatwg.org/#dom-processinginstruction-target) — The target getter steps are to return this’s target.

- [ ] [ProcessingInstruction.hasAttributes()](https://dom.spec.whatwg.org/#dom-processinginstruction-hasattributes) — The hasAttributes() method steps are to return false if this’s attribute map is empty; otherwise true.

- [ ] [ProcessingInstruction.getAttributeNames()](https://dom.spec.whatwg.org/#dom-processinginstruction-getattributenames) — The getAttributeNames() method steps are to return the result of getting the keys of this’s attribute map.

- [ ] [ProcessingInstruction.getAttribute(name)](https://dom.spec.whatwg.org/#dom-processinginstruction-getattribute) — The getAttribute(name) method steps are to return the result of getting a processing instruction attribute given this and name.

- [ ] [ProcessingInstruction.setAttribute(name, value)](https://dom.spec.whatwg.org/#dom-processinginstruction-setattribute) — The setAttribute(name, value) method steps are:
  - 1. If name is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
  - 2. Set this’s attribute map[name] to value.
  - 3. Update data from attributes given this.

- [ ] [ProcessingInstruction.removeAttribute(name)](https://dom.spec.whatwg.org/#dom-processinginstruction-removeattribute) — The removeAttribute(name) method steps are:
  - 1. Remove this’s attribute map[name].
  - 2. Update data from attributes given this.

- [ ] [ProcessingInstruction.toggleAttribute(name, force)](https://dom.spec.whatwg.org/#dom-processinginstruction-toggleattribute) — The toggleAttribute(name, force) method steps are:
  - 1. If name is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
  - 2. Let attributes be this’s attribute map.
  - 3. If attributes[name] does not exist:
    - 3a. If force is not given or is true, set attributes[name] to the empty string, update data from attributes given this, and then return true.
    - 3b. Return false.
  - 4. If force is not given or is false, remove attributes[name], update data from attributes given this, and then return false.
  - 5. Return true.

- [ ] [ProcessingInstruction.hasAttribute(name)](https://dom.spec.whatwg.org/#dom-processinginstruction-hasattribute) — The hasAttribute(name) method steps are to return true if this’s attribute map[name] exists; otherwise false.

### [4.14. Interface Comment](https://dom.spec.whatwg.org/#interface-comment)

Status: unreviewed

- [ ] [create a comment node](https://dom.spec.whatwg.org/#create-a-comment-node) — To create a comment node given a document document and a string data:
  - 1. Let comment be the result of creating a node that implements Comment, given document.
  - 2. Set comment’s data to data.
  - 3. Return comment.

- [ ] [Comment.new Comment(data)](https://dom.spec.whatwg.org/#dom-comment-comment) — The new Comment(data) constructor steps are to set this’s data to data and this’s node document to current global object’s associated Document.

## [5. Ranges](https://dom.spec.whatwg.org/#ranges)

Status: unreviewed

### [5.1. Introduction to "DOM Ranges"](https://dom.spec.whatwg.org/#introduction-to-dom-ranges)

Status: unreviewed

### [5.2. Boundary points](https://dom.spec.whatwg.org/#boundary-points)

Status: unreviewed
<!-- definitions: concept-range-bp boundary-point-node concept-range-bp-offset concept-range-bp-before concept-range-bp-equal concept-range-bp-after -->

- [ ] [position (boundary point)](https://dom.spec.whatwg.org/#concept-range-bp-position) — The position of a boundary point (nodeA, offsetA) relative to a boundary point (nodeB, offsetB) is before, equal, or after, as returned by these steps:
  - 1. Assert: nodeA and nodeB have the same root.
  - 2. If nodeA is nodeB, then return equal if offsetA is offsetB, before if offsetA is less than offsetB, and after if offsetA is greater than offsetB.
  - 3. If nodeA is following nodeB, then if the position of (nodeB, offsetB) relative to (nodeA, offsetA) is before, return after, and if it is after, return before.
  - 4. If nodeA is an ancestor of nodeB:
    - 4a. Let child be nodeB.
    - 4b. While child is not a child of nodeA, set child to its parent.
    - 4c. If child’s index is less than offsetA, then return after.
  - 5. Return before.

### [5.3. Interface AbstractRange](https://dom.spec.whatwg.org/#interface-abstractrange)

Status: unreviewed
<!-- definitions: concept-range concept-range-start concept-range-end concept-range-start-node concept-range-start-offset concept-range-end-node concept-range-end-offset -->

- [ ] [collapsed (range)](https://dom.spec.whatwg.org/#range-collapsed) — A range is collapsed if its start node is its end node and its start offset is its end offset.

- [ ] [AbstractRange.startContainer](https://dom.spec.whatwg.org/#dom-range-startcontainer) — The startContainer getter steps are to return this’s start node.

- [ ] [AbstractRange.startOffset](https://dom.spec.whatwg.org/#dom-range-startoffset) — The startOffset getter steps are to return this’s start offset.

- [ ] [AbstractRange.endContainer](https://dom.spec.whatwg.org/#dom-range-endcontainer) — The endContainer getter steps are to return this’s end node.

- [ ] [AbstractRange.endOffset](https://dom.spec.whatwg.org/#dom-range-endoffset) — The endOffset getter steps are to return this’s end offset.

- [ ] [AbstractRange.collapsed](https://dom.spec.whatwg.org/#dom-range-collapsed) — The collapsed getter steps are to return true if this is collapsed; otherwise false.

### [5.4. Interface StaticRange](https://dom.spec.whatwg.org/#interface-staticrange)

Status: unreviewed

- [ ] [StaticRange.new StaticRange(init)](https://dom.spec.whatwg.org/#dom-staticrange-staticrange) — The new StaticRange(init) constructor steps are:
  - 1. If init["startContainer"] or init["endContainer"] is a DocumentType or Attr node, then throw an "InvalidNodeTypeError" DOMException.
  - 2. Set this’s start to (init["startContainer"], init["startOffset"]) and end to (init["endContainer"], init["endOffset"]).

- [ ] [valid (StaticRange)](https://dom.spec.whatwg.org/#staticrange-valid) — A StaticRange is valid if all of the following are true:

### [5.5. Interface Range](https://dom.spec.whatwg.org/#interface-range)

Status: unreviewed
<!-- definitions: concept-live-range concept-range-root contained partially-contained -->

- [ ] [live range pre-remove steps](https://dom.spec.whatwg.org/#live-range-pre-remove-steps) — The live range pre-remove steps given a node node, are:
  - 1. Let parent be node’s parent.
  - 2. Assert: parent is non-null.
  - 3. Let index be node’s index.
  - 4. For each live range whose start node is an inclusive descendant of node, set its start to (parent, index).
  - 5. For each live range whose end node is an inclusive descendant of node, set its end to (parent, index).
  - 6. For each live range whose start node is parent and start offset is greater than index, decrease its start offset by 1.
  - 7. For each live range whose end node is parent and end offset is greater than index, decrease its end offset by 1.

- [ ] [Range.new Range()](https://dom.spec.whatwg.org/#dom-range-range) — The new Range() constructor steps are to set this’s start and end to (current global object’s associated Document, 0).

- [ ] [get the common ancestor](https://dom.spec.whatwg.org/#get-the-common-ancestor) — To get the common ancestor of a live range range:
  - 1. Let container be range’s start node.
  - 2. While container is not an inclusive ancestor of range’s end node: set container to container’s parent.
  - 3. Return container.

- [ ] [Range.commonAncestorContainer](https://dom.spec.whatwg.org/#dom-range-commonancestorcontainer) — The commonAncestorContainer getter steps are to return the result of getting the common ancestor of this.

- [ ] [set the start or end (Range)](https://dom.spec.whatwg.org/#concept-range-bp-set) — To set the start or end of a range to a boundary point (node, offset):
  - 1. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
  - 2. If offset is greater than node’s length, then throw an "IndexSizeError" DOMException.
  - 3. Let boundaryPoint be the boundary point (node, offset).
  - 4. If these steps were invoked as "set the start" If these steps were invoked as "set the end"
    - 4a. If range’s root is not node’s root or boundaryPoint is after range’s end, then set range’s end to boundaryPoint.
    - 4b. Set range’s start to boundaryPoint.
    - 4a. If range’s root is not node’s root or boundaryPoint is before range’s start, then set range’s start to boundaryPoint.
    - 4b. Set range’s end to boundaryPoint.

- [ ] [Range.setStart(node, offset)](https://dom.spec.whatwg.org/#dom-range-setstart) — The setStart(node, offset) method steps are to set the start of this to boundary point (node, offset).

- [ ] [Range.setEnd(node, offset)](https://dom.spec.whatwg.org/#dom-range-setend) — The setEnd(node, offset) method steps are to set the end of this to boundary point (node, offset).

- [ ] [Range.setStartBefore(node)](https://dom.spec.whatwg.org/#dom-range-setstartbefore) — The setStartBefore(node) method steps are:
  - 1. Let parent be node’s parent.
  - 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
  - 3. Set the start of this to boundary point (parent, node’s index).

- [ ] [Range.setStartAfter(node)](https://dom.spec.whatwg.org/#dom-range-setstartafter) — The setStartAfter(node) method steps are:
  - 1. Let parent be node’s parent.
  - 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
  - 3. Set the start of this to boundary point (parent, node’s index plus 1).

- [ ] [Range.setEndBefore(node)](https://dom.spec.whatwg.org/#dom-range-setendbefore) — The setEndBefore(node) method steps are:
  - 1. Let parent be node’s parent.
  - 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
  - 3. Set the end of this to boundary point (parent, node’s index).

- [ ] [Range.setEndAfter(node)](https://dom.spec.whatwg.org/#dom-range-setendafter) — The setEndAfter(node) method steps are:
  - 1. Let parent be node’s parent.
  - 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
  - 3. Set the end of this to boundary point (parent, node’s index plus 1).

- [ ] [Range.collapse(toStart)](https://dom.spec.whatwg.org/#dom-range-collapse) — The collapse(toStart) method steps are to, if toStart is true, set end to start; otherwise set start to end.

- [ ] [select (range)](https://dom.spec.whatwg.org/#concept-range-select) — To select a node node within a range range:
  - 1. Let parent be node’s parent.
  - 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
  - 3. Let index be node’s index.
  - 4. Set range’s start to boundary point (parent, index).
  - 5. Set range’s end to boundary point (parent, index plus 1).

- [ ] [Range.selectNode(node)](https://dom.spec.whatwg.org/#dom-range-selectnode) — The selectNode(node) method steps are to select node within this.

- [ ] [Range.selectNodeContents(node)](https://dom.spec.whatwg.org/#dom-range-selectnodecontents) — The selectNodeContents(node) method steps are:
  - 1. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
  - 2. Let length be the length of node.
  - 3. Set start to the boundary point (node, 0).
  - 4. Set end to the boundary point (node, length).

- [ ] [Range.compareBoundaryPoints(how, sourceRange)](https://dom.spec.whatwg.org/#dom-range-compareboundarypoints) — The compareBoundaryPoints(how, sourceRange) method steps are:
  - 1. If how is not one of START_TO_START,; START_TO_END,; END_TO_END, and; END_TO_START, then throw a "NotSupportedError" DOMException.
  - 2. If this’s root is not sourceRange’s root, then throw a "WrongDocumentError" DOMException.
  - 3. Let thisPoint and sourcePoint be null.
  - 4. Switch on how: START_TO_START: Set thisPoint to this’s start and sourcePoint to sourceRange’s start. START_TO_END: Set thisPoint to this’s end and sourcePoint to sourceRange’s start. END_TO_END: Set thisPoint to this’s end and sourcePoint to sourceRange’s end. END_TO_START: Set thisPoint to this’s start and sourcePoint to sourceRange’s end.
  - 5. Switch on the position of thisPoint relative to sourcePoint: before Return −1. equal Return 0. after Return 1.

- [ ] [Range.deleteContents()](https://dom.spec.whatwg.org/#dom-range-deletecontents) — The deleteContents() method steps are:
  - 1. If this is collapsed, then return.
  - 2. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be this’s start node, start offset, end node, and end offset, respectively.
  - 3. If originalStartNode is originalEndNode and it is a CharacterData node:
    - 3a. Replace data of originalStartNode with originalStartOffset, originalEndOffset − originalStartOffset, and the empty string.
    - 3b. Return.
  - 4. Let nodesToRemove be a list of all the nodes that are contained in this, in tree order, omitting any node whose parent is also contained in this.
  - 5. Let newNode and newOffset be null.
  - 6. If originalStartNode is an inclusive ancestor of originalEndNode, then set newNode to originalStartNode and newOffset to originalStartOffset.
  - 7. Otherwise:
    - 7a. Let referenceNode be originalStartNode.
    - 7b. While referenceNode’s parent is non-null and is not an inclusive ancestor of originalEndNode: set referenceNode to its parent.
    - 7c. Set newNode to referenceNode’s parent and newOffset to referenceNode’s index + 1.
  - 8. Set this’s start and end to (newNode, newOffset).
  - 9. If originalStartNode is a CharacterData node, then replace data of originalStartNode with originalStartOffset, originalStartNode’s length − originalStartOffset, and the empty string.
  - 10. For each node of nodesToRemove, in tree order: remove node.
  - 11. If originalEndNode is a CharacterData node, then replace data of originalEndNode with 0, originalEndOffset, and the empty string.

- [ ] [extract (live range)](https://dom.spec.whatwg.org/#concept-range-extract) — To extract a live range range:
  - 1. Let fragment be the result of creating a document fragment given range’s start node’s node document.
  - 2. If range is collapsed, then return fragment.
  - 3. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be range’s start node, start offset, end node, and end offset, respectively.
  - 4. If originalStartNode is originalEndNode and it is a CharacterData node:
    - 4a. Let clone be a clone of originalStartNode.
    - 4b. Set clone’s data to the result of substringing data of originalStartNode with originalStartOffset and originalEndOffset − originalStartOffset.
    - 4c. Append clone to fragment.
    - 4d. Replace data of originalStartNode with originalStartOffset, originalEndOffset − originalStartOffset, and the empty string.
    - 4e. Return fragment.
  - 5. Let commonAncestor be the result of getting the common ancestor of range.
  - 6. Let firstPartiallyContainedChild be null.
  - 7. If originalStartNode is not an inclusive ancestor of originalEndNode, then set firstPartiallyContainedChild to the first child of commonAncestor that is partially contained in range.
  - 8. Let lastPartiallyContainedChild be null.
  - 9. If originalEndNode is not an inclusive ancestor of originalStartNode, then set lastPartiallyContainedChild to the last child of commonAncestor that is partially contained in range.
  - 10. Let containedChildren be a list of all children of commonAncestor that are contained in range, in tree order.
  - 11. If any member of containedChildren is a doctype, then throw a "HierarchyRequestError" DOMException.
  - 12. Let newNode and newOffset be null.
  - 13. If originalStartNode is an inclusive ancestor of originalEndNode, then set newNode to originalStartNode and newOffset to originalStartOffset.
  - 14. Otherwise:
    - 14a. Let referenceNode be originalStartNode.
    - 14b. While referenceNode’s parent is non-null and is not an inclusive ancestor of originalEndNode: set referenceNode to its parent.
    - 14c. Set newNode to the parent of referenceNode, and newOffset to referenceNode’s index + 1.
  - 15. Set range’s start and end to (newNode, newOffset).
  - 16. If firstPartiallyContainedChild is a CharacterData node:
    - 16a. Let clone be a clone of originalStartNode.
    - 16b. Set the data of clone to the result of substringing data of originalStartNode with originalStartOffset and originalStartNode’s length − originalStartOffset.
    - 16c. Append clone to fragment.
    - 16d. Replace data of originalStartNode with originalStartOffset, originalStartNode’s length − originalStartOffset, and the empty string.
  - 17. Otherwise, if firstPartiallyContainedChild is non-null:
    - 17a. Let clone be a clone of firstPartiallyContainedChild.
    - 17b. Append clone to fragment.
    - 17c. Let subrange be a new live range whose start is (originalStartNode, originalStartOffset) and whose end is (firstPartiallyContainedChild, firstPartiallyContainedChild’s length).
    - 17d. Let subfragment be the result of extracting subrange.
    - 17e. Append subfragment to clone.
  - 18. For each contained child of containedChildren: append contained child to fragment.
  - 19. If lastPartiallyContainedChild is a CharacterData node:
    - 19a. Let clone be a clone of originalEndNode.
    - 19b. Set clone’s data to the result of substringing data of originalEndNode with 0 and originalEndOffset.
    - 19c. Append clone to fragment.
    - 19d. Replace data of originalEndNode with 0, originalEndOffset, and the empty string.
  - 20. Otherwise, if lastPartiallyContainedChild is non-null:
    - 20a. Let clone be a clone of lastPartiallyContainedChild.
    - 20b. Append clone to fragment.
    - 20c. Let subrange be a new live range whose start is (lastPartiallyContainedChild, 0) and whose end is (originalEndNode, originalEndOffset).
    - 20d. Let subfragment be the result of extracting subrange.
    - 20e. Append subfragment to clone.
  - 21. Return fragment.

- [ ] [Range.extractContents()](https://dom.spec.whatwg.org/#dom-range-extractcontents) — The extractContents() method steps are to return the result of extracting this.

- [ ] [clone the contents (live range)](https://dom.spec.whatwg.org/#concept-range-clone) — To clone the contents of a live range range:
  - 1. Let fragment be the result of creating a document fragment given range’s start node’s node document.
  - 2. If range is collapsed, then return fragment.
  - 3. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be range’s start node, start offset, end node, and end offset, respectively.
  - 4. If originalStartNode is originalEndNode and it is a CharacterData node:
    - 4a. Let clone be a clone of originalStartNode.
    - 4b. Set clone’s data to the result of substringing data of originalStartNode with originalStartOffset and originalEndOffset − originalStartOffset.
    - 4c. Append clone to fragment.
    - 4d. Return fragment.
  - 5. Let commonAncestor be the result of getting the common ancestor of range.
  - 6. Let firstPartiallyContainedChild be null.
  - 7. If originalStartNode is not an inclusive ancestor of originalEndNode, then set firstPartiallyContainedChild to the first child of commonAncestor that is partially contained in range.
  - 8. Let lastPartiallyContainedChild be null.
  - 9. If originalEndNode is not an inclusive ancestor of originalStartNode, then set lastPartiallyContainedChild to the last child of commonAncestor that is partially contained in range.
  - 10. Let containedChildren be a list of all children of commonAncestor that are contained in range, in tree order.
  - 11. If any member of containedChildren is a doctype, then throw a "HierarchyRequestError" DOMException.
  - 12. If firstPartiallyContainedChild is a CharacterData node:
    - 12a. Let clone be a clone of originalStartNode.
    - 12b. Set the data of clone to the result of substringing data of originalStartNode with originalStartOffset and originalStartNode’s length − originalStartOffset.
    - 12c. Append clone to fragment.
  - 13. Otherwise, if firstPartiallyContainedChild is non-null:
    - 13a. Let clone be a clone of firstPartiallyContainedChild.
    - 13b. Append clone to fragment.
    - 13c. Let subrange be a new live range whose start is (originalStartNode, originalStartOffset) and whose end is (firstPartiallyContainedChild, firstPartiallyContainedChild’s length).
    - 13d. Let subfragment be the result of cloning the contents of subrange.
    - 13e. Append subfragment to clone.
  - 14. For each contained child of containedChildren:
    - 14a. Let clone be a clone of contained child with subtree set to true.
    - 14b. Append clone to fragment.
  - 15. If lastPartiallyContainedChild is a CharacterData node:
    - 15a. Let clone be a clone of originalEndNode.
    - 15b. Set clone’s data to the result of substringing data of originalEndNode with 0 and originalEndOffset.
    - 15c. Append clone to fragment.
  - 16. Otherwise, if lastPartiallyContainedChild is non-null:
    - 16a. Let clone be a clone of lastPartiallyContainedChild.
    - 16b. Append clone to fragment.
    - 16c. Let subrange be a new live range whose start is (lastPartiallyContainedChild, 0) and whose end is (originalEndNode, originalEndOffset).
    - 16d. Let subfragment be the result of cloning the contents of subrange.
    - 16e. Append subfragment to clone.
  - 17. Return fragment.

- [ ] [Range.cloneContents()](https://dom.spec.whatwg.org/#dom-range-clonecontents) — The cloneContents() method steps are to return the result of cloning the contents of this.

- [ ] [insert (live range)](https://dom.spec.whatwg.org/#concept-range-insert) — To insert a node node into a live range range:
  - 1. If range’s start node is a ProcessingInstruction or Comment node, is a Text node whose parent is null, or is node, then throw a "HierarchyRequestError" DOMException.
  - 2. Let referenceNode be null.
  - 3. If range’s start node is a Text node, then set referenceNode to that Text node.
  - 4. Otherwise, set referenceNode to the child of range’s start node whose index is range’s start offset if there is such a child; otherwise null.
  - 5. Let parent be range’s start node if referenceNode is null; otherwise referenceNode’s parent.
  - 6. Ensure pre-insert validity given node, parent, referenceNode, and « ».
  - 7. If range’s start node is a Text node, then set referenceNode to the result of splitting it with offset range’s start offset.
  - 8. If node is referenceNode, then set referenceNode to its next sibling.
  - 9. If node’s parent is non-null, then remove node.
  - 10. Let newOffset be parent’s length if referenceNode is null; otherwise referenceNode’s index.
  - 11. Increase newOffset by node’s length if node is a DocumentFragment node; otherwise 1.
  - 12. Pre-insert node into parent before referenceNode.
  - 13. If range is collapsed, then set range’s end to (parent, newOffset).

- [ ] [Range.insertNode(node)](https://dom.spec.whatwg.org/#dom-range-insertnode) — The insertNode(node) method steps are to insert node into this.

- [ ] [Range.surroundContents(newParent)](https://dom.spec.whatwg.org/#dom-range-surroundcontents) — The surroundContents(newParent) method steps are:
  - 1. If a non-Text node is partially contained in this, then throw an "InvalidStateError" DOMException.
  - 2. If newParent is a Document, DocumentType, or DocumentFragment node, then throw an "InvalidNodeTypeError" DOMException.
  - 3. Let fragment be the result of extracting this.
  - 4. If newParent has children, then replace all with null within newParent.
  - 5. Insert newParent into this.
  - 6. Append fragment to newParent.
  - 7. Select newParent within this.

- [ ] [Range.cloneRange()](https://dom.spec.whatwg.org/#dom-range-clonerange) — The cloneRange() method steps are to return a new live range with the same start and end as this.

- [ ] [Range.detach()](https://dom.spec.whatwg.org/#dom-range-detach) — The detach() method steps are to do nothing.

- [ ] [Range.isPointInRange(node, offset)](https://dom.spec.whatwg.org/#dom-range-ispointinrange) — The isPointInRange(node, offset) method steps are:
  - 1. If node’s root is not this’s root, then return false.
  - 2. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
  - 3. If offset is greater than node’s length, then throw an "IndexSizeError" DOMException.
  - 4. If (node, offset) is before start or after end, then return false.
  - 5. Return true.

- [ ] [Range.comparePoint(node, offset)](https://dom.spec.whatwg.org/#dom-range-comparepoint) — The comparePoint(node, offset) method steps are:
  - 1. If node’s root is not this’s root, then throw a "WrongDocumentError" DOMException.
  - 2. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
  - 3. If offset is greater than node’s length, then throw an "IndexSizeError" DOMException.
  - 4. If (node, offset) is before start, then return −1.
  - 5. If (node, offset) is after end, then return 1.
  - 6. Return 0.

- [ ] [Range.intersectsNode(node)](https://dom.spec.whatwg.org/#dom-range-intersectsnode) — The intersectsNode(node) method steps are:
  - 1. If node’s root is not this’s root, then return false.
  - 2. Let parent be node’s parent.
  - 3. If parent is null, then return true.
  - 4. Let offset be node’s index.
  - 5. If (parent, offset) is before end and (parent, offset + 1) is after start, then return true.
  - 6. Return false.

- [ ] [stringification behavior (Range)](https://dom.spec.whatwg.org/#dom-range-stringifier) — The stringification behavior must run these steps:
  - 1. Let string be the empty string.
  - 2. If this’s start node is this’s end node and it is a Text node, then return the substring of that Text node’s data beginning at this’s start offset and ending at this’s end offset.
  - 3. If this’s start node is a Text node, then append the substring of that node’s data from this’s start offset until the end to string.
  - 4. Append the concatenation of the data of all Text nodes that are contained in this, in tree order, to string.
  - 5. If this’s end node is a Text node, then append the substring of that node’s data from its start until this’s end offset to string.
  - 6. Return string.

## [6. Traversal](https://dom.spec.whatwg.org/#traversal)

Status: unreviewed
<!-- definitions: concept-traversal-active concept-traversal-root concept-traversal-whattoshow concept-traversal-filter -->

- [ ] [filter](https://dom.spec.whatwg.org/#concept-node-filter) — To filter a node node within a NodeIterator or TreeWalker object traverser:
  - 1. If traverser’s is active is true, then throw an "InvalidStateError" DOMException.
  - 2. Let n be node’s nodeType attribute value − 1.
  - 3. If the nth bit (where 0 is the least significant bit) of traverser’s whatToShow is not set, then return FILTER_SKIP.
  - 4. If traverser’s filter is null, then return FILTER_ACCEPT.
  - 5. Set traverser’s is active to true.
  - 6. Let result be the return value of call a user object’s operation with traverser’s filter, "acceptNode", and « node ». If this throws an exception, then set traverser’s is active to false and rethrow the exception.
  - 7. Set traverser’s is active to false.
  - 8. Return result.

### [6.1. Interface NodeIterator](https://dom.spec.whatwg.org/#interface-nodeiterator)

Status: unreviewed
<!-- definitions: iterator-collection node-pointer node-pointer-node node-pointer-pointer-before nodeiterator-reference nodeiterator-candidate-reference -->

- [ ] [NodeIterator pre-remove steps](https://dom.spec.whatwg.org/#nodeiterator-pre-removing-steps) — The NodeIterator pre-remove steps given a NodeIterator object nodeIterator and node toBeRemovedNode, are:
  - 1. Set nodeIterator’s reference to the result of adjusting a node pointer given nodeIterator’s reference, nodeIterator, and toBeRemovedNode.
  - 2. If nodeIterator’s candidate reference is non-null, then set nodeIterator’s candidate reference to the result of adjusting a node pointer given nodeIterator’s candidate reference, nodeIterator, and toBeRemovedNode.

- [ ] [adjust a node pointer (NodeIterator)](https://dom.spec.whatwg.org/#nodeiterator-adjust-a-node-pointer) — To adjust a node pointer given a node pointer nodePointer, a NodeIterator object nodeIterator, and a node toBeRemovedNode, perform the following steps. They return a node pointer.
  - 1. If toBeRemovedNode is not an inclusive ancestor of nodePointer’s node, or toBeRemovedNode is an inclusive ancestor of nodeIterator’s root, then return nodePointer.
  - 2. If nodePointer’s pointer before is true:
    - 2a. Let next be toBeRemovedNode’s first following node that is an inclusive descendant of nodeIterator’s root and is not an inclusive descendant of toBeRemovedNode, if there is such a node; otherwise null.
    - 2b. If next is non-null, then return (next, true).
  - 3. Let newNode be toBeRemovedNode’s parent, if toBeRemovedNode’s previous sibling is null; otherwise the inclusive descendant of toBeRemovedNode’s previous sibling that appears last in tree order.
  - 4. Return (newNode, false).

- [ ] [NodeIterator.root](https://dom.spec.whatwg.org/#dom-nodeiterator-root) — The root getter steps are to return this’s root.

- [ ] [NodeIterator.referenceNode](https://dom.spec.whatwg.org/#dom-nodeiterator-referencenode) — The referenceNode getter steps are to return this’s reference’s node.

- [ ] [NodeIterator.pointerBeforeReferenceNode](https://dom.spec.whatwg.org/#dom-nodeiterator-pointerbeforereferencenode) — The pointerBeforeReferenceNode getter steps are to return this’s reference’s pointer before.

- [ ] [NodeIterator.whatToShow](https://dom.spec.whatwg.org/#dom-nodeiterator-whattoshow) — The whatToShow getter steps are to return this’s whatToShow.

- [ ] [NodeIterator.filter](https://dom.spec.whatwg.org/#dom-nodeiterator-filter) — The filter getter steps are to return this’s filter.

- [ ] [traverse (NodeIterator)](https://dom.spec.whatwg.org/#concept-nodeiterator-traverse) — To traverse, given a NodeIterator object iterator and "next" or "previous" type:
  - 1. Set iterator’s candidate reference to iterator’s reference.
  - 2. Let result be null.
  - 3. While true:
    - 3a. If type is "next":
      - 3a.i. If iterator’s candidate reference’s pointer before is false:
        - 3a.i.A. Let following be the first node following iterator’s candidate reference’s node in iterator’s iterator collection. If there is no such node, then break.
        - 3a.i.B. Set iterator’s candidate reference to (following, false).
      - 3a.ii. Otherwise, set iterator’s candidate reference to (iterator’s candidate reference’s node, false).
    - 3b. Otherwise:
      - 3b.i. If iterator’s candidate reference’s pointer before is true:
        - 3b.i.A. Let preceding be the first node preceding iterator’s candidate reference’s node in iterator’s iterator collection. If there is no such node, then break.
        - 3b.i.B. Set iterator’s candidate reference to (preceding, true).
      - 3b.ii. Otherwise, set iterator’s candidate reference to (iterator’s candidate reference’s node, true).
    - 3c. Let node be iterator’s candidate reference’s node.
    - 3d. Let filterResult be the result of filtering node within iterator. If this throws an exception, then set iterator’s candidate reference to null and rethrow that exception.
    - 3e. If filterResult is FILTER_ACCEPT:
      - 3e.i. Set iterator’s reference to iterator’s candidate reference.
      - 3e.ii. Set result to node.
      - 3e.iii. Break.
  - 4. Set iterator’s candidate reference to null.
  - 5. Return result.

- [ ] [NodeIterator.nextNode()](https://dom.spec.whatwg.org/#dom-nodeiterator-nextnode) — The nextNode() method steps are to return the result of traversing with this and "next".

- [ ] [NodeIterator.previousNode()](https://dom.spec.whatwg.org/#dom-nodeiterator-previousnode) — The previousNode() method steps are to return the result of traversing with this and "previous".

- [ ] [NodeIterator.detach()](https://dom.spec.whatwg.org/#dom-nodeiterator-detach) — The detach() method steps are to do nothing.

### [6.2. Interface TreeWalker](https://dom.spec.whatwg.org/#interface-treewalker)

Status: unreviewed
<!-- definitions: treewalker-current -->

- [ ] [TreeWalker.root](https://dom.spec.whatwg.org/#dom-treewalker-root) — The root getter steps are to return this’s root.

- [ ] [TreeWalker.whatToShow](https://dom.spec.whatwg.org/#dom-treewalker-whattoshow) — The whatToShow getter steps are to return this’s whatToShow.

- [ ] [TreeWalker.filter](https://dom.spec.whatwg.org/#dom-treewalker-filter) — The filter getter steps are to return this’s filter.

- [ ] [TreeWalker.currentNode](https://dom.spec.whatwg.org/#dom-treewalker-currentnode) — The currentNode getter steps are to return this’s current.

- [ ] [The currentNode setter steps](https://dom.spec.whatwg.org/#dom-treewalker-currentnode) — The currentNode setter steps are to set this’s current to the given value.

- [ ] [TreeWalker.parentNode()](https://dom.spec.whatwg.org/#dom-treewalker-parentnode) — The parentNode() method steps are:
  - 1. Let node be this’s current.
  - 2. While node is non-null and is not this’s root:
    - 2a. Set node to node’s parent.
    - 2b. If node is non-null and filtering node within this returns FILTER_ACCEPT, then set this’s current to node and return node.
  - 3. Return null.

- [ ] [traverse children (TreeWalker)](https://dom.spec.whatwg.org/#concept-traverse-children) — To traverse children, given a TreeWalker object walker and "first" or "last" type:
  - 1. Let node be walker’s current.
  - 2. Set node to node’s first child if type is "first"; otherwise to node’s last child.
  - 3. While node is non-null:
    - 3a. Let result be the result of filtering node within walker.
    - 3b. If result is FILTER_ACCEPT, then set walker’s current to node and return node.
    - 3c. If result is FILTER_SKIP:
      - 3c.i. Let child be node’s first child if type is "first"; otherwise node’s last child.
      - 3c.ii. If child is non-null, then set node to child and continue.
    - 3d. While node is non-null:
      - 3d.i. Let sibling be node’s next sibling if type is "first"; otherwise node’s previous sibling.
      - 3d.ii. If sibling is non-null, then set node to sibling and break.
      - 3d.iii. Let parent be node’s parent.
      - 3d.iv. If parent is null, walker’s root, or walker’s current, then return null.
      - 3d.v. Set node to parent.
  - 4. Return null.

- [ ] [TreeWalker.firstChild()](https://dom.spec.whatwg.org/#dom-treewalker-firstchild) — The firstChild() method steps are to traverse children with this and "first".

- [ ] [TreeWalker.lastChild()](https://dom.spec.whatwg.org/#dom-treewalker-lastchild) — The lastChild() method steps are to traverse children with this and "last".

- [ ] [traverse siblings](https://dom.spec.whatwg.org/#concept-traverse-siblings) — To traverse siblings, given a TreeWalker object walker and "next" or "previous" type:
  - 1. Let node be walker’s current.
  - 2. If node is root, then return null.
  - 3. While true:
    - 3a. Let sibling be node’s next sibling if type is "next"; otherwise node’s previous sibling.
    - 3b. While sibling is non-null:
      - 3b.i. Set node to sibling.
      - 3b.ii. Let result be the result of filtering node within walker.
      - 3b.iii. If result is FILTER_ACCEPT, then set walker’s current to node and return node.
      - 3b.iv. Set sibling to node’s first child if type is "next"; otherwise to node’s last child.
      - 3b.v. If result is FILTER_REJECT or sibling is null, then set sibling to node’s next sibling if type is "next"; otherwise to node’s previous sibling.
    - 3c. Set node to node’s parent.
    - 3d. If node is null or walker’s root, then return null.
    - 3e. If the return value of filtering node within walker is FILTER_ACCEPT, then return null.

- [ ] [TreeWalker.nextSibling()](https://dom.spec.whatwg.org/#dom-treewalker-nextsibling) — The nextSibling() method steps are to traverse siblings with this and "next".

- [ ] [TreeWalker.previousSibling()](https://dom.spec.whatwg.org/#dom-treewalker-previoussibling) — The previousSibling() method steps are to traverse siblings with this and "previous".

- [ ] [TreeWalker.previousNode()](https://dom.spec.whatwg.org/#dom-treewalker-previousnode) — The previousNode() method steps are:
  - 1. Let node be this’s current.
  - 2. While node is not this’s root:
    - 2a. Let sibling be node’s previous sibling.
    - 2b. While sibling is non-null:
      - 2b.i. Set node to sibling.
      - 2b.ii. Let result be the result of filtering node within this.
      - 2b.iii. While result is not FILTER_REJECT and node has a child:
        - 2b.iii.A. Set node to node’s last child.
        - 2b.iii.B. Set result to the result of filtering node within this.
      - 2b.iv. If result is FILTER_ACCEPT, then set this’s current to node and return node.
      - 2b.v. Set sibling to node’s previous sibling.
    - 2c. If node is this’s root or node’s parent is null, then return null.
    - 2d. Set node to node’s parent.
    - 2e. If the return value of filtering node within this is FILTER_ACCEPT, then set this’s current to node and return node.
  - 3. Return null.

- [ ] [TreeWalker.nextNode()](https://dom.spec.whatwg.org/#dom-treewalker-nextnode) — The nextNode() method steps are:
  - 1. Let node be this’s current.
  - 2. Let result be FILTER_ACCEPT.
  - 3. While true:
    - 3a. While result is not FILTER_REJECT and node has a child:
      - 3a.i. Set node to its first child.
      - 3a.ii. Set result to the result of filtering node within this.
      - 3a.iii. If result is FILTER_ACCEPT, then set this’s current to node and return node.
    - 3b. Let sibling be null.
    - 3c. Let temporary be node.
    - 3d. While temporary is non-null:
      - 3d.i. If temporary is this’s root, then return null.
      - 3d.ii. Set sibling to temporary’s next sibling.
      - 3d.iii. If sibling is non-null, then break.
      - 3d.iv. Set temporary to temporary’s parent.
    - 3e. If sibling is null, then return null.
    - 3f. Set node to sibling.
    - 3g. Set result to the result of filtering node within this.
    - 3h. If result is FILTER_ACCEPT, then set this’s current to node and return node.

### [6.3. Interface NodeFilter](https://dom.spec.whatwg.org/#interface-nodefilter)

Status: unreviewed
<!-- definitions: dom-nodefilter-filter_accept dom-nodefilter-filter_reject dom-nodefilter-filter_skip dom-nodefilter-show_all dom-nodefilter-show_element dom-nodefilter-show_attribute dom-nodefilter-show_text dom-nodefilter-show_cdata_section dom-nodefilter-show_processing_instruction dom-nodefilter-show_comment dom-nodefilter-show_document dom-nodefilter-show_document_type dom-nodefilter-show_document_fragment -->

## [7. Sets](https://dom.spec.whatwg.org/#sets)

Status: unreviewed

### [7.1. Interface DOMTokenList](https://dom.spec.whatwg.org/#interface-domtokenlist)

Status: unreviewed
<!-- definitions: concept-dtl-tokens domtokenlist-element domtokenlist-attribute-name concept-supported-tokens -->

- [ ] [validation steps (DOMTokenList)](https://dom.spec.whatwg.org/#concept-domtokenlist-validation) — A DOMTokenList object set’s validation steps for a given token are:
  - 1. If set’s element and attribute name does not define supported tokens, then throw a TypeError.
  - 2. Let lowercaseToken be token, in ASCII lowercase.
  - 3. If lowercaseToken is present in the supported tokens of set’s element and attribute name, then return true.
  - 4. Return false.

- [ ] [update steps (DOMTokenList)](https://dom.spec.whatwg.org/#concept-dtl-update) — A DOMTokenList object set’s update steps are:
  - 1. If get an attribute by namespace and local name given null, set’s attribute name, and set’s element returns null and set’s token set is empty, then return.
  - 2. Set an attribute value given set’s element, set’s attribute name, and the result of running the ordered set serializer for set’s token set.

- [ ] [serialize steps (DOMTokenList)](https://dom.spec.whatwg.org/#concept-dtl-serialize) — A DOMTokenList object set’s serialize steps are to return the result of running get an attribute value given set’s element and set’s attribute name.

- [ ] [A DOMTokenList object set has these attribute change steps for set’s element](https://dom.spec.whatwg.org/#concept-dtl-serialize) — A DOMTokenList object set has these attribute change steps for set’s element:
  - 1. If localName is set’s attribute name, namespace is null, and value is null, then empty token set.
  - 2. Otherwise, if localName is set’s attribute name and namespace is null, then set set’s token set to value, parsed.

- [ ] [When a DOMTokenList object set is created](https://dom.spec.whatwg.org/#concept-dtl-serialize) — When a DOMTokenList object set is created:
  - 1. Let element be set’s element.
  - 2. Let attributeName be set’s attribute name.
  - 3. Let value be the result of getting an attribute value given element and attributeName.
  - 4. Run the attribute change steps for element, attributeName, value, value, and null.

- [ ] [DOMTokenList.length](https://dom.spec.whatwg.org/#dom-domtokenlist-length) — The length getter steps are to return this’s token set’s size.

- [ ] [DOMTokenList.item(index)](https://dom.spec.whatwg.org/#dom-domtokenlist-item) — The item(index) method steps are:
  - 1. If index is equal to or greater than this’s token set’s size, then return null.
  - 2. Return this’s token set[index].

- [ ] [DOMTokenList.contains(token)](https://dom.spec.whatwg.org/#dom-domtokenlist-contains) — The contains(token) method steps are to return true if this’s token set[token] exists; otherwise false.

- [ ] [DOMTokenList.add(tokens…)](https://dom.spec.whatwg.org/#dom-domtokenlist-add) — The add(tokens…) method steps are:
  - 1. For each token of tokens:
    - 1a. If token is the empty string, then throw a "SyntaxError" DOMException.
    - 1b. If token contains any ASCII whitespace, then throw an "InvalidCharacterError" DOMException.
  - 2. For each token of tokens: append token to this’s token set.
  - 3. Run the update steps.

- [ ] [DOMTokenList.remove(tokens…)](https://dom.spec.whatwg.org/#dom-domtokenlist-remove) — The remove(tokens…) method steps are:
  - 1. For each token of tokens:
    - 1a. If token is the empty string, then throw a "SyntaxError" DOMException.
    - 1b. If token contains any ASCII whitespace, then throw an "InvalidCharacterError" DOMException.
  - 2. For each token of tokens: remove token from this’s token set.
  - 3. Run the update steps.

- [ ] [DOMTokenList.toggle(token, force)](https://dom.spec.whatwg.org/#dom-domtokenlist-toggle) — The toggle(token, force) method steps are:
  - 1. If token is the empty string, then throw a "SyntaxError" DOMException.
  - 2. If token contains any ASCII whitespace, then throw an "InvalidCharacterError" DOMException.
  - 3. If this’s token set[token] exists:
    - 3a. If force is either not given or is false, then remove token from this’s token set, run the update steps and return false.
    - 3b. Return true.
  - 4. Otherwise, if force not given or is true, append token to this’s token set, run the update steps, and return true.
  - 5. Return false.

- [ ] [DOMTokenList.replace(token, newToken)](https://dom.spec.whatwg.org/#dom-domtokenlist-replace) — The replace(token, newToken) method steps are:
  - 1. If either token or newToken is the empty string, then throw a "SyntaxError" DOMException.
  - 2. If either token or newToken contains any ASCII whitespace, then throw an "InvalidCharacterError" DOMException.
  - 3. If this’s token set does not contain token, then return false.
  - 4. Replace token in this’s token set with newToken.
  - 5. Run the update steps.
  - 6. Return true.

- [ ] [DOMTokenList.supports(token)](https://dom.spec.whatwg.org/#dom-domtokenlist-supports) — The supports(token) method steps are:
  - 1. Let result be the return value of validation steps called with token.
  - 2. Return result.

- [ ] [DOMTokenList.value](https://dom.spec.whatwg.org/#dom-domtokenlist-value) — The value getter steps are to return the result of running this’s serialize steps.

- [ ] [The value setter steps](https://dom.spec.whatwg.org/#dom-domtokenlist-value) — The value setter steps are to set an attribute value for this’s element using this’s attribute name and the given value.

## [8. XPath](https://dom.spec.whatwg.org/#xpath)

Status: unreviewed

### [8.1. Interface XPathResult](https://dom.spec.whatwg.org/#interface-xpathresult)

Status: unreviewed

### [8.2. Interface XPathExpression](https://dom.spec.whatwg.org/#interface-xpathexpression)

Status: unreviewed

### [8.3. Mixin XPathEvaluatorBase](https://dom.spec.whatwg.org/#mixin-xpathevaluatorbase)

Status: unreviewed

- [ ] [XPathEvaluatorBase.createNSResolver(nodeResolver)](https://dom.spec.whatwg.org/#dom-xpathevaluatorbase-creatensresolver) — The createNSResolver(nodeResolver) method steps are to return nodeResolver.

### [8.4. Interface XPathEvaluator](https://dom.spec.whatwg.org/#interface-xpathevaluator)

Status: unreviewed

## [9. XSLT](https://dom.spec.whatwg.org/#xslt)

Status: unreviewed

### [9.1. Interface XSLTProcessor](https://dom.spec.whatwg.org/#interface-xsltprocessor)

Status: unreviewed

## [10. Security and privacy considerations](https://dom.spec.whatwg.org/#security-and-privacy)

Status: unreviewed

## [11. Historical](https://dom.spec.whatwg.org/#historical)

Status: unreviewed
<!-- definitions: domconfiguration domerror domerrorhandler domimplementationlist domimplementationsource domlocator domobject domuserdata entity entityreference mutationevent mutationnameevent namelist notation rangeexception typeinfo userdatahandler dom-attr-schematypeinfo dom-attr-isid dom-document-createentityreference dom-document-xmlencoding dom-document-xmlstandalone dom-document-xmlversion dom-document-stricterrorchecking dom-document-domconfig dom-document-normalizedocument dom-document-renamenode dom-documenttype-entities dom-documenttype-notations dom-documenttype-internalsubset dom-domimplementation-getfeature dom-element-schematypeinfo dom-element-setidattribute dom-element-setidattributens dom-element-setidattributenode dom-node-issupported dom-node-getfeature dom-node-getuserdata dom-node-setuserdata dom-nodeiterator-expandentityreferences dom-text-iselementcontentwhitespace dom-text-replacewholetext dom-treewalker-expandentityreferences -->

## [Acknowledgments](https://dom.spec.whatwg.org/#acks)

Status: unreviewed

## [Intellectual property rights](https://dom.spec.whatwg.org/#ipr)

Status: unreviewed

## [Index](https://dom.spec.whatwg.org/#index)

Status: unreviewed

### [Terms defined by this specification](https://dom.spec.whatwg.org/#index-defined-here)

Status: unreviewed

### [Terms defined by reference](https://dom.spec.whatwg.org/#index-defined-elsewhere)

Status: unreviewed

## [References](https://dom.spec.whatwg.org/#references)

Status: unreviewed

### [Normative References](https://dom.spec.whatwg.org/#normative)

Status: unreviewed

### [Non-Normative References](https://dom.spec.whatwg.org/#informative)

Status: unreviewed

## [IDL Index](https://dom.spec.whatwg.org/#idl-index)

Status: unreviewed
