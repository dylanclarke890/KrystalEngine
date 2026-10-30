# DOM checklist

Generated from <https://dom.spec.whatwg.org/> on 2026-10-25: 1794 checkbox items. Each section links to the spec. Each
algorithm is a top-level checkbox with its steps nested beneath it; the surrounding prose, notes
and IDL are kept as plain text.

Legend: `- [x]` done; `- [ ] … (partial)` partly done; `- [x] … (n/a: reason)` doesn't apply
(e.g. JS-only). Add notes as nested bullets or `> Notes:` blockquotes under a section.

## [1. Infrastructure](https://dom.spec.whatwg.org/#infrastructure)

This specification depends on the Infra Standard. \[INFRA\]

Some of the terms used in this specification are defined in Encoding, Selectors, Trusted Types, Web IDL, XML, Namespaces in XML, and Associating Style Sheets with XML documents. \[ENCODING\] \[SELECTORS4\] \[TRUSTED-TYPES\] \[WEBIDL\] \[XML\] \[XML-NAMES\] \[XML-STYLESHEET\]

When extensions are needed, the DOM Standard can be updated accordingly, or a new standard can be written that hooks into the provided extensibility hooks for [**applicable specifications**](https://dom.spec.whatwg.org/#other-applicable-specifications).

### [1.1. Trees](https://dom.spec.whatwg.org/#trees)

A [**tree**](https://dom.spec.whatwg.org/#concept-tree) is a finite hierarchical tree structure. In [**tree order**](https://dom.spec.whatwg.org/#concept-tree-order) is preorder, depth-first traversal of a tree.

An object that [**participates**](https://dom.spec.whatwg.org/#concept-tree-participate) in a tree has a [**parent**](https://dom.spec.whatwg.org/#concept-tree-parent), which is either null or an object, and has [**children**](https://dom.spec.whatwg.org/#concept-tree-child), which is an ordered set of objects. An object A whose parent is object B is a child of B.

The [**root**](https://dom.spec.whatwg.org/#concept-tree-root) of an object is itself, if its parent is null, or else it is the root of its parent. The root of a tree is any object participating in that tree whose parent is null.

An object A is called a [**descendant**](https://dom.spec.whatwg.org/#concept-tree-descendant) of an object B, if either A is a child of B or A is a child of an object C that is a descendant of B.

An [**inclusive descendant**](https://dom.spec.whatwg.org/#concept-tree-inclusive-descendant) is an object or one of its descendants.

An object A is called an [**ancestor**](https://dom.spec.whatwg.org/#concept-tree-ancestor) of an object B if and only if B is a descendant of A.

An [**inclusive ancestor**](https://dom.spec.whatwg.org/#concept-tree-inclusive-ancestor) is an object or one of its ancestors.

An object A is called a [**sibling**](https://dom.spec.whatwg.org/#concept-tree-sibling) of an object B, if and only if B and A share the same non-null parent.

An [**inclusive sibling**](https://dom.spec.whatwg.org/#concept-tree-inclusive-sibling) is an object or one of its siblings.

An object A is [**preceding**](https://dom.spec.whatwg.org/#concept-tree-preceding) an object B if A and B are in the same tree and A comes before B in tree order.

An object A is [**following**](https://dom.spec.whatwg.org/#concept-tree-following) an object B if A and B are in the same tree and A comes after B in tree order.

The [**first child**](https://dom.spec.whatwg.org/#concept-tree-first-child) of an object is its first child or null if it has no children.

The [**last child**](https://dom.spec.whatwg.org/#concept-tree-last-child) of an object is its last child or null if it has no children.

The [**previous sibling**](https://dom.spec.whatwg.org/#concept-tree-previous-sibling) of an object is its first preceding sibling or null if it has no preceding sibling.

The [**next sibling**](https://dom.spec.whatwg.org/#concept-tree-next-sibling) of an object is its first following sibling or null if it has no following sibling.

The [**index**](https://dom.spec.whatwg.org/#concept-tree-index) of an object is its number of preceding siblings, or 0 if it has none.

### [1.2. Ordered sets](https://dom.spec.whatwg.org/#ordered-sets)

- [ ] [ordered set parser](https://dom.spec.whatwg.org/#concept-ordered-set-parser) — The ordered set parser takes a string input and then runs these steps:
  - [ ] 1. Let inputTokens be the result of splitting input on ASCII whitespace.
  - [ ] 2. Let tokens be a new ordered set.
  - [ ] 3. For each token of inputTokens: append token to tokens.
  - [ ] 4. Return tokens.

- [ ] [ordered set serializer](https://dom.spec.whatwg.org/#concept-ordered-set-serializer) — The ordered set serializer takes a set and returns the concatenation of set using U+0020 SPACE.

### [1.3. Selectors](https://dom.spec.whatwg.org/#selectors)

- [ ] [scope-match a selectors string](https://dom.spec.whatwg.org/#scope-match-a-selectors-string) — To scope-match a selectors string given a string selectors against a node node:
  - [ ] 1. Let selector be the result of parse a selector selectors. \[SELECTORS4\]
  - [ ] 2. If selector is failure, then throw a "`SyntaxError`" `DOMException`.
  - [ ] 3. Return the result of match a selector against a tree with selector and node’s root using scoping root node. \[SELECTORS4\].

> **Note:** Support for namespaces within selectors is not planned and will not be added.

### [1.4. Name validation](https://dom.spec.whatwg.org/#namespaces)

- [ ] [valid namespace prefix](https://dom.spec.whatwg.org/#valid-namespace-prefix) — A string is a valid namespace prefix if its length is at least 1 and it does not contain ASCII whitespace, U+0000 NULL, U+002F (/), or U+003E (\>).

- [ ] [valid attribute local name](https://dom.spec.whatwg.org/#valid-attribute-local-name) — A string is a valid attribute local name if its length is at least 1 and it does not contain ASCII whitespace, U+0000 NULL, U+002F (/), U+003D (=), or U+003E (\>).

- [ ] [valid element local name](https://dom.spec.whatwg.org/#valid-element-local-name) — A string name is a valid element local name if the following steps return true:
  - [ ] 1. If name’s length is 0, then return false.
  - [ ] 2. If name’s 0th code point is an ASCII alpha:
    - [ ] 1. If name contains ASCII whitespace, U+0000 NULL, U+002F (/), or U+003E (\>), then return false.
    - [ ] 2. Return true.
  - [ ] 3. If name’s 0th code point is not U+003A (:), U+005F (\_), or in the range U+0080 to U+10FFFF, inclusive, then return false.
  - [ ] 4. If name’s subsequent code points, if any, are not ASCII alphas, ASCII digits, U+002D (-), U+002E (.), U+003A (:), U+005F (\_), or in the range U+0080 to U+10FFFF, inclusive, then return false.
  - [ ] 5. Return true.

> **Note:** This concept is used to validate element local names, when constructed by DOM APIs. The intention is to allow any name that is possible to construct using the HTML parser (the branch where the first code point is an ASCII alpha), plus some additional possibilities. For those additional possibilities, the ASCII range is restricted for historical reasons, but beyond ASCII anything is allowed.

> **Note:**
>
> The following JavaScript-compatible regular expression is an implementation of valid element local name:
>
> ```
> /^(?:[A-Za-z][^\0\t\n\f\r\u0020/>]*|[:_\u0080-\u{10FFFF}][A-Za-z0-9-.:_\u0080-\u{10FFFF}]*)$/u
> ```

A string is a [**valid doctype name**](https://dom.spec.whatwg.org/#valid-doctype-name) if it does not contain ASCII whitespace, U+0000 NULL, or U+003E (\>).

> **Note:** The empty string is a valid doctype name.

- [ ] [validate and extract](https://dom.spec.whatwg.org/#validate-and-extract) — To validate and extract a namespace and qualifiedName, given a context:
  - [ ] 1. If namespace is the empty string, then set it to null.
  - [ ] 2. Let prefix be null.
  - [ ] 3. Let localName be qualifiedName.
  - [ ] 4. If qualifiedName contains a U+003A (:):
    - [ ] 1. Set prefix to the part of qualifiedName before the first U+003A (:).
    - [ ] 2. Set localName to the part of qualifiedName after the first U+003A (:).
    - [ ] 3. If prefix is not a valid namespace prefix, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 5. Assert: prefix is either null or a valid namespace prefix.
  - [ ] 6. If context is "`attribute`" and localName is not a valid attribute local name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 7. If context is "`element`" and localName is not a valid element local name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 8. If prefix is non-null and namespace is null, then throw a "`NamespaceError`" `DOMException`.
  - [ ] 9. If prefix is "`xml`" and namespace is not the XML namespace, then throw a "`NamespaceError`" `DOMException`.
  - [ ] 10. If either qualifiedName or prefix is "`xmlns`" and namespace is not the XMLNS namespace, then throw a "`NamespaceError`" `DOMException`.
  - [ ] 11. If namespace is the XMLNS namespace and neither qualifiedName nor prefix is "`xmlns`", then throw a "`NamespaceError`" `DOMException`.
  - [ ] 12. Return (namespace, prefix, localName).

> **Note:**
>
> Various APIs in this specification used to validate namespace prefixes, attribute local names, element local names, and doctype names more strictly. This was done in a way that aligned with various XML-related specifications. (Although not all rules from the those specifications were enforced.)
>
> This was found to be annoying for web developers, especially since it meant there were some names that could be created by the HTML parser, but not by DOM APIs. So, the validations have been loosened to just those described above.

## [2. Events](https://dom.spec.whatwg.org/#events)

### [2.1. Introduction to "DOM Events"](https://dom.spec.whatwg.org/#introduction-to-dom-events)

Throughout the web platform events are dispatched to objects to signal an occurrence, such as network activity or user interaction. These objects implement the `EventTarget` interface and can therefore add event listeners to observe events by calling `addEventListener()`:

```js
obj.addEventListener("load", imgFetched)

function imgFetched(ev) {
  // great success
  …
}
```

Event listeners can be removed by utilizing the `removeEventListener()` method, passing the same arguments.

Alternatively, event listeners can be removed by passing an `AbortSignal` to `addEventListener()` and calling `abort()` on the controller owning the signal.

Events are objects too and implement the `Event` interface (or a derived interface). In the example above ev is the event. ev is passed as an argument to the event listener’s callback (typically a JavaScript Function as shown above). Event listeners key off the event’s `type` attribute value ("`load`" in the above example). The event’s `target` attribute value returns the object to which the event was dispatched (obj above).

Although events are typically dispatched by the user agent as the result of user interaction or the completion of some task, applications can dispatch events themselves by using what are commonly known as synthetic events:

```js
// add an appropriate event listener
obj.addEventListener("cat", function(e) { process(e.detail) })

// create and dispatch the event
var event = new CustomEvent("cat", {"detail":{"hazcheeseburger":true}})
obj.dispatchEvent(event)
```

Apart from signaling, events are sometimes also used to let an application control what happens next in an operation. For instance as part of form submission an event whose `type` attribute value is "`submit`" is dispatched. If this event’s `preventDefault()` method is invoked, form submission will be terminated. Applications who wish to make use of this functionality through events dispatched by the application (synthetic events) can make use of the return value of the `dispatchEvent()` method:

```js
if(obj.dispatchEvent(event)) {
  // event was not canceled, time for some magic
  …
}
```

When an event is dispatched to an object that participates in a tree (e.g., an element), it can reach event listeners on that object’s ancestors too. Effectively, all the object’s inclusive ancestor event listeners whose capture is true are invoked, in tree order. And then, if event’s `bubbles` is true, all the object’s inclusive ancestor event listeners whose capture is false are invoked, now in reverse tree order.

Let’s look at an example of how events work in a tree:

```html
<!doctype html>
<html>
 <head>
  <title>Boring example</title>
 </head>
 <body>
  <p>Hello <span id=x>world</span>!</p>
  <script>
   function test(e) {
     debug(e.target, e.currentTarget, e.eventPhase)
   }
   document.addEventListener("hey", test, {capture: true})
   document.body.addEventListener("hey", test)
   var ev = new Event("hey", {bubbles:true})
   document.getElementById("x").dispatchEvent(ev)
  </script>
 </body>
</html>
```

The `debug` function will be invoked twice. Each time the event’s `target` attribute value will be the `span` element. The first time `currentTarget` attribute’s value will be the document, the second time the `body` element. `eventPhase` attribute’s value switches from `CAPTURING_PHASE` to `BUBBLING_PHASE`. If an event listener was registered for the `span` element, `eventPhase` attribute’s value would have been `AT_TARGET`.

### [2.2. Interface Event](https://dom.spec.whatwg.org/#interface-event)

```webidl
[Exposed=*]
interface Event {
  constructor(DOMString type, optional EventInit eventInitDict = {});

  readonly attribute DOMString type;
  readonly attribute EventTarget? target;
  readonly attribute EventTarget? srcElement; // legacy
  readonly attribute EventTarget? currentTarget;
  sequence<EventTarget> composedPath();

  const unsigned short NONE = 0;
  const unsigned short CAPTURING_PHASE = 1;
  const unsigned short AT_TARGET = 2;
  const unsigned short BUBBLING_PHASE = 3;
  readonly attribute unsigned short eventPhase;

  undefined stopPropagation();
           attribute boolean cancelBubble; // legacy alias of .stopPropagation()
  undefined stopImmediatePropagation();

  readonly attribute boolean bubbles;
  readonly attribute boolean cancelable;
           attribute boolean returnValue;  // legacy
  undefined preventDefault();
  readonly attribute boolean defaultPrevented;
  readonly attribute boolean composed;

  [LegacyUnforgeable] readonly attribute boolean isTrusted;
  readonly attribute DOMHighResTimeStamp timeStamp;

  undefined initEvent(DOMString type, optional boolean bubbles = false, optional boolean cancelable = false); // legacy
};

dictionary EventInit {
  boolean bubbles = false;
  boolean cancelable = false;
  boolean composed = false;
};
```

An `Event` object is simply named an [**event**](https://dom.spec.whatwg.org/#concept-event). It allows for signaling that something has occurred, e.g., that an image has completed downloading.

A [**potential event target**](https://dom.spec.whatwg.org/#potential-event-target) is null or an `EventTarget` object.

An event has an associated [**target**](https://dom.spec.whatwg.org/#event-target) (a potential event target). Unless stated otherwise it is null.

An event has an associated [**relatedTarget**](https://dom.spec.whatwg.org/#event-relatedtarget) (a potential event target). Unless stated otherwise it is null.

> **Note:** Other specifications use relatedTarget to define a `relatedTarget` attribute. \[UIEVENTS\]

An event has an associated [**touch target list**](https://dom.spec.whatwg.org/#event-touch-target-list) (a list of zero or more potential event targets). Unless stated otherwise it is the empty list.

> **Note:** The touch target list is for the exclusive use of defining the `TouchEvent` interface and related interfaces. \[TOUCH-EVENTS\]

An event has an associated [**path**](https://dom.spec.whatwg.org/#event-path). A path is a list of event path items. An [**event path item**](https://dom.spec.whatwg.org/#event-path-item) is a struct consisting of an [**invocation target**](https://dom.spec.whatwg.org/#event-path-invocation-target) (an `EventTarget` object), an [**invocation-target-in-shadow-tree**](https://dom.spec.whatwg.org/#event-path-invocation-target-in-shadow-tree) (a boolean), a [**shadow-adjusted target**](https://dom.spec.whatwg.org/#event-path-shadow-adjusted-target) (a potential event target), a [**relatedTarget**](https://dom.spec.whatwg.org/#event-path-relatedtarget) (a potential event target), a [**touch target list**](https://dom.spec.whatwg.org/#event-path-touch-target-list) (a list of potential event targets), a [**root-of-closed-tree**](https://dom.spec.whatwg.org/#event-path-root-of-closed-tree) (a boolean), and a [**slot-in-closed-tree**](https://dom.spec.whatwg.org/#event-path-slot-in-closed-tree) (a boolean). A path is initially the empty list.

> **For web developers:**
>
> - `event = new Event(type [, eventInitDict])`
>   Returns a new event whose `type` attribute value is set to type. The eventInitDict argument allows for setting the `bubbles` and `cancelable` attributes via object members of the same name.
> - `event.type`
>   Returns the type of event, e.g. "`click`", "`hashchange`", or "`submit`".
> - `event.target`
>   Returns the object to which event is dispatched (its target).
> - `event.currentTarget`
>   Returns the object whose event listener’s callback is currently being invoked.
> - `event.composedPath()`
>   Returns the invocation target objects of event’s path (objects on which listeners will be invoked), except for any nodes in shadow trees of which the shadow root’s mode is "`closed`" that are not reachable from event’s `currentTarget`.
> - `event.eventPhase`
>   Returns the event’s phase, which is one of `NONE`, `CAPTURING_PHASE`, `AT_TARGET`, and `BUBBLING_PHASE`.
> - `event.stopPropagation()`
>   When dispatched in a tree, invoking this method prevents event from reaching any objects other than the current object.
> - `event.stopImmediatePropagation()`
>   Invoking this method prevents event from reaching any registered event listeners after the current one finishes running and, when dispatched in a tree, also prevents event from reaching any other objects.
> - `event.bubbles`
>   Returns true or false depending on how event was initialized. True if event goes through its target’s ancestors in reverse tree order; otherwise false.
> - `event.cancelable`
>   Returns true or false depending on how event was initialized. Its return value does not always carry meaning, but true can indicate that part of the operation during which event was dispatched, can be canceled by invoking the `preventDefault()` method.
> - `event.preventDefault()`
>   If invoked when the `cancelable` attribute value is true, and while executing a listener for the event with `passive` set to false, signals to the operation that caused event to be dispatched that it needs to be canceled.
> - `event.defaultPrevented`
>   Returns true if `preventDefault()` was invoked successfully to indicate cancelation; otherwise false.
> - `event.composed`
>   Returns true or false depending on how event was initialized. True if event invokes listeners past a `ShadowRoot` node that is the root of its target; otherwise false.
> - `event.isTrusted`
>   Returns true if event was dispatched by the user agent, and false otherwise.
> - `event.timeStamp`
>   Returns the event’s timestamp as the number of milliseconds measured relative to the occurrence.

- [ ] [type](https://dom.spec.whatwg.org/#dom-event-type) — The `type` attribute must return the value it was initialized to. When an event is created the attribute must be initialized to the empty string.

- [ ] [target](https://dom.spec.whatwg.org/#dom-event-target) — The `target` getter steps are to return this’s target.

- [ ] [srcElement](https://dom.spec.whatwg.org/#dom-event-srcelement) — The `srcElement` getter steps are to return this’s target.

- [ ] [currentTarget](https://dom.spec.whatwg.org/#dom-event-currenttarget) — The `currentTarget` attribute must return the value it was initialized to. When an event is created the attribute must be initialized to null.

- [ ] [composedPath()](https://dom.spec.whatwg.org/#dom-event-composedpath) — The `composedPath()` method steps are:
  - [ ] 1. Let composedPath be an empty list.
  - [ ] 2. Let path be this’s path.
  - [ ] 3. If path is empty, then return composedPath.
  - [ ] 4. Let currentTarget be this’s `currentTarget` attribute value.
  - [ ] 5. Assert: currentTarget is an `EventTarget` object.
  - [ ] 6. Append currentTarget to composedPath.
  - [ ] 7. Let currentTargetIndex be 0.
  - [ ] 8. Let currentTargetHiddenSubtreeLevel be 0.
  - [ ] 9. Let index be path’s size − 1.
  - [ ] 10. While index is greater than or equal to 0:
    - [ ] 1. If path\[index\]'s root-of-closed-tree is true, then increase currentTargetHiddenSubtreeLevel by 1.
    - [ ] 2. If path\[index\]'s invocation target is currentTarget, then set currentTargetIndex to index and break.
    - [ ] 3. If path\[index\]'s slot-in-closed-tree is true, then decrease currentTargetHiddenSubtreeLevel by 1.
    - [ ] 4. Decrease index by 1.
  - [ ] 11. Let currentHiddenLevel and maxHiddenLevel be currentTargetHiddenSubtreeLevel.
  - [ ] 12. Set index to currentTargetIndex − 1.
  - [ ] 13. While index is greater than or equal to 0:
    - [ ] 1. If path\[index\]'s root-of-closed-tree is true, then increase currentHiddenLevel by 1.
    - [ ] 2. If currentHiddenLevel is less than or equal to maxHiddenLevel, then prepend path\[index\]'s invocation target to composedPath.
    - [ ] 3. If path\[index\]'s slot-in-closed-tree is true:
      - [ ] 1. Decrease currentHiddenLevel by 1.
      - [ ] 2. If currentHiddenLevel is less than maxHiddenLevel, then set maxHiddenLevel to currentHiddenLevel.
    - [ ] 4. Decrease index by 1.
  - [ ] 14. Set currentHiddenLevel and maxHiddenLevel to currentTargetHiddenSubtreeLevel.
  - [ ] 15. Set index to currentTargetIndex + 1.
  - [ ] 16. While index is less than path’s size:
    - [ ] 1. If path\[index\]'s slot-in-closed-tree is true, then increase currentHiddenLevel by 1.
    - [ ] 2. If currentHiddenLevel is less than or equal to maxHiddenLevel, then append path\[index\]'s invocation target to composedPath.
    - [ ] 3. If path\[index\]'s root-of-closed-tree is true:
      - [ ] 1. Decrease currentHiddenLevel by 1.
      - [ ] 2. If currentHiddenLevel is less than maxHiddenLevel, then set maxHiddenLevel to currentHiddenLevel.
    - [ ] 4. Increase index by 1.
  - [ ] 17. Return composedPath.

- [ ] [eventPhase](https://dom.spec.whatwg.org/#dom-event-eventphase) — The `eventPhase` attribute must return the value it was initialized to, which must be one of the following:

  - **[**NONE**](https://dom.spec.whatwg.org/#dom-event-none) (numeric value 0)**: Events not currently dispatched are in this phase.
  - **[**CAPTURING\_PHASE**](https://dom.spec.whatwg.org/#dom-event-capturing_phase) (numeric value 1)**: When an event is dispatched to an object that participates in a tree it will be in this phase before it reaches its target.
  - **[**AT\_TARGET**](https://dom.spec.whatwg.org/#dom-event-at_target) (numeric value 2)**: When an event is dispatched it will be in this phase on its target.
  - **[**BUBBLING\_PHASE**](https://dom.spec.whatwg.org/#dom-event-bubbling_phase) (numeric value 3)**: When an event is dispatched to an object that participates in a tree it will be in this phase after it reaches its target.

Initially the attribute must be initialized to `NONE`.

Each event has the following associated flags that are all initially unset:

- [**stop propagation flag**](https://dom.spec.whatwg.org/#stop-propagation-flag)
- [**stop immediate propagation flag**](https://dom.spec.whatwg.org/#stop-immediate-propagation-flag)
- [**canceled flag**](https://dom.spec.whatwg.org/#canceled-flag)
- [**in passive listener flag**](https://dom.spec.whatwg.org/#in-passive-listener-flag)
- [**composed flag**](https://dom.spec.whatwg.org/#composed-flag)
- [**initialized flag**](https://dom.spec.whatwg.org/#initialized-flag)
- [**dispatch flag**](https://dom.spec.whatwg.org/#dispatch-flag)

- [ ] [stopPropagation()](https://dom.spec.whatwg.org/#dom-event-stoppropagation) — The `stopPropagation()` method steps are to set this’s stop propagation flag.

- [ ] [cancelBubble](https://dom.spec.whatwg.org/#dom-event-cancelbubble) — The `cancelBubble` getter steps are to return true if this’s stop propagation flag is set; otherwise false.

- [ ] The `cancelBubble` setter steps are to set this’s stop propagation flag if the given value is true; otherwise do nothing.

- [ ] [stopImmediatePropagation()](https://dom.spec.whatwg.org/#dom-event-stopimmediatepropagation) — The `stopImmediatePropagation()` method steps are to set this’s stop propagation flag and this’s stop immediate propagation flag.

- [ ] [bubbles](https://dom.spec.whatwg.org/#dom-event-bubbles) — The `bubbles` and `cancelable` attributes must return the values they were initialized to.

To [**set the canceled flag**](https://dom.spec.whatwg.org/#set-the-canceled-flag), given an event event, if event’s `cancelable` attribute value is true and event’s in passive listener flag is unset, then set event’s canceled flag, and do nothing otherwise.

- [ ] [returnValue](https://dom.spec.whatwg.org/#dom-event-returnvalue) — The `returnValue` getter steps are to return false if this’s canceled flag is set; otherwise true.

- [ ] The `returnValue` setter steps are to set the canceled flag with this if the given value is false; otherwise do nothing.

- [ ] [preventDefault()](https://dom.spec.whatwg.org/#dom-event-preventdefault) — The `preventDefault()` method steps are to set the canceled flag with this.

> **Note:** There are scenarios where invoking `preventDefault()` has no effect. User agents are encouraged to log the precise cause in a developer console, to aid debugging.

- [ ] [defaultPrevented](https://dom.spec.whatwg.org/#dom-event-defaultprevented) — The `defaultPrevented` getter steps are to return true if this’s canceled flag is set; otherwise false.

- [ ] [composed](https://dom.spec.whatwg.org/#dom-event-composed) — The `composed` getter steps are to return true if this’s composed flag is set; otherwise false.

- [ ] [isTrusted](https://dom.spec.whatwg.org/#dom-event-istrusted) — The `isTrusted` attribute must return the value it was initialized to. When an event is created the attribute must be initialized to false.

> **Note:** `isTrusted` is a convenience that indicates whether an event is dispatched by the user agent (as opposed to using `dispatchEvent()`). The sole legacy exception is `click()`, which causes the user agent to dispatch an event whose `isTrusted` attribute is initialized to false.

- [ ] [timeStamp](https://dom.spec.whatwg.org/#dom-event-timestamp) — The `timeStamp` attribute must return the value it was initialized to.

- [ ] [initialize](https://dom.spec.whatwg.org/#concept-event-initialize) — To initialize an event, with type, bubbles, and cancelable, run these steps:
  - [ ] 1. Set event’s initialized flag.
  - [ ] 2. Unset event’s stop propagation flag, stop immediate propagation flag, and canceled flag.
  - [ ] 3. Set event’s `isTrusted` attribute to false.
  - [ ] 4. Set event’s target to null.
  - [ ] 5. Set event’s `type` attribute to type.
  - [ ] 6. Set event’s `bubbles` attribute to bubbles.
  - [ ] 7. Set event’s `cancelable` attribute to cancelable.

- [ ] [initEvent(type, bubbles, cancelable)](https://dom.spec.whatwg.org/#dom-event-initevent) — The `initEvent(type, bubbles, cancelable)` method steps are:
  - [ ] 1. If this’s dispatch flag is set, then return.
  - [ ] 2. Initialize this with type, bubbles, and cancelable.

> **Note:** `initEvent()` is redundant with event constructors and incapable of setting `composed`. It has to be supported for legacy content.

### [2.3. Legacy extensions to the Window interface](https://dom.spec.whatwg.org/#interface-window-extensions)

```webidl
partial interface Window {
  [Replaceable] readonly attribute (Event or undefined) event; // legacy
};
```

Each `Window` object has an associated [**current event**](https://dom.spec.whatwg.org/#window-current-event) (undefined or an `Event` object). Unless stated otherwise it is undefined.

- [ ] [event](https://dom.spec.whatwg.org/#dom-window-event) — The `event` getter steps are to return this’s current event.

> **Note:** Web developers are strongly encouraged to instead rely on the `Event` object passed to event listeners, as that will result in more portable code. This attribute is not available in workers or worklets, and is inaccurate for events dispatched in shadow trees.

### [2.4. Interface CustomEvent](https://dom.spec.whatwg.org/#interface-customevent)

```webidl
[Exposed=*]
interface CustomEvent : Event {
  constructor(DOMString type, optional CustomEventInit eventInitDict = {});

  readonly attribute any detail;

  undefined initCustomEvent(DOMString type, optional boolean bubbles = false, optional boolean cancelable = false, optional any detail = null); // legacy
};

dictionary CustomEventInit : EventInit {
  any detail = null;
};
```

Events using the `CustomEvent` interface can be used to carry custom data.

> **For web developers:**
>
> - `event = new CustomEvent(type [, eventInitDict])`
>   Works analogously to the constructor for `Event` except that the eventInitDict argument now allows for setting the `detail` attribute too.
> - `event.detail`
>   Returns any custom data event was created with. Typically used for synthetic events.

- [ ] [detail](https://dom.spec.whatwg.org/#dom-customevent-detail) — The `detail` attribute must return the value it was initialized to.

- [ ] [initCustomEvent(type, bubbles, cancelable, detail)](https://dom.spec.whatwg.org/#dom-customevent-initcustomevent) — The `initCustomEvent(type, bubbles, cancelable, detail)` method steps are:
  - [ ] 1. If this’s dispatch flag is set, then return.
  - [ ] 2. Initialize this with type, bubbles, and cancelable.
  - [ ] 3. Set this’s `detail` attribute to detail.

### [2.5. Constructing events](https://dom.spec.whatwg.org/#constructing-events)

Specifications may define [**event constructing steps**](https://dom.spec.whatwg.org/#concept-event-constructor-ext) for all or some events. The algorithm is passed an event event and an `EventInit` eventInitDict as indicated in the inner event creation steps.

> **Note:** This construct can be used by `Event` subclasses that have a more complex structure than a simple 1:1 mapping between their initializing dictionary members and IDL attributes.

- [ ] [constructor](https://dom.spec.whatwg.org/#concept-event-constructor) — When a constructor of the `Event` interface, or of an interface that inherits from the `Event` interface, is invoked, these steps must be run, given the arguments type and eventInitDict:
  - [ ] 1. Let event be the result of running the inner event creation steps with this interface, null, now, and eventInitDict.
  - [ ] 2. Initialize event’s `type` attribute to type.
  - [ ] 3. Return event.

- [ ] [create an event](https://dom.spec.whatwg.org/#concept-event-create) — To create an event using eventInterface, which must be either `Event` or an interface that inherits from it, and optionally given a realm realm, run these steps:
  - [ ] 1. If realm is not given, then set it to null.
  - [ ] 2. Let dictionary be the result of converting the JavaScript value undefined to the dictionary type accepted by eventInterface’s constructor. (This dictionary type will either be `EventInit` or a dictionary that inherits from it.)
  - [ ] 3. Let event be the result of running the inner event creation steps with eventInterface, realm, the time of the occurrence that the event is signaling, and dictionary.
  - [ ] 4. Initialize event’s `isTrusted` attribute to true.
  - [ ] 5. Return event.

> **Note:** Create an event is meant to be used by other specifications which need to separately create and dispatch events, instead of simply firing them. It ensures the event’s attributes are initialized to the correct defaults.

- [ ] [inner event creation steps](https://dom.spec.whatwg.org/#inner-event-creation-steps) — The inner event creation steps, given an eventInterface, realm, time, and dictionary, are as follows:
  - [ ] 1. Let event be the result of creating a new object using eventInterface. If realm is non-null, then use that realm; otherwise, use the default behavior defined in Web IDL.
  - [ ] 2. Set event’s initialized flag.
  - [ ] 3. Initialize event’s `timeStamp` attribute to the relative high resolution coarse time given time and event’s relevant global object.
  - [ ] 4. For each member → value of dictionary: if event has an attribute whose identifier is member, then initialize that attribute to value.
  - [ ] 5. Run the event constructing steps with event and dictionary.
  - [ ] 6. Return event.

### [2.6. Defining event interfaces](https://dom.spec.whatwg.org/#defining-event-interfaces)

In general, when defining a new interface that inherits from `Event` please always ask feedback from the WHATWG or the W3C WebApps WG community.

The `CustomEvent` interface can be used as starting point. However, do not introduce any `init*Event()` methods as they are redundant with constructors. Interfaces that inherit from the `Event` interface that have such a method only have it for historical reasons.

### [2.7. Interface EventTarget](https://dom.spec.whatwg.org/#interface-eventtarget)

```webidl
[Exposed=*]
interface EventTarget {
  constructor();

  undefined addEventListener(DOMString type, EventListener? callback, optional (AddEventListenerOptions or boolean) options = {});
  undefined removeEventListener(DOMString type, EventListener? callback, optional (EventListenerOptions or boolean) options = {});
  boolean dispatchEvent(Event event);
};

callback interface EventListener {
  undefined handleEvent(Event event);
};

dictionary EventListenerOptions {
  boolean capture = false;
};

dictionary AddEventListenerOptions : EventListenerOptions {
  boolean passive;
  boolean once = false;
  AbortSignal signal;
};
```

An `EventTarget` object represents a target to which an event can be dispatched when something has occurred.

Each `EventTarget` object has an associated [**event listener list**](https://dom.spec.whatwg.org/#eventtarget-event-listener-list) (a list of zero or more event listeners). It is initially the empty list.

An [**event listener**](https://dom.spec.whatwg.org/#concept-event-listener) can be used to observe a specific event and consists of:

- [**type**](https://dom.spec.whatwg.org/#event-listener-type) (a string)
- [**callback**](https://dom.spec.whatwg.org/#event-listener-callback) (null or an `EventListener` object)
- [**capture**](https://dom.spec.whatwg.org/#event-listener-capture) (a boolean, initially false)
- [**passive**](https://dom.spec.whatwg.org/#event-listener-passive) (null or a boolean, initially null)
- [**once**](https://dom.spec.whatwg.org/#event-listener-once) (a boolean, initially false)
- [**signal**](https://dom.spec.whatwg.org/#event-listener-signal) (null or an `AbortSignal` object)
- [**removed**](https://dom.spec.whatwg.org/#event-listener-removed) (a boolean for bookkeeping purposes, initially false)

> **Note:** Although callback is an `EventListener` object, an event listener is a broader concept as can be seen above.

Each `EventTarget` object also has an associated [**get the parent**](https://dom.spec.whatwg.org/#get-the-parent) algorithm, which takes an event event, and returns an `EventTarget` object. Unless specified otherwise it returns null.

> **Note:** Nodes, shadow roots, and documents override the get the parent algorithm.

Each `EventTarget` object can have an associated [**activation behavior**](https://dom.spec.whatwg.org/#eventtarget-activation-behavior) algorithm. The activation behavior algorithm is passed an event, as indicated in the dispatch algorithm.

> **Note:** This exists because user agents perform certain actions for certain `EventTarget` objects, e.g., the `area` element, in response to synthetic `MouseEvent` events whose `type` attribute is `click`. Web compatibility prevented it from being removed and it is now the enshrined way of defining an activation of something. \[HTML\]

Each `EventTarget` object that has activation behavior, can additionally have both (not either) a [**legacy-pre-activation behavior**](https://dom.spec.whatwg.org/#eventtarget-legacy-pre-activation-behavior) algorithm and a [**legacy-canceled-activation behavior**](https://dom.spec.whatwg.org/#eventtarget-legacy-canceled-activation-behavior) algorithm.

> **Note:** These algorithms only exist for checkbox and radio `input` elements and are not to be used for anything else. \[HTML\]

> **For web developers:**
>
> - `target = new EventTarget();`
>   Creates a new `EventTarget` object, which can be used by developers to dispatch and listen for events.
> - `target.addEventListener(type, callback [, options])`
>   Appends an event listener for events whose `type` attribute value is type. The callback argument sets the callback that will be invoked when the event is dispatched. The options argument sets listener-specific options. For compatibility this can be a boolean, in which case the method behaves exactly as if the value was specified as options’s `capture`. When set to true, options’s `capture` prevents callback from being invoked when the event’s `eventPhase` attribute value is `BUBBLING_PHASE`. When false (or not present), callback will not be invoked when event’s `eventPhase` attribute value is `CAPTURING_PHASE`. Either way, callback will be invoked if event’s `eventPhase` attribute value is `AT_TARGET`. When set to true, options’s `passive` indicates that the callback will not cancel the event by invoking `preventDefault()`. This is used to enable performance optimizations described in § 2.8 Observing event listeners. When set to true, options’s `once` indicates that the callback will only be invoked once after which the event listener will be removed. If an `AbortSignal` is passed for options’s `signal`, then the event listener will be removed when signal is aborted. The event listener is appended to target’s event listener list and is not appended if it has the same type, callback, and capture.
> - `target.removeEventListener(type, callback [, options])`
>   Removes the event listener in target’s event listener list with the same type, callback, and options.
> - `target.dispatchEvent(event)`
>   Dispatches a synthetic event event to target and returns true if either event’s `cancelable` attribute value is false or its `preventDefault()` method was not invoked; otherwise false.

- [ ] [flatten](https://dom.spec.whatwg.org/#concept-flatten-options) — To flatten options, run these steps:
  - [ ] 1. If options is a boolean, then return options.
  - [ ] 2. Return options\["`capture`"\].

- [ ] [flatten more](https://dom.spec.whatwg.org/#event-flatten-more) — To flatten more options, run these steps:
  - [ ] 1. Let capture be the result of flattening options.
  - [ ] 2. Let once be false.
  - [ ] 3. Let passive and signal be null.
  - [ ] 4. If options is a dictionary:
    - [ ] 1. Set once to options\["`once`"\].
    - [ ] 2. If options\["`passive`"\] exists, then set passive to options\["`passive`"\].
    - [ ] 3. If options\["`signal`"\] exists, then set signal to options\["`signal`"\].
  - [ ] 5. Return capture, passive, once, and signal.

- [ ] [new EventTarget()](https://dom.spec.whatwg.org/#dom-eventtarget-eventtarget) — The `new EventTarget()` constructor steps are to do nothing.

> **Note:** Because of the defaults stated elsewhere, the returned `EventTarget`’s get the parent algorithm will return null, and it will have no activation behavior, legacy-pre-activation behavior, or legacy-canceled-activation behavior.

> **Note:** In the future we could allow custom get the parent algorithms. If this would be useful, support issue #583. For now, all author-created `EventTarget`s do not participate in a tree structure.

- [ ] [default passive value](https://dom.spec.whatwg.org/#default-passive-value) — The default passive value, given an event type type and an `EventTarget` eventTarget, is determined as follows:
  - [ ] 1. Return true if all of the following are true: type is one of "`touchstart`", "`touchmove`", "`wheel`", or "`mousewheel`". \[TOUCH-EVENTS\] \[UIEVENTS\] eventTarget is a `Window` object, or is a node whose node document is eventTarget, or is a node whose node document’s document element is eventTarget, or is a node whose node document’s body element is eventTarget. \[HTML\]
  - [ ] 2. Return false.

- [ ] [add an event listener](https://dom.spec.whatwg.org/#add-an-event-listener) — To add an event listener, given an `EventTarget` object eventTarget and an event listener listener, run these steps:
  - [ ] 1. If eventTarget is a `ServiceWorkerGlobalScope` object, its service worker’s script resource’s has ever been evaluated flag is set, and listener’s type matches the `type` attribute value of any of the service worker events, then report a warning to the console that this might not give the expected results. \[SERVICE-WORKERS\]
  - [ ] 2. If listener’s signal is non-null and is aborted, then return.
  - [ ] 3. If listener’s callback is null, then return.
  - [ ] 4. If listener’s passive is null, then set it to the default passive value given listener’s type and eventTarget.
  - [ ] 5. If eventTarget’s event listener list does not contain an event listener whose type is listener’s type, callback is listener’s callback, and capture is listener’s capture, then append listener to eventTarget’s event listener list.
  - [ ] 6. If listener’s signal is non-null, then add the following abort steps to it:
    - [ ] 1. Remove an event listener with eventTarget and listener.

> **Note:** The add an event listener concept exists to ensure event handlers use the same code path. \[HTML\]

- [ ] [addEventListener(type, callback, options)](https://dom.spec.whatwg.org/#dom-eventtarget-addeventlistener) — The `addEventListener(type, callback, options)` method steps are:
  - [ ] 1. Let capture, passive, once, and signal be the result of flattening more options.
  - [ ] 2. Add an event listener with this and an event listener whose type is type, callback is callback, capture is capture, passive is passive, once is once, and signal is signal.

- [ ] [remove an event listener](https://dom.spec.whatwg.org/#remove-an-event-listener) — To remove an event listener, given an `EventTarget` object eventTarget and an event listener listener, run these steps:
  - [ ] 1. If eventTarget is a `ServiceWorkerGlobalScope` object and its service worker’s set of event types to handle contains listener’s type, then report a warning to the console that this might not give the expected results. \[SERVICE-WORKERS\]
  - [ ] 2. Set listener’s removed to true and remove listener from eventTarget’s event listener list.

> **Note:** HTML needs this to define event handlers. \[HTML\]

- [ ] [remove all event listeners](https://dom.spec.whatwg.org/#remove-all-event-listeners) — To remove all event listeners, given an `EventTarget` object eventTarget: for each listener of eventTarget’s event listener list: remove an event listener with eventTarget and listener.

> **Note:** HTML needs this to define `document.open()`. \[HTML\]

- [ ] [removeEventListener(type, callback, options)](https://dom.spec.whatwg.org/#dom-eventtarget-removeeventlistener) — The `removeEventListener(type, callback, options)` method steps are:
  - [ ] 1. Let capture be the result of flattening options.
  - [ ] 2. If this’s event listener list contains an event listener whose type is type, callback is callback, and capture is capture, then remove an event listener with this and that event listener.

> **Note:** The event listener list will not contain multiple event listeners with equal type, callback, and capture, as add an event listener prevents that.

- [ ] [dispatchEvent(event)](https://dom.spec.whatwg.org/#dom-eventtarget-dispatchevent) — The `dispatchEvent(event)` method steps are:
  - [ ] 1. If event’s dispatch flag is set, or if its initialized flag is not set, then throw an "`InvalidStateError`" `DOMException`.
  - [ ] 2. Initialize event’s `isTrusted` attribute to false.
  - [ ] 3. Return the result of dispatching event to this.

### [2.8. Observing event listeners](https://dom.spec.whatwg.org/#observing-event-listeners)

In general, developers do not expect the presence of an event listener to be observable. The impact of an event listener is determined by its callback. That is, a developer adding a no-op event listener would not expect it to have any side effects.

Unfortunately, some event APIs have been designed such that implementing them efficiently requires observing event listeners. This can make the presence of listeners observable in that even empty listeners can have a dramatic performance impact on the behavior of the application. For example, touch and wheel events which can be used to block asynchronous scrolling. In some cases this problem can be mitigated by specifying the event to be `cancelable` only when there is at least one non-`passive` listener. For example, non-`passive` `TouchEvent` listeners must block scrolling, but if all listeners are `passive` then scrolling can be allowed to start in parallel by making the `TouchEvent` uncancelable (so that calls to `preventDefault()` are ignored). So code dispatching an event is able to observe the absence of non-`passive` listeners, and use that to clear the `cancelable` property of the event being dispatched.

Ideally, any new event APIs are defined such that they do not need this property. (Use whatwg/dom for discussion.)

- [ ] [legacy-obtain service worker fetch event listener callbacks](https://dom.spec.whatwg.org/#legacy-obtain-service-worker-fetch-event-listener-callbacks) — To legacy-obtain service worker fetch event listener callbacks given a `ServiceWorkerGlobalScope` global, run these steps. They return a list of `EventListener` objects.
  - [ ] 1. Let callbacks be « ».
  - [ ] 2. For each listener of global’s event listener list: if listener’s type is "`fetch`" and listener’s callback is non-null, then append listener’s callback to callbacks.
  - [ ] 3. Return callbacks.

### [2.9. Dispatching events](https://dom.spec.whatwg.org/#dispatching-events)

- [ ] [dispatch](https://dom.spec.whatwg.org/#concept-event-dispatch) — To dispatch an event event to an `EventTarget` object target, with an optional legacy target override flag and an optional legacyOutputDidListenersThrowFlag:
  - [ ] 1. Set event’s dispatch flag.
  - [ ] 2. Let targetOverride be target, if legacy target override flag is not given, and target’s associated `Document` otherwise. \[HTML\]
  - [ ] 3. Let activationTarget be null.
  - [ ] 4. Let relatedTarget be the result of retargeting event’s relatedTarget against target.
  - [ ] 5. Let clearTargets be false.
  - [ ] 6. If target is not relatedTarget or target is event’s relatedTarget:
    - [ ] 1. Let touchTargets be a new list.
    - [ ] 2. For each touchTarget of event’s touch target list: append the result of retargeting touchTarget against target to touchTargets.
    - [ ] 3. Append to an event path with event, target, targetOverride, relatedTarget, touchTargets, and false.
    - [ ] 4. Let isActivationEvent be true, if event is a `MouseEvent` object and event’s `type` attribute is "`click`"; otherwise false.
    - [ ] 5. If isActivationEvent is true and target has activation behavior, then set activationTarget to target.
    - [ ] 6. Let slottable be target, if target is a slottable and is assigned, and null otherwise.
    - [ ] 7. Let slotInClosedTree be false.
    - [ ] 8. Let parent be the result of invoking target’s get the parent with event.
    - [ ] 9. While parent is non-null:
      - [ ] 1. If slottable is non-null:
        - [ ] 1. Assert: parent is a slot.
        - [ ] 2. Set slottable to null.
        - [ ] 3. If parent’s root is a shadow root whose mode is "`closed`", then set slotInClosedTree to true.
      - [ ] 2. If parent is a slottable and is assigned, then set slottable to parent.
      - [ ] 3. Let relatedTarget be the result of retargeting event’s relatedTarget against parent.
      - [ ] 4. Let touchTargets be a new list.
      - [ ] 5. For each touchTarget of event’s touch target list: append the result of retargeting touchTarget against parent to touchTargets.
      - [ ] 6. If parent is a `Window` object, or parent is a node and target’s root is a shadow-including inclusive ancestor of parent:
        - [ ] 1. If isActivationEvent is true, event’s `bubbles` attribute is true, activationTarget is null, and parent has activation behavior, then set activationTarget to parent.
        - [ ] 2. Append to an event path with event, parent, null, relatedTarget, touchTargets, and slotInClosedTree.
      - [ ] 7. Otherwise, if parent is relatedTarget, then set parent to null.
      - [ ] 8. Otherwise:
        - [ ] 1. Set target to parent.
        - [ ] 2. If isActivationEvent is true, activationTarget is null, and target has activation behavior, then set activationTarget to target.
        - [ ] 3. Append to an event path with event, parent, target, relatedTarget, touchTargets, and slotInClosedTree.
      - [ ] 9. If parent is non-null, then set parent to the result of invoking parent’s get the parent with event.
      - [ ] 10. Set slotInClosedTree to false.
    - [ ] 10. Let clearTargetsItem be the last event path item in event’s path whose shadow-adjusted target is non-null.
    - [ ] 11. If clearTargetsItem’s shadow-adjusted target, clearTargetsItem’s relatedTarget, or an `EventTarget` object in clearTargetsItem’s touch target list is a node whose root is a shadow root: set clearTargets to true.
    - [ ] 12. If activationTarget is non-null and activationTarget has legacy-pre-activation behavior, then run activationTarget’s legacy-pre-activation behavior.
    - [ ] 13. For each item of event’s path, in reverse order:
      - [ ] 1. If item’s shadow-adjusted target is non-null, then set event’s `eventPhase` attribute to `AT_TARGET`.
      - [ ] 2. Otherwise, set event’s `eventPhase` attribute to `CAPTURING_PHASE`.
      - [ ] 3. Invoke with item, event, "`capturing`", and legacyOutputDidListenersThrowFlag if given.
    - [ ] 14. For each item of event’s path:
      - [ ] 1. If item’s shadow-adjusted target is non-null, then set event’s `eventPhase` attribute to `AT_TARGET`.
      - [ ] 2. Otherwise:
        - [ ] 1. If event’s `bubbles` attribute is false, then continue.
        - [ ] 2. Set event’s `eventPhase` attribute to `BUBBLING_PHASE`.
      - [ ] 3. Invoke with item, event, "`bubbling`", and legacyOutputDidListenersThrowFlag if given.
  - [ ] 7. Set event’s `eventPhase` attribute to `NONE`.
  - [ ] 8. Set event’s `currentTarget` attribute to null.
  - [ ] 9. Set event’s path to the empty list.
  - [ ] 10. Unset event’s dispatch flag, stop propagation flag, and stop immediate propagation flag.
  - [ ] 11. If clearTargets is true:
    - [ ] 1. Set event’s target to null.
    - [ ] 2. Set event’s relatedTarget to null.
    - [ ] 3. Set event’s touch target list to the empty list.
  - [ ] 12. If activationTarget is non-null:
    - [ ] 1. If event’s canceled flag is unset, then run activationTarget’s activation behavior with event.
    - [ ] 2. Otherwise, if activationTarget has legacy-canceled-activation behavior, then run activationTarget’s legacy-canceled-activation behavior.
  - [ ] 13. Return false if event’s canceled flag is set; otherwise true.

- [ ] [append to an event path](https://dom.spec.whatwg.org/#concept-event-path-append) — To append to an event path, given an event event, an `EventTarget` object invocationTarget, a potential event target shadowAdjustedTarget, a potential event target relatedTarget, a list of potential event targets touchTargets, and a boolean slotInClosedTree:
  - [ ] 1. Let invocationTargetInShadowTree be false.
  - [ ] 2. If invocationTarget is a node and its root is a shadow root, then set invocationTargetInShadowTree to true.
  - [ ] 3. Let rootOfClosedTree be false.
  - [ ] 4. If invocationTarget is a shadow root whose mode is "`closed`", then set rootOfClosedTree to true.
  - [ ] 5. Append a new event path item to event’s path whose invocation target is invocationTarget, invocation-target-in-shadow-tree is invocationTargetInShadowTree, shadow-adjusted target is shadowAdjustedTarget, relatedTarget is relatedTarget, touch target list is touchTargets, root-of-closed-tree is rootOfClosedTree, and slot-in-closed-tree is slotInClosedTree.

- [ ] [invoke](https://dom.spec.whatwg.org/#concept-event-listener-invoke) — To invoke, given an event path item pathItem, an event event, a string phase, and an optional legacyOutputDidListenersThrowFlag:
  - [ ] 1. Let targetItem be pathItem.
  - [ ] 2. While targetItem’s shadow-adjusted target is null: set targetItem to the event path item preceding targetItem in event’s path.
  - [ ] 3. Set event’s target to targetItem’s shadow-adjusted target.
  - [ ] 4. Set event’s relatedTarget to pathItem’s relatedTarget.
  - [ ] 5. Set event’s touch target list to pathItem’s touch target list.
  - [ ] 6. If event’s stop propagation flag is set, then return.
  - [ ] 7. Initialize event’s `currentTarget` attribute to pathItem’s invocation target.
  - [ ] 8. Let listeners be a clone of event’s `currentTarget` attribute value’s event listener list.
  - [ ] 9. Let invocationTargetInShadowTree be pathItem’s invocation-target-in-shadow-tree.
  - [ ] 10. Let found be the result of running inner invoke with event, listeners, phase, invocationTargetInShadowTree, and legacyOutputDidListenersThrowFlag if given.
  - [ ] 11. If found is false and event’s `isTrusted` attribute is true:
    - [ ] 1. Let originalEventType be event’s `type` attribute value.
    - [ ] 2. If event’s `type` attribute value is a match for any of the strings in the first column in the following table, set event’s `type` attribute value to the string in the second column on the same row as the matching string, and return otherwise.

      | Event type | Legacy event type |
      |---|---|
      | "`animationend`" | "`webkitAnimationEnd`" |
      | "`animationiteration`" | "`webkitAnimationIteration`" |
      | "`animationstart`" | "`webkitAnimationStart`" |
      | "`transitionend`" | "`webkitTransitionEnd`" |

    - [ ] 3. Inner invoke with event, listeners, phase, invocationTargetInShadowTree, and legacyOutputDidListenersThrowFlag if given.
    - [ ] 4. Set event’s `type` attribute value to originalEventType.

- [ ] [inner invoke](https://dom.spec.whatwg.org/#concept-event-listener-inner-invoke) — To inner invoke, given an event event, a list of event listeners listeners, a string phase, a boolean invocationTargetInShadowTree, and an optional legacyOutputDidListenersThrowFlag:
  - [ ] 1. Let found be false.
  - [ ] 2. For each listener of listeners, whose removed is false:
    - [ ] 1. If event’s `type` attribute value is not listener’s type, then continue.
    - [ ] 2. Set found to true.
    - [ ] 3. If phase is "`capturing`" and listener’s capture is false, then continue.
    - [ ] 4. If phase is "`bubbling`" and listener’s capture is true, then continue.
    - [ ] 5. If listener’s once is true, then remove an event listener given event’s `currentTarget` attribute value and listener.
    - [ ] 6. Let targetGlobal be event’s `currentTarget`’s node document’s relevant global object if event’s `currentTarget` is a node; otherwise event’s `currentTarget`’s relevant global object.
    - [ ] 7. Let listenerGlobal be listener callback’s associated realm’s global object.
    - [ ] 8. If targetGlobal or listenerGlobal is a `Window` object whose associated `Document` is not fully active, then continue.
    - [ ] 9. Let currentEvent be undefined.
    - [ ] 10. If listenerGlobal is a `Window` object:
      - [ ] 1. Set currentEvent to listenerGlobal’s current event.
      - [ ] 2. If invocationTargetInShadowTree is false, then set listenerGlobal’s current event to event.
    - [ ] 11. If listener’s passive is true, then set event’s in passive listener flag.
    - [ ] 12. If listenerGlobal is a `Window` object, then record timing info for event listener given event and listener.
    - [ ] 13. Call a user object’s operation with listener’s callback, "`handleEvent`", « event », and event’s `currentTarget` attribute value. If this throws an exception exception:
      - [ ] 1. Report exception for listener’s callback’s corresponding JavaScript object’s associated realm’s global object.
      - [ ] 2. Set legacyOutputDidListenersThrowFlag if given.
    - [ ] 14. Unset event’s in passive listener flag.
    - [ ] 15. If listenerGlobal is a `Window` object, then set listenerGlobal’s current event to currentEvent.
    - [ ] 16. If event’s stop immediate propagation flag is set, then break.
  - [ ] 3. Return found.

### [2.10. Firing events](https://dom.spec.whatwg.org/#firing-events)

- [ ] [fire an event](https://dom.spec.whatwg.org/#concept-event-fire) — To fire an event named e at target, optionally using an eventConstructor, with a description of how IDL attributes are to be initialized, and a legacy target override flag, run these steps:
  - [ ] 1. Let recordingTimingAsTask be the result of calling start recording task time if needed given target’s relevant settings object.
  - [ ] 2. If eventConstructor is not given, then let eventConstructor be `Event`.
  - [ ] 3. Let event be the result of creating an event given eventConstructor, in the relevant realm of target.
  - [ ] 4. Initialize event’s `type` attribute to e.
  - [ ] 5. Initialize any other IDL attributes of event as described in the invocation of this algorithm.
  - [ ] 6. Let result be the result of dispatching event at target, with legacy target override flag set if set.
  - [ ] 7. If recordingTimingAsTask is true:
    - [ ] 1. Assert: target’s relevant settings object is a `Document`.
    - [ ] 2. Record task end time given the unsafe shared current time and target’s relevant settings object.
  - [ ] 8. Return result.

> **Note:** Fire in the context of DOM is short for creating, initializing, and dispatching an event. Fire an event makes that process easier to write down.

> **Example:**
>
> If the event needs its `bubbles` or `cancelable` attribute initialized, one could write "fire an event named `submit` at target with its `cancelable` attribute initialized to true".
>
> Or, when a custom constructor is needed, "fire an event named `click` at target using `MouseEvent` with its `detail` attribute initialized to 1".
>
> Occasionally the return value is important:
>
> 1. Let doAction be the result of firing an event named `like` at target.
> 2. If doAction is true, then …

### [2.11. Action versus occurrence](https://dom.spec.whatwg.org/#action-versus-occurrence)

An event signifies an occurrence, not an action. Phrased differently, it represents a notification from an algorithm and can be used to influence the future course of that algorithm (e.g., through invoking `preventDefault()`). Events must not be used as actions or initiators that cause some algorithm to start running. That is not what they are for.

> **Note:** This is called out here specifically because previous iterations of the DOM had a concept of "default actions" associated with events that gave folks all the wrong ideas. Events do not represent or cause actions, they can only be used to influence an ongoing one.

## [3. Aborting ongoing activities](https://dom.spec.whatwg.org/#aborting-ongoing-activities)

Though promises do not have a built-in aborting mechanism, many APIs using them require abort semantics. `AbortController` is meant to support these requirements by providing an `abort()` method that toggles the state of a corresponding `AbortSignal` object. The API which wishes to support aborting can accept an `AbortSignal` object, and use its state to determine how to proceed.

APIs that rely upon `AbortController` are encouraged to respond to `abort()` by rejecting any unsettled promise with the `AbortSignal`’s abort reason.

> **Example:**
>
> A hypothetical `doAmazingness({... })` method could accept an `AbortSignal` object to support aborting as follows:
>
> ```
> const controller = new AbortController();
> const signal = controller.signal;
>
> startSpinner();
>
> doAmazingness({ ..., signal })
>   .then(result => ...)
>   .catch(err => {
>     if (err.name == 'AbortError') return;
>     showUserErrorMessage();
>   })
>   .then(() => stopSpinner());
>
> // …
>
> controller.abort();
> ```
>
> `doAmazingness` could be implemented as follows:
>
> ```
> function doAmazingness({signal}) {
>   return new Promise((resolve, reject) => {
>     signal.throwIfAborted();
>
>     // Begin doing amazingness, and call resolve(result) when done.
>     // But also, watch for signals:
>     signal.addEventListener('abort', () => {
>       // Stop doing amazingness, and:
>       reject(signal.reason);
>     });
>   });
> }
> ```

APIs that do not return promises can either react in an equivalent manner or opt to not surface the `AbortSignal`’s abort reason at all. `addEventListener()` is an example of an API where the latter made sense.

APIs that require more granular control could extend both `AbortController` and `AbortSignal` objects according to their needs.

### [3.1. Interface AbortController](https://dom.spec.whatwg.org/#interface-abortcontroller)

```webidl
[Exposed=*]
interface AbortController {
  constructor();

  [SameObject] readonly attribute AbortSignal signal;

  undefined abort(optional any reason);
};
```

> **For web developers:**
>
> - `controller = new AbortController()`
>   Returns a new controller whose `signal` is set to a newly created `AbortSignal` object.
> - `controller.signal`
>   Returns the `AbortSignal` object associated with this object.
> - `controller.abort(reason)`
>   Invoking this method will store reason in this object’s `AbortSignal`’s abort reason, and signal to any observers that the associated activity is to be aborted. If reason is undefined, then an "`AbortError`" `DOMException` will be stored.

An `AbortController` object has an associated [**signal**](https://dom.spec.whatwg.org/#abortcontroller-signal) (an `AbortSignal` object).

- [ ] [new AbortController()](https://dom.spec.whatwg.org/#dom-abortcontroller-abortcontroller) — The `new AbortController()` constructor steps are:
  - [ ] 1. Let signal be a new `AbortSignal` object.
  - [ ] 2. Set this’s signal to signal.

- [ ] [signal](https://dom.spec.whatwg.org/#dom-abortcontroller-signal) — The `signal` getter steps are to return this’s signal.

- [ ] [abort(reason)](https://dom.spec.whatwg.org/#dom-abortcontroller-abort) — The `abort(reason)` method steps are to signal abort on this with reason if it is given.

- [ ] [signal abort](https://dom.spec.whatwg.org/#abortcontroller-signal-abort) — To signal abort on an `AbortController` controller with an optional reason, signal abort on controller’s signal with reason if it is given.

### [3.2. Interface AbortSignal](https://dom.spec.whatwg.org/#interface-AbortSignal)

```webidl
[Exposed=*]
interface AbortSignal : EventTarget {
  [NewObject] static AbortSignal abort(optional any reason);
  [Exposed=(Window,Worker), NewObject] static AbortSignal timeout([EnforceRange] unsigned long long milliseconds);
  [NewObject] static AbortSignal _any(sequence<AbortSignal> signals);

  readonly attribute boolean aborted;
  readonly attribute any reason;
  undefined throwIfAborted();

  attribute EventHandler onabort;
};
```

> **For web developers:**
>
> - `AbortSignal.abort(reason)`
>   Returns an `AbortSignal` instance whose abort reason is set to reason if not undefined; otherwise to an "`AbortError`" `DOMException`.
> - `AbortSignal.any(signals)`
>   Returns an `AbortSignal` instance which will be aborted once any of signals is aborted. Its abort reason will be set to whichever one of signals caused it to be aborted.
> - `AbortSignal.timeout(milliseconds)`
>   Returns an `AbortSignal` instance which will be aborted in milliseconds milliseconds. Its abort reason will be set to a "`TimeoutError`" `DOMException`.
> - `signal.aborted`
>   Returns true if signal’s `AbortController` has signaled to abort; otherwise false.
> - `signal.reason`
>   Returns signal’s abort reason.
> - `signal.throwIfAborted()`
>   Throws signal’s abort reason, if signal’s `AbortController` has signaled to abort; otherwise, does nothing.

An `AbortSignal` object has an associated [**abort reason**](https://dom.spec.whatwg.org/#abortsignal-abort-reason) (a JavaScript value), which is initially undefined.

An `AbortSignal` object has associated [**abort algorithms**](https://dom.spec.whatwg.org/#abortsignal-abort-algorithms), (a set of algorithms which are to be executed when it is aborted), which is initially empty.

> **Note:** The abort algorithms enable APIs with complex requirements to react in a reasonable way to `abort()`. For example, a given API’s abort reason might need to be propagated to a cross-thread environment, such as a service worker.

An `AbortSignal` object has a [**dependent**](https://dom.spec.whatwg.org/#abortsignal-dependent) (a boolean), which is initially false.

An `AbortSignal` object has associated [**source signals**](https://dom.spec.whatwg.org/#abortsignal-source-signals) (a weak set of `AbortSignal` objects that the object is dependent on for its aborted state), which is initially empty.

An `AbortSignal` object has associated [**dependent signals**](https://dom.spec.whatwg.org/#abortsignal-dependent-signals) (a weak set of `AbortSignal` objects that are dependent on the object for their aborted state), which is initially empty.

- [ ] [abort(reason)](https://dom.spec.whatwg.org/#dom-abortsignal-abort) — The static `abort(reason)` method steps are:
  - [ ] 1. Let signal be a new `AbortSignal` object.
  - [ ] 2. Set signal’s abort reason to reason if it is given; otherwise to a new "`AbortError`" `DOMException`.
  - [ ] 3. Return signal.

- [ ] [timeout(milliseconds)](https://dom.spec.whatwg.org/#dom-abortsignal-timeout) — The static `timeout(milliseconds)` method steps are:
  - [ ] 1. Let signal be a new `AbortSignal` object.
  - [ ] 2. Let global be signal’s relevant global object.
  - [ ] 3. Run steps after a timeout given global, "`AbortSignal-timeout`", milliseconds, and the following step: For the duration of this timeout, if signal has any event listeners registered for its `abort` event, there must be a strong reference from global to signal.
    - [ ] 1. Queue a global task on the timer task source given global to signal abort given signal and a new "`TimeoutError`" `DOMException`.
  - [ ] 4. Return signal.

- [ ] [any(signals)](https://dom.spec.whatwg.org/#dom-abortsignal-any) — The static `any(signals)` method steps are to return the result of creating a dependent abort signal from signals using `AbortSignal` and the current realm.

- [ ] [aborted](https://dom.spec.whatwg.org/#dom-abortsignal-aborted) — The `aborted` getter steps are to return true if this is aborted; otherwise false.

- [ ] [reason](https://dom.spec.whatwg.org/#dom-abortsignal-reason) — The `reason` getter steps are to return this’s abort reason.

- [ ] [throwIfAborted()](https://dom.spec.whatwg.org/#dom-abortsignal-throwifaborted) — The `throwIfAborted()` method steps are to throw this’s abort reason, if this is aborted.

> **Example:**
>
> This method is primarily useful for when functions accepting `AbortSignal`s want to throw (or return a rejected promise) at specific checkpoints, instead of passing along the `AbortSignal` to other methods. For example, the following function allows aborting in between each attempt to poll for a condition. This gives opportunities to abort the polling process, even though the actual asynchronous operation (i.e., `await func()`) does not accept an `AbortSignal`.
>
> ```js
> async function waitForCondition(func, targetValue, { signal } = {}) {
>   while (true) {
>     signal?.throwIfAborted();
>
>     const result = await func();
>     if (result === targetValue) {
>       return;
>     }
>   }
> }
> ```

- [ ] [onabort](https://dom.spec.whatwg.org/#dom-abortsignal-onabort) — The `onabort` attribute is an event handler IDL attribute for the `onabort` event handler, whose event handler event type is `abort`.

> **Note:** Changes to an `AbortSignal` object represent the wishes of the corresponding `AbortController` object, but an API observing the `AbortSignal` object can choose to ignore them. For instance, if the operation has already completed.

An `AbortSignal` object is [**aborted**](https://dom.spec.whatwg.org/#abortsignal-aborted) when its abort reason is not undefined.

- [ ] [add](https://dom.spec.whatwg.org/#abortsignal-add) — To add an algorithm algorithm to an `AbortSignal` object signal:
  - [ ] 1. If signal is aborted, then return.
  - [ ] 2. Append algorithm to signal’s abort algorithms.

- [ ] [remove](https://dom.spec.whatwg.org/#abortsignal-remove) — To remove an algorithm algorithm from an `AbortSignal` signal, remove algorithm from signal’s abort algorithms.

- [ ] [signal abort](https://dom.spec.whatwg.org/#abortsignal-signal-abort) — To signal abort, given an `AbortSignal` object signal and an optional reason:
  - [ ] 1. If signal is aborted, then return.
  - [ ] 2. Set signal’s abort reason to reason if it is given; otherwise to a new "`AbortError`" `DOMException`.
  - [ ] 3. Let dependentSignalsToAbort be a new list.
  - [ ] 4. For each dependentSignal of signal’s dependent signals:
    - [ ] 1. If dependentSignal is not aborted:
      - [ ] 1. Set dependentSignal’s abort reason to signal’s abort reason.
      - [ ] 2. Append dependentSignal to dependentSignalsToAbort.
  - [ ] 5. Run the abort steps for signal.
  - [ ] 6. For each dependentSignal of dependentSignalsToAbort, run the abort steps for dependentSignal.

- [ ] [run the abort steps](https://dom.spec.whatwg.org/#run-the-abort-steps) — To run the abort steps for an `AbortSignal` signal:
  - [ ] 1. For each algorithm of signal’s abort algorithms: run algorithm.
  - [ ] 2. Empty signal’s abort algorithms.
  - [ ] 3. Fire an event named `abort` at signal.

- [ ] [create a dependent abort signal](https://dom.spec.whatwg.org/#create-a-dependent-abort-signal) — To create a dependent abort signal from a list of `AbortSignal` objects signals, using signalInterface, which must be either `AbortSignal` or an interface that inherits from it, and a realm:
  - [ ] 1. Let resultSignal be a new object implementing signalInterface using realm.
  - [ ] 2. For each signal of signals: if signal is aborted, then set resultSignal’s abort reason to signal’s abort reason and return resultSignal.
  - [ ] 3. Set resultSignal’s dependent to true.
  - [ ] 4. For each signal of signals:
    - [ ] 1. If signal’s dependent is false:
      - [ ] 1. Append signal to resultSignal’s source signals.
      - [ ] 2. Append resultSignal to signal’s dependent signals.
    - [ ] 2. Otherwise, for each sourceSignal of signal’s source signals:
      - [ ] 1. Assert: sourceSignal is not aborted and not dependent.
      - [ ] 2. Append sourceSignal to resultSignal’s source signals.
      - [ ] 3. Append resultSignal to sourceSignal’s dependent signals.
  - [ ] 5. Return resultSignal.

#### [3.2.1. Garbage collection](https://dom.spec.whatwg.org/#abort-signal-garbage-collection)

A non-aborted dependent `AbortSignal` object must not be garbage collected while its source signals is non-empty and it has registered event listeners for its `abort` event or its abort algorithms is non-empty.

### [3.3. Using AbortController and AbortSignal objects in APIs](https://dom.spec.whatwg.org/#abortcontroller-api-integration)

Any web platform API using promises to represent operations that can be aborted must adhere to the following:

- Accept `AbortSignal` objects through a `signal` dictionary member.
- Convey that the operation got aborted by rejecting the promise with `AbortSignal` object’s abort reason.
- Reject immediately if the `AbortSignal` is already aborted, otherwise:
- Use the abort algorithms mechanism to observe changes to the `AbortSignal` object and do so in a manner that does not lead to clashes with other observers.

> **Example:**
>
> The method steps for a promise-returning method `doAmazingness(options)` could be as follows:
>
> 1. Let global be this’s relevant global object.
> 2. Let p be a new promise.
> 3. If options\["`signal`"\] exists:
>    1. Let signal be options\["`signal`"\].
>    2. If signal is aborted, then reject p with signal’s abort reason and return p.
>    3. Add the following abort steps to signal:
>       1. Stop doing amazing things.
>       2. Reject p with signal’s abort reason.
> 4. Run these steps in parallel:
>    1. Let amazingResult be the result of doing some amazing things.
>    2. Queue a global task on the amazing task source given global to resolve p with amazingResult.
> 5. Return p.

APIs not using promises should still adhere to the above as much as possible.

## [4. Nodes](https://dom.spec.whatwg.org/#nodes)

### [4.1. Introduction to "The DOM"](https://dom.spec.whatwg.org/#introduction-to-the-dom)

In its original sense, "The DOM" is an API for accessing and manipulating documents (in particular, HTML and XML documents). In this specification, the term "document" is used for any markup-based resource, ranging from short static documents to long essays or reports with rich multimedia, as well as to fully-fledged interactive applications.

Each such document is represented as a node tree. Some of the nodes in a tree can have children, while others are always leaves.

To illustrate, consider this HTML document:

```html
<!DOCTYPE html>
<html class=e>
 <head><title>Aliens?</title></head>
 <body>Why yes.</body>
</html>
```

It is represented as follows:

- Document
   - Doctype: `html`
   - `Element`: `html` `class`="`e`"
      - `Element`: `head`
         - `Element`: `title`
            - `Text`: Aliens?
      - `Text`: ⏎␣
      - `Element`: `body`
         - `Text`: Why yes.⏎

Note that, due to the magic that is HTML parsing, not all ASCII whitespace were turned into `Text` nodes, but the general concept is clear. Markup goes in, a tree of nodes comes out.

> **Note:** The most excellent Live DOM Viewer can be used to explore this matter in more detail.

### [4.2. Node tree](https://dom.spec.whatwg.org/#node-trees)

[**Nodes**](https://dom.spec.whatwg.org/#concept-node) are objects that implement `Node`. Nodes participate in a tree, which is known as the [**node tree**](https://dom.spec.whatwg.org/#concept-node-tree).

> **Note:**
>
> In practice you deal with more specific objects.
>
> Objects that implement `Node` also implement an inherited interface: `Document`, `DocumentType`, `DocumentFragment`, `Element`, `CharacterData`, or `Attr`.
>
> Objects that implement `DocumentFragment` sometimes implement `ShadowRoot`.
>
> Objects that implement `Element` also typically implement an inherited interface, such as `HTMLAnchorElement`.
>
> Objects that implement `CharacterData` also implement an inherited interface: `Text`, `ProcessingInstruction`, or `Comment`.
>
> Objects that implement `Text` sometimes implement `CDATASection`.
>
> Thus, every node’s primary interface is one of: `Document`, `DocumentType`, `DocumentFragment`, `ShadowRoot`, `Element` or an inherited interface of `Element`, `Attr`, `Text`, `CDATASection`, `ProcessingInstruction`, or `Comment`.

For brevity, this specification refers to an object that implements `Node` and an inherited interface `NodeInterface`, as a `NodeInterface` node.

A node tree is constrained as follows, expressed as a relationship between a node and its potential children:

- **`Document`**

- [ ] In tree order:
  - [ ] 1. Zero or more `ProcessingInstruction` or `Comment` nodes.
  - [ ] 2. Optionally one `DocumentType` node.
  - [ ] 3. Zero or more `ProcessingInstruction` or `Comment` nodes.
  - [ ] 4. Optionally one `Element` node.
  - [ ] 5. Zero or more `ProcessingInstruction` or `Comment` nodes.
- **`DocumentFragment` `Element`**: Zero or more `Element` or `CharacterData` nodes.
- **`DocumentType` `CharacterData` `Attr`**: No children.

> **Note:** `Attr` nodes participate in a tree for historical reasons; they never have a (non-null) parent or any children and are therefore alone in a tree.

- [ ] [length](https://dom.spec.whatwg.org/#concept-node-length) — To determine the length of a node node, run these steps:
  - [ ] 1. If node is a `DocumentType` or `Attr` node, then return 0.
  - [ ] 2. If node is a `CharacterData` node, then return node’s data’s length.
  - [ ] 3. Return the number of node’s children.

A node is considered [**empty**](https://dom.spec.whatwg.org/#concept-node-empty) if its length is 0.

#### [4.2.1. Document tree](https://dom.spec.whatwg.org/#document-trees)

A [**document tree**](https://dom.spec.whatwg.org/#concept-document-tree) is a node tree whose root is a document.

The [**document element**](https://dom.spec.whatwg.org/#document-element) of a document is the element whose parent is that document, if it exists; otherwise null.

> **Note:** Per the node tree constraints, there can be only one such element.

A node is [**in a document tree**](https://dom.spec.whatwg.org/#in-a-document-tree) if its root is a document.

A node is [**in a document**](https://dom.spec.whatwg.org/#in-a-document) if it is in a document tree. The term in a document is no longer supposed to be used. It indicates that the standard using it has not been updated to account for shadow trees.

#### [4.2.2. Shadow tree](https://dom.spec.whatwg.org/#shadow-trees)

A [**shadow tree**](https://dom.spec.whatwg.org/#concept-shadow-tree) is a node tree whose root is a shadow root.

A shadow root is always attached to another node tree through its host. A shadow tree is therefore never alone. The node tree of a shadow root’s host is sometimes referred to as the [**light tree**](https://dom.spec.whatwg.org/#concept-light-tree).

> **Note:** A shadow tree’s corresponding light tree can be a shadow tree itself.

A node is [**connected**](https://dom.spec.whatwg.org/#connected) if its shadow-including root is a document.

##### [4.2.2.1. Slots](https://dom.spec.whatwg.org/#shadow-tree-slots)

A shadow tree contains zero or more elements that are [**slots**](https://dom.spec.whatwg.org/#concept-slot).

> **Note:** A slot can only be created through HTML’s `slot` element.

A slot has an associated [**name**](https://dom.spec.whatwg.org/#slot-name) (a string). Unless stated otherwise it is the empty string.

- [ ] Use these attribute change steps to update a slot’s name:
  - [ ] 1. If element is a slot, localName is `name`, and namespace is null:
    - [ ] 1. If value is oldValue, then return.
    - [ ] 2. If value is null and oldValue is the empty string, then return.
    - [ ] 3. If value is the empty string and oldValue is null, then return.
    - [ ] 4. If value is null or the empty string, then set element’s name to the empty string.
    - [ ] 5. Otherwise, set element’s name to value.
    - [ ] 6. Run assign slottables for a tree with element’s root.

> **Note:** The first slot in a shadow tree, in tree order, whose name is the empty string, is sometimes known as the "default slot".

A slot has an associated [**assigned nodes**](https://dom.spec.whatwg.org/#slot-assigned-nodes) (a list of slottables). Unless stated otherwise it is empty.

##### [4.2.2.2. Slottables](https://dom.spec.whatwg.org/#light-tree-slotables)

`Element` and `Text` nodes are [**slottables**](https://dom.spec.whatwg.org/#concept-slotable).

> **Note:** A slot can be a slottable.

A slottable has an associated [**name**](https://dom.spec.whatwg.org/#slotable-name) (a string). Unless stated otherwise it is the empty string.

- [ ] Use these attribute change steps to update a slottable’s name:
  - [ ] 1. If localName is `slot` and namespace is null:
    - [ ] 1. If value is oldValue, then return.
    - [ ] 2. If value is null and oldValue is the empty string, then return.
    - [ ] 3. If value is the empty string and oldValue is null, then return.
    - [ ] 4. If value is null or the empty string, then set element’s name to the empty string.
    - [ ] 5. Otherwise, set element’s name to value.
    - [ ] 6. If element is assigned, then run assign slottables for element’s assigned slot.
    - [ ] 7. Run assign a slot for element.

A slottable has an associated [**assigned slot**](https://dom.spec.whatwg.org/#slotable-assigned-slot) (null or a slot). Unless stated otherwise it is null. A slottable is [**assigned**](https://dom.spec.whatwg.org/#slotable-assigned) if its assigned slot is non-null.

A slottable has an associated [**manual slot assignment**](https://dom.spec.whatwg.org/#slottable-manual-slot-assignment) (null or a slot). Unless stated otherwise, it is null.

> **Note:** A slottable’s manual slot assignment can be implemented using a weak reference to the slot, because this variable is not directly accessible from script.

##### [4.2.2.3. Finding slots and slottables](https://dom.spec.whatwg.org/#finding-slots-and-slotables)

- [ ] [find a slot](https://dom.spec.whatwg.org/#find-a-slot) — To find a slot for a given slottable slottable and an optional boolean open (default false):
  - [ ] 1. If slottable’s parent is null, then return null.
  - [ ] 2. Let shadow be slottable’s parent’s shadow root.
  - [ ] 3. If shadow is null, then return null.
  - [ ] 4. If open is true and shadow’s mode is not "`open`", then return null.
  - [ ] 5. If shadow’s slot assignment is "`manual`", then return the slot in shadow’s descendants whose manually assigned nodes contains slottable, if any; otherwise null.
  - [ ] 6. Return the first slot in tree order in shadow’s descendants whose name is slottable’s name, if any; otherwise null.

- [ ] [find slottables](https://dom.spec.whatwg.org/#find-slotables) — To find slottables for a given slot slot:
  - [ ] 1. Let result be « ».
  - [ ] 2. Let root be slot’s root.
  - [ ] 3. If root is not a shadow root, then return result.
  - [ ] 4. Let host be root’s host.
  - [ ] 5. If root’s slot assignment is "`manual`":
    - [ ] 1. For each slottable slottable of slot’s manually assigned nodes, if slottable’s parent is host, append slottable to result.
  - [ ] 6. Otherwise, for each slottable child slottable of host, in tree order:
    - [ ] 1. Let foundSlot be the result of finding a slot given slottable.
    - [ ] 2. If foundSlot is slot, then append slottable to result.
  - [ ] 7. Return result.

- [ ] [find flattened slottables](https://dom.spec.whatwg.org/#find-flattened-slotables) — To find flattened slottables for a given slot slot:
  - [ ] 1. Let result be « ».
  - [ ] 2. If slot’s root is not a shadow root, then return result.
  - [ ] 3. Let slottables be the result of finding slottables given slot.
  - [ ] 4. If slottables is the empty list, then append each slottable child of slot, in tree order, to slottables.
  - [ ] 5. For each node of slottables:
    - [ ] 1. If node is a slot whose root is a shadow root:
      - [ ] 1. Let temporaryResult be the result of finding flattened slottables given node.
      - [ ] 2. Append each slottable in temporaryResult, in order, to result.
    - [ ] 2. Otherwise, append node to result.
  - [ ] 6. Return result.

##### [4.2.2.4. Assigning slottables and slots](https://dom.spec.whatwg.org/#assigning-slotables-and-slots)

- [ ] [assign slottables](https://dom.spec.whatwg.org/#assign-slotables) — To assign slottables for a slot slot:
  - [ ] 1. Let slottables be the result of finding slottables for slot.
  - [ ] 2. If slottables and slot’s assigned nodes are not identical, then run signal a slot change for slot.
  - [ ] 3. Set slot’s assigned nodes to slottables.
  - [ ] 4. For each slottable of slottables: set slottable’s assigned slot to slot.

- [ ] [assign slottables for a tree](https://dom.spec.whatwg.org/#assign-slotables-for-a-tree) — To assign slottables for a tree, given a node root, run assign slottables for each slot of root’s inclusive descendants, in tree order.

- [ ] [assign a slot](https://dom.spec.whatwg.org/#assign-a-slot) — To assign a slot, given a slottable slottable:
  - [ ] 1. Let slot be the result of finding a slot with slottable.
  - [ ] 2. If slot is non-null, then run assign slottables for slot.

##### [4.2.2.5. Signaling slot change](https://dom.spec.whatwg.org/#signaling-slot-change)

Each similar-origin window agent has [**signal slots**](https://dom.spec.whatwg.org/#signal-slot-list) (a set of slots), which is initially empty. \[HTML\]

- [ ] [signal a slot change](https://dom.spec.whatwg.org/#signal-a-slot-change) — To signal a slot change, for a slot slot:
  - [ ] 1. Append slot to slot’s relevant agent’s signal slots.
  - [ ] 2. Queue a mutation observer microtask.

#### [4.2.3. Mutation algorithms](https://dom.spec.whatwg.org/#mutation-algorithms)

- [ ] [ensure pre-insert validity](https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity) — To ensure pre-insert validity given a node node, a node parent, null or a node child, and a list of nodes childrenToExclude:
  - [ ] 1. If parent is not a `Document`, `DocumentFragment`, or `Element` node, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 2. If node is a host-including inclusive ancestor of parent, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 3. If child is non-null and its parent is not parent, then throw a "`NotFoundError`" `DOMException`.
  - [ ] 4. If node is not a `DocumentFragment`, `DocumentType`, `Element`, or `CharacterData` node, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 5. If parent is not a document:
    - [ ] 1. If node is a doctype, then throw a "`HierarchyRequestError`" `DOMException`.
    - [ ] 2. Return.
  - [ ] 6. If node is a `Text` node, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 7. If node is a `CharacterData` node, then return.
  - [ ] 8. If node is a `DocumentFragment` node:
    - [ ] 1. If node has more than one element child or has a `Text` node child, then throw a "`HierarchyRequestError`" `DOMException`.
    - [ ] 2. If node has no element child, then return.
  - [ ] 9. If node is a `DocumentFragment` or `Element` node:
    - [ ] 1. If any of the following are true: parent has an element child that childrenToExclude does not contain; child is non-null and a doctype is following child; or child is a doctype that childrenToExclude does not contain, then throw a "`HierarchyRequestError`" `DOMException`.
    - [ ] 2. Return.
  - [ ] 10. Assert: node is a doctype.
  - [ ] 11. If any of the following are true: parent has a doctype child that childrenToExclude does not contain; child is non-null and an element is preceding child; or child is null and parent has an element child that childrenToExclude does not contain, then throw a "`HierarchyRequestError`" `DOMException`.

- [ ] [pre-insert](https://dom.spec.whatwg.org/#concept-node-pre-insert) — To pre-insert a node node into a node parent before null or node child:
  - [ ] 1. Ensure pre-insert validity given node, parent, child, and « ».
  - [ ] 2. Let referenceChild be child.
  - [ ] 3. If referenceChild is node, then set referenceChild to node’s next sibling.
  - [ ] 4. Insert node into parent before referenceChild.
  - [ ] 5. Return node.

- [ ] [insertion steps](https://dom.spec.whatwg.org/#concept-node-insert-ext) — Specifications may define insertion steps for all or some nodes. The algorithm is passed insertedNode, as indicated in the insert algorithm below. These steps must not modify the node tree that insertedNode participates in, create browsing contexts, fire events, or otherwise execute JavaScript. These steps may queue tasks to do these things asynchronously, however.

> **Example:**
>
> While the insertion steps cannot execute JavaScript (among other things), they will indeed have script-observable consequences. Consider the below example:
>
> ```js
> const h1 = document.querySelector('h1');
>
> const fragment = new DocumentFragment();
> const script = fragment.appendChild(document.createElement('script'));
> const style = fragment.appendChild(document.createElement('style'));
>
> script.innerText= 'console.log(getComputedStyle(h1).color)'; // Logs 'rgb(255, 0, 0)'
> style.innerText = 'h1 {color: rgb(255, 0, 0);}';
>
> document.body.append(fragment);
> ```
>
> The script in the above example logs `'rgb(255, 0, 0)'` because the following happen in order:
>
> 1. The insert algorithm runs, which will insert the `script` and `style` elements in order.
>    1. The HTML Standard’s insertion steps run for the `script` element; they do nothing. \[HTML\]
>    2. The HTML Standard’s insertion steps run for the `style` element; they immediately apply its style rules to the document. \[HTML\]
>    3. The HTML Standard’s post-connection steps run for the `script` element; they run the script, which immediately observes the style rules that were applied in the above step. \[HTML\]

- [ ] [post-connection steps](https://dom.spec.whatwg.org/#concept-node-post-connection-ext) — Specifications may also define post-connection steps for all or some nodes. The algorithm is passed connectedNode, as indicated in the insert algorithm below.

> **Note:** The purpose of the post-connection steps is to provide an opportunity for nodes to perform any connection-related operations that modify the node tree that connectedNode participates in, create browsing contexts, or otherwise execute JavaScript. These steps allow a batch of nodes to be inserted atomically with respect to script, with all major side effects occurring after the batch insertions into the node tree is complete. This ensures that all pending node tree insertions completely finish before more insertions can occur.

- [ ] [children changed steps](https://dom.spec.whatwg.org/#concept-node-children-changed-ext) — Specifications may define children changed steps for all or some nodes. The algorithm is passed no argument and is called from insert, remove, and replace data.

- [ ] [insert](https://dom.spec.whatwg.org/#concept-node-insert) — To insert a node node into a node parent before null or a node child, with an optional boolean suppressObservers (default false):
  - [ ] 1. Let nodes be node’s children, if node is a `DocumentFragment` node; otherwise « node ».
  - [ ] 2. Let count be nodes’s size.
  - [ ] 3. If count is 0, then return.
  - [ ] 4. If node is a `DocumentFragment` node:
    - [ ] 1. Remove its children with suppressObservers set to true.
    - [ ] 2. Queue a tree mutation record for node with « », nodes, null, and null.
  - [ ] 5. If child is non-null:
    - [ ] 1. For each live range whose start node is parent and start offset is greater than child’s index: increase its start offset by count.
    - [ ] 2. For each live range whose end node is parent and end offset is greater than child’s index: increase its end offset by count.
  - [ ] 6. Let previousSibling be child’s previous sibling or parent’s last child if child is null.
  - [ ] 7. For each node in nodes, in tree order:
    - [ ] 1. Adopt node into parent’s node document.
    - [ ] 2. If child is null, then append node to parent’s children.
    - [ ] 3. Otherwise, insert node into parent’s children before child’s index.
    - [ ] 4. If parent is a shadow host whose shadow root’s slot assignment is "`named`" and node is a slottable, then assign a slot for node.
    - [ ] 5. If parent’s root is a shadow root, and parent is a slot whose assigned nodes is the empty list, then run signal a slot change for parent.
    - [ ] 6. Run assign slottables for a tree with node’s root.
    - [ ] 7. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including tree order:
      - [ ] 1. Run the insertion steps with inclusiveDescendant.
      - [ ] 2. If inclusiveDescendant is not connected, then continue.
      - [ ] 3. If inclusiveDescendant is an element and inclusiveDescendant’s custom element registry is non-null:
        - [ ] 1. If inclusiveDescendant’s custom element registry’s is scoped is true, then append inclusiveDescendant’s node document to inclusiveDescendant’s custom element registry’s scoped document set.
        - [ ] 2. If inclusiveDescendant is custom, then enqueue a custom element callback reaction with inclusiveDescendant, callback name "`connectedCallback`", and « ».
        - [ ] 3. Otherwise, try to upgrade inclusiveDescendant.
      - [ ] 4. Otherwise, if inclusiveDescendant is a shadow root, inclusiveDescendant’s custom element registry is non-null, and inclusiveDescendant’s custom element registry’s is scoped is true, then append inclusiveDescendant’s node document to inclusiveDescendant’s custom element registry’s scoped document set.
  - [ ] 8. If suppressObservers is false, then queue a tree mutation record for parent with nodes, « », previousSibling, and child.
  - [ ] 9. Run the children changed steps for parent.
  - [ ] 10. Let staticNodeList be a list of nodes, initially « ».
  - [ ] 11. For each node of nodes, in tree order:
    - [ ] 1. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including tree order: append inclusiveDescendant to staticNodeList.
  - [ ] 12. For each node of staticNodeList: if node is connected, then run the post-connection steps with node.

- [ ] [append](https://dom.spec.whatwg.org/#concept-node-append) — To append a node node to a node parent: pre-insert node into parent before null.

- [ ] [moving steps](https://dom.spec.whatwg.org/#concept-node-move-ext) — Specifications may define moving steps for all or some nodes. The algorithm is passed a node movedNode, a boolean isSubtreeRoot, and a node oldAncestor as indicated in the move algorithm below. Like the insertion steps, these steps must not modify the node tree that movedNode participates in, create browsing contexts, fire events, or otherwise execute JavaScript. These steps may queue tasks to do these things asynchronously, however.

- [ ] [move](https://dom.spec.whatwg.org/#move) — To move a node node into a node newParent before null or a node child:
  - [ ] 1. If newParent’s shadow-including root is not the same as node’s shadow-including root, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 2. If node is a host-including inclusive ancestor of newParent, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 3. If child is non-null and its parent is not newParent, then throw a "`NotFoundError`" `DOMException`.
  - [ ] 4. If node is not an `Element` or a `CharacterData` node, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 5. If node is a `Text` node and newParent is a document, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 6. If newParent is a document, node is an `Element` node, and either newParent has an element child, child is a doctype, or child is non-null and a doctype is following child then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 7. Let oldParent be node’s parent.
  - [ ] 8. Assert: oldParent is non-null.
  - [ ] 9. Run the live range pre-remove steps, given node.
  - [ ] 10. For each `NodeIterator` object iterator whose root’s node document is node’s node document: run the `NodeIterator` pre-remove steps given node and iterator.
  - [ ] 11. Let oldPreviousSibling be node’s previous sibling.
  - [ ] 12. Let oldNextSibling be node’s next sibling.
  - [ ] 13. Remove node from oldParent’s children.
  - [ ] 14. If node is assigned, then run assign slottables for node’s assigned slot.
  - [ ] 15. If oldParent’s root is a shadow root, and oldParent is a slot whose assigned nodes is empty, then run signal a slot change for oldParent.
  - [ ] 16. If node has an inclusive descendant that is a slot:
    - [ ] 1. Run assign slottables for a tree with oldParent’s root.
    - [ ] 2. Run assign slottables for a tree with node.
  - [ ] 17. Add transient registered observers given node and oldParent.
  - [ ] 18. If child is non-null:
    - [ ] 1. For each live range whose start node is newParent and start offset is greater than child’s index: increase its start offset by 1.
    - [ ] 2. For each live range whose end node is newParent and end offset is greater than child’s index: increase its end offset by 1.
  - [ ] 19. Let newPreviousSibling be child’s previous sibling if child is non-null, and newParent’s last child otherwise.
  - [ ] 20. If child is null, then append node to newParent’s children.
  - [ ] 21. Otherwise, insert node into newParent’s children before child’s index.
  - [ ] 22. If newParent is a shadow host whose shadow root’s slot assignment is "`named`" and node is a slottable, then assign a slot for node.
  - [ ] 23. If newParent’s root is a shadow root, and newParent is a slot whose assigned nodes is empty, then run signal a slot change for newParent.
  - [ ] 24. Run assign slottables for a tree with node’s root.
  - [ ] 25. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including tree order:
    - [ ] 1. Let isSubtreeRoot be true if inclusiveDescendant is node; otherwise false.
    - [ ] 2. Run the moving steps with inclusiveDescendant, isSubtreeRoot, and oldParent.
    - [ ] 3. If inclusiveDescendant is custom and newParent is connected, then enqueue a custom element callback reaction with inclusiveDescendant, callback name "`connectedMoveCallback`", and « ».
  - [ ] 26. Queue a tree mutation record for oldParent with « », « node », oldPreviousSibling, and oldNextSibling.
  - [ ] 27. Queue a tree mutation record for newParent with « node », « », newPreviousSibling, and child.

- [ ] [replace](https://dom.spec.whatwg.org/#concept-node-replace) — To replace a node child with a node node within a node parent:
  - [ ] 1. Ensure pre-insert validity given node, parent, child, and « child ».
  - [ ] 2. Let referenceChild be child’s next sibling.
  - [ ] 3. If referenceChild is node, then set referenceChild to node’s next sibling.
  - [ ] 4. Let previousSibling be child’s previous sibling.
  - [ ] 5. Let removedNodes be the empty set.
  - [ ] 6. Adopt node into parent’s node document.
  - [ ] 7. If child’s parent is non-null:
    - [ ] 1. Set removedNodes to « child ».
    - [ ] 2. Remove child with suppressObservers set to true.
  - [ ] 8. Let nodes be node’s children if node is a `DocumentFragment` node; otherwise « node ».
  - [ ] 9. Insert node into parent before referenceChild with suppressObservers set to true.
  - [ ] 10. Queue a tree mutation record for parent with nodes, removedNodes, previousSibling, and referenceChild.
  - [ ] 11. Return child.

- [ ] [replace all](https://dom.spec.whatwg.org/#concept-node-replace-all) — To replace all with a node or null node within a node parent:
  - [ ] 1. Let removedNodes be parent’s children.
  - [ ] 2. Let addedNodes be the empty set.
  - [ ] 3. If node is a `DocumentFragment` node, then set addedNodes to node’s children.
  - [ ] 4. Otherwise, if node is non-null, set addedNodes to « node ».
  - [ ] 5. Remove all parent’s children, in tree order, with suppressObservers set to true.
  - [ ] 6. If node is non-null, then insert node into parent before null with suppressObservers set to true.
  - [ ] 7. If either addedNodes or removedNodes is not empty, then queue a tree mutation record for parent with addedNodes, removedNodes, null, and null.

> **Note:** This algorithm does not make any checks with regards to the node tree constraints. Specification authors need to use it wisely.

- [ ] [pre-remove](https://dom.spec.whatwg.org/#concept-node-pre-remove) — To pre-remove a node child from a node parent:
  - [ ] 1. If child’s parent is not parent, then throw a "`NotFoundError`" `DOMException`.
  - [ ] 2. Remove child.
  - [ ] 3. Return child.

- [ ] [removing steps](https://dom.spec.whatwg.org/#concept-node-remove-ext) — Specifications may define removing steps for all or some nodes. The algorithm is passed a node removedNode, a boolean isSubtreeRoot, and a node oldAncestor as indicated in the remove algorithm below.

- [ ] [remove](https://dom.spec.whatwg.org/#concept-node-remove) — To remove a node node, with an optional boolean suppressObservers (default false):
  - [ ] 1. Let parent be node’s parent.
  - [ ] 2. Assert: parent is non-null.
  - [ ] 3. Run the live range pre-remove steps, given node.
  - [ ] 4. For each `NodeIterator` object iterator whose root’s node document is node’s node document: run the `NodeIterator` pre-remove steps given node and iterator.
  - [ ] 5. Let oldPreviousSibling be node’s previous sibling.
  - [ ] 6. Let oldNextSibling be node’s next sibling.
  - [ ] 7. Remove node from its parent’s children.
  - [ ] 8. If node is assigned, then run assign slottables for node’s assigned slot.
  - [ ] 9. If parent’s root is a shadow root, and parent is a slot whose assigned nodes is the empty list, then run signal a slot change for parent.
  - [ ] 10. If node has an inclusive descendant that is a slot:
    - [ ] 1. Run assign slottables for a tree with parent’s root.
    - [ ] 2. Run assign slottables for a tree with node.
  - [ ] 11. Run the removing steps with node, true, and parent.
  - [ ] 12. Let isParentConnected be parent’s connected.
  - [ ] 13. If node is custom and isParentConnected is true, then enqueue a custom element callback reaction with node, callback name "`disconnectedCallback`", and « ».
  - [ ] 14. For each shadow-including descendant descendant of node, in shadow-including tree order:
    - [ ] 1. Run the removing steps with descendant, false, and parent.
    - [ ] 2. If descendant is custom and isParentConnected is true, then enqueue a custom element callback reaction with descendant, callback name "`disconnectedCallback`", and « ».
  - [ ] 15. Add transient registered observers given node and parent.
  - [ ] 16. If suppressObservers is false, then queue a tree mutation record for parent with « », « node », oldPreviousSibling, and oldNextSibling.
  - [ ] 17. Run the children changed steps for parent.

#### [4.2.4. Mixin NonElementParentNode](https://dom.spec.whatwg.org/#interface-nonelementparentnode)

> **Note:** Web compatibility prevents the `getElementById()` method from being exposed on elements (and therefore on `ParentNode`).

```webidl
interface mixin NonElementParentNode {
  Element? getElementById(DOMString elementId);
};
Document includes NonElementParentNode;
DocumentFragment includes NonElementParentNode;
```

> **For web developers:**
>
> - `node.getElementById(elementId)`
>   Returns the first element within node’s descendants whose ID is elementId.

- [ ] [get an element by ID](https://dom.spec.whatwg.org/#get-an-element-by-id) — To get an element by ID, given a node node and a string elementId, return the first element, in tree order, within node’s descendants, whose ID is elementId; otherwise, if there is no such element, null.

- [ ] [getElementById(elementId)](https://dom.spec.whatwg.org/#dom-nonelementparentnode-getelementbyid) — The `getElementById(elementId)` method steps are to return the result of getting an element by ID given this and elementId.

#### [4.2.5. Mixin DocumentOrShadowRoot](https://dom.spec.whatwg.org/#mixin-documentorshadowroot)

```webidl
interface mixin DocumentOrShadowRoot {
  readonly attribute CustomElementRegistry? customElementRegistry;
};
Document includes DocumentOrShadowRoot;
ShadowRoot includes DocumentOrShadowRoot;
```

> **For web developers:**
>
> - `registry = documentOrShadowRoot.customElementRegistry`
>   Returns documentOrShadowRoot’s `CustomElementRegistry` object, if any; otherwise null.

- [ ] [customElementRegistry](https://dom.spec.whatwg.org/#dom-documentorshadowroot-customelementregistry) — The `customElementRegistry` getter steps are:
  - [ ] 1. If this is a document, then return this’s custom element registry.
  - [ ] 2. Assert: this is a `ShadowRoot` node.
  - [ ] 3. Return this’s custom element registry.

> **Note:** The `DocumentOrShadowRoot` mixin is also expected to be used by other standards that want to define APIs shared between documents and shadow roots.

#### [4.2.6. Mixin ParentNode](https://dom.spec.whatwg.org/#interface-parentnode)

- [ ] [convert nodes into a node](https://dom.spec.whatwg.org/#convert-nodes-into-a-node) — To convert nodes into a node, given a list of nodes and strings nodes, and document document:
  - [ ] 1. Replace each string in nodes with the result of creating a text node given document and the string.
  - [ ] 2. If nodes’s size is 1, then return nodes\[0\].
  - [ ] 3. Let fragment be the result of creating a document fragment given document.
  - [ ] 4. For each node of nodes: append node to fragment.
  - [ ] 5. Return fragment.

```webidl
interface mixin ParentNode {
  [SameObject] readonly attribute HTMLCollection children;
  readonly attribute Element? firstElementChild;
  readonly attribute Element? lastElementChild;
  readonly attribute unsigned long childElementCount;

  [CEReactions, Unscopable] undefined prepend((Node or DOMString)... nodes);
  [CEReactions, Unscopable] undefined append((Node or DOMString)... nodes);
  [CEReactions, Unscopable] undefined replaceChildren((Node or DOMString)... nodes);

  [CEReactions] undefined moveBefore(Node node, Node? child);

  Element? querySelector(DOMString selectors);
  [NewObject] NodeList querySelectorAll(DOMString selectors);
};
Document includes ParentNode;
DocumentFragment includes ParentNode;
Element includes ParentNode;
```

> **For web developers:**
>
> - `collection = node.children`
>   Returns the child elements.
> - `element = node.firstElementChild`
>   Returns the first child that is an element; otherwise null.
> - `element = node.lastElementChild`
>   Returns the last child that is an element; otherwise null.
> - `node.prepend(nodes)`
>   Inserts nodes before the first child of node, while replacing strings in nodes with equivalent `Text` nodes. Throws a "`HierarchyRequestError`" `DOMException` if the constraints of the node tree are violated.
> - `node.append(nodes)`
>   Inserts nodes after the last child of node, while replacing strings in nodes with equivalent `Text` nodes. Throws a "`HierarchyRequestError`" `DOMException` if the constraints of the node tree are violated.
> - `node.replaceChildren(nodes)`
>   Replace all children of node with nodes, while replacing strings in nodes with equivalent `Text` nodes. Throws a "`HierarchyRequestError`" `DOMException` if the constraints of the node tree are violated.
> - `node.moveBefore(movedNode, child)`
>   Moves, without first removing, movedNode into node after child if child is non-null; otherwise after the last child of node. This method preserves state associated with movedNode. Throws a "`HierarchyRequestError`" `DOMException` if the constraints of the node tree are violated, or the state associated with the moved node cannot be preserved.
> - `node.querySelector(selectors)`
>   Returns the first element that is a descendant of node that matches selectors.
> - `node.querySelectorAll(selectors)`
>   Returns all element descendants of node that match selectors.

- [ ] [children](https://dom.spec.whatwg.org/#dom-parentnode-children) — The `children` getter steps are to return an `HTMLCollection` collection rooted at this matching only element children.

- [ ] [firstElementChild](https://dom.spec.whatwg.org/#dom-parentnode-firstelementchild) — The `firstElementChild` getter steps are to return the first child that is an element; otherwise null.

- [ ] [lastElementChild](https://dom.spec.whatwg.org/#dom-parentnode-lastelementchild) — The `lastElementChild` getter steps are to return the last child that is an element; otherwise null.

- [ ] [childElementCount](https://dom.spec.whatwg.org/#dom-parentnode-childelementcount) — The `childElementCount` getter steps are to return the number of children of this that are elements.

- [ ] [prepend(nodes)](https://dom.spec.whatwg.org/#dom-parentnode-prepend) — The `prepend(nodes)` method steps are:
  - [ ] 1. Let node be the result of converting nodes into a node given nodes and this’s node document.
  - [ ] 2. Pre-insert node into this before this’s first child.

- [ ] [append(nodes)](https://dom.spec.whatwg.org/#dom-parentnode-append) — The `append(nodes)` method steps are:
  - [ ] 1. Let node be the result of converting nodes into a node given nodes and this’s node document.
  - [ ] 2. Append node to this.

- [ ] [replaceChildren(nodes)](https://dom.spec.whatwg.org/#dom-parentnode-replacechildren) — The `replaceChildren(nodes)` method steps are:
  - [ ] 1. Let node be the result of converting nodes into a node given nodes and this’s node document.
  - [ ] 2. Ensure pre-insert validity given node, this, null, and this’s children.
  - [ ] 3. Replace all with node within this.

- [ ] [moveBefore(node, child)](https://dom.spec.whatwg.org/#dom-parentnode-movebefore) — The `moveBefore(node, child)` method steps are:
  - [ ] 1. Let referenceChild be child.
  - [ ] 2. If referenceChild is node, then set referenceChild to node’s next sibling.
  - [ ] 3. Move node into this before referenceChild.

- [ ] [querySelector(selectors)](https://dom.spec.whatwg.org/#dom-parentnode-queryselector) — The `querySelector(selectors)` method steps are to return the first result of running scope-match a selectors string selectors against this, if the result is not an empty list; otherwise null.

- [ ] [querySelectorAll(selectors)](https://dom.spec.whatwg.org/#dom-parentnode-queryselectorall) — The `querySelectorAll(selectors)` method steps are to return the static result of running scope-match a selectors string selectors against this.

#### [4.2.7. Mixin NonDocumentTypeChildNode](https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode)

> **Note:** Web compatibility prevents the `previousElementSibling` and `nextElementSibling` attributes from being exposed on doctypes (and therefore on `ChildNode`).

```webidl
interface mixin NonDocumentTypeChildNode {
  readonly attribute Element? previousElementSibling;
  readonly attribute Element? nextElementSibling;
};
Element includes NonDocumentTypeChildNode;
CharacterData includes NonDocumentTypeChildNode;
```

> **For web developers:**
>
> - `element = node.previousElementSibling`
>   Returns the first preceding sibling that is an element; otherwise null.
> - `element = node.nextElementSibling`
>   Returns the first following sibling that is an element; otherwise null.

- [ ] [previousElementSibling](https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling) — The `previousElementSibling` getter steps are to return the first preceding sibling that is an element; otherwise null.

- [ ] [nextElementSibling](https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling) — The `nextElementSibling` getter steps are to return the first following sibling that is an element; otherwise null.

#### [4.2.8. Mixin ChildNode](https://dom.spec.whatwg.org/#interface-childnode)

```webidl
interface mixin ChildNode {
  [CEReactions, Unscopable] undefined before((Node or DOMString)... nodes);
  [CEReactions, Unscopable] undefined after((Node or DOMString)... nodes);
  [CEReactions, Unscopable] undefined replaceWith((Node or DOMString)... nodes);
  [CEReactions, Unscopable] undefined remove();
};
DocumentType includes ChildNode;
Element includes ChildNode;
CharacterData includes ChildNode;
```

> **For web developers:**
>
> - `node.before(...nodes)`
>   Inserts nodes just before node, while replacing strings in nodes with equivalent `Text` nodes. Throws a "`HierarchyRequestError`" `DOMException` if the constraints of the node tree are violated.
> - `node.after(...nodes)`
>   Inserts nodes just after node, while replacing strings in nodes with equivalent `Text` nodes. Throws a "`HierarchyRequestError`" `DOMException` if the constraints of the node tree are violated.
> - `node.replaceWith(...nodes)`
>   Replaces node with nodes, while replacing strings in nodes with equivalent `Text` nodes. Throws a "`HierarchyRequestError`" `DOMException` if the constraints of the node tree are violated.
> - `node.remove()`
>   Removes node.

- [ ] [before(nodes)](https://dom.spec.whatwg.org/#dom-childnode-before) — The `before(nodes)` method steps are:
  - [ ] 1. Let parent be this’s parent.
  - [ ] 2. If parent is null, then return.
  - [ ] 3. Let viablePreviousSibling be this’s first preceding sibling not in nodes; otherwise null.
  - [ ] 4. Let node be the result of converting nodes into a node, given nodes and this’s node document.
  - [ ] 5. If viablePreviousSibling is null, then set it to parent’s first child; otherwise to viablePreviousSibling’s next sibling.
  - [ ] 6. Pre-insert node into parent before viablePreviousSibling.

- [ ] [after(nodes)](https://dom.spec.whatwg.org/#dom-childnode-after) — The `after(nodes)` method steps are:
  - [ ] 1. Let parent be this’s parent.
  - [ ] 2. If parent is null, then return.
  - [ ] 3. Let viableNextSibling be this’s first following sibling not in nodes; otherwise null.
  - [ ] 4. Let node be the result of converting nodes into a node, given nodes and this’s node document.
  - [ ] 5. Pre-insert node into parent before viableNextSibling.

- [ ] [replaceWith(nodes)](https://dom.spec.whatwg.org/#dom-childnode-replacewith) — The `replaceWith(nodes)` method steps are:
  - [ ] 1. Let parent be this’s parent.
  - [ ] 2. If parent is null, then return.
  - [ ] 3. Let viableNextSibling be this’s first following sibling not in nodes; otherwise null.
  - [ ] 4. Let node be the result of converting nodes into a node, given nodes and this’s node document.
  - [ ] 5. If this’s parent is parent, replace this with node within parent.
  - [ ] 6. Otherwise, pre-insert node into parent before viableNextSibling.

- [ ] [remove()](https://dom.spec.whatwg.org/#dom-childnode-remove) — The `remove()` method steps are:
  - [ ] 1. If this’s parent is null, then return.
  - [ ] 2. Remove this.

#### [4.2.9. Mixin Slottable](https://dom.spec.whatwg.org/#mixin-slotable)

```webidl
interface mixin Slottable {
  readonly attribute HTMLSlotElement? assignedSlot;
};
Element includes Slottable;
Text includes Slottable;
```

- [ ] [assignedSlot](https://dom.spec.whatwg.org/#dom-slotable-assignedslot) — The `assignedSlot` getter steps are to return the result of find a slot given this and true.

#### [4.2.10. Old-style collections: NodeList and HTMLCollection](https://dom.spec.whatwg.org/#old-style-collections)

A [**collection**](https://dom.spec.whatwg.org/#concept-collection) is an object that represents a list of nodes. A collection can be either [**live**](https://dom.spec.whatwg.org/#concept-collection-live) or [**static**](https://dom.spec.whatwg.org/#concept-collection-static). Unless otherwise stated, a collection must be live.

If a collection is live, then the attributes and methods on that object must operate on the actual underlying data, not a snapshot of the data.

When a collection is created, a filter and a root are associated with it.

The collection then [**represents**](https://dom.spec.whatwg.org/#represented-by-the-collection) a view of the subtree rooted at the collection’s root, containing only nodes that match the given filter. The view is linear. In the absence of specific requirements to the contrary, the nodes within the collection must be sorted in tree order.

##### [4.2.10.1. Interface NodeList](https://dom.spec.whatwg.org/#interface-nodelist)

A `NodeList` object is a collection of nodes.

```webidl
[Exposed=Window]
interface NodeList {
  getter Node? item(unsigned long index);
  readonly attribute unsigned long length;
  iterable<Node>;
};
```

> **For web developers:**
>
> - collection. `length`
>   Returns the number of nodes in the collection.
> - element = collection. `item(index)` element = collection\[index\]
>   Returns the node with index index from the collection. The nodes are sorted in tree order.

The object’s supported property indices are the numbers in the range zero to one less than the number of nodes represented by the collection. If there are no such elements, then there are no supported property indices.

- [ ] [length](https://dom.spec.whatwg.org/#dom-nodelist-length) — The `length` attribute must return the number of nodes represented by the collection.

- [ ] [item(index)](https://dom.spec.whatwg.org/#dom-nodelist-item) — The `item(index)` method must return the indexth node in the collection. If there is no indexth node in the collection, then the method must return null.

##### [4.2.10.2. Interface HTMLCollection](https://dom.spec.whatwg.org/#interface-htmlcollection)

```webidl
[Exposed=Window, LegacyUnenumerableNamedProperties]
interface HTMLCollection {
  readonly attribute unsigned long length;
  getter Element? item(unsigned long index);
  getter Element? namedItem(DOMString name);
};
```

An `HTMLCollection` object is a collection of elements.

> **Note:** `HTMLCollection` is a historical artifact we cannot rid the web of. While developers are of course welcome to keep using it, new API standard designers ought not to use it (use `sequence<T>` in IDL instead).

> **For web developers:**
>
> - collection. `length`
>   Returns the number of elements in the collection.
> - element = collection. `item(index)` element = collection\[index\]
>   Returns the element with index index from the collection. The elements are sorted in tree order.
> - element = collection. `namedItem(name)` element = collection\[name\]
>   Returns the first element with ID or name name from the collection.

The object’s supported property indices are the numbers in the range zero to one less than the number of elements represented by the collection. If there are no such elements, then there are no supported property indices.

- [ ] [length](https://dom.spec.whatwg.org/#dom-htmlcollection-length) — The `length` getter steps are to return the number of nodes represented by the collection.

- [ ] [item(index)](https://dom.spec.whatwg.org/#dom-htmlcollection-item) — The `item(index)` method steps are to return the indexth element in the collection. If there is no indexth element in the collection, then the method must return null.

- [ ] The supported property names are the values from the list returned by these steps:
  - [ ] 1. Let result be an empty list.
  - [ ] 2. For each element represented by the collection, in tree order:
    - [ ] 1. If element has an ID which is not in result, append element’s ID to result.
    - [ ] 2. If element is in the HTML namespace and has a `name` attribute whose value is neither the empty string nor is in result, append element’s `name` attribute value to result.
  - [ ] 3. Return result.

- [ ] [namedItem(key)](https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key) — The `namedItem(key)` method steps are:
  - [ ] 1. If key is the empty string, return null.
  - [ ] 2. Return the first element in the collection for which at least one of the following is true: it has an ID which is key; it is in the HTML namespace and has a `name` attribute whose value is key; or null if there is no such element.

### [4.3. Mutation observers](https://dom.spec.whatwg.org/#mutation-observers)

Each similar-origin window agent has a [**mutation observer microtask queued**](https://dom.spec.whatwg.org/#mutation-observer-compound-microtask-queued-flag) (a boolean), which is initially false. \[HTML\]

Each similar-origin window agent also has [**pending mutation observers**](https://dom.spec.whatwg.org/#mutation-observer-list) (a set of zero or more `MutationObserver` objects), which is initially empty.

Each similar-origin window agent also has a [**mutation observer counter**](https://dom.spec.whatwg.org/#mutation-observer-counter) (a number), which is initially 0.

- [ ] [queue a mutation observer microtask](https://dom.spec.whatwg.org/#queue-a-mutation-observer-compound-microtask) — To queue a mutation observer microtask:
  - [ ] 1. If the surrounding agent’s mutation observer microtask queued is true, then return.
  - [ ] 2. Set the surrounding agent’s mutation observer microtask queued to true.
  - [ ] 3. Queue a microtask to notify mutation observers.

- [ ] [notify mutation observers](https://dom.spec.whatwg.org/#notify-mutation-observers) — To notify mutation observers:
  - [ ] 1. Set the surrounding agent’s mutation observer microtask queued to false.
  - [ ] 2. Let notifyList be the result of sorting the surrounding agent’s pending mutation observers in ascending order, with a being less than b if a’s creation index is less than b’s creation index.
  - [ ] 3. Empty the surrounding agent’s pending mutation observers.
  - [ ] 4. Let signalSet be a clone of the surrounding agent’s signal slots.
  - [ ] 5. Empty the surrounding agent’s signal slots.
  - [ ] 6. For each mo of notifyList:
    - [ ] 1. Let records be a clone of mo’s record queue.
    - [ ] 2. Empty mo’s record queue.
    - [ ] 3. Remove transient registered observers for mo.
    - [ ] 4. If records is not empty, then invoke mo’s callback with « records, mo » and "`report`", and with callback this value mo.
  - [ ] 7. For each slot of signalSet: fire an event named `slotchange`, with its `bubbles` attribute set to true, at slot.

Each node has a [**registered observer list**](https://dom.spec.whatwg.org/#registered-observer-list) (a list of zero or more registered observers), which is initially empty.

A [**registered observer**](https://dom.spec.whatwg.org/#registered-observer) consists of an [**observer**](https://dom.spec.whatwg.org/#registered-observer-observer) (a `MutationObserver` object) and [**options**](https://dom.spec.whatwg.org/#registered-observer-options) (a `MutationObserverInit` dictionary).

A [**transient registered observer**](https://dom.spec.whatwg.org/#transient-registered-observer) is a registered observer that also consists of a [**source**](https://dom.spec.whatwg.org/#transient-registered-observer-source) (a registered observer that is not a transient registered observer).

> **Note:** Transient registered observers are used to track mutations within a given node’s descendants after node has been removed or moved so they do not get lost when `subtree` is set to true on node’s former parent. They are removed the next time mutation observers are notified.

- [ ] [add transient registered observers](https://dom.spec.whatwg.org/#add-transient-registered-observers) — To add transient registered observers given a node node and a node parent:
  - [ ] 1. For each inclusive ancestor inclusiveAncestor of parent, and then for each registered of inclusiveAncestor’s registered observer list:
    - [ ] 1. If registered’s options\["`subtree`"\] is false, then continue.
    - [ ] 2. Let observer be registered’s observer.
    - [ ] 3. Let source be registered’s source if registered is a transient registered observer; otherwise registered.
    - [ ] 4. If node’s registered observer list contains a transient registered observer whose source is source, then continue.
    - [ ] 5. Append a new transient registered observer whose observer is observer, options is source’s options, and source is source to node’s registered observer list.
    - [ ] 6. Append a weak reference to node to observer’s transient node set.
    - [ ] 7. Append observer to the surrounding agent’s pending mutation observers.
    - [ ] 8. Queue a mutation observer microtask.

- [ ] [remove transient registered observers](https://dom.spec.whatwg.org/#remove-transient-registered-observers) — To remove transient registered observers for a `MutationObserver` object observer:
  - [ ] 1. For each node of observer’s transient node set: remove all transient registered observers whose observer is observer from node’s registered observer list.
  - [ ] 2. Empty observer’s transient node set.

#### [4.3.1. Interface MutationObserver](https://dom.spec.whatwg.org/#interface-mutationobserver)

```webidl
[Exposed=Window]
interface MutationObserver {
  constructor(MutationCallback callback);

  undefined observe(Node target, optional MutationObserverInit options = {});
  undefined disconnect();
  sequence<MutationRecord> takeRecords();
};

callback MutationCallback = undefined (sequence<MutationRecord> mutations, MutationObserver observer);

dictionary MutationObserverInit {
  boolean childList = false;
  boolean attributes;
  boolean characterData;
  boolean subtree = false;
  boolean attributeOldValue;
  boolean characterDataOldValue;
  sequence<DOMString> attributeFilter;
};
```

A `MutationObserver` object can be used to observe mutations to the tree of nodes.

Each `MutationObserver` object has these associated concepts:

- A [**callback**](https://dom.spec.whatwg.org/#concept-mo-callback) set on creation.
- A [**creation index**](https://dom.spec.whatwg.org/#mutationobserver-creation-index) (a number) set on creation.
- A [**node list**](https://dom.spec.whatwg.org/#mutationobserver-node-list) (a list of weak references to nodes), which is initially empty.
- A [**transient node set**](https://dom.spec.whatwg.org/#mutationobserver-transient-node-set) (a set of weak references to nodes), which is initially empty.
- A [**record queue**](https://dom.spec.whatwg.org/#concept-mo-queue) (a queue of zero or more `MutationRecord` objects), which is initially empty.

> **For web developers:**
>
> - `observer = new MutationObserver(callback)`
>   Constructs a `MutationObserver` object and sets its callback to callback. The callback is invoked with a list of `MutationRecord` objects as first argument and the constructed `MutationObserver` object as second argument. It is invoked after nodes registered with the `observe()` method, are mutated.
> - `observer.observe(target, options)`
>   Instructs the user agent to observe a given target (a node) and report any mutations based on the criteria given by options (an object). The options argument allows for setting mutation observation options via object members. These are the object members that can be used:
> - `observer.disconnect()`
>   Stops observer from observing any mutations. Until the `observe()` method is used again, observer’s callback will not be invoked.
> - `observer.takeRecords()`
>   Empties the record queue and returns what was in there.

- [ ] [new MutationObserver(callback)](https://dom.spec.whatwg.org/#dom-mutationobserver-mutationobserver) — The `new MutationObserver(callback)` constructor steps are:
  - [ ] 1. Set this’s callback to callback.
  - [ ] 2. Set this’s creation index to this’s relevant agent’s mutation observer counter.
  - [ ] 3. Increase this’s relevant agent’s mutation observer counter by 1.

- [ ] [observe(target, options)](https://dom.spec.whatwg.org/#dom-mutationobserver-observe) — The `observe(target, options)` method steps are:
  - [ ] 1. If either options\["`attributeOldValue`"\] or options\["`attributeFilter`"\] exists, and options\["`attributes`"\] does not exist, then set options\["`attributes`"\] to true.
  - [ ] 2. If options\["`characterDataOldValue`"\] exists and options\["`characterData`"\] does not exist, then set options\["`characterData`"\] to true.
  - [ ] 3. If none of options\["`childList`"\], options\["`attributes`"\], and options\["`characterData`"\] is true, then throw a `TypeError`.
  - [ ] 4. If options\["`attributeOldValue`"\] is true and options\["`attributes`"\] is false, then throw a `TypeError`.
  - [ ] 5. If options\["`attributeFilter`"\] exists and options\["`attributes`"\] is false, then throw a `TypeError`.
  - [ ] 6. If options\["`characterDataOldValue`"\] is true and options\["`characterData`"\] is false, then throw a `TypeError`.
  - [ ] 7. Let registered be the registered observer in target’s registered observer list that is not a transient registered observer and whose observer is this, if any; otherwise null.
  - [ ] 8. If registered is non-null:
    - [ ] 1. For each node of this’s transient node set: remove all transient registered observers whose source is registered from node’s registered observer list.
    - [ ] 2. Set registered’s options to options.
  - [ ] 9. Otherwise:
    - [ ] 1. Append a new registered observer whose observer is this and options is options to target’s registered observer list.
    - [ ] 2. Append a weak reference to target to this’s node list.

- [ ] [disconnect()](https://dom.spec.whatwg.org/#dom-mutationobserver-disconnect) — The `disconnect()` method steps are:
  - [ ] 1. For each node of this’s node list: remove any registered observer from node’s registered observer list for which this is the observer.
  - [ ] 2. Remove transient registered observers for this.
  - [ ] 3. Empty this’s node list.
  - [ ] 4. Empty this’s record queue.

- [ ] [takeRecords()](https://dom.spec.whatwg.org/#dom-mutationobserver-takerecords) — The `takeRecords()` method steps are:
  - [ ] 1. Let records be a clone of this’s record queue.
  - [ ] 2. Empty this’s record queue.
  - [ ] 3. Return records.

#### [4.3.2. Queuing a mutation record](https://dom.spec.whatwg.org/#queueing-a-mutation-record)

- [ ] [queue a mutation record](https://dom.spec.whatwg.org/#queue-a-mutation-record) — To queue a mutation record of type for target with name, namespace, oldValue, addedNodes, removedNodes, previousSibling, and nextSibling:
  - [ ] 1. Let interestedObservers be an empty map.
  - [ ] 2. Let nodes be the inclusive ancestors of target.
  - [ ] 3. For each node of nodes, and then for each registered of node’s registered observer list:
    - [ ] 1. Let options be registered’s options.
    - [ ] 2. If none of the following are true node is not target and options\["`subtree`"\] is false type is "`attributes`" and options\["`attributes`"\] either does not exist or is false type is "`attributes`", options\["`attributeFilter`"\] exists, and options\["`attributeFilter`"\] does not contain name or namespace is non-null type is "`characterData`" and options\["`characterData`"\] either does not exist or is false type is "`childList`" and options\["`childList`"\] is false then:
      - [ ] 1. Let mo be registered’s observer.
      - [ ] 2. If interestedObservers\[mo\] does not exist, then set interestedObservers\[mo\] to null.
      - [ ] 3. If either type is "`attributes`" and options\["`attributeOldValue`"\] is true, or type is "`characterData`" and options\["`characterDataOldValue`"\] is true, then set interestedObservers\[mo\] to oldValue.
  - [ ] 4. If interestedObservers is empty, then return.
  - [ ] 5. For each observer → mappedOldValue of interestedObservers:
    - [ ] 1. Let record be a new `MutationRecord` object with its `type` set to type, `target` set to target, `attributeName` set to name, `attributeNamespace` set to namespace, `oldValue` set to mappedOldValue, `addedNodes` set to addedNodes, `removedNodes` set to removedNodes, `previousSibling` set to previousSibling, and `nextSibling` set to nextSibling.
    - [ ] 2. Enqueue record to observer’s record queue.
    - [ ] 3. Append observer to the surrounding agent’s pending mutation observers.
  - [ ] 6. Queue a mutation observer microtask.

- [ ] [queue a tree mutation record](https://dom.spec.whatwg.org/#queue-a-tree-mutation-record) — To queue a tree mutation record for target with addedNodes, removedNodes, previousSibling, and nextSibling:
  - [ ] 1. Assert: either addedNodes or removedNodes is not empty.
  - [ ] 2. Queue a mutation record of "`childList`" for target with null, null, null, addedNodes, removedNodes, previousSibling, and nextSibling.

#### [4.3.3. Interface MutationRecord](https://dom.spec.whatwg.org/#interface-mutationrecord)

```webidl
[Exposed=Window]
interface MutationRecord {
  readonly attribute DOMString type;
  [SameObject] readonly attribute Node target;
  [SameObject] readonly attribute NodeList addedNodes;
  [SameObject] readonly attribute NodeList removedNodes;
  readonly attribute Node? previousSibling;
  readonly attribute Node? nextSibling;
  readonly attribute DOMString? attributeName;
  readonly attribute DOMString? attributeNamespace;
  readonly attribute DOMString? oldValue;
};
```

> **For web developers:**
>
> - `record.type`
>   Returns "`attributes`" if it was an attribute mutation. "`characterData`" if it was a mutation to a `CharacterData` node. And "`childList`" if it was a mutation to the tree of nodes.
> - `record.target`
>   Returns the node the mutation affected, depending on the `type`. For "`attributes`", it is the element whose attribute changed. For "`characterData`", it is the `CharacterData` node. For "`childList`", it is the node whose children changed.
> - `record.addedNodes` `record.removedNodes`
>   Return the nodes added and removed respectively.
> - `record.previousSibling` `record.nextSibling`
>   Return the previous and next sibling respectively of the added or removed nodes; otherwise null.
> - `record.attributeName`
>   Returns the local name of the changed attribute; otherwise null.
> - `record.attributeNamespace`
>   Returns the namespace of the changed attribute; otherwise null.
> - `record.oldValue`
>   The return value depends on `type`. For "`attributes`", it is the value of the changed attribute before the change. For "`characterData`", it is the data of the changed node before the change. For "`childList`", it is null.

- [ ] [type](https://dom.spec.whatwg.org/#dom-mutationrecord-type) — The `type`, `target`, `addedNodes`, `removedNodes`, `previousSibling`, `nextSibling`, `attributeName`, `attributeNamespace`, and `oldValue` attributes must return the values they were initialized to.

### [4.4. Interface Node](https://dom.spec.whatwg.org/#interface-node)

```webidl
[Exposed=Window]
interface Node : EventTarget {
  const unsigned short ELEMENT_NODE = 1;
  const unsigned short ATTRIBUTE_NODE = 2;
  const unsigned short TEXT_NODE = 3;
  const unsigned short CDATA_SECTION_NODE = 4;
  const unsigned short ENTITY_REFERENCE_NODE = 5; // legacy
  const unsigned short ENTITY_NODE = 6; // legacy
  const unsigned short PROCESSING_INSTRUCTION_NODE = 7;
  const unsigned short COMMENT_NODE = 8;
  const unsigned short DOCUMENT_NODE = 9;
  const unsigned short DOCUMENT_TYPE_NODE = 10;
  const unsigned short DOCUMENT_FRAGMENT_NODE = 11;
  const unsigned short NOTATION_NODE = 12; // legacy
  readonly attribute unsigned short nodeType;
  readonly attribute DOMString nodeName;

  readonly attribute USVString baseURI;

  readonly attribute boolean isConnected;
  readonly attribute Document? ownerDocument;
  Node getRootNode(optional GetRootNodeOptions options = {});
  readonly attribute Node? parentNode;
  readonly attribute Element? parentElement;
  boolean hasChildNodes();
  [SameObject] readonly attribute NodeList childNodes;
  readonly attribute Node? firstChild;
  readonly attribute Node? lastChild;
  readonly attribute Node? previousSibling;
  readonly attribute Node? nextSibling;

  [CEReactions] attribute DOMString? nodeValue;
  [CEReactions] attribute DOMString? textContent;
  [CEReactions] undefined normalize();

  [CEReactions, NewObject] Node cloneNode(optional boolean subtree = false);
  boolean isEqualNode(Node? otherNode);
  boolean isSameNode(Node? otherNode); // legacy alias of ===

  const unsigned short DOCUMENT_POSITION_DISCONNECTED = 0x01;
  const unsigned short DOCUMENT_POSITION_PRECEDING = 0x02;
  const unsigned short DOCUMENT_POSITION_FOLLOWING = 0x04;
  const unsigned short DOCUMENT_POSITION_CONTAINS = 0x08;
  const unsigned short DOCUMENT_POSITION_CONTAINED_BY = 0x10;
  const unsigned short DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20;
  unsigned short compareDocumentPosition(Node other);
  boolean contains(Node? other);

  DOMString? lookupPrefix(DOMString? namespace);
  DOMString? lookupNamespaceURI(DOMString? prefix);
  boolean isDefaultNamespace(DOMString? namespace);

  [CEReactions] Node insertBefore(Node node, Node? child);
  [CEReactions] Node appendChild(Node node);
  [CEReactions] Node replaceChild(Node node, Node child);
  [CEReactions] Node removeChild(Node child);
};

dictionary GetRootNodeOptions {
  boolean composed = false;
};
```

> **Note:** `Node` is an abstract interface that is used by all nodes. You cannot get a direct instance of it.

Each node has an associated [**node document**](https://dom.spec.whatwg.org/#concept-node-document), set upon creation, that is a document.

> **Note:** A node’s node document can be changed by the adopt algorithm.

A node’s get the parent algorithm, given an event, returns the node’s assigned slot, if node is assigned; otherwise node’s parent.

> **Note:** Each node also has a registered observer list.

- [ ] [create a node](https://dom.spec.whatwg.org/#create-a-node) — To create a node that implements an interface interface, given a document document:
  - [ ] 1. Assert: interface is neither `Document` nor an interface that inherits from `Document`.
  - [ ] 2. Return a new node that implements interface, in document’s relevant realm, with its node document set to document.

> **For web developers:**
>
> - `node.nodeType`
>   Returns a number appropriate for the type of node, as follows:
> - `node.nodeName`
>   Returns a string appropriate for the type of node, as follows:

- [ ] [nodeType](https://dom.spec.whatwg.org/#dom-node-nodetype) — The `nodeType` getter steps are to return the first matching statement, switching on the interface this implements:

  - **`Element`**: [**ELEMENT\_NODE**](https://dom.spec.whatwg.org/#dom-node-element_node) (1)
  - **`Attr`**: [**ATTRIBUTE\_NODE**](https://dom.spec.whatwg.org/#dom-node-attribute_node) (2);
  - **An exclusive `Text` node**: [**TEXT\_NODE**](https://dom.spec.whatwg.org/#dom-node-text_node) (3);
  - **`CDATASection`**: [**CDATA\_SECTION\_NODE**](https://dom.spec.whatwg.org/#dom-node-cdata_section_node) (4);
  - **`ProcessingInstruction`**: [**PROCESSING\_INSTRUCTION\_NODE**](https://dom.spec.whatwg.org/#dom-node-processing_instruction_node) (7);
  - **`Comment`**: [**COMMENT\_NODE**](https://dom.spec.whatwg.org/#dom-node-comment_node) (8);
  - **`Document`**: [**DOCUMENT\_NODE**](https://dom.spec.whatwg.org/#dom-node-document_node) (9);
  - **`DocumentType`**: [**DOCUMENT\_TYPE\_NODE**](https://dom.spec.whatwg.org/#dom-node-document_type_node) (10);
  - **`DocumentFragment`**: [**DOCUMENT\_FRAGMENT\_NODE**](https://dom.spec.whatwg.org/#dom-node-document_fragment_node) (11).

- [ ] [nodeName](https://dom.spec.whatwg.org/#dom-node-nodename) — The `nodeName` getter steps are to return the first matching statement, switching on the interface this implements:

  - **`Element`**: Its HTML-uppercased qualified name.
  - **`Attr`**: Its qualified name.
  - **An exclusive `Text` node**: "`#text`".
  - **`CDATASection`**: "`#cdata-section`".
  - **`ProcessingInstruction`**: Its target.
  - **`Comment`**: "`#comment`".
  - **`Document`**: "`#document`".
  - **`DocumentType`**: Its name.
  - **`DocumentFragment`**: "`#document-fragment`".

> **For web developers:**
>
> - `node.baseURI`
>   Returns node’s node document’s base URL.

- [ ] [baseURI](https://dom.spec.whatwg.org/#dom-node-baseuri) — The `baseURI` getter steps are:
  - [ ] 1. Let baseURL be the result of running this’s node document’s get the base URL algorithm.
  - [ ] 2. If baseURL is null, then set baseURL to this’s node document’s URL.
  - [ ] 3. Return baseURL, serialized.

> **For web developers:**
>
> - `node.isConnected`
>   Returns true if node is connected; otherwise false.
> - `node.ownerDocument`
>   Returns the node document. Returns null for documents.
> - `node.getRootNode()`
>   Returns node’s root.
> - `node.getRootNode({ composed:true })`
>   Returns node’s shadow-including root.
> - `node.parentNode`
>   Returns the parent.
> - `node.parentElement`
>   Returns the parent element.
> - `node.hasChildNodes()`
>   Returns whether node has children.
> - `node.childNodes`
>   Returns the children.
> - `node.firstChild`
>   Returns the first child.
> - `node.lastChild`
>   Returns the last child.
> - `node.previousSibling`
>   Returns the previous sibling.
> - `node.nextSibling`
>   Returns the next sibling.

- [ ] [isConnected](https://dom.spec.whatwg.org/#dom-node-isconnected) — The `isConnected` getter steps are to return true, if this is connected; otherwise false.

- [ ] [ownerDocument](https://dom.spec.whatwg.org/#dom-node-ownerdocument) — The `ownerDocument` getter steps are to return null, if this is a document; otherwise this’s node document.

> **Note:** The node document of a document is that document itself. All nodes have a node document at all times.

- [ ] [getRootNode(options)](https://dom.spec.whatwg.org/#dom-node-getrootnode) — The `getRootNode(options)` method steps are to return this’s shadow-including root if options\["`composed`"\] is true; otherwise this’s root.

- [ ] [parentNode](https://dom.spec.whatwg.org/#dom-node-parentnode) — The `parentNode` getter steps are to return this’s parent.

- [ ] [parentElement](https://dom.spec.whatwg.org/#dom-node-parentelement) — The `parentElement` getter steps are to return this’s parent element.

- [ ] [hasChildNodes()](https://dom.spec.whatwg.org/#dom-node-haschildnodes) — The `hasChildNodes()` method steps are to return true if this has children; otherwise false.

- [ ] [childNodes](https://dom.spec.whatwg.org/#dom-node-childnodes) — The `childNodes` getter steps are to return a `NodeList` rooted at this matching only children.

- [ ] [firstChild](https://dom.spec.whatwg.org/#dom-node-firstchild) — The `firstChild` getter steps are to return this’s first child.

- [ ] [lastChild](https://dom.spec.whatwg.org/#dom-node-lastchild) — The `lastChild` getter steps are to return this’s last child.

- [ ] [previousSibling](https://dom.spec.whatwg.org/#dom-node-previoussibling) — The `previousSibling` getter steps are to return this’s previous sibling.

- [ ] [nextSibling](https://dom.spec.whatwg.org/#dom-node-nextsibling) — The `nextSibling` getter steps are to return this’s next sibling.

- [ ] [nodeValue](https://dom.spec.whatwg.org/#dom-node-nodevalue) — The `nodeValue` getter steps are to return the following, switching on the interface this implements:

  - **`Attr`**: this’s value.
  - **`CharacterData`**: this’s data.
  - **Otherwise**: Null.

- [ ] The `nodeValue` setter steps are to, if the given value is null, act as if it was the empty string instead, and then do as described below, switching on the interface this implements:

  - **`Attr`**: Set an existing attribute value with this and the given value.
  - **`CharacterData`**: Replace data of this with 0, this’s length, and the given value.
  - **Otherwise**: Do nothing.

To [**get text content**](https://dom.spec.whatwg.org/#get-text-content) with a node node, return the following, switching on the interface node implements:

- **`DocumentFragment` `Element`**: The descendant text content of node.
- **`Attr`**: node’s value.
- **`CharacterData`**: node’s data.
- **Otherwise**: Null.

- [ ] [textContent](https://dom.spec.whatwg.org/#dom-node-textcontent) — The `textContent` getter steps are to return the result of running get text content with this.

- [ ] [string replace all](https://dom.spec.whatwg.org/#string-replace-all) — To string replace all with a string string within a node parent, run these steps:
  - [ ] 1. Let node be null.
  - [ ] 2. If string is not the empty string, then set node to the result of creating a text node given parent’s node document and string.
  - [ ] 3. Replace all with node within parent.

To [**set text content**](https://dom.spec.whatwg.org/#set-text-content) with a node node and a string value, do as defined below, switching on the interface node implements:

- **`DocumentFragment` `Element`**: String replace all with value within node.
- **`Attr`**: Set an existing attribute value with node and value.
- **`CharacterData`**: Replace data of node with 0, node’s length, and value.
- **Otherwise**: Do nothing.

- [ ] The `textContent` setter steps are to, if the given value is null, act as if it was the empty string instead, and then run set text content with this and the given value.

> **For web developers:**
>
> - `node.normalize()`
>   Removes empty exclusive `Text` nodes and concatenates the data of remaining contiguous exclusive `Text` nodes into the first of their nodes.

- [ ] [normalize()](https://dom.spec.whatwg.org/#dom-node-normalize) — The `normalize()` method steps are to run these steps for each descendant exclusive `Text` node node of this:
  - [ ] 1. Let length be node’s length.
  - [ ] 2. If length is zero, then remove node and continue with the next exclusive `Text` node, if any.
  - [ ] 3. Let data be the concatenation of the data of node’s contiguous exclusive `Text` nodes (excluding itself), in tree order.
  - [ ] 4. Replace data of node with length, 0, and data.
  - [ ] 5. Let currentNode be node’s next sibling.
  - [ ] 6. While currentNode is an exclusive `Text` node:
    - [ ] 1. For each live range whose start node is currentNode: add length to its start offset and set its start node to node.
    - [ ] 2. For each live range whose end node is currentNode: add length to its end offset and set its end node to node.
    - [ ] 3. For each live range whose start node is currentNode’s parent and start offset is currentNode’s index: set its start node to node and its start offset to length.
    - [ ] 4. For each live range whose end node is currentNode’s parent and end offset is currentNode’s index: set its end node to node and its end offset to length.
    - [ ] 5. Add currentNode’s length to length.
    - [ ] 6. Set currentNode to its next sibling.
  - [ ] 7. Remove node’s contiguous exclusive `Text` nodes (excluding itself), in tree order.

> **For web developers:**
>
> - `node.cloneNode([subtree = false])`
>   Returns a copy of node. If subtree is true, the copy also includes the node’s descendants.
> - `node.isEqualNode(otherNode)`
>   Returns whether node and otherNode have the same properties.

> **Note:** HTML defines cloning steps for several elements, such as `input`, `script`, and `template`. SVG ought to do the same for its `script` elements, but does not.

- [ ] [clone a node](https://dom.spec.whatwg.org/#concept-node-clone) — To clone a node given a node node and an optional document document (default node’s node document), boolean subtree (default false), node-or-null parent (default null), and null or a `CustomElementRegistry` object fallbackRegistry (default null):
  - [ ] 1. Assert: node is not a document or node is document.
  - [ ] 2. Let copy be the result of cloning a single node given node, document, and fallbackRegistry.
  - [ ] 3. Run any cloning steps defined for node in other applicable specifications and pass node, copy, and subtree as parameters.
  - [ ] 4. If parent is non-null, then append copy to parent.
  - [ ] 5. If subtree is true, then for each child of node’s children, in tree order: clone a node given child with document set to document, subtree set to subtree, parent set to copy, and fallbackRegistry set to fallbackRegistry.
  - [ ] 6. If node is an element, node is a shadow host, and node’s shadow root’s clonable is true:
    - [ ] 1. Assert: copy is not a shadow host.
    - [ ] 2. Let shadowRootRegistry be node’s shadow root’s custom element registry.
    - [ ] 3. If shadowRootRegistry is a global custom element registry, then set shadowRootRegistry to document’s custom element registry’s effective global custom element registry.
    - [ ] 4. Attach a shadow root with copy, node’s shadow root’s mode, node’s shadow root’s delegates focus, node’s shadow root’s serializable, node’s shadow root’s slot assignment, true, and shadowRootRegistry.
    - [ ] 5. Set copy’s shadow root’s declarative to node’s shadow root’s declarative.
    - [ ] 6. Set copy’s shadow root’s keep custom element registry null to node’s shadow root’s keep custom element registry null.
    - [ ] 7. For each child of node’s shadow root’s children, in tree order: clone a node given child with document set to document, subtree set to true, and parent set to copy’s shadow root.
  - [ ] 7. Return copy.

- [ ] [clone a single node](https://dom.spec.whatwg.org/#clone-a-single-node) — To clone a single node given a node node, document document, and null or a `CustomElementRegistry` object fallbackRegistry:
  - [ ] 1. Let copy be null.
  - [ ] 2. If node is an element:
    - [ ] 1. Let registry be node’s custom element registry.
    - [ ] 2. If registry is null, then set registry to fallbackRegistry.
    - [ ] 3. If registry is a global custom element registry, then set registry to document’s custom element registry’s effective global custom element registry.
    - [ ] 4. Set copy to the result of creating an element, given document, node’s local name, node’s namespace, node’s namespace prefix, node’s `is` value, false, and registry.
    - [ ] 5. For each attribute of node’s attribute list:
      - [ ] 1. Let copyAttribute be the result of cloning a single node given attribute, document, and null.
      - [ ] 2. Append copyAttribute to copy.
  - [ ] 3. Otherwise, if node is a document, set copy to the result of creating a document that implements the same interfaces as node, given document’s relevant realm.
  - [ ] 4. Otherwise, set copy to the result of creating a node that implements the same interfaces as node, given document.
  - [ ] 5. Fulfill these additional requirements, switching on the interface node implements: `Document` `DocumentType` Set copy’s name, public ID, and system ID to those of node. `Attr` Set copy’s namespace, namespace prefix, local name, and value to those of node. `Text` `Comment` Set copy’s data to that of node. `ProcessingInstruction` Set copy’s target and data to those of node. Otherwise Do nothing.
    - Case `Document`:
      - [ ] 1. Set copy’s encoding, content type, URL, origin, type, mode, and allow declarative shadow roots, to those of node.
      - [ ] 2. Set copy’s base URL override to the result of running node’s get the base URL.
      - [ ] 3. If node’s custom element registry’s is scoped is true, then set copy’s custom element registry to node’s custom element registry.
  - [ ] 6. Assert: copy is a node.
  - [ ] 7. If node is a document, then set document to copy.
  - [ ] 8. Set copy’s node document to document.
  - [ ] 9. Return copy.

- [ ] [cloneNode(subtree)](https://dom.spec.whatwg.org/#dom-node-clonenode) — The `cloneNode(subtree)` method steps are:
  - [ ] 1. If this is a shadow root, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 2. Return the result of cloning a node given this with subtree set to subtree.

- [ ] [equals](https://dom.spec.whatwg.org/#concept-node-equals) — A node A equals a node B if all of the following conditions are true:

- A and B implement the same interfaces.
- The following are equal, switching on the interface A implements:

   - **`DocumentType`**: Its name, public ID, and system ID.
   - **`Element`**: Its namespace, namespace prefix, local name, and its attribute list’s size.
   - **`Attr`**: Its namespace, local name, and value.
   - **`ProcessingInstruction`**: Its target and data.
   - **`Text` `Comment`**: Its data.
   - **Otherwise**: —
- If A is an element, each attribute in its attribute list equals an attribute in B’s attribute list.
- A and B have the same number of children.
- Each child of A equals the child of B at the identical index.

- [ ] [isEqualNode(otherNode)](https://dom.spec.whatwg.org/#dom-node-isequalnode) — The `isEqualNode(otherNode)` method steps are to return true if otherNode is non-null and this equals otherNode; otherwise false.

- [ ] [isSameNode(otherNode)](https://dom.spec.whatwg.org/#dom-node-issamenode) — The `isSameNode(otherNode)` method steps are to return true if otherNode is this; otherwise false.

> **For web developers:**
>
> - `node.compareDocumentPosition(other)`
>   Returns a bitmask indicating the position of other relative to node. These are the bits that can be set:
> - `node.contains(other)`
>   Returns true if other is an inclusive descendant of node; otherwise false.

These are the constants `compareDocumentPosition()` returns as mask:

- [**DOCUMENT\_POSITION\_DISCONNECTED**](https://dom.spec.whatwg.org/#dom-node-document_position_disconnected) (1);
- [**DOCUMENT\_POSITION\_PRECEDING**](https://dom.spec.whatwg.org/#dom-node-document_position_preceding) (2);
- [**DOCUMENT\_POSITION\_FOLLOWING**](https://dom.spec.whatwg.org/#dom-node-document_position_following) (4);
- [**DOCUMENT\_POSITION\_CONTAINS**](https://dom.spec.whatwg.org/#dom-node-document_position_contains) (8);
- [**DOCUMENT\_POSITION\_CONTAINED\_BY**](https://dom.spec.whatwg.org/#dom-node-document_position_contained_by) (16, 10 in hexadecimal);
- [**DOCUMENT\_POSITION\_IMPLEMENTATION\_SPECIFIC**](https://dom.spec.whatwg.org/#dom-node-document_position_implementation_specific) (32, 20 in hexadecimal).

- [ ] [compareDocumentPosition(other)](https://dom.spec.whatwg.org/#dom-node-comparedocumentposition) — The `compareDocumentPosition(other)` method steps are:
  - [ ] 1. If this is other, then return zero.
  - [ ] 2. Let node1 be other and node2 be this.
  - [ ] 3. Let attr1 and attr2 be null.
  - [ ] 4. If node1 is an attribute, then set attr1 to node1 and node1 to attr1’s element.
  - [ ] 5. If node2 is an attribute:
    - [ ] 1. Set attr2 to node2 and node2 to attr2’s element.
    - [ ] 2. If attr1 and node1 are non-null, and node2 is node1:
      - [ ] 1. For each attr of node2’s attribute list:
        - [ ] 1. If attr equals attr1, then return the result of adding `DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC` and `DOCUMENT_POSITION_PRECEDING`.
        - [ ] 2. If attr equals attr2, then return the result of adding `DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC` and `DOCUMENT_POSITION_FOLLOWING`.
  - [ ] 6. If node1 or node2 is null, or node1’s root is not node2’s root, then return the result of adding `DOCUMENT_POSITION_DISCONNECTED`, `DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC`, and either `DOCUMENT_POSITION_PRECEDING` or `DOCUMENT_POSITION_FOLLOWING`, with the constraint that this is to be consistent, together.
  - [ ] 7. If node1 is an ancestor of node2 and attr1 is null, or node1 is node2 and attr2 is non-null, then return the result of adding `DOCUMENT_POSITION_CONTAINS` to `DOCUMENT_POSITION_PRECEDING`.
  - [ ] 8. If node1 is a descendant of node2 and attr2 is null, or node1 is node2 and attr1 is non-null, then return the result of adding `DOCUMENT_POSITION_CONTAINED_BY` to `DOCUMENT_POSITION_FOLLOWING`.
  - [ ] 9. If node1 is preceding node2, then return `DOCUMENT_POSITION_PRECEDING`.
  - [ ] 10. Return `DOCUMENT_POSITION_FOLLOWING`.

- [ ] [contains(other)](https://dom.spec.whatwg.org/#dom-node-contains) — The `contains(other)` method steps are to return true if other is an inclusive descendant of this; otherwise false (including when other is null).

- [ ] [locate a namespace prefix](https://dom.spec.whatwg.org/#locate-a-namespace-prefix) — To locate a namespace prefix for an element using namespace, run these steps:
  - [ ] 1. If element’s namespace is namespace and its namespace prefix is non-null, then return its namespace prefix.
  - [ ] 2. If element has an attribute whose namespace prefix is "`xmlns`" and value is namespace, then return element’s first such attribute’s local name.
  - [ ] 3. If element’s parent element is non-null, then return the result of running locate a namespace prefix on that element using namespace.
  - [ ] 4. Return null.

To [**locate a namespace**](https://dom.spec.whatwg.org/#locate-a-namespace) for a node using prefix, switch on the interface node implements:

- **`Element`**

- [ ] [locate a namespace](https://dom.spec.whatwg.org/#locate-a-namespace) — case `Element`:
  - [ ] 1. If prefix is "`xml`", then return the XML namespace.
  - [ ] 2. If prefix is "`xmlns`", then return the XMLNS namespace.
  - [ ] 3. If its namespace is non-null and its namespace prefix is prefix, then return namespace.
  - [ ] 4. If it has an attribute whose namespace is the XMLNS namespace, namespace prefix is "`xmlns`", and local name is prefix, or if prefix is null and it has an attribute whose namespace is the XMLNS namespace, namespace prefix is null, and local name is "`xmlns`", then return its value if it is not the empty string, and null otherwise.
  - [ ] 5. If its parent element is null, then return null.
  - [ ] 6. Return the result of running locate a namespace on its parent element using prefix.
- **`Document`**

- [ ] [locate a namespace](https://dom.spec.whatwg.org/#locate-a-namespace) — case `Document`:
  - [ ] 1. If its document element is null, then return null.
  - [ ] 2. Return the result of running locate a namespace on its document element using prefix.
- **`DocumentType` `DocumentFragment`**: Return null.
- **`Attr`**

- [ ] [locate a namespace](https://dom.spec.whatwg.org/#locate-a-namespace) — case `Attr`:
  - [ ] 1. If its element is null, then return null.
  - [ ] 2. Return the result of running locate a namespace on its element using prefix.
- **Otherwise**

- [ ] [locate a namespace](https://dom.spec.whatwg.org/#locate-a-namespace) — case Otherwise:
  - [ ] 1. If its parent element is null, then return null.
  - [ ] 2. Return the result of running locate a namespace on its parent element using prefix.

- [ ] [lookupPrefix(namespace)](https://dom.spec.whatwg.org/#dom-node-lookupprefix) — The `lookupPrefix(namespace)` method steps are:
  - [ ] 1. If namespace is null or the empty string, then return null.
  - [ ] 2. Switch on the interface this implements: `Element` Return the result of locating a namespace prefix for this using namespace. `Document` `DocumentType` `DocumentFragment` Return null. `Attr` Otherwise
    - Case `Document`:
      - [ ] 1. If this’s document element is null, then return null.
      - [ ] 2. Return the result of locating a namespace prefix for this’s document element using namespace.
    - Case `Attr`:
      - [ ] 1. If this’s element is null, then return null.
      - [ ] 2. Return the result of locating a namespace prefix for this’s element using namespace.
    - Case Otherwise:
      - [ ] 1. If this’s parent element is null, then return null.
      - [ ] 2. Return the result of locating a namespace prefix for this’s parent element using namespace.

- [ ] [lookupNamespaceURI(prefix)](https://dom.spec.whatwg.org/#dom-node-lookupnamespaceuri) — The `lookupNamespaceURI(prefix)` method steps are:
  - [ ] 1. If prefix is the empty string, then set it to null.
  - [ ] 2. Return the result of running locate a namespace for this using prefix.

- [ ] [isDefaultNamespace(namespace)](https://dom.spec.whatwg.org/#dom-node-isdefaultnamespace) — The `isDefaultNamespace(namespace)` method steps are:
  - [ ] 1. If namespace is the empty string, then set it to null.
  - [ ] 2. Let defaultNamespace be the result of running locate a namespace for this using null.
  - [ ] 3. Return true if defaultNamespace is the same as namespace; otherwise false.

- [ ] [insertBefore(node, child)](https://dom.spec.whatwg.org/#dom-node-insertbefore) — The `insertBefore(node, child)` method steps are to return the result of pre-inserting node into this before child.

- [ ] [appendChild(node)](https://dom.spec.whatwg.org/#dom-node-appendchild) — The `appendChild(node)` method steps are to return the result of appending node to this.

- [ ] [replaceChild(node, child)](https://dom.spec.whatwg.org/#dom-node-replacechild) — The `replaceChild(node, child)` method steps are to return the result of replacing child with node within this.

- [ ] [removeChild(child)](https://dom.spec.whatwg.org/#dom-node-removechild) — The `removeChild(child)` method steps are to return the result of pre-removing child from this.

- [ ] [list of elements with qualified name qualifiedName](https://dom.spec.whatwg.org/#concept-getelementsbytagname) — The list of elements with qualified name qualifiedName for a node root is the `HTMLCollection` returned by the following algorithm:
  - [ ] 1. If qualifiedName is U+002A (\*), then return an `HTMLCollection` rooted at root, whose filter matches only descendant elements.
  - [ ] 2. Otherwise, if root’s node document is an HTML document, return an `HTMLCollection` rooted at root, whose filter matches the following descendant elements: Whose namespace is the HTML namespace and whose qualified name is qualifiedName, in ASCII lowercase. Whose namespace is not the HTML namespace and whose qualified name is qualifiedName.
  - [ ] 3. Otherwise, return an `HTMLCollection` rooted at root, whose filter matches descendant elements whose qualified name is qualifiedName.

When invoked with the same argument, and as long as root’s node document’s type has not changed, the same `HTMLCollection` object may be returned as returned by an earlier call.

- [ ] [list of elements with namespace namespace and local name localName](https://dom.spec.whatwg.org/#concept-getelementsbytagnamens) — The list of elements with namespace namespace and local name localName for a node root is the `HTMLCollection` returned by the following algorithm:
  - [ ] 1. If namespace is the empty string, then set it to null.
  - [ ] 2. If both namespace and localName are U+002A (\*), then return an `HTMLCollection` rooted at root, whose filter matches descendant elements.
  - [ ] 3. If namespace is U+002A (\*), then return an `HTMLCollection` rooted at root, whose filter matches descendant elements whose local name is localName.
  - [ ] 4. If localName is U+002A (\*), then return an `HTMLCollection` rooted at root, whose filter matches descendant elements whose namespace is namespace.
  - [ ] 5. Return an `HTMLCollection` rooted at root, whose filter matches descendant elements whose namespace is namespace and local name is localName.

When invoked with the same arguments, the same `HTMLCollection` object may be returned as returned by an earlier call.

- [ ] [list of elements with class names classNames](https://dom.spec.whatwg.org/#concept-getelementsbyclassname) — The list of elements with class names classNames for a node root is the `HTMLCollection` returned by the following algorithm:
  - [ ] 1. Let classes be the result of running the ordered set parser on classNames.
  - [ ] 2. If classes is the empty set, return an empty `HTMLCollection`.
  - [ ] 3. Return an `HTMLCollection` rooted at root, whose filter matches descendant elements that have all their classes in classes. The comparisons for the classes must be done in an ASCII case-insensitive manner if root’s node document’s mode is "`quirks`"; otherwise in an identical to manner.

When invoked with the same argument, the same `HTMLCollection` object may be returned as returned by an earlier call.

### [4.5. Interface Document](https://dom.spec.whatwg.org/#interface-document)

```webidl
[Exposed=Window]
interface Document : Node {
  constructor();

  [SameObject] readonly attribute DOMImplementation implementation;
  readonly attribute USVString URL;
  readonly attribute USVString documentURI;
  readonly attribute DOMString compatMode;
  readonly attribute DOMString characterSet;
  readonly attribute DOMString charset; // legacy alias of .characterSet
  readonly attribute DOMString inputEncoding; // legacy alias of .characterSet
  readonly attribute DOMString contentType;

  readonly attribute DocumentType? doctype;
  readonly attribute Element? documentElement;
  HTMLCollection getElementsByTagName(DOMString qualifiedName);
  HTMLCollection getElementsByTagNameNS(DOMString? namespace, DOMString localName);
  HTMLCollection getElementsByClassName(DOMString classNames);

  [CEReactions, NewObject] Element createElement(DOMString localName, optional (DOMString or ElementCreationOptions) options = {});
  [CEReactions, NewObject] Element createElementNS(DOMString? namespace, DOMString qualifiedName, optional (DOMString or ElementCreationOptions) options = {});
  [NewObject] DocumentFragment createDocumentFragment();
  [NewObject] Text createTextNode(DOMString data);
  [NewObject] CDATASection createCDATASection(DOMString data);
  [NewObject] Comment createComment(DOMString data);
  [NewObject] ProcessingInstruction createProcessingInstruction(DOMString target, DOMString data);

  [CEReactions, NewObject] Node importNode(Node node, optional (boolean or ImportNodeOptions) options = false);
  [CEReactions] Node adoptNode(Node node);

  [NewObject] Attr createAttribute(DOMString localName);
  [NewObject] Attr createAttributeNS(DOMString? namespace, DOMString qualifiedName);

  [NewObject] Event createEvent(DOMString interface); // legacy

  [NewObject] Range createRange();

  // NodeFilter.SHOW_ALL = 0xFFFFFFFF
  [NewObject] NodeIterator createNodeIterator(Node root, optional unsigned long whatToShow = 0xFFFFFFFF, optional NodeFilter? filter = null);
  [NewObject] TreeWalker createTreeWalker(Node root, optional unsigned long whatToShow = 0xFFFFFFFF, optional NodeFilter? filter = null);
};

[Exposed=Window]
interface XMLDocument : Document {};

dictionary ElementCreationOptions {
  CustomElementRegistry? customElementRegistry;
  DOMString is;
};

dictionary ImportNodeOptions {
  CustomElementRegistry customElementRegistry;
  boolean selfOnly = false;
};
```

`Document` nodes are simply known as [**documents**](https://dom.spec.whatwg.org/#concept-document).

A document’s node document is itself.

Each document has an associated [**encoding**](https://dom.spec.whatwg.org/#concept-document-encoding) (an encoding), [**content type**](https://dom.spec.whatwg.org/#concept-document-content-type) (a string), [**URL**](https://dom.spec.whatwg.org/#concept-document-url) (a URL), [**origin**](https://dom.spec.whatwg.org/#concept-document-origin) (an origin), [**type**](https://dom.spec.whatwg.org/#concept-document-type) ("`xml`" or "`html`"), [**mode**](https://dom.spec.whatwg.org/#concept-document-mode) ("`no-quirks`", "`quirks`", or "`limited-quirks`"), [**allow declarative shadow roots**](https://dom.spec.whatwg.org/#document-allow-declarative-shadow-roots) (a boolean), and [**custom element registry**](https://dom.spec.whatwg.org/#document-custom-element-registry) (null or a `CustomElementRegistry` object). \[ENCODING\] \[URL\] \[HTML\]

Unless stated otherwise, a document’s encoding is the utf-8 encoding, content type is "`application/xml`", URL is "`about:blank`", origin is an opaque origin, type is "`xml`", mode is "`no-quirks`", allow declarative shadow roots is false, and custom element registry is null.

A document is said to be an [**XML document**](https://dom.spec.whatwg.org/#xml-document) if its type is "`xml`"; otherwise an [**HTML document**](https://dom.spec.whatwg.org/#html-document). Whether a document is an HTML document or an XML document affects the behavior of certain APIs.

Each document has an associated [**get the base URL**](https://dom.spec.whatwg.org/#get-the-base-url) algorithm, which returns a URL or null. Unless specified otherwise it returns null.

Each document has a [**base URL override**](https://dom.spec.whatwg.org/#document-base-url-override) (a URL or null), initially null.

> **Note:** The base URL override is set when cloning a document. HTML overrides the get the base URL algorithm to account for the `base` element and for the base URL "`about:`"-schemed documents inherit. \[HTML\]

A document is said to be in [**no-quirks mode**](https://dom.spec.whatwg.org/#concept-document-no-quirks) if its mode is "`no-quirks`", [**quirks mode**](https://dom.spec.whatwg.org/#concept-document-quirks) if its mode is "`quirks`", and [**limited-quirks mode**](https://dom.spec.whatwg.org/#concept-document-limited-quirks) if its mode is "`limited-quirks`".

> **Note:**
>
> The mode is only ever changed from the default for documents created by the HTML parser based on the presence, absence, or value of the DOCTYPE string, and by a new browsing context (initial "`about:blank`"). \[HTML\]
>
> No-quirks mode was originally known as "standards mode" and limited-quirks mode was once known as "almost standards mode". They have been renamed because their details are now defined by standards. (And because Ian Hickson vetoed their original names on the basis that they are nonsensical.)

A document’s get the parent algorithm, given an event, returns null if event’s `type` attribute value is "`load`" or document does not have a browsing context; otherwise the document’s relevant global object.

- [ ] [create a document](https://dom.spec.whatwg.org/#create-a-document) — To create a document that implements an interface interface, given a realm realm:
  - [ ] 1. Assert: interface is `Document` or an interface that inherits from `Document`.
  - [ ] 2. Return a new node that implements interface, in realm.

> **For web developers:**
>
> - `document = new Document()`
>   Returns a new document.
> - `document.implementation`
>   Returns document’s `DOMImplementation` object.
> - `document.URL` `document.documentURI`
>   Returns document’s URL.
> - `document.compatMode`
>   Returns the string "`BackCompat`" if document’s mode is "`quirks`"; otherwise "`CSS1Compat`".
> - `document.characterSet`
>   Returns document’s encoding.
> - `document.contentType`
>   Returns document’s content type.

- [ ] [new Document()](https://dom.spec.whatwg.org/#dom-document-document) — The `new Document()` constructor steps are to set this’s origin to the origin of current global object’s associated `Document`. \[HTML\]

> **Note:** Unlike `createDocument()`, this constructor does not return an `XMLDocument` object, but a document (`Document` object).

- [ ] [implementation](https://dom.spec.whatwg.org/#dom-document-implementation) — The `implementation` getter steps are to return the `DOMImplementation` object that is associated with this.

- [ ] [URL](https://dom.spec.whatwg.org/#dom-document-url) — The `URL` and `documentURI` getter steps are to return this’s URL, serialized.

- [ ] [compatMode](https://dom.spec.whatwg.org/#dom-document-compatmode) — The `compatMode` getter steps are to return "`BackCompat`" if this’s mode is "`quirks`"; otherwise "`CSS1Compat`".

- [ ] [characterSet](https://dom.spec.whatwg.org/#dom-document-characterset) — The `characterSet`, `charset`, and `inputEncoding` getter steps are to return this’s encoding’s name.

- [ ] [contentType](https://dom.spec.whatwg.org/#dom-document-contenttype) — The `contentType` getter steps are to return this’s content type.

> **For web developers:**
>
> - document. `doctype`
>   Returns the doctype or null if there is none.
> - document. `documentElement`
>   Returns the document element.
> - `collection = document.getElementsByTagName(qualifiedName)`
>   If qualifiedName is "`*`" returns an `HTMLCollection` of all descendant elements. Otherwise, returns an `HTMLCollection` of all descendant elements whose qualified name is qualifiedName. (Matches case-insensitively against elements in the HTML namespace within an HTML document.)
> - `collection = document.getElementsByTagNameNS(namespace, localName)`
>   If namespace and localName are "`*`", returns an `HTMLCollection` of all descendant elements. If only namespace is "`*`", returns an `HTMLCollection` of all descendant elements whose local name is localName. If only localName is "`*`", returns an `HTMLCollection` of all descendant elements whose namespace is namespace. Otherwise, returns an `HTMLCollection` of all descendant elements whose namespace is namespace and local name is localName.
> - `collection = document.getElementsByClassName(classNames)` `collection = element.getElementsByClassName(classNames)`
>   Returns an `HTMLCollection` of the elements in the object on which the method was invoked (a document or an element) that have all the classes given by classNames. The classNames argument is interpreted as a space-separated list of classes.

- [ ] [doctype](https://dom.spec.whatwg.org/#dom-document-doctype) — The `doctype` getter steps are to return the child of this that is a doctype; otherwise null.

- [ ] [documentElement](https://dom.spec.whatwg.org/#dom-document-documentelement) — The `documentElement` getter steps are to return this’s document element.

- [ ] [getElementsByTagName(qualifiedName)](https://dom.spec.whatwg.org/#dom-document-getelementsbytagname) — The `getElementsByTagName(qualifiedName)` method steps are to return the list of elements with qualified name qualifiedName for this.

> **Note:** Thus, in an HTML document, `document.getElementsByTagName("FOO")` will match `<FOO>` elements that are not in the HTML namespace, and `<foo>` elements that are in the HTML namespace, but not `<FOO>` elements that are in the HTML namespace.

- [ ] [getElementsByTagNameNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-document-getelementsbytagnamens) — The `getElementsByTagNameNS(namespace, localName)` method steps are to return the list of elements with namespace namespace and local name localName for this.

- [ ] [getElementsByClassName(classNames)](https://dom.spec.whatwg.org/#dom-document-getelementsbyclassname) — The `getElementsByClassName(classNames)` method steps are to return the list of elements with class names classNames for this.

> **Example:**
>
> ```
> <div id="example">
>   <p id="p1" class="aaa bbb"/>
>   <p id="p2" class="aaa ccc"/>
>   <p id="p3" class="bbb ccc"/>
> </div>
> ```
>
> A call to `document.getElementById("example").getElementsByClassName("aaa")` would return an `HTMLCollection` with the two paragraphs `p1` and `p2` in it.
>
> A call to `getElementsByClassName("ccc bbb")` would only return one node, however, namely `p3`. A call to `document.getElementById("example").getElementsByClassName("bbb ccc ")` would return the same thing.
>
> A call to `getElementsByClassName("aaa,bbb")` would return no nodes; none of the elements above are in the `aaa,bbb` class.

> **For web developers:**
>
> - `element = document.createElement(localName [, options])`
>   Returns an element with localName as local name (if document is an HTML document, localName gets lowercased). The element’s namespace is the HTML namespace when document is an HTML document or document’s content type is "`application/xhtml+xml`"; otherwise null. When supplied, options’s `customElementRegistry` can be used to set the `CustomElementRegistry`. When supplied, options’s `is` can be used to create a customized built-in element. If localName is not a valid element local name an "`InvalidCharacterError`" `DOMException` will be thrown. When both options’s `customElementRegistry` and options’s `is` are supplied, a "`NotSupportedError`" `DOMException` will be thrown.
> - `element = document.createElementNS(namespace, qualifiedName [, options])`
>   Returns an element with namespace namespace. Its namespace prefix will be everything before U+003A (:) in qualifiedName or null. Its local name will be everything after U+003A (:) in qualifiedName or qualifiedName. When supplied, options’s `customElementRegistry` can be used to set the `CustomElementRegistry`. When supplied, options’s `is` can be used to create a customized built-in element. If qualifiedName is not a (possibly-prefixed) valid element local name an "`InvalidCharacterError`" `DOMException` will be thrown. If one of the following conditions is true a "`NamespaceError`" `DOMException` will be thrown: When both options’s `customElementRegistry` and options’s `is` are supplied, a "`NotSupportedError`" `DOMException` will be thrown.
> - `documentFragment = document.createDocumentFragment()`
>   Returns a `DocumentFragment` node.
> - `text = document.createTextNode(data)`
>   Returns a `Text` node whose data is data.
> - `text = document.createCDATASection(data)`
>   Returns a `CDATASection` node whose data is data.
> - `comment = document.createComment(data)`
>   Returns a `Comment` node whose data is data.
> - `processingInstruction = document.createProcessingInstruction(target, data)`
>   Returns a `ProcessingInstruction` node whose target is target and data is data. If target does not match the `Name` production an "`InvalidCharacterError`" `DOMException` will be thrown. If data contains "`?>`" an "`InvalidCharacterError`" `DOMException` will be thrown.

The [**element interface**](https://dom.spec.whatwg.org/#concept-element-interface) for any name and namespace is `Element`, unless stated otherwise.

> **Note:** The HTML Standard will, e.g., define that for `html` and the HTML namespace, the `HTMLHtmlElement` interface is used. \[HTML\]

- [ ] [createElement(localName, options)](https://dom.spec.whatwg.org/#dom-document-createelement) — The `createElement(localName, options)` method steps are:
  - [ ] 1. If localName is not a valid element local name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. If this is an HTML document, then set localName to localName in ASCII lowercase.
  - [ ] 3. Let registry and is be the result of flattening element creation options given options and this.
  - [ ] 4. Let namespace be the HTML namespace, if this is an HTML document or this’s content type is "`application/xhtml+xml`"; otherwise null.
  - [ ] 5. Return the result of creating an element given this, localName, namespace, null, is, true, and registry.

- [ ] [internal createElementNS steps](https://dom.spec.whatwg.org/#internal-createelementns-steps) — The internal `createElementNS` steps, given document, namespace, qualifiedName, and options, are as follows:
  - [ ] 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and qualifiedName given "`element`".
  - [ ] 2. Let registry and is be the result of flattening element creation options given options and this.
  - [ ] 3. Return the result of creating an element given document, localName, namespace, prefix, is, true, and registry.

- [ ] [createElementNS(namespace, qualifiedName, options)](https://dom.spec.whatwg.org/#dom-document-createelementns) — The `createElementNS(namespace, qualifiedName, options)` method steps are to return the result of running the internal `createElementNS` steps, given this, namespace, qualifiedName, and options.

- [ ] [flatten element creation options](https://dom.spec.whatwg.org/#flatten-element-creation-options) — To flatten element creation options, given a string or `ElementCreationOptions` dictionary options and a document document:
  - [ ] 1. Let registry be the result of looking up a custom element registry given document.
  - [ ] 2. Let is be null.
  - [ ] 3. If options is a dictionary:
    - [ ] 1. If options\["`is`"\] exists, then set is to it.
    - [ ] 2. If options\["`customElementRegistry`"\] exists:
      - [ ] 1. If is is non-null, then throw a "`NotSupportedError`" `DOMException`.
      - [ ] 2. Set registry to options\["`customElementRegistry`"\].
    - [ ] 3. If registry is non-null, registry’s is scoped is false, and registry is not document’s custom element registry, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 4. Return registry and is.

> **Note:** `createElement()` and `createElementNS()`’s options parameter is allowed to be a string for web compatibility.

- [ ] [createDocumentFragment()](https://dom.spec.whatwg.org/#dom-document-createdocumentfragment) — The `createDocumentFragment()` method steps are to return the result of creating a document fragment given this.

- [ ] [createTextNode(data)](https://dom.spec.whatwg.org/#dom-document-createtextnode) — The `createTextNode(data)` method steps are to return the result of creating a text node given this and data.

- [ ] [createCDATASection(data)](https://dom.spec.whatwg.org/#dom-document-createcdatasection) — The `createCDATASection(data)` method steps are:
  - [ ] 1. If this is an HTML document, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 2. If data contains the string "`]]>`", then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 3. Return the result of creating a CDATA section node given this and data.

- [ ] [createComment(data)](https://dom.spec.whatwg.org/#dom-document-createcomment) — The `createComment(data)` method steps are to return the result of creating a comment node given this and data.

- [ ] [createProcessingInstruction(target, data)](https://dom.spec.whatwg.org/#dom-document-createprocessinginstruction) — The `createProcessingInstruction(target, data)` method steps are to return the result of creating a processing instruction node given this, target, and data.

> **For web developers:**
>
> - `clone = document.importNode(node [, options = false])`
>   Returns a copy of node. If options is true or options is a dictionary whose `selfOnly` is false, the copy also includes the node’s descendants. options’s `customElementRegistry` can be used to set the `CustomElementRegistry` of elements that have none. If node is a document or a shadow root, throws a "`NotSupportedError`" `DOMException`.
> - `node = document.adoptNode(node)`
>   Moves node from another document and returns it. If node is a document, throws a "`NotSupportedError`" `DOMException` or, if node is a shadow root, throws a "`HierarchyRequestError`" `DOMException`.

- [ ] [importNode(node, options)](https://dom.spec.whatwg.org/#dom-document-importnode) — The `importNode(node, options)` method steps are:
  - [ ] 1. If node is a document or shadow root, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 2. Let subtree be false.
  - [ ] 3. Let registry be null.
  - [ ] 4. If options is a boolean, then set subtree to options.
  - [ ] 5. Otherwise:
    - [ ] 1. Set subtree to the negation of options\["`selfOnly`"\].
    - [ ] 2. If options\["`customElementRegistry`"\] exists, then set registry to it.
    - [ ] 3. If registry’s is scoped is false and registry is not this’s custom element registry, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 6. If registry is null, then set registry to the result of looking up a custom element registry given this.
  - [ ] 7. Return the result of cloning a node given node with document set to this, subtree set to subtree, and fallbackRegistry set to registry.

- [ ] [adopting steps](https://dom.spec.whatwg.org/#concept-node-adopt-ext) — Specifications may define adopting steps for all or some nodes. The algorithm is passed node and oldDocument, as indicated in the adopt algorithm.

- [ ] [adopt](https://dom.spec.whatwg.org/#concept-node-adopt) — To adopt a node node into a document document:
  - [ ] 1. Let oldDocument be node’s node document.
  - [ ] 2. If node’s parent is non-null, then remove node.
  - [ ] 3. If document is not oldDocument, then for each inclusiveDescendant of node’s shadow-including inclusive descendants, in shadow-including tree order:
    - [ ] 1. Set inclusiveDescendant’s node document to document.
    - [ ] 2. If inclusiveDescendant is a shadow root and if any of the following are true: inclusiveDescendant’s custom element registry is null and inclusiveDescendant’s keep custom element registry null is false; or inclusiveDescendant’s custom element registry is a global custom element registry, then set inclusiveDescendant’s custom element registry to document’s custom element registry’s effective global custom element registry.
    - [ ] 3. Otherwise, if inclusiveDescendant is an element:
      - [ ] 1. Set the node document of each attribute in inclusiveDescendant’s attribute list to document.
      - [ ] 2. If inclusiveDescendant’s custom element registry is null or inclusiveDescendant’s custom element registry’s is scoped is false:
        - [ ] 1. Let registry be null.
        - [ ] 2. If inclusiveDescendant’s custom element registry is non-null, inclusiveDescendant’s parent is null, or inclusiveDescendant’s parent is an exclusive `DocumentFragment` node, then set registry to document’s custom element registry.
        - [ ] 3. Otherwise, set registry to the result of looking up a custom element registry given inclusiveDescendant’s parent.
        - [ ] 4. Set inclusiveDescendant’s custom element registry to registry’s effective global custom element registry.
      - [ ] 3. If inclusiveDescendant is custom, then enqueue a custom element callback reaction with inclusiveDescendant, callback name "`adoptedCallback`", and « oldDocument, document ».
    - [ ] 4. Run the adopting steps with inclusiveDescendant and oldDocument.

- [ ] [adoptNode(node)](https://dom.spec.whatwg.org/#dom-document-adoptnode) — The `adoptNode(node)` method steps are:
  - [ ] 1. If node is a document, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 2. If node is a shadow root, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 3. Adopt node into this.
  - [ ] 4. Return node.

Null or a `CustomElementRegistry` object registry [**is a global custom element registry**](https://dom.spec.whatwg.org/#is-a-global-custom-element-registry) if registry is non-null and registry’s is scoped is false.

- [ ] [effective global custom element registry](https://dom.spec.whatwg.org/#effective-global-custom-element-registry) — Null or a `CustomElementRegistry` object registry’s effective global custom element registry is:
  - [ ] 1. If registry is a global custom element registry, then return registry.
  - [ ] 2. Return null.

- [ ] [createAttribute(localName)](https://dom.spec.whatwg.org/#dom-document-createattribute) — The `createAttribute(localName)` method steps are:
  - [ ] 1. If localName is not a valid attribute local name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. If this is an HTML document, then set localName to localName in ASCII lowercase.
  - [ ] 3. Return the result of creating an attribute given this and localName.

- [ ] [createAttributeNS(namespace, qualifiedName)](https://dom.spec.whatwg.org/#dom-document-createattributens) — The `createAttributeNS(namespace, qualifiedName)` method steps are:
  - [ ] 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and qualifiedName given "`attribute`".
  - [ ] 2. Return the result of creating an attribute given this, localName, namespace, and prefix.

- [ ] [createEvent(interface)](https://dom.spec.whatwg.org/#dom-document-createevent) — The `createEvent(interface)` method steps are:
  - [ ] 1. Let constructor be null.
  - [ ] 2. If interface is an ASCII case-insensitive match for any of the strings in the first column in the following table, then set constructor to the interface in the second column on the same row as the matching string:

    | String | Interface | Notes |
    |---|---|---|
    | "`beforeunloadevent`" | `BeforeUnloadEvent` | \[HTML\] |
    | "`compositionevent`" | `CompositionEvent` | \[UIEVENTS\] |
    | "`customevent`" | `CustomEvent` |  |
    | "`devicemotionevent`" | `DeviceMotionEvent` | \[DEVICE-ORIENTATION\] |
    | "`deviceorientationevent`" | `DeviceOrientationEvent` |  |
    | "`dragevent`" | `DragEvent` | \[HTML\] |
    | "`event`" | `Event` |  |
    | "`events`" |  |  |
    | "`focusevent`" | `FocusEvent` | \[UIEVENTS\] |
    | "`hashchangeevent`" | `HashChangeEvent` | \[HTML\] |
    | "`htmlevents`" | `Event` |  |
    | "`keyboardevent`" | `KeyboardEvent` | \[UIEVENTS\] |
    | "`messageevent`" | `MessageEvent` | \[HTML\] |
    | "`mouseevent`" | `MouseEvent` | \[UIEVENTS\] |
    | "`mouseevents`" |  |  |
    | "`storageevent`" | `StorageEvent` | \[HTML\] |
    | "`svgevents`" | `Event` |  |
    | "`textevent`" | `TextEvent` | \[UIEVENTS\] |
    | "`touchevent`" | `TouchEvent` | \[TOUCH-EVENTS\] |
    | "`uievent`" | `UIEvent` | \[UIEVENTS\] |
    | "`uievents`" |  |  |

  - [ ] 3. If constructor is null, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 4. If the interface indicated by constructor is not exposed on the relevant global object of this, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 5. Let event be the result of creating an event given constructor.
  - [ ] 6. Initialize event’s `type` attribute to the empty string.
  - [ ] 7. Initialize event’s `timeStamp` attribute to the result of calling current high resolution time with this’s relevant global object.
  - [ ] 8. Initialize event’s `isTrusted` attribute to false.
  - [ ] 9. Unset event’s initialized flag.
  - [ ] 10. Return event.

> **Note:** Event constructors ought to be used instead.

- [ ] [createRange()](https://dom.spec.whatwg.org/#dom-document-createrange) — The `createRange()` method steps are to return a new live range with (this, 0) as its start an end.

> **Note:** The `Range()` constructor can be used instead.

- [ ] [createNodeIterator(root, whatToShow, filter)](https://dom.spec.whatwg.org/#dom-document-createnodeiterator) — The `createNodeIterator(root, whatToShow, filter)` method steps are:
  - [ ] 1. Let iterator be a new `NodeIterator` object.
  - [ ] 2. Set iterator’s root to root.
  - [ ] 3. Set iterator’s reference to (root, true).
  - [ ] 4. Set iterator’s whatToShow to whatToShow.
  - [ ] 5. Set iterator’s filter to filter.
  - [ ] 6. Return iterator.

- [ ] [createTreeWalker(root, whatToShow, filter)](https://dom.spec.whatwg.org/#dom-document-createtreewalker) — The `createTreeWalker(root, whatToShow, filter)` method steps are:
  - [ ] 1. Let walker be a new `TreeWalker` object.
  - [ ] 2. Set walker’s root and walker’s current to root.
  - [ ] 3. Set walker’s whatToShow to whatToShow.
  - [ ] 4. Set walker’s filter to filter.
  - [ ] 5. Return walker.

#### [4.5.1. Interface DOMImplementation](https://dom.spec.whatwg.org/#interface-domimplementation)

User agents must create a `DOMImplementation` object whenever a document is created and associate it with that document.

```webidl
[Exposed=Window]
interface DOMImplementation {
  [NewObject] DocumentType createDocumentType(DOMString name, DOMString publicId, DOMString systemId);
  [NewObject] XMLDocument createDocument(DOMString? namespace, [LegacyNullToEmptyString] DOMString qualifiedName, optional DocumentType? doctype = null);
  [NewObject] Document createHTMLDocument(optional DOMString title);

  boolean hasFeature(); // historical; always returns true
};
```

> **For web developers:**
>
> - `doctype = document.implementation.createDocumentType(name, publicId, systemId)`
>   Returns a doctype, with the given name, publicId, and systemId. If name is not a valid doctype name, an "`InvalidCharacterError`" `DOMException` is thrown.
> - `doc = document.implementation.createDocument(namespace, qualifiedName [, doctype = null])`
>   Returns an `XMLDocument`, with a document element whose local name is qualifiedName and whose namespace is namespace (unless qualifiedName is the empty string), and with doctype, if it is given, as its doctype. This method throws the same exceptions as the `createElementNS()` method, when invoked with namespace and qualifiedName.
> - `doc = document.implementation.createHTMLDocument([title])`
>   Returns a document, with a basic tree already constructed including a `title` element, unless the title argument is omitted.

- [ ] [createDocumentType(name, publicId, systemId)](https://dom.spec.whatwg.org/#dom-domimplementation-createdocumenttype) — The `createDocumentType(name, publicId, systemId)` method steps are:
  - [ ] 1. If name is not a valid doctype name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. Return the result of creating a doctype given the associated document of this, name, publicId, and systemId.

- [ ] [createDocument(namespace, qualifiedName, doctype)](https://dom.spec.whatwg.org/#dom-domimplementation-createdocument) — The `createDocument(namespace, qualifiedName, doctype)` method steps are:
  - [ ] 1. Let document be the result of creating a document that implements `XMLDocument`, given this’s relevant realm.
  - [ ] 2. Let element be null.
  - [ ] 3. If qualifiedName is not the empty string, then set element to the result of running the internal `createElementNS` steps, given document, namespace, qualifiedName, and an empty dictionary.
  - [ ] 4. If doctype is non-null, append doctype to document.
  - [ ] 5. If element is non-null, append element to document.
  - [ ] 6. document’s origin is this’s associated document’s origin.
  - [ ] 7. document’s content type is determined by namespace: HTML namespace `application/xhtml+xml` SVG namespace `image/svg+xml` Any other namespace `application/xml`
  - [ ] 8. Return document.

- [ ] [createHTMLDocument(title)](https://dom.spec.whatwg.org/#dom-domimplementation-createhtmldocument) — The `createHTMLDocument(title)` method steps are:
  - [ ] 1. Let doc be the result of creating a document that implements `Document`, given this’s relevant realm.
  - [ ] 2. Set doc’s type to "`html`" and content type to "`text/html`".
  - [ ] 3. Let doctype be the result of creating a doctype given doc and "`html`".
  - [ ] 4. Append doctype to doc.
  - [ ] 5. Append the result of creating an element given doc, "`html`", and the HTML namespace, to doc.
  - [ ] 6. Append the result of creating an element given doc, "`head`", and the HTML namespace, to the `html` element created earlier.
  - [ ] 7. If title is given:
    - [ ] 1. Append the result of creating an element given doc, "`title`", and the HTML namespace, to the `head` element created earlier.
    - [ ] 2. Let text be the result of creating a text node given doc and title (which could be the empty string).
    - [ ] 3. Append text to the `title` element created earlier.
  - [ ] 8. Append the result of creating an element given doc, "`body`", and the HTML namespace, to the `html` element created earlier.
  - [ ] 9. doc’s origin is this’s associated document’s origin.
  - [ ] 10. Return doc.

- [ ] [hasFeature()](https://dom.spec.whatwg.org/#dom-domimplementation-hasfeature) — The `hasFeature()` method steps are to return true.

> **Note:** `hasFeature()` originally would report whether the user agent claimed to support a given DOM feature, but experience proved it was not nearly as reliable or granular as simply checking whether the desired objects, attributes, or methods existed. As such, it is no longer to be used, but continues to exist (and simply returns true) so that old pages don’t stop working.

### [4.6. Interface DocumentType](https://dom.spec.whatwg.org/#interface-documenttype)

```webidl
[Exposed=Window]
interface DocumentType : Node {
  readonly attribute DOMString name;
  readonly attribute DOMString publicId;
  readonly attribute DOMString systemId;
};
```

`DocumentType` nodes are simply known as [**doctypes**](https://dom.spec.whatwg.org/#concept-doctype).

Doctypes have an associated [**name**](https://dom.spec.whatwg.org/#concept-doctype-name) (a string), [**public ID**](https://dom.spec.whatwg.org/#concept-doctype-publicid) (a string), and [**system ID**](https://dom.spec.whatwg.org/#concept-doctype-systemid) (a string).

- [ ] [create a doctype](https://dom.spec.whatwg.org/#create-a-doctype) — To create a doctype given a document document, a string name, and optionally a string publicId (default the empty string) and a string systemId (default the empty string):
  - [ ] 1. Let doctype be the result of creating a node that implements `DocumentType`, given document.
  - [ ] 2. Set doctype’s name to name, public ID to publicId, and system ID to systemId.
  - [ ] 3. Return doctype.

- [ ] [name](https://dom.spec.whatwg.org/#dom-documenttype-name) — The `name` getter steps are to return this’s name.

- [ ] [publicId](https://dom.spec.whatwg.org/#dom-documenttype-publicid) — The `publicId` getter steps are to return this’s public ID.

- [ ] [systemId](https://dom.spec.whatwg.org/#dom-documenttype-systemid) — The `systemId` getter steps are to return this’s system ID.

### [4.7. Interface DocumentFragment](https://dom.spec.whatwg.org/#interface-documentfragment)

```webidl
[Exposed=Window]
interface DocumentFragment : Node {
  constructor();
};
```

> **For web developers:**
>
> - `tree = new DocumentFragment()`
>   Returns a new `DocumentFragment` node.

- [ ] [exclusive DocumentFragment node](https://dom.spec.whatwg.org/#exclusive-documentfragment-node) — An exclusive `DocumentFragment` node is a `DocumentFragment` node that is not a `ShadowRoot` node.

A `DocumentFragment` node has an associated [**host**](https://dom.spec.whatwg.org/#concept-documentfragment-host) (null or an element in a different node tree). It is null unless otherwise stated.

- [ ] [host-including inclusive ancestor](https://dom.spec.whatwg.org/#concept-tree-host-including-inclusive-ancestor) — An object A is a host-including inclusive ancestor of an object B, if either A is an inclusive ancestor of B, or if B’s root has a non-null host and A is a host-including inclusive ancestor of B’s root’s host.

> **Note:** The `DocumentFragment` node’s host concept is useful for HTML’s `template` element and for shadow roots, and impacts the pre-insert and replace algorithms.

- [ ] [create a document fragment](https://dom.spec.whatwg.org/#create-a-document-fragment) — To create a document fragment given a document document: return the result of creating a node that implements `DocumentFragment`, given document.

- [ ] [new DocumentFragment()](https://dom.spec.whatwg.org/#dom-documentfragment-documentfragment) — The `new DocumentFragment()` constructor steps are to set this’s node document to current global object’s associated `Document`.

### [4.8. Interface ShadowRoot](https://dom.spec.whatwg.org/#interface-shadowroot)

```webidl
[Exposed=Window]
interface ShadowRoot : DocumentFragment {
  readonly attribute ShadowRootMode mode;
  readonly attribute boolean delegatesFocus;
  readonly attribute boolean serializable;
  readonly attribute SlotAssignmentMode slotAssignment;
  readonly attribute boolean clonable;
  readonly attribute Element host;

  attribute EventHandler onslotchange;
};

enum ShadowRootMode { "open", "closed" };
enum SlotAssignmentMode { "manual", "named" };
```

`ShadowRoot` nodes are simply known as [**shadow roots**](https://dom.spec.whatwg.org/#concept-shadow-root).

Shadow roots’s associated host is never null.

Shadow roots have an associated [**mode**](https://dom.spec.whatwg.org/#shadowroot-mode) ("`open`" or "`closed`").

Shadow roots have an associated [**delegates focus**](https://dom.spec.whatwg.org/#shadowroot-delegates-focus) (a boolean). It is initially set to false.

Shadow roots have an associated [**available to element internals**](https://dom.spec.whatwg.org/#shadowroot-available-to-element-internals) (a boolean). It is initially set to false.

Shadow roots have an associated [**declarative**](https://dom.spec.whatwg.org/#shadowroot-declarative) (a boolean). It is initially set to false.

Shadow roots have an associated [**serializable**](https://dom.spec.whatwg.org/#shadowroot-serializable) (a boolean). It is initially set to false.

Shadow roots have an associated [**slot assignment**](https://dom.spec.whatwg.org/#shadowroot-slot-assignment) ("`manual`" or "`named`").

Shadow roots have an associated [**clonable**](https://dom.spec.whatwg.org/#shadowroot-clonable) (a boolean). It is initially set to false.

Shadow roots have an associated [**custom element registry**](https://dom.spec.whatwg.org/#shadowroot-custom-element-registry) (null or a `CustomElementRegistry` object). It is initially null.

Shadow roots have an associated [**keep custom element registry null**](https://dom.spec.whatwg.org/#shadowroot-keep-custom-element-registry-null) (a boolean). It is initially false.

> **Note:** This can only ever be true in combination with declarative shadow roots. And it only matters for as long as the shadow root’s custom element registry is null.

A shadow root’s get the parent algorithm, given an event, returns null if event’s composed flag is unset and shadow root is the root of event’s path’s first event path item’s invocation target; otherwise shadow root’s host.

- [ ] [mode](https://dom.spec.whatwg.org/#dom-shadowroot-mode) — The `mode` getter steps are to return this’s mode.

- [ ] [delegatesFocus](https://dom.spec.whatwg.org/#dom-shadowroot-delegatesfocus) — The `delegatesFocus` getter steps are to return this’s delegates focus.

- [ ] [serializable](https://dom.spec.whatwg.org/#dom-shadowroot-serializable) — The `serializable` getter steps are to return this’s serializable.

- [ ] [slotAssignment](https://dom.spec.whatwg.org/#dom-shadowroot-slotassignment) — The `slotAssignment` getter steps are to return this’s slot assignment.

- [ ] [clonable](https://dom.spec.whatwg.org/#dom-shadowroot-clonable) — The `clonable` getter steps are to return this’s clonable.

- [ ] [host](https://dom.spec.whatwg.org/#dom-shadowroot-host) — The `host` getter steps are to return this’s host.

- [ ] [onslotchange](https://dom.spec.whatwg.org/#dom-shadowroot-onslotchange) — The `onslotchange` attribute is an event handler IDL attribute for the `onslotchange` event handler, whose event handler event type is `slotchange`.

In [**shadow-including tree order**](https://dom.spec.whatwg.org/#concept-shadow-including-tree-order) is shadow-including preorder, depth-first traversal of a node tree. [**Shadow-including preorder, depth-first traversal**](https://dom.spec.whatwg.org/#shadow-including-preorder-depth-first-traversal) of a node tree tree is preorder, depth-first traversal of tree, with for each shadow host encountered in tree, shadow-including preorder, depth-first traversal of that element’s shadow root’s node tree just after it is encountered.

The [**shadow-including root**](https://dom.spec.whatwg.org/#concept-shadow-including-root) of an object is its root’s host’s shadow-including root, if the object’s root is a shadow root; otherwise its root.

An object A is a [**shadow-including descendant**](https://dom.spec.whatwg.org/#concept-shadow-including-descendant) of an object B, if A is a descendant of B, or A’s root is a shadow root and A’s root’s host is a shadow-including inclusive descendant of B.

A [**shadow-including inclusive descendant**](https://dom.spec.whatwg.org/#concept-shadow-including-inclusive-descendant) is an object or one of its shadow-including descendants.

An object A is a [**shadow-including ancestor**](https://dom.spec.whatwg.org/#concept-shadow-including-ancestor) of an object B, if and only if B is a shadow-including descendant of A.

A [**shadow-including inclusive ancestor**](https://dom.spec.whatwg.org/#concept-shadow-including-inclusive-ancestor) is an object or one of its shadow-including ancestors.

A node A is [**closed-shadow-hidden**](https://dom.spec.whatwg.org/#concept-closed-shadow-hidden) from a node B if all of the following conditions are true:

- A’s root is a shadow root.
- A’s root is not a shadow-including inclusive ancestor of B.
- A’s root is a shadow root whose mode is "`closed`" or A’s root’s host is closed-shadow-hidden from B.

- [ ] [retarget](https://dom.spec.whatwg.org/#retarget) — To retarget an object A against an object B, repeat these steps until they return an object:
  - [ ] 1. If one of the following is true A is not a node A’s root is not a shadow root B is a node and A’s root is a shadow-including inclusive ancestor of B then return A.
  - [ ] 2. Set A to A’s root’s host.

> **Note:** The retargeting algorithm is used by event dispatch as well as other specifications, such as Fullscreen. \[FULLSCREEN\]

### [4.9. Interface Element](https://dom.spec.whatwg.org/#interface-element)

```webidl
[Exposed=Window]
interface Element : Node {
  readonly attribute DOMString? namespaceURI;
  readonly attribute DOMString? prefix;
  readonly attribute DOMString localName;
  readonly attribute DOMString tagName;

  [CEReactions] attribute DOMString id;
  [CEReactions] attribute DOMString className;
  [SameObject, PutForwards=value] readonly attribute DOMTokenList classList;
  [CEReactions, Unscopable] attribute DOMString slot;

  boolean hasAttributes();
  [SameObject] readonly attribute NamedNodeMap attributes;
  sequence<DOMString> getAttributeNames();
  DOMString? getAttribute(DOMString qualifiedName);
  DOMString? getAttributeNS(DOMString? namespace, DOMString localName);
  [CEReactions] undefined setAttribute(DOMString qualifiedName, (TrustedType or DOMString) value);
  [CEReactions] undefined setAttributeNS(DOMString? namespace, DOMString qualifiedName, (TrustedType or DOMString) value);
  [CEReactions] undefined removeAttribute(DOMString qualifiedName);
  [CEReactions] undefined removeAttributeNS(DOMString? namespace, DOMString localName);
  [CEReactions] boolean toggleAttribute(DOMString qualifiedName, optional boolean force);
  boolean hasAttribute(DOMString qualifiedName);
  boolean hasAttributeNS(DOMString? namespace, DOMString localName);

  Attr? getAttributeNode(DOMString qualifiedName);
  Attr? getAttributeNodeNS(DOMString? namespace, DOMString localName);
  [CEReactions] Attr? setAttributeNode(Attr attr);
  [CEReactions] Attr? setAttributeNodeNS(Attr attr);
  [CEReactions] Attr removeAttributeNode(Attr attr);

  ShadowRoot attachShadow(ShadowRootInit init);
  readonly attribute ShadowRoot? shadowRoot;

  readonly attribute CustomElementRegistry? customElementRegistry;

  Element? closest(DOMString selectors);
  boolean matches(DOMString selectors);
  boolean webkitMatchesSelector(DOMString selectors); // legacy alias of .matches

  HTMLCollection getElementsByTagName(DOMString qualifiedName);
  HTMLCollection getElementsByTagNameNS(DOMString? namespace, DOMString localName);
  HTMLCollection getElementsByClassName(DOMString classNames);

  [CEReactions] Element? insertAdjacentElement(DOMString where, Element element); // legacy
  undefined insertAdjacentText(DOMString where, DOMString data); // legacy
};

dictionary ShadowRootInit {
  required ShadowRootMode mode;
  boolean delegatesFocus = false;
  boolean serializable = false;
  SlotAssignmentMode slotAssignment = "named";
  boolean clonable = false;
  CustomElementRegistry? customElementRegistry;
};
```

> **Note:** `ShadowRootInit` somewhat unusually allows both `undefined` and `null` to be passed to its `customElementRegistry` member to allow web developers to pass a `ShadowRoot` node instead of a dictionary to `attachShadow()`.

`Element` nodes are simply known as [**elements**](https://dom.spec.whatwg.org/#concept-element).

Elements have an associated:

- **[**namespace**](https://dom.spec.whatwg.org/#concept-element-namespace)**: Null or a non-empty string.
- **[**namespace prefix**](https://dom.spec.whatwg.org/#concept-element-namespace-prefix)**: Null or a non-empty string.
- **[**local name**](https://dom.spec.whatwg.org/#concept-element-local-name)**: A non-empty string.
- **[**custom element registry**](https://dom.spec.whatwg.org/#element-custom-element-registry)**: Null or a `CustomElementRegistry` object.
- **[**custom element state**](https://dom.spec.whatwg.org/#concept-element-custom-element-state)**: "`undefined`", "`failed`", "`uncustomized`", "`precustomized`", or "`custom`".
- **[**custom element definition**](https://dom.spec.whatwg.org/#concept-element-custom-element-definition)**: Null or a custom element definition.
- **[**is value**](https://dom.spec.whatwg.org/#concept-element-is-value)**: Null or a valid custom element name.

When an element is created, all of these values are initialized.

An element whose custom element state is "`uncustomized`" or "`custom`" is said to be [**defined**](https://dom.spec.whatwg.org/#concept-element-defined). An element whose custom element state is "`custom`" is said to be [**custom**](https://dom.spec.whatwg.org/#concept-element-custom).

> **Note:** Whether or not an element is defined is used to determine the behavior of the:defined pseudo-class. Whether or not an element is custom is used to determine the behavior of the mutation algorithms. The "`failed`" and "`precustomized`" states are used to ensure that if a custom element constructor fails to execute correctly the first time, it is not executed again by an upgrade.

> **Example:**
>
> The following code illustrates elements in each of these four states:
>
> ```
> <!DOCTYPE html>
> <script>
>   window.customElements.define("sw-rey", class extends HTMLElement {})
>   window.customElements.define("sw-finn", class extends HTMLElement {}, { extends: "p" })
>   window.customElements.define("sw-kylo", class extends HTMLElement {
>     constructor() {
>       // super() intentionally omitted for this example
>     }
>   })
> </script>
>
> <!-- "undefined" (not defined, not custom) -->
> <sw-han></sw-han>
> <p is="sw-luke"></p>
> <p is="asdf"></p>
>
> <!-- "failed" (not defined, not custom) -->
> <sw-kylo></sw-kylo>
>
> <!-- "uncustomized" (defined, not custom) -->
> <p></p>
> <asdf></asdf>
>
> <!-- "custom" (defined, custom) -->
> <sw-rey></sw-rey>
> <p is="sw-finn"></p>
> ```

Elements also have an associated [**shadow root**](https://dom.spec.whatwg.org/#concept-element-shadow-root) (null or a shadow root). It is null unless otherwise stated. An element is a [**shadow host**](https://dom.spec.whatwg.org/#element-shadow-host) if its shadow root is non-null.

An element’s [**qualified name**](https://dom.spec.whatwg.org/#concept-element-qualified-name) is its local name if its namespace prefix is null; otherwise its namespace prefix, followed by "`:`", followed by its local name.

- [ ] [HTML-uppercased qualified name](https://dom.spec.whatwg.org/#element-html-uppercased-qualified-name) — An element’s HTML-uppercased qualified name is the return value of these steps:
  - [ ] 1. Let qualifiedName be this’s qualified name.
  - [ ] 2. If this is in the HTML namespace and its node document is an HTML document, then return qualifiedName in ASCII uppercase.
  - [ ] 3. Return qualifiedName.

> **Note:** User agents could optimize qualified name and HTML-uppercased qualified name by storing them in internal slots.

- [ ] [create an element](https://dom.spec.whatwg.org/#concept-create-element) — To create an element, given a document document, string localName, string-or-null namespace, and optionally a string-or-null prefix (default null), string-or-null is (default null), boolean synchronousCustomElements (default false), and "`default`", null, or a `CustomElementRegistry` object registry (default "`default`"):
  - [ ] 1. Let result be null.
  - [ ] 2. If registry is "`default`", then set registry to the result of looking up a custom element registry given document.
  - [ ] 3. Let definition be the result of looking up a custom element definition given registry, namespace, localName, and is.
  - [ ] 4. If definition is non-null, and definition’s name is not equal to its local name (i.e., definition represents a customized built-in element):
    - [ ] 1. Let interface be the element interface for localName and the HTML namespace.
    - [ ] 2. Set result to the result of creating an element internal given document, interface, localName, the HTML namespace, prefix, "`undefined`", is, and registry.
    - [ ] 3. If synchronousCustomElements is true, then run this step while catching any exceptions: If this step threw an exception exception:
      - [ ] 1. Upgrade result using definition.
      - [ ] 1. Report exception for definition’s constructor’s corresponding JavaScript object’s associated realm’s global object.
      - [ ] 2. Set result’s custom element state to "`failed`".
    - [ ] 4. Otherwise, enqueue a custom element upgrade reaction given result and definition.
  - [ ] 5. Otherwise, if definition is non-null:
    - [ ] 1. If synchronousCustomElements is true:
      - [ ] 1. Let C be definition’s constructor.
      - [ ] 2. Let previousRegistry be the surrounding agent’s active custom element constructor map\[C\] with default null.
      - [ ] 3. Set the surrounding agent’s active custom element constructor map\[C\] to registry.
      - [ ] 4. Run these steps while catching any exceptions: If any of these steps threw an exception exception:
        - [ ] 1. Set result to the result of constructing C, with no arguments.
        - [ ] 2. Assert: one of the following is true: result’s custom element state is "`custom`" and its custom element definition is non-null; result’s custom element state is "`precustomized`"; or result’s custom element state is neither "`custom`" nor "`precustomized`" and its custom element definition is null.
        - [ ] 3. Assert: result’s namespace is the HTML namespace.
        - [ ] 4. If result’s attribute list is not empty, then throw a "`NotSupportedError`" `DOMException`.
        - [ ] 5. If result has children, then throw a "`NotSupportedError`" `DOMException`.
        - [ ] 6. If result’s parent is non-null, then throw a "`NotSupportedError`" `DOMException`.
        - [ ] 7. If result’s node document is not document, then throw a "`NotSupportedError`" `DOMException`.
        - [ ] 8. If result’s local name is not equal to localName, then throw a "`NotSupportedError`" `DOMException`.
        - [ ] 9. Set result’s namespace prefix to prefix.
        - [ ] 10. Set result’s `is` value to null.
        - [ ] 11. Set result’s custom element registry to registry.
        - [ ] 1. Report exception for definition’s constructor’s corresponding JavaScript object’s associated realm’s global object.
        - [ ] 2. Set result to the result of creating an element internal given document, `HTMLUnknownElement`, localName, the HTML namespace, prefix, "`failed`", null, and registry.
      - [ ] 5. If previousRegistry is null, then remove the surrounding agent’s active custom element constructor map\[C\].
      - [ ] 6. Otherwise, set the surrounding agent’s active custom element constructor map\[C\] to previousRegistry.
    - [ ] 2. Otherwise:
      - [ ] 1. Set result to the result of creating an element internal given document, `HTMLElement`, localName, the HTML namespace, prefix, "`undefined`", null, and registry.
      - [ ] 2. Enqueue a custom element upgrade reaction given result and definition.
  - [ ] 6. Otherwise:
    - [ ] 1. Let interface be the element interface for localName and namespace.
    - [ ] 2. Set result to the result of creating an element internal given document, interface, localName, namespace, prefix, "`uncustomized`", is, and registry.
    - [ ] 3. If namespace is the HTML namespace, and either localName is a valid custom element name or is is non-null, then set result’s custom element state to "`undefined`".
  - [ ] 7. Return result.

- [ ] [create an element internal](https://dom.spec.whatwg.org/#create-an-element-internal) — To create an element internal given a document document, an interface interface a string localName, a string-or-null namespace, a string-or-null prefix, a string state, a string-or-null is, and null or a `CustomElementRegistry` object registry:
  - [ ] 1. Let element be the result of creating a node that implements interface, given document.
  - [ ] 2. Set element’s namespace to namespace, namespace prefix to prefix, local name to localName, custom element registry to registry, custom element state to state, custom element definition to null, and `is` value to is.
  - [ ] 3. Assert: element’s attribute list is empty.
  - [ ] 4. Return element.

Elements also have an [**attribute list**](https://dom.spec.whatwg.org/#concept-element-attribute), which is a list exposed through a `NamedNodeMap`. Unless explicitly given when an element is created, its attribute list is empty.

An element [**has**](https://dom.spec.whatwg.org/#concept-element-attribute-has) an attribute A if its attribute list contains A.

This and other specifications may define [**attribute change steps**](https://dom.spec.whatwg.org/#concept-element-attributes-change-ext) for elements. The algorithm is passed element, localName, oldValue, value, and namespace.

- [ ] [handle attribute changes](https://dom.spec.whatwg.org/#handle-attribute-changes) — To handle attribute changes for an attribute attribute with element, oldValue, and newValue, run these steps:
  - [ ] 1. Queue a mutation record of "`attributes`" for element with attribute’s local name, attribute’s namespace, oldValue, « », « », null, and null.
  - [ ] 2. If element is custom, then enqueue a custom element callback reaction with element, callback name "`attributeChangedCallback`", and « attribute’s local name, oldValue, newValue, attribute’s namespace ».
  - [ ] 3. Run the attribute change steps with element, attribute’s local name, oldValue, newValue, and attribute’s namespace.

- [ ] [change](https://dom.spec.whatwg.org/#concept-element-attributes-change) — To change an attribute attribute to value:
  - [ ] 1. Let oldValue be attribute’s value.
  - [ ] 2. Set attribute’s value to value.
  - [ ] 3. Handle attribute changes for attribute with attribute’s element, oldValue, and value.

- [ ] [append](https://dom.spec.whatwg.org/#concept-element-attributes-append) — To append an attribute attribute to an element element:
  - [ ] 1. Append attribute to element’s attribute list.
  - [ ] 2. Set attribute’s element to element.
  - [ ] 3. Set attribute’s node document to element’s node document.
  - [ ] 4. Handle attribute changes for attribute with element, null, and attribute’s value.

- [ ] [remove](https://dom.spec.whatwg.org/#concept-element-attributes-remove) — To remove an attribute attribute:
  - [ ] 1. Let element be attribute’s element.
  - [ ] 2. Remove attribute from element’s attribute list.
  - [ ] 3. Set attribute’s element to null.
  - [ ] 4. Handle attribute changes for attribute with element, attribute’s value, and null.

- [ ] [replace](https://dom.spec.whatwg.org/#concept-element-attributes-replace) — To replace an attribute oldAttribute with an attribute newAttribute:
  - [ ] 1. Let element be oldAttribute’s element.
  - [ ] 2. Replace oldAttribute by newAttribute in element’s attribute list.
  - [ ] 3. Set newAttribute’s element to element.
  - [ ] 4. Set newAttribute’s node document to element’s node document.
  - [ ] 5. Set oldAttribute’s element to null.
  - [ ] 6. Handle attribute changes for oldAttribute with element, oldAttribute’s value, and newAttribute’s value.

- [ ] [get an attribute by name](https://dom.spec.whatwg.org/#concept-element-attributes-get-by-name) — To get an attribute by name given a string qualifiedName and an element element:
  - [ ] 1. If element is in the HTML namespace and its node document is an HTML document, then set qualifiedName to qualifiedName in ASCII lowercase.
  - [ ] 2. Return the first attribute in element’s attribute list whose qualified name is qualifiedName; otherwise null.

- [ ] [get an attribute by namespace and local name](https://dom.spec.whatwg.org/#concept-element-attributes-get-by-namespace) — To get an attribute by namespace and local name given null or a string namespace, a string localName, and an element element:
  - [ ] 1. If namespace is the empty string, then set it to null.
  - [ ] 2. Return the attribute in element’s attribute list whose namespace is namespace and local name is localName, if any; otherwise null.

- [ ] [get an attribute value](https://dom.spec.whatwg.org/#concept-element-attributes-get-value) — To get an attribute value given an element element, a string localName, and an optional null or string namespace (default null):
  - [ ] 1. Let attr be the result of getting an attribute given namespace, localName, and element.
  - [ ] 2. If attr is null, then return the empty string.
  - [ ] 3. Return attr’s value.

- [ ] [set an attribute](https://dom.spec.whatwg.org/#concept-element-attributes-set) — To set an attribute given an attribute attr and an element element:
  - [ ] 1. Let verifiedValue be the result of calling get trusted type compliant attribute value with attr’s local name, attr’s namespace, element, and attr’s value. \[TRUSTED-TYPES\]
  - [ ] 2. If attr’s element is neither null nor element, throw an "`InUseAttributeError`" `DOMException`.
  - [ ] 3. Let oldAttr be the result of getting an attribute given attr’s namespace, attr’s local name, and element.
  - [ ] 4. If oldAttr is attr, return attr.
  - [ ] 5. Set attr’s value to verifiedValue.
  - [ ] 6. If oldAttr is non-null, then replace oldAttr with attr.
  - [ ] 7. Otherwise, append attr to element.
  - [ ] 8. Return oldAttr.

- [ ] [set an attribute value](https://dom.spec.whatwg.org/#concept-element-attributes-set-value) — To set an attribute value given an element element, a string localName, a string value, an optional null or string prefix (default null), and an optional null or string namespace (default null):
  - [ ] 1. Let attribute be the result of getting an attribute given namespace, localName, and element.
  - [ ] 2. If attribute is null, then append the result of creating an attribute given element’s node document, localName, namespace, prefix, and value to element, and then return.
  - [ ] 3. Change attribute to value.

- [ ] [remove an attribute by name](https://dom.spec.whatwg.org/#concept-element-attributes-remove-by-name) — To remove an attribute by name given a string qualifiedName and an element element:
  - [ ] 1. Let attr be the result of getting an attribute given qualifiedName and element.
  - [ ] 2. If attr is non-null, then remove attr.
  - [ ] 3. Return attr.

- [ ] [remove an attribute by namespace and local name](https://dom.spec.whatwg.org/#concept-element-attributes-remove-by-namespace) — To remove an attribute by namespace and local name given null or a string namespace, a string localName, and an element element:
  - [ ] 1. Let attr be the result of getting an attribute given namespace, localName, and element.
  - [ ] 2. If attr is non-null, then remove attr.
  - [ ] 3. Return attr.

An element can have an associated [**unique identifier (ID)**](https://dom.spec.whatwg.org/#concept-id)

> **Note:** Historically elements could have multiple identifiers e.g., by using the HTML `id` attribute and a DTD. This specification makes ID a concept of the DOM and allows for only one per element, given by an `id` attribute.

- [ ] Use these attribute change steps to update an element’s ID:
  - [ ] 1. If localName is `id`, namespace is null, and value is null or the empty string, then unset element’s ID.
  - [ ] 2. Otherwise, if localName is `id`, namespace is null, then set element’s ID to value.

> **Note:** While this specification defines requirements for `class`, `id`, and `slot` attributes on any element, it makes no claims as to whether using them is conforming or not.

A node’s parent of type `Element` is known as its [**parent element**](https://dom.spec.whatwg.org/#parent-element). If the node has a parent of a different type, its parent element is null.

> **For web developers:**
>
> - namespace = element. `namespaceURI`
>   Returns the namespace.
> - prefix = element. `prefix`
>   Returns the namespace prefix.
> - localName = element. `localName`
>   Returns the local name.
> - qualifiedName = element. `tagName`
>   Returns the HTML-uppercased qualified name.

- [ ] [namespaceURI](https://dom.spec.whatwg.org/#dom-element-namespaceuri) — The `namespaceURI` getter steps are to return this’s namespace.

- [ ] [prefix](https://dom.spec.whatwg.org/#dom-element-prefix) — The `prefix` getter steps are to return this’s namespace prefix.

- [ ] [localName](https://dom.spec.whatwg.org/#dom-element-localname) — The `localName` getter steps are to return this’s local name.

- [ ] [tagName](https://dom.spec.whatwg.org/#dom-element-tagname) — The `tagName` getter steps are to return this’s HTML-uppercased qualified name.

> **For web developers:**
>
> - `element.id [ = value ]`
>   Returns the value of element’s `id` content attribute. Can be set to change it.
> - `element.className [ = value ]`
>   Returns the value of element’s `class` content attribute. Can be set to change it.
> - `element.classList`
>   Allows for manipulation of element’s `class` content attribute as a set of whitespace-separated tokens through a `DOMTokenList` object.
> - `element.slot [ = value ]`
>   Returns the value of element’s `slot` content attribute. Can be set to change it.

IDL attributes that are defined to [**reflect**](https://dom.spec.whatwg.org/#concept-reflect) a string name, must have these getter and setter steps:

- **getter steps**: Return the result of running get an attribute value given this and name.
- **setter steps**: Set an attribute value for this using name and the given value.

- [ ] [id](https://dom.spec.whatwg.org/#dom-element-id) — The `id` attribute must reflect "`id`".

- [ ] [className](https://dom.spec.whatwg.org/#dom-element-classname) — The `className` attribute must reflect "`class`".

- [ ] [classList](https://dom.spec.whatwg.org/#dom-element-classlist) — The `classList` getter steps are to return a `DOMTokenList` object whose associated element is this and whose associated attribute’s local name is `class`. The token set of this particular `DOMTokenList` object are also known as the element’s classes.

- [ ] [slot](https://dom.spec.whatwg.org/#dom-element-slot) — The `slot` attribute must reflect "`slot`".

> **Note:** `id`, `class`, and `slot` are effectively superglobal attributes as they can appear on any element, regardless of that element’s namespace.

> **For web developers:**
>
> - `element.hasAttributes()`
>   Returns true if element has attributes; otherwise false.
> - `element.getAttributeNames()`
>   Returns the qualified names of all element’s attributes. Can contain duplicates.
> - `element.getAttribute(qualifiedName)`
>   Returns element’s first attribute whose qualified name is qualifiedName, and null if there is no such attribute otherwise.
> - `element.getAttributeNS(namespace, localName)`
>   Returns element’s attribute whose namespace is namespace and local name is localName, and null if there is no such attribute otherwise.
> - `element.setAttribute(qualifiedName, value)`
>   Sets the value of element’s first attribute whose qualified name is qualifiedName to value.
> - `element.setAttributeNS(namespace, localName, value)`
>   Sets the value of element’s attribute whose namespace is namespace and local name is localName to value.
> - `element.removeAttribute(qualifiedName)`
>   Removes element’s first attribute whose qualified name is qualifiedName.
> - `element.removeAttributeNS(namespace, localName)`
>   Removes element’s attribute whose namespace is namespace and local name is localName.
> - `element.toggleAttribute(qualifiedName [, force])`
>   If force is not given, "toggles" qualifiedName, removing it if it is present and adding it if it is not present. If force is true, adds qualifiedName. If force is false, removes qualifiedName. Returns true if qualifiedName is now present; otherwise false.
> - `element.hasAttribute(qualifiedName)`
>   Returns true if element has an attribute whose qualified name is qualifiedName; otherwise false.
> - `element.hasAttributeNS(namespace, localName)`
>   Returns true if element has an attribute whose namespace is namespace and local name is localName.

- [ ] [hasAttributes()](https://dom.spec.whatwg.org/#dom-element-hasattributes) — The `hasAttributes()` method steps are to return false if this’s attribute list is empty; otherwise true.

- [ ] [attributes](https://dom.spec.whatwg.org/#dom-element-attributes) — The `attributes` getter steps are to return the associated `NamedNodeMap`.

- [ ] [getAttributeNames()](https://dom.spec.whatwg.org/#dom-element-getattributenames) — The `getAttributeNames()` method steps are to return the qualified names of the attributes in this’s attribute list, in order; otherwise a new list.

> **Note:** These are not guaranteed to be unique.

- [ ] [getAttribute(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-getattribute) — The `getAttribute(qualifiedName)` method steps are:
  - [ ] 1. Let attr be the result of getting an attribute given qualifiedName and this.
  - [ ] 2. If attr is null, return null.
  - [ ] 3. Return attr’s value.

- [ ] [getAttributeNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-getattributens) — The `getAttributeNS(namespace, localName)` method steps are:
  - [ ] 1. Let attr be the result of getting an attribute given namespace, localName, and this.
  - [ ] 2. If attr is null, return null.
  - [ ] 3. Return attr’s value.

- [ ] [setAttribute(qualifiedName, value)](https://dom.spec.whatwg.org/#dom-element-setattribute) — The `setAttribute(qualifiedName, value)` method steps are:
  - [ ] 1. If qualifiedName is not a valid attribute local name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName to qualifiedName in ASCII lowercase.
  - [ ] 3. Let verifiedValue be the result of calling get trusted type compliant attribute value with qualifiedName, null, this, and value. \[TRUSTED-TYPES\]
  - [ ] 4. Let attribute be the first attribute in this’s attribute list whose qualified name is qualifiedName, and null otherwise.
  - [ ] 5. If attribute is non-null, then change attribute to verifiedValue and return.
  - [ ] 6. Set attribute to the result of creating an attribute given this’s node document, qualifiedName, null, null, and verifiedValue.
  - [ ] 7. Append attribute to this.

- [ ] [setAttributeNS(namespace, qualifiedName, value)](https://dom.spec.whatwg.org/#dom-element-setattributens) — The `setAttributeNS(namespace, qualifiedName, value)` method steps are:
  - [ ] 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and qualifiedName given "`attribute`".
  - [ ] 2. Let verifiedValue be the result of calling get trusted type compliant attribute value with localName, namespace, this, and value. \[TRUSTED-TYPES\]
  - [ ] 3. Set an attribute value for this using localName, verifiedValue, prefix, and namespace.

- [ ] [removeAttribute(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-removeattribute) — The `removeAttribute(qualifiedName)` method steps are to remove an attribute given qualifiedName and this, and then return undefined.

- [ ] [removeAttributeNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-removeattributens) — The `removeAttributeNS(namespace, localName)` method steps are to remove an attribute given namespace, localName, and this, and then return undefined.

- [ ] [hasAttribute(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-hasattribute) — The `hasAttribute(qualifiedName)` method steps are:
  - [ ] 1. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName to qualifiedName in ASCII lowercase.
  - [ ] 2. Return true if this has an attribute whose qualified name is qualifiedName; otherwise false.

- [ ] [toggleAttribute(qualifiedName, force)](https://dom.spec.whatwg.org/#dom-element-toggleattribute) — The `toggleAttribute(qualifiedName, force)` method steps are:
  - [ ] 1. If qualifiedName is not a valid attribute local name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName to qualifiedName in ASCII lowercase.
  - [ ] 3. Let attribute be the first attribute in this’s attribute list whose qualified name is qualifiedName, and null otherwise.
  - [ ] 4. If attribute is null:
    - [ ] 1. If force is not given or is true, then append the result of creating an attribute given this’s node document and qualifiedName to this, and then return true.
    - [ ] 2. Return false.
  - [ ] 5. If force is not given or is false, remove an attribute given qualifiedName and this, and then return false.
  - [ ] 6. Return true.

- [ ] [hasAttributeNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-hasattributens) — The `hasAttributeNS(namespace, localName)` method steps are:
  - [ ] 1. If namespace is the empty string, then set it to null.
  - [ ] 2. Return true if this has an attribute whose namespace is namespace and local name is localName; otherwise false.

- [ ] [getAttributeNode(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-getattributenode) — The `getAttributeNode(qualifiedName)` method steps are to return the result of getting an attribute given qualifiedName and this.

- [ ] [getAttributeNodeNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-getattributenodens) — The `getAttributeNodeNS(namespace, localName)` method steps are to return the result of getting an attribute given namespace, localName, and this.

- [ ] [setAttributeNode(attr)](https://dom.spec.whatwg.org/#dom-element-setattributenode) — The `setAttributeNode(attr)` and `setAttributeNodeNS(attr)` methods steps are to return the result of setting an attribute given attr and this.

- [ ] [removeAttributeNode(attr)](https://dom.spec.whatwg.org/#dom-element-removeattributenode) — The `removeAttributeNode(attr)` method steps are:
  - [ ] 1. If this’s attribute list does not contain attr, then throw a "`NotFoundError`" `DOMException`.
  - [ ] 2. Remove attr.
  - [ ] 3. Return attr.

> **For web developers:**
>
> - `shadow = element.attachShadow(init)`
>   Creates a shadow root for element and returns it.
> - `shadow = element.shadowRoot`
>   Returns element’s shadow root, if any, and if shadow root’s mode is "`open`"; otherwise null.

A [**valid shadow host name**](https://dom.spec.whatwg.org/#valid-shadow-host-name) is:

- a valid custom element name
- "`article`", "`aside`", "`blockquote`", "`body`", "`div`", "`footer`", "`h1`", "`h2`", "`h3`", "`h4`", "`h5`", "`h6`", "`header`", "`main`", "`nav`", "`p`", "`section`", or "`span`"

> **Note:** This list is intentionally limited so that built-in elements can gain internal shadow trees over time as needed.

- [ ] [attachShadow(init)](https://dom.spec.whatwg.org/#dom-element-attachshadow) — The `attachShadow(init)` method steps are:
  - [ ] 1. Let registry be this’s node document’s custom element registry.
  - [ ] 2. If init\["`customElementRegistry`"\] exists, then set registry to it.
  - [ ] 3. If registry is non-null, registry’s is scoped is false, and registry is not this’s node document’s custom element registry, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 4. Run attach a shadow root with this, init\["`mode`"\], init\["`delegatesFocus`"\], init\["`serializable`"\], init\["`slotAssignment`"\], init\["`clonable`"\], and registry.
  - [ ] 5. Return this’s shadow root.

- [ ] [attach a shadow root](https://dom.spec.whatwg.org/#concept-attach-a-shadow-root) — To attach a shadow root, given an element element, a string mode, a boolean delegatesFocus, a boolean serializable, a string slotAssignment, a boolean clonable, and null or a `CustomElementRegistry` object registry:
  - [ ] 1. If element’s namespace is not the HTML namespace, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 2. If element’s local name is not a valid shadow host name, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 3. If element’s local name is a valid custom element name, or element’s `is` value is non-null:
    - [ ] 1. Let definition be the result of looking up a custom element definition given element’s custom element registry, its namespace, its local name, and its `is` value.
    - [ ] 2. If definition is non-null and definition’s disable shadow is true, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 4. If element is a shadow host:
    - [ ] 1. Let currentShadowRoot be element’s shadow root.
    - [ ] 2. If any of the following are true: currentShadowRoot’s declarative is false; or currentShadowRoot’s mode is not mode, then throw a "`NotSupportedError`" `DOMException`.
    - [ ] 3. Remove all of currentShadowRoot’s children, in tree order.
    - [ ] 4. Set currentShadowRoot’s declarative to false.
    - [ ] 5. Return.
  - [ ] 5. Let shadow be the result of creating a node that implements `ShadowRoot`, given element’s node document.
  - [ ] 6. Set shadow’s host to element.
  - [ ] 7. Set shadow’s mode to mode.
  - [ ] 8. Set shadow’s delegates focus to delegatesFocus.
  - [ ] 9. If element’s custom element state is "`precustomized`" or "`custom`", then set shadow’s available to element internals to true.
  - [ ] 10. Set shadow’s declarative to false.
  - [ ] 11. Set shadow’s serializable to serializable.
  - [ ] 12. Set shadow’s slot assignment to slotAssignment.
  - [ ] 13. Set shadow’s clonable to clonable.
  - [ ] 14. Set shadow’s custom element registry to registry.
  - [ ] 15. Set element’s shadow root to shadow.

- [ ] [shadowRoot](https://dom.spec.whatwg.org/#dom-element-shadowroot) — The `shadowRoot` getter steps are:
  - [ ] 1. Let shadow be this’s shadow root.
  - [ ] 2. If shadow is null or its mode is "`closed`", then return null.
  - [ ] 3. Return shadow.

> **For web developers:**
>
> - `registry = element.customElementRegistry`
>   Returns element’s `CustomElementRegistry` object, if any; otherwise null.

- [ ] [customElementRegistry](https://dom.spec.whatwg.org/#dom-element-customelementregistry) — The `customElementRegistry` getter steps are to return this’s custom element registry.

> **For web developers:**
>
> - `element.closest(selectors)`
>   Returns the first (starting at element) inclusive ancestor that matches selectors, and null otherwise.
> - `element.matches(selectors)`
>   Returns true if matching selectors against element’s root yields element; otherwise false.

- [ ] [closest(selectors)](https://dom.spec.whatwg.org/#dom-element-closest) — The `closest(selectors)` method steps are:
  - [ ] 1. Let selector be the result of parse a selector from selectors. \[SELECTORS4\]
  - [ ] 2. If selector is failure, then throw a "`SyntaxError`" `DOMException`.
  - [ ] 3. Let elements be this’s inclusive ancestors that are elements, in reverse tree order.
  - [ ] 4. For each element of elements: if match a selector against an element, using selector, element, and scoping root this, returns success, return element. \[SELECTORS4\]
  - [ ] 5. Return null.

- [ ] [matches(selectors)](https://dom.spec.whatwg.org/#dom-element-matches) — The `matches(selectors)` and `webkitMatchesSelector(selectors)` method steps are:
  - [ ] 1. Let selector be the result of parse a selector from selectors. \[SELECTORS4\]
  - [ ] 2. If selector is failure, then throw a "`SyntaxError`" `DOMException`.
  - [ ] 3. If the result of match a selector against an element, using selector, this, and scoping root this, returns success, then return true; otherwise, return false. \[SELECTORS4\]

- [ ] [getElementsByTagName(qualifiedName)](https://dom.spec.whatwg.org/#dom-element-getelementsbytagname) — The `getElementsByTagName(qualifiedName)` method steps are to return the list of elements with qualified name qualifiedName for this.

- [ ] [getElementsByTagNameNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-element-getelementsbytagnamens) — The `getElementsByTagNameNS(namespace, localName)` method steps are to return the list of elements with namespace namespace and local name localName for this.

- [ ] [getElementsByClassName(classNames)](https://dom.spec.whatwg.org/#dom-element-getelementsbyclassname) — The `getElementsByClassName(classNames)` method steps are to return the list of elements with class names classNames for this.

To [**insert adjacent**](https://dom.spec.whatwg.org/#insert-adjacent), given an element element, string where, and a node node, run the steps associated with the first ASCII case-insensitive match for where:

- **"`beforebegin`"**: If element’s parent is null, return null. Return the result of pre-inserting node into element’s parent before element.
- **"`afterbegin`"**: Return the result of pre-inserting node into element before element’s first child.
- **"`beforeend`"**: Return the result of pre-inserting node into element before null.
- **"`afterend`"**: If element’s parent is null, return null. Return the result of pre-inserting node into element’s parent before element’s next sibling.
- **Otherwise**: Throw a "`SyntaxError`" `DOMException`.

- [ ] [insertAdjacentElement(where, element)](https://dom.spec.whatwg.org/#dom-element-insertadjacentelement) — The `insertAdjacentElement(where, element)` method steps are to return the result of running insert adjacent, give this, where, and element.

- [ ] [insertAdjacentText(where, data)](https://dom.spec.whatwg.org/#dom-element-insertadjacenttext) — The `insertAdjacentText(where, data)` method steps are:
  - [ ] 1. Let text be the result of creating a text node given this’s node document and data.
  - [ ] 2. Run insert adjacent, given this, where, and text.

> **Note:** This method returns nothing because it existed before we had a chance to design it.

#### [4.9.1. Interface NamedNodeMap](https://dom.spec.whatwg.org/#interface-namednodemap)

```webidl
[Exposed=Window,
 LegacyUnenumerableNamedProperties]
interface NamedNodeMap {
  readonly attribute unsigned long length;
  getter Attr? item(unsigned long index);
  getter Attr? getNamedItem(DOMString qualifiedName);
  Attr? getNamedItemNS(DOMString? namespace, DOMString localName);
  [CEReactions] Attr? setNamedItem(Attr attr);
  [CEReactions] Attr? setNamedItemNS(Attr attr);
  [CEReactions] Attr removeNamedItem(DOMString qualifiedName);
  [CEReactions] Attr removeNamedItemNS(DOMString? namespace, DOMString localName);
};
```

A `NamedNodeMap` has an associated [**element**](https://dom.spec.whatwg.org/#concept-namednodemap-element) (an element).

A `NamedNodeMap` object’s [**attribute list**](https://dom.spec.whatwg.org/#concept-namednodemap-attribute) is its element’s attribute list.

A `NamedNodeMap` object’s supported property indices are the numbers in the range zero to its attribute list’s size − 1, unless the attribute list is empty, in which case there are no supported property indices.

- [ ] [length](https://dom.spec.whatwg.org/#dom-namednodemap-length) — The `length` getter steps are to return the attribute list’s size.

- [ ] [item(index)](https://dom.spec.whatwg.org/#dom-namednodemap-item) — The `item(index)` method steps are:
  - [ ] 1. If index is equal to or greater than this’s attribute list’s size, then return null.
  - [ ] 2. Otherwise, return this’s attribute list\[index\].

- [ ] A `NamedNodeMap` object’s supported property names are the return value of running these steps:
  - [ ] 1. Let names be the qualified names of the attributes in this `NamedNodeMap` object’s attribute list, with duplicates omitted, in order.
  - [ ] 2. If this `NamedNodeMap` object’s element is in the HTML namespace and its node document is an HTML document, then for each name of names:
    - [ ] 1. Let lowercaseName be name, in ASCII lowercase.
    - [ ] 2. If lowercaseName is not equal to name, remove name from names.
  - [ ] 3. Return names.

- [ ] [getNamedItem(qualifiedName)](https://dom.spec.whatwg.org/#dom-namednodemap-getnameditem) — The `getNamedItem(qualifiedName)` method steps are to return the result of getting an attribute given qualifiedName and element.

- [ ] [getNamedItemNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-namednodemap-getnameditemns) — The `getNamedItemNS(namespace, localName)` method steps are to return the result of getting an attribute given namespace, localName, and element.

- [ ] [setNamedItem(attr)](https://dom.spec.whatwg.org/#dom-namednodemap-setnameditem) — The `setNamedItem(attr)` and `setNamedItemNS(attr)` method steps are to return the result of setting an attribute given attr and element.

- [ ] [removeNamedItem(qualifiedName)](https://dom.spec.whatwg.org/#dom-namednodemap-removenameditem) — The `removeNamedItem(qualifiedName)` method steps are:
  - [ ] 1. Let attr be the result of removing an attribute given qualifiedName and element.
  - [ ] 2. If attr is null, then throw a "`NotFoundError`" `DOMException`.
  - [ ] 3. Return attr.

- [ ] [removeNamedItemNS(namespace, localName)](https://dom.spec.whatwg.org/#dom-namednodemap-removenameditemns) — The `removeNamedItemNS(namespace, localName)` method steps are:
  - [ ] 1. Let attr be the result of removing an attribute given namespace, localName, and element.
  - [ ] 2. If attr is null, then throw a "`NotFoundError`" `DOMException`.
  - [ ] 3. Return attr.

#### [4.9.2. Interface Attr](https://dom.spec.whatwg.org/#interface-attr)

```webidl
[Exposed=Window]
interface Attr : Node {
  readonly attribute DOMString? namespaceURI;
  readonly attribute DOMString? prefix;
  readonly attribute DOMString localName;
  readonly attribute DOMString name;
  [CEReactions] attribute DOMString value;

  readonly attribute Element? ownerElement;

  readonly attribute boolean specified; // historical; always returns true
};
```

`Attr` nodes are simply known as [**attributes**](https://dom.spec.whatwg.org/#concept-attribute). They are sometimes referred to as content attributes to avoid confusion with IDL attributes.

Attributes have a [**namespace**](https://dom.spec.whatwg.org/#concept-attribute-namespace) (null or a non-empty string), [**namespace prefix**](https://dom.spec.whatwg.org/#concept-attribute-namespace-prefix) (null or a non-empty string), [**local name**](https://dom.spec.whatwg.org/#concept-attribute-local-name) (a non-empty string), [**value**](https://dom.spec.whatwg.org/#concept-attribute-value) (a string), and [**element**](https://dom.spec.whatwg.org/#concept-attribute-element) (null or an element).

> **Note:** If designed today they would just have a name and value. ☹

An attribute’s [**qualified name**](https://dom.spec.whatwg.org/#concept-attribute-qualified-name) is its local name if its namespace prefix is null, and its namespace prefix, followed by "`:`", followed by its local name, otherwise.

> **Note:** User agents could have this as an internal slot as an optimization.

- [ ] [create an attribute](https://dom.spec.whatwg.org/#create-an-attribute) — To create an attribute given a document document, a string localName, and optionally a string-or-null namespace (default null), a string-or-null prefix (default null), and a string value (default the empty string):
  - [ ] 1. Let attribute be the result of creating a node that implements `Attr`, given document.
  - [ ] 2. Set attribute’s namespace to namespace, namespace prefix to prefix, local name to localName, and value to value.
  - [ ] 3. Return attribute.

An [**A attribute**](https://dom.spec.whatwg.org/#concept-named-attribute) is an attribute whose local name is `A` and whose namespace and namespace prefix are null.

- [ ] [namespaceURI](https://dom.spec.whatwg.org/#dom-attr-namespaceuri) — The `namespaceURI` getter steps are to return this’s namespace.

- [ ] [prefix](https://dom.spec.whatwg.org/#dom-attr-prefix) — The `prefix` getter steps are to return this’s namespace prefix.

- [ ] [localName](https://dom.spec.whatwg.org/#dom-attr-localname) — The `localName` getter steps are to return this’s local name.

- [ ] [name](https://dom.spec.whatwg.org/#dom-attr-name) — The `name` getter steps are to return this’s qualified name.

- [ ] [value](https://dom.spec.whatwg.org/#dom-attr-value) — The `value` getter steps are to return this’s value.

- [ ] [set an existing attribute value](https://dom.spec.whatwg.org/#set-an-existing-attribute-value) — To set an existing attribute value, given an attribute attribute and string value, run these steps:
  - [ ] 1. If attribute’s element is null, then set attribute’s value to value and return.
  - [ ] 2. Let element be attribute’s element.
  - [ ] 3. Let verifiedValue be the result of calling get trusted type compliant attribute value with attribute’s local name, attribute’s namespace, element, and value. \[TRUSTED-TYPES\]
  - [ ] 4. If attribute’s element is null, then set attribute’s value to verifiedValue and return.
  - [ ] 5. Change attribute to verifiedValue.

- [ ] The `value` setter steps are to set an existing attribute value with this and the given value.

- [ ] [ownerElement](https://dom.spec.whatwg.org/#dom-attr-ownerelement) — The `ownerElement` getter steps are to return this’s element.

- [ ] [specified](https://dom.spec.whatwg.org/#dom-attr-specified) — The `specified` getter steps are to return true.

### [4.10. Interface CharacterData](https://dom.spec.whatwg.org/#interface-characterdata)

```webidl
[Exposed=Window]
interface CharacterData : Node {
  attribute [LegacyNullToEmptyString] DOMString data;
  readonly attribute unsigned long length;
  DOMString substringData(unsigned long offset, unsigned long count);
  undefined appendData(DOMString data);
  undefined insertData(unsigned long offset, DOMString data);
  undefined deleteData(unsigned long offset, unsigned long count);
  undefined replaceData(unsigned long offset, unsigned long count, DOMString data);
};
```

> **Note:** `CharacterData` is an abstract interface. You cannot get a direct instance of it. It is used by `Text`, `ProcessingInstruction`, and `Comment` nodes.

Each node inheriting from the `CharacterData` interface has an associated mutable string called [**data**](https://dom.spec.whatwg.org/#concept-cd-data).

- [ ] [replace data](https://dom.spec.whatwg.org/#concept-cd-replace) — To replace data of a node node with an integer offset, integer count, string data, and an optional boolean piAttributesAlreadyUpdated (default false):
  - [ ] 1. Let length be node’s length.
  - [ ] 2. If offset is greater than length, then throw an "`IndexSizeError`" `DOMException`.
  - [ ] 3. If offset + count is greater than length, then set count to length − offset.
  - [ ] 4. Queue a mutation record of "`characterData`" for node with null, null, node’s data, « », « », null, and null.
  - [ ] 5. Insert data into node’s data after offset code units.
  - [ ] 6. Let deleteOffset be offset + data’s length.
  - [ ] 7. Starting from deleteOffset code units, remove count code units from node’s data.
  - [ ] 8. For each live range whose start node is node and start offset is greater than offset but less than or equal to offset + count: set its start offset to offset.
  - [ ] 9. For each live range whose end node is node and end offset is greater than offset but less than or equal to offset + count: set its end offset to offset.
  - [ ] 10. For each live range whose start node is node and start offset is greater than offset + count: increase its start offset by data’s length and decrease it by count.
  - [ ] 11. For each live range whose end node is node and end offset is greater than offset + count: increase its end offset by data’s length and decrease it by count.
  - [ ] 12. If node is a `ProcessingInstruction` node and piAttributesAlreadyUpdated is false, then update attributes from data given node.
  - [ ] 13. If node’s parent is non-null, then run the children changed steps for node’s parent.

- [ ] [substring data](https://dom.spec.whatwg.org/#concept-cd-substring) — To substring data of a node node with an integer offset and integer count:
  - [ ] 1. Let length be node’s length.
  - [ ] 2. If offset is greater than length, then throw an "`IndexSizeError`" `DOMException`.
  - [ ] 3. If offset + count is greater than length, then return a string whose value is the code units from the offsetth code unit to the end of node’s data.
  - [ ] 4. Return a string whose value is the code units from the offsetth code unit to the offset+countth code unit in node’s data.

- [ ] [data](https://dom.spec.whatwg.org/#dom-characterdata-data) — The `data` getter steps are to return this’s data. Its setter steps are to replace data of this with 0, this’s length, and the given value.

- [ ] [length](https://dom.spec.whatwg.org/#dom-characterdata-length) — The `length` getter steps are to return this’s length.

- [ ] [substringData(offset, count)](https://dom.spec.whatwg.org/#dom-characterdata-substringdata) — The `substringData(offset, count)` method steps are to return the result of substringing data of this with offset and count.

- [ ] [appendData(data)](https://dom.spec.whatwg.org/#dom-characterdata-appenddata) — The `appendData(data)` method steps are to replace data of this with this’s length, 0, and data.

- [ ] [insertData(offset, data)](https://dom.spec.whatwg.org/#dom-characterdata-insertdata) — The `insertData(offset, data)` method steps are to replace data of this with offset, 0, and data.

- [ ] [deleteData(offset, count)](https://dom.spec.whatwg.org/#dom-characterdata-deletedata) — The `deleteData(offset, count)` method steps are to replace data of this with offset, count, and the empty string.

- [ ] [replaceData(offset, count, data)](https://dom.spec.whatwg.org/#dom-characterdata-replacedata) — The `replaceData(offset, count, data)` method steps are to replace data of this with offset, count, and data.

### [4.11. Interface Text](https://dom.spec.whatwg.org/#interface-text)

```webidl
[Exposed=Window]
interface Text : CharacterData {
  constructor(optional DOMString data = "");

  [NewObject] Text splitText(unsigned long offset);
  readonly attribute DOMString wholeText;
};
```

> **For web developers:**
>
> - `text = new Text([data = ""])`
>   Returns a new `Text` node whose data is data.
> - `text.splitText(offset)`
>   Splits data at the given offset and returns the remainder as `Text` node.
> - `text.wholeText`
>   Returns the combined data of all direct `Text` node siblings.

- [ ] [exclusive Text node](https://dom.spec.whatwg.org/#exclusive-text-node) — An exclusive `Text` node is a `Text` node that is not a `CDATASection` node.

- [ ] [contiguous Text nodes](https://dom.spec.whatwg.org/#contiguous-text-nodes) — The contiguous `Text` nodes of a node node are node, node’s previous sibling `Text` node, if any, and its contiguous `Text` nodes, and node’s next sibling `Text` node, if any, and its contiguous `Text` nodes, avoiding any duplicates.

- [ ] [contiguous exclusive Text nodes](https://dom.spec.whatwg.org/#contiguous-exclusive-text-nodes) — The contiguous exclusive `Text` nodes of a node node are node, node’s previous sibling exclusive `Text` node, if any, and its contiguous exclusive `Text` nodes, and node’s next sibling exclusive `Text` node, if any, and its contiguous exclusive `Text` nodes, avoiding any duplicates.

- [ ] [child text content](https://dom.spec.whatwg.org/#concept-child-text-content) — The child text content of a node node is the concatenation of the data of all the `Text` node children of node, in tree order.

- [ ] [descendant text content](https://dom.spec.whatwg.org/#concept-descendant-text-content) — The descendant text content of a node node is the concatenation of the data of all the `Text` node descendants of node, in tree order.

- [ ] [create a text node](https://dom.spec.whatwg.org/#create-a-text-node) — To create a text node given a document document and a string data:
  - [ ] 1. Let text be the result of creating a node that implements `Text`, given document.
  - [ ] 2. Set text’s data to data.
  - [ ] 3. Return text.

- [ ] [new Text(data)](https://dom.spec.whatwg.org/#dom-text-text) — The `new Text(data)` constructor steps are to set this’s data to data and this’s node document to current global object’s associated `Document`.

- [ ] [split](https://dom.spec.whatwg.org/#concept-text-split) — To split a `Text` node node with integer offset:
  - [ ] 1. Let length be node’s length.
  - [ ] 2. If offset is greater than length, then throw an "`IndexSizeError`" `DOMException`.
  - [ ] 3. Let count be length − offset.
  - [ ] 4. Let newData be the result of substringing data of node with offset and count.
  - [ ] 5. Let newNode be the result of creating a text node given node’s node document and newData.
  - [ ] 6. Let parent be node’s parent.
  - [ ] 7. If parent is non-null:
    - [ ] 1. Insert newNode into parent before node’s next sibling.
    - [ ] 2. For each live range whose start node is node and start offset is greater than offset, set its start node to newNode and decrease its start offset by offset.
    - [ ] 3. For each live range whose end node is node and end offset is greater than offset, set its end node to newNode and decrease its end offset by offset.
    - [ ] 4. For each live range whose start node is parent and start offset is equal to the index of node plus 1, increase its start offset by 1.
    - [ ] 5. For each live range whose end node is parent and end offset is equal to the index of node plus 1, increase its end offset by 1.
  - [ ] 8. Replace data of node with offset, count, and the empty string.
  - [ ] 9. Return newNode.

- [ ] [splitText(offset)](https://dom.spec.whatwg.org/#dom-text-splittext) — The `splitText(offset)` method steps are to split this with offset offset.

- [ ] [wholeText](https://dom.spec.whatwg.org/#dom-text-wholetext) — The `wholeText` getter steps are to return the concatenation of the data of the contiguous `Text` nodes of this, in tree order.

### [4.12. Interface CDATASection](https://dom.spec.whatwg.org/#interface-cdatasection)

```webidl
[Exposed=Window]
interface CDATASection : Text {
};
```

- [ ] [create a CDATA section node](https://dom.spec.whatwg.org/#create-a-cdata-section-node) — To create a CDATA section node given a document document and a string data:
  - [ ] 1. Let node be the result of creating a node that implements `CDATASection`, given document.
  - [ ] 2. Set node’s data to data.
  - [ ] 3. Return node.

### [4.13. Interface ProcessingInstruction](https://dom.spec.whatwg.org/#interface-processinginstruction)

```webidl
[Exposed=Window]
interface ProcessingInstruction : CharacterData {
  constructor(DOMString target, optional DOMString data = "");

  readonly attribute DOMString target;

  boolean hasAttributes();
  sequence<DOMString> getAttributeNames();
  DOMString? getAttribute(DOMString name);
  undefined setAttribute(DOMString name, DOMString value);
  undefined removeAttribute(DOMString name);
  boolean toggleAttribute(DOMString name, optional boolean force);
  boolean hasAttribute(DOMString name);
};
```

> **For web developers:**
>
> - `pi = new ProcessingInstruction(target [, data = ""])`
>   Returns a new `ProcessingInstruction` node whose target is target and data is data.
> - `pi.target`
>   Returns the target.
> - `pi.hasAttributes()`
>   Returns true if pi has attributes; otherwise false.
> - `pi.getAttributeNames()`
>   Returns the names of all of pi’s attributes. Cannot contain duplicates.
> - `pi.getAttribute(name)`
>   Returns the value of pi’s attribute named name, and null if there is no such attribute.
> - `pi.setAttribute(name, value)`
>   Sets pi’s attribute named name to value.
> - `pi.removeAttribute(name)`
>   Removes pi’s attribute named name.
> - `pi.toggleAttribute(name [, force])`
>   If force is not given, "toggles" name, removing it if it is present and adding it if it is not present. If force is true, adds name. If force is false, removes name. Returns true if name is now present; otherwise false.
> - `pi.hasAttribute(name)`
>   Returns true if pi has an attribute named name; otherwise false.

`ProcessingInstruction` nodes have an associated [**target**](https://dom.spec.whatwg.org/#concept-pi-target).

`ProcessingInstruction` nodes have an associated [**attribute map**](https://dom.spec.whatwg.org/#concept-pi-attribute-map), which is a map, initially empty.

- [ ] [create a processing instruction node](https://dom.spec.whatwg.org/#create-a-processing-instruction-node) — To create a processing instruction node given a document document, a string target, and a string data:
  - [ ] 1. Let pi be the result of creating a node that implements `ProcessingInstruction`, given document.
  - [ ] 2. Initialize pi with target and data.
  - [ ] 3. Return pi.

- [ ] [initialize](https://dom.spec.whatwg.org/#processinginstruction-initialize) — To initialize a `ProcessingInstruction` node pi, with target and data:
  - [ ] 1. If target does not match the `Name` production, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. If data contains the string "`?>`", then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 3. Set pi’s target to target.
  - [ ] 4. Set pi’s data to data.
  - [ ] 5. Update attributes from data given pi.

- [ ] [update attributes from data](https://dom.spec.whatwg.org/#update-attributes-from-data) — To update attributes from data, given a `ProcessingInstruction` node pi:
  - [ ] 1. Clear pi’s attribute map.
  - [ ] 2. Let result be the parsing result of invoking the rules for parsing pseudo-attributes from a string given pi’s data.
  - [ ] 3. If result is an error, then return.
  - [ ] 4. For each pseudo-attribute pseudoAttr of result: set pi’s attribute map\[pseudoAttr’s name\] to pseudoAttr’s value.

- [ ] [update data from attributes](https://dom.spec.whatwg.org/#update-data-from-attributes) — To update data from attributes, given a `ProcessingInstruction` node pi:
  - [ ] 1. Let data be the empty string.
  - [ ] 2. For each name → value of pi’s attribute map:
    - [ ] 1. If data is not the empty string, then append U+0020 SPACE to data.
    - [ ] 2. Append name to data.
    - [ ] 3. Append U+003D (=) to data.
    - [ ] 4. Append U+0022 (") to data.
    - [ ] 5. Replace any U+0026 (&) in value with "`&amp;`".
    - [ ] 6. Replace any U+003C (\<) in value with "`&lt;`".
    - [ ] 7. Replace any U+003E (\>) in value with "`&gt;`".
    - [ ] 8. Replace any U+0022 (") in value with "`&quot;`".
    - [ ] 9. Append value to data.
    - [ ] 10. Append U+0022 (") to data.
  - [ ] 3. Replace data of pi with 0, pi’s length, data, and true.

- [ ] [get a processing instruction attribute](https://dom.spec.whatwg.org/#get-a-processing-instruction-attribute) — To get a processing instruction attribute, given a `ProcessingInstruction` node pi and a string name: return pi’s attribute map\[name\] with default null.

- [ ] [new ProcessingInstruction(target, data)](https://dom.spec.whatwg.org/#dom-processinginstruction-processinginstruction) — The `new ProcessingInstruction(target, data)` constructor steps are:
  - [ ] 1. Set this’s node document to current global object’s associated `Document`.
  - [ ] 2. Initialize this with target and data.

- [ ] [target](https://dom.spec.whatwg.org/#dom-processinginstruction-target) — The `target` getter steps are to return this’s target.

- [ ] [hasAttributes()](https://dom.spec.whatwg.org/#dom-processinginstruction-hasattributes) — The `hasAttributes()` method steps are to return false if this’s attribute map is empty; otherwise true.

- [ ] [getAttributeNames()](https://dom.spec.whatwg.org/#dom-processinginstruction-getattributenames) — The `getAttributeNames()` method steps are to return the result of getting the keys of this’s attribute map.

- [ ] [getAttribute(name)](https://dom.spec.whatwg.org/#dom-processinginstruction-getattribute) — The `getAttribute(name)` method steps are to return the result of getting a processing instruction attribute given this and name.

- [ ] [setAttribute(name, value)](https://dom.spec.whatwg.org/#dom-processinginstruction-setattribute) — The `setAttribute(name, value)` method steps are:
  - [ ] 1. If name is not a valid attribute local name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. Set this’s attribute map\[name\] to value.
  - [ ] 3. Update data from attributes given this.

- [ ] [removeAttribute(name)](https://dom.spec.whatwg.org/#dom-processinginstruction-removeattribute) — The `removeAttribute(name)` method steps are:
  - [ ] 1. Remove this’s attribute map\[name\].
  - [ ] 2. Update data from attributes given this.

- [ ] [toggleAttribute(name, force)](https://dom.spec.whatwg.org/#dom-processinginstruction-toggleattribute) — The `toggleAttribute(name, force)` method steps are:
  - [ ] 1. If name is not a valid attribute local name, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. Let attributes be this’s attribute map.
  - [ ] 3. If attributes\[name\] does not exist:
    - [ ] 1. If force is not given or is true, set attributes\[name\] to the empty string, update data from attributes given this, and then return true.
    - [ ] 2. Return false.
  - [ ] 4. If force is not given or is false, remove attributes\[name\], update data from attributes given this, and then return false.
  - [ ] 5. Return true.

- [ ] [hasAttribute(name)](https://dom.spec.whatwg.org/#dom-processinginstruction-hasattribute) — The `hasAttribute(name)` method steps are to return true if this’s attribute map\[name\] exists; otherwise false.

### [4.14. Interface Comment](https://dom.spec.whatwg.org/#interface-comment)

```webidl
[Exposed=Window]
interface Comment : CharacterData {
  constructor(optional DOMString data = "");
};
```

> **For web developers:**
>
> - `comment = new Comment([data = ""])`
>   Returns a new `Comment` node whose data is data.

- [ ] [create a comment node](https://dom.spec.whatwg.org/#create-a-comment-node) — To create a comment node given a document document and a string data:
  - [ ] 1. Let comment be the result of creating a node that implements `Comment`, given document.
  - [ ] 2. Set comment’s data to data.
  - [ ] 3. Return comment.

- [ ] [new Comment(data)](https://dom.spec.whatwg.org/#dom-comment-comment) — The `new Comment(data)` constructor steps are to set this’s data to data and this’s node document to current global object’s associated `Document`.

## [5. Ranges](https://dom.spec.whatwg.org/#ranges)

### [5.1. Introduction to "DOM Ranges"](https://dom.spec.whatwg.org/#introduction-to-dom-ranges)

`StaticRange` and `Range` objects (ranges) represent a sequence of content within a node tree. Each range has a start and an end which are boundary points. A boundary point is a tuple consisting of a node and an offset. So in other words, a range represents a piece of content within a node tree between two boundary points.

Ranges are frequently used in editing for selecting and copying content.

- `Element`: `p`
   - `Element`: `<img src="insanity-wolf" alt="Little-endian BOM; decode as big-endian!">`
   - `Text`: CSS 2.1 syndata is
   - `Element`: `<em>`
      - `Text`: awesome
   - `Text`: !

In the node tree above, a range can be used to represent the sequence “syndata is awes”. Assuming p is assigned to the `p` element, and em to the `em` element, this would be done as follows:

```js
var range = new Range(),
    firstText = p.childNodes[1],
    secondText = em.firstChild
range.setStart(firstText, 9) // do not forget the leading space
range.setEnd(secondText, 4)
// range now stringifies to the aforementioned quote
```

> **Note:** Attributes such as `src` and `alt` in the node tree above cannot be represented by a range. Ranges are only useful for nodes.

`Range` objects, unlike `StaticRange` objects, are affected by mutations to the node tree. Therefore they are also known as live ranges. Such mutations will not invalidate them and will try to ensure that it still represents the same piece of content. Necessarily, a live range might itself be modified as part of the mutation to the node tree when, e.g., part of the content it represents is mutated.

> **Note:** See the insert and remove algorithms, the `normalize()` method, and the replace data and split algorithms for details.

Updating live ranges in response to node tree mutations can be expensive. For every node tree change, all affected `Range` objects need to be updated. Even if the application is uninterested in some live ranges, it still has to pay the cost of keeping them up-to-date when a mutation occurs.

A `StaticRange` object is a lightweight range that does not update when the node tree mutates. It is therefore not subject to the same maintenance cost as live ranges.

### [5.2. Boundary points](https://dom.spec.whatwg.org/#boundary-points)

A [**boundary point**](https://dom.spec.whatwg.org/#concept-range-bp) is a tuple consisting of a [**node**](https://dom.spec.whatwg.org/#boundary-point-node) (a node) and an [**offset**](https://dom.spec.whatwg.org/#concept-range-bp-offset) (a non-negative integer).

> **Note:** A correct boundary point’s offset will be between 0 and the boundary point’s node’s length, inclusive.

- [ ] [position](https://dom.spec.whatwg.org/#concept-range-bp-position) — The position of a boundary point (nodeA, offsetA) relative to a boundary point (nodeB, offsetB) is before, equal, or after, as returned by these steps:
  - [ ] 1. Assert: nodeA and nodeB have the same root.
  - [ ] 2. If nodeA is nodeB, then return equal if offsetA is offsetB, before if offsetA is less than offsetB, and after if offsetA is greater than offsetB.
  - [ ] 3. If nodeA is following nodeB, then if the position of (nodeB, offsetB) relative to (nodeA, offsetA) is before, return after, and if it is after, return before.
  - [ ] 4. If nodeA is an ancestor of nodeB:
    - [ ] 1. Let child be nodeB.
    - [ ] 2. While child is not a child of nodeA, set child to its parent.
    - [ ] 3. If child’s index is less than offsetA, then return after.
  - [ ] 5. Return before.

### [5.3. Interface AbstractRange](https://dom.spec.whatwg.org/#interface-abstractrange)

```webidl
[Exposed=Window]
interface AbstractRange {
  readonly attribute Node startContainer;
  readonly attribute unsigned long startOffset;
  readonly attribute Node endContainer;
  readonly attribute unsigned long endOffset;
  readonly attribute boolean collapsed;
};
```

Objects implementing the `AbstractRange` interface are known as [**ranges**](https://dom.spec.whatwg.org/#concept-range).

A range has two associated boundary points — a [**start**](https://dom.spec.whatwg.org/#concept-range-start) and [**end**](https://dom.spec.whatwg.org/#concept-range-end).

For convenience, a range’s [**start node**](https://dom.spec.whatwg.org/#concept-range-start-node) is its start’s node, its [**start offset**](https://dom.spec.whatwg.org/#concept-range-start-offset) is its start’s offset, its [**end node**](https://dom.spec.whatwg.org/#concept-range-end-node) is its end’s node, and its [**end offset**](https://dom.spec.whatwg.org/#concept-range-end-offset) is its end’s offset.

- [ ] [collapsed](https://dom.spec.whatwg.org/#range-collapsed) — A range is collapsed if its start node is its end node and its start offset is its end offset.

> **For web developers:**
>
> - `node = range.startContainer`
>   Returns range’s start node.
> - `offset = range.startOffset`
>   Returns range’s start offset.
> - `node = range.endContainer`
>   Returns range’s end node.
> - `offset = range.endOffset`
>   Returns range’s end offset.
> - `collapsed = range.collapsed`
>   Returns true if range is collapsed; otherwise false.

- [ ] [startContainer](https://dom.spec.whatwg.org/#dom-range-startcontainer) — The `startContainer` getter steps are to return this’s start node.

- [ ] [startOffset](https://dom.spec.whatwg.org/#dom-range-startoffset) — The `startOffset` getter steps are to return this’s start offset.

- [ ] [endContainer](https://dom.spec.whatwg.org/#dom-range-endcontainer) — The `endContainer` getter steps are to return this’s end node.

- [ ] [endOffset](https://dom.spec.whatwg.org/#dom-range-endoffset) — The `endOffset` getter steps are to return this’s end offset.

- [ ] [collapsed](https://dom.spec.whatwg.org/#dom-range-collapsed) — The `collapsed` getter steps are to return true if this is collapsed; otherwise false.

### [5.4. Interface StaticRange](https://dom.spec.whatwg.org/#interface-staticrange)

```webidl
dictionary StaticRangeInit {
  required Node startContainer;
  required unsigned long startOffset;
  required Node endContainer;
  required unsigned long endOffset;
};

[Exposed=Window]
interface StaticRange : AbstractRange {
  constructor(StaticRangeInit init);
};
```

> **For web developers:**
>
> - `staticRange = new StaticRange(init)`
>   Returns a new range object that does not update when the node tree mutates.

- [ ] [new StaticRange(init)](https://dom.spec.whatwg.org/#dom-staticrange-staticrange) — The `new StaticRange(init)` constructor steps are:
  - [ ] 1. If init\["`startContainer`"\] or init\["`endContainer`"\] is a `DocumentType` or `Attr` node, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 2. Set this’s start to (init\["`startContainer`"\], init\["`startOffset`"\]) and end to (init\["`endContainer`"\], init\["`endOffset`"\]).

- [ ] [valid](https://dom.spec.whatwg.org/#staticrange-valid) — A `StaticRange` is valid if all of the following are true:

- Its start and end are in the same node tree.
- Its start offset is between 0 and its start node’s length, inclusive.
- Its end offset is between 0 and its end node’s length, inclusive.
- Its start is before or equal to its end.

### [5.5. Interface Range](https://dom.spec.whatwg.org/#interface-range)

```webidl
[Exposed=Window]
interface Range : AbstractRange {
  constructor();

  readonly attribute Node commonAncestorContainer;

  undefined setStart(Node node, unsigned long offset);
  undefined setEnd(Node node, unsigned long offset);
  undefined setStartBefore(Node node);
  undefined setStartAfter(Node node);
  undefined setEndBefore(Node node);
  undefined setEndAfter(Node node);
  undefined collapse(optional boolean toStart = false);
  undefined selectNode(Node node);
  undefined selectNodeContents(Node node);

  const unsigned short START_TO_START = 0;
  const unsigned short START_TO_END = 1;
  const unsigned short END_TO_END = 2;
  const unsigned short END_TO_START = 3;
  short compareBoundaryPoints(unsigned short how, Range sourceRange);

  [CEReactions] undefined deleteContents();
  [CEReactions, NewObject] DocumentFragment extractContents();
  [CEReactions, NewObject] DocumentFragment cloneContents();
  [CEReactions] undefined insertNode(Node node);
  [CEReactions] undefined surroundContents(Node newParent);

  [NewObject] Range cloneRange();
  undefined detach();

  boolean isPointInRange(Node node, unsigned long offset);
  short comparePoint(Node node, unsigned long offset);

  boolean intersectsNode(Node node);

  stringifier;
};
```

Objects implementing the `Range` interface are known as [**live ranges**](https://dom.spec.whatwg.org/#concept-live-range).

> **Note:** Algorithms that modify a tree (in particular the insert, remove, move, replace data, and split algorithms) modify live ranges associated with that tree.

The [**root**](https://dom.spec.whatwg.org/#concept-range-root) of a live range is the root of its start node.

A node node is [**contained**](https://dom.spec.whatwg.org/#contained) in a live range range if node’s root is range’s root, and (node, 0) is after range’s start, and (node, node’s length) is before range’s end.

A node is [**partially contained**](https://dom.spec.whatwg.org/#partially-contained) in a live range if it’s an inclusive ancestor of the live range’s start node but not its end node, or vice versa.

> **Note:**
>
> Some facts to better understand these definitions:
>
> - The content that one would think of as being within the live range consists of all contained nodes, plus possibly some of the contents of the start node and end node if those are `CharacterData` nodes.
> - The nodes that are contained in a live range will generally not be contiguous, because the parent of a contained node will not always be contained.
> - However, the descendants of a contained node are contained, and if two siblings are contained, so are any siblings that lie between them.
> - The start node and end node of a live range are never contained within it.
> - The first contained node (if there are any) will always be after the start node, and the last contained node will always be equal to or before the end node’s last descendant.
> - There exists a partially contained node if and only if the start node and end node are different.
> - The `commonAncestorContainer` attribute value is neither contained nor partially contained.
> - If the start node is an ancestor of the end node, the common inclusive ancestor will be the start node. Exactly one of its children will be partially contained, and a child will be contained if and only if it precedes the partially contained child. If the end node is an ancestor of the start node, the opposite holds.
> - If the start node is not an inclusive ancestor of the end node, nor vice versa, the common inclusive ancestor will be distinct from both of them. Exactly two of its children will be partially contained, and a child will be contained if and only if it lies between those two.

- [ ] [live range pre-remove steps](https://dom.spec.whatwg.org/#live-range-pre-remove-steps) — The live range pre-remove steps given a node node, are:
  - [ ] 1. Let parent be node’s parent.
  - [ ] 2. Assert: parent is non-null.
  - [ ] 3. Let index be node’s index.
  - [ ] 4. For each live range whose start node is an inclusive descendant of node, set its start to (parent, index).
  - [ ] 5. For each live range whose end node is an inclusive descendant of node, set its end to (parent, index).
  - [ ] 6. For each live range whose start node is parent and start offset is greater than index, decrease its start offset by 1.
  - [ ] 7. For each live range whose end node is parent and end offset is greater than index, decrease its end offset by 1.

> **For web developers:**
>
> - `range = new Range()`
>   Returns a new live range.

- [ ] [new Range()](https://dom.spec.whatwg.org/#dom-range-range) — The `new Range()` constructor steps are to set this’s start and end to (current global object’s associated `Document`, 0).

> **For web developers:**
>
> - container = range. `commonAncestorContainer`
>   Returns the node, furthest away from the document, that is an ancestor of both range’s start node and end node.

- [ ] [get the common ancestor](https://dom.spec.whatwg.org/#get-the-common-ancestor) — To get the common ancestor of a live range range:
  - [ ] 1. Let container be range’s start node.
  - [ ] 2. While container is not an inclusive ancestor of range’s end node: set container to container’s parent.
  - [ ] 3. Return container.

- [ ] [commonAncestorContainer](https://dom.spec.whatwg.org/#dom-range-commonancestorcontainer) — The `commonAncestorContainer` getter steps are to return the result of getting the common ancestor of this.

- [ ] [set the start or end](https://dom.spec.whatwg.org/#concept-range-bp-set) — To set the start or end of a range to a boundary point (node, offset):
  - [ ] 1. If node is a doctype, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 2. If offset is greater than node’s length, then throw an "`IndexSizeError`" `DOMException`.
  - [ ] 3. Let boundaryPoint be the boundary point (node, offset).
  - [ ] 4. If these steps were invoked as "set the start" If these steps were invoked as "set the end"
    - Case If these steps were invoked as "set the start":
      - [ ] 1. If range’s root is not node’s root or boundaryPoint is after range’s end, then set range’s end to boundaryPoint.
      - [ ] 2. Set range’s start to boundaryPoint.
    - Case If these steps were invoked as "set the end":
      - [ ] 1. If range’s root is not node’s root or boundaryPoint is before range’s start, then set range’s start to boundaryPoint.
      - [ ] 2. Set range’s end to boundaryPoint.

- [ ] [setStart(node, offset)](https://dom.spec.whatwg.org/#dom-range-setstart) — The `setStart(node, offset)` method steps are to set the start of this to boundary point (node, offset).

- [ ] [setEnd(node, offset)](https://dom.spec.whatwg.org/#dom-range-setend) — The `setEnd(node, offset)` method steps are to set the end of this to boundary point (node, offset).

- [ ] [setStartBefore(node)](https://dom.spec.whatwg.org/#dom-range-setstartbefore) — The `setStartBefore(node)` method steps are:
  - [ ] 1. Let parent be node’s parent.
  - [ ] 2. If parent is null, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 3. Set the start of this to boundary point (parent, node’s index).

- [ ] [setStartAfter(node)](https://dom.spec.whatwg.org/#dom-range-setstartafter) — The `setStartAfter(node)` method steps are:
  - [ ] 1. Let parent be node’s parent.
  - [ ] 2. If parent is null, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 3. Set the start of this to boundary point (parent, node’s index plus 1).

- [ ] [setEndBefore(node)](https://dom.spec.whatwg.org/#dom-range-setendbefore) — The `setEndBefore(node)` method steps are:
  - [ ] 1. Let parent be node’s parent.
  - [ ] 2. If parent is null, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 3. Set the end of this to boundary point (parent, node’s index).

- [ ] [setEndAfter(node)](https://dom.spec.whatwg.org/#dom-range-setendafter) — The `setEndAfter(node)` method steps are:
  - [ ] 1. Let parent be node’s parent.
  - [ ] 2. If parent is null, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 3. Set the end of this to boundary point (parent, node’s index plus 1).

- [ ] [collapse(toStart)](https://dom.spec.whatwg.org/#dom-range-collapse) — The `collapse(toStart)` method steps are to, if toStart is true, set end to start; otherwise set start to end.

- [ ] [select](https://dom.spec.whatwg.org/#concept-range-select) — To select a node node within a range range:
  - [ ] 1. Let parent be node’s parent.
  - [ ] 2. If parent is null, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 3. Let index be node’s index.
  - [ ] 4. Set range’s start to boundary point (parent, index).
  - [ ] 5. Set range’s end to boundary point (parent, index plus 1).

- [ ] [selectNode(node)](https://dom.spec.whatwg.org/#dom-range-selectnode) — The `selectNode(node)` method steps are to select node within this.

- [ ] [selectNodeContents(node)](https://dom.spec.whatwg.org/#dom-range-selectnodecontents) — The `selectNodeContents(node)` method steps are:
  - [ ] 1. If node is a doctype, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 2. Let length be the length of node.
  - [ ] 3. Set start to the boundary point (node, 0).
  - [ ] 4. Set end to the boundary point (node, length).

- [ ] [compareBoundaryPoints(how, sourceRange)](https://dom.spec.whatwg.org/#dom-range-compareboundarypoints) — The `compareBoundaryPoints(how, sourceRange)` method steps are:
  - [ ] 1. If how is not one of `START_TO_START`, `START_TO_END`, `END_TO_END`, and `END_TO_START`, then throw a "`NotSupportedError`" `DOMException`.
  - [ ] 2. If this’s root is not sourceRange’s root, then throw a "`WrongDocumentError`" `DOMException`.
  - [ ] 3. Let thisPoint and sourcePoint be null.
  - [ ] 4. Switch on how: `START_TO_START`: Set thisPoint to this’s start and sourcePoint to sourceRange’s start. `START_TO_END`: Set thisPoint to this’s end and sourcePoint to sourceRange’s start. `END_TO_END`: Set thisPoint to this’s end and sourcePoint to sourceRange’s end. `END_TO_START`: Set thisPoint to this’s start and sourcePoint to sourceRange’s end.
  - [ ] 5. Switch on the position of thisPoint relative to sourcePoint: before Return −1. equal Return 0. after Return 1.

- [ ] [deleteContents()](https://dom.spec.whatwg.org/#dom-range-deletecontents) — The `deleteContents()` method steps are:
  - [ ] 1. If this is collapsed, then return.
  - [ ] 2. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be this’s start node, start offset, end node, and end offset, respectively.
  - [ ] 3. If originalStartNode is originalEndNode and it is a `CharacterData` node:
    - [ ] 1. Replace data of originalStartNode with originalStartOffset, originalEndOffset − originalStartOffset, and the empty string.
    - [ ] 2. Return.
  - [ ] 4. Let nodesToRemove be a list of all the nodes that are contained in this, in tree order, omitting any node whose parent is also contained in this.
  - [ ] 5. Let newNode and newOffset be null.
  - [ ] 6. If originalStartNode is an inclusive ancestor of originalEndNode, then set newNode to originalStartNode and newOffset to originalStartOffset.
  - [ ] 7. Otherwise:
    - [ ] 1. Let referenceNode be originalStartNode.
    - [ ] 2. While referenceNode’s parent is non-null and is not an inclusive ancestor of originalEndNode: set referenceNode to its parent.
    - [ ] 3. Set newNode to referenceNode’s parent and newOffset to referenceNode’s index + 1.
  - [ ] 8. Set this’s start and end to (newNode, newOffset).
  - [ ] 9. If originalStartNode is a `CharacterData` node, then replace data of originalStartNode with originalStartOffset, originalStartNode’s length − originalStartOffset, and the empty string.
  - [ ] 10. For each node of nodesToRemove, in tree order: remove node.
  - [ ] 11. If originalEndNode is a `CharacterData` node, then replace data of originalEndNode with 0, originalEndOffset, and the empty string.

- [ ] [extract](https://dom.spec.whatwg.org/#concept-range-extract) — To extract a live range range:
  - [ ] 1. Let fragment be the result of creating a document fragment given range’s start node’s node document.
  - [ ] 2. If range is collapsed, then return fragment.
  - [ ] 3. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be range’s start node, start offset, end node, and end offset, respectively.
  - [ ] 4. If originalStartNode is originalEndNode and it is a `CharacterData` node:
    - [ ] 1. Let clone be a clone of originalStartNode.
    - [ ] 2. Set clone’s data to the result of substringing data of originalStartNode with originalStartOffset and originalEndOffset − originalStartOffset.
    - [ ] 3. Append clone to fragment.
    - [ ] 4. Replace data of originalStartNode with originalStartOffset, originalEndOffset − originalStartOffset, and the empty string.
    - [ ] 5. Return fragment.
  - [ ] 5. Let commonAncestor be the result of getting the common ancestor of range.
  - [ ] 6. Let firstPartiallyContainedChild be null.
  - [ ] 7. If originalStartNode is not an inclusive ancestor of originalEndNode, then set firstPartiallyContainedChild to the first child of commonAncestor that is partially contained in range.
  - [ ] 8. Let lastPartiallyContainedChild be null.
  - [ ] 9. If originalEndNode is not an inclusive ancestor of originalStartNode, then set lastPartiallyContainedChild to the last child of commonAncestor that is partially contained in range.
  - [ ] 10. Let containedChildren be a list of all children of commonAncestor that are contained in range, in tree order.
  - [ ] 11. If any member of containedChildren is a doctype, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 12. Let newNode and newOffset be null.
  - [ ] 13. If originalStartNode is an inclusive ancestor of originalEndNode, then set newNode to originalStartNode and newOffset to originalStartOffset.
  - [ ] 14. Otherwise:
    - [ ] 1. Let referenceNode be originalStartNode.
    - [ ] 2. While referenceNode’s parent is non-null and is not an inclusive ancestor of originalEndNode: set referenceNode to its parent.
    - [ ] 3. Set newNode to the parent of referenceNode, and newOffset to referenceNode’s index + 1.
  - [ ] 15. Set range’s start and end to (newNode, newOffset).
  - [ ] 16. If firstPartiallyContainedChild is a `CharacterData` node:
    - [ ] 1. Let clone be a clone of originalStartNode.
    - [ ] 2. Set the data of clone to the result of substringing data of originalStartNode with originalStartOffset and originalStartNode’s length − originalStartOffset.
    - [ ] 3. Append clone to fragment.
    - [ ] 4. Replace data of originalStartNode with originalStartOffset, originalStartNode’s length − originalStartOffset, and the empty string.
  - [ ] 17. Otherwise, if firstPartiallyContainedChild is non-null:
    - [ ] 1. Let clone be a clone of firstPartiallyContainedChild.
    - [ ] 2. Append clone to fragment.
    - [ ] 3. Let subrange be a new live range whose start is (originalStartNode, originalStartOffset) and whose end is (firstPartiallyContainedChild, firstPartiallyContainedChild’s length).
    - [ ] 4. Let subfragment be the result of extracting subrange.
    - [ ] 5. Append subfragment to clone.
  - [ ] 18. For each contained child of containedChildren: append contained child to fragment.
  - [ ] 19. If lastPartiallyContainedChild is a `CharacterData` node:
    - [ ] 1. Let clone be a clone of originalEndNode.
    - [ ] 2. Set clone’s data to the result of substringing data of originalEndNode with 0 and originalEndOffset.
    - [ ] 3. Append clone to fragment.
    - [ ] 4. Replace data of originalEndNode with 0, originalEndOffset, and the empty string.
  - [ ] 20. Otherwise, if lastPartiallyContainedChild is non-null:
    - [ ] 1. Let clone be a clone of lastPartiallyContainedChild.
    - [ ] 2. Append clone to fragment.
    - [ ] 3. Let subrange be a new live range whose start is (lastPartiallyContainedChild, 0) and whose end is (originalEndNode, originalEndOffset).
    - [ ] 4. Let subfragment be the result of extracting subrange.
    - [ ] 5. Append subfragment to clone.
  - [ ] 21. Return fragment.

- [ ] [extractContents()](https://dom.spec.whatwg.org/#dom-range-extractcontents) — The `extractContents()` method steps are to return the result of extracting this.

- [ ] [clone the contents](https://dom.spec.whatwg.org/#concept-range-clone) — To clone the contents of a live range range:
  - [ ] 1. Let fragment be the result of creating a document fragment given range’s start node’s node document.
  - [ ] 2. If range is collapsed, then return fragment.
  - [ ] 3. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be range’s start node, start offset, end node, and end offset, respectively.
  - [ ] 4. If originalStartNode is originalEndNode and it is a `CharacterData` node:
    - [ ] 1. Let clone be a clone of originalStartNode.
    - [ ] 2. Set clone’s data to the result of substringing data of originalStartNode with originalStartOffset and originalEndOffset − originalStartOffset.
    - [ ] 3. Append clone to fragment.
    - [ ] 4. Return fragment.
  - [ ] 5. Let commonAncestor be the result of getting the common ancestor of range.
  - [ ] 6. Let firstPartiallyContainedChild be null.
  - [ ] 7. If originalStartNode is not an inclusive ancestor of originalEndNode, then set firstPartiallyContainedChild to the first child of commonAncestor that is partially contained in range.
  - [ ] 8. Let lastPartiallyContainedChild be null.
  - [ ] 9. If originalEndNode is not an inclusive ancestor of originalStartNode, then set lastPartiallyContainedChild to the last child of commonAncestor that is partially contained in range.
  - [ ] 10. Let containedChildren be a list of all children of commonAncestor that are contained in range, in tree order.
  - [ ] 11. If any member of containedChildren is a doctype, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 12. If firstPartiallyContainedChild is a `CharacterData` node:
    - [ ] 1. Let clone be a clone of originalStartNode.
    - [ ] 2. Set the data of clone to the result of substringing data of originalStartNode with originalStartOffset and originalStartNode’s length − originalStartOffset.
    - [ ] 3. Append clone to fragment.
  - [ ] 13. Otherwise, if firstPartiallyContainedChild is non-null:
    - [ ] 1. Let clone be a clone of firstPartiallyContainedChild.
    - [ ] 2. Append clone to fragment.
    - [ ] 3. Let subrange be a new live range whose start is (originalStartNode, originalStartOffset) and whose end is (firstPartiallyContainedChild, firstPartiallyContainedChild’s length).
    - [ ] 4. Let subfragment be the result of cloning the contents of subrange.
    - [ ] 5. Append subfragment to clone.
  - [ ] 14. For each contained child of containedChildren:
    - [ ] 1. Let clone be a clone of contained child with subtree set to true.
    - [ ] 2. Append clone to fragment.
  - [ ] 15. If lastPartiallyContainedChild is a `CharacterData` node:
    - [ ] 1. Let clone be a clone of originalEndNode.
    - [ ] 2. Set clone’s data to the result of substringing data of originalEndNode with 0 and originalEndOffset.
    - [ ] 3. Append clone to fragment.
  - [ ] 16. Otherwise, if lastPartiallyContainedChild is non-null:
    - [ ] 1. Let clone be a clone of lastPartiallyContainedChild.
    - [ ] 2. Append clone to fragment.
    - [ ] 3. Let subrange be a new live range whose start is (lastPartiallyContainedChild, 0) and whose end is (originalEndNode, originalEndOffset).
    - [ ] 4. Let subfragment be the result of cloning the contents of subrange.
    - [ ] 5. Append subfragment to clone.
  - [ ] 17. Return fragment.

- [ ] [cloneContents()](https://dom.spec.whatwg.org/#dom-range-clonecontents) — The `cloneContents()` method steps are to return the result of cloning the contents of this.

- [ ] [insert](https://dom.spec.whatwg.org/#concept-range-insert) — To insert a node node into a live range range:
  - [ ] 1. If range’s start node is a `ProcessingInstruction` or `Comment` node, is a `Text` node whose parent is null, or is node, then throw a "`HierarchyRequestError`" `DOMException`.
  - [ ] 2. Let referenceNode be null.
  - [ ] 3. If range’s start node is a `Text` node, then set referenceNode to that `Text` node.
  - [ ] 4. Otherwise, set referenceNode to the child of range’s start node whose index is range’s start offset if there is such a child; otherwise null.
  - [ ] 5. Let parent be range’s start node if referenceNode is null; otherwise referenceNode’s parent.
  - [ ] 6. Ensure pre-insert validity given node, parent, referenceNode, and « ».
  - [ ] 7. If range’s start node is a `Text` node, then set referenceNode to the result of splitting it with offset range’s start offset.
  - [ ] 8. If node is referenceNode, then set referenceNode to its next sibling.
  - [ ] 9. If node’s parent is non-null, then remove node.
  - [ ] 10. Let newOffset be parent’s length if referenceNode is null; otherwise referenceNode’s index.
  - [ ] 11. Increase newOffset by node’s length if node is a `DocumentFragment` node; otherwise 1.
  - [ ] 12. Pre-insert node into parent before referenceNode.
  - [ ] 13. If range is collapsed, then set range’s end to (parent, newOffset).

- [ ] [insertNode(node)](https://dom.spec.whatwg.org/#dom-range-insertnode) — The `insertNode(node)` method steps are to insert node into this.

- [ ] [surroundContents(newParent)](https://dom.spec.whatwg.org/#dom-range-surroundcontents) — The `surroundContents(newParent)` method steps are:
  - [ ] 1. If a non-`Text` node is partially contained in this, then throw an "`InvalidStateError`" `DOMException`.
  - [ ] 2. If newParent is a `Document`, `DocumentType`, or `DocumentFragment` node, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 3. Let fragment be the result of extracting this.
  - [ ] 4. If newParent has children, then replace all with null within newParent.
  - [ ] 5. Insert newParent into this.
  - [ ] 6. Append fragment to newParent.
  - [ ] 7. Select newParent within this.

- [ ] [cloneRange()](https://dom.spec.whatwg.org/#dom-range-clonerange) — The `cloneRange()` method steps are to return a new live range with the same start and end as this.

- [ ] [detach()](https://dom.spec.whatwg.org/#dom-range-detach) — The `detach()` method steps are to do nothing.

> **For web developers:**
>
> - position = range. `comparePoint(node, offset)`
>   Returns −1 if the point is before the range, 0 if the point is in the range, and 1 if the point is after the range.
> - intersects = range. `intersectsNode(node)`
>   Returns whether range intersects node.

- [ ] [isPointInRange(node, offset)](https://dom.spec.whatwg.org/#dom-range-ispointinrange) — The `isPointInRange(node, offset)` method steps are:
  - [ ] 1. If node’s root is not this’s root, then return false.
  - [ ] 2. If node is a doctype, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 3. If offset is greater than node’s length, then throw an "`IndexSizeError`" `DOMException`.
  - [ ] 4. If (node, offset) is before start or after end, then return false.
  - [ ] 5. Return true.

- [ ] [comparePoint(node, offset)](https://dom.spec.whatwg.org/#dom-range-comparepoint) — The `comparePoint(node, offset)` method steps are:
  - [ ] 1. If node’s root is not this’s root, then throw a "`WrongDocumentError`" `DOMException`.
  - [ ] 2. If node is a doctype, then throw an "`InvalidNodeTypeError`" `DOMException`.
  - [ ] 3. If offset is greater than node’s length, then throw an "`IndexSizeError`" `DOMException`.
  - [ ] 4. If (node, offset) is before start, then return −1.
  - [ ] 5. If (node, offset) is after end, then return 1.
  - [ ] 6. Return 0.

- [ ] [intersectsNode(node)](https://dom.spec.whatwg.org/#dom-range-intersectsnode) — The `intersectsNode(node)` method steps are:
  - [ ] 1. If node’s root is not this’s root, then return false.
  - [ ] 2. Let parent be node’s parent.
  - [ ] 3. If parent is null, then return true.
  - [ ] 4. Let offset be node’s index.
  - [ ] 5. If (parent, offset) is before end and (parent, offset + 1) is after start, then return true.
  - [ ] 6. Return false.

- [ ] [stringification behavior](https://dom.spec.whatwg.org/#dom-range-stringifier) — The stringification behavior must run these steps:
  - [ ] 1. Let string be the empty string.
  - [ ] 2. If this’s start node is this’s end node and it is a `Text` node, then return the substring of that `Text` node’s data beginning at this’s start offset and ending at this’s end offset.
  - [ ] 3. If this’s start node is a `Text` node, then append the substring of that node’s data from this’s start offset until the end to string.
  - [ ] 4. Append the concatenation of the data of all `Text` nodes that are contained in this, in tree order, to string.
  - [ ] 5. If this’s end node is a `Text` node, then append the substring of that node’s data from its start until this’s end offset to string.
  - [ ] 6. Return string.

> **Note:** The `createContextualFragment()`, `getClientRects()`, and `getBoundingClientRect()` methods are defined in other specifications. \[DOM-Parsing\] \[CSSOM-VIEW\]

## [6. Traversal](https://dom.spec.whatwg.org/#traversal)

`NodeIterator` and `TreeWalker` objects can be used to filter and traverse node trees.

Each `NodeIterator` and `TreeWalker` object has an associated boolean [**is active**](https://dom.spec.whatwg.org/#concept-traversal-active) to avoid recursive invocations. It is initially false.

Each `NodeIterator` and `TreeWalker` object also has an associated [**root**](https://dom.spec.whatwg.org/#concept-traversal-root) (a node), a [**whatToShow**](https://dom.spec.whatwg.org/#concept-traversal-whattoshow) (a bitmask), and a [**filter**](https://dom.spec.whatwg.org/#concept-traversal-filter) (a callback).

- [ ] [filter](https://dom.spec.whatwg.org/#concept-node-filter) — To filter a node node within a `NodeIterator` or `TreeWalker` object traverser:
  - [ ] 1. If traverser’s is active is true, then throw an "`InvalidStateError`" `DOMException`.
  - [ ] 2. Let n be node’s `nodeType` attribute value − 1.
  - [ ] 3. If the nth bit (where 0 is the least significant bit) of traverser’s whatToShow is not set, then return `FILTER_SKIP`.
  - [ ] 4. If traverser’s filter is null, then return `FILTER_ACCEPT`.
  - [ ] 5. Set traverser’s is active to true.
  - [ ] 6. Let result be the return value of call a user object’s operation with traverser’s filter, "`acceptNode`", and « node ». If this throws an exception, then set traverser’s is active to false and rethrow the exception.
  - [ ] 7. Set traverser’s is active to false.
  - [ ] 8. Return result.

### [6.1. Interface NodeIterator](https://dom.spec.whatwg.org/#interface-nodeiterator)

```webidl
[Exposed=Window]
interface NodeIterator {
  [SameObject] readonly attribute Node root;
  readonly attribute Node referenceNode;
  readonly attribute boolean pointerBeforeReferenceNode;
  readonly attribute unsigned long whatToShow;
  readonly attribute NodeFilter? filter;

  Node? nextNode();
  Node? previousNode();

  undefined detach();
};
```

> **Note:** `NodeIterator` objects can be created using the `createNodeIterator()` method on `Document` objects.

Each `NodeIterator` object has an associated [**iterator collection**](https://dom.spec.whatwg.org/#iterator-collection), which is a collection rooted at the `NodeIterator` object’s root, whose filter matches any node.

A [**node pointer**](https://dom.spec.whatwg.org/#node-pointer) is a tuple consisting of a [**node**](https://dom.spec.whatwg.org/#node-pointer-node) (a node) and a [**pointer before**](https://dom.spec.whatwg.org/#node-pointer-pointer-before) (a boolean).

Each `NodeIterator` object also has an associated [**reference**](https://dom.spec.whatwg.org/#nodeiterator-reference) (a node pointer) and [**candidate reference**](https://dom.spec.whatwg.org/#nodeiterator-candidate-reference) (null or a node pointer, and initially null).

> **Note:** As mentioned earlier, `NodeIterator` objects have an associated is active, root, whatToShow, and filter as well.

- [ ] [NodeIterator pre-remove steps](https://dom.spec.whatwg.org/#nodeiterator-pre-removing-steps) — The `NodeIterator` pre-remove steps given a `NodeIterator` object nodeIterator and node toBeRemovedNode, are:
  - [ ] 1. Set nodeIterator’s reference to the result of adjusting a node pointer given nodeIterator’s reference, nodeIterator, and toBeRemovedNode.
  - [ ] 2. If nodeIterator’s candidate reference is non-null, then set nodeIterator’s candidate reference to the result of adjusting a node pointer given nodeIterator’s candidate reference, nodeIterator, and toBeRemovedNode.

- [ ] [adjust a node pointer](https://dom.spec.whatwg.org/#nodeiterator-adjust-a-node-pointer) — To adjust a node pointer given a node pointer nodePointer, a `NodeIterator` object nodeIterator, and a node toBeRemovedNode, perform the following steps. They return a node pointer.
  - [ ] 1. If toBeRemovedNode is not an inclusive ancestor of nodePointer’s node, or toBeRemovedNode is an inclusive ancestor of nodeIterator’s root, then return nodePointer.
  - [ ] 2. If nodePointer’s pointer before is true:
    - [ ] 1. Let next be toBeRemovedNode’s first following node that is an inclusive descendant of nodeIterator’s root and is not an inclusive descendant of toBeRemovedNode, if there is such a node; otherwise null.
    - [ ] 2. If next is non-null, then return (next, true).
  - [ ] 3. Let newNode be toBeRemovedNode’s parent, if toBeRemovedNode’s previous sibling is null; otherwise the inclusive descendant of toBeRemovedNode’s previous sibling that appears last in tree order.
  - [ ] 4. Return (newNode, false).

- [ ] [root](https://dom.spec.whatwg.org/#dom-nodeiterator-root) — The `root` getter steps are to return this’s root.

- [ ] [referenceNode](https://dom.spec.whatwg.org/#dom-nodeiterator-referencenode) — The `referenceNode` getter steps are to return this’s reference’s node.

- [ ] [pointerBeforeReferenceNode](https://dom.spec.whatwg.org/#dom-nodeiterator-pointerbeforereferencenode) — The `pointerBeforeReferenceNode` getter steps are to return this’s reference’s pointer before.

- [ ] [whatToShow](https://dom.spec.whatwg.org/#dom-nodeiterator-whattoshow) — The `whatToShow` getter steps are to return this’s whatToShow.

- [ ] [filter](https://dom.spec.whatwg.org/#dom-nodeiterator-filter) — The `filter` getter steps are to return this’s filter.

- [ ] [traverse](https://dom.spec.whatwg.org/#concept-nodeiterator-traverse) — To traverse, given a `NodeIterator` object iterator and "`next`" or "`previous`" type:
  - [ ] 1. Set iterator’s candidate reference to iterator’s reference.
  - [ ] 2. Let result be null.
  - [ ] 3. While true:
    - [ ] 1. If type is "`next`":
      - [ ] 1. If iterator’s candidate reference’s pointer before is false:
        - [ ] 1. Let following be the first node following iterator’s candidate reference’s node in iterator’s iterator collection. If there is no such node, then break.
        - [ ] 2. Set iterator’s candidate reference to (following, false).
      - [ ] 2. Otherwise, set iterator’s candidate reference to (iterator’s candidate reference’s node, false).
    - [ ] 2. Otherwise:
      - [ ] 1. If iterator’s candidate reference’s pointer before is true:
        - [ ] 1. Let preceding be the first node preceding iterator’s candidate reference’s node in iterator’s iterator collection. If there is no such node, then break.
        - [ ] 2. Set iterator’s candidate reference to (preceding, true).
      - [ ] 2. Otherwise, set iterator’s candidate reference to (iterator’s candidate reference’s node, true).
    - [ ] 3. Let node be iterator’s candidate reference’s node.
    - [ ] 4. Let filterResult be the result of filtering node within iterator. If this throws an exception, then set iterator’s candidate reference to null and rethrow that exception.
    - [ ] 5. If filterResult is `FILTER_ACCEPT`:
      - [ ] 1. Set iterator’s reference to iterator’s candidate reference.
      - [ ] 2. Set result to node.
      - [ ] 3. Break.
  - [ ] 4. Set iterator’s candidate reference to null.
  - [ ] 5. Return result.

- [ ] [nextNode()](https://dom.spec.whatwg.org/#dom-nodeiterator-nextnode) — The `nextNode()` method steps are to return the result of traversing with this and "`next`".

- [ ] [previousNode()](https://dom.spec.whatwg.org/#dom-nodeiterator-previousnode) — The `previousNode()` method steps are to return the result of traversing with this and "`previous`".

- [ ] [detach()](https://dom.spec.whatwg.org/#dom-nodeiterator-detach) — The `detach()` method steps are to do nothing.

### [6.2. Interface TreeWalker](https://dom.spec.whatwg.org/#interface-treewalker)

```webidl
[Exposed=Window]
interface TreeWalker {
  [SameObject] readonly attribute Node root;
  readonly attribute unsigned long whatToShow;
  readonly attribute NodeFilter? filter;
           attribute Node currentNode;

  Node? parentNode();
  Node? firstChild();
  Node? lastChild();
  Node? previousSibling();
  Node? nextSibling();
  Node? previousNode();
  Node? nextNode();
};
```

> **Note:** `TreeWalker` objects can be created using the `createTreeWalker()` method on `Document` objects.

Each `TreeWalker` object has an associated [**current**](https://dom.spec.whatwg.org/#treewalker-current) (a node).

> **Note:** As mentioned earlier `TreeWalker` objects have an associated root, whatToShow, and filter as well.

- [ ] [root](https://dom.spec.whatwg.org/#dom-treewalker-root) — The `root` getter steps are to return this’s root.

- [ ] [whatToShow](https://dom.spec.whatwg.org/#dom-treewalker-whattoshow) — The `whatToShow` getter steps are to return this’s whatToShow.

- [ ] [filter](https://dom.spec.whatwg.org/#dom-treewalker-filter) — The `filter` getter steps are to return this’s filter.

- [ ] [currentNode](https://dom.spec.whatwg.org/#dom-treewalker-currentnode) — The `currentNode` getter steps are to return this’s current.

- [ ] The `currentNode` setter steps are to set this’s current to the given value.

- [ ] [parentNode()](https://dom.spec.whatwg.org/#dom-treewalker-parentnode) — The `parentNode()` method steps are:
  - [ ] 1. Let node be this’s current.
  - [ ] 2. While node is non-null and is not this’s root:
    - [ ] 1. Set node to node’s parent.
    - [ ] 2. If node is non-null and filtering node within this returns `FILTER_ACCEPT`, then set this’s current to node and return node.
  - [ ] 3. Return null.

- [ ] [traverse children](https://dom.spec.whatwg.org/#concept-traverse-children) — To traverse children, given a `TreeWalker` object walker and "`first`" or "`last`" type:
  - [ ] 1. Let node be walker’s current.
  - [ ] 2. Set node to node’s first child if type is "`first`"; otherwise to node’s last child.
  - [ ] 3. While node is non-null:
    - [ ] 1. Let result be the result of filtering node within walker.
    - [ ] 2. If result is `FILTER_ACCEPT`, then set walker’s current to node and return node.
    - [ ] 3. If result is `FILTER_SKIP`:
      - [ ] 1. Let child be node’s first child if type is "`first`"; otherwise node’s last child.
      - [ ] 2. If child is non-null, then set node to child and continue.
    - [ ] 4. While node is non-null:
      - [ ] 1. Let sibling be node’s next sibling if type is "`first`"; otherwise node’s previous sibling.
      - [ ] 2. If sibling is non-null, then set node to sibling and break.
      - [ ] 3. Let parent be node’s parent.
      - [ ] 4. If parent is null, walker’s root, or walker’s current, then return null.
      - [ ] 5. Set node to parent.
  - [ ] 4. Return null.

- [ ] [firstChild()](https://dom.spec.whatwg.org/#dom-treewalker-firstchild) — The `firstChild()` method steps are to traverse children with this and "`first`".

- [ ] [lastChild()](https://dom.spec.whatwg.org/#dom-treewalker-lastchild) — The `lastChild()` method steps are to traverse children with this and "`last`".

- [ ] [traverse siblings](https://dom.spec.whatwg.org/#concept-traverse-siblings) — To traverse siblings, given a `TreeWalker` object walker and "`next`" or "`previous`" type:
  - [ ] 1. Let node be walker’s current.
  - [ ] 2. If node is root, then return null.
  - [ ] 3. While true:
    - [ ] 1. Let sibling be node’s next sibling if type is "`next`"; otherwise node’s previous sibling.
    - [ ] 2. While sibling is non-null:
      - [ ] 1. Set node to sibling.
      - [ ] 2. Let result be the result of filtering node within walker.
      - [ ] 3. If result is `FILTER_ACCEPT`, then set walker’s current to node and return node.
      - [ ] 4. Set sibling to node’s first child if type is "`next`"; otherwise to node’s last child.
      - [ ] 5. If result is `FILTER_REJECT` or sibling is null, then set sibling to node’s next sibling if type is "`next`"; otherwise to node’s previous sibling.
    - [ ] 3. Set node to node’s parent.
    - [ ] 4. If node is null or walker’s root, then return null.
    - [ ] 5. If the return value of filtering node within walker is `FILTER_ACCEPT`, then return null.

- [ ] [nextSibling()](https://dom.spec.whatwg.org/#dom-treewalker-nextsibling) — The `nextSibling()` method steps are to traverse siblings with this and "`next`".

- [ ] [previousSibling()](https://dom.spec.whatwg.org/#dom-treewalker-previoussibling) — The `previousSibling()` method steps are to traverse siblings with this and "`previous`".

- [ ] [previousNode()](https://dom.spec.whatwg.org/#dom-treewalker-previousnode) — The `previousNode()` method steps are:
  - [ ] 1. Let node be this’s current.
  - [ ] 2. While node is not this’s root:
    - [ ] 1. Let sibling be node’s previous sibling.
    - [ ] 2. While sibling is non-null:
      - [ ] 1. Set node to sibling.
      - [ ] 2. Let result be the result of filtering node within this.
      - [ ] 3. While result is not `FILTER_REJECT` and node has a child:
        - [ ] 1. Set node to node’s last child.
        - [ ] 2. Set result to the result of filtering node within this.
      - [ ] 4. If result is `FILTER_ACCEPT`, then set this’s current to node and return node.
      - [ ] 5. Set sibling to node’s previous sibling.
    - [ ] 3. If node is this’s root or node’s parent is null, then return null.
    - [ ] 4. Set node to node’s parent.
    - [ ] 5. If the return value of filtering node within this is `FILTER_ACCEPT`, then set this’s current to node and return node.
  - [ ] 3. Return null.

- [ ] [nextNode()](https://dom.spec.whatwg.org/#dom-treewalker-nextnode) — The `nextNode()` method steps are:
  - [ ] 1. Let node be this’s current.
  - [ ] 2. Let result be `FILTER_ACCEPT`.
  - [ ] 3. While true:
    - [ ] 1. While result is not `FILTER_REJECT` and node has a child:
      - [ ] 1. Set node to its first child.
      - [ ] 2. Set result to the result of filtering node within this.
      - [ ] 3. If result is `FILTER_ACCEPT`, then set this’s current to node and return node.
    - [ ] 2. Let sibling be null.
    - [ ] 3. Let temporary be node.
    - [ ] 4. While temporary is non-null:
      - [ ] 1. If temporary is this’s root, then return null.
      - [ ] 2. Set sibling to temporary’s next sibling.
      - [ ] 3. If sibling is non-null, then break.
      - [ ] 4. Set temporary to temporary’s parent.
    - [ ] 5. If sibling is null, then return null.
    - [ ] 6. Set node to sibling.
    - [ ] 7. Set result to the result of filtering node within this.
    - [ ] 8. If result is `FILTER_ACCEPT`, then set this’s current to node and return node.

### [6.3. Interface NodeFilter](https://dom.spec.whatwg.org/#interface-nodefilter)

```webidl
[Exposed=Window]
callback interface NodeFilter {
  // Constants for acceptNode()
  const unsigned short FILTER_ACCEPT = 1;
  const unsigned short FILTER_REJECT = 2;
  const unsigned short FILTER_SKIP = 3;

  // Constants for whatToShow
  const unsigned long SHOW_ALL = 0xFFFFFFFF;
  const unsigned long SHOW_ELEMENT = 0x1;
  const unsigned long SHOW_ATTRIBUTE = 0x2;
  const unsigned long SHOW_TEXT = 0x4;
  const unsigned long SHOW_CDATA_SECTION = 0x8;
  const unsigned long SHOW_ENTITY_REFERENCE = 0x10; // legacy
  const unsigned long SHOW_ENTITY = 0x20; // legacy
  const unsigned long SHOW_PROCESSING_INSTRUCTION = 0x40;
  const unsigned long SHOW_COMMENT = 0x80;
  const unsigned long SHOW_DOCUMENT = 0x100;
  const unsigned long SHOW_DOCUMENT_TYPE = 0x200;
  const unsigned long SHOW_DOCUMENT_FRAGMENT = 0x400;
  const unsigned long SHOW_NOTATION = 0x800; // legacy

  unsigned short acceptNode(Node node);
};
```

> **Note:** `NodeFilter` objects can be used as filter for `NodeIterator` and `TreeWalker` objects and also provide constants for their whatToShow bitmask. A `NodeFilter` object is typically implemented as a JavaScript function.

These constants can be used as filter return value:

- [**FILTER\_ACCEPT**](https://dom.spec.whatwg.org/#dom-nodefilter-filter_accept) (1);
- [**FILTER\_REJECT**](https://dom.spec.whatwg.org/#dom-nodefilter-filter_reject) (2);
- [**FILTER\_SKIP**](https://dom.spec.whatwg.org/#dom-nodefilter-filter_skip) (3).

These constants can be used for whatToShow:

- [**SHOW\_ALL**](https://dom.spec.whatwg.org/#dom-nodefilter-show_all) (4294967295, FFFFFFFF in hexadecimal);
- [**SHOW\_ELEMENT**](https://dom.spec.whatwg.org/#dom-nodefilter-show_element) (1);
- [**SHOW\_ATTRIBUTE**](https://dom.spec.whatwg.org/#dom-nodefilter-show_attribute) (2);
- [**SHOW\_TEXT**](https://dom.spec.whatwg.org/#dom-nodefilter-show_text) (4);
- [**SHOW\_CDATA\_SECTION**](https://dom.spec.whatwg.org/#dom-nodefilter-show_cdata_section) (8);
- [**SHOW\_PROCESSING\_INSTRUCTION**](https://dom.spec.whatwg.org/#dom-nodefilter-show_processing_instruction) (64, 40 in hexadecimal);
- [**SHOW\_COMMENT**](https://dom.spec.whatwg.org/#dom-nodefilter-show_comment) (128, 80 in hexadecimal);
- [**SHOW\_DOCUMENT**](https://dom.spec.whatwg.org/#dom-nodefilter-show_document) (256, 100 in hexadecimal);
- [**SHOW\_DOCUMENT\_TYPE**](https://dom.spec.whatwg.org/#dom-nodefilter-show_document_type) (512, 200 in hexadecimal);
- [**SHOW\_DOCUMENT\_FRAGMENT**](https://dom.spec.whatwg.org/#dom-nodefilter-show_document_fragment) (1024, 400 in hexadecimal).

## [7. Sets](https://dom.spec.whatwg.org/#sets)

> **Note:** Yes, the name `DOMTokenList` is an unfortunate legacy mishap.

### [7.1. Interface DOMTokenList](https://dom.spec.whatwg.org/#interface-domtokenlist)

```webidl
[Exposed=Window]
interface DOMTokenList {
  readonly attribute unsigned long length;
  getter DOMString? item(unsigned long index);
  boolean contains(DOMString token);
  [CEReactions] undefined add(DOMString... tokens);
  [CEReactions] undefined remove(DOMString... tokens);
  [CEReactions] boolean toggle(DOMString token, optional boolean force);
  [CEReactions] boolean replace(DOMString token, DOMString newToken);
  boolean supports(DOMString token);
  [CEReactions] stringifier attribute DOMString value;
  iterable<DOMString>;
};
```

A `DOMTokenList` object has an associated [**token set**](https://dom.spec.whatwg.org/#concept-dtl-tokens) (a set), which is initially empty.

A `DOMTokenList` object also has an associated [**element**](https://dom.spec.whatwg.org/#domtokenlist-element) (an element) and an [**attribute name**](https://dom.spec.whatwg.org/#domtokenlist-attribute-name) (an attribute’s local name).

Specifications may define [**supported tokens**](https://dom.spec.whatwg.org/#concept-supported-tokens) for a `DOMTokenList`’s element and attribute name.

- [ ] [validation steps](https://dom.spec.whatwg.org/#concept-domtokenlist-validation) — A `DOMTokenList` object set’s validation steps for a given token are:
  - [ ] 1. If set’s element and attribute name does not define supported tokens, then throw a `TypeError`.
  - [ ] 2. Let lowercaseToken be token, in ASCII lowercase.
  - [ ] 3. If lowercaseToken is present in the supported tokens of set’s element and attribute name, then return true.
  - [ ] 4. Return false.

- [ ] [update steps](https://dom.spec.whatwg.org/#concept-dtl-update) — A `DOMTokenList` object set’s update steps are:
  - [ ] 1. If get an attribute by namespace and local name given null, set’s attribute name, and set’s element returns null and set’s token set is empty, then return.
  - [ ] 2. Set an attribute value given set’s element, set’s attribute name, and the result of running the ordered set serializer for set’s token set.

- [ ] [serialize steps](https://dom.spec.whatwg.org/#concept-dtl-serialize) — A `DOMTokenList` object set’s serialize steps are to return the result of running get an attribute value given set’s element and set’s attribute name.

- [ ] A `DOMTokenList` object set has these attribute change steps for set’s element:
  - [ ] 1. If localName is set’s attribute name, namespace is null, and value is null, then empty token set.
  - [ ] 2. Otherwise, if localName is set’s attribute name and namespace is null, then set set’s token set to value, parsed.

- [ ] When a `DOMTokenList` object set is created:
  - [ ] 1. Let element be set’s element.
  - [ ] 2. Let attributeName be set’s attribute name.
  - [ ] 3. Let value be the result of getting an attribute value given element and attributeName.
  - [ ] 4. Run the attribute change steps for element, attributeName, value, value, and null.

> **For web developers:**
>
> - `tokenlist.length`
>   Returns the number of tokens.
> - `tokenlist.item(index)` `tokenlist[index]`
>   Returns the token with index index.
> - `tokenlist.contains(token)`
>   Returns true if token is present; otherwise false.
> - `tokenlist.add(tokens…)`
>   Adds all arguments passed, except those already present. Throws a "`SyntaxError`" `DOMException` if one of the arguments is the empty string. Throws an "`InvalidCharacterError`" `DOMException` if one of the arguments contains any ASCII whitespace.
> - `tokenlist.remove(tokens…)`
>   Removes arguments passed, if they are present. Throws a "`SyntaxError`" `DOMException` if one of the arguments is the empty string. Throws an "`InvalidCharacterError`" `DOMException` if one of the arguments contains any ASCII whitespace.
> - `tokenlist.toggle(token [, force])`
>   If force is not given, "toggles" token, removing it if it’s present and adding it if it’s not present. If force is true, adds token (same as `add()`). If force is false, removes token (same as `remove()`). Returns true if token is now present; otherwise false. Throws a "`SyntaxError`" `DOMException` if token is empty. Throws an "`InvalidCharacterError`" `DOMException` if token contains any spaces.
> - `tokenlist.replace(token, newToken)`
>   Replaces token with newToken. Returns true if token was replaced with newToken; otherwise false. Throws a "`SyntaxError`" `DOMException` if one of the arguments is the empty string. Throws an "`InvalidCharacterError`" `DOMException` if one of the arguments contains any ASCII whitespace.
> - `tokenlist.supports(token)`
>   Returns true if token is in the associated attribute’s supported tokens. Returns false otherwise. Throws a `TypeError` if the associated attribute has no supported tokens defined.
> - `tokenlist.value`
>   Returns the associated set as string. Can be set, to change the associated attribute.

- [ ] [length](https://dom.spec.whatwg.org/#dom-domtokenlist-length) — The `length` getter steps are to return this’s token set’s size.

The object’s supported property indices are the numbers in the range zero to object’s token set’s size − 1, unless token set is empty, in which case there are no supported property indices.

- [ ] [item(index)](https://dom.spec.whatwg.org/#dom-domtokenlist-item) — The `item(index)` method steps are:
  - [ ] 1. If index is equal to or greater than this’s token set’s size, then return null.
  - [ ] 2. Return this’s token set\[index\].

- [ ] [contains(token)](https://dom.spec.whatwg.org/#dom-domtokenlist-contains) — The `contains(token)` method steps are to return true if this’s token set\[token\] exists; otherwise false.

- [ ] [add(tokens…)](https://dom.spec.whatwg.org/#dom-domtokenlist-add) — The `add(tokens…)` method steps are:
  - [ ] 1. For each token of tokens:
    - [ ] 1. If token is the empty string, then throw a "`SyntaxError`" `DOMException`.
    - [ ] 2. If token contains any ASCII whitespace, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. For each token of tokens: append token to this’s token set.
  - [ ] 3. Run the update steps.

- [ ] [remove(tokens…)](https://dom.spec.whatwg.org/#dom-domtokenlist-remove) — The `remove(tokens…)` method steps are:
  - [ ] 1. For each token of tokens:
    - [ ] 1. If token is the empty string, then throw a "`SyntaxError`" `DOMException`.
    - [ ] 2. If token contains any ASCII whitespace, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 2. For each token of tokens: remove token from this’s token set.
  - [ ] 3. Run the update steps.

- [ ] [toggle(token, force)](https://dom.spec.whatwg.org/#dom-domtokenlist-toggle) — The `toggle(token, force)` method steps are:
  - [ ] 1. If token is the empty string, then throw a "`SyntaxError`" `DOMException`.
  - [ ] 2. If token contains any ASCII whitespace, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 3. If this’s token set\[token\] exists:
    - [ ] 1. If force is either not given or is false, then remove token from this’s token set, run the update steps and return false.
    - [ ] 2. Return true.
  - [ ] 4. Otherwise, if force not given or is true, append token to this’s token set, run the update steps, and return true.
  - [ ] 5. Return false.

> **Note:** The update steps are not always run for `toggle()` for web compatibility.

- [ ] [replace(token, newToken)](https://dom.spec.whatwg.org/#dom-domtokenlist-replace) — The `replace(token, newToken)` method steps are:
  - [ ] 1. If either token or newToken is the empty string, then throw a "`SyntaxError`" `DOMException`.
  - [ ] 2. If either token or newToken contains any ASCII whitespace, then throw an "`InvalidCharacterError`" `DOMException`.
  - [ ] 3. If this’s token set does not contain token, then return false.
  - [ ] 4. Replace token in this’s token set with newToken.
  - [ ] 5. Run the update steps.
  - [ ] 6. Return true.

> **Note:** The update steps are not always run for `replace()` for web compatibility.

- [ ] [supports(token)](https://dom.spec.whatwg.org/#dom-domtokenlist-supports) — The `supports(token)` method steps are:
  - [ ] 1. Let result be the return value of validation steps called with token.
  - [ ] 2. Return result.

- [ ] [value](https://dom.spec.whatwg.org/#dom-domtokenlist-value) — The `value` getter steps are to return the result of running this’s serialize steps.

- [ ] The `value` setter steps are to set an attribute value for this’s element using this’s attribute name and the given value.

## [8. XPath](https://dom.spec.whatwg.org/#xpath)

> **Issue:** DOM Level 3 XPath defined an API for evaluating XPath 1.0 expressions. These APIs are widely implemented, but have not been maintained. The interface definitions are maintained here so that they can be updated when Web IDL changes. Complete definitions of these APIs remain necessary and such work is tracked and can be contributed to in whatwg/dom#67. \[DOM-Level-3-XPath\] \[XPath\] \[WEBIDL\]

### [8.1. Interface XPathResult](https://dom.spec.whatwg.org/#interface-xpathresult)

```webidl
[Exposed=Window]
interface XPathResult {
  const unsigned short ANY_TYPE = 0;
  const unsigned short NUMBER_TYPE = 1;
  const unsigned short STRING_TYPE = 2;
  const unsigned short BOOLEAN_TYPE = 3;
  const unsigned short UNORDERED_NODE_ITERATOR_TYPE = 4;
  const unsigned short ORDERED_NODE_ITERATOR_TYPE = 5;
  const unsigned short UNORDERED_NODE_SNAPSHOT_TYPE = 6;
  const unsigned short ORDERED_NODE_SNAPSHOT_TYPE = 7;
  const unsigned short ANY_UNORDERED_NODE_TYPE = 8;
  const unsigned short FIRST_ORDERED_NODE_TYPE = 9;

  readonly attribute unsigned short resultType;
  readonly attribute unrestricted double numberValue;
  readonly attribute DOMString stringValue;
  readonly attribute boolean booleanValue;
  readonly attribute Node? singleNodeValue;
  readonly attribute boolean invalidIteratorState;
  readonly attribute unsigned long snapshotLength;

  Node? iterateNext();
  Node? snapshotItem(unsigned long index);
};
```

### [8.2. Interface XPathExpression](https://dom.spec.whatwg.org/#interface-xpathexpression)

```webidl
[Exposed=Window]
interface XPathExpression {
  // XPathResult.ANY_TYPE = 0
  XPathResult evaluate(Node contextNode, optional unsigned short type = 0, optional XPathResult? result = null);
};
```

### [8.3. Mixin XPathEvaluatorBase](https://dom.spec.whatwg.org/#mixin-xpathevaluatorbase)

```webidl
callback interface XPathNSResolver {
  DOMString? lookupNamespaceURI(DOMString? prefix);
};

interface mixin XPathEvaluatorBase {
  [NewObject] XPathExpression createExpression(DOMString expression, optional XPathNSResolver? resolver = null);
  Node createNSResolver(Node nodeResolver); // legacy
  // XPathResult.ANY_TYPE = 0
  XPathResult evaluate(DOMString expression, Node contextNode, optional XPathNSResolver? resolver = null, optional unsigned short type = 0, optional XPathResult? result = null);
};
Document includes XPathEvaluatorBase;
```

- [ ] [createNSResolver(nodeResolver)](https://dom.spec.whatwg.org/#dom-xpathevaluatorbase-creatensresolver) — The `createNSResolver(nodeResolver)` method steps are to return nodeResolver.

> **Note:** This method exists only for historical reasons.

### [8.4. Interface XPathEvaluator](https://dom.spec.whatwg.org/#interface-xpathevaluator)

```webidl
[Exposed=Window]
interface XPathEvaluator {
  constructor();
};

XPathEvaluator includes XPathEvaluatorBase;
```

> **Note:** For historical reasons you can both construct `XPathEvaluator` and access the same methods on `Document`.

## [9. XSLT](https://dom.spec.whatwg.org/#xslt)

> **Warning:** Avoid using client-side XSLT in browsers. The browser implementations of XSLT are highly susceptible to memory safety vulnerabilities, and the web platform has evolved toward JavaScript as the safe replacement. For these reasons, this feature is in the process of being removed from the web platform. (This is a long process that takes many years.)

> **Issue:** XSL Transformations (XSLT) is a language for transforming XML documents into other XML documents. The APIs defined in this section have been widely implemented, and are maintained here so that they can be updated when Web IDL changes. Complete definitions of these APIs remain necessary and such work is tracked and can be contributed to in whatwg/dom#181. \[XSLT\]

### [9.1. Interface XSLTProcessor](https://dom.spec.whatwg.org/#interface-xsltprocessor)

```webidl
[Exposed=Window]
interface XSLTProcessor {
  constructor();
  undefined importStylesheet(Node style);
  [CEReactions] DocumentFragment transformToFragment(Node source, Document output);
  [CEReactions] Document transformToDocument(Node source);
  undefined setParameter([LegacyNullToEmptyString] DOMString namespaceURI, DOMString localName, any value);
  any getParameter([LegacyNullToEmptyString] DOMString namespaceURI, DOMString localName);
  undefined removeParameter([LegacyNullToEmptyString] DOMString namespaceURI, DOMString localName);
  undefined clearParameters();
  undefined reset();
};
```

## [10. Security and privacy considerations](https://dom.spec.whatwg.org/#security-and-privacy)

There are no known security or privacy considerations for this standard.

## [11. Historical](https://dom.spec.whatwg.org/#historical)

This standard used to contain several interfaces and interface members that have been removed.

These interfaces have been removed:

- [**DOMConfiguration**](https://dom.spec.whatwg.org/#domconfiguration)
- [**DOMError**](https://dom.spec.whatwg.org/#domerror)
- [**DOMErrorHandler**](https://dom.spec.whatwg.org/#domerrorhandler)
- [**DOMImplementationList**](https://dom.spec.whatwg.org/#domimplementationlist)
- [**DOMImplementationSource**](https://dom.spec.whatwg.org/#domimplementationsource)
- [**DOMLocator**](https://dom.spec.whatwg.org/#domlocator)
- [**DOMObject**](https://dom.spec.whatwg.org/#domobject)
- [**DOMUserData**](https://dom.spec.whatwg.org/#domuserdata)
- [**Entity**](https://dom.spec.whatwg.org/#entity)
- [**EntityReference**](https://dom.spec.whatwg.org/#entityreference)
- [**MutationEvent**](https://dom.spec.whatwg.org/#mutationevent)
- [**MutationNameEvent**](https://dom.spec.whatwg.org/#mutationnameevent)
- [**NameList**](https://dom.spec.whatwg.org/#namelist)
- [**Notation**](https://dom.spec.whatwg.org/#notation)
- [**RangeException**](https://dom.spec.whatwg.org/#rangeexception)
- [**TypeInfo**](https://dom.spec.whatwg.org/#typeinfo)
- [**UserDataHandler**](https://dom.spec.whatwg.org/#userdatahandler)

And these interface members have been removed:

- **`Attr`**

  - [**schemaTypeInfo**](https://dom.spec.whatwg.org/#dom-attr-schematypeinfo)
  - [**isId**](https://dom.spec.whatwg.org/#dom-attr-isid)
- **`Document`**

  - [**createEntityReference()**](https://dom.spec.whatwg.org/#dom-document-createentityreference)
  - [**xmlEncoding**](https://dom.spec.whatwg.org/#dom-document-xmlencoding)
  - [**xmlStandalone**](https://dom.spec.whatwg.org/#dom-document-xmlstandalone)
  - [**xmlVersion**](https://dom.spec.whatwg.org/#dom-document-xmlversion)
  - [**strictErrorChecking**](https://dom.spec.whatwg.org/#dom-document-stricterrorchecking)
  - [**domConfig**](https://dom.spec.whatwg.org/#dom-document-domconfig)
  - [**normalizeDocument()**](https://dom.spec.whatwg.org/#dom-document-normalizedocument)
  - [**renameNode()**](https://dom.spec.whatwg.org/#dom-document-renamenode)
- **`DocumentType`**

  - [**entities**](https://dom.spec.whatwg.org/#dom-documenttype-entities)
  - [**notations**](https://dom.spec.whatwg.org/#dom-documenttype-notations)
  - [**internalSubset**](https://dom.spec.whatwg.org/#dom-documenttype-internalsubset)
- **`DOMImplementation`**

  - [**getFeature()**](https://dom.spec.whatwg.org/#dom-domimplementation-getfeature)
- **`Element`**

  - [**schemaTypeInfo**](https://dom.spec.whatwg.org/#dom-element-schematypeinfo)
  - [**setIdAttribute()**](https://dom.spec.whatwg.org/#dom-element-setidattribute)
  - [**setIdAttributeNS()**](https://dom.spec.whatwg.org/#dom-element-setidattributens)
  - [**setIdAttributeNode()**](https://dom.spec.whatwg.org/#dom-element-setidattributenode)
- **`Node`**

  - [**isSupported**](https://dom.spec.whatwg.org/#dom-node-issupported)
  - [**getFeature()**](https://dom.spec.whatwg.org/#dom-node-getfeature)
  - [**getUserData()**](https://dom.spec.whatwg.org/#dom-node-getuserdata)
  - [**setUserData()**](https://dom.spec.whatwg.org/#dom-node-setuserdata)
- **`NodeIterator`**

  - [**expandEntityReferences**](https://dom.spec.whatwg.org/#dom-nodeiterator-expandentityreferences)
- **`Text`**

  - [**isElementContentWhitespace**](https://dom.spec.whatwg.org/#dom-text-iselementcontentwhitespace)
  - [**replaceWholeText()**](https://dom.spec.whatwg.org/#dom-text-replacewholetext)
- **`TreeWalker`**

  - [**expandEntityReferences**](https://dom.spec.whatwg.org/#dom-treewalker-expandentityreferences)
