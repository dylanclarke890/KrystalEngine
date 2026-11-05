# HTML review checklist: 7.5 Document lifecycle

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 17 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html document-lifecycle` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [7.5 Document lifecycle](https://html.spec.whatwg.org/#document-lifecycle)

Status: reviewed (2026-11-01)
> The sections below.

#### [7.5.1 Shared document creation infrastructure](https://html.spec.whatwg.org/#shared-document-creation-infrastructure)

Status: reviewed (2026-11-01)
> `DocumentWriter::Begin` creates the document (`DOMImplementation::CreateDocument` for the response's type), shows it in the frame, which gives it a new window, and starts its parser; the initial about:blank document's window isn't reused, as its origin would be opaque. Origins, policies, CSP and timing info are absent (see `KNOWN-ISSUES.md`).

- [x] [create and initialize a Document object](https://html.spec.whatwg.org/#initialise-the-document-object) — When loading a document using one of the below algorithms, we use the following steps to create and initialize a Document object, given a type type, content type contentType, and navigation params navigationParams:
  - 1. Let browsingContext be the result of obtaining a browsing context to use for a navigation response given navigationParams.
  - 2. Let permissionsPolicy be the result of creating a permissions policy from a response given navigationParams's navigable's container, navigationParams's origin, and navigationParams's response. [PERMISSIONSPOLICY]
  - 3. Let creationURL be navigationParams's response's URL.
  - 4. If navigationParams's request is non-null, then set creationURL to navigationParams's request's current URL.
  - 5. Let window be null.
  - 6. If browsingContext's active document's is initial about:blank is true, and browsingContext's active document's origin is same origin-domain with navigationParams's origin, then set window to browsingContext's active window.
  - 7. Otherwise:
    - 7a. Let oacHeader be the result of getting a structured field value given `Origin-Agent-Cluster` and "item" from navigationParams's response's header list.
    - 7b. Let requestsOAC be true if oacHeader is not null and oacHeader[0] is the boolean true; otherwise false.
    - 7c. If navigationParams's reserved environment is a non-secure context, then set requestsOAC to false.
    - 7d. Let agent be the result of obtaining a similar-origin window agent given navigationParams's origin, browsingContext's group, and requestsOAC.
    - 7e. Let realmExecutionContext be the result of creating a new realm given agent and the following customizations: For the global object, create a new Window object.; For the global this binding, use browsingContext's WindowProxy object.
    - 7f. Set window to the global object of realmExecutionContext's Realm component.
    - 7g. Let topLevelCreationURL be creationURL.
    - 7h. Let topLevelOrigin be navigationParams's origin.
    - 7i. If navigationParams's navigable's container is not null:
      - 7i.i. Let parentEnvironment be navigationParams's navigable's container's relevant settings object.
      - 7i.ii. Set topLevelCreationURL to parentEnvironment's top-level creation URL.
      - 7i.iii. Set topLevelOrigin to parentEnvironment's top-level origin.
    - 7j. Set up a window environment settings object with creationURL, realmExecutionContext, navigationParams's reserved environment, topLevelCreationURL, and topLevelOrigin.
  - 8. Let loadTimingInfo be a new document load timing info with its navigation start time set to navigationParams's response's timing info's start time.
  - 9. Let document be a new Document, with typetypecontent typecontentTypeoriginnavigationParams's originbrowsing contextbrowsingContext policy containernavigationParams's policy containerpermissions policypermissionsPolicyactive sandboxing flag setnavigationParams's final sandboxing flag setopener policynavigationParams's cross-origin opener policyload timing infoloadTimingInfowas created via cross-or…
  - 10. Set window's associated Document to document.
  - 11. Set document's internal ancestor origin objects list to the result of running the internal ancestor origin objects list creation steps given document and navigationParams's iframe element referrer policy.
  - 12. Set document's ancestor origins list to the result of running the ancestor origins list creation steps given document.
  - 13. Run CSP initialization for a Document given document. [CSP]
  - 14. If navigationParams's request is non-null:
    - 14a. Set document's referrer to the empty string.
    - 14b. Let referrer be navigationParams's request's referrer.
    - 14c. If referrer is a URL record, then set document's referrer to the serialization of referrer.
  - 15. If navigationParams's fetch controller is not null:
    - 15a. Let fullTimingInfo be the result of extracting the full timing info from navigationParams's fetch controller.
    - 15b. Let redirectCount be 0.
    - 15c. If navigationParams's response's redirect taint is "same-origin", or all of the following are true: navigationParams's request's client is null, or navigationParams's request's referrer is not "no-referrer", and; navigation TAO check given navigationParams's response and navigationParams's origin returns success, then set redirectCount to navigationParams's request's redirect count.
    - 15d. Create the navigation timing entry for document, given fullTimingInfo, redirectCount, navigationTimingType, navigationParams's response's service worker timing info, and navigationParams's response's body info.
  - 16. Create the navigation timing entry for document, with navigationParams's response's timing info, redirectCount, navigationParams's navigation timing type, and navigationParams's response's service worker timing info.
  - 17. If navigationParams's response has a `Refresh` header:
    - 17a. Let value be the isomorphic decoding of the value of the header.
    - 17b. Run the shared declarative refresh steps with document and value.
  - 18. If navigationParams's commit early hints is not null, then call navigationParams's commit early hints with document.
  - 19. Process link headers given document, navigationParams's response, and "pre-media".
  - 20. If navigationParams's navigable is a top-level traversable, then process the `Speculation-Rules` header given document and navigationParams's response.
  - 21. Potentially free deferred fetch quota for document.
  - 22. Return document.

- [x] [populate with html/head/body](https://html.spec.whatwg.org/#populate-with-html/head/body) — To populate with html/head/body given a Document document:
  - 1. Let html be the result of creating an element given document, "html", and the HTML namespace.
  - 2. Let head be the result of creating an element given document, "head", and the HTML namespace.
  - 3. Let body be the result of creating an element given document, "body", and the HTML namespace.
  - 4. Append html to document.
  - 5. Append head to html.
  - 6. Append body to html.

#### [7.5.2 Loading HTML documents](https://html.spec.whatwg.org/#read-html)

Status: reviewed (2026-11-01)
> An about:blank document is parsed from no data, which gives it `html`, `head` and `body` as it commits.

- [x] [load an HTML document](https://html.spec.whatwg.org/#navigate-html) — To load an HTML document, given navigation params navigationParams:
  - 1. Let document be the result of creating and initializing a Document object given "html", "text/html", and navigationParams.
  - 2. If document's URL is about:blank, then populate with html/head/body given document.
  - 3. Otherwise, create an HTML parser whose allow declarative shadow roots is true and associate it with document. Each task that the networking task source places on the task queue while fetching runs must then fill the parser's input byte stream with the fetched bytes and cause the HTML parser to perform the appropriate processing of the input stream. The first task that the networking task source p…
  - 4. Return document.

#### [7.5.3 Loading XML documents](https://html.spec.whatwg.org/#read-xml)

Status: reviewed (2026-11-01)
> `XMLDocumentParser` parses XHTML, SVG and other XML documents.

#### [7.5.4 Loading text documents](https://html.spec.whatwg.org/#read-text)

Status: reviewed (2026-11-01)
> Fixed: a text document (a JavaScript, JSON, `text/css`, `text/plain` or `text/vtt` response) is an HTML document of its type whose parser starts with a `pre` element in the PLAINTEXT state (`DocumentParser::StartPlainText`); it had been parsed as HTML.

- [x] [load a text document](https://html.spec.whatwg.org/#navigate-text) — To load a text document, given a navigation params navigationParams and a string type:
  - 1. Let document be the result of creating and initializing a Document object given "html", type, and navigationParams.
  - 2. Set document's parser cannot change the mode flag to true.
  - 3. Set document's mode to "no-quirks".
  - 4. Create an HTML parser and associate it with the document. Act as if the tokenizer had emitted a start tag token with the tag name "pre" followed by a single U+000A LINE FEED (LF) character, and switch the HTML parser's tokenizer to the PLAINTEXT state. Each task that the networking task source places on the task queue while fetching runs must then fill the parser's input byte stream with the fetc…
  - 5. User agents may add content to the head element of document, e.g., linking to a style sheet, providing script, or giving the document a title.
  - 6. Return document.

#### [7.5.5 Loading multipart/x-mixed-replace documents](https://html.spec.whatwg.org/#read-multipart-x-mixed-replace)

Status: n/a (later: network loader)

- [x] [load a multipart/x-mixed-replace document](https://html.spec.whatwg.org/#navigate-multipart-x-mixed-replace) — To load a multipart/x-mixed-replace document, given navigation params navigationParams, source snapshot params sourceSnapshotParams, and origin initiatorOrigin:
  - 1. Parse navigationParams's response's body using the rules for multipart types. [RFC2046]
  - 2. Let firstPartNavigationParams be a copy of navigationParams.
  - 3. Set firstPartNavigationParams's response to a new response representing the first part of navigationParams's response's body's multipart stream.
  - 4. Let document be the result of loading a document given firstPartNavigationParams, sourceSnapshotParams, and initiatorOrigin. For each additional body part obtained from navigationParams's response, the user agent must navigate document's node navigable to navigationParams's request's URL, using document, with response set to navigationParams's response and historyHandling set to "replace".
  - 5. Return document.

#### [7.5.6 Loading media documents](https://html.spec.whatwg.org/#read-media)

Status: n/a (later: media playback)

- [x] [load a media document](https://html.spec.whatwg.org/#navigate-media) — To load a media document, given navigationParams and a string type:
  - 1. Let document be the result of creating and initializing a Document object given "html", type, and navigationParams.
  - 2. Set document's mode to "no-quirks".
  - 3. Populate with html/head/body given document.
  - 4. Append an element host element for the media, as described below, to the body element.
  - 5. Set the appropriate attribute of the element host element, as described below, to the address of the image, video, or audio resource.
  - 6. User agents may add content to the head element of document, or attributes to host element, e.g., to link to a style sheet, to provide a script, to give the document a title, or to make the media autoplay.
  - 7. Process link headers given document, navigationParams's response, and "media".
  - 8. Act as if the user agent had stopped parsing document.
  - 9. Return document.

#### [7.5.7 Loading a document for inline content that doesn't have a DOM](https://html.spec.whatwg.org/#read-ua-inline)

Status: n/a (no plugins or inline content handlers)

- [x] [create a document to display a user agent page or PDF viewer inline](https://html.spec.whatwg.org/#navigate-ua-inline) — When the user agent is to create a document to display a user agent page or PDF viewer inline, provided a navigable navigable, a navigation ID navigationId, a NavigationTimingType navTimingType, and a user navigation involvement userInvolvement, the user agent should:
  - 1. Let origin be a new opaque origin.
  - 2. Let coop be a new opener policy.
  - 3. Let coopEnforcementResult be a new opener policy enforcement result with urlresponse's URLoriginoriginopener policycoop
  - 4. Let navigationParams be a new navigation params with idnavigationIdnavigablenavigablerequestnullresponsea new responseoriginoriginfetch controllernullcommit early hintsnullCOOP enforcement resultcoopEnforcementResultreserved environmentnullpolicy containera new policy containerfinal sandboxing flag setan empty setiframe element referrer policythe empty stringopener policycoopnavigation timing typ…
  - 5. Let document be the result of creating and initializing a Document object given "html", "text/html", and navigationParams.
  - 6. Either associate document with a custom rendering that is not rendered using the normal Document rendering rules, or mutate document until it represents the content the user agent wants to render.
  - 7. Act as if the user agent had stopped parsing document.
  - 8. Return document.

#### [7.5.8 Finishing the loading process](https://html.spec.whatwg.org/#loading-documents)

Status: reviewed (2026-11-01)
> The document's load timing has `DOMCompleteTime`; there are no containers to fire load at, as there are no child navigables.
<!-- definitions: completely-loaded-time -->

- [x] [completely loaded](https://html.spec.whatwg.org/#completely-loaded) — A Document is considered completely loaded if its completely loaded time is non-null.

- [x] [completely finish loading](https://html.spec.whatwg.org/#completely-finish-loading) — To completely finish loading a Document document:
  - 1. Assert: document's browsing context is non-null.
  - 2. Set document's completely loaded time to the current time.
  - 3. Let container be document's node navigable's container.
  - 4. If container is an iframe element, then queue an element task on the DOM manipulation task source given container to run the iframe load event steps given container.
  - 5. Otherwise, if container is non-null, then queue an element task on the DOM manipulation task source given container to fire an event named load at container.

#### [7.5.9 Unloading documents](https://html.spec.whatwg.org/#unloading-documents)

Status: reviewed (2026-11-01)
> `FrameLoader::DispatchUnloadEvents` and `Document::DispatchPagehideEvent`; the back/forward cache state is the salvageable state. Hiding the document fires `visibilitychange` (`Document::SetVisibilityHiddenDueToDismissal`), and there's no termination nesting level, as there's no script.
<!-- definitions: concept-document-salvageable page-showing suspension-time suspended-timer-handles termination-nesting-level unload-counter -->

- [x] [unload](https://html.spec.whatwg.org/#unload-a-document) — To unload a Document oldDocument, given an optional Document newDocument:
  - 1. Assert: this is running as part of a task queued on oldDocument's relevant agent's event loop.
  - 2. Let unloadTimingInfo be a new document unload timing info.
  - 3. If newDocument is not given, then set unloadTimingInfo to null.
  - 4. Otherwise, if newDocument's event loop is not oldDocument's event loop, then the user agent may be unloading oldDocument in parallel. In that case, the user agent should set unloadTimingInfo to null.
  - 5. Let intendToKeepInBfcache be true if the user agent intends to keep oldDocument alive in a session history entry, such that it can later be used for history traversal. This must be false if oldDocument is not salvageable, or if there are any descendants of oldDocument which the user agent does not intend to keep alive in the same way (including due to their lack of salvageability).
  - 6. Let eventLoop be oldDocument's relevant agent's event loop.
  - 7. Increase eventLoop's termination nesting level by 1.
  - 8. Increase oldDocument's unload counter by 1.
  - 9. If intendToKeepInBfcache is false, then set oldDocument's salvageable state to false.
  - 10. If oldDocument's page showing is true:
    - 10a. Set oldDocument's page showing to false.
    - 10b. Fire a page transition event named pagehide at oldDocument's relevant global object with oldDocument's salvageable state.
    - 10c. Update the visibility state of oldDocument to "hidden".
  - 11. If unloadTimingInfo is not null, then set unloadTimingInfo's unload event start time to the current high resolution time given newDocument's relevant global object, coarsened given oldDocument's relevant settings object's cross-origin isolated capability.
  - 12. If oldDocument's salvageable state is false, then fire an event named unload at oldDocument's relevant global object, with legacy target override flag set.
  - 13. If unloadTimingInfo is not null, then set unloadTimingInfo's unload event end time to the current high resolution time given newDocument's relevant global object, coarsened given oldDocument's relevant settings object's cross-origin isolated capability.
  - 14. Decrease eventLoop's termination nesting level by 1.
  - 15. Set oldDocument's suspension time to the current high resolution time given document's relevant global object.
  - 16. Set oldDocument's suspended timer handles to the result of getting the keys for the map of active timers.
  - 17. Set oldDocument's has been scrolled by the user to false.
  - 18. Run any unloading document cleanup steps for oldDocument that are defined by this specification and other applicable specifications.
  - 19. If oldDocument's node navigable is a top-level traversable, build not restored reasons for a top-level traversable and its descendants given oldDocument's node navigable.
  - 20. If oldDocument's salvageable state is false, then destroy oldDocument.
  - 21. Decrease oldDocument's unload counter by 1.
  - 22. If newDocument is given, newDocument's was created via cross-origin redirects is false, and newDocument's origin is the same as oldDocument's origin, then set newDocument's previous document unload timing to unloadTimingInfo.

- [x] [unload a document and its descendants](https://html.spec.whatwg.org/#unload-a-document-and-its-descendants) — To unload a document and its descendants, given a Document document, an optional Document-or-null newDocument (default null), an optional set of steps afterAllUnloads, and an optional set of steps firePageSwapSteps:
  - 1. Assert: this is running within document's node navigable's traversable navigable's session history traversal queue.
  - 2. Let childNavigables be document's child navigables.
  - 3. Let numberUnloaded be 0.
  - 4. For each childNavigable of childNavigables , queue a global task on the navigation and traversal task source given childNavigable's active window to perform the following steps:
    - 4a. Let incrementUnloaded be an algorithm step which increments numberUnloaded.
    - 4b. Unload a document and its descendants given childNavigable's active document, null, and incrementUnloaded.
  - 5. Wait until numberUnloaded equals childNavigable's size.
  - 6. Queue a global task on the navigation and traversal task source given document's relevant global object to perform the following steps:
    - 6a. If firePageSwapSteps is given, then run firePageSwapSteps.
    - 6b. Unload document, passing along newDocument if it is not null.
    - 6c. If afterAllUnloads was given, then run it.

- [x] [unloading document cleanup steps](https://html.spec.whatwg.org/#unloading-document-cleanup-steps) — This specification defines the following unloading document cleanup steps. Other specifications can define more. Given a Document document:
  - 1. Let window be document's relevant global object.
  - 2. For each WebSocket object webSocket whose relevant global object is window, make disappear webSocket. If this affected any WebSocket objects, then make document unsalvageable given document and "websocket".
  - 3. For each WebTransport object transport whose relevant global object is window, run the context cleanup steps given transport.
  - 4. If document's salvageable state is false:
    - 4a. For each EventSource object eventSource whose relevant global object is equal to window, forcibly close eventSource.
    - 4b. Clear window's map of active timers.

#### [7.5.10 Destroying documents](https://html.spec.whatwg.org/#destroying-documents)

Status: reviewed (2026-11-01)
> A document is destroyed when it's released, rather than by these steps; tasks of a document that isn't fully active still run (see `KNOWN-ISSUES.md`).

- [x] [destroy](https://html.spec.whatwg.org/#destroy-a-document) — To destroy a Document document:
  - 1. Assert: this is running as part of a task queued on document's relevant agent's event loop.
  - 2. Abort document.
  - 3. Set document's salvageable state to false.
  - 4. Run any unloading document cleanup steps for document that are defined by this specification and other applicable specifications.
  - 5. Remove any tasks whose document is document from any task queue (without running those tasks).
  - 6. Set document's browsing context to null.
  - 7. Set document's node navigable's active session history entry's document state's document to null.
  - 8. Remove document from the owner set of each WorkerGlobalScope object whose set contains document.
  - 9. For each workletGlobalScope in document's worklet global scopes, terminate workletGlobalScope.

- [x] [destroy a document and its descendants](https://html.spec.whatwg.org/#destroy-a-document-and-its-descendants) — To destroy a document and its descendants given a Document document and an optional set of steps afterAllDestruction, perform the following steps in parallel:
  - 1. If document is not fully active:
    - 1a. Let reason be a string from user-agent specific blocking reasons. If none apply, then let reason be "masked".
    - 1b. Make document unsalvageable given document and reason.
    - 1c. If document's node navigable is a top-level traversable, build not restored reasons for a top-level traversable and its descendants given document's node navigable.
  - 2. Let childNavigables be document's child navigables.
  - 3. Let numberDestroyed be 0.
  - 4. For each childNavigable of childNavigables , queue a global task on the navigation and traversal task source given childNavigable's active window to perform the following steps:
    - 4a. Let incrementDestroyed be an algorithm step which increments numberDestroyed.
    - 4b. Destroy a document and its descendants given childNavigable's active document and incrementDestroyed.
  - 5. Wait until numberDestroyed equals childNavigable's size.
  - 6. Queue a global task on the navigation and traversal task source given document's relevant global object to perform the following steps:
    - 6a. Destroy document.
    - 6b. If afterAllDestruction was given, then run it.

#### [7.5.11 Aborting a document load](https://html.spec.whatwg.org/#aborting-a-document-load)

Status: reviewed (2026-11-01)
> `FrameLoader::StopAllLoaders` stops a provisional load; `window.stop()` doesn't stop loading (see `KNOWN-ISSUES.md`).

- [x] [abort](https://html.spec.whatwg.org/#abort-a-document) — To abort a Document document:
  - 1. Assert: this is running as part of a task queued on document's relevant agent's event loop.
  - 2. Cancel any instances of the fetch algorithm in the context of document, discarding any tasks queued for them, and discarding any further data received from the network for them. If this resulted in any instances of the fetch algorithm being canceled or any queued tasks or any network data getting discarded, then make document unsalvageable given document and "fetch".
  - 3. If document's during-loading navigation ID for WebDriver BiDi is non-null:
    - 3a. Invoke WebDriver BiDi navigation aborted with document's node navigable and a new WebDriver BiDi navigation status whose id is document's during-loading navigation ID for WebDriver BiDi, status is "canceled", and url is document's URL.
    - 3b. Set document's during-loading navigation ID for WebDriver BiDi to null.
  - 4. If document has an active parser:
    - 4a. Set document's active parser was aborted to true.
    - 4b. Abort that parser.
    - 4c. Make document unsalvageable given document and "parser-aborted".

- [x] [abort a document and its descendants](https://html.spec.whatwg.org/#abort-a-document-and-its-descendants) — To abort a document and its descendants given a Document document:
  - 1. Assert: this is running as part of a task queued on document's relevant agent's event loop.
  - 2. Let descendantNavigables be document's descendant navigables.
  - 3. For each descendantNavigable of descendantNavigables , queue a global task on the navigation and traversal task source given descendantNavigable's active window to perform the following steps:
    - 3a. Abort descendantNavigable's active document.
    - 3b. If descendantNavigable's active document's salvageable is false, then set document's salvageable to false.
  - 4. Abort document.

- [x] [stop loading](https://html.spec.whatwg.org/#nav-stop) — To stop loading a navigable navigable:
  - 1. Let document be navigable's active document.
  - 2. If document's unload counter is 0, and navigable's ongoing navigation is a navigation ID, then set the ongoing navigation for navigable to null.
  - 3. Abort a document and its descendants given document.
