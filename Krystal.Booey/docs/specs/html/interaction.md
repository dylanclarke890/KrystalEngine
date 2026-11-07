# HTML review checklist: 6 User interaction

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 100 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html interaction` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [6 User interaction](https://html.spec.whatwg.org/#editing)

Status: reviewed (2026-11-01)
> The sections below.

### [6.1 The hidden attribute](https://html.spec.whatwg.org/#the-hidden-attribute)

Status: reviewed (2026-11-01)
> `HTMLElement::Hidden` (the setter's steps are split between its overloads) and the ancestor revealing algorithm, `RevealClosedDetailsAndHiddenUntilFoundAncestors`.
<!-- definitions: attr-hidden ancestor-reveal-pair ancestor-reveal-pair-node ancestor-reveal-pair-string -->

- [x] [Not Hidden](https://html.spec.whatwg.org/#attr-hidden-not-hidden-state) — The attribute's missing value default is the Not Hidden state, and its invalid value default and empty value default are both the Hidden state.

- [x] [HTMLElement.hidden](https://html.spec.whatwg.org/#dom-hidden) — The hidden getter steps are:
  - 1. If the hidden attribute is in the Hidden Until Found state, then return "until-found".
  - 2. If the hidden attribute is set, then return true.
  - 3. Return false.

- [x] [The hidden setter steps](https://html.spec.whatwg.org/#dom-hidden) — The hidden setter steps are:
  - 1. If the given value is a string that is an ASCII case-insensitive match for "until-found", then set the hidden attribute to "until-found".
  - 2. Otherwise, if the given value is false, then remove the hidden attribute.
  - 3. Otherwise, if the given value is the empty string, then remove the hidden attribute.
  - 4. Otherwise, if the given value is null, then remove the hidden attribute.
  - 5. Otherwise, if the given value is 0, then remove the hidden attribute.
  - 6. Otherwise, if the given value is NaN, then remove the hidden attribute.
  - 7. Otherwise, set the hidden attribute to the empty string.

- [x] [ancestor revealing algorithm](https://html.spec.whatwg.org/#ancestor-revealing-algorithm) — The ancestor revealing algorithm given a node target is:
  - 1. Let ancestorsToReveal be « ».
  - 2. Let ancestor be target.
  - 3. While ancestor has a parent node within the flat tree:
    - 3a. If ancestor has a hidden attribute in the Hidden Until Found state, then append (ancestor, "until-found") to ancestorsToReveal.
    - 3b. If ancestor is slotted into the second slot of a details element which does not have an open attribute, then append (ancestor's parent node, "details") to ancestorsToReveal.
    - 3c. Set ancestor to the parent node of ancestor within the flat tree.
  - 4. For each (ancestorToReveal, revealType) of ancestorsToReveal:
    - 4a. If ancestorToReveal is not connected, then return.
    - 4b. If revealType is "until-found":
      - 4b.i. If ancestorToReveal's hidden attribute is not in the Hidden Until Found state, then return.
      - 4b.ii. Fire an event named beforematch at ancestorToReveal with the bubbles attribute initialized to true.
      - 4b.iii. If ancestorToReveal is not connected, then return.
      - 4b.iv. If ancestorToReveal's hidden attribute is not in the Hidden Until Found state, then return.
      - 4b.v. Remove the hidden attribute from ancestorToReveal.
    - 4c. Otherwise:
      - 4c.i. Assert: revealType is "details".
      - 4c.ii. If ancestorToReveal has an open attribute, then return.
      - 4c.iii. Set ancestorToReveal's open attribute to the empty string.

### [6.2 Page visibility](https://html.spec.whatwg.org/#page-visibility)

Status: reviewed (2026-11-01)
> A document's visibility state is its page's, updated as the host shows or hides the page (`Document::VisibilityStateChanged`), as upstream (see `DIFFERENCES.md`); unloading a document hides it for good (`Document::SetVisibilityHiddenDueToDismissal`). There's no initial visibility state to set without navigables.
<!-- definitions: visibility-state page-visibility-change-steps -->

- [x] [When a user agent determines that the system visibility state for traversable n…](https://html.spec.whatwg.org/#page-visibility) — When a user agent determines that the system visibility state for traversable navigable traversable has changed to newState, it must run the following steps:
  - 1. Let navigables be the inclusive descendant navigables of traversable's active document.
  - 2. For each navigable of navigables :
    - 2a. Let document be navigable's active document.
    - 2b. Queue a global task on the user interaction task source given document's relevant global object to update the visibility state of document with newState.

- [x] [Document.visibilityState](https://html.spec.whatwg.org/#dom-document-visibilitystate) — The visibilityState getter steps are to return this's visibility state.

- [x] [Document.hidden](https://html.spec.whatwg.org/#dom-document-hidden) — The hidden getter steps are to return true if this's visibility state is "hidden", otherwise false.

- [x] [update the visibility state](https://html.spec.whatwg.org/#update-the-visibility-state) — To update the visibility state of Document document to visibilityState:
  - 1. If document's visibility state equals visibilityState, then return.
  - 2. Set document's visibility state to visibilityState.
  - 3. Queue a new VisibilityStateEntry whose visibility state is visibilityState and whose timestamp is the current high resolution time given document's relevant global object.
  - 4. Run the screen orientation change steps with document. [SCREENORIENTATION]
  - 5. Run the view transition page visibility change steps with document.
  - 6. Run any page visibility change steps which may be defined in other specifications, with visibility state and document.
  - 7. Fire an event named visibilitychange at document, with its bubbles attribute initialized to true.

- [x] [set the initial visibility state](https://html.spec.whatwg.org/#set-the-initial-visibility-state) — To set the initial visibility state of Document document to visibilityState:
  - 1. Set document's visibility state to visibilityState.
  - 2. Queue a new VisibilityStateEntry whose visibility state is document's visibility state and whose timestamp is 0.

#### [6.2.1 The VisibilityStateEntry interface](https://html.spec.whatwg.org/#the-visibilitystateentry-interface)

Status: n/a (no performance timeline)
<!-- definitions: visibilitystateentry-timestamp visibilitystateentry-state -->

- [x] [VisibilityStateEntry.name](https://html.spec.whatwg.org/#visibilitystateentry-name) — The name getter steps are to return this's visibility state.

- [x] [VisibilityStateEntry.entryType](https://html.spec.whatwg.org/#visibilitystateentry-entrytype) — The entryType getter steps are to return "visibility-state".

- [x] [VisibilityStateEntry.startTime](https://html.spec.whatwg.org/#visibilitystateentry-starttime) — The startTime getter steps are to return this's timestamp.

- [x] [VisibilityStateEntry.duration](https://html.spec.whatwg.org/#visibilitystateentry-duration) — The duration getter steps are to return zero.

### [6.3 Inert subtrees](https://html.spec.whatwg.org/#inert-subtrees)

Status: reviewed (2026-11-01)
> `FocusAlgorithms::IsInert`, and the style's effective inertness (`Style::Adjuster`) for hit testing, selection and editing.
<!-- definitions: inert -->

#### [6.3.1 Modal dialogs and inert subtrees](https://html.spec.whatwg.org/#modal-dialogs-and-inert-subtrees)

Status: reviewed (2026-11-01)
> `Document::ActiveModalDialog`.
> Fixed: an element in a modal dialog under an `inert` ancestor was inert for focus, although the dialog escapes its ancestors' inertness.

- [x] [blocked by a modal dialog](https://html.spec.whatwg.org/#blocked-by-a-modal-dialog) — A Document document is blocked by a modal dialog subject if subject is the topmost dialog element in document's top layer. While document is so blocked, every node that is connected to document, with the exception of the subject element and its flat tree descendants, must become inert.

#### [6.3.2 The inert attribute](https://html.spec.whatwg.org/#the-inert-attribute)

Status: reviewed (2026-11-01)
> `HTMLElement::Inert` reflects it.

### [6.4 Tracking user activation](https://html.spec.whatwg.org/#tracking-user-activation)

Status: reviewed (2026-11-01)
> The sections below.

#### [6.4.1 Data model](https://html.spec.whatwg.org/#user-activation-data-model)

Status: reviewed (2026-11-01)
> `LocalDOMWindow`'s timestamps; the transient activation duration is 5 seconds.
<!-- definitions: last-activation-timestamp last-history-action-activation-timestamp transient-activation-duration sticky-activation transient-activation history-action-activation -->

- [x] [expired](https://html.spec.whatwg.org/#activation-expiry) — The transient activation state is considered expired if it becomes false because the transient activation duration time has elapsed since the last user activation. Note that it can become false even before the expiry time through an activation consumption.

#### [6.4.2 Processing model](https://html.spec.whatwg.org/#user-activation-processing-model)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::NotifyActivated` from `EventHandler` (presses, key downs other than Escape, touch ends), and the consume steps; there are no other frames' windows.

- [x] [activation notification](https://html.spec.whatwg.org/#activation-notification) — When a user interaction causes firing of an activation triggering input event in a Document document, the user agent must perform the following activation notification steps before dispatching the event:
  - 1. Assert: document is fully active.
  - 2. Let windows be « document's relevant global object ».
  - 3. Extend windows with the active window of each of document's ancestor navigables.
  - 4. Extend windows with the active window of each of document's descendant navigables, filtered to include only those navigables whose active document's origin is same origin with document's origin.
  - 5. For each window in windows:
    - 5a. Set window's last activation timestamp to the current high resolution time.
    - 5b. Notify the close watcher manager about user activation given window.

- [x] [activation triggering input event](https://html.spec.whatwg.org/#activation-triggering-input-event) — An activation triggering input event is any event whose isTrusted attribute is true and whose type is one of:

- [x] [consume user activation](https://html.spec.whatwg.org/#consume-user-activation) — Activation consuming APIs defined in this and other specifications can consume user activation by performing the following steps, given a Window W:
  - 1. If W's navigable is null, then return.
  - 2. Let top be W's navigable's top-level traversable.
  - 3. Let navigables be the inclusive descendant navigables of top's active document.
  - 4. Let windows be the list of Window objects constructed by taking the active window of each item in navigables.
  - 5. For each window in windows, if window's last activation timestamp is not positive infinity, then set window's last activation timestamp to negative infinity.

- [x] [consume history-action user activation](https://html.spec.whatwg.org/#consume-history-action-user-activation) — History-action activation-consuming APIs can consume history-action user activation by performing the following steps, given a Window W:
  - 1. If W's navigable is null, then return.
  - 2. Let top be W's navigable's top-level traversable.
  - 3. Let navigables be the inclusive descendant navigables of top's active document.
  - 4. Let windows be the list of Window objects constructed by taking the active window of each item in navigables.
  - 5. For each window in windows, set window's last history-action activation timestamp to window's last activation timestamp.

#### [6.4.3 APIs gated by user activation](https://html.spec.whatwg.org/#user-activation-gated-apis)

Status: reviewed (2026-11-01)
> `window.open()` is transient activation-gated; close watchers use history-action activation.
<!-- definitions: sticky-activation-gated-api transient-activation-gated-api activation-consuming-api history-action-activation-consuming-api -->

#### [6.4.4 The UserActivation interface](https://html.spec.whatwg.org/#the-useractivation-interface)

Status: reviewed (2026-11-01)
> `UserActivation`, made when the navigator is first asked for it.

- [x] [associated UserActivation](https://html.spec.whatwg.org/#associated-useractivation) — Each Window has an associated UserActivation, which is a UserActivation object. Upon creation of the Window object, its associated UserActivation must be set to a new UserActivation object created in the Window object's relevant realm.

- [x] [Navigator.userActivation](https://html.spec.whatwg.org/#dom-navigator-useractivation) — The userActivation getter steps are to return this's relevant global object's associated UserActivation.

- [x] [UserActivation.hasBeenActive](https://html.spec.whatwg.org/#dom-useractivation-hasbeenactive) — The hasBeenActive getter steps are to return true if this's relevant global object has sticky activation, and false otherwise.

- [x] [UserActivation.isActive](https://html.spec.whatwg.org/#dom-useractivation-isactive) — The isActive getter steps are to return true if this's relevant global object has transient activation, and false otherwise.

#### [6.4.5 User agent automation](https://html.spec.whatwg.org/#user-activation-user-agent-automation)

Status: n/a (no WebDriver)

- [x] [The remote end steps](https://html.spec.whatwg.org/#user-activation-user-agent-automation) — The remote end steps are:
  - 1. Let window be the current browsing context's active window.
  - 2. Let consume be true if window has transient activation; otherwise false.
  - 3. If consume is true, then consume user activation of window.
  - 4. Return success with data consume.

### [6.5 Activation behavior of elements](https://html.spec.whatwg.org/#activation)

Status: reviewed (2026-11-01)
> `HTMLElement::Click`, through `SimulateClick`.
<!-- definitions: click-in-progress-flag -->

- [x] [HTMLElement.click()](https://html.spec.whatwg.org/#dom-click) — The click() method must run the following steps:
  - 1. If this element is a form control that is disabled, then return.
  - 2. If this element's click in progress flag is set, then return.
  - 3. Set this element's click in progress flag.
  - 4. Fire a synthetic pointer event named click at this element, with the not trusted flag set.
  - 5. Unset this element's click in progress flag.

#### [6.5.1 The ToggleEvent interface](https://html.spec.whatwg.org/#the-toggleevent-interface)

Status: reviewed (2026-11-01)
> `ToggleEvent` and `ToggleEventTask`.
<!-- definitions: dom-toggleevent-newstate toggle-task-tracker toggle-task-task toggle-task-old-state -->

- [x] [ToggleEvent.oldState](https://html.spec.whatwg.org/#dom-toggleevent-oldstate) — The oldState and newState attributes must return the values they are initialized to.

- [x] [ToggleEvent.source](https://html.spec.whatwg.org/#dom-toggleevent-source) — The source getter steps are to return the result of retargeting source against this's currentTarget.

#### [6.5.2 The CommandEvent interface](https://html.spec.whatwg.org/#the-commandevent-interface)

Status: reviewed (2026-11-01)
> `CommandEvent`.

- [x] [CommandEvent.command](https://html.spec.whatwg.org/#dom-commandevent-command) — The command attribute must return the value it was initialized to.

- [x] [CommandEvent.source](https://html.spec.whatwg.org/#dom-commandevent-source) — The source getter steps are to return the result of retargeting source against this's currentTarget.

### [6.6 Focus](https://html.spec.whatwg.org/#focus)

Status: reviewed (2026-11-01)
> The sections below.

#### [6.6.1 Introduction](https://html.spec.whatwg.org/#introduction-8)

Status: n/a (introduction)

#### [6.6.2 Data model](https://html.spec.whatwg.org/#data-model)

Status: reviewed (2026-11-01)
> `FocusAlgorithms`: focusable areas, the focus chain, the currently focused area and focus navigation scopes. There are no scrollable regions or nested navigables as focusable areas (see `KNOWN-ISSUES.md`).
<!-- definitions: user-attention focusable-area dom-anchor focused-area-of-the-document focused focusable -->

- [x] [system focus (top-level traversable)](https://html.spec.whatwg.org/#system-focus) — A top-level traversable has system focus when it can receive keyboard input channeled from the operating system, possibly targeted at one of its active document's descendant navigables.

- [x] [fully active descendant of a top-level traversable with user attention (Document)](https://html.spec.whatwg.org/#fully-active-descendant-of-a-top-level-traversable-with-user-attention) — A Document d is a fully active descendant of a top-level traversable with user attention when d is fully active and d's node navigable's top-level traversable has user attention.

- [x] [currently focused area of a top-level traversable](https://html.spec.whatwg.org/#currently-focused-area-of-a-top-level-traversable) — The currently focused area of a top-level traversable traversable is the focusable area-or-null returned by this algorithm:
  - 1. If traversable does not have system focus, then return null.
  - 2. Let candidate be traversable's active document.
  - 3. While candidate's focused area is a navigable container with a non-null content navigable: set candidate to the active document of that navigable container's content navigable.
  - 4. If candidate's focused area is non-null, set candidate to candidate's focused area.
  - 5. Return candidate.

- [x] [current focus chain of a top-level traversable](https://html.spec.whatwg.org/#current-focus-chain-of-a-top-level-traversable) — The current focus chain of a top-level traversable traversable is the focus chain of the currently focused area of traversable, if traversable is non-null, or an empty list otherwise.

- [x] [gain focus](https://html.spec.whatwg.org/#gains-focus) — An element that is the DOM anchor of a focusable area is said to gain focus when that focusable area becomes the currently focused area of a top-level traversable. When an element is the DOM anchor of a focusable area of the currently focused area of a top-level traversable, it is focused.

- [x] [focus chain](https://html.spec.whatwg.org/#focus-chain) — The focus chain of a focusable area subject is the ordered list constructed as follows:
  - 1. Let output be an empty list.
  - 2. Let currentObject be subject.
  - 3. While true:
    - 3a. Append currentObject to output.
    - 3b. If currentObject is an area element's shape, then append that area element to output. Otherwise, if currentObject's DOM anchor is an element that is not currentObject itself, then append currentObject's DOM anchor to output.
    - 3c. If currentObject is a focusable area, then set currentObject to currentObject's DOM anchor's node document. Otherwise, if currentObject is a Document whose node navigable's parent is non-null, then set currentObject to currentObject's node navigable's parent. Otherwise, break.
  - 4. Return output.

- [x] [sequentially focusable](https://html.spec.whatwg.org/#sequentially-focusable) — A focusable area is said to be sequentially focusable if it is included in its Document's sequential focus navigation order and the user agent determines that it is sequentially focusable.

- [x] [click focusable](https://html.spec.whatwg.org/#click-focusable) — A focusable area is said to be click focusable if the user agent determines that it is click focusable. User agents should consider focusable areas with non-null tabindex values to be click focusable.

- [x] [focus navigation scope owner](https://html.spec.whatwg.org/#focus-navigation-scope-owner) — A node is a focus navigation scope owner if it is a Document, a shadow host, a slot, or an element which is the popover trigger of an element in the popover showing state.

- [x] [focus navigation scope](https://html.spec.whatwg.org/#focus-navigation-scope) — Each focus navigation scope owner has a focus navigation scope, which is a list of elements. Its contents are determined as follows:

- [x] [associated focus navigation owner](https://html.spec.whatwg.org/#associated-focus-navigation-owner) — Every element element has an associated focus navigation owner, which is either null or a focus navigation scope owner. It is determined by the following algorithm:
  - 1. If element's parent is null, then return null.
  - 2. If element's parent is a shadow host, then return element's assigned slot.
  - 3. If element's parent is a shadow root, then return the parent's host.
  - 4. If element's parent is the document element, then return the parent's node document.
  - 5. If element is in the popover showing state and has a popover trigger set, then return element's popover trigger.
  - 6. Return element's parent's associated focus navigation owner.

- [x] [tabindex-ordered focus navigation scope](https://html.spec.whatwg.org/#tabindex-ordered-focus-navigation-scope) — A tabindex-ordered focus navigation scope is a list of focusable areas and focus navigation scope owners. Every focus navigation scope owner owner has tabindex-ordered focus navigation scope, whose contents are determined as follows:

- [x] [flattened tabindex-ordered focus navigation scope](https://html.spec.whatwg.org/#flattened-tabindex-ordered-focus-navigation-scope) — A flattened tabindex-ordered focus navigation scope is a list of focusable areas. Every focus navigation scope owner owner owns a distinct flattened tabindex-ordered focus navigation scope, whose contents are determined by the following algorithm:
  - 1. Let result be a clone of owner's tabindex-ordered focus navigation scope.
  - 2. For each item of result:
    - 2a. If item is not a focus navigation scope owner, then continue.
    - 2b. If item is not a focusable area, then replace item with all of the items in item's flattened tabindex-ordered focus navigation scope.
    - 2c. Otherwise, insert the contents of item's flattened tabindex-ordered focus navigation scope after item.

#### [6.6.3 The tabindex attribute](https://html.spec.whatwg.org/#the-tabindex-attribute)

Status: reviewed (2026-11-01)
> `FocusAlgorithms::TabIndexValue` and `HTMLOrSVGOrMathMLElement::TabIndex`.
<!-- definitions: attr-tabindex -->

- [x] [tabindex value](https://html.spec.whatwg.org/#tabindex-value) — The tabindex value of an element is the value of its tabindex attribute, parsed using the rules for parsing integers. If parsing fails or the attribute is not specified, then the tabindex value is null.

- [x] [HTMLOrSVGOrMathMLElement.tabIndex](https://html.spec.whatwg.org/#dom-tabindex) — The tabIndex getter steps are:
  - 1. Let attribute be this's tabindex attribute.
  - 2. If attribute is not null:
    - 2a. Let parsedValue be the result of integer parsing attribute's value.
    - 2b. If parsedValue is not an error and is within the long range, then return parsedValue.
  - 3. Return 0 if this is an a, area, button, frame, iframe, input, object, select, textarea, or SVG a element, or MathML a element, or is a summary element that is a summary for its parent details; otherwise −1.

#### [6.6.4 Processing model](https://html.spec.whatwg.org/#focus-processing-model)

Status: reviewed (2026-11-01)
> `FocusAlgorithms`; the old focused element loses its focus before `blur` fires, and no events fire at the window (see `DIFFERENCES.md` and `KNOWN-ISSUES.md`).
> Fixed: key events went nowhere in a document without a body and with nothing focused, rather than to the document element.

- [x] [get the focusable area](https://html.spec.whatwg.org/#get-the-focusable-area) — To get the focusable area for a focus target that is either an element that is not a focusable area, or is a navigable, given an optional string focus trigger (default "other"), run the first matching set of steps from the following list:
  - • If focus target is an area element with one or more shapes that are focusable areas: Return the shape corresponding to the first img element in tree order that uses the image map to which the area element belongs.
  - • If focus target is an element with one or more scrollable regions that are focusable areas: Return the element's first scrollable region, according to a pre-order, depth-first traversal of the flat tree. [CSSSCOPING]
  - • If focus target is the document element of its Document: Return the Document's viewport.
  - • If focus target is a navigable: Return the navigable's active document.
  - • If focus target is a navigable container with a non-null content navigable: Return the navigable container's content navigable's active document.
  - • If focus target is a shadow host whose shadow root's delegates focus is true:
    - 1. Let focusedElement be the currently focused area of a top-level traversable's DOM anchor.
    - 2. If focus target is a shadow-including inclusive ancestor of focusedElement, then return focusedElement.
    - 3. Return the focus delegate for focus target given focus trigger.
  - • Otherwise: Return null.

- [x] [focus delegate](https://html.spec.whatwg.org/#focus-delegate) — The focus delegate for a focusTarget, given an optional string focusTrigger (default "other"), is given by the following steps:
  - 1. If focusTarget is a shadow host and its shadow root's delegates focus is false, then return null.
  - 2. Let whereToLook be focusTarget.
  - 3. If whereToLook is a shadow host, then set whereToLook to whereToLook's shadow root.
  - 4. Let autofocusDelegate be the autofocus delegate for whereToLook given focusTrigger.
  - 5. If autofocusDelegate is not null, then return autofocusDelegate.
  - 6. For each descendant of whereToLook's descendants, in tree order:
    - 6a. Let focusableArea be null.
    - 6b. If focusTarget is a dialog element and descendant is sequentially focusable, then set focusableArea to descendant.
    - 6c. Otherwise, if focusTarget is not a dialog and descendant is a focusable area, set focusableArea to descendant.
    - 6d. Otherwise, set focusableArea to the result of getting the focusable area for descendant given focusTrigger.
    - 6e. If focusableArea is not null, then return focusableArea.
  - 7. Return null.

- [x] [autofocus delegate](https://html.spec.whatwg.org/#autofocus-delegate) — The autofocus delegate for a focus target given a focus trigger is given by the following steps:
  - 1. For each descendant descendant of focus target, in tree order:
    - 1a. If descendant does not have an autofocus content attribute, then continue.
    - 1b. Let focusable area be descendant, if descendant is a focusable area; otherwise let focusable area be the result of getting the focusable area for descendant given focus trigger.
    - 1c. If focusable area is null, then continue.
    - 1d. If focusable area is not click focusable and focus trigger is "click", then continue.
    - 1e. Return focusable area.
  - 2. Return null.

- [x] [focusing steps](https://html.spec.whatwg.org/#focusing-steps) — The focusing steps for an object new focus target that is either a focusable area, or an element that is not a focusable area, or a navigable, are as follows. They can optionally be run with a fallback target and a string focus trigger.
  - 1. If new focus target is not a focusable area, then set new focus target to the result of getting the focusable area for new focus target, given focus trigger if it was passed.
  - 2. If new focus target is null:
    - 2a. If no fallback target was specified, then return.
    - 2b. Otherwise, set new focus target to the fallback target.
  - 3. If new focus target is a navigable container with non-null content navigable, then set new focus target to the content navigable's active document.
  - 4. If new focus target is a focusable area and its DOM anchor is inert, then return.
  - 5. If new focus target is the currently focused area of a top-level traversable, then return.
  - 6. Let old chain be the current focus chain of the top-level traversable in which new focus target finds itself.
  - 7. Let new chain be the focus chain of new focus target.
  - 8. Run the focus update steps with old chain, new chain, and new focus target respectively.

- [x] [unfocusing steps](https://html.spec.whatwg.org/#unfocusing-steps) — The unfocusing steps for an object old focus target that is either a focusable area or an element that is not a focusable area are as follows:
  - 1. If old focus target is a shadow host whose shadow root's delegates focus is true, and old focus target's shadow root is a shadow-including inclusive ancestor of the currently focused area of a top-level traversable's DOM anchor, then set old focus target to that currently focused area of a top-level traversable.
  - 2. If old focus target is inert, then return.
  - 3. If old focus target is an area element and one of its shapes is the currently focused area of a top-level traversable, or, if old focus target is an element with one or more scrollable regions, and one of them is the currently focused area of a top-level traversable, then let old focus target be that currently focused area of a top-level traversable.
  - 4. Let old chain be the current focus chain of the top-level traversable in which old focus target finds itself.
  - 5. If old focus target is not one of the entries in old chain, then return.
  - 6. If old focus target is not a focusable area, then return.
  - 7. Let topDocument be old chain's last entry.
  - 8. If topDocument's node navigable has system focus, then run the focusing steps for topDocument's viewport. Otherwise, apply any relevant platform-specific conventions for removing system focus from topDocument's node navigable, and run the focus update steps given old chain, an empty list, and null.

- [x] [focus update steps](https://html.spec.whatwg.org/#focus-update-steps) — The focus update steps, given an old chain, a new chain, and a new focus target respectively, are as follows:
  - 1. If the last entry in old chain and the last entry in new chain are the same, pop the last entry from old chain and the last entry from new chain and redo this step.
  - 2. For each entry entry in old chain, in order, run these substeps:
    - 2a. If entry is an input element, and the change event applies to the element, and the element does not have a defined activation behavior, and the user has changed the element's value or its list of selected files while the control was focused without committing that change (such that it is different to what it was when the control was first focused):
      - 2a.i. Set entry's user validity to true.
      - 2a.ii. Fire an event named change at the element, with the bubbles attribute initialized to true.
    - 2b. If entry is an element, let blur event target be entry. If entry is a Document object, let blur event target be that Document object's relevant global object. Otherwise, let blur event target be null.
    - 2c. If entry is the last entry in old chain, and entry is an Element, and the last entry in new chain is also an Element, then let related blur target be the last entry in new chain. Otherwise, let related blur target be null.
    - 2d. If blur event target is not null, fire a focus event named blur at blur event target, with related blur target as the related target.
  - 3. Apply any relevant platform-specific conventions for focusing new focus target. (For example, some platforms select the contents of a text control when that control is focused.)
  - 4. For each entry entry in new chain, in reverse order, run these substeps:
    - 4a. If entry is a focusable area, and the focused area of the document is not entry:
      - 4a.i. Set document's relevant global object's navigation API's focus changed during ongoing navigation to true.
      - 4a.ii. Designate entry as the focused area of the document.
    - 4b. If entry is an element, let focus event target be entry. If entry is a Document object, let focus event target be that Document object's relevant global object. Otherwise, let focus event target be null.
    - 4c. If entry is the last entry in new chain, and entry is an Element, and the last entry in old chain is also an Element, then let related focus target be the last entry in old chain. Otherwise, let related focus target be null.
    - 4d. If focus event target is not null, fire a focus event named focus at focus event target, with related focus target as the related target.

- [x] [fire a focus event](https://html.spec.whatwg.org/#fire-a-focus-event) — To fire a focus event named e at an element or a Window object target given a related target relatedTarget:
  - 1. Let view be target if target is a Window object; otherwise target's node document's relevant global object.
  - 2. Fire an event named e at target, using FocusEvent, with the relatedTarget attribute initialized to relatedTarget, the view attribute initialized to view, and the composed flag set.

- [x] [When a key event](https://html.spec.whatwg.org/#fire-a-focus-event) — When a key event is to be routed in a top-level traversable, the user agent must run the following steps:
  - 1. Let target area be the currently focused area of the top-level traversable.
  - 2. Assert: target area is not null, since key events are only routed to top-level traversables that have system focus. Therefore, target area is a focusable area.
  - 3. Let target node be target area's DOM anchor.
  - 4. If target node is a Document that has a body element, then let target node be the body element of that Document. Otherwise, if target node is a Document object that has a non-null document element, then let target node be that document element.
  - 5. If target node is not inert:
    - 5a. Let canHandle be the result of dispatching the key event at target node.
    - 5b. If canHandle is true, then let target area handle the key event. This might include firing a click event at target node.

- [x] [has focus steps](https://html.spec.whatwg.org/#has-focus-steps) — The has focus steps, given a Document object target, are as follows:
  - 1. If target's node navigable's top-level traversable does not have system focus, then return false.
  - 2. Let candidate be target's node navigable's top-level traversable's active document.
  - 3. While true:
    - 3a. If candidate is target, then return true.
    - 3b. If the focused area of candidate is a navigable container with a non-null content navigable, then set candidate to the active document of that navigable container's content navigable.
    - 3c. Otherwise, return false.

#### [6.6.5 Sequential focus navigation](https://html.spec.whatwg.org/#sequential-focus-navigation)

Status: reviewed (2026-11-01)
> `FocusAlgorithms::SequentialFocusNavigationOrder` and `SequentialNavigation`, which restarts from the document at the end, as there are no user agent controls.
<!-- definitions: sequential-focus-navigation-order sequential-focus-navigation-starting-point sequential-focus-direction sequential-focus-forward sequential-focus-backward selection-mechanism selection-mechanism-dom suitable-sequentially-focusable-area -->

- [x] [When the user requests that focus move from the currently focused area of a top…](https://html.spec.whatwg.org/#selection-mechanism-sequential) — When the user requests that focus move from the currently focused area of a top-level traversable to the next or previous focusable area (e.g., as the default action of pressing the tab key), or when the user requests that focus sequentially move to a top-level traversable in the first place (e.g.,…
  - 1. Let starting point be the currently focused area of a top-level traversable, if the user requested to move focus sequentially from there, or else the top-level traversable itself, if the user instead requested to move focus from outside the top-level traversable.
  - 2. If there is a sequential focus navigation starting point defined and it is inside starting point, then let starting point be the sequential focus navigation starting point instead.
  - 3. Let direction be "forward" if the user requested the next control, and "backward" if the user requested the previous control.
  - 4. Loop: Let selection mechanism be "sequential" if starting point is a navigable or if starting point is in its Document's sequential focus navigation order. Otherwise, starting point is not in its Document's sequential focus navigation order; let selection mechanism be "DOM".
  - 5. Let candidate be the result of running the sequential navigation search algorithm with starting point, direction, and selection mechanism.
  - 6. If candidate is not null, then run the focusing steps for candidate and return.
  - 7. Otherwise, unset the sequential focus navigation starting point.
  - 8. If starting point is a top-level traversable, or a focusable area in the top-level traversable, the user agent should transfer focus to its own controls appropriately (if any), honouring direction, and then return. If the user agent has no sequentially focusable controls — a kiosk-mode browser, for instance — then the user agent may instead restart these steps with the starting point being the to…
  - 9. Otherwise, starting point is a focusable area in a child navigable. Set starting point to that child navigable's parent and return to the step labeled loop.

- [x] [sequential navigation search algorithm](https://html.spec.whatwg.org/#sequential-navigation-search-algorithm) — The sequential navigation search algorithm, given a focusable area starting point, sequential focus direction direction, and selection mechanism selection mechanism, consists of the following steps. They return a focusable area-or-null.
  - 1. Pick the appropriate cell from the following table, and follow the instructions in that cell. The appropriate cell is the one that is from the column whose header describes direction and from the first row whose header describes starting point and selection mechanism. A suitable sequentially focusable area is a focusable area whose DOM anchor is not inert and is sequentially focusable.
  - 2. If candidate is a navigable container with a non-null content navigable:
    - 2a. Let recursive candidate be the result of running the sequential navigation search algorithm with candidate's content navigable, direction, and "sequential".
    - 2b. If recursive candidate is null, then return the result of running the sequential navigation search algorithm with candidate, direction, and selection mechanism.
    - 2c. Otherwise, set candidate to recursive candidate.
  - 3. Return candidate.

#### [6.6.6 Focus management APIs](https://html.spec.whatwg.org/#focus-management-apis)

Status: reviewed (2026-11-01)
> `activeElement`, `hasFocus()`, and `focus()`/`blur()` in `HTMLOrSVGOrMathMLElement`; focus is always allowed, and `window.focus()`/`blur()` differ (see `DIFFERENCES.md`).
> Fixed: calling `focus()` on an element that couldn't be focused scrolled the focused element into view.
<!-- definitions: dom-focusoptions-focusvisible dom-focusoptions-preventscroll -->

- [x] [DocumentOrShadowRoot.activeElement](https://html.spec.whatwg.org/#dom-documentorshadowroot-activeelement) — The DocumentOrShadowRoot activeElement getter steps are:
  - 1. Let candidate be this's node document's focused area's DOM anchor.
  - 2. Set candidate to the result of retargeting candidate against this.
  - 3. If candidate's root is not this, then return null.
  - 4. If candidate is not a Document object, then return candidate.
  - 5. If candidate has a body element, then return that body element.
  - 6. If candidate's document element is non-null, then return that document element.
  - 7. Return null.

- [x] [Document.hasFocus()](https://html.spec.whatwg.org/#dom-document-hasfocus) — The Document hasFocus() method steps are to return the result of running the has focus steps given this.

- [x] [Window.focus()](https://html.spec.whatwg.org/#dom-window-focus) — The Window focus() method steps are:
  - 1. Let current be this's navigable.
  - 2. If current is null, then return.
  - 3. If the allow focus steps given current's active document return false, then return.
  - 4. Run the focusing steps with current.
  - 5. If current is a top-level traversable, user agents are encouraged to trigger some sort of notification to indicate to the user that the page is attempting to gain focus.

- [x] [Window.blur()](https://html.spec.whatwg.org/#dom-window-blur) — The Window blur() method steps are to do nothing.

- [x] [HTMLOrSVGOrMathMLElement.focus(options)](https://html.spec.whatwg.org/#dom-focus) — The HTMLOrSVGOrMathMLElement focus(options) method steps are:
  - 1. If the allow focus steps given this's node document return false, then return.
  - 2. Run the focusing steps for this.
  - 3. If options["focusVisible"] is true, or does not exist but in an implementation-defined way the user agent determines it would be best to do so, then indicate focus.
  - 4. If options["preventScroll"] is false, then scroll a target into view given this, "auto", "center", and "center".

- [x] [HTMLOrSVGOrMathMLElement.blur()](https://html.spec.whatwg.org/#dom-blur) — The HTMLOrSVGOrMathMLElement blur() method steps are:
  - 1. The user agent should run the unfocusing steps given this. User agents may instead selectively or uniformly do nothing, for usability reasons.

- [x] [allow focus steps](https://html.spec.whatwg.org/#allow-focus-steps) — The allow focus steps, given a Document object target, are:
  - 1. If target is allowed to use the "focus-without-user-activation" feature, then return true.
  - 2. If target's relevant global object has transient activation, then return true.
  - 3. Return false.

#### [6.6.7 The autofocus attribute](https://html.spec.whatwg.org/#the-autofocus-attribute)

Status: reviewed (2026-11-01)
> `FocusAlgorithms::AutofocusInsertionSteps` and `FlushAutofocusCandidates`; the nearest ancestor autofocus scoping root element is only used by an authoring requirement.
> Fixed: a candidate that isn't a focusable area (a shadow host that delegates focus) didn't focus its focusable area.
<!-- definitions: attr-fe-autofocus autofocus-candidates -->

- [x] [nearest ancestor autofocus scoping root element](https://html.spec.whatwg.org/#nearest-ancestor-autofocus-scoping-root-element) — To find the nearest ancestor autofocus scoping root element given an Element element:
  - 1. If element is a dialog element, then return element.
  - 2. If element's popover attribute is not in the No Popover state, then return element.
  - 3. Let ancestor be element.
  - 4. While ancestor has a parent element:
    - 4a. Set ancestor to ancestor's parent element.
    - 4b. If ancestor is a dialog element, then return ancestor.
    - 4c. If ancestor's popover attribute is not in the No Popover state, then return ancestor.
  - 5. Return ancestor.

- [x] [When an element with the autofocus attribute specified is inserted into a docum…](https://html.spec.whatwg.org/#autofocus-processed-flag) — When an element with the autofocus attribute specified is inserted into a document, run the following steps:
  - 1. If the user has indicated (for example, by starting to type in a form control) that they do not wish focus to be changed, then optionally return.
  - 2. Let target be the element's node document.
  - 3. If target is not fully active, then return.
  - 4. If target's active sandboxing flag set has the sandboxed automatic features browsing context flag, then return.
  - 5. If the allow focus steps given target return false, then return.
  - 6. Let topDocument be target's node navigable's top-level traversable's active document.
  - 7. If topDocument's autofocus processed flag is false, then remove the element from topDocument's autofocus candidates, and append the element to topDocument's autofocus candidates.

- [x] [flush autofocus candidates](https://html.spec.whatwg.org/#flush-autofocus-candidates) — To flush autofocus candidates for a document topDocument, run these steps:
  - 1. If topDocument's autofocus processed flag is true, then return.
  - 2. Let candidates be topDocument's autofocus candidates.
  - 3. If candidates is empty, then return.
  - 4. If topDocument's focused area is not topDocument itself, or topDocument has non-null target element:
    - 4a. Empty candidates.
    - 4b. Set topDocument's autofocus processed flag to true.
    - 4c. Return.
  - 5. While candidates is not empty:
    - 5a. Let element be candidates[0].
    - 5b. Let doc be element's node document.
    - 5c. If doc is not fully active, then remove element from candidates, and continue.
    - 5d. If doc's node navigable's top-level traversable is not the same as topDocument's node navigable, then remove element from candidates, and continue.
    - 5e. If doc's script-blocking style sheet set is not empty, then return.
    - 5f. Remove element from candidates.
    - 5g. Let inclusiveAncestorDocuments be a list consisting of the active document of doc's inclusive ancestor navigables.
    - 5h. If any Document in inclusiveAncestorDocuments has non-null target element, then continue.
    - 5i. Let target be element.
    - 5j. If target is not a focusable area, then set target to the result of getting the focusable area for target.
    - 5k. If target is not null:
      - 5k.i. Empty candidates.
      - 5k.ii. Set topDocument's autofocus processed flag to true.
      - 5k.iii. Run the focusing steps for target.

### [6.7 Assigning keyboard shortcuts](https://html.spec.whatwg.org/#assigning-keyboard-shortcuts)

Status: reviewed (2026-11-01)
> The sections below.

#### [6.7.1 Introduction](https://html.spec.whatwg.org/#introduction-9)

Status: n/a (introduction)

#### [6.7.2 The accesskey attribute](https://html.spec.whatwg.org/#the-accesskey-attribute)

Status: reviewed (2026-11-01)
> `HTMLElement::AccessKey` reflects it.

#### [6.7.3 Processing model](https://html.spec.whatwg.org/#keyboard-shortcuts-processing-model)

Status: reviewed (2026-11-06)
> `Element::AssignedAccessKey`. The key is pressed with Alt, as `EventHandler::HandleAccessKey` looks for, and `HTMLElement::AccessKeyLabel` says.

- [x] [Whenever an element's accesskey attribute is set, changed, or removed, the user…](https://html.spec.whatwg.org/#assigned-access-key) — Whenever an element's accesskey attribute is set, changed, or removed, the user agent must update the element's assigned access key by running the following steps:
  - 1. If the element has no accesskey attribute, then skip to the fallback step below.
  - 2. Otherwise, split the attribute's value on ASCII whitespace, and let keys be the resulting tokens.
  - 3. For each value in keys in turn, in the order the tokens appeared in the attribute's value, run the following substeps:
    - 3a. If the value is not a string exactly one code point in length, then skip the remainder of these steps for this value.
    - 3b. If the value does not correspond to a key on the system's keyboard, then skip the remainder of these steps for this value.
    - 3c. If the user agent can find a mix of zero or more modifier keys that, combined with the key that corresponds to the value given in the attribute, can be used as the access key, then the user agent may assign that combination of keys as the element's assigned access key and return.
  - 4. Fallback: Optionally, the user agent may assign a key combination of its choosing as the element's assigned access key and then return.
  - 5. If this step is reached, the element has no assigned access key.

- [x] [HTMLElement.accessKeyLabel](https://html.spec.whatwg.org/#dom-accesskeylabel) — The accessKeyLabel IDL attribute must return a string that represents the element's assigned access key, if any. If the element does not have one, then the IDL attribute must return the empty string.

### [6.8 Editing](https://html.spec.whatwg.org/#editing-2)

Status: reviewed (2026-11-01)
> The sections below.

#### [6.8.1 Making document regions editable: The contenteditable content attribute](https://html.spec.whatwg.org/#contenteditable)

Status: reviewed (2026-11-01)
> `HTMLElement::ContentEditable` (added), `Node::IsContentEditable` and the editability from the attribute (`EditabilityFromContentEditableAttr`).
<!-- definitions: attr-contenteditable -->

- [x] [Inherit](https://html.spec.whatwg.org/#attr-contenteditable-inherit-state) — The attribute's missing value default and invalid value default are both the Inherit state. The inherit state indicates that the element is editable (or not) based on the parent element's state. The attribute's empty value default is the True state.

- [x] [ElementContentEditable.contentEditable](https://html.spec.whatwg.org/#dom-contenteditable) — The contentEditable IDL attribute, on getting, must return the string "true" if the content attribute is set to the True state, "plaintext-only" if the content attribute is set to the Plaintext-Only state, "false" if the content attribute is set to the False state, and "inherit" otherwise. On setting, if the new value is an ASCII case-insensitive match for the string "inherit", then the content attribute must be removed, if the new value is an ASCII case-insensitive match for the string "true", then the content attribute must be set to the string "true", if the new value is an ASCII case-insensitive match for the string "plaintext-only", then the content attribute must be set to the string "plaintext-only", if the new value is an ASCII case-insensitive match for the string "false", then t…

- [x] [ElementContentEditable.isContentEditable](https://html.spec.whatwg.org/#dom-iscontenteditable) — The isContentEditable IDL attribute, on getting, must return true if the element is either an editing host or editable, and false otherwise.

#### [6.8.2 Making entire documents editable: the designMode getter and setter](https://html.spec.whatwg.org/#making-entire-documents-editable:-the-designmode-idl-attribute)

Status: reviewed (2026-11-06)
> `Document::DesignMode`.
<!-- definitions: design-mode-enabled -->

- [x] [Document.designMode](https://html.spec.whatwg.org/#designMode) — The designMode getter steps are to return "on" if this's design mode enabled is true; otherwise "off".

- [x] [The designMode setter steps](https://html.spec.whatwg.org/#designMode) — The designMode setter steps are:
  - 1. Let value be the given value, converted to ASCII lowercase.
  - 2. If value is "on" and this's design mode enabled is false:
    - 2a. Set this's design mode enabled to true.
    - 2b. Reset this's active range's start and end boundary points to be at the start of this.
    - 2c. Run the focusing steps for this's document element, if non-null.
  - 3. If value is "off", then set this's design mode enabled to false.

#### [6.8.3 Best practices for in-page editors](https://html.spec.whatwg.org/#best-practices-for-in-page-editors)

Status: n/a (authoring advice)

#### [6.8.4 Editing APIs](https://html.spec.whatwg.org/#editing-apis)

Status: reviewed (2026-11-01)
> Editing hosts are `FocusAlgorithms`' `IsEditingHost` and the editing code's; `execCommand()` isn't implemented (see `KNOWN-ISSUES.md`).
<!-- definitions: active-range editing-host-of editable execCommand dom-document-querycommandenabled dom-document-querycommandindeterm dom-document-querycommandstate dom-document-querycommandsupported dom-document-querycommandvalue delete-the-selection -->

- [x] [editing host](https://html.spec.whatwg.org/#editing-host) — An editing host is either an HTML element with its contenteditable attribute in the true state or plaintext-only state, or a child HTML element of a Document whose design mode enabled is true.

#### [6.8.5 Spelling and grammar checking](https://html.spec.whatwg.org/#spelling-and-grammar-checking)

Status: reviewed (2026-11-01)
> `HTMLElement::SpellCheck`; every element's default behavior is inherit-by-default, true at the root. Text isn't checked (see `DIFFERENCES.md`).
> Fixed: the `spellcheck` getter was false in the Default state.
<!-- definitions: concept-spellcheck-default concept-spellcheck-default-true concept-spellcheck-default-false concept-spellcheck-default-inherit attr-spellcheck -->

- [x] [Default](https://html.spec.whatwg.org/#attr-spellcheck-default-state) — The attribute's missing value default and invalid value default are both the Default state. The default state indicates that the element is to act according to a default behavior, possibly based on the parent element's own spellcheck state, as defined below. The attribute's empty value default is the True state.

- [x] [HTMLElement.spellcheck](https://html.spec.whatwg.org/#dom-spellcheck) — The spellcheck IDL attribute, on getting, must return true if the element's spellcheck content attribute is in the True state, or if the element's spellcheck content attribute is in the Default state and the element's default behavior is true-by-default, or if the element's spellcheck content attribute is in the Default state and the element's default behavior is inherit-by-default and the element's parent element's spellcheck IDL attribute would return true; otherwise, if none of those conditions applies, then the attribute must instead return false.

- [x] [To determine if a word, sentence, or other piece of text in an applicable eleme…](https://html.spec.whatwg.org/#dom-spellcheck) — To determine if a word, sentence, or other piece of text in an applicable element (as defined above) is to have spelling- and grammar-checking enabled, the UA must use the following algorithm:
  - 1. If the user has disabled the checking for this text, then the checking is disabled.
  - 2. Otherwise, if the user has forced the checking for this text to always be enabled, then the checking is enabled.
  - 3. Otherwise, if the element with which the text is associated has a spellcheck content attribute, then: if that attribute is in the True state, then checking is enabled; otherwise, if that attribute is in the False state, then checking is disabled.
  - 4. Otherwise, if there is an ancestor element with a spellcheck content attribute that is not in the Default state, then: if the nearest such ancestor's spellcheck content attribute is in the True state, then checking is enabled; otherwise, checking is disabled.
  - 5. Otherwise, if the element's default behavior is true-by-default, then checking is enabled.
  - 6. Otherwise, if the element's default behavior is false-by-default, then checking is disabled.
  - 7. Otherwise, if the element's parent element has its checking enabled, then checking is enabled.
  - 8. Otherwise, checking is disabled.

#### [6.8.6 Writing suggestions](https://html.spec.whatwg.org/#writing-suggestions)

Status: reviewed (2026-11-01)
> `HTMLElement::WritingSuggestions`; no suggestions are offered.
> Fixed: the getter was empty, and an invalid value was the Default state rather than True.
<!-- definitions: attr-writingsuggestions -->

- [x] [Default](https://html.spec.whatwg.org/#attr-writingsuggestions-default-state) — The attribute's missing value default is the Default state. The default state indicates that the element is to act according to a default behavior, possibly based on the parent element's own writingsuggestions state, as defined below.

- [x] [computed writing suggestions value](https://html.spec.whatwg.org/#computed-writing-suggestions-value) — The computed writing suggestions value of a given element is determined by running the following steps:
  - 1. If element's writingsuggestions content attribute is in the False state, return "false".
  - 2. If element's writingsuggestions content attribute is in the Default state, element has a parent element, and the computed writing suggestions value of element's parent element is "false", then return "false".
  - 3. Return "true".

- [x] [HTMLElement.writingSuggestions](https://html.spec.whatwg.org/#dom-writingsuggestions) — The writingSuggestions getter steps are:
  - 1. Return this's computed writing suggestions value.

- [x] [User agents should only offer suggestions within an element's scope if the resu…](https://html.spec.whatwg.org/#dom-writingsuggestions) — User agents should only offer suggestions within an element's scope if the result of running the following algorithm given element returns true:
  - 1. If the user has disabled writing suggestions, then return false.
  - 2. If none of the following conditions are true: element is an input element whose type attribute is in either the Text, Search, Telephone, URL, or Email state and element is mutable;; element is a textarea element that is mutable; or; element is an editing host or is editable, then return false.
  - 3. If element has an inclusive ancestor with a writingsuggestions content attribute that's not in the Default and the nearest such ancestor's writingsuggestions content attribute is in the False state, then return false.
  - 4. Otherwise, return true.

#### [6.8.7 Autocapitalization](https://html.spec.whatwg.org/#autocapitalization)

Status: reviewed (2026-11-01)
> `HTMLElement::Autocapitalize`; the used autocapitalization hint isn't computed, as no input method takes one.
> Fixed: the getter was empty.
<!-- definitions: autocapitalization-hint autocap-hint-default autocap-hint-none autocap-hint-sentences autocap-hint-words autocap-hint-characters attr-autocapitalize -->

- [x] [own autocapitalization hint](https://html.spec.whatwg.org/#own-autocapitalization-hint) — To compute the own autocapitalization hint of an element element, run the following steps:
  - 1. If the autocapitalize content attribute is present on element, and its value is not the empty string, return the state of the attribute.
  - 2. If element is an autocapitalize-and-autocorrect inheriting element and has a non-null form owner, return the own autocapitalization hint of element's form owner.
  - 3. Return Default.

- [x] [HTMLElement.autocapitalize](https://html.spec.whatwg.org/#dom-autocapitalize) — The autocapitalize getter steps are to:
  - 1. Let state be the own autocapitalization hint of this.
  - 2. If state is Default, then return the empty string.
  - 3. If state is None, then return "none".
  - 4. If state is Sentences, then return "sentences".
  - 5. Return the keyword value corresponding to state.

- [x] [The used autocapitalization hint for an element element is computed using the f…](https://html.spec.whatwg.org/#used-autocapitalization-hint) — The used autocapitalization hint for an element element is computed using the following algorithm:
  - 1. If element is an input element whose type attribute is in one of the URL, Email, or Password states, then return Default.
  - 2. If element is an input element or a textarea element, then return element's own autocapitalization hint.
  - 3. If element is an editing host or an editable element, then return the own autocapitalization hint of the editing host of element.
  - 4. Assert: this step is never reached, since text input only occurs in elements that meet one of the above criteria.

#### [6.8.8 Autocorrection](https://html.spec.whatwg.org/#autocorrection)

Status: reviewed (2026-11-01)
> `HTMLElement::Autocorrect`.
> Fixed: the getter was false.
<!-- definitions: attr-autocorrect -->

- [x] [HTMLElement.autocorrect](https://html.spec.whatwg.org/#dom-autocorrect) — The autocorrect getter steps are: return true if the element's used autocorrection state is On and false if the element's used autocorrection state is Off. The setter steps are: if the given value is true, then the element's autocorrect attribute must be set to "on"; otherwise it must be set to "off".

- [x] [used autocorrection state](https://html.spec.whatwg.org/#used-autocorrection-state) — To compute the used autocorrection state of an element element, run these steps:
  - 1. If element is an input element whose type attribute is in one of the URL, Email, or Password states, then return Off.
  - 2. If the autocorrect content attribute is present on element, then return the state of the attribute.
  - 3. If element is an autocapitalize-and-autocorrect inheriting element and has a non-null form owner, then return the state of element's form owner's autocorrect attribute.
  - 4. Return On.

#### [6.8.9 Input modalities: the inputmode attribute](https://html.spec.whatwg.org/#input-modalities:-the-inputmode-attribute)

Status: reviewed (2026-11-01)
> `HTMLElement::InputMode` (added); there's no virtual keyboard to take it.
<!-- definitions: attr-inputmode -->

- [x] [ElementContentEditable.inputMode](https://html.spec.whatwg.org/#dom-inputmode) — The inputMode IDL attribute must reflect the inputmode content attribute, limited to only known values.

#### [6.8.10 Input modalities: the enterkeyhint attribute](https://html.spec.whatwg.org/#input-modalities:-the-enterkeyhint-attribute)

Status: reviewed (2026-11-01)
> `HTMLElement::EnterKeyHint` (added); there's no virtual keyboard to take it.
<!-- definitions: attr-enterkeyhint -->

- [x] [ElementContentEditable.enterKeyHint](https://html.spec.whatwg.org/#dom-enterkeyhint) — The enterKeyHint IDL attribute must reflect the enterkeyhint content attribute, limited to only known values.

### [6.9 Find-in-page](https://html.spec.whatwg.org/#find-in-page)

Status: n/a (no find-in-page)

#### [6.9.1 Introduction](https://html.spec.whatwg.org/#introduction-10)

Status: n/a (no find-in-page)
<!-- definitions: find-in-page-2 find-in-page-interface fip-query fip-matches fip-active-match -->

#### [6.9.2 Interaction with details and hidden=until-found](https://html.spec.whatwg.org/#interaction-with-details-and-hidden=until-found)

Status: n/a (no find-in-page)

- [x] [When find-in-page chooses a new active match, perform the following steps](https://html.spec.whatwg.org/#interaction-with-details-and-hidden=until-found) — When find-in-page chooses a new active match, perform the following steps:
  - 1. Let node be the first node in the active match.
  - 2. Queue a global task on the user interaction task source given node's relevant global object to run the ancestor revealing algorithm on node.

#### [6.9.3 Interaction with selection](https://html.spec.whatwg.org/#interaction-with-selection)

Status: n/a (no find-in-page)

### [6.10 Close requests and close watchers](https://html.spec.whatwg.org/#close-requests-and-close-watchers)

Status: reviewed (2026-11-01)
> The sections below.

#### [6.10.1 Close requests](https://html.spec.whatwg.org/#close-requests)

Status: reviewed (2026-11-01)
> Escape's keydown default action (`CloseWatcherManager::EscapeKeyHandler`); there's no fullscreen to exit.
<!-- definitions: close-request -->

- [x] [close request steps](https://html.spec.whatwg.org/#close-request-steps) — Whenever the user agent receives a potential close request targeted at a Document document, it must queue a global task on the user interaction task source given document's relevant global object to perform the following close request steps:
  - 1. If document's fullscreen element is not null:
    - 1a. Fully exit fullscreen given document's node navigable's top-level traversable's active document.
    - 1b. Return.
  - 2. Optionally, skip to the step labeled alternative processing.
  - 3. Fire any relevant events, per UI Events or other relevant specifications. [UIEVENTS]
  - 4. Let event be null if no such events are fired, or the Event object representing one of the fired events otherwise. If multiple events are fired, which one is chosen is implementation-defined.
  - 5. If event is not null, and its canceled flag is set, then return.
  - 6. If document is not fully active, then return.
  - 7. Let closedSomething be the result of processing close watchers on document's relevant global object.
  - 8. If closedSomething is true, then return.
  - 9. Alternative processing: Otherwise, there was nothing watching for a close request. The user agent may instead interpret this interaction as some other action, instead of interpreting it as a close request.

#### [6.10.2 Close watcher infrastructure](https://html.spec.whatwg.org/#close-watcher-infrastructure)

Status: reviewed (2026-11-01)
> `CloseWatcherManager` and `CloseWatcher`; a watcher's activeness is a flag kept with the manager's groups.
<!-- definitions: close-watcher-manager close-watcher-manager-groups close-watcher-manager-allowed-number-of-groups close-watcher-manager-next close-watcher close-watcher-window close-watcher-cancel-action close-watcher-close-action close-watcher-is-running-cancel close-watcher-get-enabled-state create-close-watcher-cancelaction create-close-watcher-closeaction create-close-watcher-getenabledstate -->

- [x] [notify the close watcher manager about user activation](https://html.spec.whatwg.org/#notify-the-close-watcher-manager-about-user-activation) — To notify the close watcher manager about user activation given a Window window:
  - 1. Let manager be window's close watcher manager.
  - 2. If manager's next user interaction allows a new group is true, then increment manager's allowed number of groups.
  - 3. Set manager's next user interaction allows a new group to false.

- [x] [is active](https://html.spec.whatwg.org/#close-watcher-active) — A close watcher closeWatcher is active if closeWatcher's window's close watcher manager's groups contains any list which contains closeWatcher.

- [x] [establish a close watcher](https://html.spec.whatwg.org/#establish-a-close-watcher) — To establish a close watcher given a Window window, a list of steps cancelAction, a list of steps closeAction, and an algorithm that returns a boolean getEnabledState:
  - 1. Assert: window's associated Document is fully active.
  - 2. Let closeWatcher be a new close watcher, with windowwindowcancel actioncancelActionclose actioncloseActionis running cancel actionfalseget enabled stategetEnabledState
  - 3. Let manager be window's close watcher manager.
  - 4. If manager's groups's size is less than manager's allowed number of groups, then append « closeWatcher » to manager's groups.
  - 5. Otherwise:
    - 5a. Assert: manager's groups's size is at least 1 in this branch, since manager's allowed number of groups is always at least 1.
    - 5b. Append closeWatcher to manager's groups's last item.
  - 6. Set manager's next user interaction allows a new group to true.
  - 7. Return closeWatcher.

- [x] [request to close](https://html.spec.whatwg.org/#close-watcher-request-close) — To request to close a close watcher closeWatcher with boolean requireHistoryActionActivation:
  - 1. If closeWatcher is not active, then return true.
  - 2. If the result of running closeWatcher's get enabled state is false, then return true.
  - 3. If closeWatcher's is running cancel action is true, then return true.
  - 4. Let window be closeWatcher's window.
  - 5. If window's associated Document is not fully active, then return true.
  - 6. Let canPreventClose be true if requireHistoryActionActivation is false, or if window's close watcher manager's groups's size is less than window's close watcher manager's allowed number of groups, and window has history-action activation; otherwise false.
  - 7. Set closeWatcher's is running cancel action to true.
  - 8. Let shouldContinue be the result of running closeWatcher's cancel action given canPreventClose.
  - 9. Set closeWatcher's is running cancel action to false.
  - 10. If shouldContinue is false:
    - 10a. Assert: canPreventClose is true.
    - 10b. Consume history-action user activation given window.
    - 10c. Return false.
  - 11. Close closeWatcher.
  - 12. Return true.

- [x] [close](https://html.spec.whatwg.org/#close-watcher-close) — To close a close watcher closeWatcher:
  - 1. If closeWatcher is not active, then return.
  - 2. If the result of running closeWatcher's get enabled state is false, then return.
  - 3. If closeWatcher's window's associated Document is not fully active, then return.
  - 4. Destroy closeWatcher.
  - 5. Run closeWatcher's close action.

- [x] [destroy](https://html.spec.whatwg.org/#close-watcher-destroy) — To destroy a close watcher closeWatcher:
  - 1. Let manager be closeWatcher's window's close watcher manager.
  - 2. For each group of manager's groups: remove closeWatcher from group.
  - 3. Remove any item from manager's groups that is empty.

- [x] [process close watchers](https://html.spec.whatwg.org/#process-close-watchers) — To process close watchers given a Window window:
  - 1. Let processedACloseWatcher be false.
  - 2. If window's close watcher manager's groups is not empty:
    - 2a. Let group be the last item in window's close watcher manager's groups.
    - 2b. For each closeWatcher of group, in reverse order:
      - 2b.i. If the result of running closeWatcher's get enabled state is true, set processedACloseWatcher to true.
      - 2b.ii. Let shouldProceed be the result of requesting to close closeWatcher with true.
      - 2b.iii. If shouldProceed is false, then break.
  - 3. If window's close watcher manager's allowed number of groups is greater than 1, decrement it by 1.
  - 4. Return processedACloseWatcher.

#### [6.10.3 The CloseWatcher interface](https://html.spec.whatwg.org/#the-closewatcher-interface)

Status: reviewed (2026-11-01)
> `CloseWatcher`, its own internal close watcher.
<!-- definitions: internal-close-watcher -->

- [x] [new CloseWatcher(options)](https://html.spec.whatwg.org/#dom-closewatcher) — The new CloseWatcher(options) constructor steps are:
  - 1. If this's relevant global object's associated Document is not fully active, then throw an "InvalidStateError" DOMException.
  - 2. Let closeWatcher be the result of establishing a close watcher given this's relevant global object, with: cancelAction given canPreventClose being to return the result of firing an event named cancel at this, with the cancelable attribute initialized to canPreventClose.; closeAction being to fire an event named close at this.; getEnabledState being to return true.
  - 3. If options["signal"] exists:
    - 3a. If options["signal"] is aborted, then destroy closeWatcher.
    - 3b. Add the following steps to options["signal"]:
      - 3b.i. Destroy closeWatcher.
  - 4. Set this's internal close watcher to closeWatcher.

- [x] [CloseWatcher.requestClose()](https://html.spec.whatwg.org/#dom-closewatcher-requestclose) — The requestClose() method steps are to request to close this's internal close watcher with false.

- [x] [CloseWatcher.close()](https://html.spec.whatwg.org/#dom-closewatcher-close) — The close() method steps are to close this's internal close watcher.

- [x] [CloseWatcher.destroy()](https://html.spec.whatwg.org/#dom-closewatcher-destroy) — The destroy() method steps are to destroy this's internal close watcher.
