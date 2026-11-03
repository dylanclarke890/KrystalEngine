# HTML review checklist: 11 Worklets

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 8 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html worklets` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [11 Worklets](https://html.spec.whatwg.org/#worklets)

Status: n/a (no JavaScript)

### [11.1 Introduction](https://html.spec.whatwg.org/#worklets-intro)

Status: n/a (no JavaScript)

#### [11.1.1 Motivations](https://html.spec.whatwg.org/#worklets-motivations)

Status: n/a (no JavaScript)

#### [11.1.2 Code idempotence](https://html.spec.whatwg.org/#worklets-idempotent)

Status: n/a (no JavaScript)

#### [11.1.3 Speculative evaluation](https://html.spec.whatwg.org/#worklets-speculative)

Status: n/a (no JavaScript)

### [11.2 Examples](https://html.spec.whatwg.org/#worklets-examples)

Status: n/a (no JavaScript)
<!-- definitions: fake-worklet-1 fake-worklet-2 registered-class-constructors-map -->

- [x] [fakeWorklet1](https://html.spec.whatwg.org/#fakeworklet1) — The fakeWorklet1 getter steps are to return this's fake worklet 1.

- [x] [fakeWorklet2](https://html.spec.whatwg.org/#fakeworklet2) — The fakeWorklet2 getter steps are to return this's fake worklet 2.

- [x] [registerFake(type, classConstructor)](https://html.spec.whatwg.org/#dom-fakeworkletglobalscope-registerfake) — The registerFake(type, classConstructor) method steps are to set this's registered class constructors map[type] to classConstructor.

#### [11.2.1 Loading scripts](https://html.spec.whatwg.org/#worklets-examples-loading)

Status: n/a (no JavaScript)

#### [11.2.2 Registering a class and invoking its methods](https://html.spec.whatwg.org/#worklets-example-registering)

Status: n/a (no JavaScript)

- [x] [find the opposite of true](https://html.spec.whatwg.org/#fakeworkletglobalscope-process) — To make use of such registered classes, the specification for fake worklets could define a find the opposite of true algorithm, given a Worklet worklet:
  - 1. Optionally, create a worklet global scope for worklet.
  - 2. Let workletGlobalScope be one of worklet's global scopes, chosen in an implementation-defined manner.
  - 3. Let classConstructor be workletGlobalScope's registered class constructors map["negation-processor"].
  - 4. Let classInstance be the result of constructing classConstructor, with no arguments.
  - 5. Let function be Get(classInstance, "process"). Rethrow any exceptions.
  - 6. Let callback be the result of converting function to a Web IDL Function instance.
  - 7. Return the result of invoking callback with « true » and "rethrow", and with callback this value set to classInstance.

### [11.3 Infrastructure](https://html.spec.whatwg.org/#worklets-infrastructure)

Status: n/a (no JavaScript)

#### [11.3.1 The global scope](https://html.spec.whatwg.org/#worklets-global)

Status: n/a (no JavaScript)
<!-- definitions: concept-workletglobalscope-module-map -->

##### [11.3.1.1 Agents and event loops](https://html.spec.whatwg.org/#worklet-agents-and-event-loops)

Status: n/a (no JavaScript)

##### [11.3.1.2 Creation and termination](https://html.spec.whatwg.org/#worklets-creation-termination)

Status: n/a (no JavaScript)

- [x] [create a worklet global scope](https://html.spec.whatwg.org/#create-a-worklet-global-scope) — To create a worklet global scope for a Worklet worklet:
  - 1. Let outsideSettings be worklet's relevant settings object.
  - 2. Let agent be the result of obtaining a worklet agent given outsideSettings. Run the rest of these steps in that agent.
  - 3. Let realmExecutionContext be the result of creating a new realm given agent and the following customizations: For the global object, create a new object of the type given by worklet's worklet global scope type.
  - 4. Let workletGlobalScope be the global object of realmExecutionContext's Realm component.
  - 5. Let insideSettings be the result of setting up a worklet environment settings object given realmExecutionContext and outsideSettings.
  - 6. Let pendingAddedModules be a clone of worklet's added modules list.
  - 7. Let runNextAddedModule be the following steps:
    - 7a. If pendingAddedModules is not empty:
      - 7a.i. Let moduleURL be the result of dequeuing from pendingAddedModules.
      - 7a.ii. Fetch a worklet script graph given moduleURL, insideSettings, worklet's worklet destination type, , insideSettings, worklet's module responses map, and with the following steps given script:
        - 7a.ii.A. Assert: script is not null, since the fetch succeeded and the source text was successfully parsed when worklet's module responses map was initially populated with moduleURL.
        - 7a.ii.B. Run a module script given script.
        - 7a.ii.C. Run runNextAddedModule.
      - 7a.iii. Abort these steps.
    - 7b. Append workletGlobalScope to outsideSettings's global object's associated Document's worklet global scopes.
    - 7c. Append workletGlobalScope to worklet's global scopes.
    - 7d. Run the responsible event loop specified by insideSettings.
  - 8. Run runNextAddedModule.

- [x] [terminate a worklet global scope](https://html.spec.whatwg.org/#terminate-a-worklet-global-scope) — To terminate a worklet global scope given a WorkletGlobalScope workletGlobalScope:
  - 1. Let eventLoop be workletGlobalScope's relevant agent's event loop.
  - 2. If there are any tasks queued in eventLoop's task queues, discard them without processing them.
  - 3. Wait for eventLoop to complete the currently running task.
  - 4. If the previous step doesn't complete within an implementation-defined period of time, then abort the script currently running in the worklet.
  - 5. Destroy eventLoop.
  - 6. Remove workletGlobalScope from the global scopes of the Worklet whose global scopes contains workletGlobalScope.
  - 7. Remove workletGlobalScope from the worklet global scopes of the Document whose worklet global scopes contains workletGlobalScope.

##### [11.3.1.3 Script settings for worklets](https://html.spec.whatwg.org/#script-settings-for-worklets)

Status: n/a (no JavaScript)

- [x] [set up a worklet environment settings object](https://html.spec.whatwg.org/#set-up-a-worklet-environment-settings-object) — To set up a worklet environment settings object, given a JavaScript execution context executionContext and an environment settings object outsideSettings:
  - 1. Let origin be a unique opaque origin.
  - 2. Let inheritedAPIBaseURL be outsideSettings's API base URL.
  - 3. Let inheritedPolicyContainer be a clone of outsideSettings's policy container.
  - 4. Let realm be the value of executionContext's Realm component.
  - 5. Let workletGlobalScope be realm's global object.
  - 6. Let settingsObject be a new environment settings object whose algorithms are defined as follows: The realm execution context Return executionContext. The module map Return workletGlobalScope's module map. The API base URL Return inheritedAPIBaseURL. The origin Return origin. The has cross-site ancestorReturn true.The policy container Return inheritedPolicyContainer. The cross-origin isolated capa…
  - 7. Set settingsObject's id to a new unique opaque string, creation URL to inheritedAPIBaseURL, top-level creation URL to null, top-level origin to outsideSettings's top-level origin, target browsing context to null, and active service worker to null.
  - 8. Set realm's [[HostDefined]] field to settingsObject.
  - 9. Return settingsObject.

#### [11.3.2 The Worklet class](https://html.spec.whatwg.org/#worklets-worklet)

Status: n/a (no JavaScript)
<!-- definitions: worklet-global-scope-type worklet-destination-type concept-worklet-global-scopes concept-worklet-added-modules-list concept-worklet-module-responses-map -->

- [x] [Worklet.addModule(moduleURL, options)](https://html.spec.whatwg.org/#dom-worklet-addmodule) — The addModule(moduleURL, options) method steps are:
  - 1. Let outsideSettings be the relevant settings object of this.
  - 2. Let moduleURLRecord be the result of encoding-parsing a URL given moduleURL, relative to outsideSettings.
  - 3. If moduleURLRecord is failure, then return a promise rejected with a "SyntaxError" DOMException.
  - 4. Let promise be a new promise.
  - 5. Let workletInstance be this.
  - 6. Run the following steps in parallel:
    - 6a. If workletInstance's global scopes is empty:
      - 6a.i. Create a worklet global scope given workletInstance.
      - 6a.ii. Optionally, create additional global scope instances given workletInstance, depending on the specific worklet in question and its specification.
      - 6a.iii. Wait for all steps of the creation process(es) — including those taking place within the worklet agents — to complete, before moving on.
    - 6b. Let pendingTasks be workletInstance's global scopes's size.
    - 6c. Let addedSuccessfully be false.
    - 6d. For each workletGlobalScope of workletInstance's global scopes, queue a global task on the networking task source given workletGlobalScope to fetch a worklet script graph given moduleURLRecord, outsideSettings, workletInstance's worklet destination type, options["credentials"], workletGlobalScope's relevant settings object, workletInstance's module responses map, and the following steps given scr…
      - 6d.i. If script is null:
        - 6d.i.A. Queue a global task on the networking task source given workletInstance's relevant global object to perform the following steps:
          - 6d.i.A.1. If pendingTasks is not −1:
            - 6d.i.A.1a. Set pendingTasks to −1.
            - 6d.i.A.1b. Reject promise with an "AbortError" DOMException.
        - 6d.i.B. Abort these steps.
      - 6d.ii. If script's error to rethrow is not null:
        - 6d.ii.A. Queue a global task on the networking task source given workletInstance's relevant global object to perform the following steps:
          - 6d.ii.A.1. If pendingTasks is not −1:
            - 6d.ii.A.1a. Set pendingTasks to −1.
            - 6d.ii.A.1b. Reject promise with script's error to rethrow.
        - 6d.ii.B. Abort these steps.
      - 6d.iii. If addedSuccessfully is false:
        - 6d.iii.A. Append moduleURLRecord to workletInstance's added modules list.
        - 6d.iii.B. Set addedSuccessfully to true.
      - 6d.iv. Run a module script given script.
      - 6d.v. Queue a global task on the networking task source given workletInstance's relevant global object to perform the following steps:
        - 6d.v.A. If pendingTasks is not −1:
          - 6d.v.A.1. Set pendingTasks to pendingTasks − 1.
          - 6d.v.A.2. If pendingTasks is 0, then resolve promise.
  - 7. Return promise.

#### [11.3.3 The worklet's lifetime](https://html.spec.whatwg.org/#worklets-lifetime)

Status: n/a (no JavaScript)
<!-- definitions: concept-document-worklet-global-scopes -->
