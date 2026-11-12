# HTML review checklist: 8 Web application APIs

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 137 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html webappapis` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [8 Web application APIs](https://html.spec.whatwg.org/#webappapis)

Status: reviewed (2026-11-01)
> The sections below.

### [8.1 Scripting](https://html.spec.whatwg.org/#scripting)

Status: reviewed (2026-11-01)
> The sections below; scripts themselves are n/a.

#### [8.1.1 Introduction](https://html.spec.whatwg.org/#introduction-11)

Status: n/a (no JavaScript)

#### [8.1.2 Agents and agent clusters](https://html.spec.whatwg.org/#agents-and-agent-clusters)

Status: n/a (no JavaScript)

##### [8.1.2.1 Integration with the JavaScript agent formalism](https://html.spec.whatwg.org/#integration-with-the-javascript-agent-formalism)

Status: n/a (no JavaScript)
<!-- definitions: similar-origin-window-agent dedicated-worker-agent shared-worker-agent service-worker-agent worklet-agent -->

- [x] [create an agent](https://html.spec.whatwg.org/#create-an-agent) — To create an agent, given a boolean canBlock:
  - 1. Let signifier be a new unique internal value.
  - 2. Let candidateExecution be a new candidate execution.
  - 3. Let agent be a new agent whose [[CanBlock]] is canBlock, [[Signifier]] is signifier, [[CandidateExecution]] is candidateExecution, and [[IsLockFree1]], [[IsLockFree2]], and [[LittleEndian]] are set at the implementation's discretion.
  - 4. Set agent's event loop to a new event loop.
  - 5. Return agent.

- [x] [the realm's agent (realm)](https://html.spec.whatwg.org/#realm's-agent) — For a realm realm, the agent whose [[Signifier]] is realm.[[AgentSignifier]] is the realm's agent.

- [x] [relevant agent](https://html.spec.whatwg.org/#relevant-agent) — The relevant agent for a platform object platformObject is platformObject's relevant realm's agent.

##### [8.1.2.2 Integration with the JavaScript agent cluster formalism](https://html.spec.whatwg.org/#integration-with-the-javascript-agent-cluster-formalism)

Status: n/a (no JavaScript)
<!-- definitions: agent-cluster-cross-origin-isolation is-origin-keyed agent-cluster-key -->

- [x] [obtain a similar-origin window agent](https://html.spec.whatwg.org/#obtain-similar-origin-window-agent) — To obtain a similar-origin window agent, given an origin origin, a browsing context group group, and a boolean requestsOAC, run these steps:
  - 1. Let site be the result of obtaining a site with origin.
  - 2. Let key be site.
  - 3. If group's cross-origin isolation mode is not "none", then set key to origin.
  - 4. Otherwise, if group's historical agent cluster key map[origin] exists, then set key to group's historical agent cluster key map[origin].
  - 5. Otherwise:
    - 5a. If requestsOAC is true, then set key to origin.
    - 5b. Set group's historical agent cluster key map[origin] to key.
  - 6. If group's agent cluster map[key] does not exist:
    - 6a. Let agentCluster be a new agent cluster.
    - 6b. Set agentCluster's cross-origin isolation mode to group's cross-origin isolation mode.
    - 6c. If key is an origin:
      - 6c.i. Assert: key is origin.
      - 6c.ii. Set agentCluster's is origin-keyed to true.
    - 6d. Add the result of creating an agent, given false, to agentCluster.
    - 6e. Set group's agent cluster map[key] to agentCluster.
  - 7. Return the single similar-origin window agent contained in group's agent cluster map[key].

- [x] [obtain a worker/worklet agent](https://html.spec.whatwg.org/#obtaining-a-worker/worklet-agent) — To obtain a worker/worklet agent, given an environment settings object or null outside settings, a boolean isTopLevel, and a boolean canBlock, run these steps:
  - 1. Let agentCluster be null.
  - 2. If isTopLevel is true:
    - 2a. Set agentCluster to a new agent cluster.
    - 2b. Set agentCluster's is origin-keyed to true.
  - 3. Otherwise:
    - 3a. Assert: outside settings is not null.
    - 3b. Let ownerAgent be outside settings's realm's agent.
    - 3c. Set agentCluster to the agent cluster which contains ownerAgent.
  - 4. Let agent be the result of creating an agent given canBlock.
  - 5. Add agent to agentCluster.
  - 6. Return agent.

- [x] [obtain a dedicated/shared worker agent](https://html.spec.whatwg.org/#obtain-a-dedicated/shared-worker-agent) — To obtain a dedicated/shared worker agent, given an environment settings object outside settings and a boolean isShared, return the result of obtaining a worker/worklet agent given outside settings, isShared, and true.

- [x] [obtain a worklet agent](https://html.spec.whatwg.org/#obtain-a-worklet-agent) — To obtain a worklet agent, given an environment settings object outside settings, return the result of obtaining a worker/worklet agent given outside settings, false, and false.

- [x] [obtain a service worker agent](https://html.spec.whatwg.org/#obtain-a-service-worker-agent) — To obtain a service worker agent, return the result of obtaining a worker/worklet agent given null, true, and false.

#### [8.1.3 Realms and their counterparts](https://html.spec.whatwg.org/#realms-and-their-counterparts)

Status: reviewed (2026-11-01)
> The sections below.

##### [8.1.3.1 Environments](https://html.spec.whatwg.org/#environments)

Status: n/a (no JavaScript)
<!-- definitions: environment concept-environment-id concept-environment-creation-url concept-environment-top-level-creation-url concept-environment-top-level-origin concept-environment-target-browsing-context concept-environment-active-service-worker concept-environment-execution-ready-flag environment-discarding-steps -->

##### [8.1.3.2 Environment settings objects](https://html.spec.whatwg.org/#environment-settings-objects)

Status: n/a (no JavaScript)
<!-- definitions: environment-settings-object realm-execution-context concept-settings-object-module-map api-base-url concept-settings-object-origin concept-settings-object-has-cross-site-ancestor concept-settings-object-policy-container concept-settings-object-cross-origin-isolated-capability concept-settings-object-time-origin -->

- [x] [responsible event loop (environment settings object)](https://html.spec.whatwg.org/#responsible-event-loop) — An environment settings object's responsible event loop is its global object's relevant agent's event loop.

##### [8.1.3.3 Realms, settings objects, and global objects](https://html.spec.whatwg.org/#realms-settings-objects-global-objects)

Status: n/a (no JavaScript)
<!-- definitions: in-error-reporting-mode outstanding-rejected-promises-weak-set about-to-be-notified-rejected-promises-list concept-global-import-map resolved-module-set concept-global-object-realm concept-entry-everything concept-incumbent-everything concept-current-everything concept-relevant-everything -->

- [x] [global object](https://html.spec.whatwg.org/#global-object) — A global object is a JavaScript object that is the [[GlobalObject]] field of a realm.

- [x] [the realm's settings object (realm)](https://html.spec.whatwg.org/#concept-realm-settings-object) — A realm has a [[HostDefined]] field, which contains the realm's settings object.

- [x] [the realm's global object (realm)](https://html.spec.whatwg.org/#concept-realm-global) — A realm has a [[GlobalObject]] field, which contains the realm's global object.

- [x] [the environment settings object's realm (environment settings object)](https://html.spec.whatwg.org/#environment-settings-object's-realm) — An environment settings object's realm execution context's Realm component is the environment settings object's realm.

- [x] [the environment settings object's global object (environment settings object)](https://html.spec.whatwg.org/#concept-settings-object-global) — An environment settings object's realm then has a [[GlobalObject]] field, which contains the environment settings object's global object.

- [x] [create a new realm](https://html.spec.whatwg.org/#creating-a-new-javascript-realm) — To create a new realm in an agent agent, optionally with instructions to create a global object or a global this binding (or both), the following steps are taken:
  - 1. Perform InitializeHostDefinedRealm() with the provided customizations for creating the global object and the global this binding.
  - 2. Let realm execution context be the running JavaScript execution context.
  - 3. Remove realm execution context from the JavaScript execution context stack.
  - 4. Let realm be realm execution context's Realm component.
  - 5. If agent's agent cluster's cross-origin isolation mode is "none":
    - 5a. Let global be realm's global object.
    - 5b. Let status be ! global.[[Delete]]("SharedArrayBuffer").
    - 5c. Assert: status is true.
  - 6. Return realm execution context.

###### [8.1.3.3.1 Entry](https://html.spec.whatwg.org/#entry)

Status: n/a (no JavaScript)
<!-- definitions: concept-entry-realm -->

- [x] [entry execution context](https://html.spec.whatwg.org/#entry-execution-context) — With this in hand, we define the entry execution context to be the most recently pushed item in the JavaScript execution context stack that is a realm execution context. The entry realm is the entry execution context's Realm component.

- [x] [entry settings object](https://html.spec.whatwg.org/#entry-settings-object) — Then, the entry settings object is the environment settings object of the entry realm.

- [x] [entry global object](https://html.spec.whatwg.org/#entry-global-object) — Similarly, the entry global object is the global object of the entry realm.

###### [8.1.3.3.2 Incumbent](https://html.spec.whatwg.org/#incumbent)

Status: n/a (no JavaScript)
<!-- definitions: skip-when-determining-incumbent-counter backup-incumbent-settings-object-stack -->

- [x] [prepare to run a callback](https://html.spec.whatwg.org/#prepare-to-run-a-callback) — To prepare to run a callback with an environment settings object settings:
  - 1. Push settings onto the backup incumbent settings object stack.
  - 2. Let context be the topmost script-having execution context.
  - 3. If context is not null, increment context's skip-when-determining-incumbent counter.

- [x] [clean up after running a callback](https://html.spec.whatwg.org/#clean-up-after-running-a-callback) — To clean up after running a callback with an environment settings object settings:
  - 1. Let context be the topmost script-having execution context.
  - 2. If context is not null, decrement context's skip-when-determining-incumbent counter.
  - 3. Assert: the topmost entry of the backup incumbent settings object stack is settings.
  - 4. Remove settings from the backup incumbent settings object stack.

- [x] [topmost script-having execution context](https://html.spec.whatwg.org/#topmost-script-having-execution-context) — Here, the topmost script-having execution context is the topmost entry of the JavaScript execution context stack that has a non-null ScriptOrModule component, or null if there is no such entry in the JavaScript execution context stack.

- [x] [incumbent settings object](https://html.spec.whatwg.org/#incumbent-settings-object) — With all this in place, the incumbent settings object is determined as follows:
  - 1. Let context be the topmost script-having execution context.
  - 2. If context is null, or if context's skip-when-determining-incumbent counter is greater than zero:
    - 2a. Assert: the backup incumbent settings object stack is not empty.
    - 2b. Return the topmost entry of the backup incumbent settings object stack.
  - 3. Return context's Realm component's settings object.

- [x] [incumbent realm](https://html.spec.whatwg.org/#concept-incumbent-realm) — Then, the incumbent realm is the realm of the incumbent settings object.

- [x] [incumbent global object](https://html.spec.whatwg.org/#concept-incumbent-global) — Similarly, the incumbent global object is the global object of the incumbent settings object.

###### [8.1.3.3.3 Current](https://html.spec.whatwg.org/#current)

Status: n/a (no JavaScript)

- [x] [current settings object](https://html.spec.whatwg.org/#current-settings-object) — Then, the current settings object is the environment settings object of the current realm.

- [x] [current global object](https://html.spec.whatwg.org/#current-global-object) — Similarly, the current global object is the global object of the current realm.

###### [8.1.3.3.4 Relevant](https://html.spec.whatwg.org/#relevant)

Status: n/a (no JavaScript)

- [x] [relevant realm](https://html.spec.whatwg.org/#concept-relevant-realm) — The relevant realm for a platform object is the value of its [[Realm]] field.

- [x] [relevant settings object](https://html.spec.whatwg.org/#relevant-settings-object) — Then, the relevant settings object for a platform object o is the environment settings object of the relevant realm for o.

- [x] [relevant global object](https://html.spec.whatwg.org/#concept-relevant-global) — Similarly, the relevant global object for a platform object o is the global object of the relevant realm for o.

##### [8.1.3.4 Enabling and disabling scripting](https://html.spec.whatwg.org/#enabling-and-disabling-scripting)

Status: reviewed (2026-11-01)
> There's no script engine, so scripting is disabled for the parser (`<noscript>` is parsed as markup); for lazy loading and the `scripting` media feature, scripting counts as enabled for an element in a frame, the application playing the part of scripts (see `DIFFERENCES.md`).

- [x] [Scripting is enabled](https://html.spec.whatwg.org/#concept-environment-script) — Scripting is enabled for an environment settings object settings when all of the following conditions are true:

- [x] [Scripting is disabled](https://html.spec.whatwg.org/#concept-environment-noscript) — Scripting is disabled for an environment settings object when scripting is not enabled for it, i.e., when any of the above conditions are false.

- [x] [Scripting is disabled](https://html.spec.whatwg.org/#concept-n-noscript) — Scripting is disabled for a platform object object if any of the following are true:

- [x] [Scripting is enabled](https://html.spec.whatwg.org/#concept-n-script) — Scripting is enabled for a platform object object, when object's scripting is not disabled.

##### [8.1.3.5 Secure contexts](https://html.spec.whatwg.org/#secure-contexts)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::IsSecureContext`: the document's URL is potentially trustworthy; there are no ancestors to check.

- [x] [secure context](https://html.spec.whatwg.org/#secure-context) — An environment environment is a secure context if the following algorithm returns true:
  - 1. If environment is an environment settings object:
    - 1a. Let global be environment's global object.
    - 1b. If global is a WorkerGlobalScope:
      - 1b.i. If global's owner set[0]'s relevant settings object is a secure context, then return true.
      - 1b.ii. Return false.
    - 1c. If global is a WorkletGlobalScope, then return true.
  - 2. If environment's top-level creation URL's scheme is "data", then return false.
  - 3. If the result of Is url potentially trustworthy? given environment's top-level creation URL is "Potentially Trustworthy", then return true.
  - 4. Return false.

- [x] [non-secure context](https://html.spec.whatwg.org/#non-secure-context) — An environment is a non-secure context if it is not a secure context.

#### [8.1.4 Script processing model](https://html.spec.whatwg.org/#scripting-processing-model)

Status: reviewed (2026-11-01)
> The sections below.

##### [8.1.4.1 Scripts](https://html.spec.whatwg.org/#script-structs)

Status: n/a (no JavaScript)
<!-- definitions: concept-script settings-object concept-script-record concept-script-parse-error concept-script-error-to-rethrow concept-script-script-fetch-options concept-script-base-url classic-script muted-errors module-script -->

- [x] [JavaScript module script](https://html.spec.whatwg.org/#javascript-module-script) — A module script is a JavaScript module script if its record is a Source Text Module Record.

- [x] [CSS module script](https://html.spec.whatwg.org/#css-module-script) — A module script is a CSS module script if its record is a Synthetic Module Record, and it was created via the create a CSS module script algorithm. CSS module scripts represent a parsed CSS style sheet.

- [x] [JSON module script](https://html.spec.whatwg.org/#json-module-script) — A module script is a JSON module script if its record is a Synthetic Module Record, and it was created via the create a JSON module script algorithm. JSON module scripts represent a parsed JSON document.

- [x] [text module script](https://html.spec.whatwg.org/#text-module-script) — A module script is a text module script if its record is a Synthetic Module Record, and it was created via the create a text module script algorithm. Text module scripts represent textual data encoded as UTF-8.

- [x] [WebAssembly module script](https://html.spec.whatwg.org/#webassembly-module-script) — A module script is a WebAssembly module script if its record is a WebAssembly Module Record.

- [x] [active script](https://html.spec.whatwg.org/#active-script) — The active script is determined by the following algorithm:
  - 1. Let record be GetActiveScriptOrModule().
  - 2. If record is null, return null.
  - 3. Return record.[[HostDefined]].

##### [8.1.4.2 Fetching scripts](https://html.spec.whatwg.org/#fetching-scripts)

Status: n/a (no JavaScript)
<!-- definitions: script-fetch-options concept-script-fetch-options-nonce concept-script-fetch-options-integrity concept-script-fetch-options-parser concept-script-fetch-options-credentials concept-script-fetch-options-referrer-policy concept-script-fetch-options-render-blocking concept-script-fetch-options-fetch-priority fetching-scripts-processcustomfetchresponse fetching-scripts-is-top-level -->

- [x] [default script fetch options](https://html.spec.whatwg.org/#default-script-fetch-options) — The default script fetch options are a script fetch options whose cryptographic nonce is the empty string, integrity metadata is the empty string, parser metadata is "not-parser-inserted", credentials mode is "same-origin", referrer policy is the empty string, and fetch priority is "auto".

- [x] [set up the classic script request](https://html.spec.whatwg.org/#set-up-the-classic-script-request) — To set up the classic script request, given a request request and a script fetch options options, set request's cryptographic nonce metadata to options's cryptographic nonce, its integrity metadata to options's integrity metadata, its parser metadata to options's parser metadata, its referrer policy to options's referrer policy, its render-blocking to options's render-blocking, and its priority to options's fetch priority.

- [x] [set up the module script request](https://html.spec.whatwg.org/#set-up-the-module-script-request) — To set up the module script request, given a request request and a script fetch options options, set request's cryptographic nonce metadata to options's cryptographic nonce, its integrity metadata to options's integrity metadata, its parser metadata to options's parser metadata, its credentials mode to options's credentials mode, its referrer policy to options's referrer policy, its render-blocking to options's render-blocking, and its priority to options's fetch priority.

- [x] [get the descendant script fetch options](https://html.spec.whatwg.org/#get-the-descendant-script-fetch-options) — To get the descendant script fetch options given a script fetch options originalOptions, a URL url, and an environment settings object settingsObject:
  - 1. Let newOptions be a copy of originalOptions.
  - 2. Let integrity be the result of resolving a module integrity metadata with url and settingsObject.
  - 3. Set newOptions's integrity metadata to integrity.
  - 4. Set newOptions's fetch priority to "auto".
  - 5. Return newOptions.

- [x] [resolve a module integrity metadata](https://html.spec.whatwg.org/#resolving-a-module-integrity-metadata) — To resolve a module integrity metadata, given a URL url and an environment settings object settingsObject:
  - 1. Let map be settingsObject's global object's import map.
  - 2. If map's integrity[url] does not exist, then return the empty string.
  - 3. Return map's integrity[url].

- [x] [perform the fetch hook (fetching scripts)](https://html.spec.whatwg.org/#fetching-scripts-perform-fetch) — Several of the below algorithms can be customized with a perform the fetch hook algorithm, which takes a request, a boolean isTopLevel, and a processCustomFetchResponse algorithm. It runs processCustomFetchResponse with a response and either null (on failure) or a byte sequence containing the response body. isTopLevel will be true for all classic script fetches, and for the initial fetch when fetching an external module script graph or fetching a module worker script graph, but false for the fetches resulting from import statements encountered throughout the graph or from import() expressions.

- [x] [fetch a classic script](https://html.spec.whatwg.org/#fetch-a-classic-script) — To fetch a classic script given a URL url, an environment settings object settingsObject, a script fetch options options, a CORS settings attribute state corsSetting, an encoding encoding, and an algorithm onComplete, run these steps. onComplete must be an algorithm accepting null (on failure) or a…
  - 1. Let request be the result of creating a potential-CORS request given url, "script", and corsSetting.
  - 2. Set request's client to settingsObject.
  - 3. Set request's initiator type to "script".
  - 4. Set up the classic script request given request and options.
  - 5. Fetch request with the following processResponseConsumeBody steps given response response and null, failure, or a byte sequence bodyBytes:
    - 5a. Set response to response's unsafe response.
    - 5b. If any of the following are true: bodyBytes is null or failure; or; response's status is not an ok status, then run onComplete given null, and abort these steps.
    - 5c. Let potentialMIMETypeForEncoding be the result of extracting a MIME type given response's header list.
    - 5d. Set encoding to the result of legacy extracting an encoding given potentialMIMETypeForEncoding and encoding.
    - 5e. Let sourceText be the result of decoding bodyBytes to Unicode, using encoding as the fallback encoding.
    - 5f. Let mutedErrors be true if response was CORS-cross-origin, and false otherwise.
    - 5g. Let script be the result of creating a classic script given sourceText, settingsObject, response's URL, options, mutedErrors, and url.
    - 5h. Run onComplete given script.

- [x] [fetch a classic worker script](https://html.spec.whatwg.org/#fetch-a-classic-worker-script) — To fetch a classic worker script given a URL url, an environment settings object fetchClient, a destination destination, an environment settings object settingsObject, an algorithm onComplete, and an optional perform the fetch hook performFetch, run these steps. onComplete must be an algorithm acce…
  - 1. Let request be a new request whose URL is url, client is fetchClient, destination is destination, initiator type is "other", mode is "same-origin", credentials mode is "same-origin", parser metadata is "not parser-inserted", and whose use-URL-credentials flag is set.
  - 2. If performFetch was given, run performFetch with request, true, and with processResponseConsumeBody as defined below. Otherwise, fetch request with processResponseConsumeBody set to processResponseConsumeBody as defined below. In both cases, let processResponseConsumeBody given response response and null, failure, or a byte sequence bodyBytes be the following algorithm:
    - 2a. Set response to response's unsafe response.
    - 2b. If any of the following are true: bodyBytes is null or failure; or; response's status is not an ok status, then run onComplete given null, and abort these steps.
    - 2c. If all of the following are true: response's URL's scheme is an HTTP(S) scheme; and; the result of extracting a MIME type from response's header list is not a JavaScript MIME type, then run onComplete given null, and abort these steps.
    - 2d. Let sourceText be the result of UTF-8 decoding bodyBytes.
    - 2e. Let script be the result of creating a classic script using sourceText, settingsObject, response's URL, and the default script fetch options.
    - 2f. Run onComplete given script.

- [x] [fetch a classic worker-imported script](https://html.spec.whatwg.org/#fetch-a-classic-worker-imported-script) — To fetch a classic worker-imported script given a URL url, an environment settings object settingsObject, and an optional perform the fetch hook performFetch, run these steps. The algorithm will return a classic script on success, or throw an exception on failure.
  - 1. Let response be null.
  - 2. Let bodyBytes be null.
  - 3. Let request be a new request whose URL is url, client is settingsObject, destination is "script", initiator type is "other", parser metadata is "not parser-inserted", and whose use-URL-credentials flag is set.
  - 4. If performFetch was given, run performFetch with request, true, and with processResponseConsumeBody as defined below. Otherwise, fetch request with processResponseConsumeBody set to processResponseConsumeBody as defined below. In both cases, let processResponseConsumeBody given response res and null, failure, or a byte sequence bb be the following algorithm:
    - 4a. Set bodyBytes to bb.
    - 4b. Set response to res.
  - 5. Pause until response is not null.
  - 6. Set response to response's unsafe response.
  - 7. If any of the following are true: bodyBytes is null or failure;; response's status is not an ok status; or; the result of extracting a MIME type from response's header list is not a JavaScript MIME type, then throw a "NetworkError" DOMException.
  - 8. Let sourceText be the result of UTF-8 decoding bodyBytes.
  - 9. Let mutedErrors be true if response was CORS-cross-origin, and false otherwise.
  - 10. Let script be the result of creating a classic script given sourceText, settingsObject, response's URL, the default script fetch options, and mutedErrors.
  - 11. Return script.

- [x] [fetch an external module script graph](https://html.spec.whatwg.org/#fetch-a-module-script-tree) — To fetch an external module script graph given a URL url, an environment settings object settingsObject, a script fetch options options, and an algorithm onComplete, run these steps. onComplete must be an algorithm accepting null (on failure) or a module script (on success).
  - 1. Fetch a single module script given url, settingsObject, "script", options, settingsObject, "client", true, and with the following steps given result:
    - 1a. If result is null, run onComplete given null, and abort these steps.
    - 1b. Fetch the descendants of and link result given settingsObject, "script", and onComplete.

- [x] [fetch a modulepreload module script graph](https://html.spec.whatwg.org/#fetch-a-modulepreload-module-script-graph) — To fetch a modulepreload module script graph given a URL url, a destination destination, an environment settings object settingsObject, a script fetch options options, and an algorithm onComplete, run these steps. onComplete must be an algorithm accepting null (on failure) or a module script (on su…
  - 1. Fetch a single module script given url, settingsObject, destination, options, settingsObject, "client", true, and with the following steps given result:
    - 1a. Run onComplete given result.
    - 1b. Assert: settingsObject's global object implements Window.
    - 1c. If result is not null, optionally fetch the descendants of and link result given settingsObject, destination, and an empty algorithm.

- [x] [fetch an inline module script graph](https://html.spec.whatwg.org/#fetch-an-inline-module-script-graph) — To fetch an inline module script graph given a string sourceText, a URL baseURL, an environment settings object settingsObject, a script fetch options options, and an algorithm onComplete, run these steps. onComplete must be an algorithm accepting null (on failure) or a module script (on success).
  - 1. Let script be the result of creating a JavaScript module script using sourceText, settingsObject, baseURL, and options.
  - 2. Fetch the descendants of and link script, given settingsObject, "script", and onComplete.

- [x] [fetch a module worker script graph](https://html.spec.whatwg.org/#fetch-a-module-worker-script-tree) — To fetch a module worker script graph given a URL url, an environment settings object fetchClient, a destination destination, a credentials mode credentialsMode, an environment settings object settingsObject, and an algorithm onComplete, fetch a worklet/module worker script graph given url, fetchClient, destination, credentialsMode, settingsObject, and onComplete.

- [x] [fetch a worklet script graph](https://html.spec.whatwg.org/#fetch-a-worklet-script-graph) — To fetch a worklet script graph given a URL url, an environment settings object fetchClient, a destination destination, a credentials mode credentialsMode, an environment settings object settingsObject, a module responses map moduleResponsesMap, and an algorithm onComplete, fetch a worklet/module w…
  - 1. Let requestURL be request's URL.
  - 2. If moduleResponsesMap[requestURL] is "fetching", wait in parallel until that entry's value changes, then queue a task on the networking task source to proceed with running the following steps.
  - 3. If moduleResponsesMap[requestURL] exists:
    - 3a. Let cached be moduleResponsesMap[requestURL].
    - 3b. Run processCustomFetchResponse with cached[0] and cached[1].
    - 3c. Return.
  - 4. Set moduleResponsesMap[requestURL] to "fetching".
  - 5. Fetch request, with processResponseConsumeBody set to the following steps given response response and null, failure, or a byte sequence bodyBytes:
    - 5a. Set moduleResponsesMap[requestURL] to (response, bodyBytes).
    - 5b. Run processCustomFetchResponse with response and bodyBytes.

- [x] [fetch a worklet/module worker script graph](https://html.spec.whatwg.org/#fetch-a-worklet/module-worker-script-graph) — To fetch a worklet/module worker script graph given a URL url, an environment settings object fetchClient, a destination destination, a credentials mode credentialsMode, an environment settings object settingsObject, an algorithm onComplete, and an optional perform the fetch hook performFetch, run…
  - 1. Let options be a script fetch options whose cryptographic nonce is the empty string, integrity metadata is the empty string, parser metadata is "not-parser-inserted", credentials mode is credentialsMode, referrer policy is the empty string, and fetch priority is "auto".
  - 2. Fetch a single module script given url, fetchClient, destination, options, settingsObject, "client", true, and onSingleFetchComplete as defined below. If performFetch was given, pass it along as well. onSingleFetchComplete given result is the following algorithm:
    - 2a. If result is null, run onComplete given null, and abort these steps.
    - 2b. Fetch the descendants of and link result given fetchClient, destination, and onComplete. If performFetch was given, pass it along as well.

- [x] [fetch the descendants of and link](https://html.spec.whatwg.org/#fetch-the-descendants-of-and-link-a-module-script) — To fetch the descendants of and link a module script moduleScript, given an environment settings object fetchClient, a destination destination, an algorithm onComplete, and an optional perform the fetch hook performFetch, run these steps. onComplete must be an algorithm accepting null (on failure)…
  - 1. Let record be moduleScript's record.
  - 2. If record is null:
    - 2a. Set moduleScript's error to rethrow to moduleScript's parse error.
    - 2b. Run onComplete given moduleScript.
    - 2c. Return.
  - 3. Let state be Record { [[ErrorToRethrow]]: null, [[Destination]]: destination, [[PerformFetch]]: null, [[FetchClient]]: fetchClient }.
  - 4. If performFetch was given, set state.[[PerformFetch]] to performFetch.
  - 5. Let loadingPromise be record.LoadRequestedModules(state).
  - 6. Upon fulfillment of loadingPromise, run the following steps:
    - 6a. Perform record.Link(). If this throws an exception, catch it, and set moduleScript's error to rethrow to that exception.
    - 6b. Run onComplete given moduleScript.
  - 7. Upon rejection of loadingPromise, run the following steps:
    - 7a. If state.[[ErrorToRethrow]] is not null, set moduleScript's error to rethrow to state.[[ErrorToRethrow]] and run onComplete given moduleScript.
    - 7b. Otherwise, run onComplete given null.

- [x] [fetch a single module script](https://html.spec.whatwg.org/#fetch-a-single-module-script) — To fetch a single module script, given a URL url, an environment settings object fetchClient, a destination destination, a script fetch options options, an environment settings object settingsObject, a referrer referrer, an optional ModuleRequest Record moduleRequest, a boolean isTopLevel, an algor…
  - 1. Let moduleType be "javascript-or-wasm".
  - 2. If moduleRequest was given, then set moduleType to the result of running the module type from module request steps given moduleRequest.
  - 3. Assert: the result of running the module type allowed steps given moduleType and settingsObject is true. Otherwise, we would not have reached this point because a failure would have been raised when inspecting moduleRequest.[[Attributes]] in HostLoadImportedModule or fetch a single imported module script.
  - 4. Let moduleMap be settingsObject's module map.
  - 5. If moduleMap[(url, moduleType)] is a module script, run onComplete given moduleMap[(url, moduleType)], and return.
  - 6. If moduleMap[(url, moduleType)] is a list, append onComplete to moduleMap[(url, moduleType)], and return.
  - 7. Set moduleMap[(url, moduleType)] to « onComplete ».
  - 8. Let request be a new request whose URL is url, mode is "cors", referrer is referrer, and client is fetchClient.
  - 9. Set request's destination to the result of running the fetch destination from module type steps given destination and moduleType.
  - 10. If destination is "worker", "sharedworker", or "serviceworker", and isTopLevel is true, then set request's mode to "same-origin".
  - 11. Set request's initiator type to "script".
  - 12. Set up the module script request given request and options.
  - 13. If performFetch was given, run performFetch with request, isTopLevel, and with processResponseConsumeBody as defined below. Otherwise, fetch request with processResponseConsumeBody set to processResponseConsumeBody as defined below. In both cases, let processResponseConsumeBody given response response and null, failure, or a byte sequence bodyBytes be the following algorithm:
    - 13a. If any of the following are true: bodyBytes is null or failure; or; response's status is not an ok status, then:
      - 13a.i. Let callbacks be moduleMap[(url, moduleType)].
      - 13a.ii. Remove moduleMap[(url, moduleType)].
      - 13a.iii. For each callback of callbacks: run callback given null.
      - 13a.iv. Return.
    - 13b. Let mimeType be the result of extracting a MIME type from response's header list.
    - 13c. Let moduleScript be null.
    - 13d. Let referrerPolicy be the result of parsing the `Referrer-Policy` header given response. [REFERRERPOLICY]
    - 13e. If referrerPolicy is not the empty string, set options's referrer policy to referrerPolicy.
    - 13f. If mimeType's essence is "application/wasm" and moduleType is "javascript-or-wasm", then set moduleScript to the result of creating a WebAssembly module script given bodyBytes, settingsObject, response's URL, and options.
    - 13g. Otherwise:
      - 13g.i. Let sourceText be the result of UTF-8 decoding bodyBytes.
      - 13g.ii. If moduleType is "text", then set moduleScript to the result of creating a text module script given sourceText and settingsObject.
      - 13g.iii. If mimeType is a JavaScript MIME type and moduleType is "javascript-or-wasm", then set moduleScript to the result of creating a JavaScript module script given sourceText, settingsObject, response's URL, and options.
      - 13g.iv. If the MIME type essence of mimeType is "text/css" and moduleType is "css", then set moduleScript to the result of creating a CSS module script given sourceText and settingsObject.
      - 13g.v. If mimeType is a JSON MIME type and moduleType is "json", then set moduleScript to the result of creating a JSON module script given sourceText and settingsObject.
    - 13h. Let callbacks be moduleMap[(url, moduleType)].
    - 13i. If moduleScript is null, then remove moduleMap[(url, moduleType)]; otherwise set moduleMap[(url, moduleType)] to moduleScript.
    - 13j. For each callback of callbacks: run callback given moduleScript.

- [x] [fetch a single imported module script](https://html.spec.whatwg.org/#fetch-a-single-imported-module-script) — To fetch a single imported module script, given a URL url, an environment settings object fetchClient, a destination destination, a script fetch options options, environment settings object settingsObject, a referrer referrer, a ModuleRequest Record moduleRequest, an algorithm onComplete, and an op…
  - 1. Assert: moduleRequest.[[Attributes]] does not contain any Record entry such that entry.[[Key]] is not "type", because we only asked for "type" attributes in HostGetSupportedImportAttributes.
  - 2. Let moduleType be the result of running the module type from module request steps given moduleRequest.
  - 3. If the result of running the module type allowed steps given moduleType and settingsObject is false, then run onComplete given null, and return.
  - 4. Fetch a single module script given url, fetchClient, destination, options, settingsObject, referrer, moduleRequest, false, and onComplete. If performFetch was given, pass it along as well.

##### [8.1.4.3 Creating scripts](https://html.spec.whatwg.org/#creating-scripts)

Status: n/a (no JavaScript)
<!-- definitions: webdriver-bidi-command-script-evaluate -->

- [x] [create a classic script](https://html.spec.whatwg.org/#creating-a-classic-script) — To create a classic script, given a string source, an environment settings object settings, a URL baseURL, a script fetch options options, an optional boolean mutedErrors (default false), an optional URL-or-null sourceURLForWindowScripts (default null), and an optional boolean bypassDisabledScripti…
  - 1. If mutedErrors is true, then set baseURL to about:blank.
  - 2. If scripting is disabled for settings and bypassDisabledScripting is false, then set source to the empty string.
  - 3. Let script be a new classic script that this algorithm will subsequently initialize.
  - 4. Set script's settings object to settings.
  - 5. Set script's base URL to baseURL.
  - 6. Set script's fetch options to options.
  - 7. Set script's muted errors to mutedErrors.
  - 8. Set script's parse error and error to rethrow to null.
  - 9. Record classic script creation time given script and sourceURLForWindowScripts.
  - 10. Let result be ParseScript(source, settings's realm, script).
  - 11. If result is a list of errors:
    - 11a. Set script's parse error and its error to rethrow to result[0].
    - 11b. Return script.
  - 12. Set script's record to result.
  - 13. Return script.

- [x] [create a JavaScript module script](https://html.spec.whatwg.org/#creating-a-javascript-module-script) — To create a JavaScript module script, given a string source, an environment settings object settings, a URL baseURL, and a script fetch options options:
  - 1. If scripting is disabled for settings, then set source to the empty string.
  - 2. Let script be a new module script that this algorithm will subsequently initialize.
  - 3. Set script's settings object to settings.
  - 4. Set script's base URL to baseURL.
  - 5. Set script's fetch options to options.
  - 6. Set script's parse error and error to rethrow to null.
  - 7. Let result be ParseModule(source, settings's realm, script).
  - 8. If result is a list of errors:
    - 8a. Set script's parse error to result[0].
    - 8b. Return script.
  - 9. Set script's record to result.
  - 10. Return script.

- [x] [create a WebAssembly module script](https://html.spec.whatwg.org/#creating-a-webassembly-module-script) — To create a WebAssembly module script, given a byte sequence bodyBytes, an environment settings object settings, a URL baseURL, and a script fetch options options:
  - 1. If scripting is disabled for settings, then set bodyBytes to the byte sequence 0x00 0x61 0x73 0x6D 0x01 0x00 0x00 0x00.
  - 2. Let script be a new module script that this algorithm will subsequently initialize.
  - 3. Set script's settings object to settings.
  - 4. Set script's base URL to baseURL.
  - 5. Set script's fetch options to options.
  - 6. Set script's parse error and error to rethrow to null.
  - 7. Let result be the result of parsing a WebAssembly module given bodyBytes, settings's realm, and script.
  - 8. If the previous step threw an error error:
    - 8a. Set script's parse error to error.
    - 8b. Return script.
  - 9. Set script's record to result.
  - 10. Return script.

- [x] [create a CSS module script](https://html.spec.whatwg.org/#creating-a-css-module-script) — To create a CSS module script, given a string source and an environment settings object settings:
  - 1. Let script be a new module script that this algorithm will subsequently initialize.
  - 2. Set script's settings object to settings.
  - 3. Set script's base URL and fetch options to null.
  - 4. Set script's parse error and error to rethrow to null.
  - 5. Let sheet be the result of running the steps to create a constructed CSSStyleSheet with an empty dictionary as the argument.
  - 6. Run the steps to synchronously replace the rules of a CSSStyleSheet on sheet given source. If this throws an exception, catch it, and set script's parse error to that exception, and return script.
  - 7. Set script's record to the result of CreateDefaultExportSyntheticModule(sheet).
  - 8. Return script.

- [x] [create a JSON module script](https://html.spec.whatwg.org/#creating-a-json-module-script) — To create a JSON module script, given a string source and an environment settings object settings:
  - 1. Let script be a new module script that this algorithm will subsequently initialize.
  - 2. Set script's settings object to settings.
  - 3. Set script's base URL and fetch options to null.
  - 4. Set script's parse error and error to rethrow to null.
  - 5. Let result be ParseJSONModule(source). If this throws an exception, catch it, and set script's parse error to that exception, and return script.
  - 6. Set script's record to result.
  - 7. Return script.

- [x] [create a text module script](https://html.spec.whatwg.org/#creating-a-text-module-script) — To create a text module script, given a string text and an environment settings object settings:
  - 1. Let script be a new module script that this algorithm will subsequently initialize.
  - 2. Set script's settings object to settings.
  - 3. Set script's base URL and fetch options to null.
  - 4. Set script's parse error and error to rethrow to null.
  - 5. Let result be CreateTextModule(text).
  - 6. Set script's record to result.
  - 7. Return script.

- [x] [module type from module request](https://html.spec.whatwg.org/#module-type-from-module-request) — The module type from module request steps, given a ModuleRequest Record moduleRequest, are as follows:
  - 1. Let moduleType be "javascript-or-wasm".
  - 2. If moduleRequest.[[Attributes]] has a Record entry such that entry.[[Key]] is "type":
    - 2a. If entry.[[Value]] is "javascript-or-wasm", then set moduleType to null.
    - 2b. Otherwise, set moduleType to entry.[[Value]].
  - 3. Return moduleType.

- [x] [module type allowed](https://html.spec.whatwg.org/#module-type-allowed) — The module type allowed steps, given a string moduleType and an environment settings object settings, are as follows:
  - 1. If moduleType is not "javascript-or-wasm", "css", "json", or "text", then return false.
  - 2. If moduleType is "css" and the CSSStyleSheet interface is not exposed in settings's realm, then return false.
  - 3. Return true.

- [x] [fetch destination from module type](https://html.spec.whatwg.org/#fetch-destination-from-module-type) — The fetch destination from module type steps, given a destination defaultDestination and a string moduleType, are as follows:
  - 1. If moduleType is "json", then return "json".
  - 2. If moduleType is "css", then return "style".
  - 3. If moduleType is "text", then return "text".
  - 4. Return defaultDestination.

##### [8.1.4.4 Calling scripts](https://html.spec.whatwg.org/#calling-scripts)

Status: n/a (no JavaScript)

- [x] [run a classic script](https://html.spec.whatwg.org/#run-a-classic-script) — To run a classic script given a classic script script and an optional boolean rethrow errors (default false):
  - 1. Let settings be the settings object of script.
  - 2. Check if we can run script with settings. If this returns "do not run", then return NormalCompletion(empty).
  - 3. Record classic script execution start time given script.
  - 4. Prepare to run script given settings.
  - 5. Let evaluationStatus be null.
  - 6. If script's error to rethrow is not null, then set evaluationStatus to ThrowCompletion(script's error to rethrow).
  - 7. Otherwise, set evaluationStatus to ScriptEvaluation(script's record). If ScriptEvaluation does not complete because the user agent has aborted the running script, leave evaluationStatus as null.
  - 8. If evaluationStatus is an abrupt completion:
    - 8a. If rethrow errors is true and script's muted errors is false:
      - 8a.i. Clean up after running script with settings.
      - 8a.ii. Rethrow evaluationStatus.[[Value]].
    - 8b. If rethrow errors is true and script's muted errors is true:
      - 8b.i. Clean up after running script with settings.
      - 8b.ii. Throw a "NetworkError" DOMException.
    - 8c. Otherwise, rethrow errors is false. Perform the following steps:
      - 8c.i. Report an exception given by evaluationStatus.[[Value]] for script's settings object's global object.
      - 8c.ii. Clean up after running script with settings.
      - 8c.iii. Return evaluationStatus.
  - 9. Clean up after running script with settings.
  - 10. If evaluationStatus is a normal completion, then return evaluationStatus.
  - 11. If we've reached this point, evaluationStatus was left as null because the script was aborted prematurely during evaluation. Return ThrowCompletion(a new QuotaExceededError).

- [x] [run a module script](https://html.spec.whatwg.org/#run-a-module-script) — To run a module script given a module script script and an optional boolean preventErrorReporting (default false):
  - 1. Let settings be the settings object of script.
  - 2. Check if we can run script with settings. If this returns "do not run", then return a promise resolved with undefined.
  - 3. Record module script execution start time given script.
  - 4. Prepare to run script given settings.
  - 5. Let evaluationPromise be null.
  - 6. If script's error to rethrow is not null, then set evaluationPromise to a promise rejected with script's error to rethrow.
  - 7. Otherwise:
    - 7a. Let record be script's record.
    - 7b. Set evaluationPromise to record.Evaluate(). If Evaluate fails to complete as a result of the user agent aborting the running script, then set evaluationPromise to a promise rejected with a new QuotaExceededError.
  - 8. If preventErrorReporting is false, then upon rejection of evaluationPromise with reason, report an exception given by reason for script's settings object's global object.
  - 9. Clean up after running script with settings.
  - 10. Return evaluationPromise.

- [x] [check if we can run script](https://html.spec.whatwg.org/#check-if-we-can-run-script) — The steps to check if we can run script with an environment settings object settings are as follows. They return either "run" or "do not run".
  - 1. If the global object specified by settings is a Window object whose Document object is not fully active, then return "do not run".
  - 2. If scripting is disabled for settings, then return "do not run".
  - 3. Return "run".

- [x] [prepare to run script](https://html.spec.whatwg.org/#prepare-to-run-script) — The steps to prepare to run script with an environment settings object settings are as follows:
  - 1. Push settings's realm execution context onto the JavaScript execution context stack; it is now the running JavaScript execution context.
  - 2. Add settings to the surrounding agent's event loop's currently running task's script evaluation environment settings object set.

- [x] [clean up after running script](https://html.spec.whatwg.org/#clean-up-after-running-script) — The steps to clean up after running script with an environment settings object settings are as follows:
  - 1. Assert: settings's realm execution context is the running JavaScript execution context.
  - 2. Remove settings's realm execution context from the JavaScript execution context stack.
  - 3. If the JavaScript execution context stack is now empty, perform a microtask checkpoint. (If this runs scripts, these algorithms will be invoked reentrantly.)

- [x] [running script](https://html.spec.whatwg.org/#running-script) — The running script is the script in the [[HostDefined]] field in the ScriptOrModule component of the running JavaScript execution context.

##### [8.1.4.5 Killing scripts](https://html.spec.whatwg.org/#killing-scripts)

Status: n/a (no JavaScript)
<!-- definitions: abort-a-running-script -->

##### [8.1.4.6 Runtime script errors](https://html.spec.whatwg.org/#runtime-script-errors)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::ReportError` reports an exception, with a message and a value but no script location (FIXME).
<!-- definitions: report-exception-omiterror report-the-exception -->

- [x] [extract error information](https://html.spec.whatwg.org/#extract-error) — To extract error information from a JavaScript value exception:
  - 1. Let attributes be an empty map keyed by IDL attributes.
  - 2. Set attributes[error] to exception.
  - 3. Set attributes[message], attributes[filename], attributes[lineno], and attributes[colno] to implementation-defined values derived from exception.
  - 4. Return attributes.

- [x] [report an exception](https://html.spec.whatwg.org/#report-an-exception) — To report an exception exception which is a JavaScript value, for a particular global object global and optional boolean omitError (default false):
  - 1. Let notHandled be true.
  - 2. Let errorInfo be the result of extracting error information from exception.
  - 3. Let script be a script found in an implementation-defined way, or null. This should usually be the running script (most notably during run a classic script).
  - 4. If script is a classic script and script's muted errors is true, then set errorInfo[error] to null, errorInfo[message] to "Script error.", errorInfo[filename] to the empty string, errorInfo[lineno] to 0, and errorInfo[colno] to 0.
  - 5. If omitError is true, then set errorInfo[error] to null.
  - 6. If global is not in error reporting mode:
    - 6a. Set global's in error reporting mode to true.
    - 6b. If global implements EventTarget, then set notHandled to the result of firing an event named error at global, using ErrorEvent, with the cancelable attribute initialized to true, and additional attributes initialized according to errorInfo.
    - 6c. Set global's in error reporting mode to false.
  - 7. If notHandled is true:
    - 7a. Set errorInfo[error] to null.
    - 7b. If global implements DedicatedWorkerGlobalScope, queue a global task on the DOM manipulation task source with the global's associated Worker's relevant global object to run these steps:
      - 7b.i. Let workerObject be the Worker object associated with global.
      - 7b.ii. Set notHandled to the result of firing an event named error at workerObject, using ErrorEvent, with the cancelable attribute initialized to true, and additional attributes initialized according to errorInfo.
      - 7b.iii. If notHandled is true, then report exception for workerObject's relevant global object with omitError set to true.
    - 7c. Otherwise, the user agent may report exception to a developer console.

- [x] [WindowOrWorkerGlobalScope.reportError(e)](https://html.spec.whatwg.org/#dom-reporterror) — The reportError(e) method steps are to report an exception e for this.

- [x] [ErrorEvent.message](https://html.spec.whatwg.org/#dom-errorevent-message) — The message attribute must return the value it was initialized to. It represents the error message.

- [x] [ErrorEvent.filename](https://html.spec.whatwg.org/#dom-errorevent-filename) — The filename attribute must return the value it was initialized to. It represents the URL of the script in which the error originally occurred.

- [x] [ErrorEvent.lineno](https://html.spec.whatwg.org/#dom-errorevent-lineno) — The lineno attribute must return the value it was initialized to. It represents the line number where the error occurred in the script.

- [x] [ErrorEvent.colno](https://html.spec.whatwg.org/#dom-errorevent-colno) — The colno attribute must return the value it was initialized to. It represents the column number where the error occurred in the script.

- [x] [ErrorEvent.error](https://html.spec.whatwg.org/#dom-errorevent-error) — The error attribute must return the value it was initialized to. It must initially be initialized to undefined. Where appropriate, it is set to the object representing the error (e.g., the exception object in the case of an uncaught exception).

##### [8.1.4.7 Unhandled promise rejections](https://html.spec.whatwg.org/#unhandled-promise-rejections)

Status: n/a (no JavaScript)

- [x] [notify about rejected promises](https://html.spec.whatwg.org/#notify-about-rejected-promises) — To notify about rejected promises given a global object global:
  - 1. Let list be a clone of global's about-to-be-notified rejected promises list.
  - 2. If list is empty, then return.
  - 3. Empty global's about-to-be-notified rejected promises list.
  - 4. Queue a global task on the DOM manipulation task source given global to run the following step:
    - 4a. For each promise p of list:
      - 4a.i. If p.[[PromiseIsHandled]] is true, then continue.
      - 4a.ii. Let notCanceled be the result of firing an event named unhandledrejection at global, using PromiseRejectionEvent, with the cancelable attribute initialized to true, the promise attribute initialized to p, and the reason attribute initialized to p.[[PromiseResult]].
      - 4a.iii. If notCanceled is true, then the user agent may report p.[[PromiseResult]] to a developer console.
      - 4a.iv. If p.[[PromiseIsHandled]] is false, then append p to global's outstanding rejected promises weak set.

- [x] [PromiseRejectionEvent.promise](https://html.spec.whatwg.org/#dom-promiserejectionevent-promise) — The promise attribute must return the value it was initialized to. It represents the promise which this notification is about.

- [x] [PromiseRejectionEvent.reason](https://html.spec.whatwg.org/#dom-promiserejectionevent-reason) — The reason attribute must return the value it was initialized to. It represents the rejection reason for the promise.

##### [8.1.4.8 Import map parse results](https://html.spec.whatwg.org/#import-map-parse-results)

Status: n/a (no JavaScript)
<!-- definitions: import-map-parse-result impr-import-map impr-error-to-rethrow -->

- [x] [create an import map parse result](https://html.spec.whatwg.org/#create-an-import-map-parse-result) — To create an import map parse result given a string input and a URL baseURL:
  - 1. Let result be an import map parse result whose import map is null and whose error to rethrow is null.
  - 2. Parse an import map string given input and baseURL, catching any exceptions. If this threw an exception, then set result's error to rethrow to that exception. Otherwise, set result's import map to the return value.
  - 3. Return result.

- [x] [register an import map](https://html.spec.whatwg.org/#register-an-import-map) — To register an import map given a Window global and an import map parse result result:
  - 1. If result's error to rethrow is not null, then report an exception given by result's error to rethrow for global and return.
  - 2. Merge existing and new import maps, given global and result's import map.

##### [8.1.4.9 Speculation rules parse results](https://html.spec.whatwg.org/#speculation-rules-parse-results)

Status: n/a (no JavaScript)
<!-- definitions: speculation-rules-parse-result srpr-sr-set srpr-error-to-rethrow -->

- [x] [create a speculation rules parse result](https://html.spec.whatwg.org/#create-a-speculation-rules-parse-result) — To create a speculation rules parse result given a string input and a Document document:
  - 1. Let result be a speculation rules parse result whose speculation rule set is null and whose error to rethrow is null.
  - 2. Parse a speculation rule set string given input, document, and document's document base URL, catching any exceptions. If this threw an exception, then set result's error to rethrow to that exception. Otherwise, set result's speculation rule set to the return value.
  - 3. Return result.

- [x] [register speculation rules](https://html.spec.whatwg.org/#register-speculation-rules) — To register speculation rules given a Window global, a speculation rules parse result result, and an optional boolean queueErrors (default false):
  - 1. If result's error to rethrow is not null:
    - 1a. If queueErrors is true, then queue a global task on the DOM manipulation task source given global to perform the following step:
      - 1a.i. Report an exception given by result's error to rethrow for global.
    - 1b. Otherwise, report an exception given by result's error to rethrow for global.
    - 1c. Return.
  - 2. Append result's speculation rule set to global's associated Document's speculation rule sets.
  - 3. Consider speculative loads for global's associated Document.

- [x] [unregister speculation rules](https://html.spec.whatwg.org/#unregister-speculation-rules) — To unregister speculation rules given a Window global and a speculation rules parse result result:
  - 1. If result's error to rethrow is not null, then return.
  - 2. Remove result's speculation rule set from global's associated Document's speculation rule sets.
  - 3. Consider speculative loads for global's associated Document.

- [x] [update speculation rules](https://html.spec.whatwg.org/#update-speculation-rules) — To update speculation rules given a Window global, a speculation rules parse result oldResult, and a speculation rules parse result newResult:
  - 1. Remove oldResult's speculation rule set from global's associated Document's speculation rule sets.
  - 2. Register speculation rules given global, newResult, and true.

#### [8.1.5 Module specifier resolution](https://html.spec.whatwg.org/#module-specifier-resolution)

Status: n/a (no JavaScript)

##### [8.1.5.1 The resolution algorithm](https://html.spec.whatwg.org/#the-resolution-algorithm)

Status: n/a (no JavaScript)

- [x] [resolve a module specifier](https://html.spec.whatwg.org/#resolve-a-module-specifier) — To resolve a module specifier given a script-or-null referringScript and a string specifier:
  - 1. Let settingsObject and baseURL be null.
  - 2. If referringScript is not null:
    - 2a. Set settingsObject to referringScript's settings object.
    - 2b. Set baseURL to referringScript's base URL.
  - 3. Otherwise:
    - 3a. Assert: there is a current settings object.
    - 3b. Set settingsObject to the current settings object.
    - 3c. Set baseURL to settingsObject's API base URL.
  - 4. Let importMap be an empty import map.
  - 5. If settingsObject's global object implements Window, then set importMap to settingsObject's global object's import map.
  - 6. Let serializedBaseURL be baseURL, serialized.
  - 7. Let asURL be the result of resolving a URL-like module specifier given specifier and baseURL.
  - 8. Let normalizedSpecifier be the serialization of asURL, if asURL is non-null; otherwise, specifier.
  - 9. Let result be a URL-or-null, initially null.
  - 10. For each scopePrefix → scopeImports of importMap's scopes:
    - 10a. If scopePrefix is serializedBaseURL, or if scopePrefix ends with U+002F (/) and scopePrefix is a code unit prefix of serializedBaseURL:
      - 10a.i. Let scopeImportsMatch be the result of resolving an imports match given normalizedSpecifier, asURL, and scopeImports.
      - 10a.ii. If scopeImportsMatch is not null, then set result to scopeImportsMatch, and break.
  - 11. If result is null, set result to the result of resolving an imports match given normalizedSpecifier, asURL, and importMap's imports.
  - 12. If result is null, set it to asURL.
  - 13. If result is not null:
    - 13a. Add module to resolved module set given settingsObject, serializedBaseURL, normalizedSpecifier, and asURL.
    - 13b. Return result.
  - 14. Throw a TypeError indicating that specifier was a bare specifier, but was not remapped to anything by importMap.

- [x] [resolve an imports match](https://html.spec.whatwg.org/#resolving-an-imports-match) — To resolve an imports match, given a string normalizedSpecifier, a URL-or-null asURL, and a module specifier map specifierMap:
  - 1. For each specifierKey → resolutionResult of specifierMap:
    - 1a. If specifierKey is normalizedSpecifier:
      - 1a.i. If resolutionResult is null, then throw a TypeError indicating that resolution of specifierKey was blocked by a null entry.
      - 1a.ii. Assert: resolutionResult is a URL.
      - 1a.iii. Return resolutionResult.
    - 1b. If all of the following are true: specifierKey ends with U+002F (/);; specifierKey is a code unit prefix of normalizedSpecifier; and; either asURL is null, or asURL is special, then:
      - 1b.i. If resolutionResult is null, then throw a TypeError indicating that the resolution of specifierKey was blocked by a null entry.
      - 1b.ii. Assert: resolutionResult is a URL.
      - 1b.iii. Let afterPrefix be the portion of normalizedSpecifier after the initial specifierKey prefix.
      - 1b.iv. Assert: resolutionResult, serialized, ends with U+002F (/), as enforced during parsing.
      - 1b.v. Let url be the result of URL parsing afterPrefix with resolutionResult.
      - 1b.vi. If url is failure, then throw a TypeError indicating that resolution of normalizedSpecifier was blocked since the afterPrefix portion could not be URL-parsed relative to the resolutionResult mapped to by the specifierKey prefix.
      - 1b.vii. Assert: url is a URL.
      - 1b.viii. If the serialization of resolutionResult is not a code unit prefix of the serialization of url, then throw a TypeError indicating that the resolution of normalizedSpecifier was blocked due to it backtracking above its prefix specifierKey.
      - 1b.ix. Return url.
  - 2. Return null.

- [x] [resolve a URL-like module specifier](https://html.spec.whatwg.org/#resolving-a-url-like-module-specifier) — To resolve a URL-like module specifier, given a string specifier and a URL baseURL:
  - 1. If specifier starts with "/", "./", or "../":
    - 1a. Let url be the result of URL parsing specifier with baseURL.
    - 1b. If url is failure, then return null.
    - 1c. Return url.
  - 2. Let url be the result of URL parsing specifier (with no base URL).
  - 3. If url is failure, then return null.
  - 4. Return url.

##### [8.1.5.2 Import maps](https://html.spec.whatwg.org/#import-maps)

Status: n/a (no JavaScript)

- [x] [import map authoring requirements](https://html.spec.whatwg.org/#import-map-authoring-requirements) — The child text content of a script element representing an import map must match the following import map authoring requirements:

- [x] [valid module specifier map](https://html.spec.whatwg.org/#valid-module-specifier-map) — A valid module specifier map is a JSON object that meets the following requirements:

##### [8.1.5.3 Import map processing model](https://html.spec.whatwg.org/#import-map-processing-model)

Status: n/a (no JavaScript)
<!-- definitions: import-map concept-import-map-imports concept-import-map-scopes concept-import-map-integrity module-specifier-map module-integrity-map empty-import-map specifier-resolution-record specifier-resolution-record-serialized-base-url specifier-resolution-record-specifier specifier-resolution-record-as-url -->

- [x] [add module to resolved module set](https://html.spec.whatwg.org/#add-module-to-resolved-module-set) — To add module to resolved module set given an environment settings object settingsObject, a string serializedBaseURL, a string normalizedSpecifier, and a URL-or-null asURL:
  - 1. Let global be settingsObject's global object.
  - 2. If global does not implement Window, then return.
  - 3. Let record be a new specifier resolution record, with serialized base URL set to serializedBaseURL, specifier set to normalizedSpecifier, and specifier as a URL set to asURL.
  - 4. Append record to global's resolved module set.

- [x] [parse an import map string](https://html.spec.whatwg.org/#parse-an-import-map-string) — To parse an import map string, given a string input and a URL baseURL:
  - 1. Let parsed be the result of parsing a JSON string to an Infra value given input.
  - 2. If parsed is not an ordered map, then throw a TypeError indicating that the top-level value needs to be a JSON object.
  - 3. Let sortedAndNormalizedImports be an empty ordered map.
  - 4. If parsed["imports"] exists:
    - 4a. If parsed["imports"] is not an ordered map, then throw a TypeError indicating that the value for the "imports" top-level key needs to be a JSON object.
    - 4b. Set sortedAndNormalizedImports to the result of sorting and normalizing a module specifier map given parsed["imports"] and baseURL.
  - 5. Let sortedAndNormalizedScopes be an empty ordered map.
  - 6. If parsed["scopes"] exists:
    - 6a. If parsed["scopes"] is not an ordered map, then throw a TypeError indicating that the value for the "scopes" top-level key needs to be a JSON object.
    - 6b. Set sortedAndNormalizedScopes to the result of sorting and normalizing scopes given parsed["scopes"] and baseURL.
  - 7. Let normalizedIntegrity be an empty ordered map.
  - 8. If parsed["integrity"] exists:
    - 8a. If parsed["integrity"] is not an ordered map, then throw a TypeError indicating that the value for the "integrity" top-level key needs to be a JSON object.
    - 8b. Set normalizedIntegrity to the result of normalizing a module integrity map given parsed["integrity"] and baseURL.
  - 9. If parsed's keys contains any items besides "imports", "scopes", or "integrity", then the user agent should report a warning to the console indicating that an invalid top-level key was present in the import map.
  - 10. Return an import map whose imports are sortedAndNormalizedImports, whose scopes are sortedAndNormalizedScopes, and whose integrity are normalizedIntegrity.

- [x] [merge module specifier maps](https://html.spec.whatwg.org/#merge-module-specifier-maps) — To merge module specifier maps, given a module specifier map newMap and a module specifier map oldMap:
  - 1. Let mergedMap be a deep copy of oldMap.
  - 2. For each specifier → url of newMap:
    - 2a. If specifier exists in oldMap:
      - 2a.i. The user agent may report a warning to the console indicating the ignored rule. They may choose to avoid reporting if the rule is identical to an existing one.
      - 2a.ii. Continue.
    - 2b. Set mergedMap[specifier] to url.
  - 3. Return mergedMap.

- [x] [merge existing and new import maps](https://html.spec.whatwg.org/#merge-existing-and-new-import-maps) — To merge existing and new import maps, given a global object global and an import map newImportMap:
  - 1. Let newImportMapScopes be a deep copy of newImportMap's scopes.
  - 2. Let oldImportMap be global's import map.
  - 3. Let newImportMapImports be a deep copy of newImportMap's imports.
  - 4. For each scopePrefix → scopeImports of newImportMapScopes:
    - 4a. For each record of global's resolved module set:
      - 4a.i. If scopePrefix is record's serialized base URL, or if scopePrefix ends with U+002F (/) and scopePrefix is a code unit prefix of record's serialized base URL:
        - 4a.i.A. For each specifierKey → resolutionResult of scopeImports:
          - 4a.i.A.1. If specifierKey is record's specifier, or if all of the following conditions are true: specifierKey ends with U+002F (/);; specifierKey is a code unit prefix of record's specifier;; either record's specifier as a URL is null or is special, then:
            - 4a.i.A.1a. The user agent may report a warning to the console indicating the ignored rule. They may choose to avoid reporting if the rule is identical to an existing one.
            - 4a.i.A.1b. Remove scopeImports[specifierKey].
    - 4b. If scopePrefix exists in oldImportMap's scopes, then set oldImportMap's scopes[scopePrefix] to the result of merging module specifier maps, given scopeImports and oldImportMap's scopes[scopePrefix].
    - 4c. Otherwise, set oldImportMap's scopes[scopePrefix] to scopeImports.
  - 5. For each url → integrity of newImportMap's integrity:
    - 5a. If url exists in oldImportMap's integrity:
      - 5a.i. The user agent may report a warning to the console indicating the ignored rule. They may choose to avoid reporting if the rule is identical to an existing one.
      - 5a.ii. Continue.
    - 5b. Set oldImportMap's integrity[url] to integrity.
  - 6. For each record of global's resolved module set:
    - 6a. For each specifier → url of newImportMapImports:
      - 6a.i. If specifier starts with record's specifier:
        - 6a.i.A. The user agent may report a warning to the console indicating the ignored rule. They may choose to avoid reporting if the rule is identical to an existing one.
        - 6a.i.B. Remove newImportMapImports[specifier].
  - 7. Set oldImportMap's imports to the result of merge module specifier maps, given newImportMapImports and oldImportMap's imports.

- [x] [sort and normalize a module specifier map](https://html.spec.whatwg.org/#sorting-and-normalizing-a-module-specifier-map) — To sort and normalize a module specifier map, given an ordered map originalMap and a URL baseURL:
  - 1. Let normalized be an empty ordered map.
  - 2. For each specifierKey → value of originalMap:
    - 2a. Let normalizedSpecifierKey be the result of normalizing a specifier key given specifierKey and baseURL.
    - 2b. If normalizedSpecifierKey is null, then continue.
    - 2c. If value is not a string:
      - 2c.i. The user agent may report a warning to the console indicating that addresses need to be strings.
      - 2c.ii. Set normalized[normalizedSpecifierKey] to null.
      - 2c.iii. Continue.
    - 2d. Let addressURL be the result of resolving a URL-like module specifier given value and baseURL.
    - 2e. If addressURL is null:
      - 2e.i. The user agent may report a warning to the console indicating that the address was invalid.
      - 2e.ii. Set normalized[normalizedSpecifierKey] to null.
      - 2e.iii. Continue.
    - 2f. If specifierKey ends with U+002F (/), and the serialization of addressURL does not end with U+002F (/):
      - 2f.i. The user agent may report a warning to the console indicating that an invalid address was given for the specifier key specifierKey; since specifierKey ends with a slash, the address needs to as well.
      - 2f.ii. Set normalized[normalizedSpecifierKey] to null.
      - 2f.iii. Continue.
    - 2g. Set normalized[normalizedSpecifierKey] to addressURL.
  - 3. Return the result of sorting in descending order normalized, with an entry a being less than an entry b if a's key is code unit less than b's key.

- [x] [sort and normalize scopes](https://html.spec.whatwg.org/#sorting-and-normalizing-scopes) — To sort and normalize scopes, given an ordered map originalMap and a URL baseURL:
  - 1. Let normalized be an empty ordered map.
  - 2. For each scopePrefix → potentialSpecifierMap of originalMap:
    - 2a. If potentialSpecifierMap is not an ordered map, then throw a TypeError indicating that the value of the scope with prefix scopePrefix needs to be a JSON object.
    - 2b. Let scopePrefixURL be the result of URL parsing scopePrefix with baseURL.
    - 2c. If scopePrefixURL is failure:
      - 2c.i. The user agent may report a warning to the console that the scope prefix URL was not parseable.
      - 2c.ii. Continue.
    - 2d. Let normalizedScopePrefix be the serialization of scopePrefixURL.
    - 2e. Set normalized[normalizedScopePrefix] to the result of sorting and normalizing a module specifier map given potentialSpecifierMap and baseURL.
  - 3. Return the result of sorting in descending order normalized, with an entry a being less than an entry b if a's key is code unit less than b's key.

- [x] [normalize a module integrity map](https://html.spec.whatwg.org/#normalizing-a-module-integrity-map) — To normalize a module integrity map, given an ordered map originalMap:
  - 1. Let normalized be an empty ordered map.
  - 2. For each key → value of originalMap:
    - 2a. Let resolvedURL be the result of resolving a URL-like module specifier given key and baseURL.
    - 2b. If resolvedURL is null:
      - 2b.i. The user agent may report a warning to the console indicating that the key failed to resolve.
      - 2b.ii. Continue.
    - 2c. If value is not a string:
      - 2c.i. The user agent may report a warning to the console indicating that integrity metadata values need to be strings.
      - 2c.ii. Continue.
    - 2d. Set normalized[resolvedURL] to value.
  - 3. Return normalized.

- [x] [normalize a specifier key](https://html.spec.whatwg.org/#normalizing-a-specifier-key) — To normalize a specifier key, given a string specifierKey and a URL baseURL:
  - 1. If specifierKey is the empty string:
    - 1a. The user agent may report a warning to the console indicating that specifier keys may not be the empty string.
    - 1b. Return null.
  - 2. Let url be the result of resolving a URL-like module specifier, given specifierKey and baseURL.
  - 3. If url is not null, then return the serialization of url.
  - 4. Return specifierKey.

#### [8.1.6 JavaScript specification host hooks](https://html.spec.whatwg.org/#javascript-specification-host-hooks)

Status: n/a (no JavaScript)

##### [8.1.6.1 HostEnsureCanAddPrivateElement(O)](https://html.spec.whatwg.org/#the-hostensurecanaddprivateelement-implementation)

Status: n/a (no JavaScript)

##### [8.1.6.2 HostEnsureCanCompileStrings(realm, parameterStrings, bodyString, codeString, compilationType, parameterArgs, bodyArg)](https://html.spec.whatwg.org/#hostensurecancompilestrings%28realm,-parameterstrings,-bodystring,-codestring,-compilationtype,-parameterargs,-bodyarg%29)

Status: n/a (no JavaScript)

##### [8.1.6.3 HostGetCodeForEval(argument)](https://html.spec.whatwg.org/#hostgetcodeforeval%28argument%29)

Status: n/a (no JavaScript)

##### [8.1.6.4 HostPromiseRejectionTracker(promise, operation)](https://html.spec.whatwg.org/#the-hostpromiserejectiontracker-implementation)

Status: n/a (no JavaScript)

##### [8.1.6.5 HostSystemUTCEpochNanoseconds(global)](https://html.spec.whatwg.org/#hostsystemutcepochnanoseconds)

Status: n/a (no JavaScript)

##### [8.1.6.6 Job-related host hooks](https://html.spec.whatwg.org/#integration-with-javascript-jobs)

Status: n/a (no JavaScript)

###### [8.1.6.6.1 HostCallJobCallback(callback, V, argumentsList)](https://html.spec.whatwg.org/#hostcalljobcallback)

Status: n/a (no JavaScript)

###### [8.1.6.6.2 HostEnqueueFinalizationRegistryCleanupJob(finalizationRegistry)](https://html.spec.whatwg.org/#hostenqueuefinalizationregistrycleanupjob)

Status: n/a (no JavaScript)
<!-- definitions: javascript-engine-task-source -->

###### [8.1.6.6.3 HostEnqueueGenericJob(job, realm)](https://html.spec.whatwg.org/#hostenqueuegenericjob)

Status: n/a (no JavaScript)

###### [8.1.6.6.4 HostEnqueuePromiseJob(job, realm)](https://html.spec.whatwg.org/#hostenqueuepromisejob)

Status: n/a (no JavaScript)

###### [8.1.6.6.5 HostEnqueueTimeoutJob(job, realm, milliseconds)](https://html.spec.whatwg.org/#hostenqueuetimeoutjob)

Status: n/a (no JavaScript)

###### [8.1.6.6.6 HostMakeJobCallback(callable)](https://html.spec.whatwg.org/#hostmakejobcallback)

Status: n/a (no JavaScript)

##### [8.1.6.7 Module-related host hooks](https://html.spec.whatwg.org/#integration-with-the-javascript-module-system)

Status: n/a (no JavaScript)
<!-- definitions: module-map -->

###### [8.1.6.7.1 HostGetImportMetaProperties(moduleRecord)](https://html.spec.whatwg.org/#hostgetimportmetaproperties)

Status: n/a (no JavaScript)
<!-- definitions: import-meta-url import-meta-resolve -->

###### [8.1.6.7.2 HostGetSupportedImportAttributes()](https://html.spec.whatwg.org/#hostgetsupportedimportattributes)

Status: n/a (no JavaScript)

###### [8.1.6.7.3 HostLoadImportedModule(referrer, moduleRequest, loadState, payload)](https://html.spec.whatwg.org/#hostloadimportedmodule)

Status: n/a (no JavaScript)

#### [8.1.7 Event loops](https://html.spec.whatwg.org/#event-loops)

Status: reviewed (2026-11-01)
> The sections below.

##### [8.1.7.1 Definitions](https://html.spec.whatwg.org/#definitions-3)

Status: reviewed (2026-11-01)
> `html::EventLoop`: one process-wide loop (FIXME), with a single queue for every task source, which keeps each source's tasks in order. A task of a document is runnable unless the document's tasks are suspended, and is dropped once its active objects are stopped (a subresource load's runs regardless, `StoppedDocumentTask::Run`); a document that isn't fully active otherwise still runs its tasks, as in Chrome (see `DIFFERENCES.md`).
<!-- definitions: event-loop concept-agent-event-loop worker-event-loop-2 worklet-event-loop task-queue concept-task concept-task-steps concept-task-source concept-task-document script-evaluation-environment-settings-object-set task-source currently-running-task microtask-queue microtask performing-a-microtask-checkpoint last-render-opportunity-time last-idle-period-start-time -->

- [x] [window event loop](https://html.spec.whatwg.org/#window-event-loop) — The event loop of a similar-origin window agent is known as a window event loop. The event loop of a dedicated worker agent, shared worker agent, or service worker agent is known as a worker event loop. And the event loop of a worklet agent is known as a worklet event loop.

- [x] [runnable](https://html.spec.whatwg.org/#concept-task-runnable) — A task is runnable if its document is either null or fully active.

- [x] [same-loop windows](https://html.spec.whatwg.org/#same-loop-windows) — To get the same-loop windows for a window event loop loop, return all Window objects whose relevant agent's event loop is loop.

##### [8.1.7.2 Queuing tasks](https://html.spec.whatwg.org/#queuing-tasks)

Status: reviewed (2026-11-01)
> `EventLoop::QueueTask` (with a document for a global or element task) and `QueueMicrotask`.
<!-- definitions: microtask-task-source implied-event-loop -->

- [x] [queue a task](https://html.spec.whatwg.org/#queue-a-task) — To queue a task on a task source source, which performs a series of steps steps, optionally given an event loop event loop and a document document:
  - 1. If event loop was not given, set event loop to the implied event loop.
  - 2. If document was not given, set document to the implied document.
  - 3. Let task be a new task.
  - 4. Set task's steps to steps.
  - 5. Set task's source to source.
  - 6. Set task's document to the document.
  - 7. Set task's script evaluation environment settings object set to an empty set.
  - 8. Let queue be the task queue to which source is associated on event loop.
  - 9. Append task to queue.

- [x] [queue a global task](https://html.spec.whatwg.org/#queue-a-global-task) — To queue a global task on a task source source, with a global object global and a series of steps steps:
  - 1. Let event loop be global's relevant agent's event loop.
  - 2. Let document be global's associated Document, if global is a Window object; otherwise null.
  - 3. Queue a task given source, event loop, document, and steps.

- [x] [queue an element task](https://html.spec.whatwg.org/#queue-an-element-task) — To queue an element task on a task source source, with an element element and a series of steps steps:
  - 1. Let global be element's relevant global object.
  - 2. Queue a global task given source, global, and steps.

- [x] [queue a microtask](https://html.spec.whatwg.org/#queue-a-microtask) — To queue a microtask which performs a series of steps steps, optionally given a document document:
  - 1. Assert: there is a surrounding agent. I.e., this algorithm is not called while in parallel.
  - 2. Let eventLoop be the surrounding agent's event loop.
  - 3. If document was not given, set document to the implied document.
  - 4. Let microtask be a new task.
  - 5. Set microtask's steps to steps.
  - 6. Set microtask's source to the microtask task source.
  - 7. Set microtask's document to document.
  - 8. Set microtask's script evaluation environment settings object set to an empty set.
  - 9. Enqueue microtask on eventLoop's microtask queue.

- [x] [implied document](https://html.spec.whatwg.org/#implied-document) — The implied document when queuing a task on an event loop event loop is determined as follows:
  - 1. If event loop is not a window event loop, then return null.
  - 2. If the task is being queued in the context of an element, then return the element's node document.
  - 3. If the task is being queued in the context of a browsing context, then return the browsing context's active document.
  - 4. If the task is being queued by or for a script, then return the script's settings object's global object's associated Document.
  - 5. Assert: this step is never reached, because one of the previous conditions is true.

##### [8.1.7.3 Processing model](https://html.spec.whatwg.org/#event-loop-processing-model)

Status: reviewed (2026-11-01)
> Fixed: the focus fixup step of updating the rendering sets the Navigation API's focus changed during ongoing navigation to false, so a navigation that finishes afterwards resets the focus.
> `EventLoop::RunNextTask` runs the oldest runnable task and a microtask checkpoint; the host runs the loop (`Run`), updates the rendering (`Page::UpdateRendering`, in upstream's order of steps, leaving out render-blocked documents and trying again in the next one) and starts idle periods each frame. There's no long task reporting or canvas context loss.
<!-- definitions: update-the-rendering context-lost-steps synchronous-section -->

- [x] [An event loop must continually run through the following steps for as long as i…](https://html.spec.whatwg.org/#event-loop-processing-model) — An event loop must continually run through the following steps for as long as it exists:
  - 1. Let oldestTask and taskStartTime be null.
  - 2. If the event loop has a task queue with at least one runnable task:
    - 2a. Let taskQueue be one such task queue, chosen in an implementation-defined manner.
    - 2b. Set taskStartTime to the unsafe shared current time.
    - 2c. Set oldestTask to the first runnable task in taskQueue, and remove it from taskQueue.
    - 2d. If oldestTask's document is not null, then record task start time given taskStartTime and oldestTask's document.
    - 2e. Set the event loop's currently running task to oldestTask.
    - 2f. Perform oldestTask's steps.
    - 2g. Set the event loop's currently running task back to null.
    - 2h. Perform a microtask checkpoint.
  - 3. Let taskEndTime be the unsafe shared current time. [HRT]
  - 4. If oldestTask is not null:
    - 4a. Let top-level browsing contexts be an empty set.
    - 4b. For each environment settings object settings of oldestTask's script evaluation environment settings object set:
      - 4b.i. Let global be settings's global object.
      - 4b.ii. If global is not a Window object, then continue.
      - 4b.iii. If global's browsing context is null, then continue.
      - 4b.iv. Let tlbc be global's browsing context's top-level browsing context.
      - 4b.v. If tlbc is not null, then append it to top-level browsing contexts.
    - 4c. Report long tasks, passing in taskStartTime, taskEndTime, top-level browsing contexts, and oldestTask.
    - 4d. If oldestTask's document is not null, then record task end time given taskEndTime and oldestTask's document.
  - 5. If this is a window event loop that has no runnable task in this event loop's task queues:
    - 5a. Set this event loop's last idle period start time to the unsafe shared current time.
    - 5b. Let computeDeadline be the following steps:
      - 5b.i. Let deadline be this event loop's last idle period start time plus 50.
      - 5b.ii. Let hasPendingRenders be false.
      - 5b.iii. For each windowInSameLoop of the same-loop windows for this event loop:
        - 5b.iii.A. If windowInSameLoop's map of animation frame callbacks is not empty, or if the user agent believes that the windowInSameLoop might have pending rendering updates, set hasPendingRenders to true.
        - 5b.iii.B. Let timerCallbackEstimates be the result of getting the values of windowInSameLoop's map of active timers.
        - 5b.iii.C. For each timeoutDeadline of timerCallbackEstimates, if timeoutDeadline is less than deadline, set deadline to timeoutDeadline.
      - 5b.iv. If hasPendingRenders is true:
        - 5b.iv.A. Let nextRenderDeadline be this event loop's last render opportunity time plus (1000 divided by the current refresh rate). The refresh rate can be hardware- or implementation-specific. For a refresh rate of 60Hz, the nextRenderDeadline would be about 16.67ms after the last render opportunity time.
        - 5b.iv.B. If nextRenderDeadline is less than deadline, then return nextRenderDeadline.
      - 5b.v. Return deadline.
    - 5c. For each win of the same-loop windows for this event loop, perform the start an idle period algorithm for win with the following step: return the result of calling computeDeadline, coarsened given win's relevant settings object's cross-origin isolated capability. [REQUESTIDLECALLBACK]
  - 6. If this is a worker event loop:
    - 6a. If this event loop's agent's single realm's global object is a supported DedicatedWorkerGlobalScope and the user agent believes that it would benefit from having its rendering updated at this time:
      - 6a.i. Let now be the current high resolution time given the DedicatedWorkerGlobalScope. [HRT]
      - 6a.ii. Run the animation frame callbacks for that DedicatedWorkerGlobalScope, passing in now as the timestamp.
      - 6a.iii. Update the rendering of that dedicated worker to reflect the current state.
    - 6b. If there are no tasks in the event loop's task queues and the WorkerGlobalScope object's closing flag is true, then destroy the event loop, aborting these steps, resuming the run a worker steps described in the Web workers section below.

- [x] [A window event loop eventLoop must also run the following in parallel, as long…](https://html.spec.whatwg.org/#event-loop-processing-model) — A window event loop eventLoop must also run the following in parallel, as long as it exists:
  - 1. Wait until at least one navigable whose active document's relevant agent's event loop is eventLoop might have a rendering opportunity.
  - 2. Set eventLoop's last render opportunity time to the unsafe shared current time.
  - 3. For each navigable that has a rendering opportunity, queue a global task on the rendering task source given navigable's active window to update the rendering:
    - 3a. Let frameTimestamp be eventLoop's last render opportunity time.
    - 3b. Let docs be all fully active Document objects whose relevant agent's event loop is eventLoop, sorted arbitrarily except that the following conditions must be met: Any Document B whose container document is A must be listed after A in the list.; If there are two documents A and B that both have the same non-null container document C, then the order of A and B in the list must match the shadow-incl…
    - 3c. Filter non-renderable documents: Remove from docs any Document object doc for which any of the following are true: doc is render-blocked;; doc's visibility state is "hidden";; doc's rendering is suppressed for view transitions; or; doc's node navigable doesn't currently have a rendering opportunity.
    - 3d. Unnecessary rendering: Remove from docs any Document object doc for which all of the following are true: the user agent believes that updating the rendering of doc's node navigable would have no visible effect; and; doc's map of animation frame callbacks is empty.
    - 3e. Remove from docs all Document objects for which the user agent believes that it's preferable to skip updating the rendering for other reasons.
    - 3f. For each doc of docs, reveal doc.
    - 3g. For each doc of docs, flush autofocus candidates for doc if its node navigable is a top-level traversable.
    - 3h. For each doc of docs, run the resize steps for doc. [CSSOMVIEW]
    - 3i. For each doc of docs, run the scroll steps for doc. [CSSOMVIEW]
    - 3j. For each doc of docs, evaluate media queries and report changes for doc. [CSSOMVIEW]
    - 3k. For each doc of docs, update animations and send events for doc, passing in relative high resolution time given frameTimestamp and doc's relevant global object as the timestamp. [WEBANIMATIONS]
    - 3l. For each doc of docs, run the fullscreen steps for doc. [FULLSCREEN]
    - 3m. For each doc of docs, if the user agent detects that the backing storage associated with a CanvasRenderingContext2D or an OffscreenCanvasRenderingContext2D, context, has been lost, then it must run the context lost steps for each such context:
      - 3m.i. Let canvas be the value of context's canvas attribute, if context is a CanvasRenderingContext2D, or the associated OffscreenCanvas object for context otherwise.
      - 3m.ii. Set context's context lost to true.
      - 3m.iii. Reset the rendering context to its default state given context.
      - 3m.iv. Let shouldRestore be the result of firing an event named contextlost at canvas, with the cancelable attribute initialized to true.
      - 3m.v. If shouldRestore is false, then abort these steps.
      - 3m.vi. Attempt to restore context by creating a backing storage using context's attributes and associating them with context. If this fails, then abort these steps.
      - 3m.vii. Set context's context lost to false.
      - 3m.viii. Fire an event named contextrestored at canvas.
    - 3n. For each doc of docs, run the animation frame callbacks for doc, passing in the relative high resolution time given frameTimestamp and doc's relevant global object as the timestamp.
    - 3o. Let unsafeStyleAndLayoutStartTime be the unsafe shared current time.
    - 3p. For each doc of docs:
      - 3p.i. Let resizeObserverDepth be 0.
      - 3p.ii. While true:
        - 3p.ii.A. Recalculate styles and update layout for doc.
        - 3p.ii.B. Let hadInitialVisibleContentVisibilityDetermination be false.
        - 3p.ii.C. For each element element with 'auto' used value of 'content-visibility':
          - 3p.ii.C.1. Let checkForInitialDetermination be true if element's proximity to the viewport is not determined and it is not relevant to the user. Otherwise, let checkForInitialDetermination be false.
          - 3p.ii.C.2. Determine proximity to the viewport for element.
          - 3p.ii.C.3. If checkForInitialDetermination is true and element is now relevant to the user, then set hadInitialVisibleContentVisibilityDetermination to true.
        - 3p.ii.D. If hadInitialVisibleContentVisibilityDetermination is true, then continue.
        - 3p.ii.E. Gather active resize observations at depth resizeObserverDepth for doc.
        - 3p.ii.F. If doc has active resize observations:
          - 3p.ii.F.1. Set resizeObserverDepth to the result of broadcasting active resize observations given doc.
          - 3p.ii.F.2. Continue.
        - 3p.ii.G. Otherwise, break.
      - 3p.iii. If doc has skipped resize observations, then deliver resize loop error given doc.
    - 3q. For each doc of docs, if the focused area of doc is not a focusable area, then run the focusing steps for doc's viewport, and set doc's relevant global object's navigation API's focus changed during ongoing navigation to false.
    - 3r. For each doc of docs, perform pending transition operations for doc. [CSSVIEWTRANSITIONS]
    - 3s. For each doc of docs, run the update intersection observations steps for doc, passing in the relative high resolution time given now and doc's relevant global object as the timestamp. [INTERSECTIONOBSERVER]
    - 3t. For each doc of docs, record rendering time for doc given unsafeStyleAndLayoutStartTime.
    - 3u. For each doc of docs, mark paint timing for doc.
    - 3v. For each doc of docs, update the rendering or user interface of doc and its node navigable to reflect the current state.
    - 3w. For each doc of docs, process top layer removals given doc.

- [x] [rendering opportunity](https://html.spec.whatwg.org/#rendering-opportunity) — A navigable has a rendering opportunity if the user agent is currently able to present the contents of the navigable to the user, accounting for hardware refresh rate constraints and user agent throttling for performance reasons, but considering content presentable even if it's outside the viewport.

- [x] [perform a microtask checkpoint](https://html.spec.whatwg.org/#perform-a-microtask-checkpoint) — When a user agent is to perform a microtask checkpoint:
  - 1. If the event loop's performing a microtask checkpoint is true, then return.
  - 2. Set the event loop's performing a microtask checkpoint to true.
  - 3. While the event loop's microtask queue is not empty:
    - 3a. Let oldestMicrotask be the result of dequeuing from the event loop's microtask queue.
    - 3b. Set the event loop's currently running task to oldestMicrotask.
    - 3c. Run oldestMicrotask.
    - 3d. Set the event loop's currently running task back to null.
  - 4. For each environment settings object settingsObject whose responsible event loop is this event loop, notify about rejected promises given settingsObject's global object.
  - 5. Cleanup Indexed Database transactions.
  - 6. Perform ClearKeptObjects().
  - 7. Set the event loop's performing a microtask checkpoint to false.
  - 8. Record timing info for microtask checkpoint.

- [x] [await a stable state](https://html.spec.whatwg.org/#await-a-stable-state) — When an algorithm running in parallel is to await a stable state, the user agent must queue a microtask that runs the following steps, and must then stop executing (execution of the algorithm resumes when the microtask is run, as described in the following steps):
  - 1. Run the algorithm's synchronous section.
  - 2. Resume execution of the algorithm in parallel, if appropriate, as described in the algorithm's steps.

- [x] [spin the event loop](https://html.spec.whatwg.org/#spin-the-event-loop) — Algorithm steps that say to spin the event loop until a condition goal is met are equivalent to substituting in the following algorithm steps:
  - 1. Let task be the event loop's currently running task.
  - 2. Let task source be task's source.
  - 3. Let old stack be a copy of the JavaScript execution context stack.
  - 4. Empty the JavaScript execution context stack.
  - 5. Perform a microtask checkpoint.
  - 6. In parallel:
    - 6a. Wait until the condition goal is met.
    - 6b. Queue a task on task source to:
      - 6b.i. Replace the JavaScript execution context stack with old stack.
      - 6b.ii. Perform any steps that appear after this spin the event loop instance in the original algorithm.
  - 7. Stop task, allowing whatever algorithm that invoked it to resume.

- [x] [pause](https://html.spec.whatwg.org/#pause) — Some of the algorithms in this specification, for historical reasons, require the user agent to pause while running a task until a condition goal is met. This means running the following steps:
  - 1. Let global be the current global object.
  - 2. Let timeBeforePause be the current high resolution time given global.
  - 3. If necessary, update the rendering or user interface of any Document or navigable to reflect the current state.
  - 4. Wait until the condition goal is met. While a user agent has a paused task, the corresponding event loop must not run further tasks, and any script in the currently running task must block. User agents should remain responsive to user input while paused, however, albeit in a reduced capacity since the event loop will not be doing anything.
  - 5. Record pause duration given the duration from timeBeforePause to the current high resolution time given global.

##### [8.1.7.4 Generic task sources](https://html.spec.whatwg.org/#generic-task-sources)

Status: reviewed (2026-11-01)
> `TaskSource`.
<!-- definitions: dom-manipulation-task-source user-interaction-task-source networking-task-source navigation-and-traversal-task-source rendering-task-source -->

##### [8.1.7.5 Dealing with the event loop from other specifications](https://html.spec.whatwg.org/#event-loop-for-spec-authors)

Status: reviewed (2026-11-01)
> No requirements.

- [x] [Putting this all together, we can provide a template for a typical algorithm th…](https://html.spec.whatwg.org/#event-loop-for-spec-authors) — Putting this all together, we can provide a template for a typical algorithm that needs to do work asynchronously:
  - 1. Do any synchronous setup work, while still on the event loop. This may include converting realm-specific JavaScript values into realm-agnostic specification-level values.
  - 2. Perform a set of potentially-expensive steps in parallel, operating entirely on realm-agnostic values, and producing a realm-agnostic result.
  - 3. Queue a global task, on a specified task source and given an appropriate global object, to convert the realm-agnostic result back into observable effects on the observable world of JavaScript objects on the event loop.

#### [8.1.8 Events](https://html.spec.whatwg.org/#events)

Status: reviewed (2026-11-01)
> The sections below.

##### [8.1.8.1 Event handlers](https://html.spec.whatwg.org/#event-handler-attributes)

Status: reviewed (2026-11-01)
> Event handlers are C++ functions set with `SetOn<Name>()`; content attributes are inert (see `DIFFERENCES.md`).
<!-- definitions: event-handlers event-handler-value event-handler-listener event-handler-name event-handler-target event-handler-map event-handler-content-attributes event-handler-event-type internal-raw-uncompiled-handler -->

- [x] [determine the target of an event handler](https://html.spec.whatwg.org/#determining-the-target-of-an-event-handler) — To determine the target of an event handler, given an EventTarget object eventTarget on which the event handler is exposed, and an event handler name name, the following steps are taken:
  - 1. If eventTarget is not a body element or a frameset element, then return eventTarget.
  - 2. If name is not the name of an attribute member of the WindowEventHandlers interface mixin and the Window-reflecting body element event handler set does not contain name, then return eventTarget.
  - 3. If eventTarget's node document is not an active document, then return null.
  - 4. Return eventTarget's node document's relevant global object.

- [x] [The getter of an event handler IDL attribute with name name, when called, must…](https://html.spec.whatwg.org/#event-handler-idl-attributes) — The getter of an event handler IDL attribute with name name, when called, must run these steps:
  - 1. Let eventTarget be the result of determining the target of an event handler given this object and name.
  - 2. If eventTarget is null, then return null.
  - 3. Return the result of getting the current value of the event handler given eventTarget and name.

- [x] [The setter of an event handler IDL attribute with name name, when called, must…](https://html.spec.whatwg.org/#event-handler-idl-attributes) — The setter of an event handler IDL attribute with name name, when called, must run these steps:
  - 1. Let eventTarget be the result of determining the target of an event handler given this object and name.
  - 2. If eventTarget is null, then return.
  - 3. If the given value is null, then deactivate an event handler given eventTarget and name.
  - 4. Otherwise:
    - 4a. Let handlerMap be eventTarget's event handler map.
    - 4b. Let eventHandler be handlerMap[name].
    - 4c. Set eventHandler's value to the given value.
    - 4d. Activate an event handler given eventTarget and name.

- [x] [deactivate an event handler](https://html.spec.whatwg.org/#deactivate-an-event-handler) — To deactivate an event handler given an EventTarget object eventTarget and a string name that is the name of an event handler, run these steps:
  - 1. Let handlerMap be eventTarget's event handler map.
  - 2. Let eventHandler be handlerMap[name].
  - 3. Set eventHandler's value to null.
  - 4. Let listener be eventHandler's listener.
  - 5. If listener is not null, then remove an event listener with eventTarget and listener.
  - 6. Set eventHandler's listener to null.

- [x] [erase all event listeners and handlers](https://html.spec.whatwg.org/#erase-all-event-listeners-and-handlers) — To erase all event listeners and handlers given an EventTarget object eventTarget, run these steps:
  - 1. If eventTarget has an associated event handler map, then for each name → eventHandler of eventTarget's associated event handler map, deactivate an event handler given eventTarget and name.
  - 2. Remove all event listeners given eventTarget.

- [x] [activate an event handler](https://html.spec.whatwg.org/#activate-an-event-handler) — To activate an event handler given an EventTarget object eventTarget and a string name that is the name of an event handler, run these steps:
  - 1. Let handlerMap be eventTarget's event handler map.
  - 2. Let eventHandler be handlerMap[name].
  - 3. If eventHandler's listener is not null, then return.
  - 4. Let callback be the result of creating a Web IDL EventListener instance representing a reference to a function of one argument that executes the steps of the event handler processing algorithm, given eventTarget, name, and its argument. The EventListener's callback context can be arbitrary; it does not impact the steps of the event handler processing algorithm. [DOM]
  - 5. Let listener be a new event listener whose type is the event handler event type corresponding to eventHandler and callback is callback.
  - 6. Add an event listener with eventTarget and listener.
  - 7. Set eventHandler's listener to listener.

- [x] [The event handler processing algorithm](https://html.spec.whatwg.org/#the-event-handler-processing-algorithm) — The event handler processing algorithm for an EventTarget object eventTarget, a string name representing the name of an event handler, and an Event object event is as follows:
  - 1. If the result of WebDriver BiDi scripting is enabled with eventTarget's relevant settings object is false, then return.
  - 2. Let callback be the result of getting the current value of the event handler given eventTarget and name.
  - 3. If callback is null, then return.
  - 4. If callback's associated realm's global object is a Window object whose associated Document is not fully active, then return.
  - 5. Let special error event handling be true if event is an ErrorEvent object, event's type is "error", and event's currentTarget implements the WindowOrWorkerGlobalScope mixin. Otherwise, let special error event handling be false.
  - 6. Process the Event object event as follows: If special error event handling is true Let return value be the result of invoking callback with « event's message, event's filename, event's lineno, event's colno, event's error », "rethrow", and with callback this value set to event's currentTarget. Otherwise Let return value be the result of invoking callback with « event », "rethrow", and with callba…
  - 7. Process return value as follows: If event is a BeforeUnloadEvent object and event's type is "beforeunload" If return value is not null: If special error event handling is trueIf return value is true, then set event's canceled flag.Otherwise If return value is false, then set event's canceled flag.
    - 7a. Set event's canceled flag.
    - 7b. If event's returnValue attribute's value is the empty string, then set event's returnValue attribute's value to return value.

- [x] [get the current value of the event handler](https://html.spec.whatwg.org/#getting-the-current-value-of-the-event-handler) — When the user agent is to get the current value of the event handler given an EventTarget object eventTarget and a string name that is the name of an event handler, it must run these steps:
  - 1. Let handlerMap be eventTarget's event handler map.
  - 2. Let eventHandler be handlerMap[name].
  - 3. If eventHandler's value is an internal raw uncompiled handler:
    - 3a. If eventTarget is an element, then let element be eventTarget, and document be element's node document. Otherwise, eventTarget is a Window object, let element be null, and document be eventTarget's associated Document.
    - 3b. If document's browsing context is null, or document's active sandboxing flag set has its sandboxed scripts browsing context flag set, then return null.
    - 3c. Let body be the uncompiled script body in eventHandler's value.
    - 3d. Let location be the location where the script body originated, as given by eventHandler's value.
    - 3e. If element is not null and element has a form owner, let form owner be that form owner. Otherwise, let form owner be null.
    - 3f. Let settings object be the relevant settings object of document.
    - 3g. If body is not parsable as FunctionBody or if parsing detects an early error, then follow these substeps:
      - 3g.i. Set eventHandler's value to null.
      - 3g.ii. Let syntaxError be a new SyntaxError exception associated with settings object's realm which describes the error while parsing. It should be based on location, where the script body originated.
      - 3g.iii. Report an exception with syntaxError for settings object's global object.
      - 3g.iv. Return null.
    - 3h. Push settings object's realm execution context onto the JavaScript execution context stack; it is now the running JavaScript execution context.
    - 3i. Let function be the result of calling OrdinaryFunctionCreate, with arguments: functionPrototype%Function.prototype%sourceText If name is onerror and eventTarget is a Window objectThe string formed by concatenating "function ", name, "(event, source, lineno, colno, error) {", U+000A LF, body, U+000A LF, and "}".OtherwiseThe string formed by concatenating "function ", name, "(event) {", U+000A LF,…
      - 3i.i. Let realm be settings object's realm.
      - 3i.ii. Let scope be realm.[[GlobalEnv]].
      - 3i.iii. If eventHandler is an element's event handler, then set scope to NewObjectEnvironment(document, true, scope). (Otherwise, eventHandler is a Window object's event handler.)
      - 3i.iv. If form owner is not null, then set scope to NewObjectEnvironment(form owner, true, scope).
      - 3i.v. If element is not null, then set scope to NewObjectEnvironment(element, true, scope).
      - 3i.vi. Return scope.
    - 3j. Remove settings object's realm execution context from the JavaScript execution context stack.
    - 3k. Set function.[[ScriptOrModule]] to null.
    - 3l. Set eventHandler's value to the result of creating a Web IDL EventHandler callback function object whose object reference is function and whose callback context is settings object.
  - 4. Return eventHandler's value.

##### [8.1.8.2 Event handlers on elements, Document objects, and Window objects](https://html.spec.whatwg.org/#event-handlers-on-elements,-document-objects,-and-window-objects)

Status: reviewed (2026-11-01)
> `EventHandlerAttributes` forwards the Window-reflecting body element event handlers of `<body>` and `<frameset>` to the window.

- [x] [Window-reflecting body element event handler set](https://html.spec.whatwg.org/#window-reflecting-body-element-event-handler-set) — We call the set of the names of the event handlers listed in the first column of this table the Window-reflecting body element event handler set.

###### [8.1.8.2.1 IDL definitions](https://html.spec.whatwg.org/#idl-definitions)

Status: reviewed (2026-11-01)
> The `On<Name>()` handlers of `HTMLElement`, `Document` and `LocalDOMWindow`.

##### [8.1.8.3 Event firing](https://html.spec.whatwg.org/#event-firing)

Status: reviewed (2026-11-01)
> `dom::SimulateClick` fires a click: a `PointerEvent` that bubbles, is cancelable and composed, with a pointer ID of -1.

- [x] [Firing a synthetic pointer event named e](https://html.spec.whatwg.org/#fire-a-synthetic-pointer-event) — Firing a synthetic pointer event named e at target, with an optional not trusted flag, means running these steps:
  - 1. Let event be the result of creating an event using PointerEvent.
  - 2. Initialize event's type attribute to e.
  - 3. Initialize event's bubbles and cancelable attributes to true.
  - 4. Set event's composed flag.
  - 5. If the not trusted flag is set, initialize event's isTrusted attribute to false.
  - 6. Initialize event's ctrlKey, shiftKey, altKey, and metaKey attributes according to the current state of the key input device, if any (false for any keys that are not available).
  - 7. Initialize event's view attribute to target's node document's Window object, if any, and null otherwise.
  - 8. event's getModifierState() method is to return values appropriately describing the current state of the key input device.
  - 9. Return the result of dispatching event at target.

- [x] [Firing a click event](https://html.spec.whatwg.org/#fire-a-click-event) — Firing a click event at target means firing a synthetic pointer event named click at target.

### [8.2 The WindowOrWorkerGlobalScope mixin](https://html.spec.whatwg.org/#windoworworkerglobalscope-mixin)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::Origin` and `IsSecureContext`; `CrossOriginIsolated` is false.

- [x] [WindowOrWorkerGlobalScope.isSecureContext](https://html.spec.whatwg.org/#dom-issecurecontext) — The isSecureContext getter steps are to return true if this's relevant settings object is a secure context, or false otherwise.

- [x] [WindowOrWorkerGlobalScope.origin](https://html.spec.whatwg.org/#dom-origin) — The origin getter steps are to return this's relevant settings object's origin, serialized.

- [x] [WindowOrWorkerGlobalScope.crossOriginIsolated](https://html.spec.whatwg.org/#dom-crossoriginisolated) — The crossOriginIsolated getter steps are to return this's relevant settings object's cross-origin isolated capability.

- [x] [An element implementing the WindowOrWorkerGlobalScope mixin has the following e…](https://html.spec.whatwg.org/#dom-crossoriginisolated) — An element implementing the WindowOrWorkerGlobalScope mixin has the following extract an origin steps:
  - 1. If this's relevant settings object's origin is not same origin-domain with the entry settings object's origin, then return null.
  - 2. Return this's relevant settings object's origin.

### [8.3 Base64 utility methods](https://html.spec.whatwg.org/#atob)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::Btoa` and `Atob`.

- [x] [WindowOrWorkerGlobalScope.btoa(data)](https://html.spec.whatwg.org/#dom-btoa) — The btoa(data) method must throw an "InvalidCharacterError" DOMException if data contains any character whose code point is greater than U+00FF. Otherwise, the user agent must convert data to a byte sequence whose nth byte is the eight-bit representation of the nth code point of data, and then must apply forgiving-base64 encode to that byte sequence and return the result.

- [x] [WindowOrWorkerGlobalScope.atob(data)](https://html.spec.whatwg.org/#dom-atob) — The atob(data) method steps are:
  - 1. Let decodedData be the result of running forgiving-base64 decode on data.
  - 2. If decodedData is failure, then throw an "InvalidCharacterError" DOMException.
  - 3. Return decodedData.
