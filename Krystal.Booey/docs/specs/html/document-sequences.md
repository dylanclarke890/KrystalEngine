# HTML review checklist: 7.3 Infrastructure for sequences of documents

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 47 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html document-sequences` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [7.3 Infrastructure for sequences of documents](https://html.spec.whatwg.org/#infrastructure-for-sequences-of-documents)

Status: reviewed (2026-11-01)
> The sections below.

#### [7.3.1 Navigables](https://html.spec.whatwg.org/#navigables)

Status: reviewed (2026-11-01)
> A `LocalFrame` is the navigable: its document is the active document (`Document::Frame` is the node navigable), its window the active window and `SpecifiedName` the target name. Session history entries are `HistoryItem`s, kept by the page's `BackForwardController`, rather than the navigable's.
<!-- definitions: navigable nav-id nav-parent nav-current-history-entry nav-active-history-entry is-closing delaying-load-events-mode allowed-to-perform-a-navigation-or-history-update -->

- [x] [active document (navigable)](https://html.spec.whatwg.org/#nav-document) — A navigable's active document is its active session history entry's document.

- [x] [active browsing context (navigable)](https://html.spec.whatwg.org/#nav-bc) — A navigable's active browsing context is its active document's browsing context. If this navigable is a traversable navigable, then its active browsing context will be a top-level browsing context.

- [x] [active WindowProxy](https://html.spec.whatwg.org/#nav-wp) — A navigable's active WindowProxy is its active browsing context's associated WindowProxy.

- [x] [active window (navigable)](https://html.spec.whatwg.org/#nav-window) — A navigable's active window is its active WindowProxy's [[Window]].

- [x] [target name](https://html.spec.whatwg.org/#nav-target) — A navigable's target name is its active session history entry's document state's navigable target name.

- [x] [node navigable](https://html.spec.whatwg.org/#node-navigable) — To get the node navigable of a node node, return the navigable whose active document is node's node document, or null if there is no such navigable.

- [x] [initialize the navigable](https://html.spec.whatwg.org/#initialize-the-navigable) — To initialize the navigable navigable navigable, given a document state documentState and an optional navigable-or-null parent (default null):
  - 1. Assert: documentState's document is non-null.
  - 2. Let entry be a new session history entry, with URLdocumentState's document's URLdocument statedocumentState
  - 3. Set navigable's current session history entry to entry.
  - 4. Set navigable's active session history entry to entry.
  - 5. Set navigable's parent to parent.
  - 6. Set the initial visibility state of documentState's document to navigable's traversable navigable's system visibility state.

##### [7.3.1.1 Traversable navigables](https://html.spec.whatwg.org/#traversable-navigables)

Status: reviewed (2026-11-01)
> The `Page` is the traversable navigable, with its main frame its only navigable; its session history is the back/forward list, and there's no session history traversal queue (traversals are `NavigationScheduler` tasks).
<!-- definitions: traversable-navigable tn-current-session-history-step tn-session-history-entries tn-session-history-traversal-queue tn-running-nested-apply-history-step system-visibility-state is-created-by-web-content -->

- [x] [traversable navigable (navigable)](https://html.spec.whatwg.org/#nav-traversable) — To get the traversable navigable of a navigable inputNavigable:
  - 1. Let navigable be inputNavigable.
  - 2. While navigable is not a traversable navigable, set navigable to navigable's parent.
  - 3. Return navigable.

##### [7.3.1.2 Top-level traversables](https://html.spec.whatwg.org/#top-level-traversables)

Status: reviewed (2026-11-01)
> The host creates each top-level traversable as a `Page`; `FrameLoader::Init` gives its frame the initial about:blank document. There's no set of them, as pages don't know of each other.
<!-- definitions: top-level-traversable-set -->

- [x] [top-level traversable](https://html.spec.whatwg.org/#top-level-traversable) — A top-level traversable is a traversable navigable with a null parent.

- [x] [top-level traversable (navigable)](https://html.spec.whatwg.org/#nav-top) — To get the top-level traversable of a navigable inputNavigable:
  - 1. Let navigable be inputNavigable.
  - 2. While navigable's parent is not null, set navigable to navigable's parent.
  - 3. Return navigable.

- [x] [create a new top-level traversable](https://html.spec.whatwg.org/#creating-a-new-top-level-traversable) — To create a new top-level traversable given a browsing context-or-null opener, a string targetName, and an optional navigable openerNavigableForWebDriver:
  - 1. Let document be null.
  - 2. If opener is null, then set document to the second return value of creating a new top-level browsing context and document.
  - 3. Otherwise, set document to the second return value of creating a new auxiliary browsing context and document given opener.
  - 4. Let documentState be a new document state, with documentdocumentinitiator originnull if opener is null; otherwise, document's originorigindocument's originnavigable target nametargetNameabout base URLdocument's about base URL
  - 5. Let traversable be a new traversable navigable.
  - 6. Initialize the navigable traversable given documentState.
  - 7. Let initialHistoryEntry be traversable's active session history entry.
  - 8. Set initialHistoryEntry's step to 0.
  - 9. Append initialHistoryEntry to traversable's session history entries.
  - 10. If opener is non-null, then legacy-clone a traversable storage shed given opener's top-level traversable and traversable. [STORAGE]
  - 11. Append traversable to the user agent's top-level traversable set.
  - 12. Invoke WebDriver BiDi navigable created with traversable and openerNavigableForWebDriver.
  - 13. Return traversable.

- [x] [create a fresh top-level traversable](https://html.spec.whatwg.org/#create-a-fresh-top-level-traversable) — To create a fresh top-level traversable given a URL initialNavigationURL and an optional POST resource-or-null initialNavigationPostResource (default null):
  - 1. Let traversable be the result of creating a new top-level traversable given null and the empty string.
  - 2. Navigate traversable to initialNavigationURL using traversable's active document, with documentResource set to initialNavigationPostResource.
  - 3. Return traversable.

##### [7.3.1.3 Child navigables](https://html.spec.whatwg.org/#child-navigables)

Status: n/a (no frame tree)
<!-- definitions: navigable-container content-navigable -->

- [x] [container (navigable)](https://html.spec.whatwg.org/#nav-container) — The container of a navigable navigable is the navigable container whose content navigable is navigable, or null if there is no such element.

- [x] [container document (navigable)](https://html.spec.whatwg.org/#nav-container-document) — The container document of a navigable navigable is the result of running these steps:
  - 1. If navigable's container is null, then return null.
  - 2. Return navigable's container's node document.

- [x] [container document](https://html.spec.whatwg.org/#doc-container-document) — The container document of a Document document is the result of running these steps:
  - 1. If document's node navigable is null, then return null.
  - 2. Return document's node navigable's container document.

- [x] [child navigable](https://html.spec.whatwg.org/#child-navigable) — A navigable navigable is a child navigable of another navigable potentialParent when navigable's parent is potentialParent. We can also just say that a navigable "is a child navigable", which means that its parent is non-null.

- [x] [content document](https://html.spec.whatwg.org/#concept-bcc-content-document) — The content document of a navigable container container is the result of running these steps:
  - 1. If container's content navigable is null, then return null.
  - 2. Let document be container's content navigable's active document.
  - 3. If document's origin and container's node document's origin are not same origin-domain, then return null.
  - 4. Return document.

- [x] [content window](https://html.spec.whatwg.org/#content-window) — The content window of a navigable container container is the result of running these steps:
  - 1. If container's content navigable is null, then return null.
  - 2. Return container's content navigable's active WindowProxy's object.

- [x] [create a new child navigable](https://html.spec.whatwg.org/#create-a-new-child-navigable) — To create a new child navigable, given an element element:
  - 1. Let parentNavigable be element's node navigable.
  - 2. Let group be element's node document's browsing context's top-level browsing context's group.
  - 3. Let browsingContext and document be the result of creating a new browsing context and document given element's node document, element, and group.
  - 4. Let targetName be null.
  - 5. If element has a name content attribute, then set targetName to the value of that attribute.
  - 6. Let documentState be a new document state, with documentdocumentinitiator origindocument's originorigindocument's originnavigable target nametargetNameabout base URLdocument's about base URL
  - 7. Let navigable be a new navigable.
  - 8. Initialize the navigable navigable given documentState and parentNavigable.
  - 9. Set element's content navigable to navigable.
  - 10. Let historyEntry be navigable's active session history entry.
  - 11. Let traversable be parentNavigable's traversable navigable.
  - 12. Append the following session history traversal steps to traversable:
    - 12a. Let parentDocState be parentNavigable's active session history entry's document state.
    - 12b. Let parentNavigableEntries be the result of getting session history entries for parentNavigable.
    - 12c. Let targetStepSHE be the first session history entry in parentNavigableEntries whose document state equals parentDocState.
    - 12d. Set historyEntry's step to targetStepSHE's step.
    - 12e. Let nestedHistory be a new nested history whose id is navigable's id and entries list is « historyEntry ».
    - 12f. Append nestedHistory to parentDocState's nested histories.
    - 12g. Update for navigable creation/destruction given traversable.
  - 13. Invoke WebDriver BiDi navigable created with traversable.

##### [7.3.1.4 Jake diagrams](https://html.spec.whatwg.org/#jake-diagrams)

Status: reviewed (2026-11-01)
> No requirements.
<!-- definitions: jake-diagram -->

##### [7.3.1.5 Related navigable collections](https://html.spec.whatwg.org/#related-navigable-collections)

Status: n/a (no frame tree)

- [x] [ancestor navigables (Document)](https://html.spec.whatwg.org/#ancestor-navigables) — The ancestor navigables of a Document document are given by these steps:
  - 1. Let navigable be document's node navigable's parent.
  - 2. Let ancestors be an empty list.
  - 3. While navigable is not null:
    - 3a. Prepend navigable to ancestors.
    - 3b. Set navigable to navigable's parent.
  - 4. Return ancestors.

- [x] [inclusive ancestor navigables (Document)](https://html.spec.whatwg.org/#inclusive-ancestor-navigables) — The inclusive ancestor navigables of a Document document are given by these steps:
  - 1. Let navigables be document's ancestor navigables.
  - 2. Append document's node navigable to navigables.
  - 3. Return navigables.

- [x] [descendant navigables (Document)](https://html.spec.whatwg.org/#descendant-navigables) — The descendant navigables of a Document document are given by these steps:
  - 1. Let navigables be new list.
  - 2. Let navigableContainers be a list of all shadow-including descendants of document that are navigable containers, in shadow-including tree order.
  - 3. For each navigableContainer of navigableContainers:
    - 3a. If navigableContainer's content navigable is null, then continue.
    - 3b. Extend navigables with navigableContainer's content navigable's active document's inclusive descendant navigables.
  - 4. Return navigables.

- [x] [inclusive descendant navigables (Document)](https://html.spec.whatwg.org/#inclusive-descendant-navigables) — The inclusive descendant navigables of a Document document are given by these steps:
  - 1. Let navigables be « document's node navigable ».
  - 2. Extend navigables with document's descendant navigables.
  - 3. Return navigables.

- [x] [document-tree child navigables](https://html.spec.whatwg.org/#document-tree-child-navigables) — The document-tree child navigables of a Document document are given by these steps:
  - 1. If document's node navigable is null, then return the empty list.
  - 2. Let navigables be new list.
  - 3. Let navigableContainers be a list of all descendants of document that are navigable containers, in tree order.
  - 4. For each navigableContainer of navigableContainers:
    - 4a. If navigableContainer's content navigable is null, then continue.
    - 4b. Append navigableContainer's content navigable to navigables.
  - 5. Return navigables.

##### [7.3.1.6 Navigable destruction](https://html.spec.whatwg.org/#garbage-collection-and-browsing-contexts)

Status: reviewed (2026-11-01)
> `window.close()` asks the page to stay (beforeunload) before setting is closing, then the host closes the window in a task; destroying the page destroys its documents. Child navigables are n/a, as there's no frame tree.

- [x] [destroy a child navigable](https://html.spec.whatwg.org/#destroy-a-child-navigable) — To destroy a child navigable given a navigable container container:
  - 1. Let navigable be container's content navigable.
  - 2. If navigable is null, then return.
  - 3. Set container's content navigable to null.
  - 4. Inform the navigation API about child navigable destruction given navigable.
  - 5. Destroy a document and its descendants given navigable's active document.
  - 6. Let parentDocState be container's node navigable's active session history entry's document state.
  - 7. Remove the nested history from parentDocState's nested histories whose id equals navigable's id.
  - 8. Let traversable be container's node navigable's traversable navigable.
  - 9. Append the following session history traversal steps to traversable:
    - 9a. Update for navigable creation/destruction given traversable.
  - 10. Invoke WebDriver BiDi navigable destroyed with navigable.

- [x] [destroy](https://html.spec.whatwg.org/#destroy-a-top-level-traversable) — To destroy a top-level traversable traversable:
  - 1. Let browsingContext be traversable's active browsing context.
  - 2. For each historyEntry in traversable's session history entries :
    - 2a. Let document be historyEntry's document.
    - 2b. If document is not null, then destroy a document and its descendants given document.
  - 3. Remove browsingContext.
  - 4. Remove traversable from the user interface (e.g., close or hide its tab in a tabbed browser).
  - 5. Remove traversable from the user agent's top-level traversable set.
  - 6. Invoke WebDriver BiDi navigable destroyed with traversable.

- [x] [close (top-level traversable)](https://html.spec.whatwg.org/#close-a-top-level-traversable) — To close a top-level traversable traversable:
  - 1. If traversable's is closing is true, then return.
  - 2. Definitely close traversable.

- [x] [definitely close](https://html.spec.whatwg.org/#definitely-close-a-top-level-traversable) — To definitely close a top-level traversable traversable:
  - 1. Let toUnload be traversable's active document's inclusive descendant navigables.
  - 2. If the result of checking if unloading is canceled for toUnload is not "continue", then return.
  - 3. Append the following session history traversal steps to traversable:
    - 3a. Let afterAllUnloads be an algorithm step which destroys traversable.
    - 3b. Unload a document and its descendants given traversable's active document, null, and afterAllUnloads.

##### [7.3.1.7 Navigable target names](https://html.spec.whatwg.org/#navigable-target-names)

Status: reviewed (2026-11-01)
> Fixed: asking the host for a new window consumes the window's transient activation (`FrameLoader::CheckNewWindowPolicy`, step 8a).
> `FrameLoader::FindFrameForNavigation`: the frame is the only navigable to find by target name. Valid target names are an authoring requirement.

- [x] [valid navigable target name](https://html.spec.whatwg.org/#valid-navigable-target-name) — A valid navigable target name is any string with at least one character that does not contain both an ASCII tab or newline and a U+003C (<), and it does not start with a U+005F (_). (Names starting with a U+005F (_) are reserved for special keywords.)

- [x] [valid navigable target name or keyword](https://html.spec.whatwg.org/#valid-navigable-target-name-or-keyword) — A valid navigable target name or keyword is any string that is either a valid navigable target name or that is an ASCII case-insensitive match for one of: _blank, _self, _parent, or _top.

- [x] [find a navigable by target name](https://html.spec.whatwg.org/#find-a-navigable-by-target-name) — To find a navigable by target name given a string name and a navigable currentNavigable:
  - 1. Let currentDocument be currentNavigable's active document.
  - 2. Let sourceSnapshotParams be the result of snapshotting source snapshot params given currentDocument.
  - 3. Let subtreesToSearch be an implementation-defined choice of one of the following: « currentNavigable's traversable navigable, currentNavigable »; the inclusive ancestor navigables of currentDocument
  - 4. For each subtreeToSearch of subtreesToSearch, in reverse order:
    - 4a. Let documentToSearch be subtreeToSearch's active document.
    - 4b. For each navigable of the inclusive descendant navigables of documentToSearch:
      - 4b.i. If currentNavigable is not allowed by sandboxing to navigate navigable given sourceSnapshotParams, then optionally continue.
      - 4b.ii. If navigable's target name is name, then return navigable.
  - 5. Let currentTopLevelBrowsingContext be currentNavigable's active browsing context's top-level browsing context.
  - 6. Let group be currentTopLevelBrowsingContext's group.
  - 7. For each topLevelBrowsingContext of group's browsing context set, in an implementation-defined order (the user agent should pick a consistent ordering, such as the most recently opened, most recently focused, or more closely related):
    - 7a. If currentTopLevelBrowsingContext is topLevelBrowsingContext, then continue.
    - 7b. Let documentToSearch be topLevelBrowsingContext's active document.
    - 7c. For each navigable of the inclusive descendant navigables of documentToSearch:
      - 7c.i. If currentNavigable's active browsing context is not familiar with navigable's active browsing context, then continue.
      - 7c.ii. If currentNavigable is not allowed by sandboxing to navigate navigable given sourceSnapshotParams, then optionally continue.
      - 7c.iii. If navigable's target name is name, then return navigable.
  - 8. Return null.

- [x] [The rules for choosing a navigable](https://html.spec.whatwg.org/#the-rules-for-choosing-a-navigable) — The rules for choosing a navigable, given a string name, a navigable currentNavigable, and a boolean noopener are as follows:
  - 1. Let chosen be null.
  - 2. Let windowType be "existing or none".
  - 3. Let sandboxingFlagSet be currentNavigable's active document's active sandboxing flag set.
  - 4. If name is the empty string or an ASCII case-insensitive match for "_self", then set chosen to currentNavigable.
  - 5. Otherwise, if name is an ASCII case-insensitive match for "_parent", set chosen to currentNavigable's parent, if any, and currentNavigable otherwise.
  - 6. Otherwise, if name is an ASCII case-insensitive match for "_top", set chosen to currentNavigable's traversable navigable.
  - 7. Otherwise, if name is not an ASCII case-insensitive match for "_blank", then set chosen to the result of finding a navigable by target name given name and currentNavigable.
  - 8. If chosen is null, then a new top-level traversable is being requested, and what happens depends on the user agent's configuration and abilities — it is determined by the rules given for the first applicable option from the following list: If currentNavigable's active window does not have transient activation and the user agent has been configured to not show popups (i.e., the user agent has a "p…
    - 8a. Consume user activation of currentNavigable's active window.
    - 8b. Set windowType to "new and unrestricted".
    - 8c. Let currentDocument be currentNavigable's active document.
    - 8d. If currentDocument's opener policy's value is "same-origin" or "same-origin-plus-COEP", and currentDocument's origin is not same origin with currentDocument's relevant settings object's top-level origin:
      - 8d.i. Set noopener to true.
      - 8d.ii. Set name to "_blank".
      - 8d.iii. Set windowType to "new with no opener".
    - 8e. Let targetName be the empty string.
    - 8f. If name is not an ASCII case-insensitive match for "_blank", then set targetName to name.
    - 8g. If noopener is true, then set chosen to the result of creating a new top-level traversable given null, targetName, and currentNavigable.
    - 8h. Otherwise:
      - 8h.i. Set chosen to the result of creating a new top-level traversable given currentNavigable's active browsing context, targetName, and currentNavigable.
      - 8h.ii. If sandboxingFlagSet's sandboxed navigation browsing context flag is set, then set chosen's active browsing context's one permitted sandboxed navigator to currentNavigable's active browsing context.
    - 8i. If sandboxingFlagSet's sandbox propagates to auxiliary browsing contexts flag is set, then all the flags that are set in sandboxingFlagSet must be set in chosen's active browsing context's popup sandboxing flag set.
    - 8j. Set chosen's is created by web content to true.
  - 9. Return chosen and windowType.

#### [7.3.2 Browsing contexts](https://html.spec.whatwg.org/#windows)

Status: reviewed (2026-11-01)
> There are no browsing contexts apart from navigables: a frame's window and document are its browsing context's, and there are no auxiliary browsing contexts, openers or popups (see `DIFFERENCES.md`).
<!-- definitions: browsing-context active-document opener-browsing-context opener-origin-at-creation is-popup is-auxiliary browsing-context-initial-url virtual-browsing-context-group-id concept-document-bc -->

- [x] [active window](https://html.spec.whatwg.org/#active-window) — A browsing context's active window is its WindowProxy object's [[Window]] internal slot value.

- [x] [top-level traversable (browsing context)](https://html.spec.whatwg.org/#bc-traversable) — A browsing context's top-level traversable is its active document's node navigable's top-level traversable.

- [x] [auxiliary browsing context](https://html.spec.whatwg.org/#auxiliary-browsing-context) — A browsing context whose is auxiliary is true is known as an auxiliary browsing context. Auxiliary browsing contexts are always top-level browsing contexts.

##### [7.3.2.1 Creating browsing contexts](https://html.spec.whatwg.org/#creating-browsing-contexts)

Status: reviewed (2026-11-01)
> Fixed: the initial about:blank document is completely loaded at once, its readiness "complete" (it had stayed "interactive").
> `FrameLoader::Init` and `DocumentLoader::CommitInitialEmptyDocument` make the frame's initial about:blank document: an HTML document in quirks mode with `html`, `head` and `body`. Origins, sandboxing and policy containers are absent (see `KNOWN-ISSUES.md`).

- [x] [create a new browsing context and document](https://html.spec.whatwg.org/#creating-a-new-browsing-context) — To create a new browsing context and document, given null or a Document object creator, null or an element embedder, and a browsing context group group:
  - 1. Let browsingContext be a new browsing context.
  - 2. Let unsafeContextCreationTime be the unsafe shared current time.
  - 3. Let creatorOrigin be null.
  - 4. Let creatorBaseURL be null.
  - 5. If creator is non-null:
    - 5a. Set creatorOrigin to creator's origin.
    - 5b. Set creatorBaseURL to creator's document base URL.
    - 5c. Set browsingContext's virtual browsing context group ID to creator's browsing context's top-level browsing context's virtual browsing context group ID.
  - 6. Let sandboxFlags be the result of determining the creation sandboxing flags given browsingContext and embedder.
  - 7. Let origin be the result of determining the origin given about:blank, sandboxFlags, and creatorOrigin.
  - 8. Let permissionsPolicy be the result of creating a permissions policy given embedder and origin. [PERMISSIONSPOLICY]
  - 9. Let agent be the result of obtaining a similar-origin window agent given origin, group, and false.
  - 10. Let realm execution context be the result of creating a new realm given agent and the following customizations: For the global object, create a new Window object.; For the global this binding, use browsingContext's WindowProxy object.
  - 11. Let topLevelCreationURL be the return value of running these steps:
    - 11a. If embedder is non-null, then return embedder's relevant settings object's top-level creation URL.
    - 11b. If creator is non-null, then return creator's relevant settings object's top-level creation URL.
    - 11c. Return about:blank.
  - 12. Let topLevelOrigin be origin if embedder is null; otherwise embedder's relevant settings object's top-level origin.
  - 13. Set up a window environment settings object with about:blank, realm execution context, null, topLevelCreationURL, and topLevelOrigin.
  - 14. Let loadTimingInfo be a new document load timing info with its navigation start time set to the result of calling coarsen time with unsafeContextCreationTime and the new environment settings object's cross-origin isolated capability.
  - 15. Let document be a new Document, with: type"html"content type"text/html"mode"quirks"originoriginbrowsing contextbrowsingContext permissions policypermissionsPolicyactive sandboxing flag setsandboxFlagsload timing infoloadTimingInfois initial about:blanktrueabout base URLcreatorBaseURLallow declarative shadow rootstruecustom element registrya new CustomElementRegistry object
  - 16. Let iframeReferrerPolicy be the result of determining the iframe element referrer policy given embedder.
  - 17. Set document's internal ancestor origin objects list to the result of running the internal ancestor origin objects list creation steps given document and iframeReferrerPolicy.
  - 18. Set document's ancestor origins list to the result of running the ancestor origins list creation steps given document.
  - 19. If creator is non-null:
    - 19a. Set document's referrer to the serialization of creator's URL.
    - 19b. Set document's policy container to a clone of creator's policy container.
    - 19c. If creator's origin is same origin with creator's relevant settings object's top-level origin, then set document's opener policy to creator's browsing context's top-level browsing context's active document's opener policy.
  - 20. Assert: document's URL and document's relevant settings object's creation URL are about:blank.
  - 21. Mark document as ready for post-load tasks.
  - 22. Populate with html/head/body given document.
  - 23. Make active document.
  - 24. Completely finish loading document.
  - 25. Return browsingContext and document.

- [x] [create a new top-level browsing context and document](https://html.spec.whatwg.org/#creating-a-new-top-level-browsing-context) — To create a new top-level browsing context and document:
  - 1. Let group and document be the result of creating a new browsing context group and document.
  - 2. Return group's browsing context set[0] and document.

- [x] [create a new auxiliary browsing context and document](https://html.spec.whatwg.org/#creating-a-new-auxiliary-browsing-context) — To create a new auxiliary browsing context and document, given a browsing context opener:
  - 1. Let openerTopLevelBrowsingContext be opener's top-level traversable's active browsing context.
  - 2. Let group be openerTopLevelBrowsingContext's group.
  - 3. Assert: group is non-null, as navigating invokes this directly.
  - 4. Let browsingContext and document be the result of creating a new browsing context and document with opener's active document, null, and group.
  - 5. Set browsingContext's is auxiliary to true.
  - 6. Append browsingContext to group.
  - 7. Set browsingContext's opener browsing context to opener.
  - 8. Set browsingContext's virtual browsing context group ID to openerTopLevelBrowsingContext's virtual browsing context group ID.
  - 9. Set browsingContext's opener origin at creation to opener's active document's origin.
  - 10. Return browsingContext and document.

- [x] [determine the origin](https://html.spec.whatwg.org/#determining-the-origin) — To determine the origin, given a URL url, a sandboxing flag set sandboxFlags, and an origin-or-null sourceOrigin:
  - 1. If sandboxFlags has its sandboxed origin browsing context flag set, then return a new opaque origin.
  - 2. If url is null, then return a new opaque origin.
  - 3. If url is about:srcdoc:
    - 3a. Assert: sourceOrigin is non-null.
    - 3b. Return sourceOrigin.
  - 4. If url matches about:blank and sourceOrigin is non-null, then return sourceOrigin.
  - 5. Return url's origin.

##### [7.3.2.2 Related browsing contexts](https://html.spec.whatwg.org/#nested-browsing-contexts)

Status: n/a (no frame tree)

- [x] [ancestor](https://html.spec.whatwg.org/#ancestor-browsing-context) — A browsing context potentialDescendant is said to be an ancestor of a browsing context potentialAncestor if the following algorithm returns true:
  - 1. Let potentialDescendantDocument be potentialDescendant's active document.
  - 2. If potentialDescendantDocument is not fully active, then return false.
  - 3. Let ancestorBCs be the list obtained by taking the browsing context of the active document of each member of potentialDescendantDocument's ancestor navigables.
  - 4. If ancestorBCs contains potentialAncestor, then return true.
  - 5. Return false.

- [x] [top-level browsing context](https://html.spec.whatwg.org/#top-level-browsing-context) — A top-level browsing context is a browsing context whose active document's node navigable is a traversable navigable.

- [x] [top-level browsing context](https://html.spec.whatwg.org/#bc-tlbc) — The top-level browsing context of a browsing context start is the result of the following algorithm:
  - 1. If start's active document is not fully active, then return null.
  - 2. Let navigable be start's active document's node navigable.
  - 3. While navigable's parent is not null, set navigable to navigable's parent.
  - 4. Return navigable's active browsing context.

- [x] [familiar with](https://html.spec.whatwg.org/#familiar-with) — A browsing context A is familiar with a second browsing context B if the following algorithm returns true:
  - 1. If A's active document's origin is same origin with B's active document's origin, then return true.
  - 2. If A's top-level browsing context is B, then return true.
  - 3. If B is an auxiliary browsing context and A is familiar with B's opener browsing context, then return true.
  - 4. If there exists an ancestor browsing context of B whose active document has the same origin as the active document of A, then return true.
  - 5. Return false.

##### [7.3.2.3 Groupings of browsing contexts](https://html.spec.whatwg.org/#groupings-of-browsing-contexts)

Status: n/a (no browsing context groups)
<!-- definitions: tlbc-group browsing-context-group-set browsing-context-group browsing-context-set agent-cluster-map historical-agent-cluster-key-map bcg-cross-origin-isolation cross-origin-isolation-mode cross-origin-isolation-none cross-origin-isolation-logical cross-origin-isolation-concrete -->

- [x] [create a new browsing context group and document](https://html.spec.whatwg.org/#creating-a-new-browsing-context-group-and-document) — To create a new browsing context group and document:
  - 1. Let group be a new browsing context group.
  - 2. Append group to the user agent's browsing context group set.
  - 3. Let browsingContext and document be the result of creating a new browsing context and document with null, null, and group.
  - 4. Append browsingContext to group.
  - 5. Return group and document.

- [x] [append](https://html.spec.whatwg.org/#bcg-append) — To append a top-level browsing context browsingContext to a browsing context group group:
  - 1. Append browsingContext to group's browsing context set.
  - 2. Set browsingContext's group to group.

- [x] [remove](https://html.spec.whatwg.org/#bcg-remove) — To remove a top-level browsing context browsingContext:
  - 1. Assert: browsingContext's group is non-null.
  - 2. Let group be browsingContext's group.
  - 3. Set browsingContext's group to null.
  - 4. Remove browsingContext from group's browsing context set.
  - 5. If group's browsing context set is empty, then remove group from the user agent's browsing context group set.

#### [7.3.3 Fully active documents](https://html.spec.whatwg.org/#fully-active-documents)

Status: reviewed (2026-11-01)
> `Document::IsFullyActive`: the frame's active document, as every frame is top-level.

- [x] [fully active (Document)](https://html.spec.whatwg.org/#fully-active) — A Document d is said to be fully active when d is the active document of a navigable navigable, and either navigable is a top-level traversable or navigable's container document is fully active.
