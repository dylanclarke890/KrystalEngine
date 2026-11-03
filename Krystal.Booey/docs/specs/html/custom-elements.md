# HTML review checklist: 4.13 Custom elements

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 28 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html custom-elements` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.13 Custom elements](https://html.spec.whatwg.org/#custom-elements)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.13.1 Introduction](https://html.spec.whatwg.org/#custom-elements-intro)

Status: n/a (introduction)

##### [4.13.1.1 Creating an autonomous custom element](https://html.spec.whatwg.org/#custom-elements-autonomous-example)

Status: n/a (example)

##### [4.13.1.2 Creating a form-associated custom element](https://html.spec.whatwg.org/#custom-elements-face-example)

Status: n/a (example)

##### [4.13.1.3 Creating a custom element with default accessible roles, states, and properties](https://html.spec.whatwg.org/#custom-elements-accessibility-example)

Status: n/a (example)

##### [4.13.1.4 Creating a customized built-in element](https://html.spec.whatwg.org/#custom-elements-customized-builtin-example)

Status: n/a (example; no customized built-in elements)

##### [4.13.1.5 Drawbacks of autonomous custom elements](https://html.spec.whatwg.org/#custom-elements-autonomous-drawbacks)

Status: n/a (advice)

##### [4.13.1.6 Upgrading elements after their creation](https://html.spec.whatwg.org/#custom-elements-upgrades-examples)

Status: n/a (example)

##### [4.13.1.7 Scoped custom element registries](https://html.spec.whatwg.org/#scoped-custom-element-registries)

Status: n/a (example)

##### [4.13.1.8 Exposing custom element states](https://html.spec.whatwg.org/#exposing-custom-element-states)

Status: n/a (example)

#### [4.13.2 Requirements for custom element constructors and reactions](https://html.spec.whatwg.org/#custom-element-conformance)

Status: n/a (author conformance)

##### [4.13.2.1 Preserving custom element state when moved](https://html.spec.whatwg.org/#preserving-custom-element-state-when-moved)

Status: n/a (author conformance)

#### [4.13.3 Core concepts](https://html.spec.whatwg.org/#custom-elements-core-concepts)

Status: reviewed (2026-11-01)
> Definitions are `CustomElementInterface`s made from a factory and `CustomElementOptions` (see `OVERVIEW.md`); `CustomElementAlgorithms` looks up definitions and registries. There are no customized built-in elements (see `DIFFERENCES.md`).
<!-- definitions: custom-element custom-element-constructor autonomous-custom-element customized-built-in-element attr-is attr-face-readonly custom-element-definition concept-custom-element-definition-name concept-custom-element-definition-local-name concept-custom-element-definition-constructor concept-custom-element-definition-observed-attributes concept-custom-element-definition-lifecycle-callbacks concept-custom-element-definition-construction-stack concept-already-constructed-marker concept-custom-element-definition-form-associated concept-custom-element-definition-disable-internals concept-custom-element-definition-disable-shadow -->

- [x] [form-associated custom element](https://html.spec.whatwg.org/#form-associated-custom-element) — An autonomous custom element is called a form-associated custom element if the element is associated with a custom element definition whose form-associated field is set to true.

- [x] [valid custom element name](https://html.spec.whatwg.org/#valid-custom-element-name) — A string name is a valid custom element name if all of the following are true:

- [x] [look up a custom element definition](https://html.spec.whatwg.org/#look-up-a-custom-element-definition) — To look up a custom element definition, given null or a CustomElementRegistry object registry, string-or-null namespace, string localName, and string-or-null is, perform the following steps. They will return either a custom element definition or null:
  - 1. If registry is null, then return null.
  - 2. If namespace is not the HTML namespace, then return null.
  - 3. If registry's custom element definition set contains an item with name and local name both equal to localName, then return that item.
  - 4. If registry's custom element definition set contains an item with name equal to is and local name equal to localName, then return that item.
  - 5. Return null.

#### [4.13.4 The CustomElementRegistry interface](https://html.spec.whatwg.org/#custom-elements-api)

Status: reviewed (2026-11-01)
> `CustomElementRegistry`; `define()` takes a factory rather than a constructor, and `whenDefined()` a callback rather than returning a promise. `Window.customElements` is `LocalDOMWindow::CustomElements`.
> Fixed: `upgrade()` upgraded elements of other registries.
<!-- definitions: active-custom-element-constructor-map is-scoped scoped-document-set custom-element-definition-set element-definition-is-running when-defined-promise-map element-definition -->

- [x] [Window.customElements](https://html.spec.whatwg.org/#dom-window-customelements) — The Window customElements getter steps are:
  - 1. Assert: this's associated Document's custom element registry is a CustomElementRegistry object.
  - 2. Return this's associated Document's custom element registry.

- [x] [look up a custom element registry](https://html.spec.whatwg.org/#look-up-a-custom-element-registry) — To look up a custom element registry, given a Node object node:
  - 1. If node is an Element object, then return node's custom element registry.
  - 2. If node is a ShadowRoot object, then return node's custom element registry.
  - 3. If node is a Document object, then return node's custom element registry.
  - 4. Return null.

- [x] [new CustomElementRegistry()](https://html.spec.whatwg.org/#dom-customelementregistry) — The new CustomElementRegistry() constructor steps are to set this's is scoped to true.

- [x] [CustomElementRegistry.define(name, constructor, options)](https://html.spec.whatwg.org/#dom-customelementregistry-define) — The define(name, constructor, options) method steps are:
  - 1. If IsConstructor(constructor) is false, then throw a TypeError.
  - 2. If name is not a valid custom element name, then throw a "SyntaxError" DOMException.
  - 3. If this's custom element definition set contains an item with name name, then throw a "NotSupportedError" DOMException.
  - 4. If this's custom element definition set contains an item with constructor constructor, then throw a "NotSupportedError" DOMException.
  - 5. Let localName be name.
  - 6. Let extends be options["extends"] if it exists; otherwise null.
  - 7. If extends is not null:
    - 7a. If this's is scoped is true, then throw a "NotSupportedError" DOMException.
    - 7b. If extends is a valid custom element name, then throw a "NotSupportedError" DOMException.
    - 7c. If the element interface for extends and the HTML namespace is HTMLUnknownElement (e.g., if extends does not indicate an element definition in this specification), then throw a "NotSupportedError" DOMException.
    - 7d. Set localName to extends.
  - 8. If this's element definition is running is true, then throw a "NotSupportedError" DOMException.
  - 9. Set this's element definition is running to true.
  - 10. Let formAssociated be false.
  - 11. Let disableInternals be false.
  - 12. Let disableShadow be false.
  - 13. Let observedAttributes be an empty sequence<DOMString>.
  - 14. Run the following steps while catching any exceptions: Then, regardless of whether the above steps threw an exception or not: set this's element definition is running to false. Finally, if the steps threw an exception, rethrow that exception.
    - 14a. Let prototype be ? Get(constructor, "prototype").
    - 14b. If prototype is not an Object, then throw a TypeError exception.
    - 14c. Let lifecycleCallbacks be the ordered map «[ "connectedCallback" → null, "disconnectedCallback" → null, "connectedMoveCallback" → null, "adoptedCallback" → null, "attributeChangedCallback" → null ]».
    - 14d. For each callbackName of the keys of lifecycleCallbacks:
      - 14d.i. Let callbackValue be ? Get(prototype, callbackName).
      - 14d.ii. If callbackValue is not undefined, then set lifecycleCallbacks[callbackName] to the result of converting callbackValue to the Web IDL Function callback type.
    - 14e. If lifecycleCallbacks["attributeChangedCallback"] is not null:
      - 14e.i. Let observedAttributesIterable be ? Get(constructor, "observedAttributes").
      - 14e.ii. If observedAttributesIterable is not undefined, then set observedAttributes to the result of converting observedAttributesIterable to a sequence<DOMString>. Rethrow any exceptions from the conversion.
    - 14f. Let disabledFeatures be an empty sequence<DOMString>.
    - 14g. Let disabledFeaturesIterable be ? Get(constructor, "disabledFeatures").
    - 14h. If disabledFeaturesIterable is not undefined, then set disabledFeatures to the result of converting disabledFeaturesIterable to a sequence<DOMString>. Rethrow any exceptions from the conversion.
    - 14i. If disabledFeatures contains "internals", then set disableInternals to true.
    - 14j. If disabledFeatures contains "shadow", then set disableShadow to true.
    - 14k. Let formAssociatedValue be ? Get( constructor, "formAssociated").
    - 14l. Set formAssociated to the result of converting formAssociatedValue to a boolean.
    - 14m. If formAssociated is true, then for each callbackName of « "formAssociatedCallback", "formResetCallback", "formDisabledCallback", "formStateRestoreCallback" »:
      - 14m.i. Let callbackValue be ? Get(prototype, callbackName).
      - 14m.ii. If callbackValue is not undefined, then set lifecycleCallbacks[callbackName] to the result of converting callbackValue to the Web IDL Function callback type.
  - 15. Let definition be a new custom element definition with name name, local name localName, constructor constructor, observed attributes observedAttributes, lifecycle callbacks lifecycleCallbacks, form-associated formAssociated, disable internals disableInternals, and disable shadow disableShadow.
  - 16. Append definition to this's custom element definition set.
  - 17. If this's is scoped is true, then for each document of this's scoped document set: upgrade particular elements within a document given this, document, definition, and localName.
  - 18. Otherwise, upgrade particular elements within a document given this, this's relevant global object's associated Document, definition, localName, and name.
  - 19. If this's when-defined promise map[name] exists:
    - 19a. Resolve this's when-defined promise map[name] with constructor.
    - 19b. Remove this's when-defined promise map[name].

- [x] [upgrade particular elements within a document](https://html.spec.whatwg.org/#upgrade-particular-elements-within-a-document) — To upgrade particular elements within a document given a CustomElementRegistry object registry, a Document object document, a custom element definition definition, a string localName, and optionally a string name (default localName):
  - 1. Let upgradeCandidates be all elements that are shadow-including descendants of document, whose custom element registry is registry, whose namespace is the HTML namespace, and whose local name is localName, in shadow-including tree order. Additionally, if name is not localName, only include elements whose is value is equal to name.
  - 2. For each element element of upgradeCandidates: enqueue a custom element upgrade reaction given element and definition.

- [x] [CustomElementRegistry.get(name)](https://html.spec.whatwg.org/#dom-customelementregistry-get) — The get(name) method steps are:
  - 1. If this's custom element definition set contains an item with name name, then return that item's constructor.
  - 2. Return undefined.

- [x] [CustomElementRegistry.getName(constructor)](https://html.spec.whatwg.org/#dom-customelementregistry-getname) — The getName(constructor) method steps are:
  - 1. If this's custom element definition set contains an item with constructor constructor, then return that item's name.
  - 2. Return null.

- [x] [CustomElementRegistry.whenDefined(name)](https://html.spec.whatwg.org/#dom-customelementregistry-whendefined) — The whenDefined(name) method steps are:
  - 1. If name is not a valid custom element name, then return a promise rejected with a "SyntaxError" DOMException.
  - 2. If this's custom element definition set contains an item with name name, then return a promise resolved with that item's constructor.
  - 3. If this's when-defined promise map[name] does not exist, then set this's when-defined promise map[name] to a new promise.
  - 4. Return this's when-defined promise map[name].

- [x] [CustomElementRegistry.upgrade(root)](https://html.spec.whatwg.org/#dom-customelementregistry-upgrade) — The upgrade(root) method steps are:
  - 1. For each shadow-including inclusive descendant candidate of root, in shadow-including tree order:
    - 1a. If candidate is not an Element node, then continue.
    - 1b. If candidate's custom element registry is not this, then continue.
    - 1c. Try to upgrade candidate.

- [x] [CustomElementRegistry.initialize(root)](https://html.spec.whatwg.org/#dom-customelementregistry-initialize) — The initialize(root) method steps are:
  - 1. If this's is scoped is false and either root is a Document node or root's node document's custom element registry is not this, then throw a "NotSupportedError" DOMException.
  - 2. If root is a Document node whose custom element registry is null, then set root's custom element registry to this.
  - 3. Otherwise, if root is a ShadowRoot node whose custom element registry is null, then set root's custom element registry to this.
  - 4. For each inclusive descendant inclusiveDescendant of root, in tree order:
    - 4a. If inclusiveDescendant is not an Element node, then continue.
    - 4b. If inclusiveDescendant's custom element registry is null:
      - 4b.i. Set inclusiveDescendant's custom element registry to this.
      - 4b.ii. If this's is scoped is true, then append inclusiveDescendant's node document to this's scoped document set.
    - 4c. If inclusiveDescendant's custom element registry is not this, then continue.
    - 4d. Try to upgrade inclusiveDescendant.

#### [4.13.5 Upgrades](https://html.spec.whatwg.org/#upgrades)

Status: reviewed (2026-11-01)
> `CustomElementInterface::UpgradeElement` and `CustomElementReactionQueue::TryToUpgradeElement`; there's no construction stack or active constructor map, as the factory is given the element.

- [x] [upgrade an element](https://html.spec.whatwg.org/#concept-upgrade-an-element) — To upgrade an element, given as input a custom element definition definition and an element element, run the following steps:
  - 1. If element's custom element state is not "undefined" or "uncustomized", then return.
  - 2. Set element's custom element definition to definition.
  - 3. Set element's custom element state to "failed".
  - 4. For each attribute in element's attribute list, in order, enqueue a custom element callback reaction with element, callback name "attributeChangedCallback", and « attribute's local name, null, attribute's value, attribute's namespace ».
  - 5. If element is connected, then enqueue a custom element callback reaction with element, callback name "connectedCallback", and « ».
  - 6. Add element to the end of definition's construction stack.
  - 7. Let C be definition's constructor.
  - 8. Let previousRegistry be the surrounding agent's active custom element constructor map[C] with default null.
  - 9. Set the surrounding agent's active custom element constructor map[C] to element's custom element registry.
  - 10. Run the following steps while catching any exceptions: Then, perform the following steps, regardless of whether the above steps threw an exception or not: Finally, if the above steps threw an exception:
    - 10a. If definition's disable shadow is true and element's shadow root is non-null, then throw a "NotSupportedError" DOMException.
    - 10b. Set element's custom element state to "precustomized".
    - 10c. Let constructResult be the result of constructing C, with no arguments.
    - 10d. If SameValue(constructResult, element) is false, then throw a TypeError.
    - 10a. If previousRegistry is null, then remove the surrounding agent's active custom element constructor map[C].
    - 10b. Otherwise, set the surrounding agent's active custom element constructor map[C] to previousRegistry.
    - 10c. Remove the last entry from the end of definition's construction stack.
    - 10a. Set element's custom element definition to null.
    - 10b. Empty element's custom element reaction queue.
    - 10c. Rethrow the exception (thus terminating this algorithm).
  - 11. If element is a form-associated custom element:
    - 11a. Reset the form owner of element. If element is associated with a form element, then enqueue a custom element callback reaction with element, callback name "formAssociatedCallback", and « the associated form ».
    - 11b. If element is disabled, then enqueue a custom element callback reaction with element, callback name "formDisabledCallback", and « true ».
  - 12. Set element's custom element state to "custom".

- [x] [try to upgrade an element](https://html.spec.whatwg.org/#concept-try-upgrade) — To try to upgrade an element given an element element:
  - 1. Let definition be the result of looking up a custom element definition given element's custom element registry, element's namespace, element's local name, and element's is value.
  - 2. If definition is not null, then enqueue a custom element upgrade reaction given element and definition.

#### [4.13.6 Custom element reactions](https://html.spec.whatwg.org/#custom-element-reactions)

Status: reviewed (2026-11-01)
> `CustomElementReactionQueue`, `CustomElementQueue` and `CEReactionsScope`; every callback is enqueued, as behaviours are C++ classes (see `DIFFERENCES.md`), and the backup element queue is global (see `KNOWN-ISSUES.md`).
<!-- definitions: custom-element-reactions-stack current-element-queue element-queue backup-element-queue processing-the-backup-element-queue custom-element-reaction-queue upgrade-reaction callback-reaction -->

- [x] [custom element reactions](https://html.spec.whatwg.org/#concept-custom-element-reaction) — We call these reactions collectively custom element reactions.

- [x] [enqueue an element on the appropriate element queue](https://html.spec.whatwg.org/#enqueue-an-element-on-the-appropriate-element-queue) — To enqueue an element on the appropriate element queue, given an element element, run the following steps:
  - 1. Let reactionsStack be element's relevant agent's custom element reactions stack.
  - 2. If reactionsStack is empty:
    - 2a. Add element to reactionsStack's backup element queue.
    - 2b. If reactionsStack's processing the backup element queue flag is set, then return.
    - 2c. Set reactionsStack's processing the backup element queue flag.
    - 2d. Queue a microtask to perform the following steps:
      - 2d.i. Invoke custom element reactions in reactionsStack's backup element queue.
      - 2d.ii. Unset reactionsStack's processing the backup element queue flag.
  - 3. Otherwise, add element to element's relevant agent's current element queue.

- [x] [enqueue a custom element callback reaction](https://html.spec.whatwg.org/#enqueue-a-custom-element-callback-reaction) — To enqueue a custom element callback reaction, given a custom element element, a callback name callbackName, and a list of arguments args, run the following steps:
  - 1. Let definition be element's custom element definition.
  - 2. Let callback be the value of the entry in definition's lifecycle callbacks with key callbackName.
  - 3. If callbackName is "connectedMoveCallback" and callback is null:
    - 3a. Let disconnectedCallback be the value of the entry in definition's lifecycle callbacks with key "disconnectedCallback".
    - 3b. Let connectedCallback be the value of the entry in definition's lifecycle callbacks with key "connectedCallback".
    - 3c. If connectedCallback and disconnectedCallback are null, then return.
    - 3d. Set callback to the following steps:
      - 3d.i. If disconnectedCallback is not null, then call disconnectedCallback with no arguments.
      - 3d.ii. If connectedCallback is not null, then call connectedCallback with no arguments.
  - 4. If callback is null, then return.
  - 5. If callbackName is "attributeChangedCallback":
    - 5a. Let attributeName be the first element of args.
    - 5b. If definition's observed attributes does not contain attributeName, then return.
  - 6. Add a new callback reaction to element's custom element reaction queue, with callback function callback and arguments args.
  - 7. Enqueue an element on the appropriate element queue given element.

- [x] [enqueue a custom element upgrade reaction](https://html.spec.whatwg.org/#enqueue-a-custom-element-upgrade-reaction) — To enqueue a custom element upgrade reaction, given an element element and custom element definition definition, run the following steps:
  - 1. Add a new upgrade reaction to element's custom element reaction queue, with custom element definition definition.
  - 2. Enqueue an element on the appropriate element queue given element.

- [x] [invoke custom element reactions](https://html.spec.whatwg.org/#invoke-custom-element-reactions) — To invoke custom element reactions in an element queue queue, run the following steps:
  - 1. While queue is not empty:
    - 1a. Let element be the result of dequeuing from queue.
    - 1b. Let reactions be element's custom element reaction queue.
    - 1c. Repeat until reactions is empty:
      - 1c.i. Remove the first element of reactions, and let reaction be that element. Switch on reaction's type: upgrade reaction Upgrade element using reaction's custom element definition. If this throws an exception, catch it, and report it for reaction's custom element definition's constructor's corresponding JavaScript object's associated realm's global object. callback reaction Invoke reaction's callback…

- [x] [Operations, attributes, setters, or deleters annotated with the [CEReactions] e…](https://html.spec.whatwg.org/#cereactions) — Operations, attributes, setters, or deleters annotated with the [CEReactions] extended attribute must run the following steps in place of the ones specified in their description:
  - 1. Push a new element queue onto this object's relevant agent's custom element reactions stack.
  - 2. Run the originally-specified steps for this construct, catching any exceptions. If the steps return a value, let value be the returned value. If they throw an exception, let exception be the thrown exception.
  - 3. Let queue be the result of popping from this object's relevant agent's custom element reactions stack.
  - 4. Invoke custom element reactions in queue.
  - 5. If an exception exception was thrown by the original steps, rethrow exception.
  - 6. If a value value was returned from the original steps, return value.

#### [4.13.7 Element internals](https://html.spec.whatwg.org/#element-internals)

Status: reviewed (2026-11-01)
> `HTMLElement::AttachInternals`.
> Fixed: an element with an `is` value could attach internals.
<!-- definitions: attached-internals -->

- [x] [HTMLElement.attachInternals()](https://html.spec.whatwg.org/#dom-attachinternals) — The attachInternals() method steps are:
  - 1. If this's is value is not null, then throw a "NotSupportedError" DOMException.
  - 2. Let definition be the result of looking up a custom element definition given this's custom element registry, this's namespace, this's local name, and null.
  - 3. If definition is null, then throw a "NotSupportedError" DOMException.
  - 4. If definition's disable internals is true, then throw a "NotSupportedError" DOMException.
  - 5. If this's attached internals is non-null, then throw a "NotSupportedError" DOMException.
  - 6. If this's custom element state is not "precustomized" or "custom", then throw a "NotSupportedError" DOMException.
  - 7. Set this's attached internals to a new ElementInternals instance whose target element is this.
  - 8. Return this's attached internals.

##### [4.13.7.1 The ElementInternals interface](https://html.spec.whatwg.org/#the-elementinternals-interface)

Status: reviewed (2026-11-01)
> `ElementInternals`.
<!-- definitions: internals-target -->

##### [4.13.7.2 Shadow root access](https://html.spec.whatwg.org/#shadow-root-access)

Status: reviewed (2026-11-01)
> `ElementInternals::ShadowRoot`.

- [x] [ElementInternals.shadowRoot](https://html.spec.whatwg.org/#dom-elementinternals-shadowroot) — The shadowRoot getter steps are:
  - 1. Let target be this's target element.
  - 2. If target is not a shadow host, then return null.
  - 3. Let shadow be target's shadow root.
  - 4. If shadow's available to element internals is false, then return null.
  - 5. Return shadow.

##### [4.13.7.3 Form-associated custom elements](https://html.spec.whatwg.org/#form-associated-custom-elements)

Status: reviewed (2026-11-01)
> `FormAssociatedCustomElement`; state is restored only in the restore mode (see `KNOWN-ISSUES.md`).
> Fixed: `setValidity()` didn't normalize the message's newlines.
<!-- definitions: face-submission-value face-state face-validation-message face-validation-anchor -->

- [x] [ElementInternals.setFormValue(value, state)](https://html.spec.whatwg.org/#dom-elementinternals-setformvalue) — The setFormValue(value, state) method steps are:
  - 1. Let element be this's target element.
  - 2. If element is not a form-associated custom element, then throw a "NotSupportedError" DOMException.
  - 3. Set element's submission value to value if value is not a FormData object, or to a clone of value's entry list otherwise.
  - 4. If the state argument of the function is omitted, set element's state to its submission value.
  - 5. Otherwise, if state is a FormData object, set element's state to a clone of state's entry list.
  - 6. Otherwise, set element's state to state.

- [x] [ElementInternals.setValidity(flags, message, anchor)](https://html.spec.whatwg.org/#dom-elementinternals-setvalidity) — The setValidity(flags, message, anchor) method steps are:
  - 1. Let element be this's target element.
  - 2. If element is not a form-associated custom element, then throw a "NotSupportedError" DOMException.
  - 3. If message is not given, then set it to the empty string.
  - 4. If flags contains one or more true values and message is the empty string, then throw a TypeError.
  - 5. Set message to the result of normalizing newlines given message.
  - 6. For each entry flag → value of flags, set element's validity flag with the name flag to value.
  - 7. Set element's validation message to the empty string if all of element's validity flags are false; otherwise to message.
  - 8. If element's customError validity flag is true, then set element's custom validity error message to element's validation message. Otherwise, set element's custom validity error message to the empty string.
  - 9. If anchor is not given, then set it to element.
  - 10. Otherwise, if anchor is not a shadow-including inclusive descendant of element, then throw a "NotFoundError" DOMException.
  - 11. Set element's validation anchor to anchor.

- [x] [ElementInternals.validationMessage](https://html.spec.whatwg.org/#dom-elementinternals-validationmessage) — The validationMessage getter steps are:
  - 1. Let element be this's target element.
  - 2. If element is not a form-associated custom element, then throw a "NotSupportedError" DOMException.
  - 3. Return element's validation message.

- [x] [entry construction algorithm](https://html.spec.whatwg.org/#face-entry-construction) — The entry construction algorithm for a form-associated custom element, given an element element and an entry list entry list, consists of the following steps:
  - 1. If element's submission value is a list of entries, then append each item of element's submission value to entry list, and return.
  - 2. If the element does not have a name attribute specified, or its name attribute's value is the empty string, then return.
  - 3. If the element's submission value is not null, create an entry with the name attribute value and the submission value, and append it to entry list.

##### [4.13.7.4 Accessibility semantics](https://html.spec.whatwg.org/#accessibility-semantics)

Status: reviewed (2026-11-01)
> There's no ARIA reflection on `ElementInternals` (see `KNOWN-ISSUES.md`).
<!-- definitions: internal-content-attribute-map -->

##### [4.13.7.5 Custom state pseudo-class](https://html.spec.whatwg.org/#custom-state-pseudo-class)

Status: reviewed (2026-11-01)
> `CustomStateSet` and `:state()`.
<!-- definitions: states-set -->

- [x] [HTMLElement.states](https://html.spec.whatwg.org/#dom-elementinternals-states) — The states getter steps are to return this's target element's states set.
