# HTML review checklist: 7.4 Navigation and session history

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 64 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html browsing-the-web` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [7.4 Navigation and session history](https://html.spec.whatwg.org/#navigation-and-session-history)

Status: reviewed (2026-11-01)
> The sections below.

#### [7.4.1 Session history](https://html.spec.whatwg.org/#session-history-infrastructure)

Status: reviewed (2026-11-01)
> The sections below.

##### [7.4.1.1 Session history entries](https://html.spec.whatwg.org/#session-history-entries)

Status: reviewed (2026-11-01)
> A `HistoryItem` is the session history entry: its URL, document sequence number, state objects (classic and Navigation API), scroll restoration mode and scroll position; its key is `UUIDIdentifier`. Documents are kept by the back/forward cache rather than the entry.
<!-- definitions: session-history-entry she-step she-url she-document-state she-classic-history-api-state she-navigation-api-state she-navigation-api-key she-navigation-api-id she-scroll-restoration-mode she-scroll-position she-other serialized-state scroll-restoration-mode dom-scrollrestoration-auto dom-scrollrestoration-manual -->

- [x] [document](https://html.spec.whatwg.org/#she-document) — To get a session history entry's document, return its document state's document.

##### [7.4.1.2 Document state](https://html.spec.whatwg.org/#document-state)

Status: reviewed (2026-11-01)
> The document state is spread over the `HistoryItem` (the document sequence number, the target name is the frame's) and the `DocumentLoader` (the request). There are no origins, policy containers or reload pending flags.
<!-- definitions: document-state-2 document-state-document document-state-history-policy-container document-state-request-referrer document-state-request-referrer-policy document-state-initiator-origin document-state-origin document-state-about-base-url document-state-nested-histories document-state-resource document-state-reload-pending document-state-ever-populated document-state-nav-target-name document-state-not-restored-reasons post-resource post-resource-request-body post-resource-request-content-type nested-history nested-history-id nested-history-entries latest-entry -->

##### [7.4.1.3 Centralized modifications of session history](https://html.spec.whatwg.org/#centralized-modifications-of-session-history)

Status: reviewed (2026-11-01)
> There's no session history traversal queue: traversals and navigations are `NavigationScheduler` tasks on the event loop.
<!-- definitions: session-history-traversal-parallel-queue session-history-traversal-parallel-queue-algorithm-set session-history-traversal-parallel-queue-sync-nav-steps session-history-traversal-parallel-queue-sync-nav-steps-target-nav -->

- [x] [append session history traversal steps](https://html.spec.whatwg.org/#tn-append-session-history-traversal-steps) — To append session history traversal steps to a traversable navigable traversable given algorithm steps steps, append steps to traversable's session history traversal queue's algorithm set.

- [x] [append session history synchronous navigation steps](https://html.spec.whatwg.org/#tn-append-session-history-sync-nav-steps) — To append session history synchronous navigation steps involving a navigable targetNavigable to a traversable navigable traversable given algorithm steps steps, append steps as synchronous navigation steps targeting target navigable targetNavigable to traversable's session history traversal queue's algorithm set.

- [x] [start a new session history traversal parallel queue](https://html.spec.whatwg.org/#starting-a-new-session-history-traversal-parallel-queue) — To start a new session history traversal parallel queue:
  - 1. Let sessionHistoryTraversalQueue be a new session history traversal parallel queue.
  - 2. Run the following steps in parallel:
    - 2a. While true:
      - 2a.i. If sessionHistoryTraversalQueue's algorithm set is empty, then continue.
      - 2a.ii. Let steps be the result of dequeuing from sessionHistoryTraversalQueue's algorithm set.
      - 2a.iii. Run steps.
  - 3. Return sessionHistoryTraversalQueue.

##### [7.4.1.4 Low-level operations on session history](https://html.spec.whatwg.org/#low-level-operations-on-session-history)

Status: reviewed (2026-11-01)
> `BackForwardController` holds the entries; pushing an item clears the forward entries. `Navigation::FilterHistoryItemsForNavigationAPI` gets the Navigation API's entries, contiguous items of the same origin.

- [x] [get session history entries](https://html.spec.whatwg.org/#getting-session-history-entries) — To get session history entries of a navigable navigable:
  - 1. Let traversable be navigable's traversable navigable.
  - 2. Assert: this is running within traversable's session history traversal queue.
  - 3. If navigable is traversable, return traversable's session history entries.
  - 4. Let docStates be an empty ordered set of document states.
  - 5. For each entry of traversable's session history entries, append entry's document state to docStates.
  - 6. For each docState of docStates:
    - 6a. For each nestedHistory of docState's nested histories:
      - 6a.i. If nestedHistory's id equals navigable's id, return nestedHistory's entries.
      - 6a.ii. For each entry of nestedHistory's entries, append entry's document state to docStates.
  - 7. Assert: this step is not reached.

- [x] [get session history entries for the navigation API](https://html.spec.whatwg.org/#getting-session-history-entries-for-the-navigation-api) — To get session history entries for the navigation API of a navigable navigable given an integer targetStep:
  - 1. Let rawEntries be the result of getting session history entries for navigable.
  - 2. Let entriesForNavigationAPI be a new empty list.
  - 3. Let startingIndex be the index of the session history entry in rawEntries who has the greatest step less than or equal to targetStep.
  - 4. Append rawEntries[startingIndex] to entriesForNavigationAPI.
  - 5. Let startingOrigin be rawEntries[startingIndex]'s document state's origin.
  - 6. Let i be startingIndex − 1.
  - 7. While i > 0:
    - 7a. If rawEntries[i]'s document state's origin is not same origin with startingOrigin, then break.
    - 7b. Prepend rawEntries[i] to entriesForNavigationAPI.
    - 7c. Set i to i − 1.
  - 8. Set i to startingIndex + 1.
  - 9. While i < rawEntries's size:
    - 9a. If rawEntries[i]'s document state's origin is not same origin with startingOrigin, then break.
    - 9b. Append rawEntries[i] to entriesForNavigationAPI.
    - 9c. Set i to i + 1.
  - 10. Return entriesForNavigationAPI.

- [x] [clear the forward session history](https://html.spec.whatwg.org/#clear-the-forward-session-history) — To clear the forward session history of a traversable navigable navigable:
  - 1. Assert: this is running within navigable's session history traversal queue.
  - 2. Let step be the navigable's current session history step.
  - 3. Let entryLists be the ordered set « navigable's session history entries ».
  - 4. For each entryList of entryLists:
    - 4a. Remove every session history entry from entryList that has a step greater than step.
    - 4b. For each entry of entryList:
      - 4b.i. For each nestedHistory of entry's document state's nested histories, append nestedHistory's entries list to entryLists.

- [x] [get all used history steps](https://html.spec.whatwg.org/#getting-all-used-history-steps) — To get all used history steps that are part of traversable navigable traversable:
  - 1. Assert: this is running within traversable's session history traversal queue.
  - 2. Let steps be an empty ordered set of non-negative integers.
  - 3. Let entryLists be the ordered set « traversable's session history entries ».
  - 4. For each entryList of entryLists:
    - 4a. For each entry of entryList:
      - 4a.i. Append entry's step to steps.
      - 4a.ii. For each nestedHistory of entry's document state's nested histories, append nestedHistory's entries list to entryLists.
  - 5. Return steps, sorted.

#### [7.4.2 Navigation](https://html.spec.whatwg.org/#navigating-across-documents)

Status: reviewed (2026-11-01)
> The sections below.

##### [7.4.2.1 Supporting concepts](https://html.spec.whatwg.org/#navigation-supporting-concepts)

Status: reviewed (2026-11-01)
> There are no source or target snapshot params (see `KNOWN-ISSUES.md`); the navigation must be a replace for the initial about:blank document (`FrameLoader::UpdateURLAndHistory`, `Navigation::Navigate`). User involvement is `NavigationAction::IsRequestFromClientOrUserInput` and the window's transient activation.
<!-- definitions: source-snapshot-params source-snapshot-params-activation source-snapshot-params-sandbox source-snapshot-params-download source-snapshot-params-client source-snapshot-params-policy-container target-snapshot-params target-snapshot-params-sandbox target-snapshot-params-iframe-referrer-policy navigation-params navigation-params-id navigation-params-navigable navigation-params-request navigation-params-response navigation-params-fetch-controller navigation-params-commit-early-hints navigation-params-coop-enforcement-result navigation-params-reserved-environment navigation-params-origin navigation-params-policy-container navigation-params-sandboxing navigation-params-iframe-referrer-policy navigation-params-coop navigation-params-nav-timing-type navigation-params-about-base-url navigation-params-user-involvement navigation-id navigationhistorybehavior-push navigationhistorybehavior-replace navigationhistorybehavior-auto history-handling-behavior user-navigation-involvement uni-browser-ui uni-activation uni-none -->

- [x] [snapshot source snapshot params](https://html.spec.whatwg.org/#snapshotting-source-snapshot-params) — To snapshot source snapshot params given a Document-or-null sourceDocument:
  - 1. If sourceDocument is null, then return a new source snapshot params with has transient activationtruesandboxing flagsan empty sandboxing flag setallows downloadingtruefetch clientnullsource policy containera new policy container
  - 2. Return a new source snapshot params with has transient activationtrue if sourceDocument's relevant global object has transient activation; otherwise falsesandboxing flagssourceDocument's active sandboxing flag setallows downloadingfalse if sourceDocument's active sandboxing flag set has the sandboxed downloads browsing context flag set; otherwise truefetch clientsourceDocument's relevant settings…

- [x] [snapshot target snapshot params](https://html.spec.whatwg.org/#snapshotting-target-snapshot-params) — To snapshot target snapshot params given a navigable targetNavigable, return a new target snapshot params with:
  - • sandboxing flags: the result of determining the creation sandboxing flags given targetNavigable's active browsing context and targetNavigable's container
  - • iframe element referrer policy: the result of determining the iframe element referrer policy given targetNavigable's container

- [x] [The navigation must be a replace](https://html.spec.whatwg.org/#the-navigation-must-be-a-replace) — The navigation must be a replace, given a URL url and a Document document, if any of the following are true:

- [x] [user navigation involvement (Event)](https://html.spec.whatwg.org/#event-uni) — For convenience at certain call sites, the user navigation involvement for an Event event is defined as follows:
  - 1. Assert: this algorithm is being called as part of an activation behavior definition.
  - 2. Assert: event's type is "click".
  - 3. If event's isTrusted is initialized to true, then return "activation".
  - 4. Return "none".

##### [7.4.2.2 Beginning navigation](https://html.spec.whatwg.org/#beginning-navigation)

Status: reviewed (2026-11-01)
> Fixed: a frame doesn't navigate while its document's beforeunload, pagehide or unload is dispatched (step 9: `FrameLoader::IsNavigationAllowed`, checked by `LoadURL`, `LoadPostRequest` and `NavigationScheduler`).
> `FrameLoader::LoadURL` and `LoadPostRequest`: "auto" history handling is a replace for the document's own URL, and a URL that differs only in its fragment is a fragment navigation. Follows upstream's `FrameLoader` and `HistoryController` rather than these steps (see `DIFFERENCES.md`).
<!-- definitions: navigation-resource navigation-response exceptions-enabled navigation-hh navigation-navigation-api-state navigation-form-data-entry-list navigation-referrer-policy navigation-user-involvement navigation-source-element navigation-initial-insertion navigation-api-method-tracker-2 -->

- [x] [navigate](https://html.spec.whatwg.org/#navigate) — To navigate a navigable navigable to a URL url using an optional Document-or-null sourceDocument (default null), with an optional POST resource, string, or null documentResource (default null), an optional response-or-null response (default null), an optional boolean exceptionsEnabled (default fals…
  - 1. Let cspNavigationType be "form-submission" if formDataEntryList is non-null; otherwise "other".
  - 2. Let sourceSnapshotParams be the result of snapshotting source snapshot params given sourceDocument.
  - 3. Let initiatorOriginSnapshot be a new opaque origin.
  - 4. Let initiatorBaseURLSnapshot be about:blank.
  - 5. If sourceDocument is null:
    - 5a. Assert: userInvolvement is "browser UI".
    - 5b. If url's scheme is "javascript", then set initiatorOriginSnapshot to navigable's active document's origin.
  - 6. Otherwise:
    - 6a. Assert: userInvolvement is not "browser UI".
    - 6b. If sourceDocument's node navigable is not allowed by sandboxing to navigate navigable given sourceSnapshotParams:
      - 6b.i. If exceptionsEnabled is true, then throw a "SecurityError" DOMException.
      - 6b.ii. Return.
    - 6c. Set initiatorOriginSnapshot to sourceDocument's origin.
    - 6d. Set initiatorBaseURLSnapshot to sourceDocument's document base URL.
  - 7. Let navigationId be the result of generating a random UUID. [WEBCRYPTO]
  - 8. If the surrounding agent is equal to navigable's active document's relevant agent, then continue these steps. Otherwise, queue a global task on the navigation and traversal task source given navigable's active window to continue these steps.
  - 9. If navigable's active document's unload counter is greater than 0, then invoke WebDriver BiDi navigation failed with navigable and a WebDriver BiDi navigation status whose id is navigationId, status is "canceled", and url is url, and return.
  - 10. Let container be navigable's container.
  - 11. If container is an iframe element and will lazy load element steps given container returns true, then stop intersection-observing a lazy loading element container and set container's lazy load resumption steps to null.
  - 12. If navigable's allowed to perform a navigation or history update returns blocked, then invoke WebDriver BiDi navigation failed with navigable and a WebDriver BiDi navigation status whose id is navigationId, status is "canceled", and url is url, and return.
  - 13. If historyHandling is "auto":
    - 13a. If url equals navigable's active document's URL, and either userInvolvement is "browser UI" or initiatorOriginSnapshot is same origin with navigable's active document's origin, then set historyHandling to "replace".
    - 13b. Otherwise, set historyHandling to "push".
  - 14. If the navigation must be a replace given url and navigable's active document, then set historyHandling to "replace".
  - 15. If all of the following are true: documentResource is null;; response is null;; url equals navigable's active session history entry's URL with exclude fragments set to true; and; url's fragment is non-null, then:
    - 15a. Navigate to a fragment given navigable, url, historyHandling, userInvolvement, sourceElement, navigationAPIState, and navigationId.
    - 15b. Return.
  - 16. If navigable's parent is non-null, then set navigable's is delaying load events to true.
  - 17. Let targetSnapshotParams be the result of snapshotting target snapshot params given navigable.
  - 18. Invoke WebDriver BiDi navigation started with navigable and a new WebDriver BiDi navigation status whose id is navigationId, status is "pending", and url is url.
  - 19. If navigable's ongoing navigation is "traversal":
    - 19a. Invoke WebDriver BiDi navigation failed with navigable and a new WebDriver BiDi navigation status whose id is navigationId, status is "canceled", and url is url.
    - 19b. Return.
  - 20. Set the ongoing navigation for navigable to navigationId.
  - 21. If url's scheme is "javascript":
    - 21a. Let request be a new request, with URLurlclientsourceSnapshotParams's fetch clientpolicy containersourceSnapshotParams's source policy container
    - 21b. Queue a global task on the navigation and traversal task source given navigable's active window to navigate to a javascript: URL given navigable, request, historyHandling, initiatorOriginSnapshot, userInvolvement, cspNavigationType, initialInsertion, and navigationId.
    - 21c. Return.
  - 22. If all of the following are true: userInvolvement is not "browser UI";; navigable's active document's origin is same origin-domain with sourceDocument's origin;; navigable's active document's is initial about:blank is false; and; url's scheme is a fetch scheme, then:
    - 22a. Let navigation be navigable's active window's navigation API.
    - 22b. Let entryListForFiring be formDataEntryList if documentResource is a POST resource; otherwise, null.
    - 22c. Let navigationAPIStateForFiring be navigationAPIState if navigationAPIState is not null; otherwise, StructuredSerializeForStorage(undefined).
    - 22d. Let continue be the result of firing a push/replace/reload navigate event at navigation with navigationType set to historyHandling, isSameDocument set to false, userInvolvement set to userInvolvement, sourceElement set to sourceElement, formDataEntryList set to entryListForFiring, destinationURL set to url, navigationAPIState set to navigationAPIStateForFiring, and apiMethodTracker set to apiMeth…
    - 22e. If continue is false, then return.
  - 23. If sourceDocument is navigable's container document, then reserve deferred fetch quota for navigable's container given url's origin.
  - 24. In parallel, run these steps:
    - 24a. Let unloadPromptCanceled be the result of checking if unloading is canceled for navigable's active document's inclusive descendant navigables.
    - 24b. If unloadPromptCanceled is not "continue", or navigable's ongoing navigation is no longer navigationId:
      - 24b.i. Invoke WebDriver BiDi navigation failed with navigable and a new WebDriver BiDi navigation status whose id is navigationId, status is "canceled", and url is url.
      - 24b.ii. Abort these steps.
    - 24c. Queue a global task on the navigation and traversal task source given navigable's active window to abort a document and its descendants given navigable's active document.
    - 24d. Let documentState be a new document state with request referrer policyreferrerPolicyinitiator origininitiatorOriginSnapshotresourcedocumentResourcenavigable target namenavigable's target name
    - 24e. If url matches about:blank or is about:srcdoc:
      - 24e.i. Set documentState's origin to initiatorOriginSnapshot.
      - 24e.ii. Set documentState's about base URL to initiatorBaseURLSnapshot.
    - 24f. Let historyEntry be a new session history entry, with its URL set to url and its document state set to documentState.
    - 24g. Let navigationParams be null.
    - 24h. If response is non-null:
      - 24h.i. Let sourcePolicyContainer be a clone of the sourceDocument's policy container, if sourceDocument is not null; otherwise null.
      - 24h.ii. Let policyContainer be the result of determining navigation params policy container given response's URL, null, sourcePolicyContainer, navigable's container document's policy container, and null.
      - 24h.iii. Let finalSandboxFlags be the union of targetSnapshotParams's sandboxing flags and policyContainer's CSP list's CSP-derived sandboxing flags.
      - 24h.iv. Let responseOrigin be the result of determining the origin given response's URL, finalSandboxFlags, and documentState's initiator origin.
      - 24h.v. Let coop be a new opener policy.
      - 24h.vi. Let coopEnforcementResult be a new opener policy enforcement result with urlresponse's URLoriginresponseOriginopener policycoop
      - 24h.vii. Set navigationParams to a new navigation params, with idnavigationIdnavigablenavigablerequestnullresponseresponsefetch controllernullcommit early hintsnullCOOP enforcement resultcoopEnforcementResultreserved environmentnulloriginresponseOriginpolicy containerpolicyContainerfinal sandboxing flag setfinalSandboxFlagsiframe element referrer policytargetSnapshotParams's iframe element referrer policy…
    - 24i. Attempt to populate the history entry's document for historyEntry, given navigable, "navigate", sourceSnapshotParams, targetSnapshotParams, userInvolvement, navigationId, navigationParams, cspNavigationType, with allowPOST set to true and completionSteps set to the following step:
      - 24i.i. Append session history traversal steps to navigable's traversable navigable to finalize a cross-document navigation given navigable, historyHandling, userInvolvement, and historyEntry.

##### [7.4.2.3 Ending navigation](https://html.spec.whatwg.org/#ending-navigation)

Status: reviewed (2026-11-01)
> The sections below.

###### [7.4.2.3.1 The usual cross-document navigation case](https://html.spec.whatwg.org/#the-usual-cross-document-navigation-case)

Status: reviewed (2026-11-01)
> Follows upstream's `FrameLoader` and `HistoryController` rather than these steps (see `DIFFERENCES.md`).

- [x] [finalize a cross-document navigation](https://html.spec.whatwg.org/#finalize-a-cross-document-navigation) — To finalize a cross-document navigation given a navigable navigable, a history handling behavior historyHandling, a user navigation involvement userInvolvement, and a session history entry historyEntry:
  - 1. Assert: this is running on navigable's traversable navigable's session history traversal queue.
  - 2. Set navigable's is delaying load events to false.
  - 3. If historyEntry's document is null, then return.
  - 4. If all of the following are true: navigable's parent is null;; historyEntry's document's browsing context is not an auxiliary browsing context whose opener browsing context is non-null; and; historyEntry's document's origin is not same origin with navigable's active document's origin, then set historyEntry's document state's navigable target name to the empty string.
  - 5. Let entryToReplace be navigable's active session history entry if historyHandling is "replace", otherwise null.
  - 6. Let traversable be navigable's traversable navigable.
  - 7. Let targetStep be null.
  - 8. Let targetEntries be the result of getting session history entries for navigable.
  - 9. If entryToReplace is null: Otherwise:
    - 9a. Clear the forward session history of traversable.
    - 9b. Set targetStep to traversable's current session history step + 1.
    - 9c. Set historyEntry's step to targetStep.
    - 9d. Append historyEntry to targetEntries.
    - 9a. Replace entryToReplace with historyEntry in targetEntries.
    - 9b. Set historyEntry's step to entryToReplace's step.
    - 9c. If historyEntry's document state's origin is same origin with entryToReplace's document state's origin, then set historyEntry's navigation API key to entryToReplace's navigation API key.
    - 9d. Set targetStep to traversable's current session history step.
  - 10. Apply the push/replace history step targetStep to traversable given historyHandling and userInvolvement.

###### [7.4.2.3.2 The javascript: URL special case](https://html.spec.whatwg.org/#the-javascript:-url-special-case)

Status: n/a (no JavaScript)

- [x] [navigate to a javascript: URL](https://html.spec.whatwg.org/#navigate-to-a-javascript:-url) — To navigate to a javascript: URL, given a navigable targetNavigable, a request request, a history handling behavior historyHandling, an origin initiatorOrigin, a user navigation involvement userInvolvement, a string cspNavigationType, a boolean initialInsertion, and a navigation ID navigationId:
  - 1. Assert: historyHandling is "replace".
  - 2. If targetNavigable's ongoing navigation is no longer navigationId, then return.
  - 3. Set the ongoing navigation for targetNavigable to null.
  - 4. If initiatorOrigin is not same origin-domain with targetNavigable's active document's origin, then return.
  - 5. If the result of should navigation request of type be blocked by Content Security Policy? given request and cspNavigationType is "Blocked", then return. [CSP]
  - 6. Let newDocument be the result of evaluating a javascript: URL given targetNavigable, request's URL, initiatorOrigin, userInvolvement, and navigationId.
  - 7. If newDocument is null:
    - 7a. Let container be targetNavigable's container.
    - 7b. If initialInsertion is true, container is non-null, and targetNavigable's active document's is initial about:blank is true:
      - 7b.i. If container is an iframe element, then run the iframe load event steps given container.
      - 7b.ii. Otherwise, fire an event named load at container.
    - 7c. Return.
  - 8. Assert: initiatorOrigin is newDocument's origin.
  - 9. Let entryToReplace be targetNavigable's active session history entry.
  - 10. Let oldDocState be entryToReplace's document state.
  - 11. Let documentState be a new document state with documentnewDocumenthistory policy containera clone of the oldDocState's history policy container if it is non-null; null otherwiserequest referreroldDocState's request referrerrequest referrer policyoldDocState's request referrer policy initiator origininitiatorOriginorigininitiatorOriginabout base URLoldDocState's about base URLresourcenullever popu…
  - 12. Let historyEntry be a new session history entry, with URLentryToReplace's URLdocument statedocumentState
  - 13. Append session history traversal steps to targetNavigable's traversable navigable to finalize a cross-document navigation with targetNavigable, historyHandling, userInvolvement, and historyEntry.

- [x] [evaluate a javascript: URL](https://html.spec.whatwg.org/#evaluate-a-javascript:-url) — To evaluate a javascript: URL given a navigable targetNavigable, a URL url, an origin newDocumentOrigin, a user navigation involvement userInvolvement, and a navigation ID navigationId:
  - 1. Let urlString be the result of running the URL serializer on url.
  - 2. Let encodedScriptSource be the result of removing the leading "javascript:" from urlString.
  - 3. Let scriptSource be the UTF-8 decoding of the percent-decoding of encodedScriptSource.
  - 4. Let settings be targetNavigable's active document's relevant settings object.
  - 5. Let baseURL be settings's API base URL.
  - 6. Let script be the result of creating a classic script given scriptSource, settings, baseURL, and the default script fetch options.
  - 7. Let evaluationStatus be the result of running the classic script script.
  - 8. Let result be null.
  - 9. If evaluationStatus is a normal completion, and evaluationStatus.[[Value]] is a String, then set result to evaluationStatus.[[Value]].
  - 10. Otherwise, return null.
  - 11. Let response be a new response with URLtargetNavigable's active document's URLheader list« (`Content-Type`, `text/html;charset=utf-8`) »bodythe UTF-8 encoding of result, as a body
  - 12. Let policyContainer be targetNavigable's active document's policy container.
  - 13. Let finalSandboxFlags be policyContainer's CSP list's CSP-derived sandboxing flags.
  - 14. Let coop be targetNavigable's active document's opener policy.
  - 15. Let coopEnforcementResult be a new opener policy enforcement result with urlurloriginnewDocumentOriginopener policycoop
  - 16. Let navigationParams be a new navigation params, with idnavigationIdnavigabletargetNavigablerequestnull responseresponsefetch controllernullcommit early hintsnullCOOP enforcement resultcoopEnforcementResultreserved environmentnulloriginnewDocumentOriginpolicy containerpolicyContainerfinal sandboxing flag setfinalSandboxFlagsiframe element referrer policytargetSnapshotParams's iframe element refer…
  - 17. Return the result of loading an HTML document given navigationParams.

###### [7.4.2.3.3 Fragment navigations](https://html.spec.whatwg.org/#scroll-to-fragid)

Status: reviewed (2026-11-01)
> `FrameLoader::LoadInSameDocument`: navigate fires first (`DispatchNavigateEvent`), then the URL and entry change, popstate and hashchange are fired (see `DIFFERENCES.md`) and the fragment is scrolled to.

- [x] [navigate to a fragment](https://html.spec.whatwg.org/#navigate-fragid) — To navigate to a fragment given a navigable navigable, a URL url, a history handling behavior historyHandling, a user navigation involvement userInvolvement, an Element-or-null sourceElement, a serialized state-or-null navigationAPIState, and a navigation ID navigationId:
  - 1. Let navigation be navigable's active window's navigation API.
  - 2. Let destinationNavigationAPIState be navigable's active session history entry's navigation API state.
  - 3. If navigationAPIState is not null, then set destinationNavigationAPIState to navigationAPIState.
  - 4. Let continue be the result of firing a push/replace/reload navigate event at navigation with navigationType set to historyHandling, isSameDocument set to true, userInvolvement set to userInvolvement, sourceElement set to sourceElement, destinationURL set to url, and navigationAPIState set to destinationNavigationAPIState.
  - 5. If continue is false:
    - 5a. If navigation's ongoing navigate event is not null, then set the ongoing navigation of navigable to navigationId.
    - 5b. Return.
  - 6. Let historyEntry be a new session history entry, with URLurldocument statenavigable's active session history entry's document statenavigation API statedestinationNavigationAPIStatescroll restoration modenavigable's active session history entry's scroll restoration mode
  - 7. Let entryToReplace be navigable's active session history entry if historyHandling is "replace", otherwise null.
  - 8. If entryToReplace is non-null, then set historyEntry's navigation API key to entryToReplace's navigation API key.
  - 9. Let history be navigable's active document's history object.
  - 10. Let scriptHistoryIndex be history's index.
  - 11. Let scriptHistoryLength be history's length.
  - 12. If historyHandling is "push":
    - 12a. Set history's state to null.
    - 12b. Increment scriptHistoryIndex.
    - 12c. Set scriptHistoryLength to scriptHistoryIndex + 1.
  - 13. Set navigable's active document's URL to url.
  - 14. Set navigable's active session history entry to historyEntry.
  - 15. Update document for history step application given navigable's active document, historyEntry, true, scriptHistoryIndex, scriptHistoryLength, and historyHandling.
  - 16. Scroll to the fragment given navigable's active document.
  - 17. Let traversable be navigable's traversable navigable.
  - 18. Append the following session history synchronous navigation steps involving navigable to traversable:
    - 18a. Finalize a same-document navigation given traversable, navigable, historyEntry, entryToReplace, historyHandling, and userInvolvement.
    - 18b. Invoke WebDriver BiDi fragment navigated with navigable and a new WebDriver BiDi navigation status whose id is navigationId, url is url, and status is "complete".

- [x] [finalize a same-document navigation](https://html.spec.whatwg.org/#finalize-a-same-document-navigation) — To finalize a same-document navigation given a traversable navigable traversable, a navigable targetNavigable, a session history entry targetEntry, a session history entry-or-null entryToReplace, a history handling behavior historyHandling, and a user navigation involvement userInvolvement:
  - 1. Assert: this is running on traversable's session history traversal queue.
  - 2. If targetNavigable's active session history entry is not targetEntry, then return.
  - 3. Let targetStep be null.
  - 4. Let targetEntries be the result of getting session history entries for targetNavigable.
  - 5. If entryToReplace is null: Otherwise:
    - 5a. Clear the forward session history of traversable.
    - 5b. Set targetStep to traversable's current session history step + 1.
    - 5c. Set targetEntry's step to targetStep.
    - 5d. Append targetEntry to targetEntries.
    - 5a. Replace entryToReplace with targetEntry in targetEntries.
    - 5b. Set targetEntry's step to entryToReplace's step.
    - 5c. Set targetStep to traversable's current session history step.
  - 6. Apply the push/replace history step targetStep to traversable given historyHandling and userInvolvement.

###### [7.4.2.3.4 Non-fetch schemes and external software](https://html.spec.whatwg.org/#non-fetch-schemes-and-external-software)

Status: reviewed (2026-11-01)
> Not implemented: a URL the loader strategy can't load fails, and isn't handed to other software.
<!-- definitions: non-fetch-scheme-navigation-params non-fetch-scheme-params-id non-fetch-scheme-params-navigable non-fetch-scheme-params-url non-fetch-scheme-params-target-sandbox non-fetch-scheme-params-source-activation non-fetch-scheme-params-initiator-origin non-fetch-scheme-params-nav-timing-type non-fetch-scheme-params-user-involvement -->

- [x] [attempt to create a non-fetch scheme document](https://html.spec.whatwg.org/#attempt-to-create-a-non-fetch-scheme-document) — To attempt to create a non-fetch scheme document, given a non-fetch scheme navigation params navigationParams:
  - 1. Let url be navigationParams's URL.
  - 2. Let navigable be navigationParams's navigable.
  - 3. If url is to be handled using a mechanism that does not affect navigable, e.g., because url's scheme is handled externally:
    - 3a. Hand-off to external software given url, navigable, navigationParams's target snapshot sandboxing flags, navigationParams's source snapshot has transient activation, and navigationParams's initiator origin.
    - 3b. Return null.
  - 4. Handle url by displaying some sort of inline content, e.g., an error message because the specified scheme is not one of the supported protocols, or an inline prompt to allow the user to select a registered handler for the given scheme. Return the result of displaying the inline content given navigable, navigationParams's id, navigationParams's navigation timing type, and navigationParams's user i…

- [x] [hand-off to external software](https://html.spec.whatwg.org/#hand-off-to-external-software) — To hand-off to external software given a URL or response resource, a navigable navigable, a sandboxing flag set sandboxFlags, a boolean hasTransientActivation, and an origin initiatorOrigin, user agents should:
  - 1. If all of the following are true: navigable is not a top-level traversable;; sandboxFlags has its sandboxed custom protocols navigation browsing context flag set; and; sandboxFlags has its sandboxed top-level navigation with user activation browsing context flag set, or hasTransientActivation is false, then return without invoking the external software package.
  - 2. Perform the appropriate handoff of resource while attempting to mitigate the risk that this is an attempt to exploit the target software. For example, user agents could prompt the user to confirm that initiatorOrigin is to be allowed to invoke the external software in question. In particular, if hasTransientActivation is false, then the user agent should not invoke the external software package w…

##### [7.4.2.4 Preventing navigation](https://html.spec.whatwg.org/#preventing-navigation)

Status: reviewed (2026-11-01)
> `FrameLoader::ShouldClose` checks if unloading is canceled for the frame's document, the only one; sandboxing isn't implemented.

- [x] [allowed by sandboxing to navigate](https://html.spec.whatwg.org/#allowed-to-navigate) — A navigable source is allowed by sandboxing to navigate a second navigable target, given a source snapshot params sourceSnapshotParams, if the following steps return true:
  - 1. If source is target, then return true.
  - 2. If source is an ancestor of target, then return true.
  - 3. If target is an ancestor of source:
    - 3a. If target is not a top-level traversable, then return true.
    - 3b. If sourceSnapshotParams's has transient activation is true, and sourceSnapshotParams's sandboxing flags's sandboxed top-level navigation with user activation browsing context flag is set, then return false.
    - 3c. If sourceSnapshotParams's has transient activation is false, and sourceSnapshotParams's sandboxing flags's sandboxed top-level navigation without user activation browsing context flag is set, then return false.
    - 3d. Return true.
  - 4. If target is a top-level traversable:
    - 4a. If source is the one permitted sandboxed navigator of target, then return true.
    - 4b. If sourceSnapshotParams's sandboxing flags's sandboxed navigation browsing context flag is set, then return false.
    - 4c. Return true.
  - 5. If sourceSnapshotParams's sandboxing flags's sandboxed navigation browsing context flag is set, then return false.
  - 6. Return true.

- [x] [check if unloading is canceled](https://html.spec.whatwg.org/#checking-if-unloading-is-canceled) — To check if unloading is canceled for a list of navigables navigablesThatNeedBeforeUnload, given an optional traversable navigable traversable, an optional integer targetStep, and an optional user navigation involvement userInvolvementForNavigateEvent, run these steps. They return "canceled-by-befo…
  - 1. Let documentsToFireBeforeunload be the active document of each item in navigablesThatNeedBeforeUnload.
  - 2. Let unloadPromptShown be false.
  - 3. Let finalStatus be "continue".
  - 4. If traversable was given:
    - 4a. Assert: targetStep and userInvolvementForNavigateEvent were given.
    - 4b. Let targetEntry be the result of getting the target history entry given traversable and targetStep.
    - 4c. If targetEntry is not traversable's current session history entry, and targetEntry's document state's origin is the same as traversable's current session history entry's document state's origin:
      - 4c.i. Let eventsFired be false.
      - 4c.ii. Let needsBeforeunload be true if navigablesThatNeedBeforeUnload contains traversable; otherwise false.
      - 4c.iii. If needsBeforeunload is true, then remove traversable's active document from documentsToFireBeforeunload.
      - 4c.iv. Queue a global task on the navigation and traversal task source given traversable's active window to perform the following steps:
        - 4c.iv.A. If needsBeforeunload is true:
          - 4c.iv.A.1. Let (unloadPromptShownForThisDocument, unloadPromptCanceledByThisDocument) be the result of running the steps to fire beforeunload given traversable's active document and false.
          - 4c.iv.A.2. If unloadPromptShownForThisDocument is true, then set unloadPromptShown to true.
          - 4c.iv.A.3. If unloadPromptCanceledByThisDocument is true, then set finalStatus to "canceled-by-beforeunload".
        - 4c.iv.B. If finalStatus is "canceled-by-beforeunload", then abort these steps.
        - 4c.iv.C. Let navigation be traversable's active window's navigation API.
        - 4c.iv.D. Let navigateEventResult be the result of firing a traverse navigate event at navigation given targetEntry and userInvolvementForNavigateEvent.
        - 4c.iv.E. If navigateEventResult is false, then set finalStatus to "canceled-by-navigate".
        - 4c.iv.F. Set eventsFired to true.
      - 4c.v. Wait until eventsFired is true.
      - 4c.vi. If finalStatus is not "continue", then return finalStatus.
  - 5. Let totalTasks be the size of documentsToFireBeforeunload.
  - 6. Let completedTasks be 0.
  - 7. For each document of documentsToFireBeforeunload, queue a global task on the navigation and traversal task source given document's relevant global object to run the steps:
    - 7a. Let (unloadPromptShownForThisDocument, unloadPromptCanceledByThisDocument) be the result of running the steps to fire beforeunload given document and unloadPromptShown.
    - 7b. If unloadPromptShownForThisDocument is true, then set unloadPromptShown to true.
    - 7c. If unloadPromptCanceledByThisDocument is true, then set finalStatus to "canceled-by-beforeunload".
    - 7d. Increment completedTasks.
  - 8. Wait for completedTasks to be totalTasks.
  - 9. Return finalStatus.

- [x] [steps to fire beforeunload](https://html.spec.whatwg.org/#steps-to-fire-beforeunload) — The steps to fire beforeunload given a Document document and a boolean unloadPromptShown are:
  - 1. Let unloadPromptCanceled be false.
  - 2. Increase the document's unload counter by 1.
  - 3. Increase document's relevant agent's event loop's termination nesting level by 1.
  - 4. Let eventFiringResult be the result of firing an event named beforeunload at document's relevant global object, using BeforeUnloadEvent, with the cancelable attribute initialized to true.
  - 5. Decrease document's relevant agent's event loop's termination nesting level by 1.
  - 6. If all of the following are true: unloadPromptShown is false;; document's active sandboxing flag set does not have its sandboxed modals flag set;; document's relevant global object has sticky activation;; eventFiringResult is false, or the returnValue attribute of event is not the empty string; and; showing an unload prompt is unlikely to be annoying, deceptive, or pointless, then:
    - 6a. Set unloadPromptShown to true.
    - 6b. Let userPromptHandler be the result of WebDriver BiDi user prompt opened with document's relevant global object, "beforeunload", and "".
    - 6c. If userPromptHandler is "dismiss", then set unloadPromptCanceled to true.
    - 6d. If userPromptHandler is "none":
      - 6d.i. Ask the user to confirm that they wish to unload the document, and pause while waiting for the user's response.
      - 6d.ii. If the user did not confirm the page navigation, then set unloadPromptCanceled to true.
    - 6e. Invoke WebDriver BiDi user prompt closed with document's relevant global object, "beforeunload", and true if unloadPromptCanceled is false or false otherwise.
  - 7. Decrease document's unload counter by 1.
  - 8. Return (unloadPromptShown, unloadPromptCanceled).

##### [7.4.2.5 Aborting navigation](https://html.spec.whatwg.org/#aborting-navigation)

Status: reviewed (2026-11-01)
> A new navigation replaces the provisional one (`FrameLoader::StopAllLoaders`, `NavigationScheduler::Cancel`), as upstream; there are no navigation IDs.
<!-- definitions: ongoing-navigation -->

- [x] [set the ongoing navigation](https://html.spec.whatwg.org/#set-the-ongoing-navigation) — To set the ongoing navigation for a navigable navigable to newValue:
  - 1. If navigable's ongoing navigation is equal to newValue, then return.
  - 2. Inform the navigation API about aborting navigation given navigable.
  - 3. Set navigable's ongoing navigation to newValue.

#### [7.4.3 Reloading and traversing](https://html.spec.whatwg.org/#reloading-and-traversing)

Status: reviewed (2026-11-01)
> `NavigationScheduler::ScheduleRefresh` reloads, and `ScheduleHistoryNavigation` traverses by a delta (going to the current entry reloads).
<!-- definitions: reload-navigation-api-state reload-user-involvement reload-api-method-tracker traverse-sourcedocument -->

- [x] [reload](https://html.spec.whatwg.org/#reload) — To reload a navigable navigable given an optional serialized state-or-null navigationAPIState (default null), an optional user navigation involvement userInvolvement (default "none"), and an optional navigation API method tracker-or-null apiMethodTracker (default null):
  - 1. If userInvolvement is not "browser UI":
    - 1a. Let navigation be navigable's active window's navigation API.
    - 1b. Let destinationNavigationAPIState be navigable's active session history entry's navigation API state.
    - 1c. If navigationAPIState is not null, then set destinationNavigationAPIState to navigationAPIState.
    - 1d. Let continue be the result of firing a push/replace/reload navigate event at navigation with navigationType set to "reload", isSameDocument set to false, userInvolvement set to userInvolvement, destinationURL set to navigable's active session history entry's URL, navigationAPIState set to destinationNavigationAPIState, and apiMethodTracker set to apiMethodTracker.
    - 1e. If continue is false, then return.
  - 2. Set navigable's active session history entry's document state's reload pending to true.
  - 3. Let traversable be navigable's traversable navigable.
  - 4. Append the following session history traversal steps to traversable:
    - 4a. Apply the reload history step to traversable given userInvolvement.

- [x] [traverse the history by a delta](https://html.spec.whatwg.org/#traverse-the-history-by-a-delta) — To traverse the history by a delta given a traversable navigable traversable, an integer delta, and an optional Document sourceDocument:
  - 1. Let sourceSnapshotParams and initiatorToCheck be null.
  - 2. Let userInvolvement be "browser UI".
  - 3. If sourceDocument is given:
    - 3a. Set sourceSnapshotParams to the result of snapshotting source snapshot params given sourceDocument.
    - 3b. Set initiatorToCheck to sourceDocument's node navigable.
    - 3c. Set userInvolvement to "none".
  - 4. Append the following session history traversal steps to traversable:
    - 4a. Let allSteps be the result of getting all used history steps for traversable.
    - 4b. Let currentStepIndex be the index of traversable's current session history step within allSteps.
    - 4c. Let targetStepIndex be currentStepIndex plus delta.
    - 4d. If allSteps[targetStepIndex] does not exist, then abort these steps.
    - 4e. Apply the traverse history step allSteps[targetStepIndex] to traversable, given sourceSnapshotParams, initiatorToCheck, and userInvolvement.

#### [7.4.4 Non-fragment synchronous "navigations"](https://html.spec.whatwg.org/#navigate-non-frag-sync)

Status: reviewed (2026-11-01)
> `FrameLoader::UpdateURLAndHistory`, with `HistoryController::PushState` and `ReplaceState`.
<!-- definitions: uhus-serializeddata uhus-historyhandling -->

- [x] [URL and history update steps](https://html.spec.whatwg.org/#url-and-history-update-steps) — The URL and history update steps, given a Document document, a URL newURL, an optional serialized state-or-null serializedData (default null), and an optional history handling behavior historyHandling (default "replace"), are:
  - 1. Let navigable be document's node navigable.
  - 2. Let activeEntry be navigable's active session history entry.
  - 3. Let newEntry be a new session history entry, with URLnewURLserialized stateif serializedData is not null, serializedData; otherwise activeEntry's classic history API statedocument stateactiveEntry's document statescroll restoration modeactiveEntry's scroll restoration modepersisted user stateactiveEntry's persisted user state
  - 4. If document's is initial about:blank is true, then set historyHandling to "replace".
  - 5. Let entryToReplace be activeEntry if historyHandling is "replace", otherwise null.
  - 6. If entryToReplace is non-null, then set newEntry's navigation API key to entryToReplace's navigation API key.
  - 7. If historyHandling is "push":
    - 7a. Increment document's history object's index.
    - 7b. Set document's history object's length to its index + 1.
  - 8. If serializedData is not null, then restore the history object state given document and newEntry.
  - 9. Set the URL given document to newURL.
  - 10. Set document's latest entry to newEntry.
  - 11. Set navigable's active session history entry to newEntry.
  - 12. Update the navigation API entries for a same-document navigation given document's relevant global object's navigation API, newEntry, and historyHandling.
  - 13. Let traversable be navigable's traversable navigable.
  - 14. Append the following session history synchronous navigation steps involving navigable to traversable:
    - 14a. Finalize a same-document navigation given traversable, navigable, newEntry, entryToReplace, historyHandling, and "none".
    - 14b. Invoke WebDriver BiDi history updated with navigable.

#### [7.4.5 Populating a session history entry](https://html.spec.whatwg.org/#populating-a-session-history-entry)

Status: reviewed (2026-11-01)
> `DocumentLoader` loads the document from the loader strategy (or a `data:` URL, or the host's markup); there's no fetching, `srcdoc` documents or non-HTML documents (see `KNOWN-ISSUES.md`).
<!-- definitions: attempt-to-populate-allow-post attempt-to-populate-completion-steps explicitly-supported-xml-mime-type explicitly-supported-json-mime-type -->

- [x] [attempt to populate the history entry's document](https://html.spec.whatwg.org/#attempt-to-populate-the-history-entry's-document) — To attempt to populate the history entry's document for a session history entry entry, given a navigable navigable, a NavigationTimingType navTimingType, a source snapshot params sourceSnapshotParams, a target snapshot params targetSnapshotParams, a user navigation involvement userInvolvement, an o…
  - 1. Assert: this is running in parallel.
  - 2. Assert: if navigationParams is non-null, then navigationParams's response is non-null.
  - 3. Let documentResource be entry's document state's resource.
  - 4. If navigationParams is null:
    - 4a. If documentResource is a string, then set navigationParams to the result of creating navigation params from a srcdoc resource given entry, navigable, targetSnapshotParams, userInvolvement, navigationId, and navTimingType.
    - 4b. Otherwise, if all of the following are true: entry's URL's scheme is a fetch scheme; and; documentResource is null, or allowPOST is true and documentResource's request body is not failure, then set navigationParams to the result of creating navigation params by fetching given entry, navigable, sourceSnapshotParams, targetSnapshotParams, cspNavigationType, userInvolvement, navigationId, and navTim…
    - 4c. Otherwise, if entry's URL's scheme is not a fetch scheme, then set navigationParams to a new non-fetch scheme navigation params, with idnavigationIdnavigablenavigableURLentry's URLtarget snapshot sandboxing flagstargetSnapshotParams's sandboxing flagssource snapshot has transient activationsourceSnapshotParams's has transient activationinitiator originentry's document state's initiator originnavi…
  - 5. Queue a global task on the navigation and traversal task source, given navigable's active window, to run these steps:
    - 5a. If navigable's ongoing navigation no longer equals navigationId, then run completionSteps and abort these steps.
    - 5b. Let saveExtraDocumentState be true.
    - 5c. If navigationParams is a non-fetch scheme navigation params:
      - 5c.i. Set entry's document state's document to the result of running attempt to create a non-fetch scheme document given navigationParams.
      - 5c.ii. Set saveExtraDocumentState to false.
    - 5d. Otherwise, if any of the following are true: navigationParams is null;; the result of should navigation response to navigation request of type in target be blocked by Content Security Policy? given navigationParams's request, navigationParams's response, navigationParams's policy container's CSP list, cspNavigationType, and navigable is "Blocked";; navigationParams's reserved environment is non-n…
      - 5d.i. Set entry's document state's document to the result of creating a document for inline content that doesn't have a DOM, given navigable, null, navTimingType, and userInvolvement. The inline content should indicate to the user the sort of error that occurred.
      - 5d.ii. Make document unsalvageable given entry's document state's document and "navigation-failure".
      - 5d.iii. Set saveExtraDocumentState to false.
      - 5d.iv. If navigationParams is not null:
        - 5d.iv.A. Run the environment discarding steps for navigationParams's reserved environment.
        - 5d.iv.B. Invoke WebDriver BiDi navigation failed with navigable and a new WebDriver BiDi navigation status whose id is navigationId, status is "canceled", and url is navigationParams's response's URL.
    - 5e. Otherwise, if navigationParams's response has a `Content-Disposition` header specifying the attachment disposition type:
      - 5e.i. Let sourceAllowsDownloading be sourceSnapshotParams's allows downloading.
      - 5e.ii. Let targetAllowsDownloading be false if navigationParams's final sandboxing flag set has the sandboxed downloads browsing context flag set; otherwise true.
      - 5e.iii. Let uaAllowsDownloading be true.
      - 5e.iv. Optionally, the user agent may set uaAllowsDownloading to false, if it believes doing so would safeguard the user from a potentially hostile download.
      - 5e.v. If sourceAllowsDownloading, targetAllowsDownloading, and uaAllowsDownloading are true:
        - 5e.v.A. Handle as a download navigationParams's response with navigable and navigationId.
    - 5f. Otherwise, if navigationParams's response's status is not 204 and is not 205, then set entry's document state's document to the result of loading a document given navigationParams, sourceSnapshotParams, and entry's document state's initiator origin.
    - 5g. If entry's document state's document is not null:
      - 5g.i. Set entry's document state's ever populated to true.
      - 5g.ii. If saveExtraDocumentState is true:
        - 5g.ii.A. Let document be entry's document state's document.
        - 5g.ii.B. Set entry's document state's origin to document's origin.
        - 5g.ii.C. If document's URL requires storing the policy container in history:
          - 5g.ii.C.1. Assert: navigationParams is a navigation params (i.e., neither null nor a non-fetch scheme navigation params).
          - 5g.ii.C.2. Set entry's document state's history policy container to navigationParams's policy container.
      - 5g.iii. If entry's document state's request referrer is "client", and navigationParams is a navigation params (i.e., neither null nor a non-fetch scheme navigation params):
        - 5g.iii.A. Assert: navigationParams's request is not null.
        - 5g.iii.B. Set entry's document state's request referrer to navigationParams's request's referrer.
    - 5h. Run completionSteps.

- [x] [create navigation params from a srcdoc resource](https://html.spec.whatwg.org/#create-navigation-params-from-a-srcdoc-resource) — To create navigation params from a srcdoc resource given a session history entry entry, a navigable navigable, a target snapshot params targetSnapshotParams, a user navigation involvement userInvolvement, a navigation ID-or-null navigationId, and a NavigationTimingType navTimingType:
  - 1. Let documentResource be entry's document state's resource.
  - 2. Let response be a new response with URLabout:srcdocheader list« (`Content-Type`, `text/html`) »bodythe UTF-8 encoding of documentResource, as a body
  - 3. Let responseOrigin be the result of determining the origin given response's URL, targetSnapshotParams's sandboxing flags, and entry's document state's origin.
  - 4. Let coop be a new opener policy.
  - 5. Let coopEnforcementResult be a new opener policy enforcement result with urlresponse's URLoriginresponseOriginopener policycoop
  - 6. Let policyContainer be the result of determining navigation params policy container given response's URL, entry's document state's history policy container, null, navigable's container document's policy container, and null.
  - 7. Return a new navigation params, with idnavigationIdnavigablenavigablerequestnullresponseresponsefetch controllernullcommit early hintsnullCOOP enforcement resultcoopEnforcementResultreserved environmentnulloriginresponseOriginpolicy containerpolicyContainerfinal sandboxing flag settargetSnapshotParams's sandboxing flagsiframe element referrer policytargetSnapshotParams's iframe element referrer p…

- [x] [create navigation params by fetching](https://html.spec.whatwg.org/#create-navigation-params-by-fetching) — To create navigation params by fetching given a session history entry entry, a navigable navigable, a source snapshot params sourceSnapshotParams, a target snapshot params targetSnapshotParams, a string cspNavigationType, a user navigation involvement userInvolvement, a navigation ID-or-null naviga…
  - 1. Assert: this is running in parallel.
  - 2. Let documentResource be entry's document state's resource.
  - 3. Let request be a new request, with URLentry's URLclientsourceSnapshotParams's fetch clientdestination"document" credentials mode"include"use-URL-credentials flagsetredirect mode"manual"replaces client idnavigable's active document's relevant settings object's idmode"navigate"referrerentry's document state's request referrerreferrer policyentry's document state's request referrer policypolicy cont…
  - 4. If navigable is a top-level traversable, then set request's top-level navigation initiator origin to entry's document state's initiator origin.
  - 5. If request's client is null:
    - 5a. Set request's origin to a new opaque origin.
    - 5b. Set request's service-workers mode to "all".
    - 5c. Set request's referrer to "no-referrer".
  - 6. If documentResource is a POST resource:
    - 6a. Set request's method to `POST`.
    - 6b. Set request's body to documentResource's request body.
    - 6c. Set `Content-Type` to documentResource's request content-type in request's header list.
  - 7. If entry's document state's reload pending is true, then set request's reload-navigation flag.
  - 8. Otherwise, if entry's document state's ever populated is true, then set request's history-navigation flag.
  - 9. If sourceSnapshotParams's has transient activation is true, then set request's user-activation to true.
  - 10. If navigable's container is non-null:
    - 10a. If the navigable's container has a browsing context scope origin, then set request's origin to that browsing context scope origin.
    - 10b. Set request's destination to navigable's container's local name.
    - 10c. If sourceSnapshotParams's fetch client is navigable's container document's relevant settings object, then set request's initiator type to navigable's container's local name.
  - 11. Let response be null.
  - 12. Let responseOrigin be null.
  - 13. Let fetchController be null.
  - 14. Let coopEnforcementResult be a new opener policy enforcement result, with urlnavigable's active document's URLoriginnavigable's active document's originopener policynavigable's active document's opener policycurrent context is navigation sourcetrue if navigable's active document's origin is same origin with entry's document state's initiator origin otherwise false
  - 15. Let finalSandboxFlags be an empty sandboxing flag set.
  - 16. Let responsePolicyContainer be null.
  - 17. Let responseCOOP be a new opener policy.
  - 18. Let locationURL be null.
  - 19. Let currentURL be request's current URL.
  - 20. Let commitEarlyHints be null.
  - 21. While true:
    - 21a. If request's reserved client is not null and currentURL's origin is not the same as request's reserved client's creation URL's origin:
      - 21a.i. Run the environment discarding steps for request's reserved client.
      - 21a.ii. Set request's reserved client to null.
      - 21a.iii. Set commitEarlyHints to null.
    - 21b. If request's reserved client is null:
      - 21b.i. Let topLevelCreationURL be currentURL.
      - 21b.ii. Let topLevelOrigin be null.
      - 21b.iii. If navigable is not a top-level traversable:
        - 21b.iii.A. Let parentEnvironment be navigable's parent's active document's relevant settings object.
        - 21b.iii.B. Set topLevelCreationURL to parentEnvironment's top-level creation URL.
        - 21b.iii.C. Set topLevelOrigin to parentEnvironment's top-level origin.
      - 21b.iv. Set request's reserved client to a new environment whose id is a unique opaque string, target browsing context is navigable's active browsing context, creation URL is currentURL, top-level creation URL is topLevelCreationURL, and top-level origin is topLevelOrigin.
    - 21c. If the result of should navigation request of type be blocked by Content Security Policy? given request and cspNavigationType is "Blocked", then set response to a network error and break. [CSP]
    - 21d. Set response to null.
    - 21e. If fetchController is null, then set fetchController to the result of fetching request, with processEarlyHintsResponse set to processEarlyHintsResponse as defined below, processResponse set to processResponse as defined below, and useParallelQueue set to true. Let processEarlyHintsResponse be the following algorithm given a response earlyResponse: Let processResponse be the following algorithm gi…
      - 21e.i. If commitEarlyHints is null, then set commitEarlyHints to the result of processing early hint headers given earlyResponse and request's reserved client.
      - 21e.i. Set response to fetchedResponse.
    - 21f. Otherwise, process the next manual redirect for fetchController.
    - 21g. Wait until either response is non-null, or navigable's ongoing navigation changes to no longer equal navigationId. If the latter condition occurs, then abort fetchController, and return. Otherwise, proceed onward.
    - 21h. If request's body is null, then set entry's document state's resource to null.
    - 21i. Set responsePolicyContainer to the result of creating a policy container from a fetch response given response and request's reserved client.
    - 21j. Set finalSandboxFlags to the union of targetSnapshotParams's sandboxing flags and responsePolicyContainer's CSP list's CSP-derived sandboxing flags.
    - 21k. Set responseOrigin to the result of determining the origin given response's URL, finalSandboxFlags, and entry's document state's initiator origin.
    - 21l. If navigable is a top-level traversable:
      - 21l.i. Set responseCOOP to the result of obtaining an opener policy given response and request's reserved client.
      - 21l.ii. Set coopEnforcementResult to the result of enforcing the response's opener policy given navigable's active browsing context, response's URL, responseOrigin, responseCOOP, coopEnforcementResult, and request's referrer.
      - 21l.iii. If finalSandboxFlags is not empty and responseCOOP's value is not "unsafe-none", then set response to an appropriate network error and break.
    - 21m. If all of the following are true: response is not a network error;; navigable is a child navigable;; response's URL's scheme is an HTTP(S) scheme; and; the result of performing a cross-origin resource policy check with navigable's container document's origin, navigable's container document's relevant settings object, request's destination, response, and true is blocked, then set response to a net…
    - 21n. Set locationURL to response's location URL given currentURL's fragment.
    - 21o. If locationURL is failure or null, then break.
    - 21p. Assert: locationURL is a URL.
    - 21q. Set entry's classic history API state to StructuredSerializeForStorage(null).
    - 21r. Let oldDocState be entry's document state.
    - 21s. Set entry's document state to a new document state, with history policy containera clone of the oldDocState's history policy container if it is non-null; null otherwiserequest referreroldDocState's request referrerrequest referrer policyoldDocState's request referrer policyinitiator originoldDocState's initiator originoriginoldDocState's originabout base URLoldDocState's about base URLresourceold…
    - 21t. If locationURL's scheme is not an HTTP(S) scheme:
      - 21t.i. Set entry's document state's resource to null.
      - 21t.ii. Break.
    - 21u. Set currentURL to locationURL.
    - 21v. Set entry's URL to currentURL.
  - 22. If locationURL is a URL whose scheme is not a fetch scheme, then return a new non-fetch scheme navigation params, with idnavigationIdnavigablenavigableURLlocationURLtarget snapshot sandboxing flagstargetSnapshotParams's sandboxing flagssource snapshot has transient activationsourceSnapshotParams's has transient activationinitiator originresponseOriginnavigation timing typenavTimingTypeuser involv…
  - 23. If any of the following are true: response is a network error;; locationURL is failure; or; locationURL is a URL whose scheme is a fetch scheme, then return null.
  - 24. Assert: locationURL is null and response is not a network error.
  - 25. Let resultPolicyContainer be the result of determining navigation params policy container given response's URL, entry's document state's history policy container, sourceSnapshotParams's source policy container, null, and responsePolicyContainer.
  - 26. If navigable's container is an iframe, and response's timing allow passed flag is set, then set navigable's container's pending resource-timing start time to null.
  - 27. Return a new navigation params, with idnavigationIdnavigablenavigablerequestrequestresponseresponsefetch controllerfetchControllercommit early hintscommitEarlyHintsopener policyresponseCOOPreserved environmentrequest's reserved clientoriginresponseOriginpolicy containerresultPolicyContainerfinal sandboxing flag setfinalSandboxFlagsCOOP enforcement resultcoopEnforcementResultnavigation timing type…

- [x] [browsing context scope origin](https://html.spec.whatwg.org/#browsing-context-scope-origin) — An element has a browsing context scope origin if its Document's node navigable is a top-level traversable or if all of its Document's ancestor navigables have active documents whose origins are the same origin as the element's node document's origin. If an element has a browsing context scope origin, then its value is the origin of the element's node document.

- [x] [load a document](https://html.spec.whatwg.org/#loading-a-document) — To load a document given navigation params navigationParams, source snapshot params sourceSnapshotParams, and origin initiatorOrigin, perform the following steps. They return a Document or null.
  - 1. Let type be the computed type of navigationParams's response.
  - 2. If the user agent has been configured to process resources of the given type using some mechanism other than rendering the content in a navigable, then skip this step. Otherwise, if the type is one of the following types: an HTML MIME typeReturn the result of loading an HTML document, given navigationParams.an XML MIME type that is not an explicitly supported XML MIME typeReturn the result of loa…
  - 3. If, given type, the new resource is to be handled by displaying some sort of inline content, e.g., a native rendering of the content or an error message because the specified type is not supported, then return the result of creating a document for inline content that doesn't have a DOM given navigationParams's navigable, navigationParams's id, navigationParams's navigation timing type, and naviga…
  - 4. Otherwise, the document's type is such that the resource will not affect navigationParams's navigable, e.g., because the resource is to be handed to an external application or because it is an unknown type that will be processed by handle as a download. Hand-off to external software given navigationParams's response, navigationParams's navigable, navigationParams's final sandboxing flag set, sour…
  - 5. Return null.

#### [7.4.6 Applying the history step](https://html.spec.whatwg.org/#applying-the-history-step)

Status: reviewed (2026-11-01)
> The sections below.

##### [7.4.6.1 Updating the traversable](https://html.spec.whatwg.org/#updating-the-traversable)

Status: reviewed (2026-11-01)
> Follows upstream's `FrameLoader` and `HistoryController` rather than these steps (see `DIFFERENCES.md`). There's no `pageswap` (see `KNOWN-ISSUES.md`).
<!-- definitions: changing-navigable-continuation-state changing-nav-continuation-displayed-document changing-nav-continuation-target-entry changing-nav-continuation-navigable changing-nav-continuation-update-only -->

- [x] [update for navigable creation/destruction](https://html.spec.whatwg.org/#update-for-navigable-creation/destruction) — To update for navigable creation/destruction given a traversable navigable traversable:
  - 1. Let step be traversable's current session history step.
  - 2. Return the result of applying the history step step to traversable given false, null, null, "none", and null.

- [x] [apply the push/replace history step](https://html.spec.whatwg.org/#apply-the-push/replace-history-step) — To apply the push/replace history step given a non-negative integer step to a traversable navigable traversable, given a history handling behavior historyHandling and a user navigation involvement userInvolvement:
  - 1. Return the result of applying the history step step to traversable given false, null, null, userInvolvement, and historyHandling.

- [x] [apply the reload history step](https://html.spec.whatwg.org/#apply-the-reload-history-step) — To apply the reload history step to a traversable navigable traversable given user navigation involvement userInvolvement:
  - 1. Let step be traversable's current session history step.
  - 2. Return the result of applying the history step step to traversable given true, null, null, userInvolvement, and "reload".

- [x] [apply the traverse history step](https://html.spec.whatwg.org/#apply-the-traverse-history-step) — To apply the traverse history step given a non-negative integer step to a traversable navigable traversable, with source snapshot params sourceSnapshotParams, navigable initiatorToCheck, and user navigation involvement userInvolvement:
  - 1. Return the result of applying the history step step to traversable given true, sourceSnapshotParams, initiatorToCheck, userInvolvement, and "traverse".

- [x] [resume applying the traverse history step](https://html.spec.whatwg.org/#resume-applying-the-traverse-history-step) — To resume applying the traverse history step given a non-negative integer step, a traversable navigable traversable, and user navigation involvement userInvolvement, apply step to traversable given false, null, null, userInvolvement, and "traverse".

- [x] [apply the history step](https://html.spec.whatwg.org/#apply-the-history-step) — To apply the history step given a non-negative integer step to a traversable navigable traversable, with boolean checkForCancelation, source snapshot params-or-null sourceSnapshotParams, navigable-or-null initiatorToCheck, user navigation involvement userInvolvement, and NavigationType-or-null navi…
  - 1. Assert: This is running within traversable's session history traversal queue.
  - 2. Let targetStep be the result of getting the used step given traversable and step.
  - 3. If initiatorToCheck is not null:
    - 3a. Assert: sourceSnapshotParams is not null.
    - 3b. For each navigable of get all navigables whose current session history entry will change or reload: if initiatorToCheck is not allowed by sandboxing to navigate navigable given sourceSnapshotParams, then return "initiator-disallowed".
  - 4. Let navigablesCrossingDocuments be the result of getting all navigables that might experience a cross-document traversal given traversable and targetStep.
  - 5. If checkForCancelation is true, and the result of checking if unloading is canceled given navigablesCrossingDocuments, traversable, targetStep, and userInvolvement is not "continue", then return that result.
  - 6. Let changingNavigables be the result of get all navigables whose current session history entry will change or reload given traversable and targetStep.
  - 7. Let nonchangingNavigablesThatStillNeedUpdates be the result of getting all navigables that only need history object length/index update given traversable and targetStep.
  - 8. For each navigable of changingNavigables:
    - 8a. Let targetEntry be the result of getting the target history entry given navigable and targetStep.
    - 8b. Set navigable's current session history entry to targetEntry.
    - 8c. If targetEntry's document is not navigable's active document, then queue a global task on the navigation and traversal task source of navigable's active window to run these steps:
      - 8c.i. Let navigation be navigable's active window's navigation API.
      - 8c.ii. Set navigation's ongoing navigate event to null.
      - 8c.iii. Set navigation's ongoing API method tracker to null.
    - 8d. Set navigable's ongoing navigation to "traversal".
  - 9. Let totalChangeJobs be the size of changingNavigables.
  - 10. Let completedChangeJobs be 0.
  - 11. Let changingNavigableContinuations be an empty queue of changing navigable continuation states.
  - 12. For each navigable of changingNavigables, queue a global task on the navigation and traversal task source of navigable's active window to run the steps:
    - 12a. Let displayedEntry be navigable's active session history entry.
    - 12b. Let targetEntry be navigable's current session history entry.
    - 12c. Let changingNavigableContinuation be a changing navigable continuation state with: displayed documentdisplayedEntry's documenttarget entrytargetEntrynavigablenavigableupdate-onlyfalse
    - 12d. If displayedEntry is targetEntry and targetEntry's document state's reload pending is false:
      - 12d.i. Set changingNavigableContinuation's update-only to true.
      - 12d.ii. Enqueue changingNavigableContinuation on changingNavigableContinuations.
      - 12d.iii. Abort these steps.
    - 12e. Switch on navigationType: "reload"Assert: targetEntry's document state's reload pending is true."traverse"Assert: targetEntry's document state's ever populated is true."replace"Assert: targetEntry's step is displayedEntry's step and targetEntry's document state's ever populated is false."push"Assert: targetEntry's step is displayedEntry's step + 1 and targetEntry's document state's ever populated…
    - 12f. Let oldOrigin be targetEntry's document state's origin.
    - 12g. If all of the following are true: navigationType is "traverse";; navigable is not traversable;; targetEntry is not displayedEntry; and; oldOrigin is the same as displayedEntry's document state's origin, then:
      - 12g.i. Let navigation be navigable's active window's navigation API.
      - 12g.ii. Fire a traverse navigate event at navigation given targetEntry and userInvolvement.
    - 12h. If targetEntry's document is null, or targetEntry's document state's reload pending is true: Otherwise, run afterDocumentPopulated immediately. In both cases, let afterDocumentPopulated be the following steps:
      - 12h.i. Let navTimingType be "back_forward" if targetEntry's document is null; otherwise "reload".
      - 12h.ii. Let targetSnapshotParams be the result of snapshotting target snapshot params given navigable.
      - 12h.iii. Let potentiallyTargetSpecificSourceSnapshotParams be sourceSnapshotParams.
      - 12h.iv. If potentiallyTargetSpecificSourceSnapshotParams is null, then set it to the result of snapshotting source snapshot params given navigable's active document.
      - 12h.v. Set targetEntry's document state's reload pending to false.
      - 12h.vi. Let allowPOST be targetEntry's document state's reload pending.
      - 12h.vii. In parallel, attempt to populate the history entry's document for targetEntry, given navigable, potentiallyTargetSpecificSourceSnapshotParams, targetSnapshotParams, userInvolvement, with allowPOST set to allowPOST and completionSteps set to queue a global task on the navigation and traversal task source given navigable's active window to run afterDocumentPopulated.
      - 12h.i. If targetEntry's document is null, then set changingNavigableContinuation's update-only to true.
      - 12h.ii. If targetEntry's document's origin is not same origin with oldOrigin, then set targetEntry's classic history API state to StructuredSerializeForStorage(null).
      - 12h.iii. If all of the following are true: navigable's parent is null;; targetEntry's document's browsing context is not an auxiliary browsing context whose opener browsing context is non-null; and; targetEntry's document's origin is not same origin with oldOrigin, then set targetEntry's document state's navigable target name to the empty string.
      - 12h.iv. Enqueue changingNavigableContinuation on changingNavigableContinuations.
  - 13. Let navigablesThatMustWaitBeforeHandlingSyncNavigation be an empty set.
  - 14. While completedChangeJobs does not equal totalChangeJobs:
    - 14a. If traversable's running nested apply history step is false:
      - 14a.i. While traversable's session history traversal queue's algorithm set contains one or more synchronous navigation steps with a target navigable not contained in navigablesThatMustWaitBeforeHandlingSyncNavigation:
        - 14a.i.A. Let steps be the first item in traversable's session history traversal queue's algorithm set that is synchronous navigation steps with a target navigable not contained in navigablesThatMustWaitBeforeHandlingSyncNavigation.
        - 14a.i.B. Remove steps from traversable's session history traversal queue's algorithm set.
        - 14a.i.C. Set traversable's running nested apply history step to true.
        - 14a.i.D. Run steps.
        - 14a.i.E. Set traversable's running nested apply history step to false.
    - 14b. Let changingNavigableContinuation be the result of dequeuing from changingNavigableContinuations.
    - 14c. If changingNavigableContinuation is nothing, then continue.
    - 14d. Let displayedDocument be changingNavigableContinuation's displayed document.
    - 14e. Let targetEntry be changingNavigableContinuation's target entry.
    - 14f. Let navigable be changingNavigableContinuation's navigable.
    - 14g. Let (scriptHistoryLength, scriptHistoryIndex) be the result of getting the history object length and index given traversable and targetStep.
    - 14h. Append navigable to navigablesThatMustWaitBeforeHandlingSyncNavigation.
    - 14i. Let entriesForNavigationAPI be the result of getting session history entries for the navigation API given navigable and targetStep.
    - 14j. If changingNavigableContinuation's update-only is true, or targetEntry's document is displayedDocument:
      - 14j.i. Set navigable's ongoing navigation to null.
      - 14j.ii. Queue a global task on the navigation and traversal task source given navigable's active window to perform afterPotentialUnloads.
    - 14k. Otherwise:
      - 14k.i. Assert: navigationType is not null.
      - 14k.ii. Deactivate displayedDocument, given userInvolvement, targetEntry, navigationType, and afterPotentialUnloads.
    - 14l. In both cases, let afterPotentialUnloads be the following steps:
      - 14l.i. Let previousEntry be navigable's active session history entry.
      - 14l.ii. If changingNavigableContinuation's update-only is false, then activate history entry targetEntry for navigable.
      - 14l.iii. Let updateDocument be an algorithm step which performs update document for history step application given targetEntry's document, targetEntry, changingNavigableContinuation's update-only, scriptHistoryLength, scriptHistoryIndex, navigationType, entriesForNavigationAPI, and previousEntry.
      - 14l.iv. If targetEntry's document is equal to displayedDocument, then perform updateDocument.
      - 14l.v. Otherwise, queue a global task on the navigation and traversal task source given targetEntry's document's relevant global object to perform updateDocument.
      - 14l.vi. Increment completedChangeJobs.
  - 15. Let totalNonchangingJobs be the size of nonchangingNavigablesThatStillNeedUpdates.
  - 16. Let completedNonchangingJobs be 0.
  - 17. Let (scriptHistoryLength, scriptHistoryIndex) be the result of getting the history object length and index given traversable and targetStep.
  - 18. For each navigable of nonchangingNavigablesThatStillNeedUpdates, queue a global task on the navigation and traversal task source given navigable's active window to run the steps:
    - 18a. Let document be navigable's active document.
    - 18b. Set document's history object's index to scriptHistoryIndex.
    - 18c. Set document's history object's length to scriptHistoryLength.
    - 18d. Increment completedNonchangingJobs.
  - 19. Wait for completedNonchangingJobs to equal totalNonchangingJobs.
  - 20. Set traversable's current session history step to targetStep.
  - 21. Return "applied".

- [x] [deactivate a document for a cross-document navigation](https://html.spec.whatwg.org/#deactivate-a-document-for-a-cross-document-navigation) — To deactivate a document for a cross-document navigation given a Document displayedDocument, a user navigation involvement userNavigationInvolvement, a session history entry targetEntry, a NavigationType navigationType, and afterPotentialUnloads, which is an algorithm that receives no arguments:
  - 1. Let navigable be displayedDocument's node navigable.
  - 2. Let potentiallyTriggerViewTransition be false.
  - 3. Let isBrowserUINavigation be true if userNavigationInvolvement is "browser UI"; otherwise false.
  - 4. Set potentiallyTriggerViewTransition to the result of calling can navigation trigger a cross-document view-transition? given displayedDocument, targetEntry's document, navigationType, and isBrowserUINavigation.
  - 5. If potentiallyTriggerViewTransition is false:
    - 5a. Let firePageSwapBeforeUnload be the following step:
      - 5a.i. Fire the pageswap event given displayedDocument, targetEntry, navigationType, and null.
    - 5b. Set navigable's ongoing navigation to null.
    - 5c. Unload a document and its descendants given displayedDocument, targetEntry's document, afterPotentialUnloads, and firePageSwapBeforeUnload.
  - 6. Otherwise, queue a global task on the navigation and traversal task source given navigable's active window to run the steps:
    - 6a. Let proceedWithNavigationAfterViewTransitionCapture be the following step:
      - 6a.i. Append the following session history traversal steps to navigable's traversable navigable:
        - 6a.i.A. Set navigable's ongoing navigation to null.
        - 6a.i.B. Unload a document and its descendants given displayedDocument, targetEntry's document, and afterPotentialUnloads.
    - 6b. Let viewTransition be the result of setting up a cross-document view-transition given displayedDocument, targetEntry's document, navigationType, and proceedWithNavigationAfterViewTransitionCapture.
    - 6c. Fire the pageswap event given displayedDocument, targetEntry, navigationType, and viewTransition.
    - 6d. If viewTransition is null, then run proceedWithNavigationAfterViewTransitionCapture.

- [x] [fire the pageswap event](https://html.spec.whatwg.org/#fire-the-pageswap-event) — To fire the pageswap event given a Document displayedDocument, a session history entry targetEntry, a NavigationType navigationType, and a ViewTransition-or-null viewTransition:
  - 1. Assert: this is running as part of a task queued on displayedDocument's relevant agent's event loop.
  - 2. Let navigation be displayedDocument's relevant global object's navigation API.
  - 3. Let activation be null.
  - 4. If all of the following are true: targetEntry's document's origin is same origin with displayedDocument's origin; and; targetEntry's document's was created via cross-origin redirects is false, or targetEntry's document's latest entry is not null, then:
    - 4a. Let destinationEntry be determined by switching on navigationType: "reload"The current entry of navigation"traverse"The NavigationHistoryEntry in navigation's entry list whose session history entry is targetEntry"push""replace"A new NavigationHistoryEntry in displayedDocument's relevant realm with its session history entry set to targetEntry
    - 4b. Set activation to a new NavigationActivation created in displayedDocument's relevant realm, with old entrythe current entry of navigationnew entrydestinationEntrynavigation typenavigationType
  - 5. Fire an event named pageswap at displayedDocument's relevant global object, using PageSwapEvent with its activation set to activation, and its viewTransition set to viewTransition.

- [x] [activate history entry](https://html.spec.whatwg.org/#activate-history-entry) — To activate history entry session history entry entry for navigable navigable:
  - 1. Save persisted state to the navigable's active session history entry.
  - 2. Let newDocument be entry's document.
  - 3. Assert: newDocument's is initial about:blank is false, i.e., we never traverse back to the initial about:blank Document because it always gets replaced when we navigate away from it.
  - 4. Set navigable's active session history entry to entry.
  - 5. Make active newDocument.
  - 6. Set the initial visibility state of newDocument to navigable's traversable navigable's system visibility state.

- [x] [get the used step](https://html.spec.whatwg.org/#getting-the-used-step) — To get the used step given a traversable navigable traversable, and a non-negative integer step, perform the following steps. They return a non-negative integer.
  - 1. Let steps be the result of getting all used history steps within traversable.
  - 2. Return the greatest item in steps that is less than or equal to step.

- [x] [get the history object length and index](https://html.spec.whatwg.org/#getting-the-history-object-length-and-index) — To get the history object length and index given a traversable navigable traversable, and a non-negative integer step, perform the following steps. They return a tuple of two non-negative integers.
  - 1. Let steps be the result of getting all used history steps within traversable.
  - 2. Let scriptHistoryLength be the size of steps.
  - 3. Assert: steps contains step.
  - 4. Let scriptHistoryIndex be the index of step in steps.
  - 5. Return (scriptHistoryLength, scriptHistoryIndex).

- [x] [get all navigables whose current session history entry will change or reload](https://html.spec.whatwg.org/#get-all-navigables-whose-current-session-history-entry-will-change-or-reload) — To get all navigables whose current session history entry will change or reload given a traversable navigable traversable, and a non-negative integer targetStep, perform the following steps. They return a list of navigables.
  - 1. Let results be an empty list.
  - 2. Let navigablesToCheck be « traversable ».
  - 3. For each navigable of navigablesToCheck:
    - 3a. Let targetEntry be the result of getting the target history entry given navigable and targetStep.
    - 3b. If targetEntry is not navigable's current session history entry or targetEntry's document state's reload pending is true, then append navigable to results.
    - 3c. If targetEntry's document is navigable's active document, and targetEntry's document state's reload pending is false, then extend navigablesToCheck with the child navigables of navigable.
  - 4. Return results.

- [x] [get all navigables that only need history object length/index update](https://html.spec.whatwg.org/#getting-all-navigables-that-only-need-history-object-length/index-update) — To get all navigables that only need history object length/index update given a traversable navigable traversable, and a non-negative integer targetStep, perform the following steps. They return a list of navigables.
  - 1. Let results be an empty list.
  - 2. Let navigablesToCheck be « traversable ».
  - 3. For each navigable of navigablesToCheck:
    - 3a. Let targetEntry be the result of getting the target history entry given navigable and targetStep.
    - 3b. If targetEntry is navigable's current session history entry and targetEntry's document state's reload pending is false:
      - 3b.i. Append navigable to results.
      - 3b.ii. Extend navigablesToCheck with navigable's child navigables.
  - 4. Return results.

- [x] [get the target history entry](https://html.spec.whatwg.org/#getting-the-target-history-entry) — To get the target history entry given a navigable navigable, and a non-negative integer step, perform the following steps. They return a session history entry.
  - 1. Let entries be the result of getting session history entries for navigable.
  - 2. Return the item in entries that has the greatest step less than or equal to step.

- [x] [get all navigables that might experience a cross-document traversal](https://html.spec.whatwg.org/#getting-all-navigables-that-might-experience-a-cross-document-traversal) — To get all navigables that might experience a cross-document traversal given a traversable navigable traversable, and a non-negative integer targetStep, perform the following steps. They return a list of navigables.
  - 1. Let results be an empty list.
  - 2. Let navigablesToCheck be « traversable ».
  - 3. For each navigable of navigablesToCheck:
    - 3a. Let targetEntry be the result of getting the target history entry given navigable and targetStep.
    - 3b. If targetEntry's document is not navigable's active document or targetEntry's document state's reload pending is true, then append navigable to results.
    - 3c. Otherwise, extend navigablesToCheck with navigable's child navigables.
  - 4. Return results.

##### [7.4.6.2 Updating the document](https://html.spec.whatwg.org/#updating-the-document)

Status: reviewed (2026-11-01)
> `Document::StatePopped` restores the history object's state and fires popstate; `EnqueueHashchangeEvent` fires hashchange; the back/forward cache reactivates documents. There are no not restored reasons (see `KNOWN-ISSUES.md`).
<!-- definitions: scripts-may-run-for-the-newly-created-document -->

- [x] [update document for history step application](https://html.spec.whatwg.org/#update-document-for-history-step-application) — To update document for history step application given a Document document, a session history entry entry, a boolean doNotReactivate, integers scriptHistoryLength and scriptHistoryIndex, NavigationType-or-null navigationType, an optional list of session history entries entriesForNavigationAPI, and a…
  - 1. Let documentIsNew be true if document's latest entry is null; otherwise false.
  - 2. Let documentsEntryChanged be true if document's latest entry is not entry; otherwise false.
  - 3. Set document's history object's index to scriptHistoryIndex.
  - 4. Set document's history object's length to scriptHistoryLength.
  - 5. Let navigation be history's relevant global object's navigation API.
  - 6. If documentsEntryChanged is true:
    - 6a. Let oldURL be document's latest entry's URL.
    - 6b. Set document's latest entry to entry.
    - 6c. Restore the history object state given document and entry.
    - 6d. If documentIsNew is false:
      - 6d.i. Assert: navigationType is not null.
      - 6d.ii. Update the navigation API entries for a same-document navigation given navigation, entry, and navigationType.
      - 6d.iii. Fire an event named popstate at document's relevant global object, using PopStateEvent, with the state attribute initialized to document's history object's state and hasUAVisualTransition initialized to true if a visual transition, to display a cached rendered state of the latest entry, was done by the user agent.
      - 6d.iv. Restore persisted state given entry.
      - 6d.v. If oldURL's fragment is not equal to entry's URL's fragment, then queue a global task on the DOM manipulation task source given document's relevant global object to fire an event named hashchange at document's relevant global object, using HashChangeEvent, with the oldURL attribute initialized to the serialization of oldURL and the newURL attribute initialized to the serialization of entry's URL.
    - 6e. Otherwise:
      - 6e.i. Assert: entriesForNavigationAPI is given.
      - 6e.ii. Restore persisted state given entry.
      - 6e.iii. Initialize the navigation API entries for a new document given navigation, entriesForNavigationAPI, and entry.
  - 7. If all the following are true: previousEntryForActivation is given;; navigationType is non-null; and; navigationType is "reload" or previousEntryForActivation's document is not document, then:
    - 7a. If navigation's activation is null, then set navigation's activation to a new NavigationActivation object in navigation's relevant realm.
    - 7b. Let previousEntryIndex be the result of getting the navigation API entry index of previousEntryForActivation within navigation.
    - 7c. If previousEntryIndex is non-negative, then set activation's old entry to navigation's entry list[previousEntryIndex].
    - 7d. Otherwise, if all the following are true: navigationType is "replace";; previousEntryForActivation's document state's origin is same origin with document's origin; and; previousEntryForActivation's document's is initial about:blank is false, then set activation's old entry to a new NavigationHistoryEntry in navigation's relevant realm, whose session history entry is previousEntryForActivation.
    - 7e. Set activation's new entry to navigation's current entry.
    - 7f. Set activation's navigation type to navigationType.
  - 8. If documentIsNew is true:
    - 8a. Assert: document's during-loading navigation ID for WebDriver BiDi is not null.
    - 8b. Invoke WebDriver BiDi navigation committed with navigable and a new WebDriver BiDi navigation status whose id is document's during-loading navigation ID for WebDriver BiDi, status is "committed", and url is document's URL.
    - 8c. Try to scroll to the fragment for document.
    - 8d. At this point scripts may run for the newly-created document document.
  - 9. Otherwise, if documentsEntryChanged is false and doNotReactivate is false:
    - 9a. Assert: entriesForNavigationAPI is given.
    - 9b. Reactivate document given entry and entriesForNavigationAPI.

- [x] [restore the history object state](https://html.spec.whatwg.org/#restore-the-history-object-state) — To restore the history object state given Document document and session history entry entry:
  - 1. Let targetRealm be document's relevant realm.
  - 2. Let state be StructuredDeserialize(entry's classic history API state, targetRealm). If this throws an exception, catch it and let state be null.
  - 3. Set document's history object's state to state.

- [x] [make active](https://html.spec.whatwg.org/#make-active) — To make active a Document document:
  - 1. Let window be document's relevant global object.
  - 2. Set document's browsing context's active document to document.
  - 3. Set document's browsing context's WindowProxy's [[Window]] internal slot value to window.
  - 4. Set window's relevant settings object's execution ready flag.

- [x] [reactivate (Document)](https://html.spec.whatwg.org/#reactivate-a-document) — To reactivate a Document document given a session history entry reactivatedEntry and a list of session history entries entriesForNavigationAPI:
  - 1. For each formControl of form controls in document with an autofill field name of "off", invoke the reset algorithm for formControl.
  - 2. If document's suspended timer handles is not empty:
    - 2a. Assert: document's suspension time is not zero.
    - 2b. Let suspendDuration be the current high resolution time minus document's suspension time.
    - 2c. Let activeTimers be document's relevant global object's map of active timers.
    - 2d. For each handle in document's suspended timer handles, if activeTimers[handle] exists, then increase activeTimers[handle] by suspendDuration.
  - 3. Update the navigation API entries for reactivation given document's relevant global object's navigation API, entriesForNavigationAPI, and reactivatedEntry.
  - 4. If document's current document readiness is "complete", and document's page showing is false:
    - 4a. Set document's page showing to true.
    - 4b. Set document's has been revealed to false.
    - 4c. Update the visibility state of document to "visible".
    - 4d. Fire a page transition event named pageshow at document's relevant global object with true.

- [x] [try to scroll to the fragment](https://html.spec.whatwg.org/#try-to-scroll-to-the-fragment) — To try to scroll to the fragment for a Document document, perform the following steps in parallel:
  - 1. Wait for an implementation-defined amount of time. (This is intended to allow the user agent to optimize the user experience in the face of performance concerns.)
  - 2. Queue a global task on the navigation and traversal task source given document's relevant global object to run these steps:
    - 2a. If document has no parser, or its parser has stopped parsing, or the user agent has reason to believe the user is no longer interested in scrolling to the fragment, then abort these steps.
    - 2b. Scroll to the fragment given document.
    - 2c. If document's indicated part is still null, then try to scroll to the fragment for document.

- [x] [make document unsalvageable](https://html.spec.whatwg.org/#make-document-unsalvageable) — To make document unsalvageable, given a Document document and a string reason:
  - 1. Let details be a new not restored reason details whose reason is reason.
  - 2. Append details to document's bfcache blocking details.
  - 3. Set document's salvageable state to false.

- [x] [build not restored reasons for document state](https://html.spec.whatwg.org/#build-not-restored-reasons-for-document-state) — To build not restored reasons for document state given Document document:
  - 1. Let notRestoredReasonsForDocument be a new not restored reasons.
  - 2. Set notRestoredReasonsForDocument's URL to document's URL.
  - 3. Let container be document's node navigable's container.
  - 4. If container is an iframe element:
    - 4a. Let src be the empty string.
    - 4b. If container has a src attribute:
      - 4b.i. Let src be the result of encoding-parsing-and-serializing a URL given container's src attribute's value, relative to container's node document.
      - 4b.ii. If src is failure, then set src to container's src attribute's value.
    - 4c. Set notRestoredReasonsForDocument's src to src.
    - 4d. Set notRestoredReasonsForDocument's id to container's id attribute's value, or the empty string if it has no such attribute.
    - 4e. Set notRestoredReasonsForDocument's name to container's name attribute's value, or the empty string if it has no such attribute.
  - 5. Set notRestoredReasonsForDocument's reasons to a clone of document's bfcache blocking details.
  - 6. For each navigable of document's document-tree child navigables:
    - 6a. Let childDocument be navigable's active document.
    - 6b. Build not restored reasons for document state given childDocument.
    - 6c. Append childDocument's not restored reasons to notRestoredReasonsForDocument's children.
  - 7. Set document's node navigable's active session history entry's document state's not restored reasons to notRestoredReasonsForDocument.

- [x] [build not restored reasons for a top-level traversable and its descendants](https://html.spec.whatwg.org/#build-not-restored-reasons-for-a-top-level-traversable-and-its-descendants) — To build not restored reasons for a top-level traversable and its descendants given top-level traversable topLevelTraversable:
  - 1. Build not restored reasons for document state given topLevelTraversable's active document.
  - 2. Let crossOriginDescendants be an empty list.
  - 3. For each childNavigable of topLevelTraversable's active document's descendant navigables:
    - 3a. If childNavigable's active document's origin is not same origin with topLevelTraversable's active document's origin, then append childNavigable to crossOriginDescendants.
  - 4. Let crossOriginDescendantsPreventsBfcache be false.
  - 5. For each crossOriginNavigable of crossOriginDescendants:
    - 5a. Let reasonsForCrossOriginChild be crossOriginNavigable's active document's document state's not restored reasons.
    - 5b. If reasonsForCrossOriginChild's reasons is not empty, set crossOriginDescendantsPreventsBfcache to true.
    - 5c. Set reasonsForCrossOriginChild's URL to null.
    - 5d. Set reasonsForCrossOriginChild's reasons to null.
    - 5e. Set reasonsForCrossOriginChild's children to null.
  - 6. If crossOriginDescendantsPreventsBfcache is true, make document unsalvageable given topLevelTraversable's active document and "masked".

##### [7.4.6.3 Revealing the document](https://html.spec.whatwg.org/#revealing-the-document)

Status: reviewed (2026-11-01)
> There's no `pagereveal` (see `KNOWN-ISSUES.md`).
<!-- definitions: has-been-revealed -->

- [x] [reveal](https://html.spec.whatwg.org/#reveal) — To reveal a Document document:
  - 1. If document's has been revealed is true, then return.
  - 2. Set document's has been revealed to true.
  - 3. Let transition be the result of resolving inbound cross-document view-transition for document.
  - 4. Fire an event named pagereveal at document's relevant global object, using PageRevealEvent with its viewTransition set to transition.
  - 5. If transition is not null:
    - 5a. Prepare to run script given document's relevant settings object.
    - 5b. Activate transition.
    - 5c. Clean up after running script given document's relevant settings object.

##### [7.4.6.4 Scrolling to a fragment](https://html.spec.whatwg.org/#scrolling-to-a-fragment)

Status: reviewed (2026-11-01)
> `LocalFrameView::ScrollToFragment` and `ScrollToAnchorFragment`, with `Document::FindAnchor` (which matches an `a` element's `name` ASCII case-insensitively in quirks mode; see `DIFFERENCES.md`). The scroll position is kept at the target while layout changes.
<!-- definitions: target-element top-of-the-document -->

- [x] [scroll to the fragment](https://html.spec.whatwg.org/#scroll-to-the-fragment-identifier) — To scroll to the fragment given a Document document:
  - 1. If document's indicated part is null, then set document's target element to null.
  - 2. Otherwise, if document's indicated part is top of the document:
    - 2a. Set document's target element to null.
    - 2b. Scroll to the beginning of the document for document. [CSSOMVIEW]
    - 2c. Return.
  - 3. Otherwise:
    - 3a. Assert: document's indicated part is an element.
    - 3b. Let target be document's indicated part.
    - 3c. Set document's target element to target.
    - 3d. Run the ancestor revealing algorithm on target.
    - 3e. Scroll target into view, with behavior set to "auto", block set to "start", and inline set to "nearest". [CSSOMVIEW]
    - 3f. Run the focusing steps for target, with the Document's viewport as the fallback target.
    - 3g. Move the sequential focus navigation starting point to target.

- [x] [indicated part](https://html.spec.whatwg.org/#the-indicated-part-of-the-document) — A Document's indicated part is the one that its URL's fragment identifies, or null if the fragment does not identify anything. The semantics of the fragment in terms of mapping it to a node is defined by the specification that defines the MIME type used by the Document (for example, the processing of fragments for XML MIME types is the responsibility of RFC7303). [RFC7303]

- [x] [select the indicated part](https://html.spec.whatwg.org/#select-the-indicated-part) — To select the indicated part given a Document document and a URL url:
  - 1. If document's URL does not equal url with exclude fragments set to true, then return null.
  - 2. Let fragment be url's fragment.
  - 3. If fragment is the empty string, then return the special value top of the document.
  - 4. Let potentialIndicatedElement be the result of finding a potential indicated element given document and fragment.
  - 5. If potentialIndicatedElement is not null, then return potentialIndicatedElement.
  - 6. Let fragmentBytes be the result of percent-decoding fragment.
  - 7. Let decodedFragment be the result of running UTF-8 decode without BOM on fragmentBytes.
  - 8. Set potentialIndicatedElement to the result of finding a potential indicated element given document and decodedFragment.
  - 9. If potentialIndicatedElement is not null, then return potentialIndicatedElement.
  - 10. If decodedFragment is an ASCII case-insensitive match for the string top, then return the top of the document.
  - 11. Return null.

- [x] [find a potential indicated element](https://html.spec.whatwg.org/#find-a-potential-indicated-element) — To find a potential indicated element given a Document document and a string fragment, run these steps:
  - 1. If there is an element in the document tree whose root is document and that has an ID equal to fragment, then return the first such element in tree order.
  - 2. If there is an a element in the document tree whose root is document that has a name attribute whose value is equal to fragment, then return the first such element in tree order.
  - 3. Return null.

##### [7.4.6.5 Persisted history entry state](https://html.spec.whatwg.org/#persisted-user-state-restoration)

Status: reviewed (2026-11-01)
> `HistoryController::SaveScrollPositionAndViewStateToItem` and `RestoreScrollPositionAndViewState`; the scroll restoration mode "manual" leaves the position alone. Form control state is restored by `FormController`.
<!-- definitions: has-been-scrolled-by-the-user -->

- [x] [save persisted state](https://html.spec.whatwg.org/#save-persisted-state) — To save persisted state to a session history entry entry:
  - 1. Set the scroll position data of entry to contain the scroll positions for all of entry's document's restorable scrollable regions.
  - 2. Optionally, update entry's persisted user state to reflect any state that the user agent wishes to persist, such as the values of form fields.

- [x] [restore persisted state](https://html.spec.whatwg.org/#restore-persisted-state) — To restore persisted state from a session history entry entry:
  - 1. If entry's scroll restoration mode is "auto", and entry's document's relevant global object's navigation API's suppress normal scroll restoration during ongoing navigation is false, then restore scroll position data given entry.
  - 2. Optionally, update other aspects of entry's document and its rendering, for instance values of form fields, that the user agent had previously recorded in entry's persisted user state.

- [x] [restorable scrollable regions](https://html.spec.whatwg.org/#restorable-scrollable-regions) — The restorable scrollable regions of a Document document are document's viewport, and all of document's scrollable regions excepting any navigable containers.

- [x] [restore scroll position data](https://html.spec.whatwg.org/#restore-scroll-position-data) — To restore scroll position data given a session history entry entry:
  - 1. Let document be entry's document.
  - 2. If document's has been scrolled by the user is true, then the user agent should return.
  - 3. The user agent should attempt to use entry's scroll position data to restore the scroll positions of entry's document's restorable scrollable regions. The user agent may continue to attempt to do so periodically, until document's has been scrolled by the user becomes true.
